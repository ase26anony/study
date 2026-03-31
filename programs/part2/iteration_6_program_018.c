/**
 * Test program to cover the default case in gcov-dump's option parsing.
 * This triggers the "unknown flag" error message for invalid single-character flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_CMD_LEN 1024
#define MAX_OUTPUT_LEN 4096

/**
 * Execute a shell command and capture its output.
 * Returns 1 if output contains "unknown flag", 0 otherwise.
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
    
    if (expect_error) {
        // Check for "unknown flag" in output
        if (strstr(output, "unknown flag") != NULL) {
            printf("  ✓ Found 'unknown flag' message\n");
            found = 1;
        } else {
            printf("  ✗ Missing 'unknown flag' message\n");
            printf("  Output: %s\n", output);
        }
        
        // Check for non-zero exit status
        if (WEXITSTATUS(status) != 0) {
            printf("  ✓ Got non-zero exit status: %d\n", WEXITSTATUS(status));
        } else {
            printf("  ✗ Expected non-zero exit status, got: %d\n", WEXITSTATUS(status));
        }
    } else {
        // For valid commands, just report success/failure
        if (WEXITSTATUS(status) == 0) {
            printf("  ✓ Valid command executed successfully\n");
            found = 1;
        } else {
            printf("  ✗ Valid command failed with status: %d\n", WEXITSTATUS(status));
            printf("  Output: %s\n", output);
        }
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
    char helper_c[] = "test_helper_gcov.c";
    char helper_exe[] = "test_helper_gcov";
    char gcda_file[] = "test_helper_gcov.gcda";
    char gcno_file[] = "test_helper_gcov.gcno";
    int tests_passed = 0;
    int total_tests = 0;
    
    printf("=== GCOV-Dump Invalid Flag Test ===\n\n");
    
    // Step 1: Create and compile helper program with GCOV instrumentation
    printf("1. Creating helper program with GCOV instrumentation...\n");
    create_helper_source(helper_c);
    
    // Compile with coverage flags
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -o %s %s", 
             helper_exe, helper_c);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile helper program\n");
        return 1;
    }
    printf("Compiled helper program: %s\n", helper_exe);
    
    // Run helper to generate .gcda file
    snprintf(cmd, sizeof(cmd), "./%s", helper_exe);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to run helper program\n");
        return 1;
    }
    
    // Verify .gcda file was created
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "GCOV data file not created: %s\n", gcda_file);
        // Try .gcno file instead
        if (stat(gcno_file, &st) != 0) {
            fprintf(stderr, "No GCOV files created\n");
            return 1;
        }
        strcpy(gcda_file, gcno_file);
    }
    printf("GCOV data file available: %s\n\n", gcda_file);
    
    // Step 2: Test valid command first (to ensure tool works)
    printf("2. Testing valid command (gcov-dump -l)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s 2>&1", gcda_file);
    if (execute_and_check(cmd, 0)) {
        tests_passed++;
    }
    total_tests++;
    printf("\n");
    
    // Step 3: Test various invalid single-character flags
    printf("3. Testing invalid single-character flags (targeting default case)...\n");
    
    // Test invalid alphabetic characters
    const char *invalid_flags[] = {
        "-a",  // alphabetic, not in {h,v,l,p,r,s}
        "-z",  // another alphabetic
        "-x",  // common invalid flag
        "-c",  // another invalid
        "-?",  // non-alphabetic character
        "-1",  // numeric
        "-@",  // symbol
        "- ",  // space (though getopt might handle differently)
        NULL
    };
    
    for (int i = 0; invalid_flags[i] != NULL; i++) {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s 2>&1", 
                 invalid_flags[i], gcda_file);
        if (execute_and_check(cmd, 1)) {
            tests_passed++;
        }
        total_tests++;
    }
    printf("\n");
    
    // Step 4: Test combination of valid and invalid flags
    printf("4. Testing mixed valid and invalid flags...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -x %s 2>&1", gcda_file);
    if (execute_and_check(cmd, 1)) {
        tests_passed++;
    }
    total_tests++;
    
    // Step 5: Test just the invalid flag without data file
    printf("5. Testing invalid flag without data file...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -x 2>&1");
    if (execute_and_check(cmd, 1)) {
        tests_passed++;
    }
    total_tests++;
    
    // Step 6: Test double dash (long option) - should not trigger the default case
    printf("6. Testing double-dash (should not trigger default case)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump --help 2>&1");
    // This should not find "unknown flag" - it's a different code path
    if (!execute_and_check(cmd, 0)) {
        // Actually a success for this test - we don't want unknown flag here
        tests_passed++;
    }
    total_tests++;
    
    // Cleanup
    printf("\n7. Cleaning up...\n");
    remove(helper_c);
    remove(helper_exe);
    remove(gcda_file);
    remove(gcno_file);
    // Also remove any .gcov files
    system("rm -f *.gcov");
    
    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    
    return (tests_passed == total_tests) ? 0 : 1;
}
