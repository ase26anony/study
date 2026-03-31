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

#define MAX_OUTPUT_SIZE 4096
#define ERROR_MSG_PREFIX "unknown flag"

/**
 * Find gcov-dump executable path
 * Returns: dynamically allocated string with path, or NULL if not found
 */
static char* find_gcov_dump(void) {
    char* path = NULL;
    
    // 1. Check environment variable
    path = getenv("GCOV_DUMP");
    if (path != NULL && access(path, X_OK) == 0) {
        return strdup(path);
    }
    
    // 2. Check common build locations
    const char* candidates[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; candidates[i] != NULL; i++) {
        if (access(candidates[i], X_OK) == 0) {
            return strdup(candidates[i]);
        }
    }
    
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr
 * Returns: dynamically allocated string with stderr output, or NULL on failure
 */
static char* run_gcov_dump(const char* gcov_dump_path, char* const argv[]) {
    int pipefd[2];
    pid_t pid;
    char* output = NULL;
    
    // Create pipe for capturing stderr
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return NULL;
    }
    
    pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return NULL;
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
        output = malloc(MAX_OUTPUT_SIZE);
        if (output == NULL) {
            close(pipefd[0]);
            waitpid(pid, NULL, 0);
            return NULL;
        }
        
        ssize_t bytes_read = read(pipefd[0], output, MAX_OUTPUT_SIZE - 1);
        close(pipefd[0]);
        
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
        } else {
            output[0] = '\0';
        }
        
        // Wait for child to finish
        waitpid(pid, NULL, 0);
        
        return output;
    }
}

/**
 * Test a specific argument combination
 * Returns: 1 if error message found, 0 otherwise
 */
static int test_argument_combination(const char* gcov_dump_path, 
                                    char* const argv[], 
                                    const char* test_name) {
    printf("Testing: %s\n", test_name);
    printf("Command: %s", gcov_dump_path);
    
    for (int i = 0; argv[i] != NULL; i++) {
        printf(" %s", argv[i]);
    }
    printf("\n");
    
    char* output = run_gcov_dump(gcov_dump_path, argv);
    if (output == NULL) {
        printf("  Failed to execute command\n");
        return 0;
    }
    
    int found = (strstr(output, ERROR_MSG_PREFIX) != NULL);
    
    if (found) {
        printf("  SUCCESS: Found error message in output:\n");
        printf("  Output: %s\n", output);
    } else {
        printf("  FAILURE: Error message not found\n");
        if (strlen(output) > 0) {
            printf("  Output: %s\n", output);
        }
    }
    
    free(output);
    return found;
}

int main(void) {
    char* gcov_dump_path = find_gcov_dump();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: gcov-dump executable not found\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Test 1: Single invalid flag at beginning
    {
        char* args[] = { "gcov-dump", "-x", NULL };
        total_tests++;
        if (test_argument_combination(gcov_dump_path, args, "Single invalid flag '-x'")) {
            passed_tests++;
        }
    }
    
    // Test 2: Invalid flag between valid flags
    {
        char* args[] = { "gcov-dump", "-l", "-z", "-p", NULL };
        total_tests++;
        if (test_argument_combination(gcov_dump_path, args, "Invalid '-z' between valid '-l -p'")) {
            passed_tests++;
        }
    }
    
    // Test 3: Multiple invalid flags
    {
        char* args[] = { "gcov-dump", "-a", "-b", "-c", NULL };
        total_tests++;
        if (test_argument_combination(gcov_dump_path, args, "Multiple invalid flags '-a -b -c'")) {
            passed_tests++;
        }
    }
    
    // Test 4: Invalid flag after filename argument
    {
        char* args[] = { "gcov-dump", "test.gcda", "-?", NULL };
        total_tests++;
        if (test_argument_combination(gcov_dump_path, args, "Invalid '-?' after filename")) {
            passed_tests++;
        }
    }
    
    // Test 5: Double dash with invalid single character
    {
        char* args[] = { "gcov-dump", "--x", NULL };
        total_tests++;
        if (test_argument_combination(gcov_dump_path, args, "Double dash with '--x'")) {
            passed_tests++;
        }
    }
    
    // Test 6: Combination with valid help flag (should not trigger error)
    {
        char* args[] = { "gcov-dump", "-h", "-y", NULL };
        total_tests++;
        // This should still trigger error because -y is invalid
        if (test_argument_combination(gcov_dump_path, args, "Valid '-h' with invalid '-y'")) {
            passed_tests++;
        }
    }
    
    // Test 7: Edge case - question mark (special for getopt)
    {
        char* args[] = { "gcov-dump", "-?", NULL };
        total_tests++;
        if (test_argument_combination(gcov_dump_path, args, "Question mark flag '-?'")) {
            passed_tests++;
        }
    }
    
    // Test 8: Invalid flag at end with valid flags
    {
        char* args[] = { "gcov-dump", "-l", "-p", "-s", "-w", NULL };
        total_tests++;
        if (test_argument_combination(gcov_dump_path, args, "Invalid '-w' at end of valid flags")) {
            passed_tests++;
        }
    }
    
    // Test 9: Just a dash (should be treated as filename by getopt)
    {
        char* args[] = { "gcov-dump", "-", NULL };
        total_tests++;
        // This might not trigger the error, but let's see what happens
        if (test_argument_combination(gcov_dump_path, args, "Single dash '-'")) {
            passed_tests++;
        }
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    
    free(gcov_dump_path);
    
    // Consider test successful if at least one invalid flag triggered the error
    return (passed_tests > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
