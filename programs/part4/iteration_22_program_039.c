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

#define MAX_CMD_LEN 1024
#define MAX_OUTPUT_LEN 4096

/**
 * Find the gcov-dump executable path.
 * Checks GCov_DUMP environment variable first, then common locations.
 */
static const char *find_gcov_dump(void) {
    const char *env_path = getenv("GCOV_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        return env_path;
    }
    
    // Common locations in a GCC build tree
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
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
    
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if "unknown flag" error is found, 0 otherwise.
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
        perror("popen failed");
        return 0;
    }
    
    // Read output
    size_t bytes_read = fread(output, 1, sizeof(output) - 1, fp);
    output[bytes_read] = '\0';
    
    pclose(fp);
    
    // Check for the error message
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

int main(void) {
    const char *gcov_dump_path = find_gcov_dump();
    
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in PATH.\n");
        return 1;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test cases for invalid flags
    const char *test_cases[] = {
        // Single invalid flags
        "-x",
        "-z",
        "-?",
        "-X",
        // Invalid flag combinations
        "-l -x -p",      // Invalid flag between valid ones
        "-x -l",         // Invalid flag first
        "-l -p -z",      // Invalid flag last
        "-x -y -z",      // Multiple invalid flags
        // Edge cases
        "--x",           // Double dash with single char (should be treated as --x argument)
        "- ",            // Space after dash
        "-",             // Just a dash
        // With filename argument
        "-x dummy.gcda",
        "dummy.gcda -x", // Invalid flag after filename
        // Mixed valid/invalid with filename
        "-l -x dummy.gcno -p",
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
    
    // Also test using exec directly for better exit code checking
    printf("Testing with execvp for exit code verification...\n");
    {
        char *args[] = { (char*)gcov_dump_path, "-x", NULL };
        pid_t pid = fork();
        
        if (pid == 0) {
            // Child process
            execvp(gcov_dump_path, args);
            perror("execvp failed");
            exit(1);
        } else if (pid > 0) {
            // Parent process
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                printf("Exit code: %d\n", WEXITSTATUS(status));
                if (WEXITSTATUS(status) != 0) {
                    passed_tests++;
                    total_tests++;
                }
            }
        }
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    
    if (passed_tests > 0) {
        printf("\n✅ Successfully triggered the uncovered default case!\n");
        return 0;
    } else {
        printf("\n❌ Failed to trigger the uncovered default case.\n");
        return 1;
    }
}
