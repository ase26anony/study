/**
 * gcov-dump_invalid_flag_test.c
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

#define MAX_PATH_LEN 1024
#define MAX_OUTPUT_LEN 4096
#define ERROR_MSG_PREFIX "unknown flag"

/**
 * Find the gcov-dump executable path.
 * Checks GCov_DUMP environment variable first, then common locations.
 */
static int find_gcov_dump(char *path, size_t path_len) {
    const char *env_path = getenv("GCOV_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, path_len - 1);
        path[path_len - 1] = '\0';
        return 0;
    }

    // Common locations in GCC build trees
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
            strncpy(path, common_paths[i], path_len - 1);
            path[path_len - 1] = '\0';
            return 0;
        }
    }

    return -1;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 0 on success (found error message), -1 on execution failure.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
    int pipefd[2];
    pid_t pid;
    char output[MAX_OUTPUT_LEN] = {0};
    int status;
    
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    if (pid == 0) {
        // Child process
        close(pipefd[0]);  // Close read end
        
        // Redirect stderr to pipe
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        // Prepare arguments
        char *args[] = {
            (char *)gcov_dump_path,
            (char *)flag,
            NULL
        };
        
        execvp(gcov_dump_path, args);
        
        // execvp only returns on error
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        close(pipefd[1]);  // Close write end
        
        // Read stderr output
        ssize_t bytes_read = read(pipefd[0], output, MAX_OUTPUT_LEN - 1);
        close(pipefd[0]);
        
        waitpid(pid, &status, 0);
        
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
            
            // Check if error message is in output
            if (strstr(output, ERROR_MSG_PREFIX) != NULL) {
                printf("SUCCESS: Found error message for flag '%s':\n%s\n", flag, output);
                return 0;
            } else {
                printf("FAILURE: No error message for flag '%s'. Output:\n%s\n", flag, output);
                return -1;
            }
        } else {
            printf("FAILURE: No output for flag '%s'\n", flag);
            return -1;
        }
    }
}

/**
 * Test multiple invalid flags in different positions.
 */
static int test_flag_combinations(const char *gcov_dump_path) {
    int success_count = 0;
    int total_tests = 0;
    
    // Test single invalid flags
    const char *invalid_flags[] = {
        "-x",  // Simple invalid flag
        "-z",  // Another invalid flag
        "-?",  // Question mark (not in switch)
        "-X",  // Uppercase (not in switch)
        "-0",  // Number (not in switch)
        NULL
    };
    
    printf("\n=== Testing single invalid flags ===\n");
    for (int i = 0; invalid_flags[i] != NULL; i++) {
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, invalid_flags[i]) == 0) {
            success_count++;
        }
    }
    
    // Test invalid flag in different positions
    printf("\n=== Testing flag positions ===\n");
    
    // Test 1: Invalid flag between valid flags
    total_tests++;
    printf("\nTest: -l -x -p (invalid in middle)\n");
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        char *args[] = {
            (char *)gcov_dump_path,
            "-l", "-x", "-p",
            NULL
        };
        
        execvp(gcov_dump_path, args);
        exit(EXIT_FAILURE);
    } else {
        close(pipefd[1]);
        char output[MAX_OUTPUT_LEN] = {0};
        ssize_t bytes_read = read(pipefd[0], output, MAX_OUTPUT_LEN - 1);
        close(pipefd[0]);
        
        waitpid(pid, NULL, 0);
        
        if (bytes_read > 0 && strstr(output, ERROR_MSG_PREFIX) != NULL) {
            printf("SUCCESS: Found error message for -l -x -p\n");
            success_count++;
        } else {
            printf("FAILURE: No error message for -l -x -p\n");
        }
    }
    
    // Test 2: Invalid flag after non-option argument
    total_tests++;
    printf("\nTest: dummy.gcda -x (invalid after filename)\n");
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }
    
    pid = fork();
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        char *args[] = {
            (char *)gcov_dump_path,
            "dummy.gcda", "-x",
            NULL
        };
        
        execvp(gcov_dump_path, args);
        exit(EXIT_FAILURE);
    } else {
        close(pipefd[1]);
        char output[MAX_OUTPUT_LEN] = {0};
        ssize_t bytes_read = read(pipefd[0], output, MAX_OUTPUT_LEN - 1);
        close(pipefd[0]);
        
        waitpid(pid, NULL, 0);
        
        if (bytes_read > 0 && strstr(output, ERROR_MSG_PREFIX) != NULL) {
            printf("SUCCESS: Found error message for dummy.gcda -x\n");
            success_count++;
        } else {
            printf("FAILURE: No error message for dummy.gcda -x\n");
        }
    }
    
    // Test 3: Double dash with invalid flag (--x)
    total_tests++;
    printf("\nTest: --x (double dash with invalid flag)\n");
    if (test_invalid_flag(gcov_dump_path, "--x") == 0) {
        success_count++;
    }
    
    printf("\n=== Summary ===\n");
    printf("Passed: %d/%d tests\n", success_count, total_tests);
    
    return (success_count == total_tests) ? 0 : 1;
}

int main(void) {
    char gcov_dump_path[MAX_PATH_LEN];
    
    printf("=== gcov-dump Invalid Flag Test ===\n");
    
    // Find gcov-dump executable
    if (find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path)) != 0) {
        fprintf(stderr, "ERROR: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n", gcov_dump_path);
    
    // Run tests
    int result = test_flag_combinations(gcov_dump_path);
    
    if (result == 0) {
        printf("\nAll tests passed! The uncovered default case was triggered.\n");
    } else {
        printf("\nSome tests failed. Not all uncovered lines were triggered.\n");
    }
    
    return result;
}
