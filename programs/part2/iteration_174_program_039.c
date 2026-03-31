/**
 * test_gcov_dump_coverage.c
 * 
 * This program tests the uncovered lines in gcov-dump.cc (lines 111-130)
 * by invoking gcov-dump with various flag combinations to ensure
 * the switch-case handling for flags is executed.
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
 * to generate a valid .gcda file for gcov-dump to process
 */
int create_test_gcda_file(const char *base_dir) {
    char dummy_c_path[MAX_PATH];
    char dummy_exe_path[MAX_PATH];
    char compile_cmd[MAX_CMD];
    char run_cmd[MAX_CMD];
    
    // Create dummy.c source file
    snprintf(dummy_c_path, sizeof(dummy_c_path), "%s/dummy.c", base_dir);
    FILE *fp = fopen(dummy_c_path, "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return 0;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        printf(\"Iteration %%d\\n\", i);\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    fclose(fp);
    
    // Compile dummy.c with coverage instrumentation
    snprintf(dummy_exe_path, sizeof(dummy_exe_path), "%s/dummy_prog", base_dir);
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s",
             dummy_c_path, dummy_exe_path);
    
    printf("Compiling test program: %s\n", compile_cmd);
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile dummy.c\n");
        return 0;
    }
    
    // Run the program to generate .gcda file
    snprintf(run_cmd, sizeof(run_cmd), "cd %s && ./dummy_prog", base_dir);
    printf("Running test program to generate .gcda: %s\n", run_cmd);
    if (system(run_cmd) != 0) {
        fprintf(stderr, "Failed to run dummy_prog\n");
        return 0;
    }
    
    // Verify .gcda file was created
    char gcda_path[MAX_PATH];
    snprintf(gcda_path, sizeof(gcda_path), "%s/dummy.gcda", base_dir);
    struct stat st;
    if (stat(gcda_path, &st) != 0 || st.st_size == 0) {
        fprintf(stderr, "Failed to create valid .gcda file\n");
        return 0;
    }
    
    printf("Successfully created .gcda file: %s (%ld bytes)\n", 
           gcda_path, st.st_size);
    return 1;
}

/**
 * Build gcov-dump with coverage instrumentation if not already built
 */
int build_instrumented_gcov_dump(const char *source_dir, const char *build_dir) {
    char gcov_dump_path[MAX_PATH];
    char build_cmd[MAX_CMD];
    struct stat st;
    
    // Check if instrumented gcov-dump already exists
    snprintf(gcov_dump_path, sizeof(gcov_dump_path), 
             "%s/gcov-dump-instrumented", build_dir);
    
    if (stat(gcov_dump_path, &st) == 0) {
        printf("Instrumented gcov-dump already exists: %s\n", gcov_dump_path);
        return 1;
    }
    
    // Try to find gcov-dump source
    char source_path[MAX_PATH];
    snprintf(source_path, sizeof(source_path), "%s/gcov-dump.cc", source_dir);
    
    if (stat(source_path, &st) != 0) {
        // Try alternative location
        snprintf(source_path, sizeof(source_path), 
                 "%s/../gcc/gcov-dump.cc", source_dir);
        if (stat(source_path, &st) != 0) {
            fprintf(stderr, "Could not find gcov-dump.cc in %s\n", source_dir);
            return 0;
        }
    }
    
    // Build command for compiling gcov-dump with coverage
    // This assumes standard GCC build structure - adjust as needed
    snprintf(build_cmd, sizeof(build_cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage "
             "-I%s -I%s/../include -I%s/../libiberty "
             "%s %s/../libiberty/libiberty.a "
             "-o %s",
             source_dir, source_dir, source_dir,
             source_path, source_dir, gcov_dump_path);
    
    printf("Building instrumented gcov-dump: %s\n", build_cmd);
    if (system(build_cmd) != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 0;
    }
    
    printf("Successfully built instrumented gcov-dump: %s\n", gcov_dump_path);
    return 1;
}

/**
 * Run gcov-dump with specific arguments and merge coverage
 */
int run_gcov_dump_test(const char *gcov_dump_path, const char *gcda_path, 
                       const char *args, int expect_success) {
    char cmd[MAX_CMD];
    int status;
    
    // Construct command
    if (gcda_path && strlen(gcda_path) > 0) {
        snprintf(cmd, sizeof(cmd), "%s %s %s", gcov_dump_path, args, gcda_path);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_path, args);
    }
    
    printf("\nRunning: %s\n", cmd);
    
    // Execute command
    status = system(cmd);
    int exit_status = WEXITSTATUS(status);
    
    // Check if result matches expectation
    if (expect_success) {
        if (exit_status != 0) {
            fprintf(stderr, "ERROR: Command failed with exit code %d\n", exit_status);
            return 0;
        }
    } else {
        if (exit_status == 0) {
            fprintf(stderr, "ERROR: Expected failure but command succeeded\n");
            return 0;
        }
    }
    
    // Merge coverage data
    char merge_cmd[MAX_CMD];
    snprintf(merge_cmd, sizeof(merge_cmd),
             "gcov -i %s 2>/dev/null", gcov_dump_path);
    system(merge_cmd);
    
    return 1;
}

/**
 * Generate final coverage report and check target lines
 */
