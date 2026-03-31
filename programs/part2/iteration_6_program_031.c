/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Test program to trigger the uncovered default case in gcov-dump.cc
 * when invalid single-character flags are provided.
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
 * Execute a command and capture its stderr output.
 * Returns 1 if "unknown flag" message is found, 0 otherwise.
 */
int execute_and_check(const char *cmd, int expect_failure) {
    char output[MAX_OUTPUT_LEN] = {0};
    FILE *fp;
    int found = 0;
    
    printf("Executing: %s\n", cmd);
    
    // Execute command and capture both stdout and stderr
    fp = popen(cmd " 2>&1", "r");
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
    
    // Look for "unknown flag" message
    if (strstr(output, "unknown flag") != NULL) {
        printf("  ✓ Found 'unknown flag' message\n");
        found = 1;
    } else {
        printf("  ✗ No 'unknown flag' message found\n");
    }
    
    // Check exit code matches expectation
    if (expect_failure) {
        if (exit_code != 0) {
            printf("  ✓ Command failed as expected (exit code: %d)\n", exit_code);
        } else {
            printf("  ✗ Command succeeded unexpectedly\n");
        }
    } else {
        if (exit_code == 0) {
            printf("  ✓ Command succeeded as expected\n");
        } else {
            printf("  ✗ Command failed unexpectedly (exit code: %d)\n", exit_code);
        }
    }
    
    // Print first few lines of output if any
    if (strlen(output) > 0) {
        printf("  Output (first 200 chars):\n  ");
        for (int i = 0; i < 200 && output[i] != '\0'; i++) {
            putchar(output[i]);
            if (output[i] == '\n' && i < 199) printf("  ");
        }
        printf("\n");
    }
    
    printf("\n");
    return found;
}

/**
 * Create a simple C program that will generate GCOV data.
 */
int create_helper_program(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create helper program");
        return 0;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    printf(\"Helper program executed.\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 1;
}

/**
 * Compile the helper program with coverage instrumentation.
 */
int compile_helper_program(const char *source, const char *executable) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 %s -o %s",
             source, executable);
    
    printf("Compiling helper: %s\n", cmd);
    return system(cmd) == 0;
}

/**
 * Run the helper program to generate .gcda file.
 */
int run_helper_program(const char *executable) {
    printf("Running helper to generate .gcda\n");
    return system(executable) == 0;
}

int main(int argc, char *argv[]) {
    char helper_c[] = "test_helper_gcov.c";
    char helper_exe[] = "test_helper_gcov";
    char gcda_file[] = "test_helper_gcov.gcda";
    char gcov_dump_path[] = "gcov-dump";  // Adjust if needed
    
    // Check if gcov-dump exists
    if (access(gcov_dump_path, X_OK) != 0) {
        fprintf(stderr, "Error: %s not found or not executable\n", gcov_dump_path);
        fprintf(stderr, "Please adjust gcov_dump_path in the test program\n");
        return 1;
    }
    
    printf("=== Generating GCOV data files ===\n");
    
    // Step 1: Create and compile helper program
    if (!create_helper_program(helper_c)) {
        return 1;
    }
    
    if (!compile_helper_program(helper_c, helper_exe)) {
        fprintf(stderr, "Failed to compile helper program\n");
        return 1;
    }
    
    // Step 2: Run helper to generate .gcda file
    if (!run_helper_program(helper_exe)) {
        fprintf(stderr, "Failed to run helper program\n");
        return 1;
    }
    
    // Check that .gcda file was created
    if (access(gcda_file, F_OK) != 0) {
        fprintf(stderr, "Error: %s was not created\n", gcda_file);
        return 1;
    }
    
    printf("\n=== Testing gcov-dump with various flags ===\n");
    
    int tests_passed = 0;
    int total_invalid_tests = 0;
    
    // Test 1: Valid flag (should work)
    printf("\n--- Test 1: Valid flag (-l) ---\n");
    char valid_cmd[MAX_CMD_LEN];
    snprintf(valid_cmd, sizeof(valid_cmd), "%s -l %s", 
             gcov_dump_path, gcda_file);
    execute_and_check(valid_cmd, 0);
    
    // Test 2-7: Various invalid single-character flags
    // These should trigger the default case in the switch statement
    char *invalid_flags[] = {"-a", "-z", "-x", "-1", "-?", "-!"};
    int num_invalid = sizeof(invalid_flags) / sizeof(invalid_flags[0]);
    
    for (int i = 0; i < num_invalid; i++) {
        printf("\n--- Test %d: Invalid flag %s ---\n", i + 2, invalid_flags[i]);
        char invalid_cmd[MAX_CMD_LEN];
        snprintf(invalid_cmd, sizeof(invalid_cmd), "%s %s %s", 
                 gcov_dump_path, invalid_flags[i], gcda_file);
        
        if (execute_and_check(invalid_cmd, 1)) {
            tests_passed++;
        }
        total_invalid_tests++;
    }
    
    // Test: Multiple invalid flags together
    printf("\n--- Test: Multiple invalid flags (-a -z) ---\n");
    char multi_invalid_cmd[MAX_CMD_LEN];
    snprintf(multi_invalid_cmd, sizeof(multi_invalid_cmd), 
             "%s -a -z %s", gcov_dump_path, gcda_file);
    if (execute_and_check(multi_invalid_cmd, 1)) {
        tests_passed++;
        total_invalid_tests++;
    }
    
    // Test: Mix valid and invalid flag
    printf("\n--- Test: Mix valid and invalid flag (-l -a) ---\n");
    char mixed_cmd[MAX_CMD_LEN];
    snprintf(mixed_cmd, sizeof(mixed_cmd), 
             "%s -l -a %s", gcov_dump_path, gcda_file);
    if (execute_and_check(mixed_cmd, 1)) {
        tests_passed++;
        total_invalid_tests++;
    }
    
    // Test: Just invalid flag without filename (different code path)
    printf("\n--- Test: Invalid flag without filename (-a) ---\n");
    char nofile_cmd[MAX_CMD_LEN];
    snprintf(nofile_cmd, sizeof(nofile_cmd), "%s -a", gcov_dump_path);
    execute_and_check(nofile_cmd, 1);
    
    printf("\n=== Summary ===\n");
    printf("Invalid flag tests passed: %d/%d\n", tests_passed, total_invalid_tests);
    
    // Cleanup
    printf("\n=== Cleaning up ===\n");
    remove(helper_c);
    remove(helper_exe);
    remove(gcda_file);
    // Also remove .gcno file created during compilation
    char gcno_file[MAX_CMD_LEN];
    snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", helper_exe);
    remove(gcno_file);
    
    if (tests_passed > 0) {
        printf("\n✓ Successfully triggered the 'unknown flag' error message\n");
        printf("  This should cover the default case in gcov-dump's switch statement.\n");
        return 0;
    } else {
        printf("\n✗ Failed to trigger 'unknown flag' messages\n");
        return 1;
    }
}
