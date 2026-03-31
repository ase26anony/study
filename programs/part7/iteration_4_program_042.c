/**
 * test_gcov_dump_switches.c
 * 
 * Test driver to exercise the command-line switch cases in gcov-dump.cc
 * Specifically targets lines 111-130: -h, -v, -l, -p, -r, -s, and invalid flag
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define TEMP_FILENAME "test_gcov_dump_coverage"

/**
 * Creates a minimal C source file with coverage instrumentation
 * to generate .gcda and .gcno files for gcov-dump to process
 */
int create_coverage_test_file(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return 0;
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
    return 1;
}

/**
 * Compiles the test program with coverage instrumentation
 */
int compile_with_coverage(const char *source_file, const char *output_file) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s 2>/dev/null",
             source_file, output_file);
    
    printf("Compiling: %s\n", cmd);
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Compilation failed\n");
        return 0;
    }
    
    // Check if binary was created
    struct stat st;
    if (stat(output_file, &st) != 0) {
        fprintf(stderr, "Binary not created\n");
        return 0;
    }
    
    return 1;
}

/**
 * Runs the test program to generate .gcda file
 */
int run_test_program(const char *program) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "./%s > /dev/null", program);
    
    printf("Running test program to generate .gcda file\n");
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Test program execution failed\n");
        return 0;
    }
    
    // Check if .gcda file was created
    char gcda_file[MAX_CMD_LEN];
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", TEMP_FILENAME);
    
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, ".gcda file not created\n");
        return 0;
    }
    
    printf("Generated %s.gcda and %s.gcno files\n", TEMP_FILENAME, TEMP_FILENAME);
    return 1;
}

/**
 * Executes gcov-dump with given arguments and captures output if needed
 * Returns 1 if command executed (regardless of gcov-dump exit code)
 */
int run_gcov_dump(const char *args, int capture_stderr) {
    char cmd[MAX_CMD_LEN];
    char output[1024];
    FILE *fp;
    int found_error = 0;
    
    snprintf(cmd, sizeof(cmd), "gcov-dump %s 2>&1", args);
    
    printf("\nExecuting: gcov-dump %s\n", args);
    
    if (capture_stderr) {
        fp = popen(cmd, "r");
        if (!fp) {
            perror("popen failed");
            return 0;
        }
        
        // Read output to check for "unknown flag" message
        while (fgets(output, sizeof(output), fp) != NULL) {
            if (strstr(output, "unknown flag") != NULL) {
                printf("SUCCESS: Triggered default case with message: %s", output);
                found_error = 1;
            }
        }
        
        pclose(fp);
        
        if (!found_error && capture_stderr) {
            printf("WARNING: Expected 'unknown flag' message not found\n");
        }
    } else {
        // Just execute without capturing output
        int result = system(cmd);
        if (result != 0) {
            printf("Note: gcov-dump exited with code %d\n", result);
        }
    }
    
    return 1;
}

/**
 * Clean up temporary files
 */
void cleanup_files(void) {
    char cmd[MAX_CMD_LEN];
    
    printf("\nCleaning up temporary files...\n");
    
    // Remove source file
    snprintf(cmd, sizeof(cmd), "rm -f %s.c", TEMP_FILENAME);
    system(cmd);
    
    // Remove binary
    snprintf(cmd, sizeof(cmd), "rm -f %s", TEMP_FILENAME);
    system(cmd);
    
    // Remove coverage files
    snprintf(cmd, sizeof(cmd), "rm -f %s.gcda %s.gcno", TEMP_FILENAME, TEMP_FILENAME);
    system(cmd);
    
    // Remove gcov output files
    snprintf(cmd, sizeof(cmd), "rm -f *.gcov 2>/dev/null");
    system(cmd);
}

