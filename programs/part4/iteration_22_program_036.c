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
#define TARGET_ERROR_MSG "unknown flag"

/**
 * Find the gcov-dump executable path.
 * Priority: 1. GCov_DUMP environment variable
 *           2. Common build paths
 * Returns dynamically allocated string or NULL if not found.
 */
static char *find_gcov_dump_path(void) {
    char *path = getenv("GCov_DUMP");
    if (path != NULL && access(path, X_OK) == 0) {
        return strdup(path);
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
            return strdup(common_paths[i]);
        }
    }
    
    return NULL;
}

/**
 * Execute command and capture stderr using pipe/fork/exec.
 * Returns dynamically allocated string with stderr output.
 */
static char *capture_stderr(const char *cmd) {
    int pipefd[2];
    pid_t pid;
    char *output = NULL;
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
        
        // Execute command
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        close(pipefd[1]);  // Close write end
        
        // Read stderr output
        char buffer[256];
        ssize_t n;
        
        while ((n = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[n] = '\0';
            
            // Reallocate output buffer if needed
            if (output_len + n + 1 > output_size) {
                size_t new_size = output_size == 0 ? 256 : output_size * 2;
                char *new_output = realloc(output, new_size);
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
            memcpy(output + output_len, buffer, n);
            output_len += n;
            output[output_len] = '\0';
        }
        
        close(pipefd[0]);
        waitpid(pid, NULL, 0);
        
        // Ensure null termination
        if (output) {
            output[output_len] = '\0';
        }
    }
    
    return output;
}

/**
 * Test a specific invalid flag combination.
 * Returns 1 if target error message found, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag_combination) {
    char cmd[MAX_CMD_LEN];
    int found = 0;
    
    // Build command
    snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_path, flag_combination);
    
    printf("Testing: %s\n", cmd);
    
    // Capture stderr
    char *stderr_output = capture_stderr(cmd);
    if (stderr_output == NULL) {
        fprintf(stderr, "Failed to execute command\n");
        return 0;
    }
    
    // Check for target error message
    if (strstr(stderr_output, TARGET_ERROR_MSG) != NULL) {
        printf("  ✓ Found target error message\n");
        printf("  Output: %s", stderr_output);
        found = 1;
    } else {
        printf("  ✗ Target error message not found\n");
        if (strlen(stderr_output) > 0) {
            printf("  Output: %s", stderr_output);
        }
    }
    
    free(stderr_output);
    return found;
}

int main(void) {
    printf("=== Testing gcov-dump default case (unknown flags) ===\n\n");
    
    // Find gcov-dump executable
    char *gcov_dump_path = find_gcov_dump_path();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: gcov-dump executable not found\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in a common location\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test various invalid flag combinations
    const char *test_cases[] = {
        // Single invalid flags
        "-x",
        "-z",
        "-?",
        "-X",
        // Invalid flag in different positions
        "-x file.gcda",           // Invalid flag before filename
        "-l -x -p",               // Invalid flag between valid flags
        "-l file.gcno -x",        // Invalid flag after filename
        "-x -l -p file.gcda",     // Invalid flag first, then valid flags
        // Multiple invalid flags
        "-x -y -z",
        "-a -b -c file.gcno",
        // Edge cases
        "--x",                    // Double dash with single char (getopt may treat as filename)
        "- ",                     // Space after dash
        "-",                      // Just a dash
        // Combination with other arguments
        "-x -v",                  // Invalid then valid (version)
        "-h -x",                  // Help then invalid
        "-x -s -r",               // Invalid with stable and raw flags
        NULL
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    
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
    
    if (passed_tests > 0) {
        printf("\n✅ Successfully triggered the uncovered default case!\n");
        return EXIT_SUCCESS;
    } else {
        printf("\n❌ Failed to trigger the uncovered default case\n");
        return EXIT_FAILURE;
    }
}
