/**
 * test_gcov_dump_default_case.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by invoking it with
 * invalid command-line flags and verifying the error message.
 * 
 * Compile with: gcc -std=c99 -O0 -g -o test_gcov_dump test_gcov_dump_default_case.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define MAX_OUTPUT_LEN 4096

/**
 * Find the gcov-dump executable path.
 * Priority: 1. GCov_DUMP environment variable
 *           2. Common build locations
 * Returns dynamically allocated string or NULL if not found.
 */
char *find_gcov_dump_path() {
    char *path = NULL;
    
    // Check environment variable first
    path = getenv("GCov_DUMP");
    if (path != NULL && access(path, X_OK) == 0) {
        return strdup(path);
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
            return strdup(common_paths[i]);
        }
    }
    
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if "unknown flag" error is found, 0 otherwise.
 */
int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
    char command[MAX_CMD_LEN];
    FILE *fp;
    char output[MAX_OUTPUT_LEN];
    int found_error = 0;
    
    // Build command to capture stderr
    snprintf(command, sizeof(command), "%s %s 2>&1", gcov_dump_path, flag);
    
    printf("Testing: %s\n", command);
    
    // Execute command and capture output
    fp = popen(command, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to execute command: %s\n", strerror(errno));
        return 0;
    }
    
    // Read output
    while (fgets(output, sizeof(output), fp) != NULL) {
        // Check for the error message
        if (strstr(output, "unknown flag") != NULL) {
            printf("  ✓ Found error: %s", output);
            found_error = 1;
        }
    }
    
    int status = pclose(fp);
    if (status != 0) {
        // Non-zero exit is expected for invalid flags
        printf("  Program exited with status: %d\n", status);
    }
    
    return found_error;
}

int main(int argc, char *argv[]) {
    char *gcov_dump_path = NULL;
    int tests_passed = 0;
    int total_tests = 0;
    
    printf("=== Testing gcov-dump default case (unknown flags) ===\n\n");
    
    // Find gcov-dump executable
    gcov_dump_path = find_gcov_dump_path();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test cases with invalid flags
    const char *test_cases[] = {
        // Single invalid flags
        "-x",
        "-z",
        "-?",
        "-X",
        "-1",
        
        // Invalid flag in different positions
        "-x -l",           // Invalid before valid
        "-l -x",           // Invalid after valid  
        "-p -x -r",        // Invalid between valid
        "-l -x -p -r",     // Multiple invalid/valid mix
        
        // With file argument (non-option argument)
        "-x dummy.gcda",
        "dummy.gcda -x",   // Invalid flag after filename
        "-l -x dummy.gcda",
        
        // Double dash cases
        "--x",             // getopt might treat as unknown option
        "-l --x -p",
        
        // Edge cases
        "-",              // Just a dash
        "- ",             // Dash with space
        "-lxpr",          // Combined flags with invalid
        "-lxpzr",         // Combined with invalid in middle
        
        NULL
    };
    
    // Run all test cases
    for (int i = 0; test_cases[i] != NULL; i++) {
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, test_cases[i])) {
            tests_passed++;
        } else {
            printf("  ✗ No 'unknown flag' error found for: %s\n", test_cases[i]);
        }
        printf("\n");
    }
    
    // Cleanup
    free(gcov_dump_path);
    
    // Summary
    printf("=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    
    if (tests_passed > 0) {
        printf("\n✅ Successfully triggered the uncovered default case!\n");
        return EXIT_SUCCESS;
    } else {
        printf("\n❌ Failed to trigger the uncovered default case\n");
        return EXIT_FAILURE;
    }
}
