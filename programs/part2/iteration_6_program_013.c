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
 * Helper function to execute a command and capture its output.
 * Returns 1 if the output contains the expected error message.
 */
int execute_and_check(const char *cmd, const char *expected_error) {
    char buffer[1024];
    int found = 0;
    
    printf("Executing: %s\n", cmd);
    
    // Use popen to capture both stdout and stderr
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return 0;
    }
    
    // Read the output
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("Output: %s", buffer);
        if (expected_error && strstr(buffer, expected_error)) {
            found = 1;
        }
    }
    
    // Get the exit status
    int status = pclose(fp);
    int exit_code = WEXITSTATUS(status);
    
    printf("Exit code: %d\n\n", exit_code);
    
    // For invalid flags, we expect non-zero exit and error message
    if (expected_error) {
        return (found && exit_code != 0);
    }
    
    return 1;
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
}

int main() {
    char helper_c[MAX_PATH] = "test_helper_gcov.c";
    char helper_exe[MAX_PATH] = "test_helper_gcov";
    char gcda_file[MAX_PATH];
    char cmd[MAX_CMD];
    int ret;
    
    printf("=== GCOV-DUMP Invalid Flag Test ===\n\n");
    
    // Step 1: Create helper C program
    printf("1. Creating helper C program...\n");
    create_helper_program(helper_c);
    
    // Step 2: Compile with coverage flags
    printf("2. Compiling helper program with coverage instrumentation...\n");
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 %s -o %s",
             helper_c, helper_exe);
    
    ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Failed to compile helper program\n");
        return 1;
    }
    
    // Step 3: Run helper program to generate .gcda file
    printf("3. Running helper program to generate .gcda file...\n");
    snprintf(cmd, sizeof(cmd), "./%s", helper_exe);
    ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Failed to run helper program\n");
        return 1;
    }
    
    // The .gcda file will have the same name as the source
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", helper_c);
    
    // Check if .gcda file exists
    if (access(gcda_file, F_OK) != 0) {
        // Try alternative naming
        snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", helper_exe);
        if (access(gcda_file, F_OK) != 0) {
            fprintf(stderr, "No .gcda file found. Trying to find any .gcda...\n");
            system("find . -name \"*.gcda\" -type f | head -5");
            // Use a generic approach
            strcpy(gcda_file, "test_helper_gcov.gcda");
        }
    }
    
    printf("Using GCOV data file: %s\n\n", gcda_file);
    
    // Step 4: Test valid flag first (to ensure tool works)
    printf("4. Testing valid flag (-l) first...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s 2>&1", gcda_file);
    if (!execute_and_check(cmd, NULL)) {
        printf("WARNING: Valid flag test failed. gcov-dump may not be in PATH.\n");
        printf("Trying with ./gcov-dump...\n");
        snprintf(cmd, sizeof(cmd), "./gcov-dump -l %s 2>&1", gcda_file);
        execute_and_check(cmd, NULL);
    }
    
    // Step 5: Test various invalid single-character flags
    printf("5. Testing invalid single-character flags (targeting default case)...\n");
    
    // Array of invalid flags to test
    char invalid_flags[] = "aAzZxXcC?1!@#$%^&*()";
    int num_tests = 0;
    int passed_tests = 0;
    
    for (int i = 0; invalid_flags[i] != '\0'; i++) {
        char flag[3] = "- ";
        flag[1] = invalid_flags[i];
        
        // Skip if it's actually a valid flag
        if (strchr("hvlprs", invalid_flags[i]) != NULL) {
            continue;
        }
        
        num_tests++;
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s 2>&1", flag, gcda_file);
        
        if (execute_and_check(cmd, "unknown flag")) {
            printf("✓ Flag '%c' correctly triggered error\n", invalid_flags[i]);
            passed_tests++;
        } else {
            printf("✗ Flag '%c' did not trigger expected error\n", invalid_flags[i]);
        }
    }
    
    // Step 6: Test edge cases
    printf("\n6. Testing edge cases...\n");
    
    // Test with just a dash
    printf("\nTesting with just '-' (no character):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump - %s 2>&1", gcda_file);
    execute_and_check(cmd, NULL);
    
    // Test with multiple invalid flags
    printf("\nTesting with multiple invalid flags:\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -a -b -c %s 2>&1", gcda_file);
    execute_and_check(cmd, "unknown flag");
    
    // Test with valid flag followed by invalid flag
    printf("\nTesting with valid flag followed by invalid flag:\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -x %s 2>&1", gcda_file);
    execute_and_check(cmd, "unknown flag");
    
    // Step 7: Cleanup
    printf("\n7. Cleaning up temporary files...\n");
    unlink(helper_c);
    unlink(helper_exe);
    
    // Remove GCOV files
    char rm_cmd[MAX_CMD];
    snprintf(rm_cmd, sizeof(rm_cmd), "rm -f *.gcda *.gcno *.gcov");
    system(rm_cmd);
    
    printf("\n=== Test Summary ===\n");
    printf("Invalid flag tests executed: %d\n", num_tests);
    printf("Tests passed: %d\n", passed_tests);
    
    if (passed_tests > 0) {
        printf("\n✅ Successfully covered the default case in gcov-dump.cc!\n");
        printf("The 'unknown flag' error message was triggered as expected.\n");
    } else {
        printf("\n❌ No tests triggered the expected error message.\n");
        printf("This could mean:\n");
        printf("1. gcov-dump is not in PATH\n");
        printf("2. The version of gcov-dump has different error handling\n");
        printf("3. The .gcda file was not properly generated\n");
    }
    
    return (passed_tests > 0) ? 0 : 1;
}
