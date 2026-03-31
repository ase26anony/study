/**
 * Test program to cover the default case in gcov-dump's option parsing.
 * This program:
 * 1. Creates a simple C program and compiles it with coverage flags
 * 2. Runs it to generate .gcda files
 * 3. Invokes gcov-dump with various invalid single-character flags
 * 4. Verifies the "unknown flag" error message is printed
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
 * Creates a simple C source file that will generate GCOV data
 */
int create_helper_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create helper source");
        return 0;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    printf(\"Generating GCOV data...\\n\");\n");
    fprintf(fp, "    int x = 0;\n");
    fprintf(fp, "    for (int i = 0; i < 10; i++) {\n");
    fprintf(fp, "        x += i;\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return x > 0 ? 0 : 1;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 1;
}

/**
 * Compiles the helper program with coverage instrumentation
 */
int compile_with_coverage(const char *source, const char *output) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s 2>&1",
             output, source);
    
    printf("Compiling: %s\n", cmd);
    int status = system(cmd);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

/**
 * Runs a command and captures its stderr output
 * Returns 1 if output contains "unknown flag", 0 otherwise
 */
int run_and_check_for_unknown_flag(const char *command) {
    char full_cmd[MAX_CMD];
    char buffer[1024];
    int found_flag_error = 0;
    
    // Redirect stderr to stdout and capture
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", command);
    
    printf("Executing: %s\n", command);
    
    FILE *pipe = popen(full_cmd, "r");
    if (!pipe) {
        perror("popen failed");
        return 0;
    }
    
    // Read command output
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        printf("  Output: %s", buffer);
        if (strstr(buffer, "unknown flag") != NULL) {
            found_flag_error = 1;
        }
    }
    
    int status = pclose(pipe);
    printf("  Exit status: %d\n", WEXITSTATUS(status));
    
    return found_flag_error;
}

/**
 * Main test execution
 */
int main(int argc, char *argv[]) {
    char helper_c[MAX_PATH] = "test_helper_gcov.c";
    char helper_exe[MAX_PATH] = "test_helper_gcov";
    char gcda_file[MAX_PATH];
    char cmd[MAX_CMD];
    int tests_passed = 0;
    int total_tests = 0;
    
    printf("=== GCOV-Dump Default Case Coverage Test ===\n\n");
    
    // Step 1: Create and compile helper program
    printf("1. Creating helper program...\n");
    if (!create_helper_source(helper_c)) {
        fprintf(stderr, "Failed to create helper source\n");
        return 1;
    }
    
    printf("2. Compiling with coverage instrumentation...\n");
    if (!compile_with_coverage(helper_c, helper_exe)) {
        fprintf(stderr, "Failed to compile helper program\n");
        unlink(helper_c);
        return 1;
    }
    
    // Step 2: Run helper to generate .gcda file
    printf("3. Running helper to generate GCOV data...\n");
    snprintf(cmd, sizeof(cmd), "./%s", helper_exe);
    system(cmd);
    
    // Construct path to .gcda file
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", helper_exe);
    
    // Check if .gcda file was created
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "No .gcda file generated at %s\n", gcda_file);
        // Try alternative location
        snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", helper_c);
        if (stat(gcda_file, &st) != 0) {
            fprintf(stderr, "No .gcda file found\n");
            // Cleanup and exit
            unlink(helper_c);
            unlink(helper_exe);
            return 1;
        }
    }
    
    printf("4. GCOV data file: %s\n\n", gcda_file);
    
    // Step 3: Test valid command first (to ensure tool works)
    printf("5. Testing valid command first...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_file);
    total_tests++;
    if (run_and_check_for_unknown_flag(cmd)) {
        printf("  ERROR: Valid flag triggered 'unknown flag' error!\n");
    } else {
        printf("  OK: Valid command executed without flag error\n");
        tests_passed++;
    }
    printf("\n");
    
    // Step 4: Test various invalid single-character flags
    // These should trigger the default case in the switch statement
    printf("6. Testing invalid single-character flags...\n");
    
    // Test invalid alphabetic characters
    char invalid_flags[] = "aAzZxXqQmM123?!@";
    for (int i = 0; invalid_flags[i] != '\0'; i++) {
        snprintf(cmd, sizeof(cmd), "gcov-dump -%c %s", 
                 invalid_flags[i], gcda_file);
        total_tests++;
        
        printf("\nTest flag '-%c':\n", invalid_flags[i]);
        if (run_and_check_for_unknown_flag(cmd)) {
            printf("  PASS: Correctly detected unknown flag '-%c'\n", 
                   invalid_flags[i]);
            tests_passed++;
        } else {
            printf("  FAIL: Did not detect unknown flag '-%c'\n", 
                   invalid_flags[i]);
        }
    }
    
    // Test edge cases
    printf("\n7. Testing edge cases...\n");
    
    // Test with just a dash (should be caught by getopt, but let's verify)
    snprintf(cmd, sizeof(cmd), "gcov-dump - %s", gcda_file);
    total_tests++;
    printf("\nTest flag '- ' (space after dash):\n");
    if (run_and_check_for_unknown_flag(cmd)) {
        printf("  PASS: Detected unknown flag\n");
        tests_passed++;
    } else {
        printf("  Note: Different error handling for '- '\n");
    }
    
    // Test multiple invalid flags in one call
    snprintf(cmd, sizeof(cmd), "gcov-dump -a -b -c %s", gcda_file);
    total_tests++;
    printf("\nTest multiple invalid flags '-a -b -c':\n");
    if (run_and_check_for_unknown_flag(cmd)) {
        printf("  PASS: Detected at least one unknown flag\n");
        tests_passed++;
    } else {
        printf("  FAIL: Did not detect unknown flags\n");
    }
    
    // Step 5: Cleanup
    printf("\n8. Cleaning up temporary files...\n");
    unlink(helper_c);
    unlink(helper_exe);
    unlink(gcda_file);
    
    // Also remove .gcno file if it exists
    char gcov_files[][MAX_PATH] = {
        "test_helper_gcov.gcno",
        "test_helper_gcov.c.gcno",
        "test_helper_gcov.gcda",
        "test_helper_gcov.c.gcda"
    };
    
    for (int i = 0; i < 4; i++) {
        if (stat(gcov_files[i], &st) == 0) {
            unlink(gcov_files[i]);
        }
    }
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    printf("Coverage target: default case in gcov-dump option parsing\n");
    
    if (tests_passed == total_tests) {
        printf("\nSUCCESS: All tests passed!\n");
        return 0;
    } else {
        printf("\nWARNING: Some tests failed\n");
        return 1;
    }
}
