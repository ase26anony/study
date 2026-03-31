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
#define ERROR_MSG_PREFIX "unknown flag"

/**
 * Find the gcov-dump executable path.
 * Priority: 1. GCov_DUMP environment variable
 *           2. Common build locations
 *           3. System PATH
 */
static char *find_gcov_dump_path(void) {
    static char path[MAX_PATH_LEN];
    
    // 1. Check environment variable
    char *env_path = getenv("GCov_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, MAX_PATH_LEN - 1);
        path[MAX_PATH_LEN - 1] = '\0';
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
            strncpy(path, common_paths[i], MAX_PATH_LEN - 1);
            path[MAX_PATH_LEN - 1] = '\0';
            return path;
        }
    }
    
    // 3. Try to find in PATH
    char *path_env = getenv("PATH");
    if (path_env) {
        char *path_copy = strdup(path_env);
        char *dir = strtok(path_copy, ":");
        
        while (dir != NULL) {
            snprintf(path, MAX_PATH_LEN, "%s/gcov-dump", dir);
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
static int test_gcov_dump_with_args(const char *gcov_dump_path, 
                                   char *const argv[]) {
    int pipefd[2];
    pid_t pid;
    int status;
    char output[MAX_OUTPUT_LEN] = {0};
    int found_error = 0;
    
    // Create pipe for stderr
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
        fprintf(stderr, "Failed to execute %s: %s\n", 
                gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else {  // Parent process
        // Close write end of pipe
        close(pipefd[1]);
        
        // Read stderr output
        ssize_t bytes_read = read(pipefd[0], output, MAX_OUTPUT_LEN - 1);
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
            
            // Check for error message
            if (strstr(output, ERROR_MSG_PREFIX) != NULL) {
                printf("Found error message in output:\n%s\n", output);
                found_error = 1;
            } else {
                printf("No error message found. Output:\n%s\n", output);
            }
        }
        
        close(pipefd[0]);
        
        // Wait for child to finish
        waitpid(pid, &status, 0);
        
        // Check exit status (should be non-zero for invalid flag)
        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            printf("Exit status: %d\n", exit_status);
        }
        
        return found_error;
    }
}

/**
 * Test cases for invalid flags
 */
typedef struct {
    char *description;
    char *args[10];  // NULL terminated
} test_case_t;

int main(void) {
    char *gcov_dump_path = find_gcov_dump_path();
    if (!gcov_dump_path) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH.\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Define test cases for invalid flags
    test_case_t test_cases[] = {
        // Single invalid flag in different positions
        {
            "Single invalid flag '-x'",
            {gcov_dump_path, "-x", NULL}
        },
        {
            "Single invalid flag '-z'",
            {gcov_dump_path, "-z", NULL}
        },
        {
            "Single invalid flag '-?'",
            {gcov_dump_path, "-?", NULL}
        },
        
        // Invalid flag between valid flags
        {
            "Invalid flag '-x' between valid flags '-l' and '-p'",
            {gcov_dump_path, "-l", "-x", "-p", NULL}
        },
        {
            "Multiple invalid flags '-x -z'",
            {gcov_dump_path, "-x", "-z", NULL}
        },
        
        // Invalid flag after filename argument
        {
            "Invalid flag '-x' after filename",
            {gcov_dump_path, "test.gcda", "-x", NULL}
        },
        
        // Double dash with invalid single char flag
        {
            "Double dash with invalid flag '--x'",
            {gcov_dump_path, "--x", NULL}
        },
        
        // Combined valid and invalid flags
        {
            "Combination '-l -x -p -r -s'",
            {gcov_dump_path, "-l", "-x", "-p", "-r", "-s", NULL}
        },
        
        // Edge case: just dash
        {
            "Just a dash '-'",
            {gcov_dump_path, "-", NULL}
        },
        
        // End marker
        {NULL, {NULL}}
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    int failed_tests = 0;
    
    // Run all test cases
    for (int i = 0; test_cases[i].description != NULL; i++) {
        printf("Test %d: %s\n", total_tests + 1, test_cases[i].description);
        printf("Command: %s", gcov_dump_path);
        
        for (int j = 1; test_cases[i].args[j] != NULL; j++) {
            printf(" %s", test_cases[i].args[j]);
        }
        printf("\n");
        
        int result = test_gcov_dump_with_args(gcov_dump_path, 
                                             test_cases[i].args);
        
        if (result == 1) {
            printf("✓ PASS: Error message found\n\n");
            passed_tests++;
        } else if (result == 0) {
            printf("✗ FAIL: Error message not found\n\n");
            failed_tests++;
        } else {
            printf("✗ ERROR: Execution failed\n\n");
            failed_tests++;
        }
        
        total_tests++;
    }
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", failed_tests);
    
    if (failed_tests > 0) {
        printf("\nSome tests failed. This could mean:\n");
        printf("1. gcov-dump handles invalid flags differently\n");
        printf("2. The executable is a different version\n");
        printf("3. The error message format has changed\n");
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
