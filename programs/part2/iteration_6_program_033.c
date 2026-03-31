/**
 * Test program to cover the default case in gcov-dump's command-line parsing
 * which handles unknown single-character flags.
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
 * Creates a simple C program that, when compiled with -fprofile-arcs -ftest-coverage,
 * will generate .gcda files when executed.
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
 * Compiles the helper program with coverage instrumentation.
 */
int compile_helper(const char *source, const char *executable) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -o %s %s", 
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
    snprintf(cmd, sizeof(cmd), "./%s", executable);
    
    printf("Running helper: %s\n", cmd);
    int status = system(cmd);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

/**
 * Executes a gcov-dump command and checks for the "unknown flag" error message.
 * Returns 1 if the error message was found (for invalid flags) or 
 * the command succeeded (for valid flags), 0 otherwise.
 */
int test_gcov_dump_command(const char *command, int expect_invalid_flag) {
    char full_cmd[MAX_CMD];
    char result[MAX_CMD * 2];
    FILE *fp;
    int found_error = 0;
    
    // Redirect stderr to stdout to capture error messages
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", command);
    
    printf("\nTesting command: %s\n", command);
    
    fp = popen(full_cmd, "r");
    if (!fp) {
        perror("popen failed");
        return 0;
    }
    
    // Read the output
    result[0] = '\0';
    while (fgets(result + strlen(result), sizeof(result) - strlen(result), fp)) {
        // Continue reading
    }
    
    int status = pclose(fp);
    int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    
    printf("Exit code: %d\n", exit_code);
    printf("Output:\n%s\n", result);
    
    if (expect_invalid_flag) {
        // For invalid flags, we expect to find "unknown flag" in stderr
        found_error = (strstr(result, "unknown flag") != NULL);
        printf("Looking for 'unknown flag': %s\n", 
               found_error ? "FOUND" : "NOT FOUND");
        
        // Invalid flags should also cause non-zero exit
        if (found_error && exit_code != 0) {
            printf("✓ Correctly detected invalid flag\n");
            return 1;
        } else {
            printf("✗ Failed to detect invalid flag properly\n");
            return 0;
        }
    } else {
        // For valid flags, we expect success (exit code 0)
        if (exit_code == 0) {
            printf("✓ Valid command executed successfully\n");
            return 1;
        } else {
            printf("✗ Valid command failed unexpectedly\n");
            return 0;
        }
    }
}

int main(int argc, char *argv[]) {
    char helper_source[MAX_PATH];
    char helper_exec[MAX_PATH];
    char gcda_file[MAX_PATH];
    char cwd[MAX_PATH];
    int all_tests_passed = 1;
    
    // Get current directory for absolute paths
    if (!getcwd(cwd, sizeof(cwd))) {
        perror("getcwd failed");
        return 1;
    }
    
    // Create temporary filenames
    snprintf(helper_source, sizeof(helper_source), "%s/helper_gcov_test.c", cwd);
    snprintf(helper_exec, sizeof(helper_exec), "%s/helper_gcov_test", cwd);
    snprintf(gcda_file, sizeof(gcda_file), "%s/helper_gcov_test.gcda", cwd);
    
    printf("=== Generating GCOV test data ===\n");
    
    // Step 1: Create and compile helper program
    if (!create_helper_source(helper_source)) {
        return 1;
    }
    
    if (!compile_helper(helper_source, helper_exec)) {
        fprintf(stderr, "Failed to compile helper program\n");
        unlink(helper_source);
        return 1;
    }
    
    // Step 2: Run helper to generate .gcda file
    if (!run_helper(helper_exec)) {
        fprintf(stderr, "Failed to run helper program\n");
        unlink(helper_source);
        unlink(helper_exec);
        return 1;
    }
    
    // Verify .gcda file was created
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "Failed to create .gcda file: %s\n", gcda_file);
        unlink(helper_source);
        unlink(helper_exec);
        return 1;
    }
    
    printf("\n=== Testing gcov-dump with various flags ===\n");
    
    // Step 3: Test valid flag first (to ensure tool works)
    char valid_cmd[MAX_CMD];
    snprintf(valid_cmd, sizeof(valid_cmd), "gcov-dump -l %s", gcda_file);
    if (!test_gcov_dump_command(valid_cmd, 0)) {
        all_tests_passed = 0;
        printf("WARNING: Valid flag test failed, but continuing...\n");
    }
    
    // Step 4: Test multiple invalid single-character flags
    // These should trigger the default case in the switch statement
    char invalid_flags[] = "a?x1zB*";  // Various invalid characters
    for (int i = 0; invalid_flags[i] != '\0'; i++) {
        char invalid_cmd[MAX_CMD];
        snprintf(invalid_cmd, sizeof(invalid_cmd), 
                 "gcov-dump -%c %s", invalid_flags[i], gcda_file);
        
        if (!test_gcov_dump_command(invalid_cmd, 1)) {
            all_tests_passed = 0;
            printf("✗ Test failed for flag '-%c'\n", invalid_flags[i]);
        }
    }
    
    // Step 5: Test edge cases
    // Test with dash followed by non-alphabetic character
    char edge_cmd[MAX_CMD];
    snprintf(edge_cmd, sizeof(edge_cmd), "gcov-dump -@ %s", gcda_file);
    if (!test_gcov_dump_command(edge_cmd, 1)) {
        all_tests_passed = 0;
    }
    
    // Test with dash followed by space (should be treated as separate argument)
    snprintf(edge_cmd, sizeof(edge_cmd), "gcov-dump - %s", gcda_file);
    if (!test_gcov_dump_command(edge_cmd, 1)) {
        all_tests_passed = 0;
    }
    
    // Step 6: Cleanup
    printf("\n=== Cleaning up temporary files ===\n");
    unlink(helper_source);
    unlink(helper_exec);
    unlink(gcda_file);
    
    // Also remove other generated files
    char gcno_file[MAX_PATH];
    snprintf(gcno_file, sizeof(gcno_file), "%s/helper_gcov_test.gcno", cwd);
    unlink(gcno_file);
    
    if (all_tests_passed) {
        printf("\n✓ All tests passed!\n");
        return 0;
    } else {
        printf("\n✗ Some tests failed\n");
        return 1;
    }
}
