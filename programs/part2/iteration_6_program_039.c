/**
 * Test program to cover the uncovered default case in gcov-dump.cc
 * Lines 111-130: default case for unknown single-character flags
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_PATH 1024
#define MAX_CMD 2048

/**
 * Execute a command and capture its stderr output
 * Returns: exit status of the command
 */
int execute_and_capture(const char *cmd, char *output, size_t output_size) {
    FILE *fp;
    char buffer[256];
    int status;
    
    // Clear output buffer
    output[0] = '\0';
    
    // Execute command and capture both stdout and stderr
    char full_cmd[MAX_CMD];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    fp = popen(full_cmd, "r");
    if (fp == NULL) {
        snprintf(output, output_size, "Failed to execute command");
        return -1;
    }
    
    // Read all output
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        strncat(output, buffer, output_size - strlen(output) - 1);
    }
    
    status = pclose(fp);
    return WEXITSTATUS(status);
}

/**
 * Check if output contains the expected error message
 */
int contains_unknown_flag(const char *output) {
    return strstr(output, "unknown flag") != NULL;
}

/**
 * Create a simple C program for generating GCOV data
 */
int create_helper_program(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create helper.c");
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
 * Compile helper program with GCOV instrumentation
 */
int compile_helper_program(const char *source, const char *executable) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 %s -o %s 2>&1",
             source, executable);
    
    int status = system(cmd);
    return WEXITSTATUS(status) == 0;
}

/**
 * Run helper program to generate .gcda file
 */
int run_helper_program(const char *executable) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "%s 2>&1", executable);
    
    int status = system(cmd);
    return WEXITSTATUS(status) == 0;
}

int main(int argc, char *argv[]) {
    char helper_c[MAX_PATH] = "/tmp/helper_gcov_test.c";
    char helper_exe[MAX_PATH] = "/tmp/helper_gcov_test";
    char gcda_file[MAX_PATH];
    char output[1024];
    int passed = 0;
    int failed = 0;
    
    printf("=== GCOV-Dump Unknown Flag Test ===\n\n");
    
    // Step 1: Create and compile helper program
    printf("1. Creating helper program...\n");
    if (!create_helper_program(helper_c)) {
        fprintf(stderr, "Failed to create helper program\n");
        return 1;
    }
    
    printf("2. Compiling helper program with GCOV instrumentation...\n");
    if (!compile_helper_program(helper_c, helper_exe)) {
        fprintf(stderr, "Failed to compile helper program\n");
        unlink(helper_c);
        return 1;
    }
    
    // Step 2: Run helper to generate .gcda file
    printf("3. Running helper program to generate .gcda file...\n");
    if (!run_helper_program(helper_exe)) {
        fprintf(stderr, "Failed to run helper program\n");
        unlink(helper_c);
        unlink(helper_exe);
        return 1;
    }
    
    // Construct .gcda filename
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", helper_exe);
    
    // Verify .gcda file exists
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "GCOV data file not found: %s\n", gcda_file);
        unlink(helper_c);
        unlink(helper_exe);
        return 1;
    }
    
    printf("4. GCOV data file created: %s\n\n", gcda_file);
    
    // Step 3: Test valid command first (to ensure tool works)
    printf("5. Testing valid command (gcov-dump -l)...\n");
    char valid_cmd[MAX_CMD];
    snprintf(valid_cmd, sizeof(valid_cmd), "gcov-dump -l %s", gcda_file);
    
    int valid_status = execute_and_capture(valid_cmd, output, sizeof(output));
    if (valid_status == 0) {
        printf("   ✓ Valid command executed successfully\n");
        passed++;
    } else {
        printf("   ✗ Valid command failed (status: %d)\n", valid_status);
        printf("   Output: %s\n", output);
        failed++;
    }
    
    // Step 4: Test various invalid single-character flags
    printf("\n6. Testing invalid single-character flags...\n");
    
    // Array of invalid flags to test
    char invalid_flags[][3] = {
        "-a",  // alphabetic, not in {h,v,l,p,r,s}
        "-z",  // another alphabetic
        "-x",  // another alphabetic
        "-1",  // numeric
        "-?",  // special character
        "-@",  // special character
        "- ",  // space (edge case)
        "--",  // double dash (should be caught earlier, but test anyway)
        "-",   // just dash (no character)
    };
    
    int num_tests = sizeof(invalid_flags) / sizeof(invalid_flags[0]);
    
    for (int i = 0; i < num_tests; i++) {
        char test_cmd[MAX_CMD];
        snprintf(test_cmd, sizeof(test_cmd), "gcov-dump %s %s", 
                 invalid_flags[i], gcda_file);
        
        printf("   Testing flag '%s'... ", invalid_flags[i]);
        
        int status = execute_and_capture(test_cmd, output, sizeof(output));
        
        // Check for expected error message
        if (contains_unknown_flag(output)) {
            printf("✓ Got 'unknown flag' error\n");
            passed++;
        } else if (status != 0) {
            // Non-zero exit is also acceptable (tool rejected invalid flag)
            printf("✓ Non-zero exit status (%d)\n", status);
            passed++;
        } else {
            printf("✗ Unexpected success\n");
            printf("   Output: %s\n", output);
            failed++;
        }
    }
    
    // Step 5: Test combination of valid and invalid flags
    printf("\n7. Testing combination of valid and invalid flags...\n");
    char combo_cmd[MAX_CMD];
    snprintf(combo_cmd, sizeof(combo_cmd), 
             "gcov-dump -l -x -p %s", gcda_file);
    
    printf("   Testing 'gcov-dump -l -x -p' (valid, invalid, valid)... ");
    
    int combo_status = execute_and_capture(combo_cmd, output, sizeof(output));
    
    if (contains_unknown_flag(output)) {
        printf("✓ Got 'unknown flag' error (for -x)\n");
        passed++;
    } else if (combo_status != 0) {
        printf("✓ Non-zero exit status (%d)\n", combo_status);
        passed++;
    } else {
        printf("✗ Unexpected success\n");
        printf("   Output: %s\n", output);
        failed++;
    }
    
    // Step 6: Test flag at different positions
    printf("\n8. Testing invalid flag at different positions...\n");
    
    // Invalid flag after filename
    char pos_cmd1[MAX_CMD];
    snprintf(pos_cmd1, sizeof(pos_cmd1), 
             "gcov-dump %s -q", gcda_file);
    
    printf("   Testing 'gcov-dump <file> -q'... ");
    
    int pos_status1 = execute_and_capture(pos_cmd1, output, sizeof(output));
    
    if (contains_unknown_flag(output)) {
        printf("✓ Got 'unknown flag' error\n");
        passed++;
    } else if (pos_status1 != 0) {
        printf("✓ Non-zero exit status (%d)\n", pos_status1);
        passed++;
    } else {
        printf("✗ Unexpected success\n");
        failed++;
    }
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    printf("Total:  %d\n\n", passed + failed);
    
    // Cleanup
    printf("Cleaning up temporary files...\n");
    unlink(helper_c);
    unlink(helper_exe);
    unlink(gcda_file);
    
    // Also remove .gcno file if it exists
    char gcov_files[MAX_PATH];
    snprintf(gcov_files, sizeof(gcov_files), "%s.gcno", helper_exe);
    unlink(gcov_files);
    
    return failed > 0 ? 1 : 0;
}
