/**
 * test_gcov_dump_default_case.c
 * 
 * Tests the uncovered default case in gcov-dump.cc that handles unknown flags.
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
 * Checks GCov_DUMP environment variable first, then common build locations.
 * Returns 1 if found (path stored in buffer), 0 otherwise.
 */
static int find_gcov_dump(char *path_buf, size_t buf_size) {
    const char *env_path = getenv("GCov_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        strncpy(path_buf, env_path, buf_size - 1);
        path_buf[buf_size - 1] = '\0';
        return 1;
    }
    
    // Common build locations in GCC source tree
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path_buf, common_paths[i], buf_size - 1);
            path_buf[buf_size - 1] = '\0';
            return 1;
        }
    }
    
    return 0;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if "unknown flag" error is found in stderr, 0 otherwise.
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
    
    // Check for "unknown flag" message (case-insensitive)
    if (strstr(output, "unknown flag") != NULL ||
        strstr(output, "unknown flag `") != NULL) {
        printf("✓ Found 'unknown flag' error for flag %s\n", flag);
        printf("  Output: %s", output);
        found = 1;
    } else {
        printf("✗ No 'unknown flag' error for flag %s\n", flag);
        printf("  Output: %s", output);
    }
    
    // Also check exit status (should be non-zero for error)
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        printf("  Exit status: %d (non-zero as expected)\n", WEXITSTATUS(status));
    }
    
    return found;
}

int main(int argc, char *argv[]) {
    char gcov_dump_path[MAX_CMD_LEN];
    
    printf("=== Testing gcov-dump unknown flag handling ===\n");
    
    // Find gcov-dump executable
    if (!find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path))) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in a common location.\n");
        return 1;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test various invalid flags and edge cases
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
        // Edge cases
        "--x",           // Double dash with single char (getopt may treat as --x argument)
        "- ",            // Space after dash
        "-",             // Just a dash (might be interpreted as stdin)
        // With filename argument
        "-x dummy.gcda",
        "dummy.gcda -x", // Invalid flag after filename
        "-l -x dummy.gcda",
        NULL
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    
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
    
    // Consider it a success if at least one test triggered the default case
    if (passed_tests > 0) {
        printf("\nSUCCESS: Triggered the uncovered default case!\n");
        return 0;
    } else {
        printf("\nFAILURE: Could not trigger the uncovered default case.\n");
        return 1;
    }
}
