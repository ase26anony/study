/**
 * test_gcov_dump_default_case.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by providing invalid
 * command-line flags and verifying the error message.
 * 
 * Compile with: gcc -std=c99 -O0 -g -o test_gcov_dump test_gcov_dump_default_case.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

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
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "../../prev-gcc/build/gcc/gcov-dump",
        "gcov-dump",  // Try PATH as last resort
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path, common_paths[i], MAX_CMD_LEN - 1);
            path[MAX_CMD_LEN - 1] = '\0';
            return path;
        }
    }
    
    // 3. Search in PATH
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
 * Returns 1 if target error message found, 0 if not, -1 on execution error.
 */
static int test_gcov_dump_with_args(const char *gcov_dump_path, const char *args) {
    char command[MAX_CMD_LEN];
    char output[MAX_OUTPUT_LEN] = {0};
    FILE *fp;
    int found = 0;
    
    // Construct command to capture stderr
    snprintf(command, MAX_CMD_LEN, "%s %s 2>&1", gcov_dump_path, args);
    
    // Execute command and capture output
    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen failed");
        return -1;
    }
    
    // Read output
    size_t bytes_read = fread(output, 1, MAX_OUTPUT_LEN - 1, fp);
    output[bytes_read] = '\0';
    
    // Check for target error message
    if (strstr(output, TARGET_ERROR_MSG) != NULL) {
        found = 1;
        printf("Found target error message in output:\n%s\n", output);
    }
    
    // Get exit status
    int status = pclose(fp);
    
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        printf("Exit code: %d\n", exit_code);
        
        // gcov-dump should exit with non-zero for invalid flags
        if (exit_code != 0) {
            printf("(Expected non-zero exit code for invalid flags)\n");
        }
    }
    
    return found;
}

int main(void) {
    printf("=== Testing gcov-dump default case (invalid flags) ===\n");
    
    // Find gcov-dump executable
    char *gcov_dump_path = find_gcov_dump_path();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH.\n");
        return 1;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test cases for invalid flags
    const char *test_cases[] = {
        // Single invalid flags
        "-x",
        "-z",
        "-?",
        "-X",
        
        // Invalid flag in different positions
        "-x -l",           // Invalid before valid
        "-l -x",           // Invalid after valid
        "-p -x -r",        // Invalid between valid flags
        "-l -x -p -z -r",  // Multiple invalid flags
        
        // Edge cases with getopt
        "-x file.gcda",    // Invalid flag before filename
        "file.gcda -x",    // Invalid flag after filename (getopt stops at first non-option)
        "--x",             // Double dash with single char (treated as --x option)
        "-",               // Just a dash (treated as stdin filename by getopt)
        
        // Combined valid and invalid
        "-lpsx",           // Combined flags with invalid at end
        "-xlps",           // Combined flags with invalid at start
        
        NULL
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Run all test cases
    for (int i = 0; test_cases[i] != NULL; i++) {
        printf("Test %d: gcov-dump %s\n", total_tests + 1, test_cases[i]);
        
        int result = test_gcov_dump_with_args(gcov_dump_path, test_cases[i]);
        
        if (result == 1) {
            printf("✓ PASS: Triggered default case\n");
            passed_tests++;
        } else if (result == 0) {
            printf("✗ FAIL: Did not trigger default case\n");
        } else {
            printf("✗ ERROR: Execution failed\n");
        }
        
        printf("\n");
        total_tests++;
    }
    
    // Summary
    printf("=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", total_tests - passed_tests);
    
    if (passed_tests > 0) {
        printf("\nSuccessfully triggered the uncovered default case!\n");
        return 0;
    } else {
        printf("\nFailed to trigger the uncovered default case.\n");
        return 1;
    }
}
