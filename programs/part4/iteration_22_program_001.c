/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Tests the uncovered default case in gcov-dump's option parsing
 * by invoking it with invalid command-line flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_CMD_LEN 1024
#define MAX_OUTPUT_LEN 4096

/**
 * Find the gcov-dump executable path.
 * Checks GCov_DUMP environment variable first, then common locations.
 * Returns 1 if found, 0 otherwise.
 */
static int find_gcov_dump(char *path, size_t path_len) {
    const char *env_path = getenv("GCOV_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        snprintf(path, path_len, "%s", env_path);
        return 1;
    }
    
    // Common locations in GCC build trees
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            snprintf(path, path_len, "%s", common_paths[i]);
            return 1;
        }
    }
    
    return 0;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if "unknown flag" message found in stderr, 0 otherwise.
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
        perror("popen failed");
        return 0;
    }
    
    // Read output
    size_t total_read = 0;
    while (fgets(output + total_read, sizeof(output) - total_read, fp) != NULL) {
        total_read = strlen(output);
        if (total_read >= sizeof(output) - 1) {
            break;  // Buffer full
        }
    }
    
    int status = pclose(fp);
    
    // Check for the error message
    if (strstr(output, "unknown flag") != NULL) {
        printf("✓ Found 'unknown flag' message for flag '%s'\n", flag);
        printf("  Output: %s", output);
        found = 1;
    } else {
        printf("✗ No 'unknown flag' message for flag '%s'\n", flag);
        printf("  Output: %s", output);
        printf("  Exit status: %d\n", WEXITSTATUS(status));
    }
    
    return found;
}

int main(void) {
    char gcov_dump_path[MAX_CMD_LEN];
    int tests_passed = 0;
    int total_tests = 0;
    
    printf("=== Testing gcov-dump invalid flag handling ===\n");
    
    // Find gcov-dump executable
    if (!find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path))) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in PATH.\n");
        return 1;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test cases covering different scenarios
    const char *test_cases[] = {
        // Single invalid flags
        "-x",
        "-z",
        "-?",
        "-X",
        // Invalid flag as first argument
        "-x -l",           // Invalid then valid
        // Invalid flag between valid flags
        "-l -x -p",        // Valid, invalid, valid
        "-p -z -s",        // Valid, invalid, valid
        // Invalid flag after valid flags
        "-l -p -x",        // Valid, valid, invalid
        // Multiple invalid flags
        "-x -y -z",
        // Invalid flag with double dash (getopt may treat differently)
        "--x",
        // Invalid flag after non-option argument (requires a dummy file)
        "dummy.gcda -x",
        // Edge case: dash alone
        "-",
        NULL
    };
    
    // Create a dummy file for tests that need a filename argument
    FILE *dummy = fopen("dummy.gcda", "w");
    if (dummy) {
        fprintf(dummy, "dummy content\n");
        fclose(dummy);
    }
    
    // Run all test cases
    for (int i = 0; test_cases[i] != NULL; i++) {
        printf("Test %d: gcov-dump %s\n", total_tests + 1, test_cases[i]);
        if (test_invalid_flag(gcov_dump_path, test_cases[i])) {
            tests_passed++;
        }
        total_tests++;
        printf("\n");
    }
    
    // Clean up dummy file
    remove("dummy.gcda");
    
    // Summary
    printf("=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    
    if (tests_passed > 0) {
        printf("SUCCESS: Triggered the uncovered default case!\n");
        return 0;
    } else {
        printf("FAILURE: Could not trigger the uncovered default case.\n");
        return 1;
    }
}