void generate_coverage_report(const char *gcov_dump_path, 
                              const char *source_path) {
    char cmd[MAX_CMD];
    
    printf("\n=== Generating Coverage Report ===\n");
    
    // Generate human-readable coverage report
    snprintf(cmd, sizeof(cmd), "gcov -b %s 2>&1 | grep -A5 -B5 'Lines 111-130'", 
             source_path);
    printf("Running: %s\n", cmd);
    system(cmd);
    
    // Also show summary
    printf("\n=== Coverage Summary ===\n");
    snprintf(cmd, sizeof(cmd), "gcov %s 2>&1 | tail -20", source_path);
    system(cmd);
}

int main(int argc, char *argv[]) {
    char base_dir[MAX_PATH];
    char source_dir[MAX_PATH];
    char gcov_dump_path[MAX_PATH];
    char gcda_path[MAX_PATH];
    int all_tests_passed = 1;
    
    // Get current directory
    if (getcwd(base_dir, sizeof(base_dir)) == NULL) {
        perror("getcwd failed");
        return 1;
    }
    
    // Use provided source directory or current directory
    if (argc > 1) {
        strncpy(source_dir, argv[1], sizeof(source_dir));
    } else {
        strncpy(source_dir, base_dir, sizeof(source_dir));
    }
    
    printf("=== Starting gcov-dump Coverage Test ===\n");
    printf("Base directory: %s\n", base_dir);
    printf("Source directory: %s\n", source_dir);
    
    // Step 1: Build instrumented gcov-dump
    if (!build_instrumented_gcov_dump(source_dir, base_dir)) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    snprintf(gcov_dump_path, sizeof(gcov_dump_path), 
             "%s/gcov-dump-instrumented", base_dir);
    
    // Step 2: Create test .gcda file
    if (!create_test_gcda_file(base_dir)) {
        fprintf(stderr, "Failed to create test .gcda file\n");
        return 1;
    }
    
    snprintf(gcda_path, sizeof(gcda_path), "%s/dummy.gcda", base_dir);
    
    // Step 3: Execute comprehensive flag tests
    
    // Test 1: Help flag (-h) - no .gcda file needed
    printf("\n--- Test 1: Help flag (-h) ---\n");
    if (!run_gcov_dump_test(gcov_dump_path, "", "-h", 1)) {
        all_tests_passed = 0;
    }
    
    // Test 2: Version flag (-v)
    printf("\n--- Test 2: Version flag (-v) ---\n");
    if (!run_gcov_dump_test(gcov_dump_path, "", "-v", 1)) {
        all_tests_passed = 0;
    }
    
    // Test 3: Individual flags with .gcda file
    printf("\n--- Test 3: Individual flags ---\n");
    
    printf("\nTesting -l flag\n");
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-l", 1)) {
        all_tests_passed = 0;
    }
    
    printf("\nTesting -p flag\n");
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-p", 1)) {
        all_tests_passed = 0;
    }
    
    printf("\nTesting -r flag\n");
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-r", 1)) {
        all_tests_passed = 0;
    }
    
    printf("\nTesting -s flag\n");
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-s", 1)) {
        all_tests_passed = 0;
    }
    
    // Test 4: Combined flags (space-separated)
    printf("\n--- Test 4: Combined flags (space-separated) ---\n");
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-l -p -r -s", 1)) {
        all_tests_passed = 0;
    }
    
    // Test 5: Concatenated flags
    printf("\n--- Test 5: Concatenated flags ---\n");
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-lprs", 1)) {
        all_tests_passed = 0;
    }
    
    // Test 6: Various flag combinations
    printf("\n--- Test 6: Various flag combinations ---\n");
    
    printf("\nTesting -lp combination\n");
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-lp", 1)) {
        all_tests_passed = 0;
    }
    
    printf("\nTesting -rs combination\n");
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-rs", 1)) {
        all_tests_passed = 0;
    }
    
    printf("\nTesting -lpr combination\n");
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-lpr", 1)) {
        all_tests_passed = 0;
    }
    
    // Test 7: Invalid flag (should trigger default case)
    printf("\n--- Test 7: Invalid flag (testing default case) ---\n");
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-x", 0)) {
        all_tests_passed = 0;
    }
    
    // Test 8: Multiple invalid flags
    printf("\n--- Test 8: Multiple invalid flags ---\n");
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-xyz", 0)) {
        all_tests_passed = 0;
    }
    
    // Test 9: Mix of valid and invalid flags
    printf("\n--- Test 9: Mix of valid and invalid flags ---\n");
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-lx", 0)) {
        all_tests_passed = 0;
    }
    
    // Step 4: Generate final coverage report
    char source_path[MAX_PATH];
    snprintf(source_path, sizeof(source_path), "%s/gcov-dump.cc", source_dir);
    generate_coverage_report(gcov_dump_path, source_path);
    
    // Step 5: Cleanup (optional)
    printf("\n=== Test Complete ===\n");
    if (all_tests_passed) {
        printf("All tests passed successfully!\n");
        
        // Show the specific coverage for our target lines
        printf("\n=== Verifying Target Lines Coverage ===\n");
        char check_cmd[MAX_CMD];
        snprintf(check_cmd, sizeof(check_cmd),
                 "echo 'Checking coverage for lines 111-130...'; "
                 "gcov -b %s 2>&1 | "
                 "sed -n '/^Lines executed:/p;/^.*111-130/p'",
                 source_path);
        system(check_cmd);
        
        return 0;
    } else {
        printf("Some tests failed!\n");
        return 1;
    }
}
