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

#define MAX_CMD_LEN 1024
#define MAX_OUTPUT_LEN 4096

/**
 * Creates a temporary C source file for generating GCOV data.
 */
int create_helper_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create helper source file");
        return 0;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    printf(\"Helper program for GCOV data generation\\n\");\n");
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
 * Compiles the helper program with GCOV instrumentation.
 */
int compile_helper(const char *source_file, const char *executable) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -o %s %s", 
             executable, source_file);
    
    printf("Compiling helper: %s\n", cmd);
    int status = system(cmd);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

/**
 * Runs the helper program to generate .gcda file.
 */
int run_helper(const char *executable) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "./%s", executable);
    
    printf("Running helper: %s\n", cmd);
    int status = system(cmd);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

/**
 * Executes a gcov-dump command and checks for expected error message.
 * Returns 1 if the error message was found, 0 otherwise.
 */
int test_gcov_dump_command(const char *command, int expect_error) {
    char full_cmd[MAX_CMD_LEN];
    char output[MAX_OUTPUT_LEN];
    FILE *fp;
    int found_error = 0;
    
    // Redirect stderr to stdout and capture output
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", command);
    
    printf("\nTesting command: %s\n", command);
    
    fp = popen(full_cmd, "r");
    if (!fp) {
        perror("popen failed");
        return 0;
    }
    
    // Read all output
    size_t total_read = 0;
    while (fgets(output + total_read, sizeof(output) - total_read, fp) != NULL) {
        total_read += strlen(output + total_read);
        if (total_read >= sizeof(output) - 1) break;
    }
    
    int status = pclose(fp);
    
    // Check for "unknown flag" error message
    if (strstr(output, "unknown flag") != NULL) {
        printf("Found expected error message: %s", output);
        found_error = 1;
    } else if (strlen(output) > 0) {
        printf("Output: %s", output);
    }
    
    // Validate exit status
    if (expect_error) {
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            printf("Exit status: %d (non-zero as expected)\n", WEXITSTATUS(status));
        } else {
            printf("Warning: Expected non-zero exit status but got %d\n", 
                   WIFEXITED(status) ? WEXITSTATUS(status) : -1);
        }
    }
    
    return expect_error ? found_error : 1;
}

/**
 * Main test function.
 */
int main(int argc, char *argv[]) {
    const char *helper_source = "helper_gcov.c";
    const char *helper_exec = "helper_gcov";
    const char *gcda_file = "helper_gcov.gcda";
    char gcov_dump_cmd[MAX_CMD_LEN];
    int all_tests_passed = 1;
    
    printf("=== GCOV-Dump Invalid Flag Test ===\n");
    
    // Step 1: Create and compile helper program
    printf("\n1. Creating GCOV data source...\n");
    if (!create_helper_source(helper_source)) {
        return 1;
    }
    
    printf("2. Compiling with GCOV instrumentation...\n");
    if (!compile_helper(helper_source, helper_exec)) {
        fprintf(stderr, "Failed to compile helper program\n");
        return 1;
    }
    
    printf("3. Running helper to generate .gcda file...\n");
    if (!run_helper(helper_exec)) {
        fprintf(stderr, "Failed to run helper program\n");
        return 1;
    }
    
    // Verify .gcda file exists
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "GCOV data file not created: %s\n", gcda_file);
        return 1;
    }
    printf("GCOV data file created: %s\n", gcda_file);
    
    // Step 2: Test gcov-dump with various flags
    printf("\n4. Testing gcov-dump commands...\n");
    
    // First, test a valid command to ensure gcov-dump works
    printf("\n--- Testing valid flag ---\n");
    snprintf(gcov_dump_cmd, sizeof(gcov_dump_cmd), 
             "gcov-dump -l %s", gcda_file);
    if (!test_gcov_dump_command(gcov_dump_cmd, 0)) {
        printf("Valid flag test failed\n");
        all_tests_passed = 0;
    }
    
    // Test various invalid single-character flags
    printf("\n--- Testing invalid flags (should trigger default case) ---\n");
    
    // Test invalid alphabetic characters
    const char *invalid_flags[] = {"-a", "-z", "-x", "-q", "-1", "-?", "-@"};
    int num_invalid_flags = sizeof(invalid_flags) / sizeof(invalid_flags[0]);
    
    for (int i = 0; i < num_invalid_flags; i++) {
        snprintf(gcov_dump_cmd, sizeof(gcov_dump_cmd), 
                 "gcov-dump %s %s", invalid_flags[i], gcda_file);
        
        if (!test_gcov_dump_command(gcov_dump_cmd, 1)) {
            printf("FAIL: Invalid flag %s did not produce expected error\n", 
                   invalid_flags[i]);
            all_tests_passed = 0;
        } else {
            printf("PASS: Invalid flag %s correctly rejected\n", invalid_flags[i]);
        }
    }
    
    // Test combination: valid flag followed by invalid flag
    printf("\n--- Testing flag combination ---\n");
    snprintf(gcov_dump_cmd, sizeof(gcov_dump_cmd), 
             "gcov-dump -l -x %s", gcda_file);
    if (!test_gcov_dump_command(gcov_dump_cmd, 1)) {
        printf("Flag combination test failed\n");
        all_tests_passed = 0;
    }
    
    // Test invalid flag without data file (different error path)
    printf("\n--- Testing invalid flag without data file ---\n");
    if (!test_gcov_dump_command("gcov-dump -x", 1)) {
        printf("Invalid flag without file test failed\n");
        all_tests_passed = 0;
    }
    
    // Step 3: Cleanup
    printf("\n5. Cleaning up temporary files...\n");
    remove(helper_source);
    remove(helper_exec);
    remove(gcda_file);
    remove("helper_gcov.gcno");  // Also remove the .gcno file
    
    printf("\n=== Test %s ===\n", 
           all_tests_passed ? "PASSED" : "FAILED");
    
    return all_tests_passed ? 0 : 1;
}
