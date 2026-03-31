/**
 * test_gcov_dump_default_case.c
 * 
 * Tests the uncovered default case in gcov-dump.cc when an invalid
 * command-line flag is provided.
 * 
 * Compile with: gcc -std=c99 -O0 -g -o test_gcov_dump test_gcov_dump_default_case.c
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
 * Priority:
 * 1. GCov_DUMP environment variable
 * 2. Common build locations
 * 3. System PATH
 */
static const char *find_gcov_dump_path(void) {
    const char *env_path = getenv("GCov_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        return env_path;
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
            return common_paths[i];
        }
    }
    
    // Last resort: try to find in PATH
    FILE *fp = popen("which gcov-dump 2>/dev/null", "r");
    if (fp != NULL) {
        static char path_buf[256];
        if (fgets(path_buf, sizeof(path_buf), fp) != NULL) {
            // Remove trailing newline
            size_t len = strlen(path_buf);
            if (len > 0 && path_buf[len-1] == '\n') {
                path_buf[len-1] = '\0';
            }
            pclose(fp);
            if (access(path_buf, X_OK) == 0) {
                return path_buf;
            }
        }
        pclose(fp);
    }
    
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if target error message found, 0 if not, -1 on error.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *test_args, 
                             const char *test_name) {
    printf("Testing: %s\n", test_name);
    
    // Build command with stderr redirection
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, test_args);
    
    // Execute and capture output
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to execute command: %s\n", cmd);
        return -1;
    }
    
    char output[MAX_OUTPUT_LEN];
    size_t total_read = 0;
    int found_target = 0;
    
    while (fgets(output, sizeof(output), fp) != NULL) {
        total_read += strlen(output);
        
        // Check for target error message
        if (strstr(output, TARGET_ERROR_MSG) != NULL) {
            found_target = 1;
            printf("  Found target message: %s", output);
        }
    }
    
    int status = pclose(fp);
    
    if (found_target) {
        printf("  ✓ SUCCESS: Triggered default case\n");
        return 1;
    } else if (total_read == 0) {
        printf("  ✗ No output (program may have crashed or not exist)\n");
        return -1;
    } else {
        printf("  ✗ FAILED: Did not trigger default case\n");
        return 0;
    }
}

/**
 * Main test driver
 */
int main(int argc, char *argv[]) {
    printf("=== Testing gcov-dump default case (invalid flags) ===\n\n");
    
    // Find gcov-dump executable
    const char *gcov_dump_path = find_gcov_dump_path();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "ERROR: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test cases designed to trigger the default case
    struct {
        const char *args;
        const char *description;
    } test_cases[] = {
        // Single invalid flags
        {"-x", "Single invalid flag '-x'"},
        {"-z", "Single invalid flag '-z'"},
        {"-?", "Single invalid flag '-?'"},
        {"-X", "Single invalid flag '-X' (uppercase)"},
        
        // Invalid flag in different positions
        {"-x -l", "Invalid flag before valid flag"},
        {"-l -x", "Invalid flag after valid flag"},
        {"-p -x -r", "Invalid flag between valid flags"},
        {"-l -x -p -r -s", "Multiple valid flags with one invalid"},
        
        // Multiple invalid flags
        {"-x -y -z", "Multiple invalid flags"},
        
        // Edge cases with double dash
        {"--x", "Double dash with single char (may be treated as --x argument)"},
        {"-x -- -l", "Invalid flag before -- separator"},
        {"-- -x", "Invalid flag after -- separator (should not be parsed as flag)"},
        
        // With filename argument
        {"-x test.gcno", "Invalid flag with filename"},
        {"test.gcno -x", "Filename before invalid flag"},
        {"-l -x test.gcno", "Valid, invalid, then filename"},
        
        // Combined short options (invalid char in cluster)
        {"-lpxr", "Cluster with invalid 'x' in middle"},
        {"-xlpr", "Cluster with invalid 'x' first"},
        {"-lprx", "Cluster with invalid 'x' last"},
        
        // Empty argument (should trigger help or error)
        {"", "No arguments (should show help, not default case)"},
        
        {NULL, NULL}
    };
    
    int total_tests = 0;
    int successful_tests = 0;
    int failed_tests = 0;
    
    // Run all test cases
    for (int i = 0; test_cases[i].args != NULL; i++) {
        int result = test_invalid_flag(gcov_dump_path, 
                                      test_cases[i].args,
                                      test_cases[i].description);
        
        total_tests++;
        
        if (result == 1) {
            successful_tests++;
        } else if (result == 0) {
            failed_tests++;
        }
        // result == -1 doesn't count as test failure (execution problem)
        
        printf("\n");
    }
    
    // Summary
    printf("=== Test Summary ===\n");
    printf("Total tests attempted: %d\n", total_tests);
    printf("Successfully triggered default case: %d\n", successful_tests);
    printf("Failed to trigger default case: %d\n", failed_tests);
    printf("Execution problems: %d\n", total_tests - successful_tests - failed_tests);
    
    // Consider test suite successful if we triggered the default case at least once
    if (successful_tests > 0) {
        printf("\n✓ SUCCESS: Triggered the uncovered default case\n");
        return EXIT_SUCCESS;
    } else {
        printf("\n✗ FAILURE: Never triggered the uncovered default case\n");
        return EXIT_FAILURE;
    }
}
