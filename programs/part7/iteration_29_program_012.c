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
 * Find the gcov-dump executable path.
 * Priority: 1. GCov_DUMP environment variable
 *           2. Common build locations
 *           3. System PATH
 */
static char *find_gcov_dump_path(void) {
    static char path[MAX_PATH_LEN];
    
    // Try environment variable first
    char *env_path = getenv("GCov_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, MAX_PATH_LEN - 1);
        path[MAX_PATH_LEN - 1] = '\0';
        return path;
    }
    
    // Try common build locations
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "./gcov-dump",
        "../gcov-dump",
        "gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path, common_paths[i], MAX_PATH_LEN - 1);
            path[MAX_PATH_LEN - 1] = '\0';
            return path;
        }
    }
    
    // Search in PATH
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
 * Returns 1 if target error message found, 0 if not, -1 on execution error.
 */
static int test_gcov_dump_args(const char *gcov_dump_path, char *const argv[]) {
    int pipefd[2];
    pid_t pid;
    char output[MAX_OUTPUT_LEN] = {0};
    int found_target = 0;
    
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
        // Redirect stderr to pipe
        close(pipefd[0]);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        // Execute gcov-dump
        execvp(gcov_dump_path, argv);
        
        // If we get here, exec failed
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else {  // Parent process
        int status;
        
        // Close write end of pipe
        close(pipefd[1]);
        
        // Read stderr output
        ssize_t bytes_read = read(pipefd[0], output, MAX_OUTPUT_LEN - 1);
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
            
            // Check for target error message
            if (strstr(output, TARGET_ERROR_MSG) != NULL) {
                found_target = 1;
                printf("Found target error message in output:\n%s\n", output);
            } else {
                printf("Output (no target message):\n%s\n", output);
            }
        }
        
        close(pipefd[0]);
        
        // Wait for child to finish
        waitpid(pid, &status, 0);
        
        // Check exit status (should be non-zero for invalid flags)
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            printf("gcov-dump exited with status %d (expected for invalid flags)\n", 
                   WEXITSTATUS(status));
        }
        
        return found_target;
    }
}

/**
 * Test a specific invalid flag scenario.
 */
static int test_invalid_flag(const char *gcov_dump_path, 
                            const char *description,
                            char *const argv[]) {
    printf("\n=== Testing: %s ===\n", description);
    printf("Command: %s", gcov_dump_path);
    for (int i = 1; argv[i] != NULL; i++) {
        printf(" %s", argv[i]);
    }
    printf("\n");
    
    int result = test_gcov_dump_args(gcov_dump_path, (char *const *)argv);
    
    if (result == 1) {
        printf("✓ SUCCESS: Triggered default case with '%s'\n", description);
    } else if (result == 0) {
        printf("✗ FAILED: Did not trigger default case with '%s'\n", description);
    } else {
        printf("✗ ERROR: Execution failed for '%s'\n", description);
    }
    
    return result;
}

int main(void) {
    char *gcov_dump_path = find_gcov_dump_path();
    
    if (!gcov_dump_path) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n", gcov_dump_path);
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Test 1: Single invalid flag as first argument
    {
        char *argv[] = { "gcov-dump", "-x", NULL };
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, "Single invalid flag '-x'", argv) == 1) {
            passed_tests++;
        }
    }
    
    // Test 2: Invalid flag between valid flags
    {
        char *argv[] = { "gcov-dump", "-l", "-z", "-p", NULL };
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, "Invalid '-z' between valid '-l -p'", argv) == 1) {
            passed_tests++;
        }
    }
    
    // Test 3: Multiple invalid flags
    {
        char *argv[] = { "gcov-dump", "-?", "-@", "-#", NULL };
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, "Multiple invalid flags '-? -@ -#'", argv) == 1) {
            passed_tests++;
        }
    }
    
    // Test 4: Invalid flag after non-option argument (simulated with a filename)
    {
        char *argv[] = { "gcov-dump", "-l", "dummy.gcda", "-y", NULL };
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, "Invalid '-y' after filename", argv) == 1) {
            passed_tests++;
        }
    }
    
    // Test 5: Double dash followed by invalid single-character flag
    {
        char *argv[] = { "gcov-dump", "--", "-w", NULL };
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, "Double dash '--' followed by '-w'", argv) == 1) {
            passed_tests++;
        }
    }
    
    // Test 6: Combination of valid and invalid flags in complex order
    {
        char *argv[] = { "gcov-dump", "-r", "-s", "-!", "-p", "-$", NULL };
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, "Complex mix '-r -s -! -p -$'", argv) == 1) {
            passed_tests++;
        }
    }
    
    // Test 7: Boundary case - invalid flag at very end
    {
        char *argv[] = { "gcov-dump", "-l", "-p", "-v", "-m", NULL };
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, "Invalid '-m' at end after valid flags", argv) == 1) {
            passed_tests++;
        }
    }
    
    // Test 8: Test with 'h' flag (should show usage, not error)
    {
        char *argv[] = { "gcov-dump", "-h", NULL };
        printf("\n=== Testing valid '-h' flag (should NOT trigger default case) ===\n");
        total_tests++;
        int result = test_gcov_dump_args(gcov_dump_path, (char *const *)argv);
        if (result == 0) {
            printf("✓ SUCCESS: '-h' correctly showed usage (didn't trigger default case)\n");
            passed_tests++;
        } else {
            printf("✗ FAILED: '-h' incorrectly triggered default case\n");
        }
    }
    
    printf("\n=== Summary ===\n");
    printf("Tests passed: %d/%d\n", passed_tests, total_tests);
    
    if (passed_tests >= 6) {  // Expect at least 6 of the 7 invalid flag tests to pass
        printf("\n✓ SUCCESS: Successfully triggered the uncovered default case in gcov-dump.cc\n");
        return EXIT_SUCCESS;
    } else {
        printf("\n✗ FAILED: Could not reliably trigger the uncovered default case\n");
        return EXIT_FAILURE;
    }
}
