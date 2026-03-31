/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Test program to cover the default case in gcov-dump's switch statement
 * for handling unknown single-character command-line flags.
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
 * Creates a simple C program that will generate GCOV data
 */
const char *helper_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Helper program for GCOV data generation\\n\");\n"
"    return 0;\n"
"}\n";

/**
 * Executes a shell command and captures its output
 * Returns: 0 on success, -1 on failure
 */
int execute_and_capture(const char *cmd, char *output, size_t output_size) {
    FILE *fp;
    char buffer[256];
    
    if (output_size > 0) {
        output[0] = '\0';
    }
    
    printf("Executing: %s\n", cmd);
    
    fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return -1;
    }
    
    // Read output if buffer provided
    if (output && output_size > 0) {
        size_t total_read = 0;
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            size_t len = strlen(buffer);
            if (total_read + len < output_size) {
                strcat(output, buffer);
                total_read += len;
            } else {
                break;
            }
        }
    } else {
        // Just consume output
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            // Do nothing, just read
        }
    }
    
    int status = pclose(fp);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/**
 * Creates a temporary file with the given content
 */
int create_temp_file(const char *filename, const char *content) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("Failed to create temp file");
        return -1;
    }
    
    fputs(content, fp);
    fclose(fp);
    return 0;
}

int main(int argc, char *argv[]) {
    char cmd[MAX_CMD_LEN];
    char output[MAX_OUTPUT_LEN];
    int status;
    int tests_passed = 0;
    int total_tests = 0;
    
    printf("=== Test program for gcov-dump invalid flag coverage ===\n\n");
    
    // Step 1: Create and compile helper program to generate GCOV data
    printf("1. Creating helper program for GCOV data generation...\n");
    
    if (create_temp_file("helper_gcov.c", helper_source) != 0) {
        fprintf(stderr, "Failed to create helper source file\n");
        return 1;
    }
    
    // Compile helper with coverage instrumentation
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage helper_gcov.c -o helper_gcov 2>&1");
    status = execute_and_capture(cmd, output, sizeof(output));
    
    if (status != 0) {
        fprintf(stderr, "Failed to compile helper program:\n%s\n", output);
        // Clean up and exit
        unlink("helper_gcov.c");
        return 1;
    }
    
    // Run helper to generate .gcda file
    printf("2. Running helper program to generate .gcda file...\n");
    snprintf(cmd, sizeof(cmd), "./helper_gcov");
    status = execute_and_capture(cmd, NULL, 0);
    
    if (status != 0) {
        fprintf(stderr, "Helper program execution failed\n");
        unlink("helper_gcov.c");
        unlink("helper_gcov");
        return 1;
    }
    
    // Verify .gcda file was created
    struct stat st;
    if (stat("helper_gcov.gcda", &st) != 0) {
        fprintf(stderr, "No .gcda file generated\n");
        // Try alternative name
        if (stat("helper_gcov.c.gcda", &st) != 0) {
            fprintf(stderr, "Could not find any .gcda file\n");
            unlink("helper_gcov.c");
            unlink("helper_gcov");
            return 1;
        } else {
            strcpy(cmd, "helper_gcov.c.gcda");
        }
    } else {
        strcpy(cmd, "helper_gcov.gcda");
    }
    
    printf("3. GCOV data file created: %s\n\n", cmd);
    char *gcda_file = strdup(cmd);
    
    // Step 2: Test valid flag first (to ensure tool works)
    printf("4. Testing with valid flag (-l) first...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s 2>&1", gcda_file);
    status = execute_and_capture(cmd, output, sizeof(output));
    
    if (status == 0) {
        printf("✓ Valid flag test passed\n");
        tests_passed++;
    } else {
        printf("✗ Valid flag test failed (status: %d)\n", status);
        // Continue anyway - maybe gcov-dump is not in PATH
    }
    total_tests++;
    
    // Step 3: Test various invalid flags to trigger the default case
    printf("\n5. Testing invalid flags to trigger default case...\n");
    
    // Array of invalid flags to test
    char invalid_flags[] = {'a', 'z', 'x', '?', '1', '!', '@', 'q', 'w', 'e'};
    int num_invalid_flags = sizeof(invalid_flags) / sizeof(invalid_flags[0]);
    
    for (int i = 0; i < num_invalid_flags; i++) {
        printf("  Testing invalid flag '-%c'... ", invalid_flags[i]);
        
        snprintf(cmd, sizeof(cmd), "gcov-dump -%c %s 2>&1", 
                 invalid_flags[i], gcda_file);
        status = execute_and_capture(cmd, output, sizeof(output));
        
        // Check for expected error message
        if (strstr(output, "unknown flag") != NULL) {
            printf("✓ Triggered 'unknown flag' error\n");
            tests_passed++;
        } else if (strstr(output, "unrecognized option") != NULL) {
            printf("✓ Triggered 'unrecognized option' error (alternative)\n");
            tests_passed++;
        } else if (strstr(output, "invalid option") != NULL) {
            printf("✓ Triggered 'invalid option' error (alternative)\n");
            tests_passed++;
        } else {
            printf("✗ No expected error message (status: %d)\n", status);
            printf("  Output: %s\n", output);
        }
        total_tests++;
    }
    
    // Step 4: Test edge cases
    printf("\n6. Testing edge cases...\n");
    
    // Test with multiple invalid flags
    printf("  Testing '-ab' (multiple invalid flags)... ");
    snprintf(cmd, sizeof(cmd), "gcov-dump -ab %s 2>&1", gcda_file);
    status = execute_and_capture(cmd, output, sizeof(output));
    
    if (strstr(output, "unknown flag") != NULL || 
        strstr(output, "unrecognized") != NULL ||
        strstr(output, "invalid") != NULL) {
        printf("✓ Triggered error\n");
        tests_passed++;
    } else {
        printf("✗ No error triggered\n");
    }
    total_tests++;
    
    // Test with just a dash
    printf("  Testing lone '-'... ");
    snprintf(cmd, sizeof(cmd), "gcov-dump - %s 2>&1", gcda_file);
    status = execute_and_capture(cmd, output, sizeof(output));
    
    // This might trigger different error, but should still fail
    if (status != 0) {
        printf("✓ Failed as expected (status: %d)\n", status);
        tests_passed++;
    } else {
        printf("✗ Unexpected success\n");
    }
    total_tests++;
    
    // Step 5: Cleanup
    printf("\n7. Cleaning up temporary files...\n");
    unlink("helper_gcov.c");
    unlink("helper_gcov");
    unlink(gcda_file);
    unlink("helper_gcov.gcno");  // Also remove .gcno if it exists
    unlink("helper_gcov.c.gcno");
    
    free(gcda_file);
    
    // Summary
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
