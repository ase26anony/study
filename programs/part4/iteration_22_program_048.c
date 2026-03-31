/**
 * test_gcov_dump_default_case.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by providing invalid
 * command-line flags and verifying the error message.
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
 * Priority: GCov_DUMP env var -> hardcoded paths -> NULL if not found.
 */
static char* find_gcov_dump_path() {
    char* path = getenv("GCov_DUMP");
    if (path != NULL && access(path, X_OK) == 0) {
        return strdup(path);
    }
    
    // Common build locations for gcov-dump
    const char* candidates[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; candidates[i] != NULL; i++) {
        if (access(candidates[i], X_OK) == 0) {
            return strdup(candidates[i]);
        }
    }
    
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 0 on success (found error message), 1 on failure.
 */
static int test_invalid_flag(const char* gcov_dump_path, const char* flag) {
    printf("Testing flag: %s\n", flag);
    
    // Build command with stderr redirected to stdout for capture
    char command[MAX_PATH_LEN + 100];
    snprintf(command, sizeof(command), "%s %s 2>&1", gcov_dump_path, flag);
    
    // Use popen to capture output
    FILE* fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen failed");
        return 1;
    }
    
    char output[MAX_OUTPUT_LEN];
    size_t total_read = 0;
    int found_error = 0;
    
    // Read output line by line
    while (fgets(output + total_read, sizeof(output) - total_read, fp) != NULL) {
        total_read = strlen(output);
        
        // Check if error message appears in output
        if (strstr(output, ERROR_MSG_PREFIX) != NULL) {
            found_error = 1;
            printf("  ✓ Found error message: %s", output);
        }
        
        if (total_read >= sizeof(output) - 1) {
            break; // Buffer full
        }
    }
    
    int status = pclose(fp);
    
    if (!found_error) {
        printf("  ✗ No error message found in output\n");
        if (output[0] != '\0') {
            printf("    Output was: %s\n", output);
        }
        return 1;
    }
    
    // Optionally check exit status (should be non-zero for error)
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("  Warning: Program exited with status 0 (expected non-zero)\n");
    }
    
    return 0;
}

/**
 * Test various invalid flag scenarios.
 */
static void run_tests(const char* gcov_dump_path) {
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test cases covering different scenarios
    const char* test_cases[] = {
        // Single invalid flags
        "-x",
        "-z",
        "-?",
        "-X",
        
        // Invalid flag as first argument
        "-x -l",  // Invalid then valid
        
        // Invalid flag between valid flags
        "-l -x -p",
        "-p -z -r",
        
        // Multiple invalid flags
        "-x -y -z",
        
        // Invalid flag after double dash (getopt behavior varies)
        "-- -x",
        
        // Invalid flag with filename argument
        "-x dummy.gcda",
        "dummy.gcda -z",  // Invalid after filename
        
        // Edge case: just a dash
        "-",
        
        NULL
    };
    
    int passed = 0;
    int total = 0;
    
    for (int i = 0; test_cases[i] != NULL; i++) {
        total++;
        if (test_invalid_flag(gcov_dump_path, test_cases[i]) == 0) {
            passed++;
        }
        printf("\n");
    }
    
    printf("Results: %d/%d tests passed\n", passed, total);
    
    if (passed == 0) {
        printf("\nERROR: No tests triggered the default case!\n");
        printf("Possible issues:\n");
        printf("1. Wrong gcov-dump executable\n");
        printf("2. Different error message format\n");
        printf("3. getopt_long might handle invalid flags differently\n");
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    char* gcov_dump_path = find_gcov_dump_path();
    
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "ERROR: gcov-dump executable not found\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or place executable in common locations\n");
        
        // Try to compile gcov-dump from source if available
        fprintf(stderr, "\nAttempting to find source and compile...\n");
        if (access("gcov-dump.cc", R_OK) == 0) {
            printf("Found gcov-dump.cc, attempting to compile...\n");
            system("g++ -std=c++11 -O0 -g gcov-dump.cc -o ./test-gcov-dump 2>&1");
            if (access("./test-gcov-dump", X_OK) == 0) {
                gcov_dump_path = strdup("./test-gcov-dump");
                printf("Compiled successfully\n");
            }
        }
        
        if (gcov_dump_path == NULL) {
            return 1;
        }
    }
    
    // If a specific test case is provided as argument, use it
    if (argc > 1) {
        printf("Running custom test: %s\n", argv[1]);
        int result = test_invalid_flag(gcov_dump_path, argv[1]);
        free(gcov_dump_path);
        return result;
    }
    
    // Run all tests
    run_tests(gcov_dump_path);
    
    free(gcov_dump_path);
    
    // Clean up temporary compiled binary if it exists
    if (access("./test-gcov-dump", F_OK) == 0) {
        unlink("./test-gcov-dump");
    }
    
    return 0;
}
