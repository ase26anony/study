/**
 * test_gcov_dump_coverage.c
 * 
 * This program tests the uncovered lines in gcov-dump.cc (lines 111-130)
 * by invoking an instrumented gcov-dump binary with various flag combinations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024
#define MAX_CMD 4096

/**
 * Compile a simple C program with coverage instrumentation
 * to generate a valid .gcda file for testing.
 */
int create_test_gcda_file(const char *temp_dir) {
    char dummy_c_path[MAX_PATH];
    char dummy_exe_path[MAX_PATH];
    char compile_cmd[MAX_CMD];
    char run_cmd[MAX_CMD];
    
    // Create dummy.c source file
    snprintf(dummy_c_path, sizeof(dummy_c_path), "%s/dummy.c", temp_dir);
    FILE *fp = fopen(dummy_c_path, "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return 0;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        printf(\"Value: %%d\\n\", i);\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    fclose(fp);
    
    // Compile dummy.c with coverage
    snprintf(dummy_exe_path, sizeof(dummy_exe_path), "%s/dummy_prog", temp_dir);
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s",
             dummy_c_path, dummy_exe_path);
    
    printf("Compiling test program: %s\n", compile_cmd);
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile dummy.c\n");
        return 0;
    }
    
    // Run the program to generate .gcda file
    snprintf(run_cmd, sizeof(run_cmd), "cd %s && ./dummy_prog", temp_dir);
    printf("Running test program to generate .gcda: %s\n", run_cmd);
    if (system(run_cmd) != 0) {
        fprintf(stderr, "Failed to run dummy_prog\n");
        return 0;
    }
    
    // Verify .gcda file was created
    char gcda_path[MAX_PATH];
    snprintf(gcda_path, sizeof(gcda_path), "%s/dummy.gcda", temp_dir);
    struct stat st;
    if (stat(gcda_path, &st) != 0 || st.st_size == 0) {
        fprintf(stderr, "Failed to create valid .gcda file\n");
        return 0;
    }
    
    printf("Successfully created .gcda file: %s\n", gcda_path);
    return 1;
}

/**
 * Build an instrumented version of gcov-dump if needed.
 * Returns the path to the instrumented binary.
 */
int build_instrumented_gcov_dump(const char *source_dir, const char *build_dir) {
    char gcov_dump_cc[MAX_PATH];
    char build_cmd[MAX_CMD];
    char binary_path[MAX_PATH];
    
    // Try to find gcov-dump.cc in common locations
    const char *possible_paths[] = {
        ".",
        "..",
        "../gcc",
        "../../gcc",
        source_dir,
        NULL
    };
    
    const char *found_path = NULL;
    for (int i = 0; possible_paths[i]; i++) {
        snprintf(gcov_dump_cc, sizeof(gcov_dump_cc), "%s/gcov-dump.cc", possible_paths[i]);
        struct stat st;
        if (stat(gcov_dump_cc, &st) == 0) {
            found_path = possible_paths[i];
            printf("Found gcov-dump.cc at: %s\n", gcov_dump_cc);
            break;
        }
    }
    
    if (!found_path) {
        fprintf(stderr, "Could not find gcov-dump.cc\n");
        return 0;
    }
    
    // Create build directory
    char mkdir_cmd[MAX_CMD];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", build_dir);
    system(mkdir_cmd);
    
    // Build instrumented gcov-dump
    snprintf(binary_path, sizeof(binary_path), "%s/gcov-dump-instrumented", build_dir);
    
    // Try to compile with minimal dependencies first
    snprintf(build_cmd, sizeof(build_cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../include -I../../include "
             "-I../../libiberty %s/gcov-dump.cc -o %s 2>/dev/null",
             found_path, binary_path);
    
    printf("Building instrumented gcov-dump: %s\n", build_cmd);
    if (system(build_cmd) != 0) {
        // Try alternative compilation
        fprintf(stderr, "First compilation attempt failed, trying alternative...\n");
        
        // Try with libiberty if available
        snprintf(build_cmd, sizeof(build_cmd),
                 "g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../include "
                 "%s/gcov-dump.cc -liberty -o %s 2>/dev/null",
                 found_path, binary_path);
        
        if (system(build_cmd) != 0) {
            fprintf(stderr, "Failed to build instrumented gcov-dump\n");
            fprintf(stderr, "You may need to adjust include paths or link libraries\n");
            return 0;
        }
    }
    
    // Verify the binary was created
    struct stat st;
    if (stat(binary_path, &st) != 0) {
        fprintf(stderr, "Instrumented binary not created\n");
        return 0;
    }
    
    printf("Successfully built instrumented gcov-dump: %s\n", binary_path);
    return 1;
}

/**
 * Run gcov-dump with specific arguments and merge coverage data.
 */
int run_gcov_dump_test(const char *gcov_dump_bin, const char *gcda_file, 
                       const char *args, int expect_success) {
    char cmd[MAX_CMD];
    int status;
    
    // Build the command
    if (gcda_file && strlen(gcda_file) > 0) {
        snprintf(cmd, sizeof(cmd), "%s %s %s", gcov_dump_bin, args, gcda_file);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_bin, args);
    }
    
    printf("\nRunning: %s\n", cmd);
    
    // Execute the command
    status = system(cmd);
    int exit_status = WEXITSTATUS(status);
    
    // Check if result matches expectation
    if (expect_success) {
        if (exit_status != 0) {
            fprintf(stderr, "Command failed unexpectedly (exit code: %d)\n", exit_status);
            return 0;
        }
    } else {
        if (exit_status == 0) {
            fprintf(stderr, "Command succeeded unexpectedly\n");
            return 0;
        }
    }
    
    // Merge coverage data after each run
    char merge_cmd[MAX_CMD];
    snprintf(merge_cmd, sizeof(merge_cmd), 
             "gcov -i gcov-dump.cc 2>/dev/null || "
             "gcov -b gcov-dump.cc 2>/dev/null || "
             "echo 'Coverage merge attempted'");
    
    system(merge_cmd);
    
    return 1;
}

