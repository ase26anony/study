/**
 * gcov-dump_invalid_flag_test.c
 * 
 * Tests the uncovered default case in gcov-dump.cc (lines 111-130)
 * by invoking gcov-dump with invalid command-line flags.
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
 * 2. Common build tree locations
 * 3. System PATH
 */
static int find_gcov_dump(char *path, size_t path_len) {
    const char *env_path = getenv("GCov_DUMP");
    const char *candidate_paths[] = {
        env_path,
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "gcov-dump",  // Try PATH
        NULL
    };
    
    for (int i = 0; candidate_paths[i] != NULL; i++) {
        if (candidate_paths[i] == NULL) continue;
        
        if (access(candidate_paths[i], X_OK) == 0) {
            strncpy(path, candidate_paths[i], path_len - 1);
            path[path_len - 1] = '\0';
            return 0;
        }
    }
    
    return -1;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 0 on success (found target error), -1 on execution failure.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char **args, 
                             int arg_count, int *found_error) {
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
    
    if (pid == 0) {  // Child process
        close(pipefd[0]);  // Close read end
        
        // Redirect stderr to pipe
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        // Prepare arguments
        char **exec_args = malloc((arg_count + 2) * sizeof(char *));
        if (!exec_args) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        
        exec_args[0] = (char *)gcov_dump_path;
        for (int i = 0; i < arg_count; i++) {
            exec_args[i + 1] = (char *)args[i];
        }
        exec_args[arg_count + 1] = NULL;
        
        execvp(gcov_dump_path, exec_args);
        
        // If we get here, exec failed
        perror("execvp");
        free(exec_args);
        exit(EXIT_FAILURE);
    }
    
    // Parent process
    close(pipefd[1]);  // Close write end
    
    // Read stderr output
    ssize_t bytes_read = read(pipefd[0], output, MAX_OUTPUT_LEN - 1);
    close(pipefd[0]);
    
    // Wait for child
    waitpid(pid, &status, 0);
    
    if (bytes_read > 0) {
        output[bytes_read] = '\0';
        
        // Check for target error message
        if (strstr(output, TARGET_ERROR_MSG) != NULL) {
            *found_error = 1;
            printf("Found target error in output:\n%s\n", output);
        } else {
            *found_error = 0;
            printf("Output (no target error):\n%s\n", output);
        }
    } else {
        *found_error = 0;
        printf("No output captured\n");
    }
    
    return 0;
}

/**
 * Test various invalid flag scenarios
 */
static int run_tests(const char *gcov_dump_path) {
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test 1: Single invalid flag as first argument
    {
        printf("Test 1: Single invalid flag '-x'\n");
        const char *args[] = {"-x"};
        int found_error = 0;
        
        if (test_invalid_flag(gcov_dump_path, args, 1, &found_error) == 0) {
            total_tests++;
            if (found_error) {
                printf("✓ PASS: Triggered unknown flag error\n\n");
                passed_tests++;
            } else {
                printf("✗ FAIL: Did not trigger unknown flag error\n\n");
            }
        }
    }
    
    // Test 2: Invalid flag between valid flags
    {
        printf("Test 2: Invalid flag '-z' between valid flags '-l -z -p'\n");
        const char *args[] = {"-l", "-z", "-p"};
        int found_error = 0;
        
        if (test_invalid_flag(gcov_dump_path, args, 3, &found_error) == 0) {
            total_tests++;
            if (found_error) {
                printf("✓ PASS: Triggered unknown flag error\n\n");
                passed_tests++;
            } else {
                printf("✗ FAIL: Did not trigger unknown flag error\n\n");
            }
        }
    }
    
    // Test 3: Multiple invalid flags
    {
        printf("Test 3: Multiple invalid flags '-? -y'\n");
        const char *args[] = {"-?", "-y"};
        int found_error = 0;
        
        if (test_invalid_flag(gcov_dump_path, args, 2, &found_error) == 0) {
            total_tests++;
            if (found_error) {
                printf("✓ PASS: Triggered unknown flag error\n\n");
                passed_tests++;
            } else {
                printf("✗ FAIL: Did not trigger unknown flag error\n\n");
            }
        }
    }
    
    // Test 4: Invalid flag after non-option argument (test file)
    {
        printf("Test 4: Invalid flag '-a' after filename 'test.gcda'\n");
        const char *args[] = {"test.gcda", "-a"};
        int found_error = 0;
        
        if (test_invalid_flag(gcov_dump_path, args, 2, &found_error) == 0) {
            total_tests++;
            if (found_error) {
                printf("✓ PASS: Triggered unknown flag error\n\n");
                passed_tests++;
            } else {
                printf("✗ FAIL: Did not trigger unknown flag error\n\n");
            }
        }
    }
    
    // Test 5: Double dash with invalid single-character flag
    {
        printf("Test 5: Double dash with invalid flag '--x'\n");
        const char *args[] = {"--x"};
        int found_error = 0;
        
        if (test_invalid_flag(gcov_dump_path, args, 1, &found_error) == 0) {
            total_tests++;
            if (found_error) {
                printf("✓ PASS: Triggered unknown flag error\n\n");
                passed_tests++;
            } else {
                printf("✗ FAIL: Did not trigger unknown flag error\n\n");
            }
        }
    }
    
    // Test 6: Combination with all valid flags plus one invalid
    {
        printf("Test 6: All valid flags plus invalid '-w': '-l -p -r -s -w'\n");
        const char *args[] = {"-l", "-p", "-r", "-s", "-w"};
        int found_error = 0;
        
        if (test_invalid_flag(gcov_dump_path, args, 5, &found_error) == 0) {
            total_tests++;
            if (found_error) {
                printf("✓ PASS: Triggered unknown flag error\n\n");
                passed_tests++;
            } else {
                printf("✗ FAIL: Did not trigger unknown flag error\n\n");
            }
        }
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", total_tests - passed_tests);
    
    return (passed_tests == total_tests && total_tests > 0) ? 0 : 1;
}

int main(void) {
    char gcov_dump_path[MAX_PATH_LEN];
    
    // Find gcov-dump executable
    if (find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path)) != 0) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Found gcov-dump: %s\n", gcov_dump_path);
    
    // Run all tests
    int result = run_tests(gcov_dump_path);
    
    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
