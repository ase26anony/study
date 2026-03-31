/**
 * test_gcov_dump_default_case.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by invoking
 * gcov-dump with invalid command-line flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define MAX_OUTPUT_LEN 4096

/**
 * Find the gcov-dump executable path.
 * Priority: 1. GCov_DUMP environment variable
 *           2. Common build locations
 *           3. System PATH
 */
static char *find_gcov_dump(void) {
    static char path[MAX_CMD_LEN];
    
    // 1. Check environment variable
    char *env_path = getenv("GCov_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, MAX_CMD_LEN - 1);
        path[MAX_CMD_LEN - 1] = '\0';
        return path;
    }
    
    // 2. Check common build locations
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "/usr/local/bin/gcov-dump",
        "/usr/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path, common_paths[i], MAX_CMD_LEN - 1);
            path[MAX_CMD_LEN - 1] = '\0';
            return path;
        }
    }
    
    // 3. Search in PATH
    char *path_env = getenv("PATH");
    if (path_env) {
        char *path_copy = strdup(path_env);
        char *dir = strtok(path_copy, ":");
        
        while (dir != NULL) {
            snprintf(path, MAX_CMD_LEN, "%s/gcov-dump", dir);
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
 * Returns 1 if "unknown flag" message found, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
    char cmd[MAX_CMD_LEN];
    char output[MAX_OUTPUT_LEN] = {0};
    FILE *fp;
    int found = 0;
    
    // Build command: redirect stderr to stdout for capture
    snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, flag);
    
    // Execute and capture output
    fp = popen(cmd, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to execute command: %s\n", cmd);
        return 0;
    }
    
    // Read output
    size_t bytes_read = fread(output, 1, sizeof(output) - 1, fp);
    output[bytes_read] = '\0';
    
    int status = pclose(fp);
    
    // Check for the error message
    if (strstr(output, "unknown flag") != NULL) {
        printf("✓ Found 'unknown flag' message for flag '%s'\n", flag);
        printf("  Output: %s", output);
        found = 1;
    } else {
        printf("✗ No 'unknown flag' message for flag '%s'\n", flag);
        printf("  Output: %s", output);
    }
    
    return found;
}

int main(void) {
    printf("=== Testing gcov-dump default case (unknown flags) ===\n");
    
    // Find gcov-dump executable
    char *gcov_dump_path = find_gcov_dump();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH.\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test various invalid flags and edge cases
    const char *test_cases[] = {
        // Single invalid flags
        "-x",
        "-z",
        "-?",
        "-X",
        "-1",
        
        // Invalid flag combinations
        "-l -x -p",      // Invalid between valid flags
        "-x -l",         // Invalid first, then valid
        "-l -p -z",      // Invalid last
        "-x -y -z",      // Multiple invalid
        
        // Edge cases for getopt parsing
        "-x file.gcda",  // Invalid flag before filename
        "file.gcda -x",  // Invalid flag after filename (getopt stops at first non-option)
        "-lpx",          // Combined flags with invalid 'x'
        "-lpz",          // Combined flags with invalid 'z'
        
        // Double dash cases
        "--x",           // Double dash with single char (getopt may treat as --x argument)
        "--",            // Just double dash (should stop option processing)
        "-- -x",         // Double dash then invalid flag (should be treated as argument)
        
        NULL
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    
    for (int i = 0; test_cases[i] != NULL; i++) {
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, test_cases[i])) {
            passed_tests++;
        }
        printf("\n");
    }
    
    // Summary
    printf("=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    
    // We consider it a success if at least one test triggered the default case
    if (passed_tests > 0) {
        printf("\nSUCCESS: Successfully triggered the default case in gcov-dump!\n");
        return EXIT_SUCCESS;
    } else {
        printf("\nFAILURE: Could not trigger the default case.\n");
        return EXIT_FAILURE;
    }
}
