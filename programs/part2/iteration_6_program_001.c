/**
 * Test program to cover the default case in gcov-dump.cc switch statement
 * for handling unknown single-character flags.
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
 * Helper function to execute a command and capture its stderr output.
 * Returns 1 if "unknown flag" message is found in stderr, 0 otherwise.
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
    
    int status = pclose(fp);
    
    // Check for "unknown flag" message in output
    if (strstr(output, "unknown flag") != NULL) {
        printf("  ✓ Found 'unknown flag' message\n");
        found = 1;
    }
    
    // Check exit status
    if (expect_error) {
        if (WEXITSTATUS(status) != 0) {
            printf("  ✓ Command exited with non-zero status (as expected)\n");
        } else {
            printf("  ✗ Command exited with zero status (unexpected)\n");
        }
    }
    
    return found;
}

/**
 * Create a simple C program that will generate GCOV data
 */
void create_helper_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create helper source");
        exit(1);
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    printf(\"Helper program executed.\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    printf("Created helper source: %s\n", filename);
}

int main(int argc, char *argv[]) {
    char cmd[MAX_CMD_LEN];
    char helper_c[] = "test_helper.c";
    char helper_exe[] = "test_helper";
    char gcda_file[] = "test_helper.gcda";
    char gcno_file[] = "test_helper.gcno";
    int tests_passed = 0;
    int total_tests = 0;
    
    printf("=== GCOV-Dump Invalid Flag Test ===\n\n");
    
    // Step 1: Create and compile helper program with GCOV instrumentation
    printf("1. Creating helper program with GCOV instrumentation...\n");
    create_helper_source(helper_c);
    
    // Compile with coverage flags
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 %s -o %s",
             helper_c, helper_exe);
    
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile helper program\n");
        // Clean up and exit
        remove(helper_c);
        return 1;
    }
    printf("   Compiled: %s\n", helper_exe);
    
    // Run helper to generate .gcda file
    if (system(helper_exe) != 0) {
        fprintf(stderr, "Failed to run helper program\n");
    }
    printf("   Generated: %s\n", gcda_file);
    
    // Verify GCOV files exist
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "GCOV data file not created: %s\n", gcda_file);
        // Try .gcno file instead
        if (stat(gcno_file, &st) != 0) {
            fprintf(stderr, "No GCOV files found. Test cannot proceed.\n");
            // Clean up
            remove(helper_c);
            remove(helper_exe);
            return 1;
        }
        strcpy(gcda_file, gcno_file);
    }
    
    printf("\n2. Testing gcov-dump with various flags...\n\n");
    
    // Step 2: Test valid flag first (to ensure tool works)
    printf("--- Test 1: Valid flag (should succeed) ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s 2>&1", gcda_file);
    total_tests++;
    if (execute_and_check(cmd, 0)) {
        printf("  ✗ Unexpected 'unknown flag' message for valid flag '-l'\n");
    } else {
        printf("  ✓ Valid flag '-l' processed correctly\n");
        tests_passed++;
    }
    printf("\n");
    
    // Step 3: Test invalid alphabetic flags
    char invalid_flags[] = "abcdfgijkmnoqtuwyz";
    for (int i = 0; invalid_flags[i] != '\0'; i++) {
        printf("--- Test %d: Invalid flag '-%c' ---\n", 
               total_tests + 1, invalid_flags[i]);
        snprintf(cmd, sizeof(cmd), "gcov-dump -%c %s 2>&1", 
                 invalid_flags[i], gcda_file);
        total_tests++;
        if (execute_and_check(cmd, 1)) {
            tests_passed++;
        }
        printf("\n");
        
        // Test a few representative flags more thoroughly
        if (i >= 2) break; // Just test a few to keep output manageable
    }
    
    // Step 4: Test invalid non-alphabetic flags
    char special_flags[] = "?123@";
    for (int i = 0; special_flags[i] != '\0'; i++) {
        printf("--- Test %d: Invalid flag '-%c' (non-alphabetic) ---\n", 
               total_tests + 1, special_flags[i]);
        snprintf(cmd, sizeof(cmd), "gcov-dump -%c %s 2>&1", 
                 special_flags[i], gcda_file);
        total_tests++;
        if (execute_and_check(cmd, 1)) {
            tests_passed++;
        }
        printf("\n");
    }
    
    // Step 5: Test combination with valid flags (invalid flag first)
    printf("--- Test: Multiple flags with invalid first ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -x -l %s 2>&1", gcda_file);
    total_tests++;
    if (execute_and_check(cmd, 1)) {
        tests_passed++;
    }
    printf("\n");
    
    // Step 6: Test invalid flag in middle
    printf("--- Test: Multiple flags with invalid in middle ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -x -p %s 2>&1", gcda_file);
    total_tests++;
    if (execute_and_check(cmd, 1)) {
        tests_passed++;
    }
    printf("\n");
    
    // Step 7: Test just the invalid flag without data file
    printf("--- Test: Invalid flag without data file ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -x 2>&1");
    total_tests++;
    if (execute_and_check(cmd, 1)) {
        tests_passed++;
    }
    printf("\n");
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Tests executed: %d\n", total_tests);
    printf("Tests passed: %d\n", tests_passed);
    printf("Success rate: %.1f%%\n", 
           (total_tests > 0 ? (100.0 * tests_passed / total_tests) : 0));
    
    // Cleanup
    printf("\nCleaning up temporary files...\n");
    remove(helper_c);
    remove(helper_exe);
    remove(gcda_file);
    remove(gcno_file);
    
    // Also clean up any .gcov files that might have been created
    system("rm -f *.gcov 2>/dev/null");
    
    return (tests_passed == total_tests) ? 0 : 1;
}
