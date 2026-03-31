/**
 * test_gcov_dump_parser.c
 * 
 * Test driver to exercise gcov-dump command-line parsing logic.
 * Specifically targets the switch cases for flags: -h, -v, -l, -p, -r, -s
 * and the default case for invalid flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define TEMP_FILENAME_TEMPLATE "/tmp/test_gcov_XXXXXX"

/**
 * Creates a minimal C source file for coverage testing.
 * Returns 0 on success, -1 on failure.
 */
static int create_test_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create test source file");
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
 * Compiles the test source with coverage instrumentation.
 * Returns 0 on success, -1 on failure.
 */
static int compile_with_coverage(const char *source_file, const char *binary_file) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s 2>&1",
             source_file, binary_file);
    
    printf("Compiling: %s\n", cmd);
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Compilation failed\n");
        return -1;
    }
    
    return 0;
}

/**
 * Executes the test binary to generate .gcda file.
 * Returns 0 on success, -1 on failure.
 */
static int run_test_binary(const char *binary_file) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "%s 2>&1", binary_file);
    
    printf("Running test binary: %s\n", binary_file);
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Test execution failed\n");
        return -1;
    }
    
    return 0;
}

/**
 * Checks if a file exists.
 * Returns 1 if exists, 0 otherwise.
 */
static int file_exists(const char *filename) {
    struct stat st;
    return stat(filename, &st) == 0;
}

/**
 * Invokes gcov-dump with specified flags and captures output.
 * Returns 0 on success, -1 on failure.
 */
static int invoke_gcov_dump(const char *flags, const char *gcda_file, int capture_stderr) {
    char cmd[MAX_CMD_LEN];
    char output[4096];
    FILE *fp;
    int found_error = 0;
    
    if (gcda_file) {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s 2>&1", flags, gcda_file);
    } else {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s 2>&1", flags);
    }
    
    printf("\nInvoking: %s\n", cmd);
    
    if (capture_stderr) {
        fp = popen(cmd, "r");
        if (!fp) {
            perror("popen failed");
            return -1;
        }
        
        // Read and check for "unknown flag" message
        while (fgets(output, sizeof(output), fp) != NULL) {
            printf("Output: %s", output);
            if (strstr(output, "unknown flag")) {
                found_error = 1;
            }
        }
        
        int status = pclose(fp);
        
        if (found_error) {
            printf("SUCCESS: Triggered default case with 'unknown flag' message\n");
        }
        
        return (status == 0) ? 0 : -1;
    } else {
        int result = system(cmd);
        return (result == 0) ? 0 : -1;
    }
}

/**
 * Cleans up temporary files.
 */
