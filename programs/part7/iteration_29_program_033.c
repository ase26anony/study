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

#define MAX_OUTPUT 4096
#define MAX_ARGS 16

/**
 * Find gcov-dump executable using environment variable or common paths
 */
static char *find_gcov_dump(void) {
    static char path[1024];
    
    // Try environment variable first
    char *env_path = getenv("GCOV_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
        return path;
    }
    
    // Try common build locations
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path, common_paths[i], sizeof(path) - 1);
            path[sizeof(path) - 1] = '\0';
            return path;
        }
    }
    
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr
 * Returns 1 if "unknown flag" error is found, 0 otherwise
 */
static int test_invalid_flag(const char *gcov_dump_path, const char **argv, int argc) {
    int pipefd[2];
    pid_t pid;
    char output[MAX_OUTPUT] = {0};
    int status;
    
    // Create pipe for capturing stderr
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
        close(pipefd[0]);  // Close read end
        
        // Redirect stderr to pipe
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        // Prepare arguments for execvp
        char *exec_args[MAX_ARGS];
        exec_args[0] = (char *)gcov_dump_path;
        
        for (int i = 0; i < argc && i < MAX_ARGS - 2; i++) {
            exec_args[i + 1] = (char *)argv[i];
        }
        exec_args[argc + 1] = NULL;
        
        // Execute gcov-dump
        execvp(gcov_dump_path, exec_args);
        
        // If we get here, exec failed
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        close(pipefd[1]);  // Close write end
        
        // Read stderr output
        ssize_t bytes_read = read(pipefd[0], output, MAX_OUTPUT - 1);
        close(pipefd[0]);
        
        // Wait for child
        waitpid(pid, &status, 0);
        
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
            
            // Check for "unknown flag" error message
            if (strstr(output, "unknown flag") != NULL ||
                strstr(output, "unknown flag `") != NULL) {
                printf("Found expected error: %s", output);
                return 1;
            }
            
            printf("Output (no match): %s", output);
        }
        
        return 0;
    }
}

/**
 * Test various invalid flag scenarios
 */
static int run_tests(const char *gcov_dump_path) {
    int tests_passed = 0;
    int total_tests = 0;
    
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test 1: Single invalid flag at beginning
    {
        printf("Test 1: Single invalid flag '-x'\n");
        const char *args[] = {"-x"};
        if (test_invalid_flag(gcov_dump_path, args, 1)) {
            tests_passed++;
        }
        total_tests++;
        printf("\n");
    }
    
    // Test 2: Invalid flag between valid flags
    {
        printf("Test 2: Invalid flag '-z' between valid flags '-l' and '-p'\n");
        const char *args[] = {"-l", "-z", "-p"};
        if (test_invalid_flag(gcov_dump_path, args, 3)) {
            tests_passed++;
        }
        total_tests++;
        printf("\n");
    }
    
    // Test 3: Multiple invalid flags
    {
        printf("Test 3: Multiple invalid flags '-? -y'\n");
        const char *args[] = {"-?", "-y"};
        if (test_invalid_flag(gcov_dump_path, args, 2)) {
            tests_passed++;
        }
        total_tests++;
        printf("\n");
    }
    
    // Test 4: Invalid flag after non-option argument
    {
        printf("Test 4: Invalid flag '-a' after filename\n");
        const char *args[] = {"-l", "test.gcda", "-a"};
        if (test_invalid_flag(gcov_dump_path, args, 3)) {
            tests_passed++;
        }
        total_tests++;
        printf("\n");
    }
    
    // Test 5: Double dash with invalid single char flag
    {
        printf("Test 5: Double dash with invalid flag '--x'\n");
        const char *args[] = {"--x"};
        if (test_invalid_flag(gcov_dump_path, args, 1)) {
            tests_passed++;
        }
        total_tests++;
        printf("\n");
    }
    
    // Test 6: Combination of valid and invalid flags
    {
        printf("Test 6: Combination '-l -x -p -r -s -q'\n");
        const char *args[] = {"-l", "-x", "-p", "-r", "-s", "-q"};
        if (test_invalid_flag(gcov_dump_path, args, 6)) {
            tests_passed++;
        }
        total_tests++;
        printf("\n");
    }
    
    // Test 7: Invalid flag with value (edge case)
    {
        printf("Test 7: Invalid flag '-k' with argument\n");
        const char *args[] = {"-k", "value"};
        if (test_invalid_flag(gcov_dump_path, args, 2)) {
            tests_passed++;
        }
        total_tests++;
        printf("\n");
    }
    
    printf("Summary: %d/%d tests passed\n", tests_passed, total_tests);
    return tests_passed > 0 ? 0 : 1;
}

int main(void) {
    char *gcov_dump_path = find_gcov_dump();
    
    if (!gcov_dump_path) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in a common location\n");
        return EXIT_FAILURE;
    }
    
    printf("Found gcov-dump: %s\n", gcov_dump_path);
    
    // Run all tests
    int result = run_tests(gcov_dump_path);
    
    if (result == 0) {
        printf("\nSuccess: Triggered the uncovered default case in gcov-dump.cc\n");
    } else {
        printf("\nFailure: Could not trigger the uncovered default case\n");
    }
    
    return result;
}
