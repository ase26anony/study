/**
 * Test program to trigger uncovered lines in gcov-dump.cc
 * Specifically targets the default case in the flag parsing switch statement
 * Compile with: gcc -std=c99 -O0 -g -o test_gcov_dump test_gcov_dump.c
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
char *find_gcov_dump_path() {
    char *path = NULL;
    
    // 1. Check environment variable
    char *env_path = getenv("GCOV_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        path = strdup(env_path);
        printf("Using gcov-dump from environment: %s\n", path);
        return path;
    }
    
    // 2. Check common build locations
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
            path = strdup(common_paths[i]);
            printf("Found gcov-dump at: %s\n", path);
            return path;
        }
    }
    
    // 3. Try to find in PATH
    char *path_env = getenv("PATH");
    if (path_env != NULL) {
        char *path_copy = strdup(path_env);
        char *dir = strtok(path_copy, ":");
        
        while (dir != NULL) {
            char full_path[MAX_PATH_LEN];
            snprintf(full_path, sizeof(full_path), "%s/gcov-dump", dir);
            
            if (access(full_path, X_OK) == 0) {
                path = strdup(full_path);
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
int test_gcov_dump(const char *gcov_dump_path, char *const argv[]) {
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
    
    if (pid == 0) {  // Child process
        // Close read end of pipe
        close(pipefd[0]);
        
        // Redirect stderr to pipe
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        // Execute gcov-dump
        execvp(gcov_dump_path, argv);
        
        // If we get here, exec failed
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {  // Parent process
        // Close write end of pipe
        close(pipefd[1]);
        
        // Read from pipe
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], output, sizeof(output) - 1)) > 0) {
            output[bytes_read] = '\0';
            
            // Check for target error message
            if (strstr(output, TARGET_ERROR_MSG) != NULL) {
                found_target = 1;
            }
            
            // Print output for debugging
            printf("Output from gcov-dump:\n%s\n", output);
        }
        
        close(pipefd[0]);
        
        // Wait for child to finish
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("gcov-dump exited with status: %d\n", WEXITSTATUS(status));
        }
    }
    
    return found_target;
}

/**
 * Test a specific invalid flag scenario
 */
int test_scenario(const char *gcov_dump_path, const char *scenario_name, 
                  char *const args[], int arg_count) {
    printf("\n=== Testing scenario: %s ===\n", scenario_name);
    
    // Build argument array (including NULL terminator)
    char **argv = malloc((arg_count + 2) * sizeof(char *));
    argv[0] = (char *)gcov_dump_path;
    
    for (int i = 0; i < arg_count; i++) {
        argv[i + 1] = args[i];
    }
    argv[arg_count + 1] = NULL;
    
    // Print command being executed
    printf("Command: ");
    for (int i = 0; argv[i] != NULL; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    
    int result = test_gcov_dump(gcov_dump_path, argv);
    
    free(argv);
    return result;
}

int main() {
    printf("=== Testing uncovered lines in gcov-dump.cc ===\n");
    
    // Find gcov-dump executable
    char *gcov_dump_path = find_gcov_dump_path();
    if (gcov_dump_path == NULL) {
        return EXIT_FAILURE;
    }
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Test 1: Single invalid flag at beginning
    {
        total_tests++;
        char *args[] = {"-x"};
        if (test_scenario(gcov_dump_path, "Single invalid flag '-x'", args, 1) == 1) {
            printf("✓ PASS: Invalid flag '-x' triggered error\n");
            passed_tests++;
        } else {
            printf("✗ FAIL: Invalid flag '-x' did not trigger expected error\n");
        }
    }
    
    // Test 2: Invalid flag between valid flags
    {
        total_tests++;
        char *args[] = {"-l", "-x", "-p"};
        if (test_scenario(gcov_dump_path, "Invalid flag '-x' between valid flags", args, 3) == 1) {
            printf("✓ PASS: Invalid flag between valid flags triggered error\n");
            passed_tests++;
        } else {
            printf("✗ FAIL: Invalid flag between valid flags did not trigger error\n");
        }
    }
    
    // Test 3: Multiple invalid flags
    {
        total_tests++;
        char *args[] = {"-z", "-?", "-@"};
        if (test_scenario(gcov_dump_path, "Multiple invalid flags", args, 3) == 1) {
            printf("✓ PASS: Multiple invalid flags triggered error\n");
            passed_tests++;
        } else {
            printf("✗ FAIL: Multiple invalid flags did not trigger error\n");
        }
    }
    
    // Test 4: Invalid flag after filename argument
    {
        total_tests++;
        char *args[] = {"test.gcda", "-y"};
        if (test_scenario(gcov_dump_path, "Invalid flag after filename", args, 2) == 1) {
            printf("✓ PASS: Invalid flag after filename triggered error\n");
            passed_tests++;
        } else {
            printf("✗ FAIL: Invalid flag after filename did not trigger error\n");
        }
    }
    
    // Test 5: Double dash with invalid single-character flag
    {
        total_tests++;
        char *args[] = {"--", "-x"};
        if (test_scenario(gcov_dump_path, "Double dash with invalid flag", args, 2) == 1) {
            printf("✓ PASS: Double dash with invalid flag triggered error\n");
            passed_tests++;
        } else {
            printf("✗ FAIL: Double dash with invalid flag did not trigger error\n");
        }
    }
    
    // Test 6: Combined valid and invalid flags in one argument
    {
        total_tests++;
        char *args[] = {"-lpz"};
        if (test_scenario(gcov_dump_path, "Combined flags with invalid 'z'", args, 1) == 1) {
            printf("✓ PASS: Combined flags with invalid character triggered error\n");
            passed_tests++;
        } else {
            printf("✗ FAIL: Combined flags did not trigger error\n");
        }
    }
    
    // Test 7: Edge case - uppercase invalid flag
    {
        total_tests++;
        char *args[] = {"-X"};
        if (test_scenario(gcov_dump_path, "Uppercase invalid flag '-X'", args, 1) == 1) {
            printf("✓ PASS: Uppercase invalid flag triggered error\n");
            passed_tests++;
        } else {
            printf("✗ FAIL: Uppercase invalid flag did not trigger error\n");
        }
    }
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", total_tests - passed_tests);
    
    free(gcov_dump_path);
    
    if (passed_tests == total_tests) {
        printf("\n✓ All tests passed! Uncovered lines should now be executed.\n");
        return EXIT_SUCCESS;
    } else {
        printf("\n✗ Some tests failed. Not all uncovered lines were triggered.\n");
        return EXIT_FAILURE;
    }
}
