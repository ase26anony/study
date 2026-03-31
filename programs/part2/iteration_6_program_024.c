/**
 * Test program to cover the default case in gcov-dump's option parser
 * Lines 111-130: default: fprintf(stderr, "unknown flag `%c'\n", opt);
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_PATH 1024
#define MAX_CMD 2048

/**
 * Execute a command and capture its stderr output
 * Returns 1 if "unknown flag" message found, 0 otherwise
 */
int execute_and_check(const char *cmd, const char *expected_substr) {
    char buffer[1024];
    int found = 0;
    
    printf("Executing: %s\n", cmd);
    
    // Execute command and capture both stdout and stderr
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return 0;
    }
    
    // Read output
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("  Output: %s", buffer);
        if (strstr(buffer, expected_substr) != NULL) {
            found = 1;
        }
    }
    
    int status = pclose(fp);
    printf("  Exit status: %d\n\n", WEXITSTATUS(status));
    
    return found;
}

/**
 * Create a simple C program that will generate GCOV data
 */
void create_helper_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("Failed to create helper source");
        exit(1);
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    printf(\"Helper program executed\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    printf("Created helper source: %s\n", filename);
}

int main(int argc, char *argv[]) {
    char helper_c[MAX_PATH] = "test_helper_gcov.c";
    char helper_exe[MAX_PATH] = "test_helper_gcov";
    char gcda_file[MAX_PATH];
    char cmd[MAX_CMD];
    char cwd[MAX_PATH];
    
    // Get current directory for absolute paths
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed");
        return 1;
    }
    
    printf("=== GCOV-DUMP Invalid Flag Test ===\n");
    printf("Working directory: %s\n\n", cwd);
    
    // Step 1: Create and compile helper program with GCOV instrumentation
    printf("1. Creating helper program with GCOV instrumentation...\n");
    create_helper_source(helper_c);
    
    // Compile with coverage flags
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 %s -o %s",
             helper_c, helper_exe);
    
    printf("Compiling: %s\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile helper program\n");
        return 1;
    }
    
    // Step 2: Run helper to generate .gcda file
    printf("\n2. Running helper to generate .gcda file...\n");
    snprintf(cmd, sizeof(cmd), "./%s", helper_exe);
    system(cmd);
    
    // Construct path to .gcda file
    snprintf(gcda_file, sizeof(gcda_file), "%s/%s.gcda", cwd, helper_exe);
    printf("GCOV data file: %s\n", gcda_file);
    
    // Verify .gcda file exists
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "GCOV data file not found: %s\n", gcda_file);
        return 1;
    }
    
    // Step 3: Test valid flag first (to ensure tool works)
    printf("\n3. Testing valid flag first...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s 2>&1", gcda_file);
    execute_and_check(cmd, "");
    
    // Step 4: Test various invalid single-character flags
    printf("\n4. Testing invalid single-character flags...\n");
    
    // Array of invalid flags to test
    char invalid_flags[] = "aAzZxX123?*";
    int total_tests = 0;
    int passed_tests = 0;
    
    for (int i = 0; invalid_flags[i] != '\0'; i++) {
        char flag[3] = "- ";
        flag[1] = invalid_flags[i];
        
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s 2>&1", flag, gcda_file);
        
        total_tests++;
        if (execute_and_check(cmd, "unknown flag")) {
            printf("✓ Flag '%c' correctly triggered 'unknown flag' error\n", invalid_flags[i]);
            passed_tests++;
        } else {
            printf("✗ Flag '%c' did NOT trigger expected error\n", invalid_flags[i]);
        }
    }
    
    // Step 5: Test edge cases
    printf("\n5. Testing edge cases...\n");
    
    // Test with just dash (should be treated as file argument, not flag)
    snprintf(cmd, sizeof(cmd), "gcov-dump - %s 2>&1", gcda_file);
    total_tests++;
    if (execute_and_check(cmd, "cannot open")) {
        printf("✓ Single dash correctly treated as filename\n");
        passed_tests++;
    }
    
    // Test with invalid flag in combination with valid flag
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -x %s 2>&1", gcda_file);
    total_tests++;
    if (execute_and_check(cmd, "unknown flag")) {
        printf("✓ Invalid flag '-x' detected even with valid '-l' flag\n");
        passed_tests++;
    }
    
    // Step 6: Cleanup
    printf("\n6. Cleaning up temporary files...\n");
    remove(helper_c);
    remove(helper_exe);
    remove(gcda_file);
    
    // Also remove .gcno file if it exists
    char gcov_files[MAX_PATH];
    snprintf(gcov_files, sizeof(gcov_files), "%s.gcno", helper_exe);
    remove(gcov_files);
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Coverage target: default case in gcov-dump option parser\n");
    
    if (passed_tests == total_tests) {
        printf("\n✅ All tests passed! The uncovered lines should now be covered.\n");
        return 0;
    } else {
        printf("\n⚠️  Some tests failed. Check the output above.\n");
        return 1;
    }
}
