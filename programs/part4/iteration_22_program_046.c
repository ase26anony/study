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

#define MAX_CMD_LEN 1024
#define MAX_OUTPUT_LEN 4096

/**
 * Get the path to gcov-dump executable.
 * Priority: 1. GCov_DUMP environment variable
 *           2. Hard-coded common build paths
 * Returns dynamically allocated string or NULL if not found.
 */
char* get_gcov_dump_path(void) {
    char* path = getenv("GCov_DUMP");
    
    if (path != NULL && access(path, X_OK) == 0) {
        return strdup(path);
    }
    
    // Try common build locations
    const char* common_paths[] = {
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
            return strdup(common_paths[i]);
        }
    }
    
    return NULL;
}

/**
 * Execute command and capture stderr output.
 * Returns dynamically allocated string with stderr output.
 * Returns NULL on failure.
 */
char* capture_stderr(const char* cmd) {
    int pipefd[2];
    pid_t pid;
    char* output = NULL;
    size_t output_size = 0;
    size_t output_len = 0;
    
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
        
        // Execute command using shell
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        _exit(127);  // execl failed
    } else {
        // Parent process
        close(pipefd[1]);  // Close write end
        
        // Read stderr output
        char buffer[256];
        ssize_t bytes_read;
        
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            // Ensure null termination
            buffer[bytes_read] = '\0';
            
            // Resize output buffer if needed
            size_t new_len = output_len + bytes_read;
            if (new_len + 1 > output_size) {
                size_t new_size = (new_len + 256) & ~255;  // Round up to 256
                char* new_output = realloc(output, new_size);
                if (!new_output) {
                    free(output);
                    close(pipefd[0]);
                    waitpid(pid, NULL, 0);
                    return NULL;
                }
                output = new_output;
                output_size = new_size;
            }
            
            // Append to output
            memcpy(output + output_len, buffer, bytes_read);
            output_len = new_len;
        }
        
        close(pipefd[0]);
        
        // Wait for child
        int status;
        waitpid(pid, &status, 0);
        
        // Null terminate the output
        if (output) {
            output[output_len] = '\0';
        }
    }
    
    return output;
}

/**
 * Test a specific invalid flag combination.
 * Returns 1 if "unknown flag" error found, 0 otherwise.
 */
int test_invalid_flag(const char* gcov_dump_path, const char* flag_combination) {
    char cmd[MAX_CMD_LEN];
    int result = 0;
    
    // Build command
    snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_path, flag_combination);
    
    printf("Testing: %s\n", cmd);
    
    // Capture stderr
    char* stderr_output = capture_stderr(cmd);
    if (stderr_output == NULL) {
        fprintf(stderr, "Failed to execute command\n");
        return 0;
    }
    
    // Check for "unknown flag" message
    if (strstr(stderr_output, "unknown flag") != NULL) {
        printf("  ✓ Found 'unknown flag' error\n");
        printf("  Output: %s", stderr_output);
        result = 1;
    } else {
        printf("  ✗ No 'unknown flag' error found\n");
        printf("  Output: %s", stderr_output);
    }
    
    free(stderr_output);
    return result;
}

int main(void) {
    printf("=== Testing gcov-dump default case (unknown flags) ===\n\n");
    
    // Get gcov-dump path
    char* gcov_dump_path = get_gcov_dump_path();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: gcov-dump executable not found\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in a common location\n");
        return 1;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test cases covering different scenarios
    const char* test_cases[] = {
        // Single invalid flags
        "-x",
        "-z",
        "-?",
        "-@",
        // Invalid flag as first argument
        "-x -l",
        // Invalid flag between valid flags
        "-l -x -p",
        "-r -z -s",
        // Invalid flag after valid flags
        "-p -r -x",
        // Multiple invalid flags
        "-x -y -z",
        // Invalid flag after double dash (getopt treats differently)
        "-- -x",
        // Invalid flag with filename argument
        "-x dummy.gcda",
        "-l -x dummy.gcno",
        // Edge case: dash only
        "-",
        // Combination with help flag (should not trigger unknown flag)
        "-h -x",  // Help should print first, but we still test
        NULL
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Run all test cases
    for (int i = 0; test_cases[i] != NULL; i++) {
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, test_cases[i])) {
            passed_tests++;
        }
        printf("\n");
    }
    
    // Cleanup
    free(gcov_dump_path);
    
    // Summary
    printf("=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    
    if (passed_tests > 0) {
        printf("\n✓ Successfully triggered the uncovered default case!\n");
        return 0;
    } else {
        printf("\n✗ Failed to trigger the uncovered default case\n");
        return 1;
    }
}
