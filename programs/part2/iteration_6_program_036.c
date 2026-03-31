/**
 * test_gcov_dump_flags.c
 * 
 * Test program to cover the default case in gcov-dump's flag parsing logic.
 * This program:
 * 1. Creates a simple C program with GCOV instrumentation
 * 2. Compiles and runs it to generate .gcda files
 * 3. Tests gcov-dump with various invalid flags to trigger the "unknown flag" error
 * 4. Also tests valid flags to ensure normal operation
 * 5. Verifies error messages and exit codes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH 1024
#define MAX_CMD 2048

/**
 * Helper function to execute a command and capture its output
 * Returns 0 on success, non-zero on failure
 */
int execute_and_capture(const char *cmd, char *output, size_t output_size, int *exit_status) {
    FILE *fp;
    char buffer[256];
    size_t total_read = 0;
    
    // Open pipe to read both stdout and stderr
    fp = popen(cmd, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to execute command: %s\n", cmd);
        return -1;
    }
    
    // Read output
    while (fgets(buffer, sizeof(buffer), fp) != NULL && total_read < output_size - 1) {
        size_t len = strlen(buffer);
        if (total_read + len < output_size) {
            strcpy(output + total_read, buffer);
            total_read += len;
        }
    }
    
    // Get exit status
    *exit_status = pclose(fp);
    
    return 0;
}

/**
 * Check if a file exists
 */
int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

/**
 * Create a simple C program for GCOV testing
 */
