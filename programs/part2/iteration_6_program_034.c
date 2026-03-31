/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Test program to trigger the uncovered default case in gcov-dump.cc
 * when processing invalid single-character command-line flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

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
    
    // For invalid flag commands, check for "unknown flag" message
    if (expect_error) {
        if (strstr(output, "unknown flag") != NULL) {
            printf("  ✓ Found 'unknown flag' message\n");
            found = 1;
        } else {
            printf("  ✗ Missing 'unknown flag' message\n");
            printf("    Output: %s\n", output);
        }
        
        // Also verify non-zero exit status
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            printf("  ✓ Command exited with non-zero status: %d\n", WEXITSTATUS(status));
        } else {
            printf("  ✗ Expected non-zero exit status\n");
        }
    } else {
        // For valid commands, just show success
        printf("  ✓ Valid command executed\n");
    }
    
    return found;
}

/**
 * Create a simple C program that will generate GCOV data.
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
    char helper_c[] = "helper_gcov.c";
    char helper_exe[] = "helper_gcov";
    char gcda_file[] = "helper_gcov.gcda";
    char gcno_file[] = "helper_gcov.gcno";
    int tests_passed = 0;
    int total_invalid_tests = 0;
    
    printf("=== GCOV-Dump Invalid Flag Test ===\n\n");
    
    // Step 1: Create and compile helper program with GCOV instrumentation
    printf("1. Creating helper program with GCOV instrumentation...\n");
    create_helper_source(helper_c);
    
    // Compile with coverage flags
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -o %s %s", 
             helper_exe, helper_c);
    printf("Compiling: %s\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile helper program\n");
        return 1;
    }
    
    // Run helper to generate .gcda file
    printf("Running helper to generate GCOV data...\n");
    if (system(helper_exe) != 0) {
        fprintf(stderr, "Failed to run helper program\n");
        return 1;
    }
    
    // Verify GCOV files were created
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "GCOV data file not created: %s\n", gcda_file);
        return 1;
    }
    printf("GCOV data file created: %s\n\n", gcda_file);
    
    // Step 2: Test valid command first (to ensure tool works)
    printf("2. Testing valid command (baseline)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_file);
    execute_and_check(cmd, 0);
    printf("\n");
    
    // Step 3: Test various invalid single-character flags
    printf("3. Testing invalid single-character flags...\n");
    
    // Array of invalid flags to test
    char invalid_flags[] = "aAbBcCdDeEfFgGiIjJkKmMnNoOqQtTuUwWxXyYzZ0123456789!@#$%^&*()_+[]{}|;:,.<>?";
    
    for (int i = 0; invalid_flags[i] != '\0'; i++) {
        char flag[3] = "- ";
        flag[1] = invalid_flags[i];
        
        // Skip if flag is actually valid (h, v, l, p, r, s)
        if (strchr("hvlprs", invalid_flags[i]) != NULL) {
            continue;
        }
        
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s", flag, gcda_file);
        total_invalid_tests++;
        
        if (execute_and_check(cmd, 1)) {
            tests_passed++;
        }
        
        // Limit output for readability - test a few representative cases
        if (i >= 10 && i < strlen(invalid_flags) - 1) {
            // Skip some to keep output manageable
            continue;
        }
    }
    
    // Step 4: Test edge cases
    printf("\n4. Testing edge cases...\n");
    
    // Test with just a dash
    snprintf(cmd, sizeof(cmd), "gcov-dump - %s", gcda_file);
    total_invalid_tests++;
    if (execute_and_check(cmd, 1)) {
        tests_passed++;
    }
    
    // Test with space after dash (should be treated as filename)
    snprintf(cmd, sizeof(cmd), "gcov-dump - %s", gcda_file);
    total_invalid_tests++;
    if (execute_and_check(cmd, 1)) {
        tests_passed++;
    }
    
    // Test multiple invalid flags
    snprintf(cmd, sizeof(cmd), "gcov-dump -a -b %s", gcda_file);
    total_invalid_tests++;
    if (execute_and_check(cmd, 1)) {
        tests_passed++;
    }
    
    // Test invalid flag combined with valid flag
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -x %s", gcda_file);
    total_invalid_tests++;
    if (execute_and_check(cmd, 1)) {
        tests_passed++;
    }
    
    // Step 5: Summary
    printf("\n5. Test Summary:\n");
    printf("   Total invalid flag tests: %d\n", total_invalid_tests);
    printf("   Tests that triggered 'unknown flag': %d\n", tests_passed);
    printf("   Success rate: %.1f%%\n", 
           total_invalid_tests > 0 ? (100.0 * tests_passed / total_invalid_tests) : 0.0);
    
    if (tests_passed > 0) {
        printf("\n✓ SUCCESS: Successfully triggered the uncovered default case!\n");
    } else {
        printf("\n✗ FAILURE: Could not trigger the uncovered default case\n");
    }
    
    // Step 6: Cleanup
    printf("\n6. Cleaning up temporary files...\n");
    remove(helper_c);
    remove(helper_exe);
    remove(gcda_file);
    remove(gcno_file);
    printf("Cleanup complete.\n");
    
    return (tests_passed > 0) ? 0 : 1;
}
