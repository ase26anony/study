/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Tests the uncovered default case in gcov-dump's option parsing.
 * Executes gcov-dump with invalid command-line flags and verifies
 * the "unknown flag" error message is printed to stderr.
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
 * Checks GCov_DUMP environment variable first, then common build locations.
 * Returns 1 if found, 0 otherwise.
 */
static int find_gcov_dump(char *path, size_t path_len) {
    const char *env_path = getenv("GCOV_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        snprintf(path, path_len, "%s", env_path);
        return 1;
    }
    
    /* Try common build locations */
    const char *candidates[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "../../gcc/gcov-dump",
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
 * Returns 1 if target error message found in stderr, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
    char cmd[MAX_CMD_LEN];
    char output[MAX_OUTPUT_LEN] = {0};
    FILE *fp;
    int found = 0;
    
    /* Construct command: redirect stderr to stdout for capture */
    snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, flag);
    
    /* Execute and capture output */
    fp = popen(cmd, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to execute command: %s\n", cmd);
        return 0;
    }
    
    /* Read output */
    size_t total_read = 0;
    while (fgets(output + total_read, sizeof(output) - total_read, fp) != NULL) {
        total_read = strlen(output);
        if (total_read >= sizeof(output) - 1) {
            break;
        }
    }
    
    int status = pclose(fp);
    
    /* Check for target error message */
    if (strstr(output, TARGET_ERROR_MSG) != NULL) {
        printf("✓ Found target error for flag '%s':\n", flag);
        printf("  Output: %s", output);
        found = 1;
    } else {
        printf("✗ Target error NOT found for flag '%s'\n", flag);
        printf("  Output: %s", output);
        printf("  Exit status: %d\n", WEXITSTATUS(status));
    }
    
    return found;
}

int main(void) {
    char gcov_dump_path[MAX_CMD_LEN];
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("=== Testing gcov-dump invalid flag handling ===\n");
    
    /* Find gcov-dump executable */
    if (!find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path))) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure it's in a standard location.\n");
        return 1;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    /* Test various invalid flags in different positions */
    const char *test_cases[] = {
        /* Single invalid flags */
        "-x",
        "-z",
        "-?",
        "-@",
        
        /* Invalid flag as first argument */
        "-x -l",        /* Invalid then valid */
        "-x somefile.gcda",  /* Invalid then filename */
        
        /* Invalid flag between valid flags */
        "-l -x -p",
        "-p -z -r",
        
        /* Multiple invalid flags */
        "-x -y -z",
        
        /* Invalid flag after double dash (getopt may treat differently) */
        "-- -x",
        "-- -z",
        
        /* Invalid flag after valid flags */
        "-l -p -x",
        "-s -r -?",
        
        /* Edge cases */
        "-",            /* Just a dash */
        "- ",           /* Dash with space */
        "-x -l -p -z",  /* Mixed valid/invalid */
        
        NULL
    };
    
    /* Run all test cases */
    for (int i = 0; test_cases[i] != NULL; i++) {
        printf("Test %d: gcov-dump %s\n", total_tests + 1, test_cases[i]);
        if (test_invalid_flag(gcov_dump_path, test_cases[i])) {
            passed_tests++;
        }
        total_tests++;
        printf("\n");
    }
    
    /* Summary */
    printf("=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    
    if (passed_tests > 0) {
        printf("\n✅ Successfully triggered the uncovered default case!\n");
        return 0;
    } else {
        printf("\n❌ Failed to trigger the uncovered default case.\n");
        return 1;
    }
}
