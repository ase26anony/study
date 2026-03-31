/**
 * test_gcov_dump_flags.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by invoking
 * gcov-dump with invalid command-line flags.
 * 
 * Compile with: gcc -std=c99 -O0 -g -o test_gcov_dump test_gcov_dump_flags.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define MAX_PATH_LEN 1024
#define MAX_OUTPUT_LEN 4096
#define ERROR_MSG_PREFIX "unknown flag"

/**
 * Find gcov-dump executable path.
 * Priority:
 * 1. GCov_DUMP environment variable
 * 2. Common build locations
 * 3. System PATH
 */
static char* find_gcov_dump(void) {
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
 * Returns 1 if error message found, 0 if not, -1 on execution error.
 */
static int test_invalid_flag(const char* gcov_dump_path, const char** args, int arg_count) {
    char command[MAX_PATH_LEN * 2];
    FILE* fp;
    char output[MAX_OUTPUT_LEN];
    int found_error = 0;
    
    // Build command string
    snprintf(command, sizeof(command), "%s", gcov_dump_path);
    for (int i = 0; i < arg_count; i++) {
        strncat(command, " ", sizeof(command) - strlen(command) - 1);
        strncat(command, args[i], sizeof(command) - strlen(command) - 1);
    }
    
    // Redirect stderr to stdout for capture
    strncat(command, " 2>&1", sizeof(command) - strlen(command) - 1);
    
    printf("Testing: %s\n", command);
    
    // Execute command
    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen failed");
        return -1;
    }
    
    // Read output
    while (fgets(output, sizeof(output), fp) != NULL) {
        printf("  Output: %s", output);
        if (strstr(output, ERROR_MSG_PREFIX) != NULL) {
            found_error = 1;
        }
    }
    
    int status = pclose(fp);
    if (status == -1) {
        perror("pclose failed");
        return -1;
    }
    
    printf("  Exit status: %d\n", status);
    printf("  Found error message: %s\n\n", found_error ? "YES" : "NO");
    
    return found_error;
}

/**
 * Test various invalid flag scenarios.
 */
int main(void) {
    char* gcov_dump_path = find_gcov_dump();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: gcov-dump executable not found\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return 1;
    }
    
    printf("Found gcov-dump at: %s\n\n", gcov_dump_path);
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Test case 1: Single invalid flag
    {
        total_tests++;
        const char* args[] = {"-x"};
        printf("Test %d: Single invalid flag '-x'\n", total_tests);
        if (test_invalid_flag(gcov_dump_path, args, 1) == 1) {
            passed_tests++;
        }
    }
    
    // Test case 2: Invalid flag between valid flags
    {
        total_tests++;
        const char* args[] = {"-l", "-x", "-p"};
        printf("Test %d: Invalid flag '-x' between valid flags '-l -p'\n", total_tests);
        if (test_invalid_flag(gcov_dump_path, args, 3) == 1) {
            passed_tests++;
        }
    }
    
    // Test case 3: Multiple invalid flags
    {
        total_tests++;
        const char* args[] = {"-x", "-z", "-?"};
        printf("Test %d: Multiple invalid flags '-x -z -?'\n", total_tests);
        if (test_invalid_flag(gcov_dump_path, args, 3) == 1) {
            passed_tests++;
        }
    }
    
    // Test case 4: Invalid flag after filename argument
    {
        total_tests++;
        const char* args[] = {"-l", "dummy.gcda", "-x"};
        printf("Test %d: Invalid flag '-x' after filename\n", total_tests);
        if (test_invalid_flag(gcov_dump_path, args, 3) == 1) {
            passed_tests++;
        }
    }
    
    // Test case 5: Double dash with invalid single char flag
    {
        total_tests++;
        const char* args[] = {"--x"};
        printf("Test %d: Double dash with invalid flag '--x'\n", total_tests);
        if (test_invalid_flag(gcov_dump_path, args, 1) == 1) {
            passed_tests++;
        }
    }
    
    // Test case 6: Combined valid and invalid flags
    {
        total_tests++;
        const char* args[] = {"-v", "-x", "-h"};
        printf("Test %d: Valid flags '-v -h' with invalid '-x' in middle\n", total_tests);
        if (test_invalid_flag(gcov_dump_path, args, 3) == 1) {
            passed_tests++;
        }
    }
    
    // Test case 7: Invalid flag as first argument with filename
    {
        total_tests++;
        const char* args[] = {"-z", "test.gcno"};
        printf("Test %d: Invalid flag '-z' as first arg with filename\n", total_tests);
        if (test_invalid_flag(gcov_dump_path, args, 2) == 1) {
            passed_tests++;
        }
    }
    
    // Test case 8: Edge case - question mark (special for getopt)
    {
        total_tests++;
        const char* args[] = {"-?"};
        printf("Test %d: Question mark flag '-?'\n", total_tests);
        if (test_invalid_flag(gcov_dump_path, args, 1) == 1) {
            passed_tests++;
        }
    }
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Success rate: %.1f%%\n", (passed_tests * 100.0) / total_tests);
    
    if (passed_tests > 0) {
        printf("\nSUCCESS: Successfully triggered the uncovered default case!\n");
        return 0;
    } else {
        printf("\nFAILURE: Could not trigger the uncovered default case\n");
        return 1;
    }
}
