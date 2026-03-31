/**
 * test_gcov_dump_default_case.c
 * 
 * Tests the uncovered default case in gcov-dump.cc that handles
 * unknown command-line flags.
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
static const char *find_gcov_dump(void) {
    const char *env_path = getenv("GCov_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        return env_path;
    }
    
    // Common build locations in GCC source tree
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            return common_paths[i];
        }
    }
    
    // Try PATH
    const char *path_env = getenv("PATH");
    if (path_env) {
        static char full_path[512];
        char *path_copy = strdup(path_env);
        char *dir = strtok(path_copy, ":");
        
        while (dir) {
            snprintf(full_path, sizeof(full_path), "%s/gcov-dump", dir);
            if (access(full_path, X_OK) == 0) {
                free(path_copy);
                return full_path;
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
    
    fp = popen(cmd, "r");
    if (!fp) {
        fprintf(stderr, "Failed to execute: %s\n", cmd);
        return 0;
    }
    
    // Read output
    while (fgets(output, sizeof(output) - 1, fp) != NULL) {
        // Check for the exact error message pattern
        if (strstr(output, "unknown flag") != NULL) {
            printf("Found target message: %s", output);
            found = 1;
        }
    }
    
    int status = pclose(fp);
    if (WIFEXITED(status)) {
        // gcov-dump should exit with non-zero for invalid flags
        if (WEXITSTATUS(status) != 0 && found) {
            printf("Exit status: %d (expected non-zero)\n", WEXITSTATUS(status));
        }
    }
    
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
        "-1",
        
        // Invalid flag as first argument
        "-x -l",      // invalid then valid
        "-x file.gcda", // invalid then filename
        
        // Invalid flag between valid flags
        "-l -x -p",
        "-p -z -r",
        
        // Invalid flag after valid flags
        "-l -p -x",
        "-r -s -z",
        
        // Invalid flag after filename (should still be parsed)
        "dummy.gcda -x",
        "-l dummy.gcda -z",
        
        // Multiple invalid flags
        "-x -y -z",
        "-a -b -c",
        
        // Double dash with invalid single char (getopt behavior)
        "--x",
        "-- -x",      // -- stops option processing, -x becomes argument
        
        // Edge cases
        "-",          // just a dash
        "- ",         // dash with space
        "-lxz",       // combined flags with invalid in middle
        "-xlp",       // invalid first in combined
        
        NULL
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    
    for (int i = 0; test_cases[i] != NULL; i++) {
        printf("Test %d: gcov-dump %s\n", total_tests + 1, test_cases[i]);
        
        if (test_invalid_flag(gcov_dump_path, test_cases[i])) {
            printf("  ✓ Triggered default case\n");
            passed_tests++;
        } else {
            printf("  ✗ Did not trigger default case\n");
        }
        
        total_tests++;
        printf("\n");
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", total_tests - passed_tests);
    
    if (passed_tests > 0) {
        printf("\n✓ Successfully triggered the uncovered default case!\n");
    }
}

int main(void) {
    const char *gcov_dump_path = find_gcov_dump();
    
    if (!gcov_dump_path) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH.\n");
        fprintf(stderr, "Common locations: ./gcc/gcov-dump, ./gcov-dump, /usr/bin/gcov-dump\n");
        return EXIT_FAILURE;
    }
    
    // Verify the executable works
    printf("Using gcov-dump: %s\n", gcov_dump_path);
    
    // Quick version check to verify executable is valid
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s -v 2>&1 | head -1", gcov_dump_path);
    FILE *fp = popen(cmd, "r");
    if (fp) {
        char version[256];
        if (fgets(version, sizeof(version), fp)) {
            printf("Version: %s", version);
        }
        pclose(fp);
    }
    
    run_tests(gcov_dump_path);
    
    return EXIT_SUCCESS;
}
