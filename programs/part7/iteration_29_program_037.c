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
#define ERROR_MSG_SUBSTRING "unknown flag"

/**
 * Execute gcov-dump with given arguments and capture stderr
 * Returns 1 if error message found, 0 if not, -1 on execution error
 */
int test_gcov_dump(const char *gcov_dump_path, char *const argv[]) {
    int pipefd[2];
    pid_t pid;
    char output[MAX_OUTPUT_SIZE] = {0};
    int status;
    ssize_t bytes_read;
    
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
        bytes_read = read(pipefd[0], output, MAX_OUTPUT_SIZE - 1);
        close(pipefd[0]);
        
        // Wait for child
        waitpid(pid, &status, 0);
        
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
            
            // Check for error message
            if (strstr(output, ERROR_MSG_SUBSTRING) != NULL) {
                printf("Found error message in output:\n%s\n", output);
                return 1;
            }
        }
        
        return 0;
    }
}

/**
 * Find gcov-dump executable
 * Checks GCov_DUMP environment variable first, then common locations
 */
char *find_gcov_dump() {
    static char path[1024];
    
    // Check environment variable
    char *env_path = getenv("GCOV_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, sizeof(path) - 1);
        return path;
    }
    
    // Check common build locations
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
            strncpy(path, common_paths[i], sizeof(path) - 1);
            return path;
        }
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    char *gcov_dump_path;
    int tests_passed = 0;
    int total_tests = 0;
    
    printf("=== Testing gcov-dump uncovered lines ===\n\n");
    
    // Find gcov-dump executable
    gcov_dump_path = find_gcov_dump();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test 1: Single invalid flag at beginning
    {
        printf("Test 1: Single invalid flag '-x' at beginning\n");
        char *test_args[] = { "gcov-dump", "-x", NULL };
        int result = test_gcov_dump(gcov_dump_path, test_args);
        total_tests++;
        if (result == 1) {
            printf("✓ PASSED\n");
            tests_passed++;
        } else if (result == 0) {
            printf("✗ FAILED: No error message found\n");
        } else {
            printf("✗ FAILED: Execution error\n");
        }
        printf("\n");
    }
    
    // Test 2: Invalid flag between valid flags
    {
        printf("Test 2: Invalid flag '-z' between valid flags '-l' and '-p'\n");
        char *test_args[] = { "gcov-dump", "-l", "-z", "-p", NULL };
        int result = test_gcov_dump(gcov_dump_path, test_args);
        total_tests++;
        if (result == 1) {
            printf("✓ PASSED\n");
            tests_passed++;
        } else if (result == 0) {
            printf("✗ FAILED: No error message found\n");
        } else {
            printf("✗ FAILED: Execution error\n");
        }
        printf("\n");
    }
    
    // Test 3: Multiple invalid flags
    {
        printf("Test 3: Multiple invalid flags '-? -# -@'\n");
        char *test_args[] = { "gcov-dump", "-?", "-#", "-@", NULL };
        int result = test_gcov_dump(gcov_dump_path, test_args);
        total_tests++;
        if (result == 1) {
            printf("✓ PASSED\n");
            tests_passed++;
        } else if (result == 0) {
            printf("✗ FAILED: No error message found\n");
        } else {
            printf("✗ FAILED: Execution error\n");
        }
        printf("\n");
    }
    
    // Test 4: Invalid flag after non-option argument (simulated with help flag)
    {
        printf("Test 4: Invalid flag '-y' after help flag '-h'\n");
        char *test_args[] = { "gcov-dump", "-h", "-y", NULL };
        int result = test_gcov_dump(gcov_dump_path, test_args);
        total_tests++;
        if (result == 1) {
            printf("✓ PASSED\n");
            tests_passed++;
        } else if (result == 0) {
            printf("✗ FAILED: No error message found\n");
        } else {
            printf("✗ FAILED: Execution error\n");
        }
        printf("\n");
    }
    
    // Test 5: Double dash with invalid single character
    {
        printf("Test 5: Double dash with invalid flag '--x'\n");
        char *test_args[] = { "gcov-dump", "--x", NULL };
        int result = test_gcov_dump(gcov_dump_path, test_args);
        total_tests++;
        if (result == 1) {
            printf("✓ PASSED\n");
            tests_passed++;
        } else if (result == 0) {
            printf("✗ FAILED: No error message found\n");
        } else {
            printf("✗ FAILED: Execution error\n");
        }
        printf("\n");
    }
    
    // Test 6: Combined valid and invalid flags
    {
        printf("Test 6: Combined valid and invalid flags '-l -x -p -r -s -w'\n");
        char *test_args[] = { "gcov-dump", "-l", "-x", "-p", "-r", "-s", "-w", NULL };
        int result = test_gcov_dump(gcov_dump_path, test_args);
        total_tests++;
        if (result == 1) {
            printf("✓ PASSED\n");
            tests_passed++;
        } else if (result == 0) {
            printf("✗ FAILED: No error message found\n");
        } else {
            printf("✗ FAILED: Execution error\n");
        }
        printf("\n");
    }
    
    // Test 7: Boundary case - invalid flag that's a number
    {
        printf("Test 7: Invalid numeric flag '-9'\n");
        char *test_args[] = { "gcov-dump", "-9", NULL };
        int result = test_gcov_dump(gcov_dump_path, test_args);
        total_tests++;
        if (result == 1) {
            printf("✓ PASSED\n");
            tests_passed++;
        } else if (result == 0) {
            printf("✗ FAILED: No error message found\n");
        } else {
            printf("✗ FAILED: Execution error\n");
        }
        printf("\n");
    }
    
    // Summary
    printf("=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    
    if (tests_passed > 0) {
        printf("\n✓ SUCCESS: Successfully triggered the uncovered default case\n");
        printf("  The error message 'unknown flag' was found in stderr output\n");
        return EXIT_SUCCESS;
    } else {
        printf("\n✗ FAILURE: Could not trigger the uncovered default case\n");
        printf("  Check if gcov-dump is the correct version or if flag parsing works differently\n");
        return EXIT_FAILURE;
    }
}
