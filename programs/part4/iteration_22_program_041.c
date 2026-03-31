/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by providing
 * invalid command-line flags and verifying the error message.
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
 * Priority: 1. GCov_DUMP environment variable
 *           2. Common build locations
 *           3. System PATH
 */
static char *find_gcov_dump_path(void) {
    static char path[MAX_CMD_LEN];
    
    // 1. Check environment variable
    char *env_path = getenv("GCov_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
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
    FILE *fp = popen("which gcov-dump 2>/dev/null", "r");
    if (fp != NULL) {
        if (fgets(path, MAX_CMD_LEN, fp) != NULL) {
            // Remove trailing newline
            path[strcspn(path, "\n")] = '\0';
            pclose(fp);
            if (access(path, X_OK) == 0) {
                return path;
            }
        }
        pclose(fp);
    }
    
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if target error message is found, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
    char command[MAX_CMD_LEN];
    char output[MAX_OUTPUT_LEN] = {0};
    FILE *fp;
    int found_target = 0;
    
    // Construct command to capture stderr
    snprintf(command, MAX_CMD_LEN, "%s %s 2>&1", gcov_dump_path, flag);
    
    printf("Testing: %s\n", command);
    
    // Execute command and capture output
    fp = popen(command, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to execute command: %s\n", strerror(errno));
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
    
    // Check if target error message is in output
    if (strstr(output, TARGET_ERROR_MSG) != NULL) {
        printf("  ✓ Found target error message\n");
        found_target = 1;
    } else {
        printf("  ✗ Target error message not found\n");
        printf("    Output: %s", output);
    }
    
    // Also check exit status (should be non-zero for error)
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        printf("  ✓ Program exited with non-zero status (%d)\n", WEXITSTATUS(status));
    } else {
        printf("  Note: Program exit status: %d\n", status);
    }
    
    return found_target;
}

/**
 * Test various invalid flag scenarios.
 */
static void run_all_tests(const char *gcov_dump_path) {
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("\n=== Testing gcov-dump invalid flags ===\n");
    printf("Using executable: %s\n\n", gcov_dump_path);
    
    // Test 1: Single invalid flag at beginning
    printf("Test 1: Single invalid flag at beginning\n");
    total_tests++;
    if (test_invalid_flag(gcov_dump_path, "-x")) {
        passed_tests++;
    }
    
    // Test 2: Single invalid flag with different character
    printf("\nTest 2: Different invalid flag\n");
    total_tests++;
    if (test_invalid_flag(gcov_dump_path, "-z")) {
        passed_tests++;
    }
    
    // Test 3: Invalid flag between valid flags
    printf("\nTest 3: Invalid flag between valid flags\n");
    total_tests++;
    if (test_invalid_flag(gcov_dump_path, "-l -x -p")) {
        passed_tests++;
    }
    
    // Test 4: Multiple invalid flags
    printf("\nTest 4: Multiple invalid flags\n");
    total_tests++;
    if (test_invalid_flag(gcov_dump_path, "-x -y -z")) {
        passed_tests++;
    }
    
    // Test 5: Invalid flag after filename argument
    printf("\nTest 5: Invalid flag after filename\n");
    total_tests++;
    if (test_invalid_flag(gcov_dump_path, "test.gcda -x")) {
        passed_tests++;
    }
    
    // Test 6: Double dash with invalid single char (edge case)
    printf("\nTest 6: Double dash with invalid flag\n");
    total_tests++;
    if (test_invalid_flag(gcov_dump_path, "--x")) {
        passed_tests++;
    }
    
    // Test 7: Question mark (special character for getopt)
    printf("\nTest 7: Question mark flag\n");
    total_tests++;
    if (test_invalid_flag(gcov_dump_path, "-?")) {
        passed_tests++;
    }
    
    // Test 8: Combination with help flag (should still trigger error)
    printf("\nTest 8: Valid help flag with invalid flag\n");
    total_tests++;
    if (test_invalid_flag(gcov_dump_path, "-h -x")) {
        passed_tests++;
    }
    
    printf("\n=== Test Results ===\n");
    printf("Passed: %d/%d tests\n", passed_tests, total_tests);
    
    if (passed_tests == total_tests) {
        printf("SUCCESS: All tests passed!\n");
    } else {
        printf("WARNING: Some tests failed\n");
    }
}

int main(void) {
    char *gcov_dump_path = find_gcov_dump_path();
    
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        fprintf(stderr, "Common locations checked:\n");
        fprintf(stderr, "  - ./gcc/gcov-dump\n");
        fprintf(stderr, "  - ./gcov-dump\n");
        fprintf(stderr, "  - /usr/bin/gcov-dump\n");
        fprintf(stderr, "  - PATH (via 'which gcov-dump')\n");
        return EXIT_FAILURE;
    }
    
    run_all_tests(gcov_dump_path);
    
    return EXIT_SUCCESS;
}
