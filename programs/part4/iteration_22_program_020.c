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

#define MAX_PATH_LEN 1024
#define MAX_OUTPUT_LEN 4096
#define ERROR_MSG_PREFIX "unknown flag"

/**
 * Find the gcov-dump executable path.
 * Priority: 1. GCov_DUMP environment variable, 2. Common build locations
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
        "../prev-gcc/build/gcc/gcov-dump",
        "../../gcc/gcov-dump",
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
    
    // Build command to capture stderr (2>&1)
    snprintf(command, sizeof(command), "%s %s 2>&1", gcov_dump_path, flag);
    
    // Execute command and capture output
    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen failed");
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
static void run_all_tests(const char *gcov_dump_path) {
    struct test_case {
        const char *description;
        const char *flag;
        int expect_error;
    };
    
    struct test_case tests[] = {
        {"Single invalid flag at start", "-x", 1},
        {"Single invalid flag with question mark", "-?", 1},
        {"Single invalid flag 'z'", "-z", 1},
        {"Invalid flag between valid flags", "-l -x -p", 1},
        {"Multiple invalid flags", "-a -b -c", 1},
        {"Invalid flag after valid flag", "-v -x", 1},
        {"Invalid flag before valid flag", "-x -v", 1},
        {"Double dash with single char (edge case)", "--x", 1},
        {"Mixed case invalid flag", "-X", 1},
        {"Valid flag only (should not trigger error)", "-v", 0},
        {"Help flag (should not trigger error)", "-h", 0},
        {"No flags (should not trigger error)", "", 0},
        {"Invalid flag after filename", "test.gcda -x", 1},
        {"Invalid flag before filename", "-x test.gcda", 1},
        {NULL, NULL, 0}
    };
    
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    int passed = 0;
    int total = 0;
    
    for (int i = 0; tests[i].description != NULL; i++) {
        printf("Test %d: %s\n", total + 1, tests[i].description);
        printf("  Command: gcov-dump %s\n", tests[i].flag);
        
        int result = test_invalid_flag(gcov_dump_path, tests[i].flag);
        
        if ((result && tests[i].expect_error) || (!result && !tests[i].expect_error)) {
            printf("  ✓ PASS\n");
            passed++;
        } else {
            printf("  ✗ FAIL - Expected error: %s, Got error: %s\n",
                   tests[i].expect_error ? "YES" : "NO",
                   result ? "YES" : "NO");
        }
        
        printf("\n");
        total++;
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, 
           total > 0 ? (passed * 100.0 / total) : 0.0);
    
    if (passed == total) {
        printf("All tests passed! The default case was successfully triggered.\n");
    } else {
        printf("Some tests failed. Check the implementation.\n");
    }
}

int main(int argc, char *argv[]) {
    char gcov_dump_path[MAX_PATH_LEN];
    
    printf("=== Testing gcov-dump default case (unknown flags) ===\n");
    
    // Find gcov-dump executable
    if (!find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path))) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in a common location.\n");
        fprintf(stderr, "Common locations checked:\n");
        fprintf(stderr, "  - ./gcc/gcov-dump\n");
        fprintf(stderr, "  - ./gcov-dump\n");
        fprintf(stderr, "  - ../gcc/gcov-dump\n");
        fprintf(stderr, "  - ../prev-gcc/build/gcc/gcov-dump\n");
        fprintf(stderr, "  - /usr/bin/gcov-dump\n");
        fprintf(stderr, "  - /usr/local/bin/gcov-dump\n");
        return EXIT_FAILURE;
    }
    
    printf("Found gcov-dump at: %s\n", gcov_dump_path);
    
    // Run comprehensive tests
    run_all_tests(gcov_dump_path);
    
    // Additional focused test to ensure the exact uncovered lines are hit
    printf("\n=== Focused Test for Uncovered Lines ===\n");
    
    // Test with multiple invalid flags to ensure we hit the fprintf line
    const char *invalid_flags[] = {"-x", "-z", "-?", "-X", "-Z", NULL};
    int triggered_count = 0;
    
    for (int i = 0; invalid_flags[i] != NULL; i++) {
        printf("Testing flag %s: ", invalid_flags[i]);
        if (test_invalid_flag(gcov_dump_path, invalid_flags[i])) {
            printf("Triggered default case\n");
            triggered_count++;
        } else {
            printf("Did NOT trigger default case\n");
        }
    }
    
    if (triggered_count > 0) {
        printf("\nSuccessfully triggered the uncovered default case %d times!\n", triggered_count);
        return EXIT_SUCCESS;
    } else {
        printf("\nFailed to trigger the uncovered default case.\n");
        return EXIT_FAILURE;
    }
}
