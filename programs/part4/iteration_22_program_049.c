/**
 * test_gcov_dump_default_case.c
 * 
 * Tests the uncovered default case in gcov-dump.cc that handles
 * unknown command-line flags.
 * 
 * Compile with: gcc -std=c99 -O0 -g -o test_gcov_dump test_gcov_dump_default_case.c
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
 * Priority: 1. GCov_DUMP environment variable
 *           2. Common build locations
 *           3. System PATH
 */
static char *find_gcov_dump(void) {
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
 * Returns 1 if "unknown flag" message found, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
    char command[MAX_CMD_LEN];
    char output[MAX_OUTPUT_LEN] = {0};
    FILE *fp;
    int found = 0;
    
    // Build command: redirect stderr to stdout for capture
    snprintf(command, MAX_CMD_LEN, "%s %s 2>&1", gcov_dump_path, flag);
    
    // Execute and capture output
    fp = popen(command, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to execute command: %s\n", command);
        return 0;
    }
    
    // Read output
    size_t total_read = 0;
    while (fgets(output + total_read, MAX_OUTPUT_LEN - total_read, fp) != NULL) {
        total_read = strlen(output);
        if (total_read >= MAX_OUTPUT_LEN - 1) {
            break;
        }
    }
    
    int status = pclose(fp);
    
    // Check for the error message
    if (strstr(output, "unknown flag") != NULL) {
        printf("✓ Found 'unknown flag' message for flag %s\n", flag);
        printf("  Output: %s", output);
        found = 1;
    } else {
        printf("✗ No 'unknown flag' message for flag %s\n", flag);
        printf("  Output: %s", output);
        printf("  Exit status: %d\n", WEXITSTATUS(status));
    }
    
    return found;
}

int main(void) {
    printf("=== Testing gcov-dump unknown flag handling ===\n");
    
    // Find gcov-dump executable
    char *gcov_dump_path = find_gcov_dump();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH.\n");
        return 1;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test various invalid flags and positions
    const char *test_cases[] = {
        // Single invalid flags
        "-x",
        "-z",
        "-?",
        "-X",
        
        // Invalid flag in different positions
        "-x -l",           // Invalid first, valid second
        "-l -x",           // Valid first, invalid second
        "-p -x -r",        // Invalid in middle
        "-x -y -z",        // Multiple invalid
        
        // With double dash (getopt may handle differently)
        "--x",
        "-l --x -p",
        
        // Edge cases
        "-",              // Just a dash
        "- ",             // Dash with space
        "-lxpr",          // Combined flags with invalid
        "-lxpzr",         // Invalid in combined flags
        
        // After non-option argument (if we have a test file)
        // Note: gcov-dump requires a file argument, so we'll test this too
        "test.gcda -x",
        "-l test.gcda -x",
        
        NULL
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Test each case
    for (int i = 0; test_cases[i] != NULL; i++) {
        printf("Test %d: gcov-dump %s\n", total_tests + 1, test_cases[i]);
        
        if (test_invalid_flag(gcov_dump_path, test_cases[i])) {
            passed_tests++;
        }
        
        total_tests++;
        printf("\n");
    }
    
    // Summary
    printf("=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    
    if (passed_tests > 0) {
        printf("\n✅ Successfully triggered the uncovered default case!\n");
        return 0;
    } else {
        printf("\n❌ Failed to trigger the uncovered default case.\n");
        return 1;
    }
}
