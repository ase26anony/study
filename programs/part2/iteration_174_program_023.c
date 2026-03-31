/**
 * test_gcov_dump_coverage.c
 * 
 * This program tests the command-line argument parsing logic in gcov-dump.cc
 * Specifically targeting lines 111-130 (switch-case handling for flags).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024

/**
 * Build an instrumented version of gcov-dump with coverage enabled
 */
int build_instrumented_gcov_dump(const char *source_dir, const char *output_path) {
    char cmd[2048];
    
    printf("Building instrumented gcov-dump...\n");
    
    // Check if source files exist
    char gcov_dump_cc[MAX_PATH];
    snprintf(gcov_dump_cc, sizeof(gcov_dump_cc), "%s/gcov-dump.cc", source_dir);
    
    struct stat st;
    if (stat(gcov_dump_cc, &st) != 0) {
        fprintf(stderr, "Error: gcov-dump.cc not found at %s\n", gcov_dump_cc);
        return 0;
    }
    
    // Build command - adjust paths based on your GCC source structure
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-I%s -I%s/../../include -I%s/../../libiberty "
        "%s "
        "%s/../../libiberty/libiberty.a "
        "-o %s",
        source_dir, source_dir, source_dir,
        gcov_dump_cc,
        source_dir,
        output_path);
    
    printf("Executing: %s\n", cmd);
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 0;
    }
    
    // Verify the binary was created
    if (stat(output_path, &st) != 0) {
        fprintf(stderr, "Instrumented binary not created\n");
        return 0;
    }
    
    printf("Instrumented gcov-dump built successfully: %s\n", output_path);
    return 1;
}

/**
 * Create a simple test program to generate GCOV data
 */
int create_test_gcov_data(const char *dummy_c_path, const char *gcda_path) {
    printf("Creating test GCOV data file...\n");
    
    // Create a simple C program
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
    
    // Compile with coverage
    char compile_cmd[1024];
    snprintf(compile_cmd, sizeof(compile_cmd),
        "gcc -O0 -fprofile-arcs -ftest-coverage %s -o dummy_prog",
        dummy_c_path);
    
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return 0;
    }
    
    // Run the program to generate .gcda file
    if (system("./dummy_prog > /dev/null 2>&1") != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        return 0;
    }
    
    // Check if .gcda was created
    struct stat st;
    if (stat("dummy.gcda", &st) != 0) {
        fprintf(stderr, "No dummy.gcda file generated\n");
        return 0;
    }
    
    // Copy to our target location
    char copy_cmd[1024];
    snprintf(copy_cmd, sizeof(copy_cmd), "cp dummy.gcda %s", gcda_path);
    system(copy_cmd);
    
    printf("Test GCOV data created: %s\n", gcda_path);
    return 1;
}

/**
 * Run gcov-dump with specific arguments and merge coverage
 */
void run_gcov_dump_test(const char *gcov_dump_path, const char *gcda_path, 
                       const char *args, int expect_success) {
    char cmd[2048];
    int result;
    
    printf("\n=== Testing: gcov-dump %s ===\n", args);
    
    // Build the full command
    if (strstr(args, "dummy.gcda") == NULL && 
        strcmp(args, "-h") != 0 && 
        strcmp(args, "-v") != 0 &&
        strstr(args, "-x") == NULL) {
        // Commands that need a data file
        snprintf(cmd, sizeof(cmd), "%s %s %s", gcov_dump_path, args, gcda_path);
    } else {
        // Commands that don't need a data file or have it already in args
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_path, args);
    }
    
    printf("Executing: %s\n", cmd);
    result = system(cmd);
    
    // Check exit code
    int exit_code = WEXITSTATUS(result);
    printf("Exit code: %d\n", exit_code);
    
    if (expect_success && exit_code != 0) {
        fprintf(stderr, "Warning: Expected success but got exit code %d\n", exit_code);
    } else if (!expect_success && exit_code == 0) {
        fprintf(stderr, "Warning: Expected failure but got exit code 0\n");
    }
    
    // Merge coverage data after each run
    char merge_cmd[1024];
    snprintf(merge_cmd, sizeof(merge_cmd), 
             "gcov -i %s/gcov-dump.cc > /dev/null 2>&1", 
             ".");
    system(merge_cmd);
}

/**
 * Generate final coverage report and check target lines
 */
