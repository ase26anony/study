/**
 * Test driver for gcov-dump command-line parsing coverage
 * Targets uncovered lines 111-130 in gcov-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

#define TEMP_SOURCE_TEMPLATE "/tmp/gcov_test_XXXXXX.c"
#define TEMP_BINARY_TEMPLATE "/tmp/gcov_test_XXXXXX"
#define MAX_CMD_LEN 1024

/**
 * Creates a minimal C source file for coverage testing
 */
int create_test_source(const char *filename) {
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
 * Compiles the test source with coverage instrumentation
 */
int compile_with_coverage(const char *source, const char *binary) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 %s -o %s 2>&1",
             source, binary);
    
    printf("Compiling: %s\n", cmd);
    int status = system(cmd);
    
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("Compilation successful\n");
        return 1;
    } else {
        printf("Compilation failed\n");
        return 0;
    }
}

/**
 * Executes the test binary to generate coverage data
 */
int generate_coverage_data(const char *binary) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "%s 2>&1", binary);
    
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("Execution successful, .gcda file should be generated\n");
        return 1;
    } else {
        printf("Execution failed\n");
        return 0;
    }
}

/**
 * Runs gcov-dump with specified flags and captures output
 */
int run_gcov_dump(const char *flags, const char *gcda_file, int capture_stderr) {
    char cmd[MAX_CMD_LEN];
    char output[4096];
    FILE *fp;
    int found_error = 0;
    
    if (gcda_file) {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s 2>&1", flags, gcda_file);
    } else {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s 2>&1", flags);
    }
    
    printf("\nRunning: %s\n", cmd);
    
    if (capture_stderr) {
        fp = popen(cmd, "r");
        if (!fp) {
            perror("popen failed");
            return 0;
        }
        
        // Read output to check for error message
        while (fgets(output, sizeof(output), fp) != NULL) {
            printf("Output: %s", output);
            if (strstr(output, "unknown flag")) {
                found_error = 1;
            }
        }
        
        int status = pclose(fp);
        if (WIFEXITED(status)) {
            printf("Exit code: %d\n", WEXITSTATUS(status));
        }
        
        if (found_error) {
            printf("SUCCESS: Triggered 'unknown flag' error message\n");
        }
    } else {
        int status = system(cmd);
        if (WIFEXITED(status)) {
            printf("Exit code: %d\n", WEXITSTATUS(status));
        }
    }
    
    return 1;
}

/**
 * Checks if a file exists
 */
int file_exists(const char *filename) {
    struct stat st;
    return stat(filename, &st) == 0;
}

/**
 * Main test driver
 */
