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
#define TARGET_ERROR_MSG "unknown flag"

/**
 * Find the gcov-dump executable path.
 * Checks GCov_DUMP environment variable first, then common locations.
 */
static const char *find_gcov_dump(void) {
    const char *env_path = getenv("GCOV_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        return env_path;
    }
    
    // Common locations in GCC build trees
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
            return common_paths[i];
        }
    }
    
    return NULL;
}

/**
 * Execute command and capture stderr output.
 * Returns dynamically allocated string with stderr output.
 * Caller must free the returned string.
 */
static char *capture_stderr(const char *cmd) {
    char *output = NULL;
    FILE *fp = NULL;
    char temp_filename[] = "/tmp/gcov_dump_test_XXXXXX";
    int fd;
    
    // Create temporary file for stderr
    fd = mkstemp(temp_filename);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }
    close(fd);
    
    // Construct command to redirect stderr to temp file
    char full_cmd[MAX_CMD_LEN];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>%s", cmd, temp_filename);
    
    // Execute command
    int status = system(full_cmd);
    (void)status; // We care about output, not exit status for this test
    
    // Read stderr output from temp file
    fp = fopen(temp_filename, "r");
    if (fp != NULL) {
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        
        if (size > 0) {
            output = malloc(size + 1);
            if (output != NULL) {
                fread(output, 1, size, fp);
                output[size] = '\0';
            }
        }
        fclose(fp);
    }
    
    // Clean up temp file
    unlink(temp_filename);
    
    return output;
}

/**
 * Test a specific invalid flag combination.
 * Returns 1 if target error message found, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag_combination) {
    char cmd[MAX_CMD_LEN];
    char *stderr_output = NULL;
    int found = 0;
    
    // Construct command with invalid flag(s)
    snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_path, flag_combination);
    
    printf("Testing: %s\n", cmd);
    
    // Capture stderr
    stderr_output = capture_stderr(cmd);
    if (stderr_output != NULL) {
        // Check for target error message
        if (strstr(stderr_output, TARGET_ERROR_MSG) != NULL) {
            printf("  ✓ Found target error message\n");
            found = 1;
        } else {
            printf("  ✗ Target error message not found\n");
            if (strlen(stderr_output) > 0) {
                printf("    Actual stderr: %s", stderr_output);
            }
        }
        free(stderr_output);
    } else {
        printf("  ✗ No stderr output captured\n");
    }
    
    return found;
}

int main(void) {
    const char *gcov_dump_path = find_gcov_dump();
    int tests_passed = 0;
    int total_tests = 0;
    
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in a common location.\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test various invalid flag scenarios
    struct {
        const char *description;
        const char *flags;
    } test_cases[] = {
        // Single invalid flags
        {"Single invalid flag -x", "-x"},
        {"Single invalid flag -z", "-z"},
        {"Single invalid flag -?", "-?"},
        {"Single invalid flag -@", "-@"},
        
        // Invalid flag in different positions
        {"Invalid flag first, then valid", "-x -l"},
        {"Valid flag first, then invalid", "-l -x"},
        {"Multiple invalid flags", "-x -y -z"},
        {"Mix of valid and invalid", "-l -x -p -z -r"},
        
        // Edge cases with getopt parsing
        {"Invalid flag after filename", "test.gcda -x"},
        {"Invalid flag between valid flags and filename", "-l -x test.gcno -p"},
        {"Double dash with invalid flag", "-- -x"},
        {"Double dash invalid flag (--x)", "--x"},
        
        // Combined flags (if supported)
        {"Combined flags with invalid", "-lpzx"},
        
        // Boundary: just a dash
        {"Single dash only", "-"},
        
        // Invalid flag with argument
        {"Invalid flag with argument", "-x argument"},
        
        NULL
    };
    
    // Run all test cases
    for (int i = 0; test_cases[i].description != NULL; i++) {
        printf("\nTest %d: %s\n", ++total_tests, test_cases[i].description);
        if (test_invalid_flag(gcov_dump_path, test_cases[i].flags)) {
            tests_passed++;
        }
    }
    
    // Summary
    printf("\n" "=" * 50 "\n");
    printf("Test Summary:\n");
    printf("  Total tests: %d\n", total_tests);
    printf("  Tests passed: %d\n", tests_passed);
    printf("  Tests failed: %d\n", total_tests - tests_passed);
    
    if (tests_passed > 0) {
        printf("\n✓ Successfully triggered the uncovered default case!\n");
        return EXIT_SUCCESS;
    } else {
        printf("\n✗ Failed to trigger the uncovered default case.\n");
        return EXIT_FAILURE;
    }
}
