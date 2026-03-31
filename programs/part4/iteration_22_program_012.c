/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Tests the uncovered default case in gcov-dump's option parsing.
 * Executes gcov-dump with various invalid flags and verifies the
 * "unknown flag" error message is printed to stderr.
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
 * Find the gcov-dump executable path.
 * Checks GCov_DUMP environment variable first, then common locations.
 * Returns dynamically allocated string (caller must free) or NULL if not found.
 */
static char *find_gcov_dump(void) {
    char *path = getenv("GCov_DUMP");
    if (path != NULL && access(path, X_OK) == 0) {
        return strdup(path);
    }
    
    // Common locations in GCC build trees
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
 * Execute command and capture stderr.
 * Returns dynamically allocated string with stderr output (caller must free),
 * or NULL on failure. Exit status stored in status_ptr.
 */
static char *run_command_capture_stderr(const char *cmd, int *status_ptr) {
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
    }
    
    // Parent process
    close(pipefd[1]);  // Close write end
    
    // Read stderr from pipe
    char buffer[256];
    ssize_t bytes_read;
    
    while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
        // Resize output buffer if needed
        if (output_len + bytes_read + 1 > output_size) {
            size_t new_size = output_size == 0 ? 512 : output_size * 2;
            char *new_output = realloc(output, new_size);
            if (!new_output) {
                free(output);
                close(pipefd[0]);
                return NULL;
            }
            output = new_output;
            output_size = new_size;
        }
        
        memcpy(output + output_len, buffer, bytes_read);
        output_len += bytes_read;
    }
    
    close(pipefd[0]);
    
    // Wait for child
    int status;
    waitpid(pid, &status, 0);
    
    if (status_ptr) {
        *status_ptr = status;
    }
    
    // Null-terminate the output
    if (output) {
        output[output_len] = '\0';
    }
    
    return output;
}

/**
 * Test a specific invalid flag combination.
 * Returns 1 if "unknown flag" error found, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag_combination) {
    char cmd[MAX_CMD_LEN];
    int status;
    
    // Build command: gcov-dump with invalid flags
    snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_path, flag_combination);
    
    printf("Testing: %s\n", cmd);
    
    char *stderr_output = run_command_capture_stderr(cmd, &status);
    if (!stderr_output) {
        printf("  Failed to execute command\n");
        return 0;
    }
    
    // Check for "unknown flag" message
    int found = (strstr(stderr_output, "unknown flag") != NULL);
    
    if (found) {
        printf("  ✓ Found 'unknown flag' error\n");
    } else {
        printf("  ✗ No 'unknown flag' error found\n");
        printf("    stderr output: %s\n", stderr_output[0] ? stderr_output : "(empty)");
    }
    
    free(stderr_output);
    return found;
}

int main(void) {
    char *gcov_dump_path = find_gcov_dump();
    if (!gcov_dump_path) {
        fprintf(stderr, "Error: gcov-dump executable not found\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test cases designed to trigger the default case
    const char *test_cases[] = {
        // Single invalid flags
        "-x",
        "-z",
        "-?",
        "-X",
        "-1",
        
        // Invalid flag in different positions
        "-x -l",           // Invalid before valid
        "-l -x",           // Invalid after valid
        "-l -x -p",        // Invalid between valid flags
        "-x -l -p -r",     // Multiple invalid/valid mix
        
        // Edge cases for getopt parsing
        "-x --",           // Invalid before double dash
        "-- -x",           // Invalid after double dash (might not trigger default case)
        "-xyz",            // Combined invalid flags
        "-lpxz",           // Combined valid and invalid
        
        // With filename argument
        "-x dummy.gcda",
        "dummy.gcda -x",   // Invalid flag after filename
        "-l -x dummy.gcda",
        
        // Multiple invalid flags
        "-x -y -z",
        "-a -b -c",
        
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
    printf("========================================\n");
    printf("Test Summary: %d/%d tests passed\n", passed_tests, total_tests);
    
    if (passed_tests > 0) {
        printf("✓ Successfully triggered the uncovered default case!\n");
        return EXIT_SUCCESS;
    } else {
        printf("✗ Failed to trigger the uncovered default case\n");
        return EXIT_FAILURE;
    }
}
