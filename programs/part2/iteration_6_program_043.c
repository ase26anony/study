/**
 * test_gcov_dump_flags.c
 * 
 * Test program to cover the uncovered default case in gcov-dump.cc
 * that handles unknown single-character command-line flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_CMD_LEN 1024
#define MAX_OUTPUT_LEN 4096

/**
 * Helper function to execute a command and capture its output.
 * Returns 1 if "unknown flag" is found in stderr, 0 otherwise.
 */
int execute_and_check(const char *cmd, int expect_error) {
    char output[MAX_OUTPUT_LEN] = {0};
    FILE *fp;
    int found = 0;
    
    printf("Executing: %s\n", cmd);
    
    // Execute command and capture both stdout and stderr
    fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return 0;
    }
    
    // Read output
    size_t bytes_read = fread(output, 1, sizeof(output) - 1, fp);
    output[bytes_read] = '\0';
    
    // Get exit status
    int status = pclose(fp);
    
    // Check for "unknown flag" in output
    if (strstr(output, "unknown flag") != NULL) {
        found = 1;
        printf("  Found 'unknown flag' message\n");
    }
    
    // For invalid flags, we expect non-zero exit status
    if (expect_error) {
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            printf("  Got expected non-zero exit status: %d\n", WEXITSTATUS(status));
        } else {
            printf("  WARNING: Expected non-zero exit status but got %d\n", 
                   WIFEXITED(status) ? WEXITSTATUS(status) : -1);
        }
    }
    
    printf("\n");
    return found;
}

/**
 * Create a simple C program that will generate GCOV data.
 */
int create_helper_program(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create helper.c");
        return 0;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    printf(\"Helper program executed.\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 1;
}

/**
 * Compile the helper program with coverage instrumentation.
 */
int compile_helper_program(const char *source, const char *executable) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -o %s %s", 
             executable, source);
    
    printf("Compiling helper program: %s\n", cmd);
    int status = system(cmd);
    
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("Helper program compiled successfully.\n\n");
        return 1;
    } else {
        printf("Failed to compile helper program.\n");
        return 0;
    }
}

/**
 * Run the helper program to generate .gcda file.
 */
int run_helper_program(const char *executable) {
    printf("Running helper program to generate .gcda file...\n");
    int status = system(executable);
    
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("Helper program executed successfully.\n\n");
        return 1;
    } else {
        printf("Failed to run helper program.\n");
        return 0;
    }
}

int main(int argc, char *argv[]) {
    char helper_source[] = "helper.c";
    char helper_exec[] = "helper";
    char gcda_file[] = "helper.gcda";
    
    // Step 1: Create and compile helper program
    if (!create_helper_program(helper_source)) {
        return 1;
    }
    
    if (!compile_helper_program(helper_source, helper_exec)) {
        unlink(helper_source);
        return 1;
    }
    
    // Step 2: Run helper to generate .gcda file
    if (!run_helper_program(helper_exec)) {
        unlink(helper_source);
        unlink(helper_exec);
        return 1;
    }
    
    // Verify .gcda file exists
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        printf("ERROR: %s not created\n", gcda_file);
        unlink(helper_source);
        unlink(helper_exec);
        return 1;
    }
    
    printf("GCOV data file created: %s\n\n", gcda_file);
    
    // Step 3: Test gcov-dump with various flags
    int tests_passed = 0;
    int total_invalid_tests = 0;
    
    // First, test with a valid flag to ensure gcov-dump works
    printf("=== Testing with valid flag ===\n");
    char valid_cmd[MAX_CMD_LEN];
    snprintf(valid_cmd, sizeof(valid_cmd), "gcov-dump -l %s 2>&1", gcda_file);
    execute_and_check(valid_cmd, 0);
    
    // Test with multiple invalid single-character flags
    printf("=== Testing with invalid flags ===\n");
    
    // Array of invalid flags to test
    char invalid_flags[] = "aAzZ19?*x";
    for (int i = 0; invalid_flags[i] != '\0'; i++) {
        char test_cmd[MAX_CMD_LEN];
        snprintf(test_cmd, sizeof(test_cmd), "gcov-dump -%c %s 2>&1", 
                 invalid_flags[i], gcda_file);
        
        total_invalid_tests++;
        if (execute_and_check(test_cmd, 1)) {
            tests_passed++;
        }
    }
    
    // Test edge case: just a dash with no character (should also trigger error)
    printf("=== Testing edge cases ===\n");
    char edge_cmd[MAX_CMD_LEN];
    snprintf(edge_cmd, sizeof(edge_cmd), "gcov-dump - %s 2>&1", gcda_file);
    total_invalid_tests++;
    if (execute_and_check(edge_cmd, 1)) {
        tests_passed++;
    }
    
    // Test with multiple invalid flags in one call
    snprintf(edge_cmd, sizeof(edge_cmd), "gcov-dump -abc %s 2>&1", gcda_file);
    total_invalid_tests++;
    if (execute_and_check(edge_cmd, 1)) {
        tests_passed++;
    }
    
    // Summary
    printf("=== Test Summary ===\n");
    printf("Invalid flag tests passed: %d/%d\n", tests_passed, total_invalid_tests);
    
    if (tests_passed > 0) {
        printf("\nSUCCESS: Covered the 'default' case in gcov-dump.cc switch statement.\n");
        printf("The 'unknown flag' message was triggered %d times.\n", tests_passed);
    } else {
        printf("\nFAILURE: Did not trigger the 'unknown flag' error message.\n");
    }
    
    // Cleanup
    printf("\nCleaning up temporary files...\n");
    unlink(helper_source);
    unlink(helper_exec);
    unlink(gcda_file);
    
    // Also remove .gcno file created during compilation
    char gcov_files[MAX_CMD_LEN];
    snprintf(gcov_files, sizeof(gcov_files), "rm -f helper.gcno helper.gcda %s", helper_exec);
    system(gcov_files);
    
    return (tests_passed > 0) ? 0 : 1;
}
