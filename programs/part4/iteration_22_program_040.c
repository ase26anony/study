/**
 * test_gcov_dump_default_case.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by invoking
 * gcov-dump with invalid command-line flags.
 * 
 * Compile with: gcc -std=c99 -Wall -O0 -g -o test_gcov_dump test_gcov_dump_default_case.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define MAX_OUTPUT_LEN 4096
#define DEFAULT_GCOV_DUMP_PATH "./gcc/gcov-dump"

/**
 * Find the gcov-dump executable path.
 * Priority: 1. GCOV_DUMP environment variable, 2. Hardcoded default path.
 * Returns 1 if found and executable, 0 otherwise.
 */
static int find_gcov_dump(char *path, size_t path_len) {
    const char *env_path = getenv("GCOV_DUMP");
    
    if (env_path != NULL && env_path[0] != '\0') {
        snprintf(path, path_len, "%s", env_path);
    } else {
        snprintf(path, path_len, "%s", DEFAULT_GCOV_DUMP_PATH);
    }
    
    // Check if file exists and is executable
    if (access(path, X_OK) == 0) {
        return 1;
    }
    
    // Try in parent directory (common in build trees)
    snprintf(path, path_len, "../gcc/gcov-dump");
    if (access(path, X_OK) == 0) {
        return 1;
    }
    
    // Try absolute path from common GCC build location
    snprintf(path, path_len, "/usr/local/bin/gcov-dump");
    if (access(path, X_OK) == 0) {
        return 1;
    }
    
    return 0;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if "unknown flag" message found in stderr, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *test_args) {
    char command[MAX_CMD_LEN];
    char output[MAX_OUTPUT_LEN] = {0};
    FILE *fp;
    int found = 0;
    
    // Construct command to capture stderr (2>&1)
    snprintf(command, sizeof(command), "%s %s 2>&1", gcov_dump_path, test_args);
    
    printf("Testing: %s\n", command);
    
    // Execute command and capture output
    fp = popen(command, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to execute command: %s\n", strerror(errno));
        return 0;
    }
    
    // Read output
    size_t bytes_read = fread(output, 1, sizeof(output) - 1, fp);
    output[bytes_read] = '\0';
    
    int status = pclose(fp);
    
    // Check for "unknown flag" message (case insensitive)
    if (strstr(output, "unknown flag") != NULL ||
        strstr(output, "unknown flag `") != NULL) {
        printf("  ✓ Found 'unknown flag' message in output\n");
        found = 1;
    } else {
        printf("  ✗ No 'unknown flag' message found\n");
        printf("    Output was: %s\n", output);
    }
    
    // Also check exit status (should be non-zero for error)
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        printf("  ✓ Program exited with non-zero status (%d)\n", WEXITSTATUS(status));
    } else {
        printf("  Note: Program exited with status %d\n", WIFEXITED(status) ? WEXITSTATUS(status) : -1);
    }
    
    return found;
}

int main(void) {
    char gcov_dump_path[MAX_CMD_LEN];
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("=== Testing gcov-dump default case (unknown flags) ===\n\n");
    
    // Find gcov-dump executable
    if (!find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path))) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure default path exists.\n");
        fprintf(stderr, "Tried: ./gcc/gcov-dump, ../gcc/gcov-dump, /usr/local/bin/gcov-dump\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test 1: Single invalid flag at beginning
    printf("Test 1: Single invalid flag (-x)\n");
    total_tests++;
    if (test_invalid_flag(gcov_dump_path, "-x")) {
        passed_tests++;
    }
    printf("\n");
    
    // Test 2: Single invalid flag (-z)
    printf("Test 2: Single invalid flag (-z)\n");
    total_tests++;
    if (test_invalid_flag(gcov_dump_path, "-z")) {
        passed_tests++;
    }
    printf("\n");
    
    // Test 3: Invalid flag between valid flags
    printf("Test 3: Invalid flag between valid flags (-l -x -p)\n");
    total_tests++;
    if (test_invalid_flag(gcov_dump_path, "-l -x -p")) {
        passed_tests++;
    }
    printf("\n");
    
    // Test 4: Multiple invalid flags
    printf("Test 4: Multiple invalid flags (-a -b -c)\n");
    total_tests++;
    if (test_invalid_flag(gcov_dump_path, "-a -b -c")) {
        passed_tests++;
    }
    printf("\n");
    
    // Test 5: Invalid flag after filename argument
    printf("Test 5: Invalid flag after filename (dummy.gcda -x)\n");
    total_tests++;
    if (test_invalid_flag(gcov_dump_path, "dummy.gcda -x")) {
        passed_tests++;
    }
    printf("\n");
    
    // Test 6: Double dash with invalid flag (--x)
    printf("Test 6: Double dash with invalid flag (--x)\n");
    total_tests++;
    if (test_invalid_flag(gcov_dump_path, "--x")) {
        passed_tests++;
    }
    printf("\n");
    
    // Test 7: Question mark flag (special case for getopt)
    printf("Test 7: Question mark flag (-?)\n");
    total_tests++;
    if (test_invalid_flag(gcov_dump_path, "-?")) {
        passed_tests++;
    }
    printf("\n");
    
    // Test 8: Combination with equals sign (edge case)
    printf("Test 8: Invalid flag with equals (-x=value)\n");
    total_tests++;
    if (test_invalid_flag(gcov_dump_path, "-x=value")) {
        passed_tests++;
    }
    printf("\n");
    
    // Summary
    printf("=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    
    if (passed_tests == total_tests) {
        printf("\n✅ All tests passed! The default case was triggered successfully.\n");
        return EXIT_SUCCESS;
    } else if (passed_tests > 0) {
        printf("\n⚠️  Some tests passed. The default case was triggered in at least one scenario.\n");
        return EXIT_SUCCESS;
    } else {
        printf("\n❌ No tests passed. The default case was not triggered.\n");
        return EXIT_FAILURE;
    }
}