/**
 * Main test driver
 */
int main(int argc, char *argv[]) {
    char temp_dir[MAX_PATH] = "/tmp/gcov_test_XXXXXX";
    char build_dir[MAX_PATH] = "/tmp/gcov_build_XXXXXX";
    char gcov_dump_bin[MAX_PATH];
    char gcda_file[MAX_PATH];
    
    printf("=== Starting gcov-dump coverage test ===\n");
    
    // Create temporary directories
    if (!mkdtemp(temp_dir)) {
        perror("Failed to create temp directory");
        return 1;
    }
    
    if (!mkdtemp(build_dir)) {
        perror("Failed to create build directory");
        return 1;
    }
    
    printf("Temp directory: %s\n", temp_dir);
    printf("Build directory: %s\n", build_dir);
    
    // Step 1: Create test .gcda file
    printf("\n--- Step 1: Creating test .gcda file ---\n");
    if (!create_test_gcda_file(temp_dir)) {
        fprintf(stderr, "Failed to create test .gcda file\n");
        return 1;
    }
    
    snprintf(gcda_file, sizeof(gcda_file), "%s/dummy.gcda", temp_dir);
    
    // Step 2: Build instrumented gcov-dump
    printf("\n--- Step 2: Building instrumented gcov-dump ---\n");
    
    // Try to use existing gcov-dump first (might already be instrumented)
    snprintf(gcov_dump_bin, sizeof(gcov_dump_bin), "./gcov-dump");
    struct stat st;
    
    if (stat(gcov_dump_bin, &st) != 0) {
        // Try to build it
        const char *source_dir = ".";  // Adjust if needed
        if (!build_instrumented_gcov_dump(source_dir, build_dir)) {
            fprintf(stderr, "Could not find or build instrumented gcov-dump\n");
            fprintf(stderr, "Please compile gcov-dump with: g++ -fprofile-arcs -ftest-coverage gcov-dump.cc -o gcov-dump\n");
            return 1;
        }
        snprintf(gcov_dump_bin, sizeof(gcov_dump_bin), "%s/gcov-dump-instrumented", build_dir);
    } else {
        printf("Using existing gcov-dump binary\n");
    }
    
    // Step 3: Run comprehensive flag tests
    printf("\n--- Step 3: Running flag coverage tests ---\n");
    
    int all_tests_passed = 1;
    
    // Test individual flags (lines 111-130)
    printf("\nTesting individual flags:\n");
    
    // Test -h (help) - should succeed without .gcda file
    if (!run_gcov_dump_test(gcov_dump_bin, "", "-h", 1)) {
        fprintf(stderr, "Test -h failed\n");
        all_tests_passed = 0;
    }
    
    // Test -v (version) - should succeed without .gcda file
    if (!run_gcov_dump_test(gcov_dump_bin, "", "-v", 1)) {
        fprintf(stderr, "Test -v failed\n");
        all_tests_passed = 0;
    }
    
    // Test -l (dump contents) - line 118
    if (!run_gcov_dump_test(gcov_dump_bin, gcda_file, "-l", 1)) {
        fprintf(stderr, "Test -l failed\n");
        all_tests_passed = 0;
    }
    
    // Test -p (dump positions) - line 121
    if (!run_gcov_dump_test(gcov_dump_bin, gcda_file, "-p", 1)) {
        fprintf(stderr, "Test -p failed\n");
        all_tests_passed = 0;
    }
    
    // Test -r (dump raw) - line 124
    if (!run_gcov_dump_test(gcov_dump_bin, gcda_file, "-r", 1)) {
        fprintf(stderr, "Test -r failed\n");
        all_tests_passed = 0;
    }
    
    // Test -s (dump stable) - line 127
    if (!run_gcov_dump_test(gcov_dump_bin, gcda_file, "-s", 1)) {
        fprintf(stderr, "Test -s failed\n");
        all_tests_passed = 0;
    }
    
    // Test combined flags (space-separated)
    printf("\nTesting combined flags (space-separated):\n");
    if (!run_gcov_dump_test(gcov_dump_bin, gcda_file, "-l -p -r -s", 1)) {
        fprintf(stderr, "Test -l -p -r -s failed\n");
        all_tests_passed = 0;
    }
    
    // Test concatenated flags
    printf("\nTesting concatenated flags:\n");
    if (!run_gcov_dump_test(gcov_dump_bin, gcda_file, "-lprs", 1)) {
        fprintf(stderr, "Test -lprs failed\n");
        all_tests_passed = 0;
    }
    
    // Test invalid flag (should trigger default case - line 129)
    printf("\nTesting invalid flag (should trigger default case):\n");
    if (!run_gcov_dump_test(gcov_dump_bin, gcda_file, "-x", 0)) {
        fprintf(stderr, "Test -x failed\n");
        all_tests_passed = 0;
    }
    
    // Test mixed valid and invalid flags
    printf("\nTesting mixed flags:\n");
    if (!run_gcov_dump_test(gcov_dump_bin, gcda_file, "-lp -x", 0)) {
        fprintf(stderr, "Test -lp -x failed\n");
        all_tests_passed = 0;
    }
    
    // Step 4: Generate final coverage report
    printf("\n--- Step 4: Generating coverage report ---\n");
    
    char coverage_cmd[MAX_CMD];
    snprintf(coverage_cmd, sizeof(coverage_cmd),
             "gcov -b %s/gcov-dump.cc 2>&1 | grep -A 20 'Lines executed:'",
             build_dir);
    
    printf("Running: %s\n", coverage_cmd);
    system(coverage_cmd);
    
    // Also try to get specific line coverage
    printf("\nChecking specific lines (111-130):\n");
    snprintf(coverage_cmd, sizeof(coverage_cmd),
             "gcov -b %s/gcov-dump.cc 2>&1 | grep -n -B 2 -A 2 '111\\|118\\|121\\|124\\|127\\|129'",
             build_dir);
    system(coverage_cmd);
    
    // Alternative: Use gcov with line-by-line output
    printf("\nDetailed line coverage:\n");
    snprintf(coverage_cmd, sizeof(coverage_cmd),
             "gcov -l %s/gcov-dump.cc 2>&1 | grep -E '^\\[.*\\].*11[1-9]|^\\[.*\\].*12[0-9]|^\\[.*\\].*130'",
             build_dir);
    system(coverage_cmd);
    
    // Cleanup
    printf("\n--- Cleaning up ---\n");
    char cleanup_cmd[MAX_CMD];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s %s", temp_dir, build_dir);
    system(cleanup_cmd);
    
    if (all_tests_passed) {
        printf("\n=== All tests passed! ===\n");
        printf("The uncovered lines in gcov-dump.cc (111-130) should now be covered.\n");
        printf("Check the .gcov file for gcov-dump.cc to verify coverage.\n");
        return 0;
    } else {
        printf("\n=== Some tests failed ===\n");
        return 1;
    }
}
