/**
 * Test program to trigger uncovered lines in gcov-dump.cc
 * Specifically targets the default case in the flag parsing switch statement
 * that prints "unknown flag `%c'\n"
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
 * Find the gcov-dump executable path
 * Returns: dynamically allocated string with path, or NULL if not found
 */
char* find_gcov_dump_path() {
    char* path = NULL;
    
    // 1. Check environment variable
    char* env_path = getenv("GCOV_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        path = strdup(env_path);
        printf("Using gcov-dump from GCOV_DUMP env: %s\n", path);
        return path;
    }
    
    // 2. Check common build locations
    const char* common_paths[] = {
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
            path = strdup(common_paths[i]);
            printf("Found gcov-dump at: %s\n", path);
            return path;
        }
    }
    
    // 3. Try to find in PATH
    char* path_env = getenv("PATH");
    if (path_env != NULL) {
        char* path_copy = strdup(path_env);
        char* dir = strtok(path_copy, ":");
        
        while (dir != NULL) {
            char test_path[MAX_PATH_LEN];
            snprintf(test_path, sizeof(test_path), "%s/gcov-dump", dir);
            
            if (access(test_path, X_OK) == 0) {
                path = strdup(test_path);
                free(path_copy);
                printf("Found gcov-dump in PATH: %s\n", path);
                return path;
            }
            
            dir = strtok(NULL, ":");
        }
        
        free(path_copy);
    }
    
    fprintf(stderr, "Error: gcov-dump executable not found\n");
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr
 * Returns: 1 if target error message found, 0 if not, -1 on execution error
 */
int test_gcov_dump_with_args(const char* gcov_dump_path, char* const argv[]) {
    int pipefd[2];
    pid_t pid;
    char output[MAX_OUTPUT_LEN] = {0};
    int found_target = 0;
    
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
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        close(pipefd[1]);  // Close write end
        
        // Read stderr output from pipe
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], output, sizeof(output) - 1)) > 0) {
            output[bytes_read] = '\0';
        }
        close(pipefd[0]);
        
        // Wait for child to finish
        int status;
        waitpid(pid, &status, 0);
        
        // Check if target error message is in output
        if (strstr(output, TARGET_ERROR_MSG) != NULL) {
            found_target = 1;
            printf("SUCCESS: Found target error message in output:\n");
            printf("%s", output);
        } else {
            printf("Output did not contain target message. Output was:\n%s", output);
        }
        
        return found_target;
    }
}

/**
 * Run a specific test case
 */
int run_test_case(const char* gcov_dump_path, const char* description, 
                  char* const args[], int arg_count) {
    printf("\n=== Test Case: %s ===\n", description);
    printf("Command: %s", gcov_dump_path);
    for (int i = 0; i < arg_count; i++) {
        printf(" %s", args[i]);
    }
    printf("\n");
    
    return test_gcov_dump_with_args(gcov_dump_path, (char* const*)args);
}

int main() {
    char* gcov_dump_path = find_gcov_dump_path();
    if (gcov_dump_path == NULL) {
        return EXIT_FAILURE;
    }
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Test Case 1: Single invalid flag as first argument
    {
        char* args[] = { "gcov-dump", "-x", NULL };
        total_tests++;
        if (run_test_case(gcov_dump_path, "Single invalid flag '-x'", args, 2) > 0) {
            passed_tests++;
        }
    }
    
    // Test Case 2: Invalid flag between valid flags
    {
        char* args[] = { "gcov-dump", "-l", "-z", "-p", NULL };
        total_tests++;
        if (run_test_case(gcov_dump_path, "Invalid '-z' between valid '-l -p'", args, 4) > 0) {
            passed_tests++;
        }
    }
    
    // Test Case 3: Multiple invalid flags
    {
        char* args[] = { "gcov-dump", "-?", "-y", "-z", NULL };
        total_tests++;
        if (run_test_case(gcov_dump_path, "Multiple invalid flags '-? -y -z'", args, 4) > 0) {
            passed_tests++;
        }
    }
    
    // Test Case 4: Invalid flag after filename argument
    {
        char* args[] = { "gcov-dump", "test.gcda", "-w", NULL };
        total_tests++;
        if (run_test_case(gcov_dump_path, "Invalid '-w' after filename", args, 3) > 0) {
            passed_tests++;
        }
    }
    
    // Test Case 5: Double dash with invalid single character
    {
        char* args[] = { "gcov-dump", "--x", NULL };
        total_tests++;
        if (run_test_case(gcov_dump_path, "Double dash with '--x'", args, 2) > 0) {
            passed_tests++;
        }
    }
    
    // Test Case 6: Combined valid and invalid in single argument
    {
        char* args[] = { "gcov-dump", "-lpz", NULL };
        total_tests++;
        if (run_test_case(gcov_dump_path, "Combined '-lpz' (z is invalid)", args, 2) > 0) {
            passed_tests++;
        }
    }
    
    // Test Case 7: Invalid flag with argument (testing getopt behavior)
    {
        char* args[] = { "gcov-dump", "-k", "dummy", NULL };
        total_tests++;
        if (run_test_case(gcov_dump_path, "Invalid '-k' with argument", args, 3) > 0) {
            passed_tests++;
        }
    }
    
    // Test Case 8: Just invalid flag with no other arguments
    {
        char* args[] = { "gcov-dump", "-a", NULL };
        total_tests++;
        if (run_test_case(gcov_dump_path, "Only invalid flag '-a'", args, 2) > 0) {
            passed_tests++;
        }
    }
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests run: %d\n", total_tests);
    printf("Tests passed: %d\n", passed_tests);
    printf("Tests failed: %d\n", total_tests - passed_tests);
    
    free(gcov_dump_path);
    
    if (passed_tests > 0) {
        printf("\nSUCCESS: Triggered the uncovered default case in gcov-dump!\n");
        return EXIT_SUCCESS;
    } else {
        printf("\nFAILURE: Could not trigger the target error message.\n");
        return EXIT_FAILURE;
    }
}