int main(int argc, char *argv[]) {
    char source_file[MAX_CMD_LEN];
    char gcda_file[MAX_CMD_LEN];
    
    printf("=== Test Driver for gcov-dump Switch Cases ===\n");
    
    // Create filenames
    snprintf(source_file, sizeof(source_file), "%s.c", TEMP_FILENAME);
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", TEMP_FILENAME);
    
    // Step 1: Create test source file
    printf("\n1. Creating test source file: %s\n", source_file);
    if (!create_coverage_test_file(source_file)) {
        fprintf(stderr, "Failed to create test source file\n");
        return 1;
    }
    
    // Step 2: Compile with coverage
    printf("\n2. Compiling with coverage instrumentation\n");
    if (!compile_with_coverage(source_file, TEMP_FILENAME)) {
        cleanup_files();
        return 1;
    }
    
    // Step 3: Run test program to generate .gcda file
    printf("\n3. Generating coverage data file\n");
    if (!run_test_program(TEMP_FILENAME)) {
        cleanup_files();
        return 1;
    }
    
    // Step 4: Test gcov-dump with various flags
    printf("\n4. Testing gcov-dump command-line switches\n");
    
    // Test 1: -h (help) - triggers print_usage()
    printf("\n--- Test 1: -h flag (help) ---");
    run_gcov_dump("-h", 0);
    
    // Test 2: -v (version) - triggers print_version()
    printf("\n--- Test 2: -v flag (version) ---");
    run_gcov_dump("-v", 0);
    
    // Test 3: -l flag with .gcda file - sets flag_dump_contents
    printf("\n--- Test 3: -l flag (dump contents) ---");
    run_gcov_dump("-l test_gcov_dump_coverage.gcda", 0);
    
    // Test 4: -p flag with .gcda file - sets flag_dump_positions
    printf("\n--- Test 4: -p flag (dump positions) ---");
    run_gcov_dump("-p test_gcov_dump_coverage.gcda", 0);
    
    // Test 5: -r flag with .gcda file - sets flag_dump_raw
    printf("\n--- Test 5: -r flag (dump raw) ---");
    run_gcov_dump("-r test_gcov_dump_coverage.gcda", 0);
    
    // Test 6: -s flag with .gcda file - sets flag_dump_stable
    printf("\n--- Test 6: -s flag (dump stable) ---");
    run_gcov_dump("-s test_gcov_dump_coverage.gcda", 0);
    
    // Test 7: Combined flags -l -p
    printf("\n--- Test 7: Combined flags -l -p ---");
    run_gcov_dump("-l -p test_gcov_dump_coverage.gcda", 0);
    
    // Test 8: Invalid flag - triggers default case and fprintf
    printf("\n--- Test 8: Invalid flag (should trigger 'unknown flag') ---");
    run_gcov_dump("-X test_gcov_dump_coverage.gcda", 1);
    
    // Test 9: Another invalid flag combination
    printf("\n--- Test 9: Another invalid flag -Z ---");
    run_gcov_dump("-Z test_gcov_dump_coverage.gcda", 1);
    
    // Test 10: Test with .gcno file instead of .gcda
    printf("\n--- Test 10: Testing with .gcno file ---");
    run_gcov_dump("-l test_gcov_dump_coverage.gcno", 0);
    
    // Test 11: Multiple valid flags with .gcno
    printf("\n--- Test 11: Multiple flags with .gcno file ---");
    run_gcov_dump("-l -p -r test_gcov_dump_coverage.gcno", 0);
    
    // Cleanup
    cleanup_files();
    
    printf("\n=== All tests completed ===\n");
    printf("The following gcov-dump switch cases should have been executed:\n");
    printf("  - Case 'h': print_usage()\n");
    printf("  - Case 'v': print_version()\n");
    printf("  - Case 'l': flag_dump_contents = 1\n");
    printf("  - Case 'p': flag_dump_positions = 1\n");
    printf("  - Case 'r': flag_dump_raw = 1\n");
    printf("  - Case 's': flag_dump_stable = 1\n");
    printf("  - Default case: fprintf(stderr, \"unknown flag\")\n");
    
    return 0;
}
