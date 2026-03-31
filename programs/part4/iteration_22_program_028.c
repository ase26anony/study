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
#include <errno.h>

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
    
    // Try common build locations
    const char *candidates[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; candidates[i] != NULL; i++) {
        if (access(candidates[i], X_OK) == 0) {
            snprintf(path, path_len, "%s", candidates[i]);
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
    
    // Construct command to capture stderr (redirect stderr to stdout)
    snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, flag);
    
    printf("Testing: %s %s\n", gcov_dump_path, flag);
    
    // Execute command and capture output
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
    
    int status = pclose(fp);
    
    // Check for the target error message
    if (strstr(output, "unknown flag") != NULL) {
        printf("  ✓ Found 'unknown flag' in output\n");
        found = 1;
    } else {
        printf("  ✗ 'unknown flag' not found in output\n");
        printf("    Output was: %s\n", output);
    }
    
    // Also check exit status (should be non-zero for error)
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        printf("  ✓ Program exited with non-zero status (%d)\n", WEXITSTATUS(status));
    } else {
        printf("  Note: Program exit status was %d\n", status);
    }
    
    return found;
}

int main(void) {
    char gcov_dump_path[MAX_CMD_LEN];
    int tests_passed = 0;
    int total_tests = 0;
    
    printf("=== Testing gcov-dump invalid flag handling ===\n\n");
    
    // Find gcov-dump executable
    if (!find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path))) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in a standard location\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test 1: Single invalid flag at beginning
    printf("Test 1: Single invalid flag at beginning\n");
    if (test_invalid_flag(gcov_dump_path, "-x")) {
        tests_passed++;
    }
    total_tests++;
    printf("\n");
    
    // Test 2: Single invalid flag (different character)
    printf("Test 2: Single invalid flag (different character)\n");
    if (test_invalid_flag(gcov_dump_path, "-z")) {
        tests_passed++;
    }
    total_tests++;
    printf("\n");
    
    // Test 3: Invalid flag between valid flags
    printf("Test 3: Invalid flag between valid flags\n");
    if (test_invalid_flag(gcov_dump_path, "-l -x -p")) {
        tests_passed++;
    }
    total_tests++;
    printf("\n");
    
    // Test 4: Multiple invalid flags
    printf("Test 4: Multiple invalid flags\n");
    if (test_invalid_flag(gcov_dump_path, "-a -b -c")) {
        tests_passed++;
    }
    total_tests++;
    printf("\n");
    
    // Test 5: Invalid flag after non-option argument (test.gcda)
    printf("Test 5: Invalid flag after filename argument\n");
    if (test_invalid_flag(gcov_dump_path, "test.gcda -x")) {
        tests_passed++;
    }
    total_tests++;
    printf("\n");
    
    // Test 6: Double dash with invalid single character flag
    printf("Test 6: Double dash with invalid single character\n");
    if (test_invalid_flag(gcov_dump_path, "--x")) {
        tests_passed++;
    }
    total_tests++;
    printf("\n");
    
    // Test 7: Question mark (special character)
    printf("Test 7: Question mark flag\n");
    if (test_invalid_flag(gcov_dump_path, "-?")) {
        tests_passed++;
    }
    total_tests++;
    printf("\n");
    
    // Test 8: Combination with valid flag and invalid flag
    printf("Test 8: Valid flag followed by invalid flag\n");
    if (test_invalid_flag(gcov_dump_path, "-v -x")) {
        tests_passed++;
    }
    total_tests++;
    printf("\n");
    
    // Summary
    printf("=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    
    if (tests_passed > 0) {
        printf("\nSuccessfully triggered the uncovered default case!\n");
        return EXIT_SUCCESS;
    } else {
        printf("\nFailed to trigger the uncovered default case.\n");
        return EXIT_FAILURE;
    }
}
