/**
 * test_gcov_dump_invalid_flags.c
 * 
 * This program tests the uncovered default case in gcov-dump.cc
 * by executing gcov-dump with invalid command-line flags.
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
 *           2. Common build paths
 * Returns: Dynamically allocated string with path, or NULL if not found.
 */
char* find_gcov_dump() {
    char* path = NULL;
    
    // Check environment variable first
    path = getenv("GCov_DUMP");
    if (path != NULL && access(path, X_OK) == 0) {
        return strdup(path);
    }
    
    // Try common build locations
    const char* common_paths[] = {
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
            return strdup(common_paths[i]);
        }
    }
    
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns: 1 if target error message found, 0 if not, -1 on execution error.
 */
int test_invalid_flag(const char* gcov_dump_path, const char* flag) {
    char cmd[MAX_CMD_LEN];
    char output[MAX_OUTPUT_LEN];
    FILE* fp;
    int found = 0;
    
    // Build command to capture stderr
    snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, flag);
    
    // Execute command and capture output
    fp = popen(cmd, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to execute command: %s\n", cmd);
        return -1;
    }
    
    // Read output
    output[0] = '\0';
    while (fgets(output + strlen(output), 
                 sizeof(output) - strlen(output), fp) != NULL) {
        // Check if we found the target error message
        if (strstr(output, TARGET_ERROR_MSG) != NULL) {
            found = 1;
        }
    }
    
    // Get exit status
    int status = pclose(fp);
    
    if (found) {
        printf("SUCCESS: Found '%s' in output for flag '%s'\n", 
               TARGET_ERROR_MSG, flag);
        printf("Output:\n%s\n", output);
        return 1;
    } else {
        printf("FAILURE: Did not find '%s' in output for flag '%s'\n", 
               TARGET_ERROR_MSG, flag);
        printf("Output:\n%s\n", output);
        return 0;
    }
}

int main(int argc, char* argv[]) {
    char* gcov_dump_path = NULL;
    int success_count = 0;
    int test_count = 0;
    
    printf("=== Testing gcov-dump invalid flag handling ===\n");
    
    // Find gcov-dump executable
    gcov_dump_path = find_gcov_dump();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "ERROR: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCov_DUMP environment variable to specify path\n");
        return 1;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test cases covering different scenarios
    const char* test_cases[] = {
        // Single invalid flags
        "-x",
        "-z",
        "-?",
        "-X",
        
        // Invalid flag as first argument
        "-x -l",
        
        // Invalid flag between valid flags
        "-l -x -p",
        "-p -z -r",
        
        // Multiple invalid flags
        "-x -y -z",
        
        // Invalid flag after valid flag
        "-l -x",
        "-p -z",
        
        // Invalid flag before filename (non-option argument)
        "-x dummy.gcda",
        
        // Invalid flag after filename
        "dummy.gcda -x",
        
        // Double dash with invalid flag (getopt behavior test)
        "--x",
        
        // Combined valid and invalid
        "-l -x -p -s -z",
        
        // Edge case: just a dash
        "-",
        
        NULL
    };
    
    // Run all test cases
    for (int i = 0; test_cases[i] != NULL; i++) {
        printf("Test %d: gcov-dump %s\n", test_count + 1, test_cases[i]);
        
        int result = test_invalid_flag(gcov_dump_path, test_cases[i]);
        if (result == 1) {
            success_count++;
        } else if (result == -1) {
            fprintf(stderr, "Execution failed for test case: %s\n", test_cases[i]);
        }
        
        test_count++;
        printf("\n");
    }
    
    // Summary
    printf("=== Test Summary ===\n");
    printf("Total tests: %d\n", test_count);
    printf("Successful (found error message): %d\n", success_count);
    printf("Failed: %d\n", test_count - success_count);
    
    free(gcov_dump_path);
    
    // Return success if at least one test found the error message
    return (success_count > 0) ? 0 : 1;
}
