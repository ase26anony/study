/**
 * test_gcov_dump_default_case.c
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
 * Priority: 1. GCov_DUMP environment variable
 *           2. Common build locations
 *           3. System PATH
 */
static int find_gcov_dump(char *path, size_t path_len) {
    const char *env_path = getenv("GCov_DUMP");
    const char *candidates[] = {
        "./gcc/gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "gcov-dump",  // Try PATH
        NULL
    };
    
    // Try environment variable first
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, path_len - 1);
        path[path_len - 1] = '\0';
        return 0;
    }
    
    // Try candidate paths
    for (int i = 0; candidates[i] != NULL; i++) {
        if (access(candidates[i], X_OK) == 0) {
            strncpy(path, candidates[i], path_len - 1);
            path[path_len - 1] = '\0';
            return 0;
        }
    }
    
    // Last resort: try which/whereis
    FILE *fp = popen("which gcov-dump 2>/dev/null", "r");
    if (fp != NULL) {
        if (fgets(path, path_len, fp) != NULL) {
            // Remove trailing newline
            path[strcspn(path, "\n")] = '\0';
            if (access(path, X_OK) == 0) {
                pclose(fp);
                return 0;
            }
        }
        pclose(fp);
    }
    
    return -1;  // Not found
}

/**
 * Test a specific invalid flag combination.
 * Returns 0 if target error message found, -1 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
    int status = -1;
    int pipe_fd[2];
    pid_t pid;
    
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
    
    if (pid == 0) {  // Child process
        // Close read end of pipe
        close(pipe_fd[0]);
        
        // Redirect stderr to pipe
        if (dup2(pipe_fd[1], STDERR_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(pipe_fd[1]);
        
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
    } else {  // Parent process
        // Close write end of pipe
        close(pipe_fd[1]);
        
        // Read stderr output from pipe
        char buffer[MAX_OUTPUT_LEN];
        ssize_t bytes_read = read(pipe_fd[0], buffer, sizeof(buffer) - 1);
        
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            
            // Check for target error message
            if (strstr(buffer, TARGET_ERROR_MSG) != NULL) {
                printf("✓ Found target error message for flag '%s':\n", flag);
                printf("  %s", buffer);
                status = 0;
            } else {
                printf("✗ No target message for flag '%s'\n", flag);
                if (bytes_read > 0) {
                    printf("  Output: %s\n", buffer);
                }
            }
        } else {
            printf("✗ No output for flag '%s'\n", flag);
        }
        
        close(pipe_fd[0]);
        
        // Wait for child to finish
        int child_status;
        waitpid(pid, &child_status, 0);
    }
    
    return status;
}

/**
 * Test multiple invalid flag scenarios.
 */
static void run_tests(const char *gcov_dump_path) {
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test cases covering different positions and combinations
    const char *test_cases[] = {
        // Single invalid flags
        "-x",
        "-z",
        "-?",
        "-@",
        
        // Invalid flag as first argument
        "-x -l",      // Invalid then valid
        "-x -p -r",   // Invalid then multiple valid
        
        // Invalid flag between valid flags
        "-l -x -p",   // Valid, invalid, valid
        "-p -? -s",   // Valid, invalid, valid
        
        // Invalid flag after non-option argument (if supported)
        // Note: gcov-dump might accept filenames
        "test.gcda -x",
        
        // Double dash with invalid flag (edge case)
        "--x",
        "-- -x",      // Double dash separator
        
        // Multiple invalid flags
        "-x -y -z",
        
        // Combined invalid flag
        "-xyz",
        
        NULL
    };
    
    int success_count = 0;
    int total_tests = 0;
    
    for (int i = 0; test_cases[i] != NULL; i++) {
        total_tests++;
        printf("Test %d: gcov-dump %s\n", total_tests, test_cases[i]);
        
        // Build command with test case
        char command[MAX_PATH_LEN + 100];
        snprintf(command, sizeof(command), "%s %s 2>&1", 
                 gcov_dump_path, test_cases[i]);
        
        // Use popen for simpler output capture
        FILE *fp = popen(command, "r");
        if (fp == NULL) {
            printf("  Failed to execute command\n");
            continue;
        }
        
        char buffer[MAX_OUTPUT_LEN];
        size_t bytes_read = fread(buffer, 1, sizeof(buffer) - 1, fp);
        buffer[bytes_read] = '\0';
        
        pclose(fp);
        
        // Check for target error message
        if (strstr(buffer, TARGET_ERROR_MSG) != NULL) {
            printf("  ✓ Triggered default case\n");
            success_count++;
            
            // Extract and show just the error line
            char *line = strtok(buffer, "\n");
            while (line != NULL) {
                if (strstr(line, TARGET_ERROR_MSG)) {
                    printf("    Error: %s\n", line);
                    break;
                }
                line = strtok(NULL, "\n");
            }
        } else {
            printf("  ✗ Did not trigger default case\n");
            if (bytes_read > 0 && bytes_read < 100) {
                printf("    Output: %s\n", buffer);
            }
        }
        printf("\n");
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Successfully triggered default case: %d\n", success_count);
    printf("Success rate: %.1f%%\n", 
           total_tests > 0 ? (100.0 * success_count / total_tests) : 0.0);
    
    if (success_count > 0) {
        printf("\n✅ SUCCESS: Uncovered default case was triggered!\n");
    } else {
        printf("\n❌ FAILURE: Could not trigger uncovered default case\n");
    }
}

int main(void) {
    char gcov_dump_path[MAX_PATH_LEN];
    
    printf("=== Testing gcov-dump Default Case Coverage ===\n\n");
    
    // Find gcov-dump executable
    if (find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path)) != 0) {
        fprintf(stderr, "Error: gcov-dump executable not found\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        fprintf(stderr, "Common locations: ./gcc/gcov-dump, ../gcc/gcov-dump\n");
        return EXIT_FAILURE;
    }
    
    printf("Found gcov-dump: %s\n", gcov_dump_path);
    
    // Verify executable
    if (access(gcov_dump_path, X_OK) != 0) {
        fprintf(stderr, "Error: Cannot execute %s\n", gcov_dump_path);
        return EXIT_FAILURE;
    }
    
    // Run comprehensive tests
    run_tests(gcov_dump_path);
    
    // Also test with a simple system call for verification
    printf("\n=== Quick Verification ===\n");
    char command[MAX_PATH_LEN + 50];
    snprintf(command, sizeof(command), "%s -x 2>&1", gcov_dump_path);
    
    FILE *fp = popen(command, "r");
    if (fp != NULL) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), fp) != NULL) {
            if (strstr(buffer, TARGET_ERROR_MSG)) {
                printf("Direct test with '-x': SUCCESS\n");
                printf("Output: %s", buffer);
            }
        }
        pclose(fp);
    }
    
    return EXIT_SUCCESS;
}
