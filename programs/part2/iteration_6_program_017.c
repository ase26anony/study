/**
 * Test program to cover the uncovered default case in gcov-dump.cc
 * which handles unknown single-character command-line flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024
#define MAX_CMD 2048

/**
 * Simple helper program that will be compiled with coverage flags
 * to generate .gcda files for gcov-dump to process.
 */
const char *helper_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Helper program executed.\\n\");\n"
"    return 0;\n"
"}\n";

/**
 * Execute a shell command and capture its output.
 * Returns the command's exit status.
 */
int execute_command(const char *cmd, char *output, size_t output_size) {
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        return -1;
    }
    
    if (output && output_size > 0) {
        output[0] = '\0';
        size_t total_read = 0;
        while (!feof(fp) && total_read < output_size - 1) {
            size_t read = fread(output + total_read, 1, output_size - total_read - 1, fp);
            total_read += read;
        }
        output[total_read] = '\0';
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
}

/**
 * Check if a string contains a substring.
 */
int contains_string(const char *str, const char *substr) {
    return strstr(str, substr) != NULL;
}

/**
 * Create a temporary file with the given content.
 */
int create_temp_file(const char *filename, const char *content) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        return 0;
    }
    fputs(content, fp);
    fclose(fp);
    return 1;
}

int main(int argc, char *argv[]) {
    char cwd[MAX_PATH];
    char helper_c_path[MAX_PATH];
    char helper_exe_path[MAX_PATH];
    char gcda_path[MAX_PATH];
    char cmd[MAX_CMD];
    char output[4096];
    int status;
    int tests_passed = 0;
    int total_tests = 0;
    
    // Get current working directory
    if (!getcwd(cwd, sizeof(cwd))) {
        fprintf(stderr, "Failed to get current directory\n");
        return 1;
    }
    
    // Create paths for helper files
    snprintf(helper_c_path, sizeof(helper_c_path), "%s/helper_gcov_test.c", cwd);
    snprintf(helper_exe_path, sizeof(helper_exe_path), "%s/helper_gcov_test", cwd);
    snprintf(gcda_path, sizeof(gcda_path), "%s/helper_gcov_test.gcda", cwd);
    
    printf("=== GCOV-Dump Invalid Flag Test ===\n");
    printf("Working directory: %s\n", cwd);
    
    // Step 1: Create helper source file
    printf("\n1. Creating helper source file...\n");
    if (!create_temp_file(helper_c_path, helper_source)) {
        fprintf(stderr, "Failed to create helper source file\n");
        return 1;
    }
    printf("   Created: %s\n", helper_c_path);
    
    // Step 2: Compile helper with coverage flags
    printf("\n2. Compiling helper with coverage instrumentation...\n");
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -o %s %s 2>&1",
             helper_exe_path, helper_c_path);
    
    status = execute_command(cmd, output, sizeof(output));
    if (status != 0) {
        fprintf(stderr, "Compilation failed:\n%s\n", output);
        // Clean up and exit
        remove(helper_c_path);
        return 1;
    }
    printf("   Compiled: %s\n", helper_exe_path);
    
    // Step 3: Run helper to generate .gcda file
    printf("\n3. Running helper to generate .gcda file...\n");
    snprintf(cmd, sizeof(cmd), "%s 2>&1", helper_exe_path);
    status = execute_command(cmd, output, sizeof(output));
    if (status != 0) {
        fprintf(stderr, "Helper execution failed:\n%s\n", output);
    } else {
        printf("   Generated: %s\n", gcda_path);
        
        // Verify .gcda file exists
        struct stat st;
        if (stat(gcda_path, &st) == 0 && st.st_size > 0) {
            printf("   Verified: .gcda file exists (%ld bytes)\n", st.st_size);
        } else {
            fprintf(stderr, "ERROR: .gcda file not created or empty\n");
        }
    }
    
    // Step 4: Test valid command first (to ensure tool works)
    printf("\n4. Testing valid gcov-dump command...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s 2>&1", gcda_path);
    total_tests++;
    
    status = execute_command(cmd, output, sizeof(output));
    if (status == 0) {
        printf("   ✓ Valid command succeeded\n");
        tests_passed++;
    } else {
        printf("   ✗ Valid command failed (status: %d)\n", status);
        printf("   Output: %s\n", output);
    }
    
    // Step 5: Test various invalid single-character flags
    printf("\n5. Testing invalid single-character flags...\n");
    
    // Array of invalid flags to test
    const char *invalid_flags[] = {
        "-a",  // alphabetic, not in {h,v,l,p,r,s}
        "-z",  // another alphabetic
        "-x",  // another alphabetic
        "-1",  // numeric
        "-?",  // symbol
        "-@",  // symbol
        "- ",  // space (edge case)
        NULL
    };
    
    for (int i = 0; invalid_flags[i] != NULL; i++) {
        total_tests++;
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s 2>&1", 
                 invalid_flags[i], gcda_path);
        
        status = execute_command(cmd, output, sizeof(output));
        
        // Check for expected error message
        if (contains_string(output, "unknown flag") || 
            contains_string(output, "unknown option") ||
            contains_string(output, "invalid option")) {
            printf("   ✓ Invalid flag %s triggered error\n", invalid_flags[i]);
            tests_passed++;
            
            // Print first line of error for verification
            char *first_line = strtok(output, "\n");
            if (first_line) {
                printf("     Error: %s\n", first_line);
            }
        } else {
            printf("   ✗ Invalid flag %s did not trigger expected error\n", 
                   invalid_flags[i]);
            printf("     Status: %d, Output: %s\n", status, 
                   output[0] ? output : "(empty)");
        }
    }
    
    // Step 6: Test combination of valid and invalid flags
    printf("\n6. Testing combination of valid and invalid flags...\n");
    total_tests++;
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -x %s 2>&1", gcda_path);
    
    status = execute_command(cmd, output, sizeof(output));
    if (contains_string(output, "unknown flag") || 
        contains_string(output, "unknown option") ||
        contains_string(output, "invalid option")) {
        printf("   ✓ Mixed flags triggered error for invalid flag\n");
        tests_passed++;
    } else {
        printf("   ✗ Mixed flags did not trigger expected error\n");
    }
    
    // Step 7: Test flag without required argument
    printf("\n7. Testing flag without required .gcda file...\n");
    total_tests++;
    snprintf(cmd, sizeof(cmd), "gcov-dump -x 2>&1");
    
    status = execute_command(cmd, output, sizeof(output));
    // This might trigger a different error (missing file), but should still
    // process the invalid flag first
    if (contains_string(output, "unknown flag") || 
        contains_string(output, "unknown option") ||
        contains_string(output, "invalid option") ||
        contains_string(output, "no input files")) {
        printf("   ✓ Invalid flag without file argument handled\n");
        tests_passed++;
    } else {
        printf("   ✗ Unexpected behavior without file argument\n");
    }
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    
    // Cleanup
    printf("\nCleaning up temporary files...\n");
    remove(helper_c_path);
    remove(helper_exe_path);
    remove(gcda_path);
    
    // Also remove other GCOV files that might have been created
    char gcno_path[MAX_PATH];
    snprintf(gcno_path, sizeof(gcno_path), "%s/helper_gcov_test.gcno", cwd);
    remove(gcno_path);
    
    // Remove any .gcov files
    snprintf(cmd, sizeof(cmd), "rm -f %s/*.gcov 2>/dev/null", cwd);
    system(cmd);
    
    printf("Cleanup complete.\n");
    
    return (tests_passed == total_tests) ? 0 : 1;
}
