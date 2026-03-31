/**
 * test_gcov_dump_default_case.c
 * 
 * Tests the uncovered default case in gcov-dump.cc that handles
 * unknown command-line flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH_LEN 1024
#define MAX_OUTPUT_LEN 4096
#define TARGET_ERROR_SUBSTRING "unknown flag"

/**
 * Find the gcov-dump executable path.
 * Priority: 1. GCov_DUMP environment variable
 *           2. Common build locations
 *           3. System PATH
 */
static int find_gcov_dump(char *path, size_t path_len) {
    const char *env_path = getenv("GCov_DUMP");
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    // Try environment variable first
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, path_len - 1);
        path[path_len - 1] = '\0';
        return 0;
    }
    
    // Try common build locations
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path, common_paths[i], path_len - 1);
            path[path_len - 1] = '\0';
            return 0;
        }
    }
    
    // Try to find in PATH
    const char *path_env = getenv("PATH");
    if (path_env != NULL) {
        char *path_copy = strdup(path_env);
        if (path_copy == NULL) {
            return -1;
        }
        
        char *dir = strtok(path_copy, ":");
        while (dir != NULL) {
            char full_path[MAX_PATH_LEN];
            snprintf(full_path, sizeof(full_path), "%s/gcov-dump", dir);
            if (access(full_path, X_OK) == 0) {
                strncpy(path, full_path, path_len - 1);
                path[path_len - 1] = '\0';
                free(path_copy);
                return 0;
            }
            dir = strtok(NULL, ":");
        }
        free(path_copy);
    }
    
    return -1; // Not found
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 0 if target error message found, -1 on execution error,
 * -2 if target message not found.
 */
static int test_gcov_dump_with_args(const char *gcov_dump_path, 
                                   const char *args[], 
                                   int arg_count) {
    int pipefd[2];
    pid_t pid;
    
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
    
    if (pid == 0) { // Child process
        // Close read end of pipe
        close(pipefd[0]);
        
        // Redirect stderr to pipe
        if (dup2(pipefd[1], STDERR_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(pipefd[1]);
        
        // Build argument array
        char **argv = malloc((arg_count + 2) * sizeof(char *));
        if (argv == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        
        argv[0] = (char *)gcov_dump_path;
        for (int i = 0; i < arg_count; i++) {
            argv[i + 1] = (char *)args[i];
        }
        argv[arg_count + 1] = NULL;
        
        // Execute gcov-dump
        execv(gcov_dump_path, argv);
        
        // If we get here, exec failed
        perror("execv");
        free(argv);
        exit(EXIT_FAILURE);
    } else { // Parent process
        // Close write end of pipe
        close(pipefd[1]);
        
        // Read stderr output from pipe
        char buffer[MAX_OUTPUT_LEN];
        ssize_t bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1);
        close(pipefd[0]);
        
        // Wait for child
        int status;
        waitpid(pid, &status, 0);
        
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            
            // Check for target error message
            if (strstr(buffer, TARGET_ERROR_SUBSTRING) != NULL) {
                printf("Found target error message in output:\n%s\n", buffer);
                return 0; // Success - found target message
            } else {
                printf("Output did not contain target message:\n%s\n", buffer);
                return -2; // Target message not found
            }
        } else if (bytes_read == 0) {
            printf("No output from gcov-dump\n");
            return -2;
        } else {
            perror("read");
            return -1;
        }
    }
}

/**
 * Test various invalid flag scenarios.
 */
static int run_tests(const char *gcov_dump_path) {
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test cases with different invalid flags and positions
    struct test_case {
        const char *name;
        const char *args[10];
        int arg_count;
    } test_cases[] = {
        // Single invalid flag
        {"Single invalid flag -x", {"-x"}, 1},
        {"Single invalid flag -z", {"-z"}, 1},
        {"Single invalid flag -?", {"-?"}, 1},
        
        // Invalid flag as first argument
        {"Invalid flag first, then valid flag", {"-x", "-l"}, 2},
        
        // Invalid flag between valid flags
        {"Valid flag, invalid flag, valid flag", {"-l", "-x", "-p"}, 3},
        
        // Multiple invalid flags
        {"Multiple invalid flags", {"-x", "-y", "-z"}, 3},
        
        // Invalid flag after filename (non-option argument)
        {"Invalid flag after filename", {"test.gcda", "-x"}, 2},
        
        // Double dash with invalid flag (edge case)
        {"Double dash with invalid flag", {"--", "-x"}, 2},
        
        // Combined valid and invalid in single argument
        {"Combined flags with invalid", {"-lxz"}, 1},
        
        // End of array marker
        {NULL, {NULL}, 0}
    };
    
    int success_count = 0;
    int total_tests = 0;
    
    for (int i = 0; test_cases[i].name != NULL; i++) {
        printf("Test %d: %s\n", i + 1, test_cases[i].name);
        printf("Args: ");
        for (int j = 0; j < test_cases[i].arg_count; j++) {
            printf("%s ", test_cases[i].args[j]);
        }
        printf("\n");
        
        int result = test_gcov_dump_with_args(gcov_dump_path, 
                                            test_cases[i].args, 
                                            test_cases[i].arg_count);
        
        if (result == 0) {
            printf("✓ SUCCESS: Triggered default case\n\n");
            success_count++;
        } else if (result == -2) {
            printf("✗ FAILED: Did not trigger default case\n\n");
        } else {
            printf("✗ ERROR: Execution failed\n\n");
        }
        
        total_tests++;
        
        // Small delay to avoid overwhelming the system
        usleep(10000);
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Successes: %d\n", success_count);
    printf("Failures: %d\n", total_tests - success_count);
    
    return (success_count > 0) ? 0 : 1;
}

int main(int argc, char *argv[]) {
    char gcov_dump_path[MAX_PATH_LEN];
    
    printf("=== Testing gcov-dump default case coverage ===\n");
    
    // Find gcov-dump executable
    if (find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path)) != 0) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Found gcov-dump: %s\n", gcov_dump_path);
    
    // Run tests
    int result = run_tests(gcov_dump_path);
    
    if (result == 0) {
        printf("\n✓ Successfully triggered the uncovered default case!\n");
    } else {
        printf("\n✗ Failed to trigger the uncovered default case\n");
    }
    
    return result;
}
