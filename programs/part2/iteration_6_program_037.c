/**
 * Test program to cover the default case in gcov-dump.cc switch statement
 * for handling unknown single-character flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_PATH 1024
#define MAX_CMD 2048

/**
 * Helper function to execute a command and capture its stderr output.
 * Returns 1 if "unknown flag" is found in stderr, 0 otherwise.
 */
int execute_and_check(const char *cmd, int expect_error) {
    char buffer[1024];
    int found = 0;
    
    printf("Executing: %s\n", cmd);
    
    // Execute command and capture both stdout and stderr
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return 0;
    }
    
    // Read the output
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (expect_error && strstr(buffer, "unknown flag") != NULL) {
            found = 1;
        }
        printf("  Output: %s", buffer);
    }
    
    int status = pclose(fp);
    int exit_code = WEXITSTATUS(status);
    
    if (expect_error) {
        printf("  Exit code: %d (expected non-zero for invalid flag)\n", exit_code);
        if (found) {
            printf("  ✓ Found 'unknown flag' message\n");
        } else {
            printf("  ✗ Did not find 'unknown flag' message\n");
        }
    } else {
        printf("  Exit code: %d (expected 0 for valid command)\n", exit_code);
    }
    
    printf("\n");
    return found;
}

/**
 * Create a simple C program that will generate GCOV data
 */
void create_helper_program(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("Failed to create helper.c");
        exit(1);
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    printf(\"Hello from helper program\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    printf("Created helper program: %s\n", filename);
}

int main(int argc, char *argv[]) {
    char helper_c[MAX_PATH];
    char helper_exe[MAX_PATH];
    char gcda_file[MAX_PATH];
    char cmd[MAX_CMD];
    char cwd[MAX_PATH];
    int i;
    
    // Get current working directory
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed");
        return 1;
    }
    
    // Create paths for helper files
    snprintf(helper_c, sizeof(helper_c), "%s/helper.c", cwd);
    snprintf(helper_exe, sizeof(helper_exe), "%s/helper", cwd);
    snprintf(gcda_file, sizeof(gcda_file), "%s/helper.gcda", cwd);
    
    // Step 1: Create and compile helper program with coverage
    printf("=== Step 1: Creating helper program with GCOV instrumentation ===\n");
    create_helper_program(helper_c);
    
    // Compile with coverage flags
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -o %s %s", 
             helper_exe, helper_c);
    printf("Compiling: %s\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile helper program\n");
        return 1;
    }
    
    // Run helper to generate .gcda file
    printf("Running helper to generate .gcda file\n");
    if (system(helper_exe) != 0) {
        fprintf(stderr, "Failed to run helper program\n");
        return 1;
    }
    
    // Verify .gcda file was created
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "Failed to create .gcda file: %s\n", gcda_file);
        return 1;
    }
    printf("Created GCOV data file: %s\n\n", gcda_file);
    
    // Step 2: Test gcov-dump with various flags
    printf("=== Step 2: Testing gcov-dump with various flags ===\n\n");
    
    // First, test a valid command to ensure gcov-dump works
    printf("--- Testing valid flag ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s 2>&1", gcda_file);
    execute_and_check(cmd, 0);
    
    // Test multiple invalid single-character flags
    char invalid_flags[] = "a?x9!z";
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("--- Testing invalid flags ---\n");
    for (i = 0; i < strlen(invalid_flags); i++) {
        snprintf(cmd, sizeof(cmd), "gcov-dump -%c %s 2>&1", 
                 invalid_flags[i], gcda_file);
        total_tests++;
        if (execute_and_check(cmd, 1)) {
            passed_tests++;
        }
    }
    
    // Test edge case: just a dash with no character
    printf("--- Testing edge case: dash with no character ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump - %s 2>&1", gcda_file);
    total_tests++;
    if (execute_and_check(cmd, 1)) {
        passed_tests++;
    }
    
    // Test combination: valid flag followed by invalid flag
    printf("--- Testing combination flag ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -la %s 2>&1", gcda_file);
    total_tests++;
    if (execute_and_check(cmd, 1)) {
        passed_tests++;
    }
    
    // Step 3: Cleanup
    printf("=== Step 3: Cleanup ===\n");
    printf("Removing temporary files...\n");
    
    unlink(helper_c);
    unlink(helper_exe);
    unlink(gcda_file);
    
    // Also remove other GCOV files that might have been created
    char gcno_file[MAX_PATH];
    snprintf(gcno_file, sizeof(gcno_file), "%s/helper.gcno", cwd);
    unlink(gcno_file);
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total invalid flag tests: %d\n", total_tests);
    printf("Tests that detected 'unknown flag': %d\n", passed_tests);
    
    if (passed_tests > 0) {
        printf("✓ Successfully covered the default case in gcov-dump switch statement\n");
        return 0;
    } else {
        printf("✗ Failed to trigger 'unknown flag' error\n");
        return 1;
    }
}
