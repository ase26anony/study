/**
 * Test driver for gcov-dump command-line parsing coverage
 * Targets specific uncovered lines in gcov-dump.cc (lines 111-130)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_CMD_LEN 1024
#define TEMP_FILENAME "test_coverage_XXXXXX"

/**
 * Creates a minimal C source file with coverage instrumentation
 * Returns 0 on success, -1 on failure
 */
int create_test_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create source file");
        return -1;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i, sum = 0;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        sum += i;\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    printf(\"Sum: %%d\\n\", sum);\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/**
 * Compiles the test program with coverage instrumentation
 * Returns 0 on success, -1 on failure
 */
int compile_with_coverage(const char *source, const char *binary) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s 2>/dev/null",
             source, binary);
    
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Compilation failed\n");
        return -1;
    }
    
    // Check if binary was created
    struct stat st;
    if (stat(binary, &st) != 0) {
        fprintf(stderr, "Binary not created\n");
        return -1;
    }
    
    return 0;
}

/**
 * Executes a command and captures stderr
 * Returns 0 if command executed (regardless of exit code), -1 on system error
 */
int execute_and_check_stderr(const char *cmd, const char *expected_error) {
    char buffer[256];
    FILE *fp;
    int found_error = 0;
    
    // Redirect stderr to stdout and capture
    char full_cmd[MAX_CMD_LEN];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    fp = popen(full_cmd, "r");
    if (!fp) {
        perror("popen failed");
        return -1;
    }
    
    // Read output
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (expected_error && strstr(buffer, expected_error)) {
            found_error = 1;
        }
    }
    
    int status = pclose(fp);
    
    if (expected_error) {
        if (found_error) {
            printf("✓ Successfully triggered error: '%s'\n", expected_error);
        } else {
            printf("✗ Did not find expected error: '%s'\n", expected_error);
        }
    }
    
    return (status == -1) ? -1 : 0;
}

/**
 * Executes a simple system command
 * Returns 0 on success, -1 on failure
 */
int execute_command(const char *cmd) {
    int ret = system(cmd);
    if (ret == -1) {
        perror("system() failed");
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    char source_file[] = TEMP_FILENAME ".c";
    char binary_file[] = TEMP_FILENAME;
    char gcda_file[] = TEMP_FILENAME ".gcda";
    char gcno_file[] = TEMP_FILENAME ".gcno";
    char cmd[MAX_CMD_LEN];
    
    printf("=== Testing gcov-dump command-line parsing ===\n\n");
    
    // Step 1: Create test source file
    printf("1. Creating test source file...\n");
    if (create_test_source(source_file) != 0) {
        fprintf(stderr, "Failed to create source file\n");
        return EXIT_FAILURE;
    }
    
    // Step 2: Compile with coverage
    printf("2. Compiling with coverage instrumentation...\n");
    if (compile_with_coverage(source_file, binary_file) != 0) {
        fprintf(stderr, "Failed to compile with coverage\n");
        unlink(source_file);
        return EXIT_FAILURE;
    }
    
    // Step 3: Execute to generate .gcda file
    printf("3. Executing test program to generate .gcda file...\n");
    snprintf(cmd, sizeof(cmd), "./%s >/dev/null 2>&1", binary_file);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Failed to execute test program\n");
        goto cleanup;
    }
    
    // Verify .gcda file exists
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, ".gcda file not created\n");
        goto cleanup;
    }
    
    printf("4. Testing gcov-dump command-line flags...\n\n");
    
    // Test 1: -h flag (help) - triggers print_usage()
    printf("Test 1: -h flag (help)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -h");
    execute_command(cmd);
    printf("\n");
    
    // Test 2: -v flag (version) - triggers print_version()
    printf("Test 2: -v flag (version)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -v");
    execute_command(cmd);
    printf("\n");
    
    // Test 3: -l flag (dump contents) with .gcda file
    printf("Test 3: -l flag (dump contents)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_file);
    execute_command(cmd);
    printf("\n");
    
    // Test 4: -p flag (dump positions) with .gcda file
    printf("Test 4: -p flag (dump positions)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -p %s", gcda_file);
    execute_command(cmd);
    printf("\n");
    
    // Test 5: -r flag (dump raw) with .gcda file
    printf("Test 5: -r flag (dump raw)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -r %s", gcda_file);
    execute_command(cmd);
    printf("\n");
    
    // Test 6: -s flag (dump stable) with .gcda file
    printf("Test 6: -s flag (dump stable)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -s %s", gcda_file);
    execute_command(cmd);
    printf("\n");
    
    // Test 7: Combined flags (-l -p) with .gcda file
    printf("Test 7: Combined flags (-l -p)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -p %s", gcda_file);
    execute_command(cmd);
    printf("\n");
    
    // Test 8: Invalid flag (-X) - triggers default case and fprintf
    printf("Test 8: Invalid flag (-X) - should trigger 'unknown flag' error\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X %s", gcda_file);
    execute_and_check_stderr(cmd, "unknown flag");
    printf("\n");
    
    // Test 9: Another invalid flag (-z) with .gcda file
    printf("Test 9: Another invalid flag (-z)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -z %s", gcda_file);
    execute_and_check_stderr(cmd, "unknown flag");
    printf("\n");
    
    // Test 10: Test with .gcno file as well
    printf("Test 10: Testing with .gcno file\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcno_file);
    execute_command(cmd);
    printf("\n");
    
    // Test 11: Multiple invalid flags
    printf("Test 11: Multiple invalid flags\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X -Y -Z %s", gcda_file);
    execute_and_check_stderr(cmd, "unknown flag");
    
    printf("\n=== All tests completed ===\n");

cleanup:
    // Cleanup temporary files
    printf("\nCleaning up temporary files...\n");
    unlink(source_file);
    unlink(binary_file);
    unlink(gcda_file);
    unlink(gcno_file);
    
    // Also clean up any other coverage files that might have been created
    char gcov_file[MAX_CMD_LEN];
    snprintf(gcov_file, sizeof(gcov_file), "%s.gcov", source_file);
    unlink(gcov_file);
    
    return EXIT_SUCCESS;
}
