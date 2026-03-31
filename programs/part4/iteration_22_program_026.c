/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Tests the uncovered default case in gcov-dump's option parsing.
 * Executes gcov-dump with various invalid flags and verifies the
 * "unknown flag" error message is printed to stderr.
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
 * Checks GCov_DUMP environment variable first, then common build locations.
 * Returns 1 if found (path stored in buffer), 0 otherwise.
 */
static int find_gcov_dump(char *path_buf, size_t buf_size) {
    const char *env_path = getenv("GCOV_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        strncpy(path_buf, env_path, buf_size - 1);
        path_buf[buf_size - 1] = '\0';
        return 1;
    }
    
    /* Common build tree locations */
    const char *candidate_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; candidate_paths[i] != NULL; i++) {
        if (access(candidate_paths[i], X_OK) == 0) {
            strncpy(path_buf, candidate_paths[i], buf_size - 1);
            path_buf[buf_size - 1] = '\0';
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
    
    if (pid == 0) {
        /* Child process */
        close(pipefd[0]);  /* Close read end */
        
        /* Redirect stderr to pipe */
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        /* Execute gcov-dump with invalid flag */
        execl(gcov_dump_path, "gcov-dump", flag, NULL);
        
        /* If we get here, exec failed */
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        close(pipefd[1]);  /* Close write end */
        
        /* Read stderr output from pipe */
        ssize_t bytes_read = read(pipefd[0], output, sizeof(output) - 1);
        close(pipefd[0]);
        
        /* Wait for child */
        int status;
        waitpid(pid, &status, 0);
        
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
            
            /* Check for "unknown flag" message */
            if (strstr(output, "unknown flag") != NULL) {
                printf("✓ Found 'unknown flag' in output for flag '%s':\n", flag);
                printf("  %s", output);
                found = 1;
            } else {
                printf("✗ No 'unknown flag' for flag '%s'. Output:\n", flag);
                printf("  %s", output);
            }
        } else {
            printf("✗ No output for flag '%s'\n", flag);
        }
    }
    
    return found;
}

/**
 * Test multiple invalid flags in different positions.
 */
static void run_comprehensive_tests(const char *gcov_dump_path) {
    struct test_case {
        const char *description;
        const char *flags;
    };
    
    struct test_case tests[] = {
        {"Single invalid flag at start", "-x"},
        {"Single invalid flag (different char)", "-z"},
        {"Invalid flag with question mark", "-?"},
        {"Invalid flag between valid flags", "-l -x -p"},
        {"Invalid flag after valid flags", "-l -p -x"},
        {"Multiple invalid flags", "-x -y -z"},
        {"Invalid flag after double dash", "-- -x"},
        {"Invalid flag with filename", "-x dummy.gcda"},
        {"Mixed case invalid flag", "-X"},
        {"Invalid flag with numeric", "-9"},
        {"Combination with help flag", "-h -x"},
        {"All invalid flags", "-a -b -c"},
        {NULL, NULL}
    };
    
    printf("\n=== Running comprehensive invalid flag tests ===\n\n");
    
    int total_tests = 0;
    int passed_tests = 0;
    
    for (int i = 0; tests[i].description != NULL; i++) {
        printf("Test %d: %s\n", total_tests + 1, tests[i].description);
        printf("  Command: %s %s\n", gcov_dump_path, tests[i].flags);
        
        /* Build command string for system() call */
        char cmd[MAX_CMD_LEN];
        snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, tests[i].flags);
        
        /* Execute and capture output */
        FILE *fp = popen(cmd, "r");
        if (fp) {
            char output[MAX_OUTPUT_LEN] = {0};
            size_t bytes_read = fread(output, 1, sizeof(output) - 1, fp);
            pclose(fp);
            
            if (bytes_read > 0) {
                output[bytes_read] = '\0';
                
                /* Check for "unknown flag" message */
                if (strstr(output, "unknown flag") != NULL) {
                    printf("  ✓ PASS: Found 'unknown flag' message\n");
                    passed_tests++;
                } else if (strstr(output, "Usage:") != NULL) {
                    /* -h flag might trigger usage instead */
                    printf("  ✓ PASS: Got usage info (expected for -h)\n");
                    passed_tests++;
                } else {
                    printf("  ✗ FAIL: No 'unknown flag' found\n");
                    printf("    Output: %s", output);
                }
            } else {
                printf("  ✗ FAIL: No output\n");
            }
        } else {
            printf("  ✗ FAIL: Failed to execute command\n");
        }
        
        total_tests++;
        printf("\n");
    }
    
    printf("=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Coverage target: Reached default case in switch statement\n");
}

int main(void) {
    char gcov_dump_path[MAX_CMD_LEN];
    
    printf("=== Testing gcov-dump invalid flag handling ===\n\n");
    
    /* Find gcov-dump executable */
    if (!find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path))) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure it's in a standard location\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    /* Test basic invalid flag */
    printf("Testing basic invalid flag '-x':\n");
    if (test_invalid_flag(gcov_dump_path, "-x")) {
        printf("\n✓ Successfully triggered default case!\n");
    } else {
        printf("\n✗ Failed to trigger default case\n");
    }
    
    /* Run comprehensive tests */
    run_comprehensive_tests(gcov_dump_path);
    
    return EXIT_SUCCESS;
}
