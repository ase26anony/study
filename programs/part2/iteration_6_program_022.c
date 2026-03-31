/**
 * test_gcov_dump_flags.c
 * 
 * Test program to cover the default case in gcov-dump's argument parser
 * that handles unknown single-character flags.
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
 * Helper function to execute a command and capture its output
 * Returns 1 if "unknown flag" is found in stderr, 0 otherwise
 */
int execute_and_check(const char *cmd, int expect_error) {
    char output[MAX_OUTPUT_LEN] = {0};
    FILE *fp;
    int found = 0;
    
    printf("Executing: %s\n", cmd);
    
    // Execute command and capture both stdout and stderr
    fp = popen(cmd " 2>&1", "r");
    if (fp == NULL) {
        perror("popen failed");
        return 0;
    }
    
    // Read output
    size_t bytes_read = fread(output, 1, sizeof(output) - 1, fp);
    output[bytes_read] = '\0';
    
    // Check return status
    int status = pclose(fp);
    
    // Look for "unknown flag" in output
    if (strstr(output, "unknown flag") != NULL) {
        found = 1;
        printf("  Found 'unknown flag' message\n");
    }
    
    if (expect_error) {
        // For invalid flags, we expect non-zero exit and the error message
        if (found && WEXITSTATUS(status) != 0) {
            printf("  ✓ Correctly rejected invalid flag (exit code: %d)\n", WEXITSTATUS(status));
            return 1;
        } else {
            printf("  ✗ Expected error but didn't get it\n");
            printf("  Output: %s\n", output);
            return 0;
        }
    } else {
        // For valid commands, we expect success
        if (WEXITSTATUS(status) == 0) {
            printf("  ✓ Valid command executed successfully\n");
            return 1;
        } else {
            printf("  ✗ Valid command failed (exit code: %d)\n", WEXITSTATUS(status));
            printf("  Output: %s\n", output);
            return 0;
        }
    }
}

/**
 * Create a simple C program that will generate GCOV data
 */
int create_helper_program(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create helper program");
        return 0;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    printf(\"Helper program executed\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 1;
}

/**
 * Compile the helper program with coverage instrumentation
 */
int compile_helper_program(const char *source, const char *executable) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 %s -o %s",
             source, executable);
    
    printf("Compiling helper: %s\n", cmd);
    return system(cmd) == 0;
}

/**
 * Run the helper program to generate .gcda file
 */
int run_helper_program(const char *executable) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "./%s", executable);
    
    printf("Running helper: %s\n", cmd);
    return system(cmd) == 0;
}

int main(int argc, char *argv[]) {
    char helper_source[] = "test_helper_gcov.c";
    char helper_exec[] = "test_helper_gcov";
    char gcda_file[] = "test_helper_gcov.gcda";
    char gcov_dump_path[] = "gcov-dump";  // Adjust if gcov-dump is in a different location
    
    int tests_passed = 0;
    int total_tests = 0;
    char cmd[MAX_CMD_LEN];
    
    printf("=== GCOV-Dump Flag Parser Coverage Test ===\n\n");
    
    // Step 1: Create and compile helper program
    printf("1. Creating helper program...\n");
    if (!create_helper_program(helper_source)) {
        fprintf(stderr, "Failed to create helper program\n");
        return 1;
    }
    
    printf("2. Compiling helper with coverage...\n");
    if (!compile_helper_program(helper_source, helper_exec)) {
        fprintf(stderr, "Failed to compile helper program\n");
        return 1;
    }
    
    printf("3. Running helper to generate .gcda file...\n");
    if (!run_helper_program(helper_exec)) {
        fprintf(stderr, "Failed to run helper program\n");
        return 1;
    }
    
    // Verify .gcda file exists
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "GCOV data file not created: %s\n", gcda_file);
        return 1;
    }
    printf("   GCOV data file created: %s\n\n", gcda_file);
    
    // Step 2: Test valid flag first (to ensure tool works)
    printf("4. Testing valid flag (should succeed)...\n");
    snprintf(cmd, sizeof(cmd), "%s -l %s", gcov_dump_path, gcda_file);
    total_tests++;
    if (execute_and_check(cmd, 0)) {
        tests_passed++;
    }
    printf("\n");
    
    // Step 3: Test various invalid single-character flags
    printf("5. Testing invalid single-character flags (should fail with 'unknown flag')...\n");
    
    // Test alphabetic characters not in {h, v, l, p, r, s}
    char invalid_flags[] = "abcdfgijkmnoqtuwxyz";
    for (int i = 0; invalid_flags[i] != '\0'; i++) {
        // Test a few representative flags to avoid too many tests
        if (i % 3 == 0) {  // Test every 3rd flag
            snprintf(cmd, sizeof(cmd), "%s -%c %s", 
                    gcov_dump_path, invalid_flags[i], gcda_file);
            total_tests++;
            if (execute_and_check(cmd, 1)) {
                tests_passed++;
            }
        }
    }
    
    // Test specific edge cases
    printf("\n6. Testing edge cases...\n");
    
    // Test non-alphabetic character
    snprintf(cmd, sizeof(cmd), "%s -? %s", gcov_dump_path, gcda_file);
    total_tests++;
    if (execute_and_check(cmd, 1)) {
        tests_passed++;
    }
    
    // Test digit
    snprintf(cmd, sizeof(cmd), "%s -1 %s", gcov_dump_path, gcda_file);
    total_tests++;
    if (execute_and_check(cmd, 1)) {
        tests_passed++;
    }
    
    // Test uppercase letter (all valid flags are lowercase)
    snprintf(cmd, sizeof(cmd), "%s -A %s", gcov_dump_path, gcda_file);
    total_tests++;
    if (execute_and_check(cmd, 1)) {
        tests_passed++;
    }
    
    // Test another valid flag to ensure we didn't break anything
    printf("\n7. Testing another valid flag (verification)...\n");
    snprintf(cmd, sizeof(cmd), "%s -v %s", gcov_dump_path, gcda_file);
    total_tests++;
    if (execute_and_check(cmd, 0)) {
        tests_passed++;
    }
    
    // Step 4: Cleanup
    printf("\n8. Cleaning up temporary files...\n");
    remove(helper_source);
    remove(helper_exec);
    remove(gcda_file);
    remove("test_helper_gcov.gcno");  // Also remove the .gcno file
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    
    if (tests_passed == total_tests) {
        printf("✓ All tests passed! The default case in gcov-dump's flag parser should now be covered.\n");
        return 0;
    } else {
        printf("✗ Some tests failed\n");
        return 1;
    }
}
