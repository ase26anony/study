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

#define MAX_OUTPUT_SIZE 4096
#define ERROR_MSG_PREFIX "unknown flag"

/**
 * Find the gcov-dump executable path
 * Returns: dynamically allocated string with path, or NULL if not found
 */
static char *find_gcov_dump(void)
{
    const char *env_path = getenv("GCOV_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        return strdup(env_path);
    }
    
    // Try common build locations
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
            return strdup(common_paths[i]);
        }
    }
    
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr
 * Returns: 1 if error message found, 0 if not, -1 on execution error
 */
static int test_gcov_dump(const char *gcov_dump_path, const char *const args[])
{
    int pipe_fd[2];
    pid_t pid;
    char output[MAX_OUTPUT_SIZE];
    int found_error = 0;
    
    // Create pipe for capturing stderr
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        return -1;
    }
    
    pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return -1;
    }
    
    if (pid == 0) {
        // Child process
        close(pipe_fd[0]);  // Close read end
        
        // Redirect stderr to pipe
        dup2(pipe_fd[1], STDERR_FILENO);
        close(pipe_fd[1]);
        
        // Execute gcov-dump
        execvp(gcov_dump_path, (char *const *)args);
        
        // If we get here, exec failed
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        close(pipe_fd[1]);  // Close write end
        
        // Read stderr output
        ssize_t bytes_read = read(pipe_fd[0], output, MAX_OUTPUT_SIZE - 1);
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
            
            // Check for the error message
            if (strstr(output, ERROR_MSG_PREFIX) != NULL) {
                printf("Found error message in output:\n%s\n", output);
                found_error = 1;
            } else {
                printf("No error message found. Output:\n%s\n", output);
            }
        }
        
        close(pipe_fd[0]);
        
        // Wait for child to finish
        int status;
        waitpid(pid, &status, 0);
        
        return found_error;
    }
}

int main(void)
{
    char *gcov_dump_path = find_gcov_dump();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: gcov-dump executable not found\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in a common location\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test cases designed to trigger the uncovered default case
    struct test_case {
        const char *description;
        const char *args[10];  // NULL terminated
    } test_cases[] = {
        // Single invalid flag tests
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
        
        // Invalid flag in different positions
        {
            "Invalid flag first: '-x -l -p'",
            {gcov_dump_path, "-x", "-l", "-p", NULL}
        },
        {
            "Invalid flag middle: '-l -x -p'",
            {gcov_dump_path, "-l", "-x", "-p", NULL}
        },
        {
            "Invalid flag last: '-l -p -x'",
            {gcov_dump_path, "-l", "-p", "-x", NULL}
        },
        
        // Multiple invalid flags
        {
            "Multiple invalid flags: '-x -z -?'",
            {gcov_dump_path, "-x", "-z", "-?", NULL}
        },
        
        // With filename argument (non-option argument)
        {
            "Invalid flag before filename: '-x dummy.gcda'",
            {gcov_dump_path, "-x", "dummy.gcda", NULL}
        },
        {
            "Valid flag, invalid flag, filename: '-l -x dummy.gcda'",
            {gcov_dump_path, "-l", "-x", "dummy.gcda", NULL}
        },
        {
            "Filename, then invalid flag: 'dummy.gcda -x'",
            {gcov_dump_path, "dummy.gcda", "-x", NULL}
        },
        
        // Double dash handling
        {
            "Double dash with invalid flag: '-- -x'",
            {gcov_dump_path, "--", "-x", NULL}
        },
        {
            "Double dash with multiple: '-- -x -z'",
            {gcov_dump_path, "--", "-x", "-z", NULL}
        },
        
        // Edge case: empty argument
        {
            "Just the program name",
            {gcov_dump_path, NULL}
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
        
        int result = test_gcov_dump(gcov_dump_path, test_cases[i].args);
        
        if (result == 1) {
            printf("✓ PASS: Triggered unknown flag error\n\n");
            passed_tests++;
        } else if (result == 0) {
            printf("✗ FAIL: Did not trigger unknown flag error\n\n");
            failed_tests++;
        } else {
            printf("✗ ERROR: Failed to execute test\n\n");
            failed_tests++;
        }
        
        total_tests++;
    }
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", failed_tests);
    
    free(gcov_dump_path);
    
    if (failed_tests > 0) {
        printf("\nSome tests failed. Check if gcov-dump handles flags differently.\n");
        return EXIT_FAILURE;
    }
    
    printf("\nAll tests passed successfully!\n");
    return EXIT_SUCCESS;
}
