/**
 * test_gcov_dump_flags.c
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
#define TARGET_ERROR_MSG "unknown flag"

/**
 * Find the gcov-dump executable path.
 * Priority:
 * 1. GCov_DUMP environment variable
 * 2. Common build locations
 * 3. System PATH
 */
static int find_gcov_dump(char *path, size_t path_len) {
    const char *env_path = getenv("GCov_DUMP");
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
    
    // Check environment variable first
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, path_len - 1);
        path[path_len - 1] = '\0';
        return 1;
    }
    
    // Check common build locations
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path, common_paths[i], path_len - 1);
            path[path_len - 1] = '\0';
            return 1;
        }
    }
    
    // Try to find in PATH
    const char *path_env = getenv("PATH");
    if (path_env != NULL) {
        char *path_copy = strdup(path_env);
        char *dir = strtok(path_copy, ":");
        
        while (dir != NULL) {
            char full_path[MAX_PATH_LEN];
            snprintf(full_path, sizeof(full_path), "%s/gcov-dump", dir);
            if (access(full_path, X_OK) == 0) {
                strncpy(path, full_path, path_len - 1);
                path[path_len - 1] = '\0';
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
 * Returns 1 if target error message found, 0 otherwise.
 */
static int test_gcov_dump_with_args(const char *gcov_dump_path, 
                                   const char *const args[], 
                                   int arg_count) {
    int pipefd[2];
    pid_t pid;
    char output[MAX_OUTPUT_LEN] = {0};
    int found_target = 0;
    
    // Create pipe for capturing stderr
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
        // Child process
        close(pipefd[0]);  // Close read end
        
        // Redirect stderr to pipe
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        // Build argument array
        char **argv = malloc((arg_count + 2) * sizeof(char *));
        if (!argv) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        
        argv[0] = (char *)gcov_dump_path;
        for (int i = 0; i < arg_count; i++) {
            argv[i + 1] = (char *)args[i];
        }
        argv[arg_count + 1] = NULL;
        
        // Execute gcov-dump
        execvp(gcov_dump_path, argv);
        
        // If we get here, exec failed
        perror("execvp");
        free(argv);
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        close(pipefd[1]);  // Close write end
        
        // Read stderr output
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], output, 
                                 sizeof(output) - 1)) > 0) {
            output[bytes_read] = '\0';
            
            // Check for target error message
            if (strstr(output, TARGET_ERROR_MSG) != NULL) {
                found_target = 1;
            }
            
            // Print output for debugging
            printf("Output: %s", output);
        }
        
        close(pipefd[0]);
        
        // Wait for child to finish
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("Exit status: %d\n", WEXITSTATUS(status));
        }
    }
    
    return found_target;
}

/**
 * Run a series of test cases with invalid flags.
 */
static int run_test_cases(const char *gcov_dump_path) {
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test Case 1: Single invalid flag as first argument
    {
        printf("Test 1: Single invalid flag '-x'\n");
        const char *args[] = {"-x"};
        if (test_gcov_dump_with_args(gcov_dump_path, args, 1)) {
            printf("✓ PASS: Found target error message\n");
            passed_tests++;
        } else {
            printf("✗ FAIL: Target error message not found\n");
        }
        total_tests++;
        printf("\n");
    }
    
    // Test Case 2: Invalid flag between valid flags
    {
        printf("Test 2: Invalid flag '-z' between valid flags '-l -z -p'\n");
        const char *args[] = {"-l", "-z", "-p"};
        if (test_gcov_dump_with_args(gcov_dump_path, args, 3)) {
            printf("✓ PASS: Found target error message\n");
            passed_tests++;
        } else {
            printf("✗ FAIL: Target error message not found\n");
        }
        total_tests++;
        printf("\n");
    }
    
    // Test Case 3: Multiple invalid flags
    {
        printf("Test 3: Multiple invalid flags '-? -@ -#'\n");
        const char *args[] = {"-?", "-@", "-#"};
        if (test_gcov_dump_with_args(gcov_dump_path, args, 3)) {
            printf("✓ PASS: Found target error message\n");
            passed_tests++;
        } else {
            printf("✗ FAIL: Target error message not found\n");
        }
        total_tests++;
        printf("\n");
    }
    
    // Test Case 4: Invalid flag after non-option argument
    {
        printf("Test 4: Invalid flag '-y' after filename 'test.gcda'\n");
        const char *args[] = {"test.gcda", "-y"};
        if (test_gcov_dump_with_args(gcov_dump_path, args, 2)) {
            printf("✓ PASS: Found target error message\n");
            passed_tests++;
        } else {
            printf("✗ FAIL: Target error message not found\n");
        }
        total_tests++;
        printf("\n");
    }
    
    // Test Case 5: Double dash with invalid single-character flag
    {
        printf("Test 5: Double dash with invalid flag '--x'\n");
        const char *args[] = {"--x"};
        if (test_gcov_dump_with_args(gcov_dump_path, args, 1)) {
            printf("✓ PASS: Found target error message\n");
            passed_tests++;
        } else {
            printf("✗ FAIL: Target error message not found\n");
        }
        total_tests++;
        printf("\n");
    }
    
    // Test Case 6: Combined valid and invalid flags
    {
        printf("Test 6: Combined flags '-l -x -p -r -s -v'\n");
        const char *args[] = {"-l", "-x", "-p", "-r", "-s", "-v"};
        if (test_gcov_dump_with_args(gcov_dump_path, args, 6)) {
            printf("✓ PASS: Found target error message\n");
            passed_tests++;
        } else {
            printf("✗ FAIL: Target error message not found\n");
        }
        total_tests++;
        printf("\n");
    }
    
    // Test Case 7: Invalid flag with help flag (should still trigger error)
    {
        printf("Test 7: Invalid flag '-w' with help '-h'\n");
        const char *args[] = {"-w", "-h"};
        if (test_gcov_dump_with_args(gcov_dump_path, args, 2)) {
            printf("✓ PASS: Found target error message\n");
            passed_tests++;
        } else {
            printf("✗ FAIL: Target error message not found\n");
        }
        total_tests++;
        printf("\n");
    }
    
    printf("========================================\n");
    printf("Test Results: %d/%d tests passed\n", passed_tests, total_tests);
    
    return (passed_tests > 0) ? 1 : 0;
}

int main(void) {
    char gcov_dump_path[MAX_PATH_LEN];
    
    printf("=== Testing gcov-dump Invalid Flag Handling ===\n\n");
    
    // Find gcov-dump executable
    if (!find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path))) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Found gcov-dump: %s\n\n", gcov_dump_path);
    
    // Run test cases
    if (run_test_cases(gcov_dump_path)) {
        printf("\n✓ SUCCESS: Successfully triggered uncovered default case\n");
        return EXIT_SUCCESS;
    } else {
        printf("\n✗ FAILURE: Could not trigger uncovered default case\n");
        return EXIT_FAILURE;
    }
}
