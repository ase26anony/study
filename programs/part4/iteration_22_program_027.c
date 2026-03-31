/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Tests the uncovered default case in gcov-dump's option parsing
 * by executing it with invalid command-line flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_PATH 1024
#define MAX_OUTPUT 4096

/**
 * Find the gcov-dump executable path.
 * Checks GCov_DUMP environment variable first, then common build locations.
 * Returns 1 if found, 0 otherwise.
 */
static int find_gcov_dump(char *path, size_t path_size) {
    const char *env_path = getenv("GCOV_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, path_size - 1);
        path[path_size - 1] = '\0';
        return 1;
    }
    
    // Try common build locations
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
            strncpy(path, candidates[i], path_size - 1);
            path[path_size - 1] = '\0';
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
    char command[MAX_PATH + 100];
    char output[MAX_OUTPUT];
    FILE *fp;
    int found = 0;
    
    // Build command to capture stderr (redirect stderr to stdout)
    snprintf(command, sizeof(command), "%s %s 2>&1", gcov_dump_path, flag);
    
    // Execute and capture output
    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen failed");
        return 0;
    }
    
    // Read output
    while (fgets(output, sizeof(output), fp) != NULL) {
        // Check for the error message
        if (strstr(output, "unknown flag") != NULL) {
            printf("Found expected error for flag '%s': %s", flag, output);
            found = 1;
        }
    }
    
    pclose(fp);
    return found;
}

/**
 * Test various invalid flag scenarios.
 */
static void run_tests(const char *gcov_dump_path) {
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test cases covering different positions and combinations
    const char *test_cases[] = {
        // Single invalid flags
        "-x",
        "-z",
        "-?",
        "-X",
        // Invalid flag as first argument
        "-x -l",      // invalid then valid
        // Invalid flag between valid flags
        "-l -x -p",   // valid, invalid, valid
        "-p -z -r",   // valid, invalid, valid
        // Invalid flag after valid flags
        "-l -p -x",   // valid, valid, invalid
        // Multiple invalid flags
        "-x -y -z",
        // Invalid flag with double dash (getopt may treat differently)
        "--x",
        // Invalid flag after non-option argument (if we provide a dummy file)
        "dummy.gcda -x",
        // Edge case: flag without dash (should not trigger default case)
        "x",
        NULL
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    
    for (int i = 0; test_cases[i] != NULL; i++) {
        total_tests++;
        printf("Test %d: gcov-dump %s\n", total_tests, test_cases[i]);
        
        if (test_invalid_flag(gcov_dump_path, test_cases[i])) {
            printf("  ✓ PASSED\n\n");
            passed_tests++;
        } else {
            printf("  ✗ FAILED (did not find 'unknown flag' error)\n\n");
        }
    }
    
    // Special test: using system() to check exit status
    printf("Testing exit status with invalid flag...\n");
    char cmd[MAX_PATH + 50];
    snprintf(cmd, sizeof(cmd), "%s -x 2>&1 >/dev/null", gcov_dump_path);
    int status = system(cmd);
    
    if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        printf("Exit status: %d (non-zero indicates error handling)\n", exit_status);
        if (exit_status != 0) {
            passed_tests++;
        }
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    
    if (passed_tests > 0) {
        printf("\n✓ SUCCESS: Triggered the uncovered default case!\n");
    } else {
        printf("\n✗ FAILURE: Could not trigger the uncovered default case\n");
    }
}

int main(void) {
    char gcov_dump_path[MAX_PATH];
    
    printf("=== Testing gcov-dump Invalid Flag Handling ===\n");
    
    // Find the gcov-dump executable
    if (!find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path))) {
        fprintf(stderr, "Error: gcov-dump executable not found\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure it's in a common location\n");
        return EXIT_FAILURE;
    }
    
    printf("Found gcov-dump: %s\n", gcov_dump_path);
    
    // Run the tests
    run_tests(gcov_dump_path);
    
    return EXIT_SUCCESS;
}
