/**
 * test_gcov_dump_invalid_flags.c
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
 * Find the gcov-dump executable path.
 * Priority:
 * 1. GCov_DUMP environment variable
 * 2. Common build locations
 * 3. Fallback to "gcov-dump" (hoping it's in PATH)
 */
static const char *find_gcov_dump(void) {
    const char *env_path = getenv("GCov_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        return env_path;
    }
    
    // Common build locations in GCC source tree
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "../../prev-gcc/build/gcc/gcov-dump",
        "gcov-dump",  // Fallback to PATH
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            return common_paths[i];
        }
    }
    
    // Last resort
    return "gcov-dump";
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if "unknown flag" error is found, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
    char cmd[MAX_CMD_LEN];
    char output[MAX_OUTPUT_LEN] = {0};
    FILE *fp;
    int found = 0;
    
    // Build command: redirect stderr to stdout for capture
    snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, flag);
    
    // Execute and capture output
    fp = popen(cmd, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to execute command: %s\n", cmd);
        return 0;
    }
    
    // Read output
    size_t total_read = 0;
    while (fgets(output + total_read, sizeof(output) - total_read, fp) != NULL) {
        total_read = strlen(output);
        if (total_read >= sizeof(output) - 1) {
            break;
        }
    }
    
    int status = pclose(fp);
    
    // Check for the error message
    if (strstr(output, "unknown flag") != NULL) {
        printf("✓ Found 'unknown flag' error for flag '%s'\n", flag);
        printf("  Output: %s", output);
        found = 1;
    } else {
        printf("✗ No 'unknown flag' error for flag '%s'\n", flag);
        printf("  Output: %s", output);
        printf("  Exit status: %d\n", WEXITSTATUS(status));
    }
    
    return found;
}

int main(void) {
    const char *gcov_dump_path = find_gcov_dump();
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test various invalid flags and edge cases
    const char *test_cases[] = {
        // Single invalid flags
        "-x",
        "-z",
        "-?",
        "-X",
        // Invalid flags in different positions
        "-x -l",           // Invalid before valid
        "-l -x -p",        // Invalid between valid flags
        "-l -p -x",        // Invalid after valid flags
        "test.gcda -x",    // Invalid after filename
        "-x test.gcda",    // Invalid before filename
        // Edge cases with double dash
        "--x",
        "-- -x",           // Double dash then invalid flag
        // Multiple invalid flags
        "-x -y -z",
        // Mixed case
        "-l -X -p -Z",
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
    
    // Summary
    printf("========================================\n");
    printf("Test Summary:\n");
    printf("  Total tests: %d\n", total_tests);
    printf("  Passed: %d\n", passed_tests);
    printf("  Failed: %d\n", total_tests - passed_tests);
    
    // Return success if at least one test found the error
    return (passed_tests > 0) ? 0 : 1;
}
