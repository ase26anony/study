/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by invoking
 * gcov-dump with invalid command-line flags.
 * 
 * Compile with: gcc -std=c99 -O0 -g -o test_gcov_dump test_gcov_dump_invalid_flags.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_OUTPUT_SIZE 4096
#define ERROR_MSG_PREFIX "unknown flag"

/**
 * Find the gcov-dump executable path.
 * Priority:
 * 1. GCov_DUMP environment variable
 * 2. Common build locations
 * 3. System PATH
 */
static char *find_gcov_dump_path(void) {
    static char path[1024];
    
    // 1. Check environment variable
    char *env_path = getenv("GCov_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
        return path;
    }
    
    // 2. Check common build locations
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path, common_paths[i], sizeof(path) - 1);
            path[sizeof(path) - 1] = '\0';
            return path;
        }
    }
    
    // 3. Try to find in PATH
    char *path_env = getenv("PATH");
    if (path_env) {
        char *path_copy = strdup(path_env);
        char *dir = strtok(path_copy, ":");
        
        while (dir != NULL) {
            snprintf(path, sizeof(path), "%s/gcov-dump", dir);
            if (access(path, X_OK) == 0) {
                free(path_copy);
                return path;
            }
            dir = strtok(NULL, ":");
        }
        free(path_copy);
    }
    
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if error message found, 0 if not, -1 on execution error.
 */
static int test_invalid_flag(const char *gcov_dump_path, char *const argv[]) {
    int pipefd[2];
    pid_t pid;
    char output[MAX_OUTPUT_SIZE] = {0};
    int found_error = 0;
    
    // Create pipe for capturing stderr
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
        
        // Execute gcov-dump
        execvp(gcov_dump_path, argv);
        
        // If we get here, exec failed
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        close(pipefd[1]);  // Close write end
        
        // Read stderr output
        ssize_t bytes_read;
        size_t total_read = 0;
        
        while ((bytes_read = read(pipefd[0], output + total_read, 
                                 sizeof(output) - total_read - 1)) > 0) {
            total_read += bytes_read;
            if (total_read >= sizeof(output) - 1) {
                break;
            }
        }
        output[total_read] = '\0';
        
        close(pipefd[0]);
        
        // Wait for child
        int status;
        waitpid(pid, &status, 0);
        
        // Check if error message is in output
        if (strstr(output, ERROR_MSG_PREFIX) != NULL) {
            printf("Found error message in output:\n%s\n", output);
            found_error = 1;
        } else {
            printf("No error message found. Output was:\n%s\n", output);
        }
        
        return found_error;
    }
}

/**
 * Test various invalid flag scenarios
 */
static void run_tests(const char *gcov_dump_path) {
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test 1: Single invalid flag as first argument
    {
        printf("Test 1: Single invalid flag '-x'\n");
        char *argv[] = { "gcov-dump", "-x", NULL };
        int result = test_invalid_flag(gcov_dump_path, argv);
        total_tests++;
        if (result == 1) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else if (result == 0) {
            printf("✗ FAILED: No error message\n");
        } else {
            printf("✗ FAILED: Execution error\n");
        }
        printf("\n");
    }
    
    // Test 2: Invalid flag between valid flags
    {
        printf("Test 2: Invalid flag '-z' between valid flags '-l -z -p'\n");
        char *argv[] = { "gcov-dump", "-l", "-z", "-p", NULL };
        int result = test_invalid_flag(gcov_dump_path, argv);
        total_tests++;
        if (result == 1) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else if (result == 0) {
            printf("✗ FAILED: No error message\n");
        } else {
            printf("✗ FAILED: Execution error\n");
        }
        printf("\n");
    }
    
    // Test 3: Multiple invalid flags
    {
        printf("Test 3: Multiple invalid flags '-? -y'\n");
        char *argv[] = { "gcov-dump", "-?", "-y", NULL };
        int result = test_invalid_flag(gcov_dump_path, argv);
        total_tests++;
        if (result == 1) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else if (result == 0) {
            printf("✗ FAILED: No error message\n");
        } else {
            printf("✗ FAILED: Execution error\n");
        }
        printf("\n");
    }
    
    // Test 4: Invalid flag after filename argument
    {
        printf("Test 4: Invalid flag '-w' after filename 'test.gcda'\n");
        char *argv[] = { "gcov-dump", "test.gcda", "-w", NULL };
        int result = test_invalid_flag(gcov_dump_path, argv);
        total_tests++;
        if (result == 1) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else if (result == 0) {
            printf("✗ FAILED: No error message\n");
        } else {
            printf("✗ FAILED: Execution error\n");
        }
        printf("\n");
    }
    
    // Test 5: Double dash with invalid single char flag
    {
        printf("Test 5: Double dash with invalid flag '--x'\n");
        char *argv[] = { "gcov-dump", "--x", NULL };
        int result = test_invalid_flag(gcov_dump_path, argv);
        total_tests++;
        if (result == 1) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else if (result == 0) {
            printf("✗ FAILED: No error message\n");
        } else {
            printf("✗ FAILED: Execution error\n");
        }
        printf("\n");
    }
    
    // Test 6: Combined valid and invalid flags
    {
        printf("Test 6: Combined '-l -x -p -r -s' (x is invalid)\n");
        char *argv[] = { "gcov-dump", "-l", "-x", "-p", "-r", "-s", NULL };
        int result = test_invalid_flag(gcov_dump_path, argv);
        total_tests++;
        if (result == 1) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else if (result == 0) {
            printf("✗ FAILED: No error message\n");
        } else {
            printf("✗ FAILED: Execution error\n");
        }
        printf("\n");
    }
    
    // Summary
    printf("========================================\n");
    printf("Test Results: %d/%d tests passed\n", passed_tests, total_tests);
    printf("========================================\n");
    
    if (passed_tests == total_tests) {
        printf("SUCCESS: All tests triggered the uncovered default case!\n");
    } else if (passed_tests > 0) {
        printf("PARTIAL SUCCESS: Some tests triggered the uncovered default case.\n");
    } else {
        printf("FAILURE: No tests triggered the uncovered default case.\n");
        printf("Possible reasons:\n");
        printf("  1. gcov-dump might handle invalid flags differently\n");
        printf("  2. The executable might not be the right version\n");
        printf("  3. The error message format might be different\n");
    }
}

int main(void) {
    // Find gcov-dump executable
    char *gcov_dump_path = find_gcov_dump_path();
    if (!gcov_dump_path) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    // Verify the executable exists and is executable
    if (access(gcov_dump_path, X_OK) != 0) {
        fprintf(stderr, "Error: gcov-dump at '%s' is not executable\n", gcov_dump_path);
        return EXIT_FAILURE;
    }
    
    // Run all tests
    run_tests(gcov_dump_path);
    
    return EXIT_SUCCESS;
}
