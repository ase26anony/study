/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by invoking
 * gcov-dump with various invalid command-line flags.
 * 
 * Compile with: gcc -std=c99 -Wall -O0 -g -o test_gcov_dump test_gcov_dump_invalid_flags.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

/* Maximum path length for gcov-dump executable */
#define MAX_PATH_LEN 1024
#define MAX_CMD_LEN 2048
#define MAX_OUTPUT_LEN 4096

/* Test cases for invalid flags */
typedef struct {
    const char *description;
    const char *args;           /* Space-separated arguments */
    int expect_error;           /* Should trigger unknown flag error? */
} test_case_t;

/* Test cases array */
static test_case_t test_cases[] = {
    /* Single invalid flags */
    {"Single invalid flag -x", "-x", 1},
    {"Single invalid flag -z", "-z", 1},
    {"Single invalid flag -?", "-?", 1},
    {"Single invalid flag -X", "-X", 1},
    
    /* Invalid flag mixed with valid flags */
    {"Valid -l then invalid -x", "-l -x", 1},
    {"Invalid -x between valid -l and -p", "-l -x -p", 1},
    {"Multiple invalid flags", "-x -y -z", 1},
    
    /* Invalid flag after filename argument */
    {"Invalid flag after filename", "test.gcda -x", 1},
    {"Valid flags then filename then invalid flag", "-l -p test.gcda -x", 1},
    
    /* Double dash with invalid flag (getopt behavior test) */
    {"Double dash with invalid flag", "-- -x", 1},
    {"Double dash with invalid flag after valid", "-l -- -x", 1},
    
    /* Edge cases */
    {"Empty argument", "", 0},  /* Should show usage, not unknown flag */
    {"Only dash", "-", 0},      /* Might be interpreted as stdin */
    
    /* Valid flags only (negative test) */
    {"Valid flag -l only", "-l", 0},
    {"Valid flags -l -p", "-l -p", 0},
};

#define NUM_TEST_CASES (sizeof(test_cases) / sizeof(test_cases[0]))

/**
 * Find gcov-dump executable path.
 * Checks GCov_DUMP environment variable first, then common locations.
 */
static int find_gcov_dump(char *path, size_t path_len) {
    const char *env_path = getenv("GCOV_DUMP");
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    /* Try environment variable first */
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, path_len - 1);
        path[path_len - 1] = '\0';
        return 0;
    }
    
    /* Try common paths */
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path, common_paths[i], path_len - 1);
            path[path_len - 1] = '\0';
            return 0;
        }
    }
    
    return -1; /* Not found */
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 0 on success (found expected error), -1 on failure.
 */
static int run_test(const char *gcov_dump_path, const test_case_t *test, 
                   int *found_error) {
    char command[MAX_CMD_LEN];
    char output[MAX_OUTPUT_LEN] = {0};
    FILE *fp;
    int status;
    
    /* Build command: redirect stderr to stdout for capture */
    snprintf(command, sizeof(command), "%s %s 2>&1", gcov_dump_path, test->args);
    
    /* Execute command and capture output */
    fp = popen(command, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to execute command: %s\n", command);
        return -1;
    }
    
    /* Read output */
    size_t total_read = 0;
    while (fgets(output + total_read, sizeof(output) - total_read, fp) != NULL) {
        total_read = strlen(output);
        if (total_read >= sizeof(output) - 1) {
            break; /* Buffer full */
        }
    }
    
    /* Get exit status */
    status = pclose(fp);
    
    /* Check for the specific error message */
    *found_error = (strstr(output, "unknown flag") != NULL);
    
    /* Debug output */
    printf("Test: %s\n", test->description);
    printf("  Command: %s\n", command);
    printf("  Exit status: %d\n", WEXITSTATUS(status));
    printf("  Output (first 200 chars): %.200s\n", output);
    printf("  Found 'unknown flag': %s\n", *found_error ? "YES" : "NO");
    printf("  Expected error: %s\n", test->expect_error ? "YES" : "NO");
    printf("\n");
    
    return 0;
}

/**
 * Create a dummy .gcda file for testing with filename arguments.
 * gcov-dump requires a valid .gcda file to process.
 */
static int create_dummy_gcda_file(void) {
    FILE *fp = fopen("dummy.gcda", "wb");
    if (fp == NULL) {
        perror("Failed to create dummy.gcda");
        return -1;
    }
    
    /* Write minimal valid .gcda header (magic + version) */
    unsigned int magic = 0x67636461; /* 'gcda' in little-endian */
    unsigned int version = 0x4020000; /* Typical GCC version */
    
    fwrite(&magic, sizeof(magic), 1, fp);
    fwrite(&version, sizeof(version), 1, fp);
    
    fclose(fp);
    return 0;
}

int main(void) {
    char gcov_dump_path[MAX_PATH_LEN];
    int total_tests = 0;
    int passed_tests = 0;
    int failed_tests = 0;
    
    printf("=== Testing gcov-dump invalid flag handling ===\n\n");
    
    /* Find gcov-dump executable */
    if (find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path)) != 0) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in PATH.\n");
        fprintf(stderr, "Common locations checked: ./gcc/gcov-dump, ./gcov-dump, /usr/bin/gcov-dump\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    /* Create dummy .gcda file for tests that need a filename argument */
    if (create_dummy_gcda_file() != 0) {
        fprintf(stderr, "Warning: Failed to create dummy.gcda file\n");
        fprintf(stderr, "Tests with filename arguments may fail\n");
    }
    
    /* Run all test cases */
    for (size_t i = 0; i < NUM_TEST_CASES; i++) {
        int found_error;
        
        total_tests++;
        
        if (run_test(gcov_dump_path, &test_cases[i], &found_error) == 0) {
            if (found_error == test_cases[i].expect_error) {
                printf("  ✓ PASS\n");
                passed_tests++;
            } else {
                printf("  ✗ FAIL: Expected error=%d, got error=%d\n", 
                       test_cases[i].expect_error, found_error);
                failed_tests++;
            }
        } else {
            printf("  ✗ FAIL: Execution error\n");
            failed_tests++;
        }
        
        printf("\n");
    }
    
    /* Clean up */
    unlink("dummy.gcda");
    
    /* Summary */
    printf("=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", failed_tests);
    
    if (failed_tests > 0) {
        printf("\nSome tests failed. This might indicate:\n");
        printf("1. gcov-dump has different error handling\n");
        printf("2. The executable path is incorrect\n");
        printf("3. Permission issues\n");
        return EXIT_FAILURE;
    }
    
    printf("\nAll tests completed successfully!\n");
    return EXIT_SUCCESS;
}
