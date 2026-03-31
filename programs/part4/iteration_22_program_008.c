/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Test program to trigger the uncovered default case in gcov-dump.cc
 * when invalid command-line flags are provided.
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
 * Priority: 1. GCov_DUMP environment variable
 *           2. Common build locations
 *           3. System PATH
 */
static const char *find_gcov_dump_path(void) {
    const char *env_path = getenv("GCov_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        return env_path;
    }
    
    // Common build locations in GCC source tree
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "gcc/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            return common_paths[i];
        }
    }
    
    // Last resort: check if it's in PATH
    if (system("which gcov-dump > /dev/null 2>&1") == 0) {
        return "gcov-dump";
    }
    
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if "unknown flag" error is found, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
    char command[MAX_CMD_LEN];
    char output[MAX_OUTPUT_LEN];
    FILE *fp;
    int found = 0;
    
    // Construct command to capture stderr
    snprintf(command, sizeof(command), 
             "%s %s 2>&1", gcov_dump_path, flag);
    
    // Execute and capture output
    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen failed");
        return 0;
    }
    
    // Read output
    while (fgets(output, sizeof(output), fp) != NULL) {
        // Check for the target error message
        if (strstr(output, "unknown flag") != NULL) {
            printf("SUCCESS: Found 'unknown flag' error for flag %s\n", flag);
            printf("Output: %s", output);
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
    
    // Test cases covering different scenarios
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
        "-x file.gcno",    // Invalid before filename
        "file.gcno -x",    // Invalid after filename (getopt stops at first non-option)
        
        // Multiple invalid flags
        "-x -y -z",
        
        // Edge cases
        "--x",             // Double dash with single char
        "-",               // Just a dash
        "- ",              // Dash with space
        
        // Combination with valid flags that might affect parsing
        "-lpx",            // Combined flags with invalid 'x'
        "-xpl",            // Invalid first in combined
        
        NULL
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    
    for (int i = 0; test_cases[i] != NULL; i++) {
        total_tests++;
        printf("Test %d: gcov-dump %s\n", total_tests, test_cases[i]);
        
        if (test_invalid_flag(gcov_dump_path, test_cases[i])) {
            passed_tests++;
        } else {
            printf("FAILED: No 'unknown flag' error for: %s\n", test_cases[i]);
        }
        printf("\n");
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    
    if (passed_tests > 0) {
        printf("\nSUCCESS: Triggered the uncovered default case!\n");
    } else {
        printf("\nFAILURE: Could not trigger the uncovered default case.\n");
    }
}

int main(void) {
    const char *gcov_dump_path = find_gcov_dump_path();
    
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "ERROR: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCov_DUMP environment variable to the path of gcov-dump.\n");
        fprintf(stderr, "Or ensure gcov-dump is in one of the common build locations.\n");
        return EXIT_FAILURE;
    }
    
    printf("Found gcov-dump: %s\n", gcov_dump_path);
    
    // Verify executable exists and is executable
    if (access(gcov_dump_path, X_OK) != 0) {
        fprintf(stderr, "ERROR: Cannot execute %s\n", gcov_dump_path);
        return EXIT_FAILURE;
    }
    
    run_tests(gcov_dump_path);
    
    return EXIT_SUCCESS;
}
