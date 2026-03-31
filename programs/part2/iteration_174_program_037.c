// File: test_gcov_dump_coverage.c
// This program tests the uncovered lines in gcov-dump.cc by executing
// gcov-dump with various command-line flags and verifying coverage.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024
#define MAX_CMD 4096

// Function prototypes
int compile_gcov_dump_with_coverage(const char *source_dir);
int compile_dummy_program(const char *dummy_source);
int run_gcov_dump(const char *gcov_dump_path, const char *gcda_file, const char *flags);
int merge_coverage_data(const char *gcov_dump_source);
int check_coverage(const char *gcov_dump_source);

int main(int argc, char *argv[]) {
    char cwd[MAX_PATH];
    char gcov_dump_path[MAX_PATH];
    char dummy_gcda_path[MAX_PATH];
    char gcov_dump_source[MAX_PATH];
    int result = 0;
    
    // Get current working directory
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed");
        return 1;
    }
    
    // Path to instrumented gcov-dump binary
    snprintf(gcov_dump_path, sizeof(gcov_dump_path), "%s/gcov-dump-instrumented", cwd);
    
    // Path to dummy.gcda file
    snprintf(dummy_gcda_path, sizeof(dummy_gcda_path), "%s/dummy.gcda", cwd);
    
    // Path to gcov-dump source (assuming it's in current directory)
    snprintf(gcov_dump_source, sizeof(gcov_dump_source), "%s/gcov-dump.cc", cwd);
    
    printf("=== Starting gcov-dump coverage test ===\n");
    
    // Step 1: Build instrumented gcov-dump if it doesn't exist
    printf("\n1. Checking for instrumented gcov-dump binary...\n");
    struct stat st;
    if (stat(gcov_dump_path, &st) != 0) {
        printf("Instrumented gcov-dump not found. Attempting to build...\n");
        if (compile_gcov_dump_with_coverage(cwd) != 0) {
            fprintf(stderr, "Failed to build instrumented gcov-dump\n");
            return 1;
        }
    } else {
        printf("Found instrumented gcov-dump at: %s\n", gcov_dump_path);
    }
    
    // Step 2: Generate test GCOV data file
    printf("\n2. Generating test GCOV data file...\n");
    if (compile_dummy_program(cwd) != 0) {
        fprintf(stderr, "Failed to generate test GCOV data\n");
        return 1;
    }
    
    // Step 3: Clear any existing coverage data
    printf("\n3. Clearing existing coverage data...\n");
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -f *.gcda *.gcno gcov-dump-instrumented.gcda 2>/dev/null");
    system(cmd);
    
    // Step 4: Execute gcov-dump with various flag combinations
    printf("\n4. Executing gcov-dump with flag combinations...\n");
    
    // Test individual flags
    printf("\n  Testing individual flags:\n");
    printf("  - Testing -h (help flag)...\n");
    if (run_gcov_dump(gcov_dump_path, NULL, "-h") != 0) {
        printf("    Warning: -h flag test failed\n");
    }
    
    printf("  - Testing -v (version flag)...\n");
    if (run_gcov_dump(gcov_dump_path, NULL, "-v") != 0) {
        printf("    Warning: -v flag test failed\n");
    }
    
    printf("  - Testing -l (dump contents flag)...\n");
    if (run_gcov_dump(gcov_dump_path, dummy_gcda_path, "-l") != 0) {
        printf("    Warning: -l flag test failed\n");
    }
    
    printf("  - Testing -p (dump positions flag)...\n");
    if (run_gcov_dump(gcov_dump_path, dummy_gcda_path, "-p") != 0) {
        printf("    Warning: -p flag test failed\n");
    }
    
    printf("  - Testing -r (dump raw flag)...\n");
    if (run_gcov_dump(gcov_dump_path, dummy_gcda_path, "-r") != 0) {
        printf("    Warning: -r flag test failed\n");
    }
    
    printf("  - Testing -s (dump stable flag)...\n");
    if (run_gcov_dump(gcov_dump_path, dummy_gcda_path, "-s") != 0) {
        printf("    Warning: -s flag test failed\n");
    }
    
    // Test combined flags (space-separated)
    printf("\n  Testing combined flags (space-separated):\n");
    printf("  - Testing -l -p -r -s...\n");
    if (run_gcov_dump(gcov_dump_path, dummy_gcda_path, "-l -p -r -s") != 0) {
        printf("    Warning: -l -p -r -s flag test failed\n");
    }
    
    // Test concatenated flags
    printf("\n  Testing concatenated flags:\n");
    printf("  - Testing -lprs...\n");
    if (run_gcov_dump(gcov_dump_path, dummy_gcda_path, "-lprs") != 0) {
        printf("    Warning: -lprs flag test failed\n");
    }
    
    // Test invalid flag (to trigger default case)
    printf("\n  Testing invalid flag (to trigger default case):\n");
    printf("  - Testing -x (invalid flag)...\n");
    if (run_gcov_dump(gcov_dump_path, dummy_gcda_path, "-x") != 0) {
        printf("    Expected: -x flag should fail with error message\n");
    }
    
    // Step 5: Merge coverage data
    printf("\n5. Merging coverage data...\n");
    if (merge_coverage_data(gcov_dump_source) != 0) {
        fprintf(stderr, "Failed to merge coverage data\n");
        result = 1;
    }
    
    // Step 6: Check coverage
    printf("\n6. Checking coverage...\n");
    if (check_coverage(gcov_dump_source) != 0) {
        fprintf(stderr, "Coverage check failed\n");
        result = 1;
    }
    
    printf("\n=== Coverage test completed ===\n");
    return result;
}

