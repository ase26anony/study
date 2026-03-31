/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by invoking
 * gcov-dump with various invalid command-line flags.
 * 
 * Compile with: gcc -std=c99 -Wall -O0 -g -o test_gcov_dump test_gcov_dump_invalid_flags.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH_LEN 1024
#define MAX_OUTPUT_LEN 4096
#define ERROR_MSG_SUBSTRING "unknown flag"

/**
 * Find the gcov-dump executable path.
 * Priority:
 * 1. GCov_DUMP environment variable
 * 2. Common build tree locations
 * 3. System PATH
 */
static char *find_gcov_dump_path(void) {
    static char path[MAX_PATH_LEN];
    
    // 1. Check environment variable
    char *env_path = getenv("GCov_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, MAX_PATH_LEN - 1);
        path[MAX_PATH_LEN - 1] = '\0';
        return path;
    }
    
    // 2. Check common build tree locations
    const char *common_paths[] = {
        "./gcc/gcov-dump",
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
    
    // 3. Try to find in PATH
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
 * Returns 1 if ERROR_MSG_SUBSTRING found in stderr, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
    int result = 0;
    char command[MAX_PATH_LEN * 2];
    FILE *fp;
    char output[MAX_OUTPUT_LEN];
    
    // Construct command to capture stderr
    snprintf(command, sizeof(command), "%s %s 2>&1", gcov_dump_path, flag);
    
    printf("Testing: %s\n", command);
    
    // Execute command and capture output
    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen failed");
        return 0;
    }
    
    // Read output
    size_t total_read = 0;
    while (fgets(output + total_read, sizeof(output) - total_read, fp) != NULL) {
        total_read += strlen(output + total_read);
        if (total_read >= sizeof(output) - 1) {
            break;
        }
    }
    
    // Check for error message
    if (strstr(output, ERROR_MSG_SUBSTRING) != NULL) {
        printf("  ✓ Found '%s' in output\n", ERROR_MSG_SUBSTRING);
        result = 1;
    } else {
        printf("  ✗ Did not find '%s' in output\n", ERROR_MSG_SUBSTRING);
        printf("    Output was: %s\n", output);
    }
    
    pclose(fp);
    return result;
}

/**
 * Test various invalid flag scenarios
 */
static int run_all_tests(const char *gcov_dump_path) {
    int passed_tests = 0;
    int total_tests = 0;
    
    // Test cases designed to trigger the default case in the switch statement
    struct {
        const char *description;
        const char *flag;
    } test_cases[] = {
        // Single invalid flags
        {"Single invalid flag 'x'", "-x"},
        {"Single invalid flag 'z'", "-z"},
        {"Single invalid flag '?'", "-?"},
        {"Single invalid flag 'X'", "-X"},
        
        // Invalid flag in different positions
        {"Invalid flag first, then valid flag", "-x -l"},
        {"Valid flag, then invalid flag", "-l -x"},
        {"Multiple invalid flags", "-x -z -y"},
        {"Mix of valid and invalid flags", "-l -x -p -z"},
        
        // Boundary cases for getopt parsing
        {"Invalid flag after filename", "test.gcda -x"},
        {"Invalid flag between valid flags and filename", "-l -x test.gcda -p"},
        
        // Double dash handling (getopt may treat this differently)
        {"Double dash with invalid flag", "--x"},
        {"Double dash with invalid flag after valid", "-l --x"},
        
        // Edge cases
        {"Invalid flag with no dash (should not trigger)", "x"},
        {"Empty flag (should not trigger)", "-"},
        
        // End marker
        {NULL, NULL}
    };
    
    printf("\n=== Running gcov-dump invalid flag tests ===\n\n");
    
    for (int i = 0; test_cases[i].description != NULL; i++) {
        printf("Test %d: %s\n", ++total_tests, test_cases[i].description);
        if (test_invalid_flag(gcov_dump_path, test_cases[i].flag)) {
            passed_tests++;
        }
        printf("\n");
    }
    
    printf("=== Test Summary ===\n");
    printf("Passed: %d/%d tests\n", passed_tests, total_tests);
    
    return (passed_tests == total_tests) ? 0 : 1;
}

int main(void) {
    char *gcov_dump_path;
    
    printf("=== gcov-dump Invalid Flag Tester ===\n");
    
    // Find gcov-dump executable
    gcov_dump_path = find_gcov_dump_path();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Found gcov-dump at: %s\n", gcov_dump_path);
    
    // Verify executable works
    if (access(gcov_dump_path, X_OK) != 0) {
        fprintf(stderr, "Error: Cannot execute %s\n", gcov_dump_path);
        return EXIT_FAILURE;
    }
    
    // Run all tests
    int result = run_all_tests(gcov_dump_path);
    
    if (result == 0) {
        printf("\nAll tests passed! The uncovered lines in gcov-dump.cc were executed.\n");
    } else {
        printf("\nSome tests failed. Check if gcov-dump handles invalid flags differently.\n");
    }
    
    return result;
}
