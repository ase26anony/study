/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Test program to cover the uncovered default case in gcov-dump.cc
 * that handles unknown single-character command-line flags.
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
 * Creates a simple C source file for generating GCOV data
 */
int create_helper_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create helper source file");
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
 * Compiles the helper program with GCOV instrumentation
 */
int compile_helper(const char *source_file, const char *executable) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -o %s %s 2>&1",
             executable, source_file);
    
    printf("Compiling helper: %s\n", cmd);
    
    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        perror("Failed to compile helper");
        return 0;
    }
    
    // Read and discard compilation output
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        // Check for compilation errors
        if (strstr(buffer, "error:") || strstr(buffer, "Error:")) {
            printf("Compilation error: %s", buffer);
            pclose(pipe);
            return 0;
        }
    }
    
    int status = pclose(pipe);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

/**
 * Runs the helper program to generate .gcda file
 */
int run_helper(const char *executable) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "./%s", executable);
    
    printf("Running helper: %s\n", cmd);
    
    int status = system(cmd);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

/**
 * Executes gcov-dump with given arguments and checks for expected output
 * Returns 1 if test passed, 0 if failed
 */
int test_gcov_dump(const char *gcov_dump_path, const char *gcda_file, 
                   const char *flag, int expect_unknown_flag) {
    char cmd[MAX_CMD_LEN];
    char output[MAX_OUTPUT_LEN] = {0};
    int found_unknown_flag = 0;
    
    // Build command with stderr redirected to stdout
    if (flag[0] == '\0') {
        // No flag case
        snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, gcda_file);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s %s 2>&1", gcov_dump_path, flag, gcda_file);
    }
    
    printf("Testing: %s\n", cmd);
    
    // Execute command
    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        perror("Failed to execute gcov-dump");
        return 0;
    }
    
    // Read output
    size_t total_read = 0;
    while (fgets(output + total_read, sizeof(output) - total_read, pipe) != NULL) {
        total_read = strlen(output);
        if (total_read >= sizeof(output) - 1) {
            break;
        }
    }
    
    int status = pclose(pipe);
    int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    
    // Check for "unknown flag" message
    if (strstr(output, "unknown flag") != NULL) {
        found_unknown_flag = 1;
        printf("  Found 'unknown flag' message\n");
    }
    
    // Validate expectations
    if (expect_unknown_flag) {
        if (!found_unknown_flag) {
            printf("  FAIL: Expected 'unknown flag' but not found\n");
            printf("  Output: %s\n", output);
            return 0;
        }
        if (exit_code == 0) {
            printf("  FAIL: Expected non-zero exit code for invalid flag\n");
            return 0;
        }
    } else {
        if (found_unknown_flag) {
            printf("  FAIL: Unexpected 'unknown flag' message\n");
            printf("  Output: %s\n", output);
            return 0;
        }
    }
    
    printf("  PASS\n");
    return 1;
}

/**
 * Main test function
 */
int main(int argc, char *argv[]) {
    const char *gcov_dump_path = "gcov-dump";
    const char *helper_source = "test_helper_gcov.c";
    const char *helper_exe = "test_helper_gcov";
    const char *gcda_file = "test_helper_gcov.gcda";
    
    printf("=== GCOV-Dump Invalid Flag Test ===\n\n");
    
    // Step 1: Create and compile helper program
    printf("1. Creating helper program...\n");
    if (!create_helper_source(helper_source)) {
        return EXIT_FAILURE;
    }
    
    printf("2. Compiling helper with GCOV instrumentation...\n");
    if (!compile_helper(helper_source, helper_exe)) {
        unlink(helper_source);
        return EXIT_FAILURE;
    }
    
    printf("3. Running helper to generate .gcda file...\n");
    if (!run_helper(helper_exe)) {
        unlink(helper_source);
        unlink(helper_exe);
        return EXIT_FAILURE;
    }
    
    // Verify .gcda file exists
    if (access(gcda_file, F_OK) != 0) {
        printf("ERROR: .gcda file not created: %s\n", gcda_file);
        unlink(helper_source);
        unlink(helper_exe);
        return EXIT_FAILURE;
    }
    
    printf("\n4. Testing gcov-dump with various flags...\n");
    
    int all_tests_passed = 1;
    
    // Test 1: Valid flag (should work, no unknown flag message)
    printf("\nTest 1: Valid flag (-l)\n");
    all_tests_passed &= test_gcov_dump(gcov_dump_path, gcda_file, "-l", 0);
    
    // Test 2: No flag (should work)
    printf("\nTest 2: No flag\n");
    all_tests_passed &= test_gcov_dump(gcov_dump_path, gcda_file, "", 0);
    
    // Test 3-8: Various invalid single-character flags
    // These should trigger the default case in the switch statement
    const char *invalid_flags[] = {
        "-a",  // alphabetic, not in {h,v,l,p,r,s}
        "-z",  // another alphabetic
        "-x",  // another alphabetic
        "-1",  // numeric
        "-?",  // symbol
        "-@",  // another symbol
        NULL
    };
    
    for (int i = 0; invalid_flags[i] != NULL; i++) {
        printf("\nTest %d: Invalid flag %s\n", i + 3, invalid_flags[i]);
        all_tests_passed &= test_gcov_dump(gcov_dump_path, gcda_file, 
                                          invalid_flags[i], 1);
    }
    
    // Test: Multiple invalid flags together
    printf("\nTest: Multiple invalid flags (-a -z)\n");
    char multi_cmd[MAX_CMD_LEN];
    snprintf(multi_cmd, sizeof(multi_cmd), "%s -a -z %s 2>&1", 
             gcov_dump_path, gcda_file);
    printf("Testing: %s\n", multi_cmd);
    
    FILE *pipe = popen(multi_cmd, "r");
    if (pipe) {
        char output[1024] = {0};
        fread(output, 1, sizeof(output) - 1, pipe);
        pclose(pipe);
        
        // Should see at least one "unknown flag" message
        if (strstr(output, "unknown flag") == NULL) {
            printf("  FAIL: Expected 'unknown flag' message\n");
            all_tests_passed = 0;
        } else {
            printf("  PASS\n");
        }
    }
    
    // Cleanup
    printf("\n5. Cleaning up...\n");
    unlink(helper_source);
    unlink(helper_exe);
    unlink(gcda_file);
    
    // Also remove .gcno file created during compilation
    char gcno_file[256];
    snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", helper_source);
    unlink(gcno_file);
    
    printf("\n=== Test Summary ===\n");
    if (all_tests_passed) {
        printf("All tests PASSED\n");
        return EXIT_SUCCESS;
    } else {
        printf("Some tests FAILED\n");
        return EXIT_FAILURE;
    }
}