// Compile gcov-dump with coverage instrumentation
int compile_gcov_dump_with_coverage(const char *source_dir) {
    char cmd[MAX_CMD];
    
    // First check if we have the source file
    char source_path[MAX_PATH];
    snprintf(source_path, sizeof(source_path), "%s/gcov-dump.cc", source_dir);
    
    struct stat st;
    if (stat(source_path, &st) != 0) {
        fprintf(stderr, "gcov-dump.cc not found in %s\n", source_dir);
        return 1;
    }
    
    // Try to compile with typical GCC build tree structure
    // This is a simplified version - adjust paths as needed for your environment
    printf("Attempting to compile gcov-dump with coverage...\n");
    
    // Try a simple compilation first (may need adjustments for your environment)
    snprintf(cmd, sizeof(cmd), 
             "g++ -O0 -fprofile-arcs -ftest-coverage -I. "
             "-I../../include -I../../libiberty "
             "%s/gcov-dump.cc ../../libiberty/libiberty.a "
             "-o %s/gcov-dump-instrumented 2>&1",
             source_dir, source_dir);
    
    printf("Compilation command: %s\n", cmd);
    
    int compile_result = system(cmd);
    if (compile_result != 0) {
        // Try alternative compilation without library paths
        printf("First compilation attempt failed, trying alternative...\n");
        snprintf(cmd, sizeof(cmd),
                 "g++ -O0 -fprofile-arcs -ftest-coverage "
                 "%s/gcov-dump.cc -o %s/gcov-dump-instrumented 2>&1",
                 source_dir, source_dir);
        
        printf("Alternative compilation command: %s\n", cmd);
        compile_result = system(cmd);
    }
    
    if (compile_result != 0) {
        fprintf(stderr, "Failed to compile gcov-dump with coverage\n");
        return 1;
    }
    
    printf("Successfully compiled instrumented gcov-dump\n");
    return 0;
}

// Compile a dummy program to generate GCOV data
int compile_dummy_program(const char *dir) {
    char dummy_source[MAX_PATH];
    char dummy_exe[MAX_PATH];
    char cmd[MAX_CMD];
    
    // Create dummy.c source file
    snprintf(dummy_source, sizeof(dummy_source), "%s/dummy.c", dir);
    
    FILE *fp = fopen(dummy_source, "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return 1;
    }
    
    // Write a simple C program that will generate coverage data
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
    snprintf(dummy_exe, sizeof(dummy_exe), "%s/dummy_prog", dir);
    snprintf(cmd, sizeof(cmd),
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s 2>&1",
             dummy_source, dummy_exe);
    
    printf("Compiling dummy program: %s\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return 1;
    }
    
    // Run dummy program to generate .gcda file
    printf("Running dummy program to generate coverage data...\n");
    snprintf(cmd, sizeof(cmd), "cd %s && ./dummy_prog 2>&1", dir);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        return 1;
    }
    
    // Verify .gcda file was created
    char gcda_path[MAX_PATH];
    snprintf(gcda_path, sizeof(gcda_path), "%s/dummy.gcda", dir);
    struct stat st;
    if (stat(gcda_path, &st) != 0) {
        fprintf(stderr, "Failed to generate dummy.gcda file\n");
        return 1;
    }
    
    printf("Generated dummy.gcda file: %s\n", gcda_path);
    return 0;
}

