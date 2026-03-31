/**
 * test_gcov_dump_switch.c
 * 
 * Test driver to exercise the command-line switch cases in gcov-dump.cc
 * Specifically targets lines 111-130 covering flags: h, v, l, p, r, s, and invalid flag
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define TEMP_FILENAME "test_gcov_dump_switch"

/**
 * Creates a simple C source file with coverage instrumentation potential
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
 * Compiles the test source with coverage instrumentation
 */
int compile_with_coverage(const char *source_file, const char *binary_name) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s 2>&1",
             source_file, binary_name);
    
    printf("Compiling: %s\n", cmd);
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Compilation failed\n");
        return -1;
    }
    
    return 0;
}

/**
 * Executes the compiled binary to generate .gcda file
 */
int execute_for_coverage(const char *binary_name) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "./%s", binary_name);
    
    printf("Executing: %s\n", cmd);
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Execution failed\n");
        return -1;
    }
    
    // Verify .gcda file was created
    char gcda_file[MAX_CMD_LEN];
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", TEMP_FILENAME);
    
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "No .gcda file created: %s\n", gcda_file);
        return -1;
    }
    
    printf("Generated: %s (size: %ld bytes)\n", gcda_file, st.st_size);
    return 0;
}

/**
 * Runs gcov-dump with specified flags and captures output
 */
int run_gcov_dump(const char *flags, const char *input_file, int capture_stderr) {
    char cmd[MAX_CMD_LEN];
    FILE *fp;
    char buffer[256];
    int found_error = 0;
    
    if (input_file) {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s 2>&1", flags, input_file);
    } else {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s 2>&1", flags);
    }
    
    printf("\nRunning: %s\n", cmd);
    
    fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        return -1;
    }
    
    // Read and optionally analyze output
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (capture_stderr) {
            // Check for "unknown flag" message from default case
            if (strstr(buffer, "unknown flag")) {
                printf("SUCCESS: Triggered default case with message: %s", buffer);
                found_error = 1;
            }
        }
        // For this test, we just need to ensure the command runs
        // The output itself isn't critical except for verification
    }
    
    int status = pclose(fp);
    
    if (capture_stderr && !found_error) {
        printf("Note: No 'unknown flag' message detected\n");
    }
    
    return WEXITSTATUS(status);
}

/**
 * Main test driver
 */
int main(int argc, char *argv[]) {
    char source_file[MAX_CMD_LEN];
    char binary_file[MAX_CMD_LEN];
    char gcda_file[MAX_CMD_LEN];
    char gcno_file[MAX_CMD_LEN];
    int ret = 0;
    
    // Setup filenames
    snprintf(source_file, sizeof(source_file), "%s.c", TEMP_FILENAME);
    snprintf(binary_file, sizeof(binary_file), "%s", TEMP_FILENAME);
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", TEMP_FILENAME);
    snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", TEMP_FILENAME);
    
    printf("=== Test Driver for gcov-dump Switch Cases ===\n");
    
    // Step 1: Create test source
    printf("\n1. Creating test source: %s\n", source_file);
    if (create_test_source(source_file) != 0) {
        return 1;
    }
    
    // Step 2: Compile with coverage
    printf("\n2. Compiling with coverage instrumentation\n");
    if (compile_with_coverage(source_file, binary_file) != 0) {
        return 1;
    }
    
    // Step 3: Execute to generate .gcda
    printf("\n3. Executing to generate coverage data\n");
    if (execute_for_coverage(binary_file) != 0) {
        return 1;
    }
    
    // Step 4: Test gcov-dump with various flags
    printf("\n4. Testing gcov-dump command-line switches\n");
    
    // Test case 1: -h (help) - triggers print_usage()
    printf("\n--- Test 1: -h flag (help) ---\n");
    if (run_gcov_dump("-h", NULL, 0) != 0) {
        fprintf(stderr, "Warning: -h flag test returned non-zero\n");
    }
    
    // Test case 2: -v (version) - triggers print_version()
    printf("\n--- Test 2: -v flag (version) ---\n");
    if (run_gcov_dump("-v", NULL, 0) != 0) {
        fprintf(stderr, "Warning: -v flag test returned non-zero\n");
    }
    
    // Test case 3: -l flag with .gcda file - sets flag_dump_contents
    printf("\n--- Test 3: -l flag (dump contents) ---\n");
    if (run_gcov_dump("-l", gcda_file, 0) != 0) {
        fprintf(stderr, "Warning: -l flag test returned non-zero\n");
    }
    
    // Test case 4: -p flag with .gcda file - sets flag_dump_positions
    printf("\n--- Test 4: -p flag (dump positions) ---\n");
    if (run_gcov_dump("-p", gcda_file, 0) != 0) {
        fprintf(stderr, "Warning: -p flag test returned non-zero\n");
    }
    
    // Test case 5: -r flag with .gcda file - sets flag_dump_raw
    printf("\n--- Test 5: -r flag (dump raw) ---\n");
    if (run_gcov_dump("-r", gcda_file, 0) != 0) {
        fprintf(stderr, "Warning: -r flag test returned non-zero\n");
    }
    
    // Test case 6: -s flag with .gcda file - sets flag_dump_stable
    printf("\n--- Test 6: -s flag (dump stable) ---\n");
    if (run_gcov_dump("-s", gcda_file, 0) != 0) {
        fprintf(stderr, "Warning: -s flag test returned non-zero\n");
    }
    
    // Test case 7: Combined flags -l -p
    printf("\n--- Test 7: Combined flags -l -p ---\n");
    if (run_gcov_dump("-l -p", gcda_file, 0) != 0) {
        fprintf(stderr, "Warning: -l -p flag test returned non-zero\n");
    }
    
    // Test case 8: Invalid flag -X - triggers default case and fprintf
    printf("\n--- Test 8: Invalid flag -X (trigger default case) ---\n");
    if (run_gcov_dump("-X", gcda_file, 1) != 0) {
        printf("Expected non-zero return for invalid flag\n");
    }
    
    // Test case 9: Also test with .gcno file
    printf("\n--- Test 9: Testing with .gcno file ---\n");
    if (run_gcov_dump("-l", gcno_file, 0) != 0) {
        fprintf(stderr, "Warning: -l with .gcno test returned non-zero\n");
    }
    
    // Test case 10: Multiple invalid flags
    printf("\n--- Test 10: Multiple invalid flags -X -Y -Z ---\n");
    if (run_gcov_dump("-X -Y -Z", gcda_file, 1) != 0) {
        printf("Expected non-zero return for invalid flags\n");
    }
    
    // Cleanup
    printf("\n5. Cleaning up temporary files\n");
    unlink(source_file);
    unlink(binary_file);
    unlink(gcda_file);
    unlink(gcno_file);
    
    printf("\n=== Test completed ===\n");
    printf("All target switch cases in gcov-dump.cc should have been executed:\n");
    printf("  - Case 'h': print_usage()\n");
    printf("  - Case 'v': print_version()\n");
    printf("  - Case 'l': flag_dump_contents = 1\n");
    printf("  - Case 'p': flag_dump_positions = 1\n");
    printf("  - Case 'r': flag_dump_raw = 1\n");
    printf("  - Case 's': flag_dump_stable = 1\n");
    printf("  - Default case: fprintf(stderr, \"unknown flag\")\n");
    
    return ret;
}
