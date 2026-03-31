/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by invoking
 * gcov-dump with invalid command-line flags.
 * 
 * Compile with: gcc -std=c99 -O0 -g -o test_gcov_dump test_gcov_dump_invalid_flags.c
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
 * Find the gcov-dump executable path.
 * Priority:
 * 1. GCov_DUMP environment variable
 * 2. Common build tree locations
 * 3. System PATH (via which)
 */
static char *find_gcov_dump_path(void) {
    char *path = NULL;
    
    // 1. Check environment variable
    path = getenv("GCov_DUMP");
    if (path != NULL && access(path, X_OK) == 0) {
        return strdup(path);
    }
    
    // 2. Check common build locations
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "../../prev-gcc/build/gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            return strdup(common_paths[i]);
        }
    }
    
    // 3. Try to find via which command
    FILE *fp = popen("which gcov-dump 2>/dev/null", "r");
    if (fp != NULL) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), fp) != NULL) {
            // Remove trailing newline
            buffer[strcspn(buffer, "\n")] = '\0';
            if (access(buffer, X_OK) == 0) {
                pclose(fp);
                return strdup(buffer);
            }
        }
        pclose(fp);
    }
    
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if error message found, 0 if not, -1 on execution error.
 */
static int test_gcov_dump_with_args(const char *gcov_dump_path, char *const argv[]) {
    int pipefd[2];
    pid_t pid;
    int status;
    char output[MAX_OUTPUT_SIZE];
    int found_error = 0;
    
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
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        close(pipefd[1]);  // Close write end
        
        // Read stderr output
        ssize_t bytes_read = read(pipefd[0], output, sizeof(output) - 1);
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
            
            // Check for the error message
            if (strstr(output, ERROR_MSG_PREFIX) != NULL) {
                printf("Found error message in output:\n%s\n", output);
                found_error = 1;
            } else {
                printf("No error message found. Output:\n%s\n", output);
            }
        } else {
            printf("No output captured from stderr\n");
        }
        
        close(pipefd[0]);
        
        // Wait for child to finish
        waitpid(pid, &status, 0);
        
        // Check exit status (should be non-zero for invalid flags)
        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            printf("Exit status: %d\n", exit_status);
        }
        
        return found_error;
    }
}

/**
 * Run a single test case.
 */
static int run_test_case(const char *gcov_dump_path, const char *test_name, 
                         char *const argv[], int expected_to_find_error) {
    printf("\n=== Test Case: %s ===\n", test_name);
    printf("Command: %s", gcov_dump_path);
    for (int i = 1; argv[i] != NULL; i++) {
        printf(" %s", argv[i]);
    }
    printf("\n");
    
    int result = test_gcov_dump_with_args(gcov_dump_path, argv);
    
    if (result == -1) {
        printf("Test failed to execute\n");
        return 0;
    }
    
    if (expected_to_find_error && !result) {
        printf("FAIL: Expected to find error message but didn't\n");
        return 0;
    } else if (!expected_to_find_error && result) {
        printf("FAIL: Unexpected error message found\n");
        return 0;
    } else {
        printf("PASS\n");
        return 1;
    }
}

int main(void) {
    char *gcov_dump_path = find_gcov_dump_path();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCov_DUMP environment variable to the path of gcov-dump\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n", gcov_dump_path);
    
    int passed_tests = 0;
    int total_tests = 0;
    
    // Test 1: Single invalid flag at the beginning
    {
        char *argv[] = { "gcov-dump", "-x", NULL };
        total_tests++;
        passed_tests += run_test_case(gcov_dump_path, 
                                     "Single invalid flag '-x' at beginning", 
                                     argv, 1);
    }
    
    // Test 2: Single invalid flag 'z'
    {
        char *argv[] = { "gcov-dump", "-z", NULL };
        total_tests++;
        passed_tests += run_test_case(gcov_dump_path, 
                                     "Single invalid flag '-z'", 
                                     argv, 1);
    }
    
    // Test 3: Invalid flag '?' (boundary case)
    {
        char *argv[] = { "gcov-dump", "-?", NULL };
        total_tests++;
        passed_tests += run_test_case(gcov_dump_path, 
                                     "Invalid flag '-?'", 
                                     argv, 1);
    }
    
    // Test 4: Invalid flag between valid flags
    {
        char *argv[] = { "gcov-dump", "-l", "-x", "-p", NULL };
        total_tests++;
        passed_tests += run_test_case(gcov_dump_path, 
                                     "Invalid flag '-x' between valid flags '-l' and '-p'", 
                                     argv, 1);
    }
    
    // Test 5: Multiple invalid flags
    {
        char *argv[] = { "gcov-dump", "-a", "-b", "-c", NULL };
        total_tests++;
        passed_tests += run_test_case(gcov_dump_path, 
                                     "Multiple invalid flags '-a -b -c'", 
                                     argv, 1);
    }
    
    // Test 6: Invalid flag after non-option argument (simulated with a filename)
    {
        char *argv[] = { "gcov-dump", "-l", "dummy.gcda", "-x", NULL };
        total_tests++;
        passed_tests += run_test_case(gcov_dump_path, 
                                     "Invalid flag '-x' after filename argument", 
                                     argv, 1);
    }
    
    // Test 7: Double dash followed by invalid single-character flag
    {
        char *argv[] = { "gcov-dump", "--", "-x", NULL };
        total_tests++;
        passed_tests += run_test_case(gcov_dump_path, 
                                     "Double dash '--' followed by '-x'", 
                                     argv, 1);
    }
    
    // Test 8: Valid flags only (negative test - should not trigger error)
    {
        char *argv[] = { "gcov-dump", "-l", "-p", "-v", NULL };
        total_tests++;
        passed_tests += run_test_case(gcov_dump_path, 
                                     "Valid flags only '-l -p -v' (negative test)", 
                                     argv, 0);
    }
    
    // Test 9: Combined valid and invalid flags
    {
        char *argv[] = { "gcov-dump", "-l", "-x", "-p", "-y", "-v", NULL };
        total_tests++;
        passed_tests += run_test_case(gcov_dump_path, 
                                     "Mixed valid and invalid flags", 
                                     argv, 1);
    }
    
    // Test 10: Invalid flag at the end
    {
        char *argv[] = { "gcov-dump", "-l", "-p", "-q", NULL };
        total_tests++;
        passed_tests += run_test_case(gcov_dump_path, 
                                     "Invalid flag '-q' at the end", 
                                     argv, 1);
    }
    
    printf("\n=== Summary ===\n");
    printf("Passed: %d/%d tests\n", passed_tests, total_tests);
    
    free(gcov_dump_path);
    
    return (passed_tests == total_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}