// Run gcov-dump with specified flags
int run_gcov_dump(const char *gcov_dump_path, const char *gcda_file, const char *flags) {
    char cmd[MAX_CMD];
    int exit_status;
    
    // Build the command
    if (gcda_file) {
        snprintf(cmd, sizeof(cmd), "%s %s %s 2>&1", gcov_dump_path, flags, gcda_file);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, flags);
    }
    
    printf("    Command: %s\n", cmd);
    
    // Execute the command
    exit_status = system(cmd);
    
    // For invalid flag test, we expect non-zero exit status
    if (strstr(flags, "-x") != NULL) {
        if (exit_status == 0) {
            printf("    ERROR: Invalid flag -x should have failed but didn't\n");
            return 1;
        }
        printf("    OK: Invalid flag correctly failed\n");
        return 0;
    }
    
    // For valid flags, check exit status
    if (exit_status != 0) {
        printf("    WARNING: Command exited with status %d\n", exit_status);
    }
    
    return 0;
}

// Merge coverage data after each run
int merge_coverage_data(const char *gcov_dump_source) {
    char cmd[MAX_CMD];
    
    // First, find the .gcda file for gcov-dump-instrumented
    // It might be named gcov-dump-instrumented.gcda or based on the source file
    printf("Looking for coverage data files...\n");
    
    // Use gcov to process the coverage data
    snprintf(cmd, sizeof(cmd), "gcov -i %s 2>&1", gcov_dump_source);
    printf("Merging coverage: %s\n", cmd);
    
    if (system(cmd) != 0) {
        printf("Warning: gcov -i command failed, trying alternative...\n");
        
        // Try to find and copy any .gcda files
        system("find . -name \"*.gcda\" -exec cp {} . \\; 2>/dev/null");
        
        // Try gcov directly on the source
        snprintf(cmd, sizeof(cmd), "gcov %s 2>&1", gcov_dump_source);
        printf("Alternative: %s\n", cmd);
        
        if (system(cmd) != 0) {
            fprintf(stderr, "Failed to merge coverage data\n");
            return 1;
        }
    }
    
    return 0;
}

// Check coverage for the target lines
int check_coverage(const char *gcov_dump_source) {
    char cmd[MAX_CMD];
    char gcov_output[MAX_PATH];
    FILE *fp;
    
    // Generate coverage report
    snprintf(gcov_output, sizeof(gcov_output), "%s.gcov", gcov_dump_source);
    
    // Remove existing .gcov file
    snprintf(cmd, sizeof(cmd), "rm -f %s 2>/dev/null", gcov_output);
    system(cmd);
    
    // Generate detailed coverage report
    snprintf(cmd, sizeof(cmd), "gcov -b %s 2>&1", gcov_dump_source);
    printf("Generating coverage report: %s\n", cmd);
    
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to generate coverage report\n");
        return 1;
    }
    
    // Check if .gcov file was created
    struct stat st;
    if (stat(gcov_output, &st) != 0) {
        fprintf(stderr, "Coverage output file not found: %s\n", gcov_output);
        return 1;
    }
    
    // Read the coverage file and look for our target lines
    printf("\nChecking coverage for target lines (111-130)...\n");
    
    fp = fopen(gcov_output, "r");
    if (!fp) {
        perror("Failed to open coverage file");
        return 1;
    }
    
    char line[1024];
    int line_num;
    int target_lines_covered = 0;
    int total_target_lines = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        // Parse line number from gcov output
        if (sscanf(line, "%d:", &line_num) == 1) {
            if (line_num >= 111 && line_num <= 130) {
                total_target_lines++;
                
                // Check if line was executed (starts with execution count)
                if (line[9] != '-' && line[9] != '#') {  // Position 9 often has execution count
                    target_lines_covered++;
                    printf("  Line %d: COVERED\n", line_num);
                } else {
                    printf("  Line %d: NOT COVERED\n", line_num);
                }
            }
        }
    }
    
    fclose(fp);
    
    printf("\nCoverage Summary for lines 111-130:\n");
    printf("  Lines covered: %d/%d\n", target_lines_covered, total_target_lines);
    
    if (total_target_lines == 0) {
        printf("ERROR: Could not find target lines in coverage report\n");
        printf("Make sure line numbers in gcov-dump.cc match (111-130)\n");
        return 1;
    }
    
    if (target_lines_covered == total_target_lines) {
        printf("SUCCESS: All target lines are covered!\n");
        return 0;
    } else {
        printf("PARTIAL: Some target lines are not covered\n");
        return 1;
    }
}
