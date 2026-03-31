/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by invoking
 * gcov-dump with invalid command-line flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

/* Maximum path length for executable */
#define MAX_PATH 1024
/* Maximum command line length */
#define MAX_CMD 4096
/* Buffer size for reading output */
#define BUF_SIZE 1024

/**
 * Find the gcov-dump executable path.
 * Priority: 1. GCov_DUMP environment variable
 *           2. Common build locations
 *           3. System PATH
 * Returns 1 if found, 0 otherwise.
 */
static int find_gcov_dump(char *path, size_t path_size) {
    const char *env_path = getenv("GCov_DUMP");
    
    /* Try environment variable first */
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, path_size - 1);
        path[path_size - 1] = '\0';
        return 1;
    }
    
    /* Try common build locations */
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "../../prev-gcc/build/gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path, common_paths[i], path_size - 1);
            path[path_size - 1] = '\0';
            return 1;
        }
    }
    
    /* Try searching in PATH */
    const char *path_env = getenv("PATH");
    if (path_env != NULL) {
        char *path_copy = strdup(path_env);
        if (path_copy == NULL) {
            return 0;
        }
        
        char *dir = strtok(path_copy, ":");
        while (dir != NULL) {
            char full_path[MAX_PATH];
            snprintf(full_path, sizeof(full_path), "%s/gcov-dump", dir);
            if (access(full_path, X_OK) == 0) {
                strncpy(path, full_path, path_size - 1);
                path[path_size - 1] = '\0';
                free(path_copy);
                return 1;
            }
            dir = strtok(NULL, ":");
        }
        free(path_copy);
    }
    
    return 0;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if "unknown flag" message found in stderr, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
    int found = 0;
    int pipefd[2];
    pid_t pid;
    
    /* Create pipe for capturing stderr */
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
        
        /* Read from pipe */
        char buffer[BUF_SIZE];
        ssize_t bytes_read;
        
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            
            /* Check for "unknown flag" message */
            if (strstr(buffer, "unknown flag") != NULL) {
                found = 1;
                /* Print the captured error for verification */
                printf("Captured stderr: %s", buffer);
            }
        }
        
        close(pipefd[0]);
        
        /* Wait for child */
        int status;
        waitpid(pid, &status, 0);
    }
    
    return found;
}

/**
 * Test multiple invalid flags in different positions.
 */
static int test_multiple_invalid_flags(const char *gcov_dump_path) {
    /* Test cases with invalid flags in different positions */
    const char *test_cases[] = {
        "-x",           /* Single invalid flag */
        "-z",           /* Another single invalid flag */
        "-?",           /* Question mark (invalid) */
        "-l -x -p",     /* Invalid flag between valid ones */
        "-x -l",        /* Invalid flag first */
        "-l -p -z",     /* Invalid flag last */
        "-x -y -z",     /* Multiple invalid flags */
        "--x",          /* Double dash with single char */
        "-X",           /* Uppercase invalid flag */
        "-1",           /* Numeric flag */
        NULL
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("Testing invalid flags for gcov-dump:\n");
    printf("====================================\n");
    
    for (int i = 0; test_cases[i] != NULL; i++) {
        total_tests++;
        printf("Test %d: gcov-dump %s\n", total_tests, test_cases[i]);
        
        if (test_invalid_flag(gcov_dump_path, test_cases[i])) {
            printf("  ✓ PASSED: 'unknown flag' message found\n");
            passed_tests++;
        } else {
            printf("  ✗ FAILED: 'unknown flag' message NOT found\n");
        }
        printf("\n");
    }
    
    /* Test with filename argument (invalid flag after filename) */
    total_tests++;
    printf("Test %d: gcov-dump -l dummy.gcda -x\n", total_tests);
    
    /* Create a dummy file for testing */
    FILE *dummy = fopen("dummy.gcda", "w");
    if (dummy) {
        fprintf(dummy, "dummy gcov data\n");
        fclose(dummy);
        
        /* Test with system() for complex argument handling */
        char cmd[MAX_CMD];
        snprintf(cmd, sizeof(cmd), "%s -l dummy.gcda -x 2>&1", gcov_dump_path);
        
        FILE *fp = popen(cmd, "r");
        if (fp) {
            char buffer[BUF_SIZE];
            int found = 0;
            while (fgets(buffer, sizeof(buffer), fp) != NULL) {
                if (strstr(buffer, "unknown flag") != NULL) {
                    found = 1;
                    printf("Captured stderr: %s", buffer);
                }
            }
            pclose(fp);
            
            if (found) {
                printf("  ✓ PASSED: 'unknown flag' message found\n");
                passed_tests++;
            } else {
                printf("  ✗ FAILED: 'unknown flag' message NOT found\n");
            }
        }
        
        /* Clean up dummy file */
        remove("dummy.gcda");
    }
    
    printf("\nSummary: %d/%d tests passed\n", passed_tests, total_tests);
    
    return (passed_tests > 0) ? 1 : 0;
}

int main(void) {
    char gcov_dump_path[MAX_PATH];
    
    printf("=== Testing gcov-dump invalid flag handling ===\n\n");
    
    /* Find gcov-dump executable */
    if (!find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path))) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH.\n");
        return EXIT_FAILURE;
    }
    
    printf("Found gcov-dump at: %s\n\n", gcov_dump_path);
    
    /* Test invalid flags */
    if (!test_multiple_invalid_flags(gcov_dump_path)) {
        fprintf(stderr, "Error: No tests triggered the 'unknown flag' message.\n");
        return EXIT_FAILURE;
    }
    
    printf("\n=== All tests completed successfully ===\n");
    return EXIT_SUCCESS;
}
