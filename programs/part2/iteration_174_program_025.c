/**
 * test_gcov_dump_coverage.c
 * 
 * This program tests the uncovered lines in gcov-dump.cc (lines 111-130)
 * by executing gcov-dump with various command-line flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024

/**
 * Compile gcov-dump with coverage instrumentation
 */
int compile_gcov_dump_with_coverage(const char *source_dir, const char *output_path) {
    char cmd[MAX_PATH * 4];
    struct stat st;
    
    printf("Checking for instrumented gcov-dump...\n");
    
    // First check if it already exists
    if (stat(output_path, &st) == 0) {
        printf("Instrumented gcov-dump found at %s\n", output_path);
        return 1;
    }
    
    // Try to compile from source
    printf("Attempting to compile gcov-dump with coverage...\n");
    
    // Simple compilation command - adjust paths as needed
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include -I../../libiberty "
        "%s/gcov-dump.cc ../../libiberty/libiberty.a -o %s",
        source_dir, output_path);
    
    printf("Compilation command: %s\n", cmd);
    
    int result = system(cmd);
    if (result != 0) {
        fprintf(stderr, "Failed to compile gcov-dump with coverage\n");
        return 0;
    }
    
    printf("Successfully compiled instrumented gcov-dump\n");
    return 1;
}

/**
 * Create a simple test program to generate .gcda files
 */
int create_test_gcda_file(const char *test_dir) {
    char dummy_c_path[MAX_PATH];
    char dummy_exe_path[MAX_PATH];
    char cmd[MAX_PATH * 4];
    
    // Create directory if it doesn't exist
    mkdir(test_dir, 0755);
    
    // Create dummy.c
    snprintf(dummy_c_path, sizeof(dummy_c_path), "%s/dummy.c", test_dir);
    FILE *fp = fopen(dummy_c_path, "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return 0;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        printf(\"Hello %%d\\n\", i);\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    fclose(fp);
    
    // Compile dummy.c with coverage
    snprintf(dummy_exe_path, sizeof(dummy_exe_path), "%s/dummy", test_dir);
    snprintf(cmd, sizeof(cmd),
        "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s",
        dummy_c_path, dummy_exe_path);
    
    printf("Compiling test program: %s\n", cmd);
    
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return 0;
    }
    
    // Run the dummy program to generate .gcda file
    snprintf(cmd, sizeof(cmd), "cd %s && ./dummy > /dev/null 2>&1", test_dir);
    printf("Running test program to generate .gcda file...\n");
    
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        return 0;
    }
    
    printf("Generated dummy.gcda file in %s\n", test_dir);
    return 1;
}

/**
 * Merge coverage data after each gcov-dump invocation
 */
void merge_coverage_data(const char *gcov_dump_path, const char *source_file) {
    char cmd[MAX_PATH * 4];
    
    // Method 1: Use gcov -i to merge coverage
    snprintf(cmd, sizeof(cmd), 
        "cd $(dirname %s) && gcov -i %s > /dev/null 2>&1",
        gcov_dump_path, source_file);
    
    // Method 2: Alternative approach - copy .gcda files to a common location
    snprintf(cmd, sizeof(cmd),
        "find . -name \"*.gcda\" -exec cp {} /tmp/coverage_merge/ \\; 2>/dev/null || true");
    system(cmd);
}

/**
 * Execute gcov-dump with given arguments and check exit code
 */
int execute_gcov_dump(const char *gcov_dump_path, const char *gcda_file, 
                     const char *args, int expect_success) {
    char cmd[MAX_PATH * 4];
    int result;
    
    if (gcda_file && strlen(gcda_file) > 0) {
        snprintf(cmd, sizeof(cmd), "%s %s %s", gcov_dump_path, args, gcda_file);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_path, args);
    }
    
    printf("Executing: %s\n", cmd);
    
    result = system(cmd);
    
    if (expect_success) {
        if (result != 0) {
            fprintf(stderr, "Command failed (expected success): %s\n", cmd);
            return 0;
        }
    } else {
        if (result == 0) {
            fprintf(stderr, "Command succeeded (expected failure): %s\n", cmd);
            return 0;
        }
    }
    
    return 1;
}

/**
 * Generate final coverage report
 */
void generate_coverage_report(const char *source_file) {
    char cmd[MAX_PATH * 4];
    
    printf("\n=== Generating Coverage Report ===\n");
    
    // Generate human-readable coverage report
    snprintf(cmd, sizeof(cmd), "gcov -b %s", source_file);
    system(cmd);
    
    // Also check specific lines
    printf("\n=== Checking Coverage for Target Lines (111-130) ===\n");
    snprintf(cmd, sizeof(cmd), 
        "gcov -l %s | grep -A20 -B5 '^111\\|^112\\|^113\\|^114\\|^115\\|^116\\|^117\\|^118\\|^119\\|^120\\|^121\\|^122\\|^123\\|^124\\|^125\\|^126\\|^127\\|^128\\|^129\\|^130'",
        source_file);
    system(cmd);
}

