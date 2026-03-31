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
 * Find the gcov-dump executable path
 * Returns: dynamically allocated string with path, or NULL if not found
 */
static char *find_gcov_dump(void)
{
    char *path = getenv("GCOV_DUMP");
    if (path != NULL && access(path, X_OK) == 0) {
        return strdup(path);
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
 * Returns: dynamically allocated string with stderr output, or NULL on failure
 */
static char *run_gcov_dump(const char *gcov_dump_path, char *const argv[])
{
    int pipefd[2];
    pid_t pid;
    char *output = NULL;
    
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
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
        } else {
            output[0] = '\0';
        }
        
        close(pipefd[0]);
        waitpid(pid, NULL, 0);
        
        return output;
    }
}

/**
 * Test a specific argument combination
 * Returns: 1 if error message found, 0 otherwise
 */
static int test_argument_combination(const char *gcov_dump_path, 
                                     char *const argv[], 
                                     const char *description)
{
    printf("Testing: %s\n", description);
    printf("Command: %s", gcov_dump_path);
    
    for (int i = 0; argv[i] != NULL; i++) {
        printf(" %s", argv[i]);
    }
    printf("\n");
    
    char *output = run_gcov_dump(gcov_dump_path, argv);
    if (output == NULL) {
        printf("  Failed to execute command\n");
        return 0;
    }
    
    int found = 0;
    if (strstr(output, ERROR_MSG_PREFIX) != NULL) {
        printf("  SUCCESS: Found error message in output:\n");
        printf("  %s", output);
        found = 1;
    } else {
        printf("  FAILED: No error message found. Output:\n");
        if (strlen(output) > 0) {
            printf("  %s", output);
        } else {
            printf("  (no output)\n");
        }
    }
    
    free(output);
    printf("\n");
    return found;
}

int main(void)
{
    char *gcov_dump_path = find_gcov_dump();
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
        char *args[] = { "gcov-dump", "-x", NULL };
        passed_tests += test_argument_combination(gcov_dump_path, args, 
            "Single invalid flag '-x' at beginning");
        total_tests++;
    }
    
    // Test 2: Single invalid flag 'z'
    {
        char *args[] = { "gcov-dump", "-z", NULL };
        passed_tests += test_argument_combination(gcov_dump_path, args, 
            "Single invalid flag '-z'");
        total_tests++;
    }
    
    // Test 3: Invalid flag '?' (boundary case)
    {
        char *args[] = { "gcov-dump", "-?", NULL };
        passed_tests += test_argument_combination(gcov_dump_path, args, 
            "Invalid flag '-?' (boundary case)");
        total_tests++;
    }
    
    // Test 4: Invalid flag between valid flags
    {
        char *args[] = { "gcov-dump", "-l", "-x", "-p", NULL };
        passed_tests += test_argument_combination(gcov_dump_path, args, 
            "Invalid flag '-x' between valid flags '-l' and '-p'");
        total_tests++;
    }
    
    // Test 5: Multiple invalid flags
    {
        char *args[] = { "gcov-dump", "-a", "-b", "-c", NULL };
        passed_tests += test_argument_combination(gcov_dump_path, args, 
            "Multiple invalid flags '-a -b -c'");
        total_tests++;
    }
    
    // Test 6: Invalid flag after filename argument
    {
        char *args[] = { "gcov-dump", "test.gcda", "-x", NULL };
        passed_tests += test_argument_combination(gcov_dump_path, args, 
            "Invalid flag '-x' after filename argument");
        total_tests++;
    }
    
    // Test 7: Double dash with invalid single-character flag
    {
        char *args[] = { "gcov-dump", "--x", NULL };
        passed_tests += test_argument_combination(gcov_dump_path, args, 
            "Double dash with invalid flag '--x'");
        total_tests++;
    }
    
    // Test 8: Combined valid and invalid in single argument
    {
        char *args[] = { "gcov-dump", "-lxpr", NULL };
        passed_tests += test_argument_combination(gcov_dump_path, args, 
            "Combined flags '-lxpr' (contains invalid 'x')");
        total_tests++;
    }
    
    // Test 9: Invalid flag with version flag (should still trigger error)
    {
        char *args[] = { "gcov-dump", "-v", "-x", NULL };
        passed_tests += test_argument_combination(gcov_dump_path, args, 
            "Invalid flag '-x' after valid '-v'");
        total_tests++;
    }
    
    // Test 10: Edge case - just a dash
    {
        char *args[] = { "gcov-dump", "-", NULL };
        passed_tests += test_argument_combination(gcov_dump_path, args, 
            "Single dash '-' (edge case)");
        total_tests++;
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    
    free(gcov_dump_path);
    
    if (passed_tests > 0) {
        printf("\nSUCCESS: Triggered the uncovered default case in gcov-dump.cc\n");
        return EXIT_SUCCESS;
    } else {
        printf("\nFAILURE: Could not trigger the uncovered default case\n");
        return EXIT_FAILURE;
    }
}
