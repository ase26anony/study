#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_OUTPUT 4096
#define DEFAULT_GCOV_DUMP_PATH "./gcc/gcov-dump"

/* Test cases for invalid flags */
static const char *invalid_flags[] = {
    "-x",        /* Simple invalid flag */
    "-z",        /* Another invalid flag */
    "-?",        /* Question mark flag */
    "-@",        /* Special character flag */
    "-1",        /* Numeric flag */
    "-l -x -p",  /* Mix of valid and invalid flags */
    "-x -l",     /* Invalid flag first */
    "-p -z -s",  /* Invalid flag in middle */
    "test.gcda -x", /* Invalid flag after filename */
    "--x",       /* Double dash with single char */
    NULL
};

/* Test cases for valid flags (to ensure program works normally) */
static const char *valid_flags[] = {
    "-h",
    "-v",
    "-l",
    "-p -r -s",
    NULL
};

/* Find gcov-dump executable */
static char *find_gcov_dump(void) {
    char *path = getenv("GCOV_DUMP");
    
    if (path && access(path, X_OK) == 0) {
        return strdup(path);
    }
    
    /* Try common build locations */
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "./gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i]; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            return strdup(common_paths[i]);
        }
    }
    
    return strdup(DEFAULT_GCOV_DUMP_PATH);
}

/* Execute command and capture stderr */
static int execute_and_capture(const char *cmd, char *output, size_t output_size) {
    FILE *fp;
    char buffer[256];
    int found_error = 0;
    
    /* Use popen to capture both stdout and stderr */
    char full_cmd[512];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    fp = popen(full_cmd, "r");
    if (!fp) {
        fprintf(stderr, "Failed to execute command: %s\n", cmd);
        return -1;
    }
    
    output[0] = '\0';
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        strncat(output, buffer, output_size - strlen(output) - 1);
        
        /* Check for the target error message */
        if (strstr(buffer, "unknown flag") != NULL) {
            found_error = 1;
        }
    }
    
    int status = pclose(fp);
    
    /* Return 1 if error message found, 0 if not found, -1 on execution error */
    if (WIFEXITED(status)) {
        return found_error ? 1 : 0;
    } else {
        return -1;
    }
}

/* Test a specific flag combination */
static int test_flag_combination(const char *gcov_dump_path, const char *flags) {
    char cmd[512];
    char output[MAX_OUTPUT];
    int result;
    
    printf("Testing: gcov-dump %s\n", flags);
    
    snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_path, flags);
    result = execute_and_capture(cmd, output, sizeof(output));
    
    if (result == 1) {
        printf("  ✓ Successfully triggered 'unknown flag' error\n");
        if (strlen(output) > 0) {
            printf("  Output: %s", output);
        }
        return 1;
    } else if (result == 0) {
        printf("  ✗ Did not trigger 'unknown flag' error\n");
        if (strlen(output) > 0) {
            printf("  Output: %s", output);
        }
        return 0;
    } else {
        printf("  ✗ Failed to execute command\n");
        return -1;
    }
}

/* Test valid flags to ensure program works */
static int test_valid_flags(const char *gcov_dump_path) {
    printf("\nTesting valid flags (sanity check):\n");
    
    for (int i = 0; valid_flags[i]; i++) {
        char cmd[512];
        char output[MAX_OUTPUT];
        
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_path, valid_flags[i]);
        printf("Testing: %s\n", cmd);
        
        if (execute_and_capture(cmd, output, sizeof(output)) >= 0) {
            printf("  ✓ Valid flag test passed\n");
        } else {
            printf("  ✗ Valid flag test failed\n");
            return 0;
        }
    }
    
    return 1;
}

int main(void) {
    char *gcov_dump_path;
    int success_count = 0;
    int total_tests = 0;
    
    printf("=== Testing gcov-dump invalid flag handling ===\n");
    
    /* Find gcov-dump executable */
    gcov_dump_path = find_gcov_dump();
    printf("Using gcov-dump at: %s\n", gcov_dump_path);
    
    /* Check if executable exists and is accessible */
    if (access(gcov_dump_path, X_OK) != 0) {
        fprintf(stderr, "Error: Cannot execute '%s': %s\n", 
                gcov_dump_path, strerror(errno));
        fprintf(stderr, "Please set GCOV_DUMP environment variable or\n");
        fprintf(stderr, "place gcov-dump in one of the common locations.\n");
        free(gcov_dump_path);
        return 1;
    }
    
    /* First, test with valid flags to ensure program works */
    if (!test_valid_flags(gcov_dump_path)) {
        fprintf(stderr, "Warning: Valid flag tests failed. Program may not be working.\n");
    }
    
    printf("\n=== Testing invalid flags ===\n");
    
    /* Test each invalid flag combination */
    for (int i = 0; invalid_flags[i]; i++) {
        int result = test_flag_combination(gcov_dump_path, invalid_flags[i]);
        total_tests++;
        
        if (result == 1) {
            success_count++;
        } else if (result == -1) {
            printf("  Skipping further tests due to execution failure\n");
            break;
        }
    }
    
    /* Additional edge case: empty arguments */
    printf("\nTesting edge case: no arguments\n");
    char output[MAX_OUTPUT];
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "%s", gcov_dump_path);
    if (execute_and_capture(cmd, output, sizeof(output)) >= 0) {
        printf("  ✓ No-argument test completed\n");
        total_tests++;
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Total tests attempted: %d\n", total_tests);
    printf("Successfully triggered 'unknown flag': %d\n", success_count);
    
    if (success_count > 0) {
        printf("\n✓ SUCCESS: Uncovered lines in gcov-dump.cc were executed!\n");
        free(gcov_dump_path);
        return 0;
    } else {
        printf("\n✗ FAILURE: Could not trigger the uncovered default case\n");
        free(gcov_dump_path);
        return 1;
    }
}
