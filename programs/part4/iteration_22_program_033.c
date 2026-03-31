/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by executing
 * gcov-dump with invalid command-line flags and verifying the
 * "unknown flag" error message is printed to stderr.
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
    if (env_path && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, path_len - 1);
        path[path_len - 1] = '\0';
        return 1;
    }
    
    /* Common build locations in GCC source tree */
    const char *candidates[] = {
        "./gcc/gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "./gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "../../prev-gcc/build/gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; candidates[i] != NULL; i++) {
        if (access(candidates[i], X_OK) == 0) {
            strncpy(path, candidates[i], path_len - 1);
            path[path_len - 1] = '\0';
            return 1;
        }
    }
    
    return 0;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if "unknown flag" found in stderr, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
    char cmd[MAX_CMD_LEN];
    char output[MAX_OUTPUT_LEN] = {0};
    FILE *fp;
    int found = 0;
    
    /* Construct command to capture stderr (2>&1) */
    snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, flag);
    
    /* Execute command and read output */
    fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return 0;
    }
    
    /* Read all output */
    size_t total_read = 0;
    while (fgets(output + total_read, sizeof(output) - total_read - 1, fp) != NULL) {
        total_read = strlen(output);
        if (total_read >= sizeof(output) - 1) {
            break;
        }
    }
    
    int status = pclose(fp);
    
    /* Check for "unknown flag" message */
    if (strstr(output, "unknown flag") != NULL) {
        printf("✓ Found 'unknown flag' in output for flag '%s'\n", flag);
        printf("  Output: %s", output);
        found = 1;
    } else {
        printf("✗ No 'unknown flag' in output for flag '%s'\n", flag);
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
    
    /* Find gcov-dump executable */
    if (!find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path))) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure it's in a common location.\n");
        return 1;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    /* Test various invalid flags and edge cases */
    const char *test_cases[] = {
        /* Single invalid flags */
        "-x",
        "-z",
        "-?",
        "-X",
        "-1",
        
        /* Invalid flag in different positions */
        "-x -l",           /* Invalid before valid */
        "-l -x",           /* Invalid after valid */
        "-l -x -p",        /* Invalid between valid flags */
        "-x -y -z",        /* Multiple invalid flags */
        
        /* With filename argument */
        "-x dummy.gcda",
        "dummy.gcda -x",   /* Invalid flag after filename */
        "-l -x dummy.gcda",
        
        /* Double dash edge cases */
        "--x",             /* getopt might treat as filename */
        "-l --x -p",
        
        /* Combined flags (if supported) */
        "-lxz",            /* Might parse as -l -x -z */
        
        /* Boundary cases */
        "-",               /* Just a dash */
        "--",              /* Just double dash */
        "- ",              /* Flag with space */
        
        NULL
    };
    
    /* Run all test cases */
    for (int i = 0; test_cases[i] != NULL; i++) {
        printf("Test %d: gcov-dump %s\n", i + 1, test_cases[i]);
        if (test_invalid_flag(gcov_dump_path, test_cases[i])) {
            tests_passed++;
        }
        total_tests++;
        printf("\n");
    }
    
    /* Summary */
    printf("=== Test Summary ===\n");
    printf("Passed: %d/%d tests\n", tests_passed, total_tests);
    
    if (tests_passed > 0) {
        printf("SUCCESS: Triggered the uncovered default case!\n");
        return 0;
    } else {
        printf("FAILURE: Could not trigger the uncovered default case.\n");
        return 1;
    }
}
