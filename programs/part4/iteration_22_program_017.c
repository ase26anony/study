/**
 * test_gcov_dump_default_case.c
 * 
 * This program tests the uncovered default case in gcov-dump.cc
 * by executing gcov-dump with invalid command-line flags.
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
 * Priority:
 * 1. GCov_DUMP environment variable
 * 2. Common build locations
 * 3. System PATH
 */
static char *find_gcov_dump_path(void) {
    static char path[MAX_CMD_LEN];
    
    // 1. Check environment variable
    char *env_path = getenv("GCov_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, MAX_CMD_LEN - 1);
        path[MAX_CMD_LEN - 1] = '\0';
        return path;
    }
    
    // 2. Check common build locations
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "../../prev-gcc/build/gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path, common_paths[i], MAX_CMD_LEN - 1);
            path[MAX_CMD_LEN - 1] = '\0';
            return path;
        }
    }
    
    // 3. Try to find in PATH
    char *path_env = getenv("PATH");
    if (path_env) {
        char *path_copy = strdup(path_env);
        char *dir = strtok(path_copy, ":");
        
        while (dir != NULL) {
            snprintf(path, MAX_CMD_LEN, "%s/gcov-dump", dir);
            if (access(path, X_OK) == 0) {
                free(path_copy);
                return path;
            }
            dir = strtok(NULL, ":");
        }
        free(path_copy);
    }
    
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if target error message found, 0 if not, -1 on error.
 */
static int test_gcov_dump_with_args(const char *gcov_dump_path, const char *args) {
    char command[MAX_CMD_LEN];
    char output[MAX_OUTPUT_LEN] = {0};
    FILE *fp;
    int found_target = 0;
    
    // Build command to capture stderr
    snprintf(command, MAX_CMD_LEN, "%s %s 2>&1", gcov_dump_path, args);
    
    // Execute command and capture output
    fp = popen(command, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to execute command: %s\n", command);
        return -1;
    }
    
    // Read output
    size_t bytes_read = fread(output, 1, MAX_OUTPUT_LEN - 1, fp);
    output[bytes_read] = '\0';
    
    // Check for target error message
    if (strstr(output, TARGET_ERROR_MSG) != NULL) {
        found_target = 1;
        printf("Found target message in output:\n%s\n", output);
    }
    
    int status = pclose(fp);
    
    // Check exit status (should be non-zero for invalid flags)
    if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        if (exit_status != 0) {
            printf("Exit status: %d (non-zero as expected)\n", exit_status);
        }
    }
    
    return found_target;
}

/**
 * Main test function.
 */
int main(void) {
    printf("=== Testing gcov-dump default case (unknown flags) ===\n");
    
    // Find gcov-dump executable
    char *gcov_dump_path = find_gcov_dump_path();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH.\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test cases for invalid flags
    const char *test_cases[] = {
        // Single invalid flags
        "-x",
        "-z",
        "-?",
        "-X",
        // Invalid flag combinations
        "-l -x -p",      // Invalid flag between valid ones
        "-x -l",         // Invalid flag first
        "-l -p -z",      // Invalid flag last
        "-x -y -z",      // Multiple invalid flags
        // With filename argument
        "-x test.gcda",
        "test.gcno -z",
        "-l -x test.gcda",
        // Double dash edge cases
        "--x",           // getopt might treat this as end of options
        "-l -- -x",      // Using -- to separate options from arguments
        // Empty argument (edge case)
        "",
        NULL
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    int failed_tests = 0;
    
    for (int i = 0; test_cases[i] != NULL; i++) {
        printf("Test %d: gcov-dump %s\n", total_tests + 1, test_cases[i]);
        
        int result = test_gcov_dump_with_args(gcov_dump_path, test_cases[i]);
        
        if (result == 1) {
            printf("Result: PASS - Target error message found\n\n");
            passed_tests++;
        } else if (result == 0) {
            printf("Result: FAIL - Target error message not found\n\n");
            failed_tests++;
        } else {
            printf("Result: ERROR - Command execution failed\n\n");
            failed_tests++;
        }
        
        total_tests++;
        
        // Small delay to avoid overwhelming the system
        usleep(10000);
    }
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", failed_tests);
    
    // Clean up any temporary files (none created in this test)
    
    if (passed_tests > 0) {
        printf("\nSUCCESS: Triggered the uncovered default case!\n");
        return EXIT_SUCCESS;
    } else {
        printf("\nFAILURE: Could not trigger the uncovered default case.\n");
        return EXIT_FAILURE;
    }
}
