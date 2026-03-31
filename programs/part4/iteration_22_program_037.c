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
    
    // Try environment variable first
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, path_len - 1);
        path[path_len - 1] = '\0';
        return 0;
    }
    
    // Try common build locations
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "../../prev-gcc/build/gcc/gcov-dump",
        "gcc/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path, common_paths[i], path_len - 1);
            path[path_len - 1] = '\0';
            return 0;
        }
    }
    
    // Try system PATH
    const char *system_cmd = "which gcov-dump 2>/dev/null";
    FILE *fp = popen(system_cmd, "r");
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
    
    return -1; // Not found
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 0 on success (found target error), 1 on failure.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
    int pipefd[2];
    pid_t pid;
    char output[MAX_OUTPUT_LEN] = {0};
    int found_target = 0;
    
    // Create pipe for capturing stderr
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }
    
    pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return 1;
    }
    
    if (pid == 0) { // Child process
        // Redirect stderr to pipe
        close(pipefd[0]);
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
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else { // Parent process
        close(pipefd[1]);
        
        // Read stderr output from pipe
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], output, sizeof(output) - 1)) > 0) {
            output[bytes_read] = '\0';
            
            // Check for target error message
            if (strstr(output, TARGET_ERROR_MSG) != NULL) {
                found_target = 1;
            }
            
            // Print output for debugging
            printf("Output for flag '%s':\n%s\n", flag, output);
        }
        close(pipefd[0]);
        
        // Wait for child to finish
        int status;
        waitpid(pid, &status, 0);
        
        if (found_target) {
            printf("✓ Successfully triggered default case with flag '%s'\n", flag);
            return 0;
        } else {
            printf("✗ Failed to trigger default case with flag '%s'\n", flag);
            return 1;
        }
    }
}

/**
 * Test multiple invalid flags in different positions.
 */
static int test_multiple_invalid_flags(const char *gcov_dump_path) {
    // Test cases: invalid flag in different positions
    const char *test_cases[] = {
        // Single invalid flags
        "-x",
        "-z",
        "-?",
        "-X",
        
        // Invalid flag as first argument
        "-x -l",      // invalid then valid
        "-x -p -r",   // invalid then multiple valid
        
        // Invalid flag between valid flags
        "-l -x -p",   // valid, invalid, valid
        "-p -? -r",   // valid, invalid, valid
        
        // Invalid flag after non-option argument (if supported)
        // Note: gcov-dump might accept filenames as arguments
        
        // Double dash with invalid flag (edge case)
        "--x",
        "--z",
        
        // Multiple invalid flags
        "-x -z -?",
        
        NULL
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("\n=== Testing invalid flags for gcov-dump ===\n");
    printf("Using executable: %s\n\n", gcov_dump_path);
    
    for (int i = 0; test_cases[i] != NULL; i++) {
        total_tests++;
        
        // Split arguments for exec
        char cmd[MAX_PATH_LEN + 100];
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_path, test_cases[i]);
        
        printf("Test %d: %s\n", i + 1, cmd);
        
        // Use popen for simpler output capture
        FILE *fp = popen(cmd, "r");
        if (fp == NULL) {
            printf("  Failed to execute command\n");
            continue;
        }
        
        char output[MAX_OUTPUT_LEN] = {0};
        size_t total_bytes = 0;
        char buffer[256];
        
        // Read both stdout and stderr (popen combines them)
        while (fgets(buffer, sizeof(buffer), fp) != NULL && 
               total_bytes < sizeof(output) - 1) {
            strncat(output, buffer, sizeof(output) - total_bytes - 1);
            total_bytes += strlen(buffer);
        }
        
        pclose(fp);
        
        // Check for target error message
        if (strstr(output, TARGET_ERROR_MSG) != NULL) {
            printf("  ✓ Triggered default case\n");
            passed_tests++;
        } else {
            printf("  ✗ Did not trigger default case\n");
            if (strlen(output) > 0) {
                printf("  Output: %s", output);
            }
        }
        
        printf("\n");
    }
    
    printf("=== Test Summary ===\n");
    printf("Passed: %d/%d tests\n", passed_tests, total_tests);
    
    return (passed_tests > 0) ? 0 : 1;
}

int main(int argc, char *argv[]) {
    char gcov_dump_path[MAX_PATH_LEN];
    
    // Find gcov-dump executable
    if (find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path)) != 0) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH.\n");
        fprintf(stderr, "Common locations checked:\n");
        fprintf(stderr, "  - ./gcc/gcov-dump\n");
        fprintf(stderr, "  - ../gcc/gcov-dump\n");
        fprintf(stderr, "  - gcc/gcov-dump\n");
        fprintf(stderr, "  - System PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Found gcov-dump at: %s\n", gcov_dump_path);
    
    // Test with multiple invalid flags
    int result = test_multiple_invalid_flags(gcov_dump_path);
    
    // Also test a specific case with more detailed output
    printf("\n=== Detailed test of single invalid flag ===\n");
    if (test_invalid_flag(gcov_dump_path, "-x") == 0) {
        printf("Successfully verified default case execution.\n");
    } else {
        printf("Warning: Could not verify default case with single flag test.\n");
    }
    
    return result;
}