int create_test_program(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create test program");
        return -1;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    printf(\"Hello from GCOV test program!\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/**
 * Clean up temporary files
 */
void cleanup(const char *base_name) {
    char cmd[MAX_CMD];
    
    // Remove compiled files
    snprintf(cmd, sizeof(cmd), "rm -f %s %s.c %s.gcno %s.gcda %s.gcov",
             base_name, base_name, base_name, base_name, base_name);
    system(cmd);
}

int main(int argc, char *argv[]) {
    char helper_c_file[MAX_PATH];
    char helper_exe[MAX_PATH];
    char gcda_file[MAX_PATH];
    char cmd[MAX_CMD];
    char output[4096];
    int exit_status;
    int test_passed = 1;
    
    // Use a unique name for the helper program
    const char *base_name = "gcov_test_helper";
    
    // Create file paths
    snprintf(helper_c_file, sizeof(helper_c_file), "%s.c", base_name);
    snprintf(helper_exe, sizeof(helper_exe), "./%s", base_name);
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", base_name);
    
    printf("=== GCOV-DUMP Flag Coverage Test ===\n\n");
    
    // Step 1: Create a simple C program
    printf("1. Creating test program: %s\n", helper_c_file);
    if (create_test_program(helper_c_file) != 0) {
        fprintf(stderr, "Failed to create test program\n");
        return 1;
    }
    
    // Step 2: Compile with GCOV instrumentation
    printf("2. Compiling with GCOV instrumentation\n");
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 %s -o %s 2>&1",
             helper_c_file, base_name);
    
    if (execute_and_capture(cmd, output, sizeof(output), &exit_status) != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        cleanup(base_name);
        return 1;
    }
    
    if (exit_status != 0) {
        fprintf(stderr, "Compilation failed:\n%s\n", output);
        cleanup(base_name);
        return 1;
    }
    
    // Step 3: Run the program to generate .gcda file
    printf("3. Running test program to generate .gcda file\n");
    if (!file_exists(helper_exe)) {
        fprintf(stderr, "Executable not found: %s\n", helper_exe);
        cleanup(base_name);
        return 1;
    }
    
    snprintf(cmd, sizeof(cmd), "%s 2>&1", helper_exe);
    if (execute_and_capture(cmd, output, sizeof(output), &exit_status) != 0) {
        fprintf(stderr, "Failed to run test program\n");
        cleanup(base_name);
        return 1;
    }
    
    if (!file_exists(gcda_file)) {
        fprintf(stderr, "GCOV data file not created: %s\n", gcda_file);
        cleanup(base_name);
        return 1;
    }
    
    printf("   Generated: %s\n", gcda_file);
    
    // Step 4: Test gcov-dump with valid flag first
    printf("\n4. Testing gcov-dump with valid flag (-l)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s 2>&1", gcda_file);
    memset(output, 0, sizeof(output));
    
    if (execute_and_capture(cmd, output, sizeof(output), &exit_status) != 0) {
        fprintf(stderr, "Failed to execute gcov-dump\n");
        cleanup(base_name);
        return 1;
    }
    
    if (exit_status != 0) {
        fprintf(stderr, "Valid flag test failed (exit status: %d)\n", exit_status);
        fprintf(stderr, "Output:\n%s\n", output);
        test_passed = 0;
    } else {
        printf("   ✓ Valid flag test passed\n");
    }
    
    // Step 5: Test with various invalid flags to cover the default case
    printf("\n5. Testing gcov-dump with invalid flags (to cover default case)\n");
    
    // Array of invalid flags to test
    const char *invalid_flags[] = {
        "-a",  // alphabetic, not in {h,v,l,p,r,s}
        "-z",  // another alphabetic
        "-1",  // numeric
        "-?",  // special character
        "-x",  // another alphabetic
        "-A",  // uppercase
        "-@",  // special character
        NULL
    };
    
    for (int i = 0; invalid_flags[i] != NULL; i++) {
        printf("   Testing flag: %s\n", invalid_flags[i]);
        
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s 2>&1", 
                 invalid_flags[i], gcda_file);
        memset(output, 0, sizeof(output));
        
        if (execute_and_capture(cmd, output, sizeof(output), &exit_status) != 0) {
            fprintf(stderr, "Failed to execute gcov-dump with flag %s\n", invalid_flags[i]);
            test_passed = 0;
            continue;
        }
        
        // Check for the expected error message
        if (strstr(output, "unknown flag") != NULL) {
            printf("     ✓ Found 'unknown flag' error message\n");
            
            // Verify non-zero exit status
            if (exit_status != 0) {
                printf("     ✓ Non-zero exit status: %d\n", exit_status);
            } else {
                printf("     ✗ Expected non-zero exit status, got: %d\n", exit_status);
                test_passed = 0;
            }
        } else {
            printf("     ✗ Did not find 'unknown flag' error message\n");
            printf("       Output:\n%s\n", output);
            test_passed = 0;
        }
    }
    
    // Step 6: Test combination of valid and invalid flags
    printf("\n6. Testing combination of valid and invalid flags\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -x %s 2>&1", gcda_file);
    memset(output, 0, sizeof(output));
    
    if (execute_and_capture(cmd, output, sizeof(output), &exit_status) != 0) {
        fprintf(stderr, "Failed to execute gcov-dump with combined flags\n");
        test_passed = 0;
    } else if (strstr(output, "unknown flag") != NULL && exit_status != 0) {
        printf("   ✓ Combined flag test correctly rejected invalid flag\n");
    } else {
        printf("   ✗ Combined flag test failed\n");
        printf("     Output:\n%s\n", output);
        test_passed = 0;
    }
    
    // Step 7: Test with only invalid flag (no gcda file)
    printf("\n7. Testing with invalid flag only (no file argument)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -x 2>&1");
    memset(output, 0, sizeof(output));
    
    if (execute_and_capture(cmd, output, sizeof(output), &exit_status) != 0) {
        fprintf(stderr, "Failed to execute gcov-dump with invalid flag only\n");
        test_passed = 0;
    } else if (strstr(output, "unknown flag") != NULL && exit_status != 0) {
        printf("   ✓ Invalid flag only test passed\n");
    } else {
        printf("   ✗ Invalid flag only test failed\n");
        printf("     Output:\n%s\n", output);
        test_passed = 0;
    }
    
    // Cleanup
    printf("\n8. Cleaning up temporary files\n");
    cleanup(base_name);
    
    // Final result
    printf("\n=== Test Result: %s ===\n", 
           test_passed ? "PASSED" : "FAILED");
    
    return test_passed ? 0 : 1;
}
