/**
 * test_gcov_dump_flags.c
 * 
 * Test program to cover the default case in gcov-dump's flag parsing
 * that handles unknown single-character command-line flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define MAX_OUTPUT_LEN 4096

/**
 * Helper function to execute a command and capture its stderr output
 * Returns 1 if "unknown flag" message is found, 0 otherwise
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
    
    // Check return status
    int status = pclose(fp);
    int exit_code = WEXITSTATUS(status);
    
    // For invalid flag commands, check for error message
    if (expect_error) {
        if (strstr(output, "unknown flag") != NULL) {
            printf("✓ Found 'unknown flag' message\n");
            found = 1;
        } else {
            printf("✗ Did not find 'unknown flag' message\n");
            printf("Output was:\n%s\n", output);
        }
        
        // Also verify non-zero exit status
        if (exit_code == 0) {
            printf("✗ Expected non-zero exit code, got %d\n", exit_code);
            found = 0;
        } else {
            printf("✓ Got expected non-zero exit code: %d\n", exit_code);
        }
    } else {
        // For valid commands, just report success/failure
        if (exit_code == 0) {
            printf("✓ Command succeeded\n");
            found = 1;
        } else {
            printf("✗ Command failed with exit code %d\n", exit_code);
            printf("Output was:\n%s\n", output);
        }
    }
    
    return found;
}

/**
 * Create a simple test program that will generate GCOV data
 */
int create_test_program(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create test program");
        return 0;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    printf(\"Test program for GCOV data generation\\n\");\n");
    fprintf(fp, "    int x = 5;\n");
    fprintf(fp, "    if (x > 0) {\n");
    fprintf(fp, "        printf(\"x is positive\\n\");\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    for (int i = 0; i < 3; i++) {\n");
    fprintf(fp, "        printf(\"Iteration %%d\\n\", i);\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 1;
}

int main(int argc, char *argv[]) {
    char cmd[MAX_CMD_LEN];
    char helper_c[] = "gcov_test_helper.c";
    char helper_exe[] = "gcov_test_helper";
    char gcda_file[] = "gcov_test_helper.gcda";
    char gcno_file[] = "gcov_test_helper.gcno";
    int success = 1;
    
    printf("=== GCOV-Dump Flag Parser Coverage Test ===\n\n");
    
    // Step 1: Create and compile test program with GCOV instrumentation
    printf("1. Creating test program with GCOV instrumentation...\n");
    if (!create_test_program(helper_c)) {
        return 1;
    }
    
    // Compile with coverage flags
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -o %s %s", 
             helper_exe, helper_c);
    printf("Compiling: %s\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return 1;
    }
    
    // Run the test program to generate .gcda file
    printf("Running test program to generate GCOV data...\n");
    snprintf(cmd, sizeof(cmd), "./%s", helper_exe);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to run test program\n");
        return 1;
    }
    
    // Verify .gcda file was created
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "GCOV data file not created: %s\n", gcda_file);
        return 1;
    }
    printf("✓ GCOV data file created: %s\n\n", gcda_file);
    
    // Step 2: Test valid flag first (to ensure tool works)
    printf("2. Testing valid flag (-l) to verify gcov-dump works...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s 2>&1", gcda_file);
    if (!execute_and_check(cmd, 0)) {
        success = 0;
    }
    printf("\n");
    
    // Step 3: Test various invalid single-character flags
    printf("3. Testing invalid single-character flags (should trigger default case)...\n");
    
    // Test alphabetic characters not in {h, v, l, p, r, s}
    const char *invalid_flags[] = {"-a", "-b", "-c", "-d", "-e", "-f", "-g", 
                                   "-i", "-j", "-k", "-m", "-n", "-o", "-q",
                                   "-t", "-u", "-w", "-x", "-y", "-z"};
    
    for (int i = 0; i < sizeof(invalid_flags)/sizeof(invalid_flags[0]); i++) {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s 2>&1", 
                 invalid_flags[i], gcda_file);
        if (!execute_and_check(cmd, 1)) {
            success = 0;
        }
        printf("\n");
    }
    
    // Test non-alphabetic characters
    printf("Testing non-alphabetic invalid flags...\n");
    const char *non_alpha_flags[] = {"-1", "-2", "-@", "-#", "-$", "-%", "-&",
                                     "-*", "-(", "-)", "-_", "-=", "-+", "-[",
                                     "-]", "-{", "-}", "-|", "-\\", "-;", "-:",
                                     "-'", "-\"", "-<", "->", "-?", "-/", "-.",
                                     "-,", "-~", "-`", "-^"};
    
    for (int i = 0; i < sizeof(non_alpha_flags)/sizeof(non_alpha_flags[0]); i++) {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s 2>&1", 
                 non_alpha_flags[i], gcda_file);
        if (!execute_and_check(cmd, 1)) {
            success = 0;
        }
        printf("\n");
    }
    
    // Step 4: Test edge cases
    printf("4. Testing edge cases...\n");
    
    // Test with multiple invalid flags
    printf("Testing multiple invalid flags...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -a -b -c %s 2>&1", gcda_file);
    if (!execute_and_check(cmd, 1)) {
        success = 0;
    }
    printf("\n");
    
    // Test mixed valid and invalid flags
    printf("Testing mixed valid and invalid flags...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -x -p %s 2>&1", gcda_file);
    if (!execute_and_check(cmd, 1)) {
        success = 0;
    }
    printf("\n");
    
    // Test with .gcno file instead of .gcda
    printf("Testing with .gcno file...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -x %s 2>&1", gcno_file);
    if (!execute_and_check(cmd, 1)) {
        success = 0;
    }
    printf("\n");
    
    // Step 5: Cleanup
    printf("5. Cleaning up temporary files...\n");
    unlink(helper_c);
    unlink(helper_exe);
    unlink(gcda_file);
    unlink(gcno_file);
    
    // Also clean up any .gcov files that might have been created
    system("rm -f *.gcov");
    
    printf("\n=== Test %s ===\n", success ? "PASSED" : "FAILED");
    return success ? 0 : 1;
}
