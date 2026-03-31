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
#include <errno.h>

#define MAX_CMD_LEN 1024
#define MAX_OUTPUT_LEN 4096

/**
 * Find the gcov-dump executable path.
 * Checks GCov_DUMP environment variable first, then common build locations.
 * Returns 1 if found (path stored in buffer), 0 otherwise.
 */
static int find_gcov_dump(char *path_buf, size_t buf_size) {
    const char *env_path = getenv("GCOV_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        strncpy(path_buf, env_path, buf_size - 1);
        path_buf[buf_size - 1] = '\0';
        return 1;
    }
    
    // Common build locations in GCC source tree
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path_buf, common_paths[i], buf_size - 1);
            path_buf[buf_size - 1] = '\0';
            return 1;
        }
    }
    
    return 0;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if "unknown flag" error found in stderr, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
    char cmd[MAX_CMD_LEN];
    char output[MAX_OUTPUT_LEN] = {0};
    FILE *fp;
    int found_error = 0;
    
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
    
    // Check for the specific error message
    if (strstr(output, "unknown flag") != NULL) {
        printf("✓ Found 'unknown flag' error for flag '%s'\n", flag);
        printf("  Output: %s", output);
        found_error = 1;
    } else {
        printf("✗ No 'unknown flag' error for flag '%s'\n", flag);
        printf("  Output: %s", output);
    }
    
    return found_error;
}

int main(int argc, char *argv[]) {
    char gcov_dump_path[MAX_CMD_LEN];
    int tests_passed = 0;
    int total_tests = 0;
    
    printf("=== Testing gcov-dump invalid flag handling ===\n");
    
    // Find gcov-dump executable
    if (!find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path))) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure it's in a common location.\n");
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
        "-x -l",         // Invalid first, valid second
        "-l -p -z",      // Invalid last
        "-x -y -z",      // Multiple invalid
        
        // Edge cases for getopt parsing
        "-x somefile.gcda",  // Invalid flag before filename
        "somefile.gcda -x",  // Invalid flag after filename (should still be parsed)
        "-l -x",             // Mixed valid/invalid
        "-x -l -p -r -s",    // Invalid first, then all valid
        
        // Double-dash cases (getopt behavior may vary)
        "--x",
        "--z",
        
        // Unknown long option (if supported)
        "--invalid",
        
        NULL
    };
    
    // Run all test cases
    for (int i = 0; test_cases[i] != NULL; i++) {
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, test_cases[i])) {
            tests_passed++;
        }
        printf("\n");
    }
    
    // Summary
    printf("=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    
    if (tests_passed > 0) {
        printf("SUCCESS: Triggered the uncovered default case in gcov-dump!\n");
        return EXIT_SUCCESS;
    } else {
        printf("FAILURE: Could not trigger the uncovered default case.\n");
        return EXIT_FAILURE;
    }
}
