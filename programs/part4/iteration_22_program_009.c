/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Tests the gcov-dump utility by passing invalid command-line flags
 * to trigger the uncovered default case in the option parsing switch.
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
static char *find_gcov_dump_path(void) {
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
        "../../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "../../prev-gcc/build/gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path, common_paths[i], MAX_CMD_LEN - 1);
            path[MAX_CMD_LEN - 1] = '\0';
            return path;
        }
    }
    
    // 3. Try to find in PATH
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
 * Returns 1 if target error message found, 0 if not, -1 on error.
 */
static int test_gcov_dump_with_args(const char *gcov_dump_path, const char *args) {
    char command[MAX_CMD_LEN];
    char output[MAX_OUTPUT_LEN] = {0};
    FILE *fp;
    int found_target = 0;
    
    // Build command to capture stderr
    snprintf(command, MAX_CMD_LEN, "%s %s 2>&1", gcov_dump_path, args);
    
    // Execute command and capture output
    fp = popen(command, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to execute command: %s\n", command);
        return -1;
    }
    
    // Read output
    size_t total_read = 0;
    while (fgets(output + total_read, MAX_OUTPUT_LEN - total_read, fp) != NULL) {
        total_read = strlen(output);
        if (total_read >= MAX_OUTPUT_LEN - 1) {
            break;
        }
    }
    
    int status = pclose(fp);
    
    // Check for target error message
    if (strstr(output, TARGET_ERROR_MSG) != NULL) {
        found_target = 1;
        printf("SUCCESS: Found target error message in output:\n%s\n", output);
    } else {
        printf("Output did not contain target message '%s':\n%s\n", 
               TARGET_ERROR_MSG, output);
    }
    
    // Also check exit status (should be non-zero for invalid flags)
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        printf("Note: Program exited with non-zero status (%d) as expected\n", 
               WEXITSTATUS(status));
    }
    
    return found_target;
}

/**
 * Run multiple test cases with different invalid flag scenarios.
 */
static int run_test_cases(const char *gcov_dump_path) {
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test case 1: Single invalid flag at beginning
    printf("Test 1: Single invalid flag '-x'\n");
    total_tests++;
    if (test_gcov_dump_with_args(gcov_dump_path, "-x") > 0) {
        passed_tests++;
    }
    printf("\n");
    
    // Test case 2: Single invalid flag '-z'
    printf("Test 2: Single invalid flag '-z'\n");
    total_tests++;
    if (test_gcov_dump_with_args(gcov_dump_path, "-z") > 0) {
        passed_tests++;
    }
    printf("\n");
    
    // Test case 3: Invalid flag '-?' (boundary case)
    printf("Test 3: Invalid flag '-?'\n");
    total_tests++;
    if (test_gcov_dump_with_args(gcov_dump_path, "-\\?") > 0) {
        passed_tests++;
    }
    printf("\n");
    
    // Test case 4: Invalid flag between valid flags
    printf("Test 4: Invalid flag between valid flags '-l -x -p'\n");
    total_tests++;
    if (test_gcov_dump_with_args(gcov_dump_path, "-l -x -p") > 0) {
        passed_tests++;
    }
    printf("\n");
    
    // Test case 5: Multiple invalid flags
    printf("Test 5: Multiple invalid flags '-x -y -z'\n");
    total_tests++;
    if (test_gcov_dump_with_args(gcov_dump_path, "-x -y -z") > 0) {
        passed_tests++;
    }
    printf("\n");
    
    // Test case 6: Invalid flag after non-option argument
    // (using a dummy file that likely doesn't exist)
    printf("Test 6: Invalid flag after filename '-l dummy.gcda -x'\n");
    total_tests++;
    if (test_gcov_dump_with_args(gcov_dump_path, "-l dummy.gcda -x") > 0) {
        passed_tests++;
    }
    printf("\n");
    
    // Test case 7: Double dash with invalid flag (edge case)
    printf("Test 7: Double dash with invalid flag '--x'\n");
    total_tests++;
    if (test_gcov_dump_with_args(gcov_dump_path, "--x") > 0) {
        passed_tests++;
    }
    printf("\n");
    
    // Test case 8: Combined valid and invalid in single argument
    printf("Test 8: Combined flags '-lxz' (valid l, invalid x, invalid z)\n");
    total_tests++;
    if (test_gcov_dump_with_args(gcov_dump_path, "-lxz") > 0) {
        passed_tests++;
    }
    printf("\n");
    
    printf("Test Summary: %d/%d tests passed\n", passed_tests, total_tests);
    
    return (passed_tests > 0) ? 0 : 1;
}

int main(void) {
    char *gcov_dump_path;
    
    // Find gcov-dump executable
    gcov_dump_path = find_gcov_dump_path();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return 1;
    }
    
    printf("Found gcov-dump: %s\n", gcov_dump_path);
    
    // Run test cases
    return run_test_cases(gcov_dump_path);
}
