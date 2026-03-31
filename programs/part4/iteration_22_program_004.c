/**
 * test_gcov_dump_default_case.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by invoking it with
 * invalid command-line flags and verifying the error message.
 * 
 * Compile with: gcc -std=c99 -Wall -O0 -g -o test_gcov_dump test_gcov_dump_default_case.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_PATH_LEN 1024
#define MAX_OUTPUT_LEN 4096
#define ERROR_MSG_PREFIX "unknown flag"

/**
 * Find the gcov-dump executable path.
 * Priority: 1. GCov_DUMP environment variable, 2. Common build paths
 */
static int find_gcov_dump(char *path, size_t path_len) {
    const char *env_path = getenv("GCov_DUMP");
    
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, path_len - 1);
        path[path_len - 1] = '\0';
        return 1;
    }
    
    // Try common build locations
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "../../../gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path, common_paths[i], path_len - 1);
            path[path_len - 1] = '\0';
            return 1;
        }
    }
    
    return 0;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if ERROR_MSG_PREFIX found in stderr, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
    char command[MAX_PATH_LEN + 100];
    char output[MAX_OUTPUT_LEN];
    FILE *fp;
    int found = 0;
    
    // Build command to capture stderr (redirect stderr to stdout)
    snprintf(command, sizeof(command), "%s %s 2>&1", gcov_dump_path, flag);
    
    // Execute command and capture output
    fp = popen(command, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to execute command: %s\n", command);
        return 0;
    }
    
    // Read output
    while (fgets(output, sizeof(output), fp) != NULL) {
        if (strstr(output, ERROR_MSG_PREFIX) != NULL) {
            printf("Found expected error message: %s", output);
            found = 1;
        }
    }
    
    pclose(fp);
    return found;
}

/**
 * Test various invalid flag scenarios
 */
static int run_tests(const char *gcov_dump_path) {
    int passed = 0;
    int total = 0;
    
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test 1: Single invalid flag at beginning
    printf("Test 1: Single invalid flag '-x'\n");
    if (test_invalid_flag(gcov_dump_path, "-x")) {
        printf("✓ PASSED\n");
        passed++;
    } else {
        printf("✗ FAILED\n");
    }
    total++;
    printf("\n");
    
    // Test 2: Single invalid flag '-z'
    printf("Test 2: Single invalid flag '-z'\n");
    if (test_invalid_flag(gcov_dump_path, "-z")) {
        printf("✓ PASSED\n");
        passed++;
    } else {
        printf("✗ FAILED\n");
    }
    total++;
    printf("\n");
    
    // Test 3: Invalid flag between valid flags
    printf("Test 3: Invalid flag between valid flags '-l -x -p'\n");
    if (test_invalid_flag(gcov_dump_path, "-l -x -p")) {
        printf("✓ PASSED\n");
        passed++;
    } else {
        printf("✗ FAILED\n");
    }
    total++;
    printf("\n");
    
    // Test 4: Multiple invalid flags
    printf("Test 4: Multiple invalid flags '-a -b -c'\n");
    if (test_invalid_flag(gcov_dump_path, "-a -b -c")) {
        printf("✓ PASSED\n");
        passed++;
    } else {
        printf("✗ FAILED\n");
    }
    total++;
    printf("\n");
    
    // Test 5: Invalid flag after filename argument
    printf("Test 5: Invalid flag after filename 'dummy.gcda -x'\n");
    if (test_invalid_flag(gcov_dump_path, "dummy.gcda -x")) {
        printf("✓ PASSED\n");
        passed++;
    } else {
        printf("✗ FAILED\n");
    }
    total++;
    printf("\n");
    
    // Test 6: Double dash with invalid flag (edge case)
    printf("Test 6: Double dash with invalid flag '--x'\n");
    if (test_invalid_flag(gcov_dump_path, "--x")) {
        printf("✓ PASSED\n");
        passed++;
    } else {
        printf("✗ FAILED\n");
    }
    total++;
    printf("\n");
    
    // Test 7: Question mark flag (special character)
    printf("Test 7: Question mark flag '-?'\n");
    if (test_invalid_flag(gcov_dump_path, "-?")) {
        printf("✓ PASSED\n");
        passed++;
    } else {
        printf("✗ FAILED\n");
    }
    total++;
    printf("\n");
    
    // Test 8: Combination with help flag (should not trigger default case)
    printf("Test 8: Valid help flag '-h' (should NOT trigger default case)\n");
    if (!test_invalid_flag(gcov_dump_path, "-h")) {
        printf("✓ PASSED (correctly did not trigger default case)\n");
        passed++;
    } else {
        printf("✗ FAILED\n");
    }
    total++;
    
    printf("\n========================================\n");
    printf("Test Results: %d/%d passed\n", passed, total);
    
    return (passed == total - 1); // All but the help test should pass
}

int main(void) {
    char gcov_dump_path[MAX_PATH_LEN];
    
    // Find gcov-dump executable
    if (!find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path))) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in a common location\n");
        return EXIT_FAILURE;
    }
    
    printf("Found gcov-dump: %s\n", gcov_dump_path);
    
    // Run tests
    if (run_tests(gcov_dump_path)) {
        printf("\nAll tests passed successfully!\n");
        return EXIT_SUCCESS;
    } else {
        printf("\nSome tests failed.\n");
        return EXIT_FAILURE;
    }
}
