/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Test program to cover the default case in gcov-dump's option parser
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
 * Helper function to execute a command and capture its stderr output.
 * Returns 1 if "unknown flag" message is found in stderr, 0 otherwise.
 */
int execute_and_check(const char *cmd, int expect_failure) {
    char output[MAX_OUTPUT_LEN] = {0};
    char full_cmd[MAX_CMD_LEN];
    FILE *fp;
    int found = 0;
    
    // Build command with stderr redirected to stdout
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    printf("Executing: %s\n", cmd);
    
    fp = popen(full_cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return 0;
    }
    
    // Read all output
    size_t bytes_read = fread(output, 1, sizeof(output) - 1, fp);
    output[bytes_read] = '\0';
    
    int status = pclose(fp);
    
    // Check for "unknown flag" message
    if (strstr(output, "unknown flag") != NULL) {
        printf("  ✓ Found 'unknown flag' message\n");
        found = 1;
    } else {
        printf("  ✗ No 'unknown flag' message found\n");
    }
    
    // Check exit status
    if (expect_failure) {
        if (WEXITSTATUS(status) != 0) {
            printf("  ✓ Command failed as expected (exit code: %d)\n", WEXITSTATUS(status));
        } else {
            printf("  ✗ Command succeeded unexpectedly\n");
        }
    }
    
    if (bytes_read > 0) {
        printf("  Output: %s", output);
    }
    
    printf("\n");
    
    return found;
}

/**
 * Create a simple C program that will generate GCOV data
 */
int create_gcov_helper(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create helper source");
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

int main(int argc, char *argv[]) {
    char cmd[MAX_CMD_LEN];
    char helper_source[] = "test_helper_gcov.c";
    char helper_binary[] = "test_helper_gcov";
    char gcda_file[] = "test_helper_gcov.gcda";
    char gcno_file[] = "test_helper_gcov.gcno";
    int tests_passed = 0;
    int total_invalid_tests = 0;
    
    printf("=== GCOV-Dump Invalid Flag Test ===\n\n");
    
    // Step 1: Create and compile helper program with coverage
    printf("1. Creating helper program with GCOV instrumentation...\n");
    if (!create_gcov_helper(helper_source)) {
        return 1;
    }
    
    // Compile with coverage flags
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 %s -o %s",
             helper_source, helper_binary);
    
    printf("Compiling: %s\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile helper program\n");
        return 1;
    }
    
    // Run helper to generate .gcda file
    printf("Running helper to generate GCOV data...\n");
    if (system(helper_binary) != 0) {
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
    
    // Step 2: Test valid flag first (to ensure tool works)
    printf("2. Testing valid flag (should succeed)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_file);
    execute_and_check(cmd, 0);
    
    // Step 3: Test various invalid single-character flags
    printf("3. Testing invalid single-character flags (should trigger default case)...\n");
    
    // Array of invalid flags to test
    char invalid_flags[] = "aAzZxX1?*@";
    
    for (int i = 0; invalid_flags[i] != '\0'; i++) {
        char flag_str[3] = "- ";
        flag_str[1] = invalid_flags[i];
        
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s", flag_str, gcda_file);
        
        total_invalid_tests++;
        if (execute_and_check(cmd, 1)) {
            tests_passed++;
        }
    }
    
    // Test edge cases
    printf("4. Testing edge cases...\n");
    
    // Test with just dash (should be caught by getopt, but good to check)
    snprintf(cmd, sizeof(cmd), "gcov-dump - %s", gcda_file);
    total_invalid_tests++;
    if (execute_and_check(cmd, 1)) {
        tests_passed++;
    }
    
    // Test with non-ASCII character (simulated with '?')
    snprintf(cmd, sizeof(cmd), "gcov-dump -? %s", gcda_file);
    total_invalid_tests++;
    if (execute_and_check(cmd, 1)) {
        tests_passed++;
    }
    
    // Test with number
    snprintf(cmd, sizeof(cmd), "gcov-dump -9 %s", gcda_file);
    total_invalid_tests++;
    if (execute_and_check(cmd, 1)) {
        tests_passed++;
    }
    
    // Step 4: Test combination of valid and invalid flags
    printf("5. Testing flag combinations...\n");
    
    // Valid flag followed by invalid flag
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -q %s", gcda_file);
    total_invalid_tests++;
    if (execute_and_check(cmd, 1)) {
        tests_passed++;
    }
    
    // Invalid flag followed by valid flag
    snprintf(cmd, sizeof(cmd), "gcov-dump -q -l %s", gcda_file);
    total_invalid_tests++;
    if (execute_and_check(cmd, 1)) {
        tests_passed++;
    }
    
    // Step 5: Cleanup
    printf("6. Cleaning up...\n");
    unlink(helper_source);
    unlink(helper_binary);
    unlink(gcda_file);
    unlink(gcno_file);
    
    // Also remove any .gcov files that might have been created
    system("rm -f *.gcov");
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Invalid flag tests executed: %d\n", total_invalid_tests);
    printf("Tests that triggered 'unknown flag' message: %d\n", tests_passed);
    
    if (tests_passed > 0) {
        printf("\n✅ SUCCESS: Successfully triggered the default case in gcov-dump's option parser!\n");
        printf("   The lines handling unknown single-character flags should now be covered.\n");
        return 0;
    } else {
        printf("\n❌ FAILURE: Could not trigger the default case\n");
        return 1;
    }
}