static void cleanup_files(const char *base_name) {
    char cmd[MAX_CMD_LEN];
    
    // Remove all generated files
    const char *extensions[] = {".c", "", ".gcda", ".gcno", ".gcov"};
    
    for (size_t i = 0; i < sizeof(extensions)/sizeof(extensions[0]); i++) {
        char filename[256];
        snprintf(filename, sizeof(filename), "%s%s", base_name, extensions[i]);
        
        if (file_exists(filename)) {
            if (remove(filename) == 0) {
                printf("Removed: %s\n", filename);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    char source_file[256];
    char binary_file[256];
    char gcda_file[256];
    int ret = 0;
    
    // Create unique temporary filenames
    char temp_template[] = TEMP_FILENAME_TEMPLATE;
    int fd = mkstemp(temp_template);
    if (fd == -1) {
        perror("Failed to create temporary filename");
        return EXIT_FAILURE;
    }
    close(fd);
    
    // Set filenames
    snprintf(source_file, sizeof(source_file), "%s.c", temp_template);
    snprintf(binary_file, sizeof(binary_file), "%s", temp_template);
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", temp_template);
    
    printf("=== Test Driver for gcov-dump Parser ===\n");
    printf("Temporary base: %s\n", temp_template);
    
    // Step 1: Create test source file
    printf("\n--- Step 1: Creating test source ---\n");
    if (create_test_source(source_file) != 0) {
        cleanup_files(temp_template);
        return EXIT_FAILURE;
    }
    printf("Created: %s\n", source_file);
    
    // Step 2: Compile with coverage
    printf("\n--- Step 2: Compiling with coverage ---\n");
    if (compile_with_coverage(source_file, binary_file) != 0) {
        cleanup_files(temp_template);
        return EXIT_FAILURE;
    }
    printf("Compiled: %s\n", binary_file);
    
    // Step 3: Run test to generate .gcda
    printf("\n--- Step 3: Running test to generate .gcda ---\n");
    if (run_test_binary(binary_file) != 0) {
        cleanup_files(temp_template);
        return EXIT_FAILURE;
    }
    
    // Verify .gcda file was created
    if (!file_exists(gcda_file)) {
        fprintf(stderr, "ERROR: .gcda file not created: %s\n", gcda_file);
        cleanup_files(temp_template);
        return EXIT_FAILURE;
    }
    printf("Generated: %s\n", gcda_file);
    
    // Step 4: Invoke gcov-dump with various flags
    printf("\n=== Testing gcov-dump command-line parsing ===\n");
    
    // Test 1: -h flag (help) - triggers print_usage()
    printf("\n--- Test 1: -h flag (help) ---\n");
    if (invoke_gcov_dump("-h", NULL, 0) != 0) {
        fprintf(stderr, "WARNING: gcov-dump -h failed\n");
        ret = 1;
    }
    
    // Test 2: -v flag (version) - triggers print_version()
    printf("\n--- Test 2: -v flag (version) ---\n");
    if (invoke_gcov_dump("-v", NULL, 0) != 0) {
        fprintf(stderr, "WARNING: gcov-dump -v failed\n");
        ret = 1;
    }
    
    // Test 3: -l flag (dump contents) - sets flag_dump_contents = 1
    printf("\n--- Test 3: -l flag (dump contents) ---\n");
    if (invoke_gcov_dump("-l", gcda_file, 0) != 0) {
        fprintf(stderr, "WARNING: gcov-dump -l failed\n");
        ret = 1;
    }
    
    // Test 4: -p flag (dump positions) - sets flag_dump_positions = 1
    printf("\n--- Test 4: -p flag (dump positions) ---\n");
    if (invoke_gcov_dump("-p", gcda_file, 0) != 0) {
        fprintf(stderr, "WARNING: gcov-dump -p failed\n");
        ret = 1;
    }
    
    // Test 5: -r flag (dump raw) - sets flag_dump_raw = 1
    printf("\n--- Test 5: -r flag (dump raw) ---\n");
    if (invoke_gcov_dump("-r", gcda_file, 0) != 0) {
        fprintf(stderr, "WARNING: gcov-dump -r failed\n");
        ret = 1;
    }
    
    // Test 6: -s flag (dump stable) - sets flag_dump_stable = 1
    printf("\n--- Test 6: -s flag (dump stable) ---\n");
    if (invoke_gcov_dump("-s", gcda_file, 0) != 0) {
        fprintf(stderr, "WARNING: gcov-dump -s failed\n");
        ret = 1;
    }
    
    // Test 7: Combined flags (-l -p)
    printf("\n--- Test 7: Combined flags (-l -p) ---\n");
    if (invoke_gcov_dump("-l -p", gcda_file, 0) != 0) {
        fprintf(stderr, "WARNING: gcov-dump -l -p failed\n");
        ret = 1;
    }
    
    // Test 8: Invalid flag (-X) - triggers default case and fprintf
    printf("\n--- Test 8: Invalid flag (-X) ---\n");
    if (invoke_gcov_dump("-X", gcda_file, 1) != 0) {
        // This is expected to fail, so we don't set ret
        printf("Expected failure for invalid flag\n");
    }
    
    // Test 9: Another invalid flag (-z)
    printf("\n--- Test 9: Invalid flag (-z) ---\n");
    if (invoke_gcov_dump("-z", gcda_file, 1) != 0) {
        printf("Expected failure for invalid flag\n");
    }
    
    // Test 10: Multiple flags including invalid
    printf("\n--- Test 10: Mixed valid/invalid flags (-l -X -p) ---\n");
    if (invoke_gcov_dump("-l -X -p", gcda_file, 1) != 0) {
        printf("Expected failure due to invalid flag\n");
    }
    
    // Cleanup
    printf("\n=== Cleaning up ===\n");
    cleanup_files(temp_template);
    
    if (ret == 0) {
        printf("\n=== All tests completed successfully ===\n");
    } else {
        printf("\n=== Some tests had warnings ===\n");
    }
    
    return ret;
}
