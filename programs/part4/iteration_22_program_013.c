/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Tests the uncovered default case in gcov-dump's option parsing
 * by executing it with invalid command-line flags.
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
 * Checks GCov_DUMP environment variable first, then common build locations.
 */
static const char *find_gcov_dump(void) {
    const char *env_path = getenv("GCOV_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        return env_path;
    }
    
    // Common build locations in GCC source tree
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
            return common_paths[i];
        }
    }
    
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if target error message found, 0 if not, -1 on error.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *invalid_flag) {
    int pipefd[2];
    pid_t pid;
    char output[MAX_OUTPUT_LEN] = {0};
    int found_target = 0;
    
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
    
    if (pid == 0) {  // Child process
        // Redirect stderr to pipe
        close(pipefd[0]);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        // Execute gcov-dump with invalid flag
        execl(gcov_dump_path, "gcov-dump", invalid_flag, NULL);
        
        // If execl fails
        perror("execl");
        exit(EXIT_FAILURE);
    } else {  // Parent process
        int status;
        
        // Close write end of pipe
        close(pipefd[1]);
        
        // Read stderr output from pipe
        ssize_t bytes_read = read(pipefd[0], output, sizeof(output) - 1);
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
            
            // Check for target error message
            if (strstr(output, TARGET_ERROR_MSG) != NULL) {
                found_target = 1;
                printf("Found target error message for flag '%s':\n", invalid_flag);
                printf("%s\n", output);
            }
        }
        
        close(pipefd[0]);
        
        // Wait for child to finish
        waitpid(pid, &status, 0);
        
        return found_target;
    }
}

/**
 * Test multiple invalid flags in different positions.
 */
static int test_multiple_invalid_flags(const char *gcov_dump_path) {
    const char *test_cases[] = {
        // Single invalid flags
        "-x",
        "-z",
        "-?",
        "-X",
        
        // Invalid flag as first argument
        "-x -l -p",  // Invalid, then valid flags
        
        // Invalid flag between valid flags
        "-l -x -p",
        "-p -z -r",
        
        // Multiple invalid flags
        "-x -y -z",
        
        // Invalid flag after double dash (getopt may treat differently)
        "-- -x",
        
        // Invalid flag with filename argument
        "-x dummy.gcda",
        "dummy.gcda -z",  // Invalid flag after filename
        
        NULL
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("Testing invalid flags for gcov-dump at: %s\n\n", gcov_dump_path);
    
    for (int i = 0; test_cases[i] != NULL; i++) {
        printf("Test %d: Running 'gcov-dump %s'\n", total_tests + 1, test_cases[i]);
        
        // Build command with popen for simplicity
        char cmd[MAX_CMD_LEN];
        snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, test_cases[i]);
        
        FILE *fp = popen(cmd, "r");
        if (fp == NULL) {
            printf("  Failed to execute command\n");
            continue;
        }
        
        char output[MAX_OUTPUT_LEN] = {0};
        size_t bytes_read = fread(output, 1, sizeof(output) - 1, fp);
        int status = pclose(fp);
        
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
            
            if (strstr(output, TARGET_ERROR_MSG) != NULL) {
                printf("  ✓ Found target error message\n");
                passed_tests++;
            } else {
                printf("  ✗ Target error message not found\n");
                if (strlen(output) > 0) {
                    printf("    Output: %s", output);
                }
            }
        } else {
            printf("  ✗ No output received\n");
        }
        
        total_tests++;
        printf("\n");
    }
    
    printf("Summary: %d/%d tests passed\n", passed_tests, total_tests);
    return (passed_tests > 0) ? 0 : 1;
}

int main(void) {
    const char *gcov_dump_path = find_gcov_dump();
    
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: gcov-dump executable not found\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in a common location\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n", gcov_dump_path);
    
    // Test with a simple invalid flag first
    printf("\n=== Testing single invalid flag ===\n");
    int result = test_invalid_flag(gcov_dump_path, "-x");
    
    if (result == 1) {
        printf("Successfully triggered the uncovered default case!\n");
    } else if (result == 0) {
        printf("Failed to trigger the uncovered default case\n");
    } else {
        printf("Error executing test\n");
    }
    
    // Test multiple flag combinations
    printf("\n=== Testing multiple invalid flag combinations ===\n");
    int multi_result = test_multiple_invalid_flags(gcov_dump_path);
    
    return (result == 1 || multi_result == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
