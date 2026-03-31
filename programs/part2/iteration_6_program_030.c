/**
 * Test program to cover the default case in gcov-dump's switch statement
 * for handling unknown single-character command-line flags.
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
 * Creates a simple C source file for generating GCOV data.
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
 * Compiles the helper program with GCOV instrumentation.
 */
int compile_helper(const char *source, const char *executable) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -o %s %s 2>&1", 
             executable, source);
    
    printf("Compiling helper: %s\n", cmd);
    int status = system(cmd);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

/**
 * Runs the helper program to generate .gcda file.
 */
int run_helper(const char *executable) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "./%s 2>&1", executable);
    
    printf("Running helper: %s\n", cmd);
    int status = system(cmd);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

/**
 * Executes a gcov-dump command and checks for expected output.
 * Returns 1 if test passed, 0 if failed.
 */
int test_gcov_dump_command(const char *command, int expect_error, const char *expected_substring) {
    char full_cmd[MAX_CMD];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", command);
    
    printf("\nTesting: %s\n", command);
    
    FILE *fp = popen(full_cmd, "r");
    if (!fp) {
        perror("popen failed");
        return 0;
    }
    
    char output[4096] = "";
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp)) {
        strncat(output, buffer, sizeof(output) - strlen(output) - 1);
    }
    
    int status = pclose(fp);
    int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    
    printf("Exit code: %d\n", exit_code);
    printf("Output:\n%s\n", output);
    
    int found_substring = (expected_substring && strstr(output, expected_substring) != NULL);
    
    if (expect_error) {
        // For invalid flags, we expect non-zero exit and error message
        if (exit_code == 0) {
            printf("FAIL: Expected non-zero exit code for invalid flag\n");
            return 0;
        }
        if (!found_substring) {
            printf("FAIL: Expected substring '%s' not found\n", expected_substring);
            return 0;
        }
        printf("PASS: Invalid flag correctly rejected\n");
    } else {
        // For valid commands, we expect success
        if (exit_code != 0) {
            printf("FAIL: Expected zero exit code for valid command\n");
            return 0;
        }
        printf("PASS: Valid command executed successfully\n");
    }
    
    return 1;
}

int main(int argc, char *argv[]) {
    char tmpdir[MAX_PATH] = "/tmp/gcov_test_XXXXXX";
    char helper_c[MAX_PATH];
    char helper_exe[MAX_PATH];
    char gcda_file[MAX_PATH];
    
    // Create temporary directory
    if (!mkdtemp(tmpdir)) {
        perror("Failed to create temporary directory");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", tmpdir);
    
    // Set up file paths
    snprintf(helper_c, sizeof(helper_c), "%s/helper.c", tmpdir);
    snprintf(helper_exe, sizeof(helper_exe), "%s/helper", tmpdir);
    snprintf(gcda_file, sizeof(gcda_file), "%s/helper.gcda", tmpdir);
    
    // Change to temp directory for relative paths
    if (chdir(tmpdir) != 0) {
        perror("Failed to change to temp directory");
        return 1;
    }
    
    // Step 1: Generate GCOV data
    printf("\n=== Step 1: Generating GCOV data ===\n");
    
    if (!create_helper_source("helper.c")) {
        return 1;
    }
    
    if (!compile_helper("helper.c", "helper")) {
        printf("Failed to compile helper program\n");
        return 1;
    }
    
    if (!run_helper("helper")) {
        printf("Failed to run helper program\n");
        return 1;
    }
    
    // Verify .gcda file was created
    struct stat st;
    if (stat("helper.gcda", &st) != 0) {
        printf("ERROR: helper.gcda not created\n");
        return 1;
    }
    
    printf("GCOV data file created: helper.gcda (%ld bytes)\n", (long)st.st_size);
    
    // Step 2: Test gcov-dump with various flags
    printf("\n=== Step 2: Testing gcov-dump command-line parsing ===\n");
    
    int all_tests_passed = 1;
    
    // Test 1: Valid command with -l flag
    printf("\n--- Test 1: Valid flag (-l) ---\n");
    if (!test_gcov_dump_command("gcov-dump -l helper.gcda", 0, NULL)) {
        all_tests_passed = 0;
    }
    
    // Test 2: Invalid flag -a (alphabetical, not in valid set)
    printf("\n--- Test 2: Invalid flag (-a) ---\n");
    if (!test_gcov_dump_command("gcov-dump -a helper.gcda", 1, "unknown flag")) {
        all_tests_passed = 0;
    }
    
    // Test 3: Invalid flag -z (another alphabetical)
    printf("\n--- Test 3: Invalid flag (-z) ---\n");
    if (!test_gcov_dump_command("gcov-dump -z helper.gcda", 1, "unknown flag")) {
        all_tests_passed = 0;
    }
    
    // Test 4: Invalid flag -1 (numeric)
    printf("\n--- Test 4: Invalid flag (-1) ---\n");
    if (!test_gcov_dump_command("gcov-dump -1 helper.gcda", 1, "unknown flag")) {
        all_tests_passed = 0;
    }
    
    // Test 5: Invalid flag -? (special character)
    printf("\n--- Test 5: Invalid flag (-?) ---\n");
    if (!test_gcov_dump_command("gcov-dump -\\? helper.gcda", 1, "unknown flag")) {
        all_tests_passed = 0;
    }
    
    // Test 6: Invalid flag -x (another alphabetical)
    printf("\n--- Test 6: Invalid flag (-x) ---\n");
    if (!test_gcov_dump_command("gcov-dump -x helper.gcda", 1, "unknown flag")) {
        all_tests_passed = 0;
    }
    
    // Test 7: Multiple invalid flags in sequence
    printf("\n--- Test 7: Multiple invalid flags (-a -z) ---\n");
    if (!test_gcov_dump_command("gcov-dump -a -z helper.gcda", 1, "unknown flag")) {
        all_tests_passed = 0;
    }
    
    // Test 8: Valid flag combined with invalid flag
    printf("\n--- Test 8: Valid + invalid flag (-l -a) ---\n");
    if (!test_gcov_dump_command("gcov-dump -l -a helper.gcda", 1, "unknown flag")) {
        all_tests_passed = 0;
    }
    
    // Test 9: Just the dash without character (should trigger different error)
    printf("\n--- Test 9: Just dash (-) ---\n");
    if (!test_gcov_dump_command("gcov-dump - helper.gcda", 1, NULL)) {
        all_tests_passed = 0;
    }
    
    // Test 10: Valid command with -v flag (version)
    printf("\n--- Test 10: Valid flag (-v) ---\n");
    if (!test_gcov_dump_command("gcov-dump -v", 0, NULL)) {
        all_tests_passed = 0;
    }
    
    // Step 3: Cleanup
    printf("\n=== Step 3: Cleanup ===\n");
    chdir("..");  // Leave temp directory
    
    char cleanup_cmd[MAX_CMD];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", tmpdir);
    printf("Cleaning up: %s\n", cleanup_cmd);
    system(cleanup_cmd);
    
    // Final result
    printf("\n=== Test Results ===\n");
    if (all_tests_passed) {
        printf("SUCCESS: All tests passed!\n");
        printf("The default case in gcov-dump's switch statement should now be covered.\n");
        return 0;
    } else {
        printf("FAILURE: Some tests failed\n");
        return 1;
    }
}
