/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by invoking
 * gcov-dump with invalid command-line flags.
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
 * Priority:
 * 1. GCov_DUMP environment variable
 * 2. Common build locations
 * 3. System PATH
 */
static int find_gcov_dump(char *path, size_t path_len) {
    const char *env_path = getenv("GCOV_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, path_len - 1);
        path[path_len - 1] = '\0';
        return 1;
    }
    
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "./gcov-dump",
        "../gcov-dump",
        "../../gcov-dump",
        "gcov-dump"  // Try PATH
    };
    
    for (size_t i = 0; i < sizeof(common_paths) / sizeof(common_paths[0]); i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path, common_paths[i], path_len - 1);
            path[path_len - 1] = '\0';
            return 1;
        }
    }
    
    return 0;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if target error message found, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char **argv) {
    int found_error = 0;
    int pipefd[2];
    pid_t pid;
    
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 0;
    }
    
    pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return 0;
    }
    
    if (pid == 0) {
        // Child process
        close(pipefd[0]);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        execvp(gcov_dump_path, (char *const *)argv);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        close(pipefd[1]);
        
        char buffer[MAX_OUTPUT_LEN];
        ssize_t bytes_read;
        
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            if (strstr(buffer, TARGET_ERROR_MSG) != NULL) {
                found_error = 1;
                printf("Found target error message: %s", buffer);
            }
        }
        
        close(pipefd[0]);
        
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            // Non-zero exit is expected for invalid flags
            printf("Exit status: %d (expected non-zero)\n", WEXITSTATUS(status));
        }
    }
    
    return found_error;
}

/**
 * Test various invalid flag scenarios.
 */
static void run_tests(const char *gcov_dump_path) {
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test 1: Single invalid flag as first argument
    {
        const char *argv[] = { "gcov-dump", "-x", NULL };
        printf("Test 1: Single invalid flag '-x'\n");
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, argv)) {
            printf("✓ PASSED\n\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n\n");
        }
    }
    
    // Test 2: Invalid flag between valid flags
    {
        const char *argv[] = { "gcov-dump", "-l", "-z", "-p", NULL };
        printf("Test 2: Invalid flag '-z' between valid flags '-l' and '-p'\n");
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, argv)) {
            printf("✓ PASSED\n\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n\n");
        }
    }
    
    // Test 3: Multiple invalid flags
    {
        const char *argv[] = { "gcov-dump", "-?", "-@", "-!", NULL };
        printf("Test 3: Multiple invalid flags '-?', '-@', '-!'\n");
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, argv)) {
            printf("✓ PASSED\n\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n\n");
        }
    }
    
    // Test 4: Invalid flag after non-option argument (filename)
    {
        const char *argv[] = { "gcov-dump", "test.gcda", "-y", NULL };
        printf("Test 4: Invalid flag '-y' after filename argument\n");
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, argv)) {
            printf("✓ PASSED\n\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n\n");
        }
    }
    
    // Test 5: Double dash with invalid single-character flag
    {
        const char *argv[] = { "gcov-dump", "--x", NULL };
        printf("Test 5: Double dash with invalid flag '--x'\n");
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, argv)) {
            printf("✓ PASSED\n\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n\n");
        }
    }
    
    // Test 6: Combined valid and invalid flags
    {
        const char *argv[] = { "gcov-dump", "-l", "-x", "-p", "-r", "-s", NULL };
        printf("Test 6: Mixed valid and invalid flags '-l -x -p -r -s'\n");
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, argv)) {
            printf("✓ PASSED\n\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n\n");
        }
    }
    
    // Test 7: Boundary case - invalid flag at end
    {
        const char *argv[] = { "gcov-dump", "-v", "-h", "-#", NULL };
        printf("Test 7: Invalid flag '-#' at the end\n");
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, argv)) {
            printf("✓ PASSED\n\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n\n");
        }
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", total_tests - passed_tests);
    
    if (passed_tests == total_tests) {
        printf("\n✓ All tests passed! The uncovered lines should now be executed.\n");
    } else {
        printf("\n⚠ Some tests failed. Check if gcov-dump handles invalid flags differently.\n");
    }
}

int main(void) {
    char gcov_dump_path[MAX_PATH_LEN];
    
    if (!find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path))) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in PATH.\n");
        fprintf(stderr, "Common locations checked:\n");
        fprintf(stderr, "  - ./gcc/gcov-dump\n");
        fprintf(stderr, "  - ../gcc/gcov-dump\n");
        fprintf(stderr, "  - ./gcov-dump\n");
        fprintf(stderr, "  - gcov-dump (from PATH)\n");
        return EXIT_FAILURE;
    }
    
    run_tests(gcov_dump_path);
    
    return EXIT_SUCCESS;
}
