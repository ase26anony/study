/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by invoking it with
 * invalid command-line flags and verifying the error message.
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
 * Priority: 1. GCov_DUMP environment variable
 *           2. Common build locations
 *           3. System PATH
 */
static int find_gcov_dump(char *path, size_t path_len) {
    const char *env_path = getenv("GCov_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, path_len - 1);
        path[path_len - 1] = '\0';
        return 0;
    }

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

    // Try to find in PATH
    const char *path_env = getenv("PATH");
    if (path_env) {
        char *path_copy = strdup(path_env);
        char *dir = strtok(path_copy, ":");
        while (dir) {
            snprintf(path, path_len, "%s/gcov-dump", dir);
            if (access(path, X_OK) == 0) {
                free(path_copy);
                return 0;
            }
            dir = strtok(NULL, ":");
        }
        free(path_copy);
    }

    return -1;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 0 on success (found target error), -1 on execution failure.
 */
static int test_gcov_dump(const char *gcov_dump_path, char *const argv[], 
                         int *found_error) {
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
        dup2(pipefd[1], STDERR_FILENO);  // Redirect stderr to pipe
        close(pipefd[1]);

        execvp(gcov_dump_path, argv);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {  // Parent process
        close(pipefd[1]);  // Close write end
        
        char buffer[MAX_OUTPUT_LEN];
        ssize_t bytes_read;
        *found_error = 0;
        
        // Read stderr output
        while ((bytes_read = read(pipefd[0], buffer, 
                                 sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            if (strstr(buffer, TARGET_ERROR_MSG) != NULL) {
                *found_error = 1;
            }
            // Print output for debugging
            fwrite(buffer, 1, bytes_read, stderr);
        }
        
        close(pipefd[0]);
        
        int status;
        waitpid(pid, &status, 0);
        
        return 0;
    }
}

/**
 * Test case structure
 */
typedef struct {
    const char *description;
    char *argv[10];  // Argument vector (NULL terminated)
} test_case_t;

int main(void) {
    char gcov_dump_path[MAX_PATH_LEN];
    
    // Find gcov-dump executable
    if (find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path)) != 0) {
        fprintf(stderr, "Error: gcov-dump executable not found\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Define test cases targeting different uncovered scenarios
    test_case_t test_cases[] = {
        // Single invalid flag at start
        {"Single invalid flag '-x' at start", 
         {"gcov-dump", "-x", NULL}},
        
        // Single invalid flag '-z'
        {"Single invalid flag '-z'", 
         {"gcov-dump", "-z", NULL}},
        
        // Invalid flag '?' (help-like but not handled)
        {"Invalid flag '-?'", 
         {"gcov-dump", "-?", NULL}},
        
        // Invalid flag between valid flags
        {"Invalid flag '-x' between valid flags '-l' and '-p'", 
         {"gcov-dump", "-l", "-x", "-p", NULL}},
        
        // Multiple invalid flags
        {"Multiple invalid flags '-x -y -z'", 
         {"gcov-dump", "-x", "-y", "-z", NULL}},
        
        // Invalid flag after filename argument
        {"Invalid flag '-x' after filename", 
         {"gcov-dump", "test.gcda", "-x", NULL}},
        
        // Double dash with invalid single char (getopt may treat differently)
        {"Double dash with invalid flag '--x'", 
         {"gcov-dump", "--x", NULL}},
        
        // Combination: valid, invalid, valid
        {"Combination '-l -x -p -r'", 
         {"gcov-dump", "-l", "-x", "-p", "-r", NULL}},
        
        // Boundary: invalid flag as last argument
        {"Invalid flag as last argument after valid flags", 
         {"gcov-dump", "-l", "-p", "-x", NULL}},
        
        // Edge case: dash only (should trigger unknown flag for empty char?)
        {"Single dash '-'", 
         {"gcov-dump", "-", NULL}},
    };
    
    int total_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int passed_tests = 0;
    int failed_tests = 0;
    
    // Execute all test cases
    for (int i = 0; i < total_tests; i++) {
        printf("Test %d: %s\n", i + 1, test_cases[i].description);
        printf("Command: %s", gcov_dump_path);
        
        for (int j = 0; test_cases[i].argv[j] != NULL; j++) {
            printf(" %s", test_cases[i].argv[j]);
        }
        printf("\n");
        
        int found_error = 0;
        if (test_gcov_dump(gcov_dump_path, test_cases[i].argv, &found_error) == 0) {
            if (found_error) {
                printf("✓ PASS: Found '%s' error message\n\n", TARGET_ERROR_MSG);
                passed_tests++;
            } else {
                printf("✗ FAIL: Did not find '%s' error message\n\n", TARGET_ERROR_MSG);
                failed_tests++;
            }
        } else {
            printf("✗ ERROR: Failed to execute test\n\n");
            failed_tests++;
        }
        
        // Small delay to avoid overwhelming the system
        usleep(10000);
    }
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", failed_tests);
    
    if (passed_tests > 0) {
        printf("\nSuccessfully triggered the uncovered default case in gcov-dump.cc!\n");
        return EXIT_SUCCESS;
    } else {
        printf("\nFailed to trigger the uncovered default case.\n");
        return EXIT_FAILURE;
    }
}
