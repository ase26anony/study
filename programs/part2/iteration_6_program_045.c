/**
 * Test program to cover the uncovered default case in gcov-dump.cc
 * which handles unknown single-character command-line flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_CMD_LEN 1024
#define MAX_OUTPUT_LEN 4096

/**
 * Creates a simple C source file for generating GCOV data
 */
void create_helper_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create helper source file");
        exit(1);
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    printf(\"Generating GCOV data...\\n\");\n");
    fprintf(fp, "    int x = 5;\n");
    fprintf(fp, "    if (x > 0) {\n");
    fprintf(fp, "        printf(\"x is positive\\n\");\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
}

/**
 * Executes a shell command and captures its output
 * Returns 1 if "unknown flag" is found in stderr, 0 otherwise
 */
int execute_and_check(const char *command, int expect_error) {
    char cmd_with_stderr[MAX_CMD_LEN];
    char output[MAX_OUTPUT_LEN];
    int found_unknown_flag = 0;
    
    // Redirect stderr to stdout for capture
    snprintf(cmd_with_stderr, sizeof(cmd_with_stderr), "%s 2>&1", command);
    
    printf("Executing: %s\n", command);
    
    FILE *fp = popen(cmd_with_stderr, "r");
    if (!fp) {
        perror("popen failed");
        return 0;
    }
    
    // Read the output
    size_t total_read = 0;
    while (fgets(output + total_read, sizeof(output) - total_read, fp) != NULL) {
        total_read += strlen(output + total_read);
        if (total_read >= sizeof(output) - 1) {
            break;
        }
    }
    
    int status = pclose(fp);
    
    // Check for "unknown flag" message
    if (strstr(output, "unknown flag") != NULL) {
        found_unknown_flag = 1;
        printf("  Found 'unknown flag' message as expected\n");
    }
    
    // Check exit status
    if (expect_error) {
        if (WEXITSTATUS(status) != 0) {
            printf("  Non-zero exit status as expected: %d\n", WEXITSTATUS(status));
        } else {
            printf("  WARNING: Expected non-zero exit status but got 0\n");
        }
    }
    
    printf("\n");
    return found_unknown_flag;
}

/**
 * Clean up temporary files
 */
void cleanup(const char *helper_c, const char *helper_exe, 
             const char *gcda_file, const char *gcno_file) {
    unlink(helper_c);
    unlink(helper_exe);
    unlink(gcda_file);
    unlink(gcno_file);
}

int main() {
    char helper_c[] = "/tmp/helper_gcov_XXXXXX.c";
    char helper_exe[] = "/tmp/helper_gcov_XXXXXX";
    char gcda_file[256];
    char gcno_file[256];
    
    // Create unique temporary filenames
    int fd = mkstemps(helper_c, 2);  // .c extension is 2 chars
    if (fd == -1) {
        perror("Failed to create temp file");
        return 1;
    }
    close(fd);
    
    // Create executable name (same base name without .c)
    strncpy(helper_exe, helper_c, strlen(helper_c) - 2);
    helper_exe[strlen(helper_c) - 2] = '\0';
    
    // Create GCOV data file names
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", helper_exe);
    snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", helper_exe);
    
    printf("=== GCOV Dump Invalid Flag Test ===\n");
    printf("Helper source: %s\n", helper_c);
    printf("Helper executable: %s\n", helper_exe);
    printf("GCOV data file: %s\n", gcda_file);
    printf("\n");
    
    // Step 1: Create helper source file
    create_helper_source(helper_c);
    
    // Step 2: Compile with GCOV instrumentation
    char compile_cmd[MAX_CMD_LEN];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -o %s %s",
             helper_exe, helper_c);
    
    printf("Compiling helper program...\n");
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile helper program\n");
        cleanup(helper_c, helper_exe, gcda_file, gcno_file);
        return 1;
    }
    
    // Step 3: Run helper to generate .gcda file
    printf("Running helper program to generate GCOV data...\n");
    if (system(helper_exe) != 0) {
        fprintf(stderr, "Failed to run helper program\n");
        cleanup(helper_c, helper_exe, gcda_file, gcno_file);
        return 1;
    }
    
    // Verify .gcda file was created
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "GCOV data file not created: %s\n", gcda_file);
        cleanup(helper_c, helper_exe, gcda_file, gcno_file);
        return 1;
    }
    
    printf("GCOV data file created successfully\n\n");
    
    // Step 4: Test various gcov-dump commands
    int tests_passed = 0;
    int total_tests = 0;
    
    // Test 1: Valid command (should work)
    printf("--- Test 1: Valid flag ---\n");
    char valid_cmd[MAX_CMD_LEN];
    snprintf(valid_cmd, sizeof(valid_cmd), "gcov-dump -l %s", gcda_file);
    execute_and_check(valid_cmd, 0);
    total_tests++;
    
    // Test 2-7: Invalid single-character flags
    // These should trigger the default case in the switch statement
    char invalid_flags[] = "a?x1z!";
    for (int i = 0; i < strlen(invalid_flags); i++) {
        printf("--- Test %d: Invalid flag '-%c' ---\n", i + 2, invalid_flags[i]);
        char invalid_cmd[MAX_CMD_LEN];
        snprintf(invalid_cmd, sizeof(invalid_cmd), 
                 "gcov-dump -%c %s", invalid_flags[i], gcda_file);
        
        if (execute_and_check(invalid_cmd, 1)) {
            tests_passed++;
        }
        total_tests++;
    }
    
    // Test 8: Multiple invalid flags (should also trigger error)
    printf("--- Test 8: Multiple invalid flags ---\n");
    char multi_invalid_cmd[MAX_CMD_LEN];
    snprintf(multi_invalid_cmd, sizeof(multi_invalid_cmd),
             "gcov-dump -a -b -c %s", gcda_file);
    if (execute_and_check(multi_invalid_cmd, 1)) {
        tests_passed++;
    }
    total_tests++;
    
    // Test 9: Valid flag combined with invalid flag
    printf("--- Test 9: Valid + invalid flag ---\n");
    char mixed_cmd[MAX_CMD_LEN];
    snprintf(mixed_cmd, sizeof(mixed_cmd),
             "gcov-dump -l -z %s", gcda_file);
    if (execute_and_check(mixed_cmd, 1)) {
        tests_passed++;
    }
    total_tests++;
    
    // Summary
    printf("=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    
    // Cleanup
    cleanup(helper_c, helper_exe, gcda_file, gcno_file);
    
    return (tests_passed == total_tests) ? 0 : 1;
}
