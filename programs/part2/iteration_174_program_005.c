/**
 * test_gcov_dump_coverage.c
 * 
 * This program tests the uncovered lines in gcov-dump.cc (lines 111-130)
 * by executing an instrumented gcov-dump binary with various flag combinations.
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
 * Build an instrumented version of gcov-dump
 */
int build_instrumented_gcov_dump(const char *source_dir, const char *output_path) {
    char cmd[MAX_CMD];
    struct stat st;
    
    printf("Building instrumented gcov-dump...\n");
    
    // Check if source files exist
    char gcov_dump_cc[MAX_PATH];
    snprintf(gcov_dump_cc, sizeof(gcov_dump_cc), "%s/gcov-dump.cc", source_dir);
    
    if (stat(gcov_dump_cc, &st) != 0) {
        fprintf(stderr, "Error: gcov-dump.cc not found at %s\n", gcov_dump_cc);
        return 0;
    }
    
    // Build command for instrumented gcov-dump
    // Note: Adjust paths based on your GCC source structure
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-I%s -I%s/../../include -I%s/../../libiberty "
        "%s/gcov-dump.cc %s/../../libiberty/libiberty.a "
        "-o %s",
        source_dir, source_dir, source_dir,
        source_dir, source_dir, output_path);
    
    printf("Executing: %s\n", cmd);
    
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 0;
    }
    
    if (stat(output_path, &st) != 0) {
        fprintf(stderr, "Instrumented binary not created\n");
        return 0;
    }
    
    printf("Instrumented gcov-dump built successfully: %s\n", output_path);
    return 1;
}

/**
 * Create a dummy program to generate GCOV data
 */
int create_dummy_program(const char *dummy_dir) {
    char dummy_c_path[MAX_PATH];
    char dummy_exe_path[MAX_PATH];
    char cmd[MAX_CMD];
    
    // Create directory if it doesn't exist
    if (mkdir(dummy_dir, 0755) != 0 && errno != EEXIST) {
        perror("Failed to create dummy directory");
        return 0;
    }
    
    // Create dummy.c source file
    snprintf(dummy_c_path, sizeof(dummy_c_path), "%s/dummy.c", dummy_dir);
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
    
    // Compile dummy program with coverage
    snprintf(dummy_exe_path, sizeof(dummy_exe_path), "%s/dummy", dummy_dir);
    snprintf(cmd, sizeof(cmd),
        "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s",
        dummy_c_path, dummy_exe_path);
    
    printf("Building dummy program: %s\n", cmd);
    
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to build dummy program\n");
        return 0;
    }
    
    // Execute dummy program to generate .gcda file
    printf("Executing dummy program to generate coverage data...\n");
    if (system(dummy_exe_path) != 0) {
        fprintf(stderr, "Failed to execute dummy program\n");
        return 0;
    }
    
    // Check if .gcda file was created
    char gcda_path[MAX_PATH];
    snprintf(gcda_path, sizeof(gcda_path), "%s/dummy.gcda", dummy_dir);
    struct stat st;
    if (stat(gcda_path, &st) != 0) {
        fprintf(stderr, "No .gcda file generated at %s\n", gcda_path);
        return 0;
    }
    
    printf("Dummy program created and executed. GCOV data at: %s\n", gcda_path);
    return 1;
}

/**
 * Execute gcov-dump with given arguments and merge coverage
 */
int execute_and_merge(const char *gcov_dump_path, const char *gcda_file, 
                      const char *args, int expect_success) {
    char cmd[MAX_CMD];
    int status;
    
    // Build the command
    if (gcda_file && strlen(gcda_file) > 0) {
        snprintf(cmd, sizeof(cmd), "%s %s %s", gcov_dump_path, args, gcda_file);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_path, args);
    }
    
    printf("\nExecuting: %s\n", cmd);
    
    // Execute the command
    status = system(cmd);
    int exit_status = WEXITSTATUS(status);
    
    // Check if execution matched expectations
    if (expect_success) {
        if (exit_status != 0) {
            fprintf(stderr, "Command failed with exit code %d (expected success)\n", exit_status);
            return 0;
        }
    } else {
        if (exit_status == 0) {
            fprintf(stderr, "Command succeeded (expected failure)\n");
            return 0;
        }
    }
    
    // Merge coverage data
    // First, find the .gcda file for gcov-dump
    char merge_cmd[MAX_CMD];
    snprintf(merge_cmd, sizeof(merge_cmd), 
             "gcov -i %s 2>/dev/null", gcov_dump_path);
    
    // This merges coverage from the current run
    system(merge_cmd);
    
    return 1;
}

/**
 * Generate final coverage report
 */
void generate_coverage_report(const char *gcov_dump_cc_path) {
    char cmd[MAX_CMD];
    
    printf("\n=== Generating Coverage Report ===\n");
    
    // Generate human-readable coverage report
    snprintf(cmd, sizeof(cmd), "gcov -b %s", gcov_dump_cc_path);
    printf("Executing: %s\n", cmd);
    system(cmd);
    
    // Also try to get line-by-line coverage
    printf("\n=== Checking target lines (111-130) ===\n");
    snprintf(cmd, sizeof(cmd), 
             "gcov -l %s 2>/dev/null | grep -A20 '^111:'", gcov_dump_cc_path);
    system(cmd);
}

