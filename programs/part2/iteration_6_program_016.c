/**
 * test_gcov_dump_invalid_flags.c
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
 * Helper function to execute a command and capture its stderr output.
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
    
    // Read the output
    size_t bytes_read = fread(output, 1, sizeof(output) - 1, fp);
    output[bytes_read] = '\0';
    
    // Get the exit status
    int status = pclose(fp);
    
    if (expect_error) {
        // Check for "unknown flag" message in output
        if (strstr(output, "unknown flag") != NULL) {
            printf("  ✓ Found 'unknown flag' message\n");
            found = 1;
        } else {
            printf("  ✗ Did not find 'unknown flag' message\n");
            printf("  Output was: %s\n", output);
        }
        
        // Check for non-zero exit status
        if (WEXITSTATUS(status) != 0) {
            printf("  ✓ Command exited with non-zero status: %d\n", WEXITSTATUS(status));
        } else {
            printf("  ✗ Command exited with zero status (unexpected)\n");
        }
    } else {
        // For valid commands, just report success/failure
        if (WEXITSTATUS(status) == 0) {
            printf("  ✓ Valid command executed successfully\n");
            found = 1;
        } else {
            printf("  ✗ Valid command failed with status: %d\n", WEXITSTATUS(status));
            printf("  Output was: %s\n", output);
        }
    }
    
    return found;
}

/**
 * Create a simple C program that will generate GCOV data
 */
void create_helper_program(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create helper program");
        exit(1);
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    printf(\"Helper program executed.\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    printf("Created helper program: %s\n", filename);
}

int main(int argc, char *argv[]) {
    char cmd[MAX_CMD_LEN];
    char helper_c[] = "helper_gcov_test.c";
    char helper_exe[] = "helper_gcov_test";
    char gcda_file[] = "helper_gcov_test.gcda";
    char gcno_file[] = "helper_gcov_test.gcno";
    int tests_passed = 0;
    int total_tests = 0;
    
    printf("=== GCOV-Dump Invalid Flag Test ===\n\n");
    
    // Step 1: Create and compile helper program with coverage
    printf("1. Creating helper program with GCOV instrumentation...\n");
    create_helper_program(helper_c);
    
    // Compile with coverage flags
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -o %s %s", 
             helper_exe, helper_c);
    printf("Compiling: %s\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile helper program\n");
        return 1;
    }
    
    // Run the helper program to generate .gcda file
    printf("Running helper program to generate GCOV data...\n");
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
    printf("2. Testing valid command (gcov-dump -l)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s 2>&1", gcda_file);
    total_tests++;
    if (execute_and_check(cmd, 0)) {
        tests_passed++;
    }
    printf("\n");
    
    // Step 3: Test various invalid single-character flags
    printf("3. Testing invalid single-character flags...\n");
    
    // Test alphabetic characters not in {h, v, l, p, r, s}
    char invalid_flags[] = "abcdegijkmnoqtuwxyz";
    for (int i = 0; invalid_flags[i] != '\0'; i++) {
        char flag[3] = "- ";
        flag[1] = invalid_flags[i];
        
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s 2>&1", flag, gcda_file);
        total_tests++;
        if (execute_and_check(cmd, 1)) {
            tests_passed++;
        }
    }
    
    // Test non-alphabetic characters
    printf("\n4. Testing non-alphabetic invalid flags...\n");
    char special_flags[] = "?123!@#$%^&*()";
    for (int i = 0; special_flags[i] != '\0'; i++) {
        char flag[3] = "- ";
        flag[1] = special_flags[i];
        
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s 2>&1", flag, gcda_file);
        total_tests++;
        if (execute_and_check(cmd, 1)) {
            tests_passed++;
        }
    }
    
    // Step 4: Test combination of valid and invalid flags
    printf("\n5. Testing combination with valid flags...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -x %s 2>&1", gcda_file);
    total_tests++;
    if (execute_and_check(cmd, 1)) {
        tests_passed++;
    }
    
    // Test with multiple invalid flags
    snprintf(cmd, sizeof(cmd), "gcov-dump -a -b -c %s 2>&1", gcda_file);
    total_tests++;
    if (execute_and_check(cmd, 1)) {
        tests_passed++;
    }
    
    // Step 5: Cleanup
    printf("\n6. Cleaning up temporary files...\n");
    unlink(helper_c);
    unlink(helper_exe);
    unlink(gcda_file);
    unlink(gcno_file);
    unlink("helper_gcov_test.c.gcov");  // In case gcov was run
    
    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    
    if (tests_passed == total_tests) {
        printf("✓ All tests passed!\n");
        return 0;
    } else {
        printf("✗ Some tests failed\n");
        return 1;
    }
}
