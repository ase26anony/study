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
 * 2. Common build tree locations
 * 3. System PATH
 */
static int find_gcov_dump(char *path, size_t path_len) {
    const char *env_path = getenv("GCov_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, path_len - 1);
        path[path_len - 1] = '\0';
        return 0;
    }
    
    // Common build tree locations
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "../../prev-gcc/build/gcc/gcov-dump",
        "gcov-dump",  // Try PATH
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
 * Returns 0 if target error message found, -1 otherwise.
 */
static int test_gcov_dump_args(const char *gcov_dump_path, 
                               char *const argv[], 
                               char *output, 
                               size_t output_len) {
    int pipefd[2];
    pid_t pid;
    
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
        
        // Execute gcov-dump
        execvp(gcov_dump_path, argv);
        
        // If we get here, exec failed
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    // Parent process
    close(pipefd[1]);  // Close write end
    
    // Read stderr output
    ssize_t bytes_read = read(pipefd[0], output, output_len - 1);
    if (bytes_read > 0) {
        output[bytes_read] = '\0';
    } else {
        output[0] = '\0';
    }
    
    close(pipefd[0]);
    
    // Wait for child
    int status;
    waitpid(pid, &status, 0);
    
    // Check if target error message is in output
    if (strstr(output, TARGET_ERROR_MSG) != NULL) {
        return 0;  // Success - found target error
    }
    
    return -1;  // Target error not found
}

/**
 * Test case structure
 */
typedef struct {
    char *description;
    char *args[10];  // NULL terminated
} test_case_t;

int main(void) {
    char gcov_dump_path[MAX_PATH_LEN];
    char output[MAX_OUTPUT_LEN];
    int tests_passed = 0;
    int total_tests = 0;
    
    // Find gcov-dump executable
    if (find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path)) != 0) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Define test cases targeting different uncovered scenarios
    test_case_t test_cases[] = {
        // Single invalid flag tests
        {"Single invalid flag -x", {"gcov-dump", "-x", NULL}},
        {"Single invalid flag -z", {"gcov-dump", "-z", NULL}},
        {"Single invalid flag -?", {"gcov-dump", "-?", NULL}},
        
        // Invalid flag in different positions
        {"Invalid flag first: -x -l", {"gcov-dump", "-x", "-l", NULL}},
        {"Valid then invalid: -l -x", {"gcov-dump", "-l", "-x", NULL}},
        {"Multiple valid with invalid middle: -l -x -p", {"gcov-dump", "-l", "-x", "-p", NULL}},
        {"Multiple invalid flags: -x -z -?", {"gcov-dump", "-x", "-z", "-?", NULL}},
        
        // With filename argument
        {"Invalid flag before filename: -x dummy.gcda", {"gcov-dump", "-x", "dummy.gcda", NULL}},
        {"Filename then invalid flag: dummy.gcda -x", {"gcov-dump", "dummy.gcda", "-x", NULL}},
        {"Valid, invalid, filename: -l -x dummy.gcda", {"gcov-dump", "-l", "-x", "dummy.gcda", NULL}},
        
        // Double dash edge cases
        {"Double dash with invalid: -- -x", {"gcov-dump", "--", "-x", NULL}},
        {"Double dash with multiple: -- -x -z", {"gcov-dump", "--", "-x", "-z", NULL}},
        
        // Combined valid and invalid
        {"All valid with one invalid: -l -p -r -s -x", {"gcov-dump", "-l", "-p", "-r", "-s", "-x", NULL}},
        
        // End marker
        {NULL, {NULL}}
    };
    
    // Run all test cases
    for (int i = 0; test_cases[i].description != NULL; i++) {
        total_tests++;
        printf("Test %d: %s\n", total_tests, test_cases[i].description);
        
        if (test_gcov_dump_args(gcov_dump_path, test_cases[i].args, output, sizeof(output)) == 0) {
            printf("  ✓ PASS - Found '%s' in output\n", TARGET_ERROR_MSG);
            tests_passed++;
            
            // Print first line of error for verification
            char *first_line = strtok(output, "\n");
            if (first_line) {
                printf("    Output: %s\n", first_line);
            }
        } else {
            printf("  ✗ FAIL - Target error not found\n");
            if (strlen(output) > 0) {
                printf("    Output: %s\n", output);
            } else {
                printf("    (No output)\n");
            }
        }
        printf("\n");
    }
    
    // Summary
    printf("========================================\n");
    printf("Test Summary:\n");
    printf("  Total tests: %d\n", total_tests);
    printf("  Passed: %d\n", tests_passed);
    printf("  Failed: %d\n", total_tests - tests_passed);
    
    if (tests_passed > 0) {
        printf("\nSuccessfully triggered the uncovered default case!\n");
        return EXIT_SUCCESS;
    } else {
        printf("\nFailed to trigger the uncovered default case.\n");
        return EXIT_FAILURE;
    }
}