int main(int argc, char *argv[]) {
    char source_file[] = TEMP_SOURCE_TEMPLATE;
    char binary_file[] = TEMP_BINARY_TEMPLATE;
    char gcda_file[256];
    char gcno_file[256];
    
    printf("=== gcov-dump Command Line Parsing Test ===\n");
    
    // Create unique filenames
    int fd = mkstemps(source_file, 2);  // .c extension
    if (fd < 0) {
        perror("Failed to create temp source filename");
        return 1;
    }
    close(fd);
    
    strcpy(binary_file, source_file);
    binary_file[strlen(binary_file) - 2] = '\0';  // Remove .c extension
    
    // Create .gcda and .gcno filenames
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", binary_file);
    snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", binary_file);
    
    printf("Source file: %s\n", source_file);
    printf("Binary file: %s\n", binary_file);
    printf("Expected gcda: %s\n", gcda_file);
    printf("Expected gcno: %s\n", gcno_file);
    
    // Step 1: Create test source
    printf("\n--- Step 1: Creating test source ---\n");
    if (!create_test_source(source_file)) {
        return 1;
    }
    
    // Step 2: Compile with coverage
    printf("\n--- Step 2: Compiling with coverage ---\n");
    if (!compile_with_coverage(source_file, binary_file)) {
        unlink(source_file);
        return 1;
    }
    
    // Step 3: Execute to generate coverage data
    printf("\n--- Step 3: Generating coverage data ---\n");
    if (!generate_coverage_data(binary_file)) {
        unlink(source_file);
        unlink(binary_file);
        unlink(gcno_file);
        return 1;
    }
    
    // Verify .gcda file was created
    if (!file_exists(gcda_file)) {
        printf("ERROR: .gcda file not created: %s\n", gcda_file);
        printf("Trying alternative: using .gcno file instead\n");
        
        if (!file_exists(gcno_file)) {
            printf("ERROR: .gcno file also not found\n");
            // Clean up and exit
            unlink(source_file);
            unlink(binary_file);
            return 1;
        }
        // Use .gcno file instead
        strcpy(gcda_file, gcno_file);
    }
    
    printf("\n--- Step 4: Testing gcov-dump command line parsing ---\n");
    
    // Test 1: -h flag (help) - triggers print_usage()
    printf("\n*** Test 1: -h flag (help) ***\n");
    run_gcov_dump("-h", NULL, 0);
    
    // Test 2: -v flag (version) - triggers print_version()
    printf("\n*** Test 2: -v flag (version) ***\n");
    run_gcov_dump("-v", NULL, 0);
    
    // Test 3: -l flag (dump contents) - sets flag_dump_contents = 1
    printf("\n*** Test 3: -l flag (dump contents) ***\n");
    run_gcov_dump("-l", gcda_file, 0);
    
    // Test 4: -p flag (dump positions) - sets flag_dump_positions = 1
    printf("\n*** Test 4: -p flag (dump positions) ***\n");
    run_gcov_dump("-p", gcda_file, 0);
    
    // Test 5: -r flag (dump raw) - sets flag_dump_raw = 1
    printf("\n*** Test 5: -r flag (dump raw) ***\n");
    run_gcov_dump("-r", gcda_file, 0);
    
    // Test 6: -s flag (dump stable) - sets flag_dump_stable = 1
    printf("\n*** Test 6: -s flag (dump stable) ***\n");
    run_gcov_dump("-s", gcda_file, 0);
    
    // Test 7: Combined flags - tests multiple flag parsing
    printf("\n*** Test 7: Combined flags (-l -p) ***\n");
    run_gcov_dump("-l -p", gcda_file, 0);
    
    // Test 8: Invalid flag - triggers default case with fprintf
    printf("\n*** Test 8: Invalid flag (-X) ***\n");
    run_gcov_dump("-X", gcda_file, 1);  // Capture stderr to verify error message
    
    // Test 9: Multiple invalid flags
    printf("\n*** Test 9: Multiple invalid flags (-X -Y) ***\n");
    run_gcov_dump("-X -Y", gcda_file, 1);
    
    // Test 10: Valid flag with invalid flag combination
    printf("\n*** Test 10: Mixed valid and invalid flags (-l -X) ***\n");
    run_gcov_dump("-l -X", gcda_file, 1);
    
    // Test 11: Long form invalid flag (should also trigger default case)
    printf("\n*** Test 11: Long invalid flag (--invalid) ***\n");
    run_gcov_dump("--invalid", gcda_file, 1);
    
    printf("\n--- Step 5: Cleanup ---\n");
    
    // Clean up temporary files
    printf("Cleaning up temporary files...\n");
    unlink(source_file);
    unlink(binary_file);
    
    if (file_exists(gcda_file)) {
        unlink(gcda_file);
    }
    
    if (file_exists(gcno_file)) {
        unlink(gcno_file);
    }
    
    // Also clean up any gcov-related files
    char gcov_file[256];
    snprintf(gcov_file, sizeof(gcov_file), "%s.c.gcov", binary_file);
    if (file_exists(gcov_file)) {
        unlink(gcov_file);
    }
    
    printf("\n=== Test Complete ===\n");
    printf("All gcov-dump command line parsing paths should have been exercised.\n");
    printf("Check coverage report for gcov-dump.cc lines 111-130.\n");
    
    return 0;
}
