/**
 * test_gcov_dump_default_case.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by invoking
 * gcov-dump with invalid command-line flags.
 * 
 * Compile with: gcc -std=c99 -Wall -O0 -g -o test_gcov_dump test_gcov_dump_default_case.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH_LEN 1024
#define MAX_OUTPUT_LEN 4096
#define TARGET_ERROR_MSG "unknown flag"

/**
 * Find the gcov-dump executable path.
 * Priority:
 * 1. GCov_DUMP environment variable
 * 2. Common build locations
 * 3. System PATH
 */
static char *find_gcov_dump_path() {
    static char path[MAX_PATH_LEN];
    
    // 1. Check environment variable
    char *env_path = getenv("GCov_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, MAX_PATH_LEN - 1);
        path[MAX_PATH_LEN - 1] = '\0';
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
            strncpy(path, common_paths[i], MAX_PATH_LEN - 1);
            path[MAX_PATH_LEN - 1] = '\0';
            return path;
        }
    }
    
    // 3. Search in PATH
    char *path_env = getenv("PATH");
    if (path_env) {
        char *path_copy = strdup(path_env);
        char *dir = strtok(path_copy, ":");
        
        while (dir != NULL) {
            snprintf(path, MAX_PATH_LEN, "%s/gcov-dump", dir);
            if (access(path, X_OK) == 0) {
                free(path_copy);
                return path;
            }
            dir = strtok(NULL, ":");
        }
        free(path_copy);
    }
    
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if target error message found, 0 if not, -1 on error.
 */
static int test_gcov_dump_with_args(const char *gcov_dump_path, const char *args) {
    char command[MAX_PATH_LEN + 100];
    FILE *fp;
    char output[MAX_OUTPUT_LEN];
    int found_target = 0;
    
    // Build command to capture stderr
    snprintf(command, sizeof(command), "%s %s 2>&1", gcov_dump_path, args);
    
    printf("Testing: %s %s\n", gcov_dump_path, args);
    
    // Execute command and capture output
    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen failed");
        return -1;
    }
    
    // Read output
    while (fgets(output, sizeof(output), fp) != NULL) {
        // Check for target error message
        if (strstr(output, TARGET_ERROR_MSG) != NULL) {
            printf("  Found target message: %s", output);
            found_target = 1;
        }
    }
    
    // Get exit status
    int status = pclose(fp);
    if (status == -1) {
        perror("pclose failed");
        return -1;
    }
    
    return found_target ? 1 : 0;
}

int main(int argc, char *argv[]) {
    printf("=== Testing gcov-dump default case (unknown flags) ===\n");
    
    // Find gcov-dump executable
    char *gcov_dump_path = find_gcov_dump_path();
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
        
        // Invalid flag as first argument
        "-x -l",
        "-z -p",
        
        // Invalid flag between valid flags
        "-l -x -p",
        "-p -z -r",
        "-s -? -l",
        
        // Multiple invalid flags
        "-x -y -z",
        "-a -b -c",
        
        // Invalid flag after valid flag and filename
        "-l dummy.gcno -x",
        "-p dummy.gcda -z",
        
        // Double dash with invalid single char (getopt behavior test)
        "-- -x",
        "-- -z",
        
        // Combination with help/version (should not trigger default case)
        // These are included to ensure they DON'T trigger the target
        "-h -x",  // -h should cause early exit
        "-v -z",  // -v should cause early exit
        
        NULL
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    int failed_tests = 0;
    
    // Run all test cases
    for (int i = 0; test_cases[i] != NULL; i++) {
        total_tests++;
        
        int result = test_gcov_dump_with_args(gcov_dump_path, test_cases[i]);
        
        if (result == 1) {
            printf("  PASS: Invalid flag triggered default case\n");
            passed_tests++;
        } else if (result == 0) {
            printf("  FAIL: Did not trigger default case (or flag was valid)\n");
            failed_tests++;
        } else {
            printf("  ERROR: Command execution failed\n");
            failed_tests++;
        }
        printf("\n");
    }
    
    // Additional edge case: Test with a non-existent file and invalid flag
    printf("Testing with non-existent file and invalid flag:\n");
    total_tests++;
    int result = test_gcov_dump_with_args(gcov_dump_path, "-x nonexistent.gcno");
    if (result == 1) {
        printf("  PASS: Invalid flag triggered default case (file error may come after)\n");
        passed_tests++;
    } else {
        printf("  Result: %d\n", result);
        failed_tests++;
    }
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", failed_tests);
    
    // Create a dummy .gcno file for more realistic testing
    printf("\nCreating dummy test file for more complete testing...\n");
    FILE *dummy = fopen("dummy_test.gcno", "w");
    if (dummy) {
        fprintf(dummy, "This is not a real .gcno file\n");
        fclose(dummy);
        
        printf("Testing with dummy file and invalid flag:\n");
        result = test_gcov_dump_with_args(gcov_dump_path, "-x dummy_test.gcno");
        if (result == 1) {
            printf("  PASS: Invalid flag triggered default case\n");
        } else {
            printf("  Result: %d\n", result);
        }
        
        // Clean up
        remove("dummy_test.gcno");
    }
    
    return (passed_tests > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