int main(int argc, char *argv[]) {
    const char *gcov_source_dir = ".";  // Directory containing gcov-dump.cc
    const char *dummy_dir = "./test_data";
    char instrumented_gcov_dump[MAX_PATH];
    char dummy_gcda[MAX_PATH];
    char gcov_dump_cc_path[MAX_PATH];
    
    // Parse command line arguments
    if (argc > 1) {
        gcov_source_dir = argv[1];
    }
    
    // Set up paths
    snprintf(instrumented_gcov_dump, sizeof(instrumented_gcov_dump),
             "%s/gcov-dump-instrumented", gcov_source_dir);
    snprintf(dummy_gcda, sizeof(dummy_gcda), "%s/dummy.gcda", dummy_dir);
    snprintf(gcov_dump_cc_path, sizeof(gcov_dump_cc_path),
             "%s/gcov-dump.cc", gcov_source_dir);
    
    printf("=== GCOV Dump Coverage Test ===\n");
    printf("Source directory: %s\n", gcov_source_dir);
    printf("Instrumented binary: %s\n", instrumented_gcov_dump);
    printf("Dummy GCOV data: %s\n", dummy_gcda);
    
    // Step 1: Build instrumented gcov-dump
    if (!build_instrumented_gcov_dump(gcov_source_dir, instrumented_gcov_dump)) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    // Step 2: Create dummy program and generate GCOV data
    if (!create_dummy_program(dummy_dir)) {
        fprintf(stderr, "Failed to create dummy program\n");
        return 1;
    }
    
    // Step 3: Execute test series with various flag combinations
    
    // Test 1: Help flag (-h)
    printf("\n--- Test 1: Help flag (-h) ---");
    execute_and_merge(instrumented_gcov_dump, NULL, "-h", 1);
    
    // Test 2: Version flag (-v)
    printf("\n--- Test 2: Version flag (-v) ---");
    execute_and_merge(instrumented_gcov_dump, NULL, "-v", 1);
    
    // Test 3: Individual flags with GCOV data file
    printf("\n--- Test 3: Individual flags ---");
    
    // -l flag (dump contents)
    printf("\nTesting -l flag:");
    execute_and_merge(instrumented_gcov_dump, dummy_gcda, "-l", 1);
    
    // -p flag (dump positions)
    printf("\nTesting -p flag:");
    execute_and_merge(instrumented_gcov_dump, dummy_gcda, "-p", 1);
    
    // -r flag (dump raw)
    printf("\nTesting -r flag:");
    execute_and_merge(instrumented_gcov_dump, dummy_gcda, "-r", 1);
    
    // -s flag (dump stable)
    printf("\nTesting -s flag:");
    execute_and_merge(instrumented_gcov_dump, dummy_gcda, "-s", 1);
    
    // Test 4: Combined flags (space-separated)
    printf("\n--- Test 4: Combined flags (space-separated) ---");
    execute_and_merge(instrumented_gcov_dump, dummy_gcda, "-l -p -r -s", 1);
    
    // Test 5: Concatenated flags
    printf("\n--- Test 5: Concatenated flags ---");
    execute_and_merge(instrumented_gcov_dump, dummy_gcda, "-lprs", 1);
    
    // Test 6: Invalid flag (to trigger default case)
    printf("\n--- Test 6: Invalid flag (should trigger default case) ---");
    execute_and_merge(instrumented_gcov_dump, dummy_gcda, "-x", 0);
    
    // Test 7: Mixed valid and invalid flags
    printf("\n--- Test 7: Mixed flags ---");
    execute_and_merge(instrumented_gcov_dump, dummy_gcda, "-lp -x", 0);
    
    // Test 8: Long form arguments (if supported)
    printf("\n--- Test 8: Long arguments ---");
    // Some gcov-dump implementations might support --help, --version
    execute_and_merge(instrumented_gcov_dump, NULL, "--help 2>/dev/null", 1);
    execute_and_merge(instrumented_gcov_dump, NULL, "--version 2>/dev/null", 1);
    
    // Step 4: Generate final coverage report
    generate_coverage_report(gcov_dump_cc_path);
    
    // Step 5: Verify coverage programmatically
    printf("\n=== Verifying Coverage ===\n");
    
    // Create a simple verification by checking if .gcov file was created
    char gcov_file[MAX_PATH];
    snprintf(gcov_file, sizeof(gcov_file), "%s.gcov", gcov_dump_cc_path);
    
    FILE *gcov_fp = fopen(gcov_file, "r");
    if (gcov_fp) {
        char line[512];
        int target_lines_covered = 0;
        
        while (fgets(line, sizeof(line), gcov_fp)) {
            // Look for lines 111-130 in the coverage output
            int line_num;
            if (sscanf(line, "%*[^:]:%d:", &line_num) == 1) {
                if (line_num >= 111 && line_num <= 130) {
                    // Check if line was executed (non-zero execution count)
                    if (strstr(line, "######") == NULL) {
                        target_lines_covered++;
                    }
                }
            }
        }
        fclose(gcov_fp);
        
        printf("Lines 111-130 covered: %d/20\n", target_lines_covered);
        
        if (target_lines_covered >= 15) {  // Most lines should be covered
            printf("SUCCESS: Target lines adequately covered!\n");
        } else {
            printf("WARNING: Some target lines may not be covered\n");
        }
    } else {
        printf("Could not open .gcov file for verification\n");
    }
    
    // Cleanup
    printf("\n=== Cleaning up ===\n");
    char cleanup[MAX_CMD];
    snprintf(cleanup, sizeof(cleanup), "rm -rf %s *.gcda *.gcno", dummy_dir);
    system(cleanup);
    
    printf("\nTest completed. Check gcov-dump.cc.gcov for detailed coverage.\n");
    
    return 0;
}