void generate_coverage_report(const char *source_dir) {
    printf("\n=== Generating Coverage Report ===\n");
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "gcov -b %s/gcov-dump.cc", source_dir);
    system(cmd);
    
    // Check if gcov-dump.cc.gcov was created
    FILE *fp = fopen("gcov-dump.cc.gcov", "r");
    if (!fp) {
        fprintf(stderr, "No coverage file generated\n");
        return;
    }
    
    printf("\n=== Checking Coverage for Target Lines (111-130) ===\n");
    
    char line[1024];
    int in_target_range = 0;
    int target_lines_covered = 0;
    int target_lines_total = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        // Parse gcov format: "    #####:   30:   flag_dump_contents = 1;"
        if (strlen(line) > 10) {
            int line_num;
            char coverage[10];
            
            if (sscanf(line, "%s %d:", coverage, &line_num) == 2) {
                if (line_num >= 111 && line_num <= 130) {
                    target_lines_total++;
                    
                    // Check if line is executed (not "#####")
                    if (strstr(coverage, "#####") == NULL) {
                        target_lines_covered++;
                        printf("Line %d: COVERED (%s)\n", line_num, coverage);
                    } else {
                        printf("Line %d: NOT COVERED\n", line_num);
                    }
                }
            }
        }
    }
    
    fclose(fp);
    
    printf("\n=== Coverage Summary for Lines 111-130 ===\n");
    printf("Lines covered: %d/%d (%.1f%%)\n", 
           target_lines_covered, target_lines_total,
           target_lines_total > 0 ? (100.0 * target_lines_covered / target_lines_total) : 0.0);
    
    if (target_lines_covered == target_lines_total) {
        printf("SUCCESS: All target lines covered!\n");
    } else {
        printf("WARNING: Not all target lines covered\n");
    }
}

int main(int argc, char *argv[]) {
    const char *source_dir = ".";  // Directory containing gcov-dump.cc
    const char *gcov_dump_instrumented = "./gcov-dump-instrumented";
    const char *dummy_c_path = "./dummy.c";
    const char *gcda_path = "./dummy.gcda";
    
    printf("=== GCOV-Dump Coverage Test ===\n");
    
    // Step 1: Build instrumented gcov-dump
    if (!build_instrumented_gcov_dump(source_dir, gcov_dump_instrumented)) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    // Step 2: Generate test GCOV data
    if (!create_test_gcov_data(dummy_c_path, gcda_path)) {
        fprintf(stderr, "Failed to create test GCOV data\n");
        return 1;
    }
    
    // Step 3: Clean up any existing coverage data
    system("rm -f *.gcda *.gcno gcov-dump.cc.gcov 2>/dev/null");
    
    // Step 4: Execute comprehensive flag testing
    // Test individual flags (requires data file)
    run_gcov_dump_test(gcov_dump_instrumented, gcda_path, "-l", 1);
    run_gcov_dump_test(gcov_dump_instrumented, gcda_path, "-p", 1);
    run_gcov_dump_test(gcov_dump_instrumented, gcda_path, "-r", 1);
    run_gcov_dump_test(gcov_dump_instrumented, gcda_path, "-s", 1);
    
    // Test help and version (don't need data file)
    run_gcov_dump_test(gcov_dump_instrumented, gcda_path, "-h", 1);
    run_gcov_dump_test(gcov_dump_instrumented, gcda_path, "-v", 1);
    
    // Test combined flags (space separated)
    run_gcov_dump_test(gcov_dump_instrumented, gcda_path, "-l -p -r -s", 1);
    
    // Test concatenated flags
    run_gcov_dump_test(gcov_dump_instrumented, gcda_path, "-lprs", 1);
    
    // Test invalid flag (should trigger default case)
    run_gcov_dump_test(gcov_dump_instrumented, gcda_path, "-x dummy.gcda", 0);
    
    // Test mixed valid flags with data file in middle
    run_gcov_dump_test(gcov_dump_instrumented, gcda_path, "-l -p dummy.gcda -r -s", 1);
    
    // Step 5: Generate and analyze coverage report
    generate_coverage_report(source_dir);
    
    // Cleanup
    printf("\n=== Cleaning up ===\n");
    system("rm -f dummy.c dummy_prog dummy.gcda dummy.gcno 2>/dev/null");
    
    printf("\n=== Test Complete ===\n");
    return 0;
}