int main(int argc, char *argv[]) {
    const char *test_dir = "/tmp/gcov_dump_test";
    const char *gcov_dump_source = ".";  // Adjust this to your gcov-dump.cc location
    char instrumented_gcov_dump[MAX_PATH];
    char gcda_file[MAX_PATH];
    
    printf("=== Starting gcov-dump Coverage Test ===\n\n");
    
    // Set paths
    snprintf(instrumented_gcov_dump, sizeof(instrumented_gcov_dump),
             "%s/gcov-dump-instrumented", test_dir);
    snprintf(gcda_file, sizeof(gcda_file), "%s/dummy.gcda", test_dir);
    
    // Step 1: Create test directory
    mkdir(test_dir, 0755);
    
    // Step 2: Compile gcov-dump with coverage instrumentation
    if (!compile_gcov_dump_with_coverage(gcov_dump_source, instrumented_gcov_dump)) {
        // Try alternative: use existing gcov-dump if available
        printf("Trying to use system gcov-dump...\n");
        if (system("which gcov-dump > /dev/null 2>&1") == 0) {
            strcpy(instrumented_gcov_dump, "gcov-dump");
            printf("Using system gcov-dump\n");
        } else {
            fprintf(stderr, "Could not find or build gcov-dump\n");
            return 1;
        }
    }
    
    // Step 3: Generate test .gcda file
    if (!create_test_gcda_file(test_dir)) {
        fprintf(stderr, "Failed to create test .gcda file\n");
        return 1;
    }
    
    // Step 4: Execute gcov-dump with various flag combinations
    
    printf("\n=== Testing Individual Flags ===\n");
    
    // Test help flag (-h) - no .gcda file needed
    if (!execute_gcov_dump(instrumented_gcov_dump, NULL, "-h", 1)) {
        fprintf(stderr, "Help flag test failed\n");
    }
    merge_coverage_data(instrumented_gcov_dump, "gcov-dump.cc");
    
    // Test version flag (-v) - no .gcda file needed
    if (!execute_gcov_dump(instrumented_gcov_dump, NULL, "-v", 1)) {
        fprintf(stderr, "Version flag test failed\n");
    }
    merge_coverage_data(instrumented_gcov_dump, "gcov-dump.cc");
    
    // Test -l flag (dump contents)
    if (!execute_gcov_dump(instrumented_gcov_dump, gcda_file, "-l", 1)) {
        fprintf(stderr, "-l flag test failed\n");
    }
    merge_coverage_data(instrumented_gcov_dump, "gcov-dump.cc");
    
    // Test -p flag (dump positions)
    if (!execute_gcov_dump(instrumented_gcov_dump, gcda_file, "-p", 1)) {
        fprintf(stderr, "-p flag test failed\n");
    }
    merge_coverage_data(instrumented_gcov_dump, "gcov-dump.cc");
    
    // Test -r flag (dump raw)
    if (!execute_gcov_dump(instrumented_gcov_dump, gcda_file, "-r", 1)) {
        fprintf(stderr, "-r flag test failed\n");
    }
    merge_coverage_data(instrumented_gcov_dump, "gcov-dump.cc");
    
    // Test -s flag (dump stable)
    if (!execute_gcov_dump(instrumented_gcov_dump, gcda_file, "-s", 1)) {
        fprintf(stderr, "-s flag test failed\n");
    }
    merge_coverage_data(instrumented_gcov_dump, "gcov-dump.cc");
    
    printf("\n=== Testing Combined Flags ===\n");
    
    // Test combined flags (space-separated)
    if (!execute_gcov_dump(instrumented_gcov_dump, gcda_file, "-l -p -r -s", 1)) {
        fprintf(stderr, "Combined flags test failed\n");
    }
    merge_coverage_data(instrumented_gcov_dump, "gcov-dump.cc");
    
    // Test concatenated flags
    if (!execute_gcov_dump(instrumented_gcov_dump, gcda_file, "-lprs", 1)) {
        fprintf(stderr, "Concatenated flags test failed\n");
    }
    merge_coverage_data(instrumented_gcov_dump, "gcov-dump.cc");
    
    printf("\n=== Testing Invalid Flag ===\n");
    
    // Test invalid flag (should trigger default case)
    if (!execute_gcov_dump(instrumented_gcov_dump, gcda_file, "-x", 0)) {
        fprintf(stderr, "Invalid flag test failed\n");
    }
    merge_coverage_data(instrumented_gcov_dump, "gcov-dump.cc");
    
    // Additional test: no arguments (should show usage or error)
    printf("\n=== Testing No Arguments ===\n");
    execute_gcov_dump(instrumented_gcov_dump, NULL, "", 0);
    merge_coverage_data(instrumented_gcov_dump, "gcov-dump.cc");
    
    // Step 5: Generate final coverage report
    generate_coverage_report("gcov-dump.cc");
    
    printf("\n=== Test Complete ===\n");
    printf("Coverage data has been accumulated for gcov-dump.cc\n");
    printf("Check gcov-dump.cc.gcov for detailed coverage information\n");
    
    // Cleanup
    char cleanup_cmd[MAX_PATH];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", test_dir);
    system(cleanup_cmd);
    
    return 0;
}
