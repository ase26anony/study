/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Tests the uncovered default case in gcov-dump's option parsing
 * by providing invalid command-line flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define MAX_OUTPUT_LEN 4096

/**
 * Find the gcov-dump executable path.
 * Checks GCov_DUMP environment variable first, then common locations.
 * Returns 1 if found, 0 otherwise.
 */
static int find_gcov_dump(char *path, size_t path_len) {
    const char *env_path = getenv("GCOV_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        snprintf(path, path_len, "%s", env_path);
        return 1;
    }
    
    /* Common locations in GCC build trees */
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            snprintf(path, path_len, "%s", common_paths[i]);
            return 1;
        }
    }
    
    return 0;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if "unknown flag" appears in stderr, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
    int pipefd[2];
    pid_t pid;
    char output[MAX_OUTPUT_LEN] = {0};
    int found = 0;
    
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 0;
    }
    
    pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return 0;
    }
    
    if (pid == 0) { /* Child process */
        close(pipefd[0]); /* Close read end */
        
        /* Redirect stderr to pipe */
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        /* Execute gcov-dump with invalid flag */
        execl(gcov_dump_path, "gcov-dump", flag, (char *)NULL);
        
        /* If execl fails */
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else { /* Parent process */
        int status;
        close(pipefd[1]); /* Close write end */
        
        /* Read stderr output from pipe */
        ssize_t bytes_read = read(pipefd[0], output, sizeof(output) - 1);
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
            
            /* Check for "unknown flag" message */
            if (strstr(output, "unknown flag") != NULL) {
                printf("Found 'unknown flag' in output for flag %s:\n", flag);
                printf("%s\n", output);
                found = 1;
            }
        }
        
        close(pipefd[0]);
        waitpid(pid, &status, 0);
        
        return found;
    }
}

/**
 * Test various invalid flag scenarios.
 */
static void run_tests(const char *gcov_dump_path) {
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    /* Individual invalid flags */
    const char *invalid_flags[] = {
        "-x",        /* Simple invalid flag */
        "-z",        /* Another invalid flag */
        "-?",        /* Question mark (not 'h') */
        "-X",        /* Uppercase invalid flag */
        "-0",        /* Numeric flag */
        "-!",        /* Special character */
        NULL
    };
    
    /* Combined flags with invalid ones */
    const char *combined_args[] = {
        "-l -x -p",          /* Invalid in middle */
        "-x -l -p",          /* Invalid first */
        "-l -p -x",          /* Invalid last */
        "-x -z -?",          /* Multiple invalid */
        "-l -x",             /* Valid then invalid */
        "-x -- -l",          /* Invalid before -- */
        "-- -x",             /* Invalid after -- (should be treated as filename) */
        "-lp -x",            /* Combined valid flags then invalid */
        "-lpx",              /* Combined with invalid at end */
        NULL
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    
    /* Test individual invalid flags */
    printf("=== Testing individual invalid flags ===\n");
    for (int i = 0; invalid_flags[i] != NULL; i++) {
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, invalid_flags[i])) {
            passed_tests++;
        } else {
            printf("FAILED: Flag %s did not trigger 'unknown flag' error\n", 
                   invalid_flags[i]);
        }
    }
    
    /* Test combined arguments */
    printf("\n=== Testing combined arguments ===\n");
    for (int i = 0; combined_args[i] != NULL; i++) {
        total_tests++;
        
        /* Build command with multiple arguments */
        char cmd[MAX_CMD_LEN];
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_path, combined_args[i]);
        
        /* Use popen to capture stderr */
        FILE *fp = popen(cmd, "r");
        if (fp) {
            char output[MAX_OUTPUT_LEN] = {0};
            size_t total_read = 0;
            char buffer[256];
            
            /* Read both stdout and stderr (popen combines them) */
            while (fgets(buffer, sizeof(buffer), fp) != NULL && 
                   total_read < sizeof(output) - 1) {
                strcat(output + total_read, buffer);
                total_read += strlen(buffer);
            }
            
            pclose(fp);
            
            if (strstr(output, "unknown flag") != NULL) {
                printf("PASSED: Combined args '%s' triggered error\n", 
                       combined_args[i]);
                passed_tests++;
            } else {
                printf("FAILED: Combined args '%s' did not trigger error\n", 
                       combined_args[i]);
            }
        }
    }
    
    /* Test with a dummy .gcno file if available */
    printf("\n=== Testing with file arguments ===\n");
    const char *test_files[] = {
        "test.gcno",
        "test.gcda",
        NULL
    };
    
    for (int i = 0; test_files[i] != NULL; i++) {
        if (access(test_files[i], F_OK) == 0) {
            total_tests++;
            char cmd[MAX_CMD_LEN];
            snprintf(cmd, sizeof(cmd), "%s -x %s", gcov_dump_path, test_files[i]);
            
            FILE *fp = popen(cmd, "r");
            if (fp) {
                char output[MAX_OUTPUT_LEN] = {0};
                char buffer[256];
                
                while (fgets(buffer, sizeof(buffer), fp) != NULL) {
                    strcat(output, buffer);
                }
                
                pclose(fp);
                
                if (strstr(output, "unknown flag") != NULL) {
                    printf("PASSED: Invalid flag with file '%s' triggered error\n", 
                           test_files[i]);
                    passed_tests++;
                } else {
                    printf("FAILED: Invalid flag with file '%s' did not trigger error\n", 
                           test_files[i]);
                }
            }
        }
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    
    if (passed_tests > 0) {
        printf("\nSUCCESS: Successfully triggered the uncovered default case!\n");
    } else {
        printf("\nFAILURE: Could not trigger the uncovered default case.\n");
    }
}

int main(void) {
    char gcov_dump_path[MAX_CMD_LEN];
    
    /* Find gcov-dump executable */
    if (!find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path))) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Found gcov-dump: %s\n", gcov_dump_path);
    
    /* Run the tests */
    run_tests(gcov_dump_path);
    
    return EXIT_SUCCESS;
}
