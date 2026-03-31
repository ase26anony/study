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

#define MAX_CMD_LEN 1024
#define MAX_OUTPUT_LEN 4096

/**
 * Find the gcov-dump executable path.
 * Checks GCov_DUMP environment variable first, then common build locations.
 * Returns 1 if found, 0 otherwise.
 */
static int find_gcov_dump(char *path, size_t path_len) {
    const char *env_path = getenv("GCOV_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        snprintf(path, path_len, "%s", env_path);
        return 1;
    }
    
    // Common build locations in GCC source tree
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
 * Returns 1 if "unknown flag" appears in stderr, 0 otherwise.
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
            break;
        }
    }
    
    pclose(fp);
    
    // Check for "unknown flag" message
    if (strstr(output, "unknown flag") != NULL) {
        printf("SUCCESS: Found 'unknown flag' in output for flag %s\n", flag);
        printf("Output: %s\n", output);
        found = 1;
    } else {
        printf("FAILURE: 'unknown flag' not found for flag %s\n", flag);
        printf("Output: %s\n", output);
    }
    
    return found;
}

int main(int argc, char *argv[]) {
    char gcov_dump_path[MAX_CMD_LEN];
    
    printf("=== Testing gcov-dump default case (unknown flags) ===\n");
    
    // Find gcov-dump executable
    if (!find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path))) {
        fprintf(stderr, "ERROR: gcov-dump executable not found\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in common locations\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test various invalid flags and combinations
    const char *test_cases[] = {
        // Single invalid flags
        "-x",
        "-z",
        "-?",
        "-X",
        // Invalid flag in different positions
        "-x -l",           // Invalid before valid
        "-l -x",           // Invalid after valid  
        "-p -x -r",        // Invalid between valid
        "-x dummy.gcda",   // Invalid before filename
        "dummy.gcda -x",   // Invalid after filename (getopt stops at first non-option)
        // Double dash with invalid single char (treated as unknown option)
        "--x",
        // Multiple invalid flags
        "-x -y -z",
        // Combined valid and invalid
        "-l -x -p -y -s",
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
    
    if (passed_tests > 0) {
        printf("\nSUCCESS: Triggered the uncovered default case!\n");
        return EXIT_SUCCESS;
    } else {
        printf("\nFAILURE: Could not trigger the default case\n");
        return EXIT_FAILURE;
    }
}
