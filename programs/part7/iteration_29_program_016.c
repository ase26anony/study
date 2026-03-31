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
 *           3. PATH search
 */
static int find_gcov_dump(char *path, size_t path_len) {
    const char *env_path = getenv("GCov_DUMP");
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "./gcov-dump",
        "../gcov-dump",
        "gcov-dump",
        NULL
    };
    
    // Try environment variable first
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, path_len - 1);
        path[path_len - 1] = '\0';
        return 1;
    }
    
    // Try common build locations
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path, common_paths[i], path_len - 1);
            path[path_len - 1] = '\0';
            return 1;
        }
    }
    
    // Search in PATH
    const char *path_env = getenv("PATH");
    if (path_env != NULL) {
        char *path_copy = strdup(path_env);
        if (path_copy == NULL) {
            return 0;
        }
        
        char *dir = strtok(path_copy, ":");
        while (dir != NULL) {
            snprintf(path, path_len, "%s/gcov-dump", dir);
            if (access(path, X_OK) == 0) {
                free(path_copy);
                return 1;
            }
            dir = strtok(NULL, ":");
        }
        free(path_copy);
    }
    
    return 0;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if error message found, 0 if not, -1 on execution error.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
    int pipefd[2];
    pid_t pid;
    char output[MAX_OUTPUT_LEN];
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
        
        // Prepare arguments
        char *args[] = {
            (char *)gcov_dump_path,
            (char *)flag,
            NULL
        };
        
        // Execute gcov-dump
        execvp(gcov_dump_path, args);
        
        // If we get here, exec failed
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        close(pipefd[1]);  // Close write end
        
        // Read stderr output
        ssize_t bytes_read = read(pipefd[0], output, MAX_OUTPUT_LEN - 1);
        close(pipefd[0]);
        
        // Wait for child
        int status;
        waitpid(pid, &status, 0);
        
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
            
            // Check for error message
            if (strstr(output, ERROR_MSG_PREFIX) != NULL) {
                printf("Found error message for flag '%s':\n%s\n", flag, output);
                found_error = 1;
            } else {
                printf("No error message found for flag '%s'. Output:\n%s\n", flag, output);
            }
        } else {
            printf("No output for flag '%s'\n", flag);
        }
        
        return found_error;
    }
}

/**
 * Test multiple invalid flags in different positions.
 */
static int test_multiple_invalid_flags(const char *gcov_dump_path) {
    // Test cases covering different positions and combinations
    const char *test_cases[] = {
        // Single invalid flags
        "-x",
        "-z",
        "-?",
        "-X",
        
        // Invalid flag as first argument
        "-x",
        
        // Invalid flag between valid flags
        "-l", "-x", "-p",  // Will be tested as "-l -x -p"
        
        // Multiple invalid flags
        "-x", "-z", "-?",  // Will be tested as "-x -z -?"
        
        // Invalid flag after double dash (getopt behavior)
        "--", "-x",
        
        // Invalid flag with valid flag
        "-l", "-x",
        
        // Edge case: just dash
        "-",
        
        NULL
    };
    
    int success_count = 0;
    int total_tests = 0;
    
    // Test single invalid flags
    for (int i = 0; test_cases[i] != NULL && i < 4; i++) {
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, test_cases[i]) == 1) {
            success_count++;
        }
    }
    
    // Test combinations (we'll build argument strings)
    const char *combinations[] = {
        "-l -x -p",
        "-x -z -?",
        "-l -x",
        "-- -x",
        "-",
        NULL
    };
    
    for (int i = 0; combinations[i] != NULL; i++) {
        total_tests++;
        
        // Build command with all arguments
        char cmd[MAX_PATH_LEN + 100];
        snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, combinations[i]);
        
        // Use popen for simplicity with multiple arguments
        FILE *fp = popen(cmd, "r");
        if (fp == NULL) {
            printf("Failed to execute: %s\n", cmd);
            continue;
        }
        
        char output[MAX_OUTPUT_LEN];
        size_t bytes_read = fread(output, 1, MAX_OUTPUT_LEN - 1, fp);
        output[bytes_read] = '\0';
        
        pclose(fp);
        
        if (strstr(output, ERROR_MSG_PREFIX) != NULL) {
            printf("Found error message for '%s':\n%s\n", combinations[i], output);
            success_count++;
        } else {
            printf("No error message found for '%s'. Output:\n%s\n", combinations[i], output);
        }
    }
    
    printf("\nTest Summary: %d/%d tests triggered the 'unknown flag' error\n", 
           success_count, total_tests);
    
    return success_count > 0 ? 1 : 0;
}

int main(void) {
    char gcov_dump_path[MAX_PATH_LEN];
    
    printf("=== Testing gcov-dump invalid flag handling ===\n\n");
    
    // Find gcov-dump executable
    if (!find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path))) {
        fprintf(stderr, "Error: gcov-dump executable not found\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test with various invalid flags
    int result = test_multiple_invalid_flags(gcov_dump_path);
    
    if (result) {
        printf("\n✓ Successfully triggered the uncovered 'default' case in gcov-dump.cc\n");
        return EXIT_SUCCESS;
    } else {
        printf("\n✗ Failed to trigger the uncovered 'default' case\n");
        return EXIT_FAILURE;
    }
}
