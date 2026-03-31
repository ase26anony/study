/**
 * test_gcov_dump_default_case.c
 * 
 * Tests the uncovered default case in gcov-dump.cc that handles unknown flags.
 * Compile with: gcc -std=c99 -O0 -g -o test_gcov_dump test_gcov_dump_default_case.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_PATH_LEN 1024
#define MAX_OUTPUT_LEN 4096

/**
 * Find the gcov-dump executable path.
 * Priority:
 * 1. GCov_DUMP environment variable
 * 2. Common build paths
 * 3. System PATH
 */
static char* find_gcov_dump_path() {
    static char path[MAX_PATH_LEN];
    
    // 1. Check environment variable
    char* env_path = getenv("GCov_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, MAX_PATH_LEN - 1);
        path[MAX_PATH_LEN - 1] = '\0';
        return path;
    }
    
    // 2. Check common build locations
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
            strncpy(path, common_paths[i], MAX_PATH_LEN - 1);
            path[MAX_PATH_LEN - 1] = '\0';
            return path;
        }
    }
    
    // 3. Search in PATH
    char* path_env = getenv("PATH");
    if (path_env) {
        char* path_copy = strdup(path_env);
        char* dir = strtok(path_copy, ":");
        
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
 * Returns 1 if "unknown flag" message found, 0 otherwise.
 */
static int test_invalid_flag(const char* gcov_dump_path, const char* flag) {
    char command[MAX_PATH_LEN + 50];
    char output[MAX_OUTPUT_LEN] = {0};
    FILE* fp;
    int found = 0;
    
    // Build command: redirect stderr to stdout for capture
    snprintf(command, sizeof(command), "%s %s 2>&1", gcov_dump_path, flag);
    
    // Execute and capture output
    fp = popen(command, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to execute command: %s\n", command);
        return 0;
    }
    
    // Read output
    size_t bytes_read = fread(output, 1, MAX_OUTPUT_LEN - 1, fp);
    output[bytes_read] = '\0';
    
    // Check for "unknown flag" message
    if (strstr(output, "unknown flag") != NULL) {
        printf("✓ Found 'unknown flag' message for flag %s\n", flag);
        printf("  Output: %s", output);
        found = 1;
    } else {
        printf("✗ No 'unknown flag' message for flag %s\n", flag);
        printf("  Output: %s", output);
    }
    
    pclose(fp);
    return found;
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
        "-1",
        
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
        
        // Invalid flag with double dash (getopt behavior)
        "--x",
        "--z",
        
        // Invalid flag after --
        "-- -x",
        
        // Invalid flag with filename
        "-x dummy.gcda",
        "-l -x dummy.gcno",
        
        // Edge case: just dash
        "-",
        
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
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    
    if (passed_tests > 0) {
        printf("\n✅ Successfully triggered the uncovered default case!\n");
    } else {
        printf("\n❌ Failed to trigger the uncovered default case.\n");
    }
}

int main(int argc, char* argv[]) {
    printf("=== Testing gcov-dump uncovered default case ===\n\n");
    
    // Find gcov-dump executable
    char* gcov_dump_path = find_gcov_dump_path();
    
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: Could not find gcov-dump executable.\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH.\n");
        fprintf(stderr, "Common locations checked:\n");
        fprintf(stderr, "  - ./gcc/gcov-dump\n");
        fprintf(stderr, "  - ./gcov-dump\n");
        fprintf(stderr, "  - ../gcc/gcov-dump\n");
        fprintf(stderr, "  - /usr/bin/gcov-dump\n");
        return EXIT_FAILURE;
    }
    
    printf("Found gcov-dump: %s\n", gcov_dump_path);
    
    // Verify it's executable
    if (access(gcov_dump_path, X_OK) != 0) {
        fprintf(stderr, "Error: %s is not executable\n", gcov_dump_path);
        return EXIT_FAILURE;
    }
    
    // Run tests
    run_tests(gcov_dump_path);
    
    return EXIT_SUCCESS;
}
