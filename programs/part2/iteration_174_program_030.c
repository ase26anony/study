/**
 * test_gcov_dump_coverage.c
 * 
 * Test program to cover the uncovered switch-case lines in gcov-dump.cc
 * Specifically targets lines 111-130 handling command-line flags.
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
 * Compile gcov-dump with coverage instrumentation
 */
int compile_instrumented_gcov_dump(const char *source_dir, const char *output_path) {
    char cmd[MAX_CMD];
    struct stat st;
    
    printf("Building instrumented gcov-dump...\n");
    
    // Check if source exists
    char source_path[MAX_PATH];
    snprintf(source_path, sizeof(source_path), "%s/gcov-dump.cc", source_dir);
    if (stat(source_path, &st) != 0) {
        fprintf(stderr, "Error: gcov-dump.cc not found at %s\n", source_path);
        return 0;
    }
    
    // Build command to compile gcov-dump with coverage
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-I%s -I%s/../../include -I%s/../../libiberty "
        "%s/gcov-dump.cc %s/../../libiberty/libiberty.a "
        "-o %s 2>&1",
        source_dir, source_dir, source_dir,
        source_dir, source_dir, output_path);
    
    printf("Compiling: %s\n", cmd);
    
    int result = system(cmd);
    if (result != 0) {
        fprintf(stderr, "Failed to compile instrumented gcov-dump\n");
        return 0;
    }
    
    if (stat(output_path, &st) != 0) {
        fprintf(stderr, "Compilation succeeded but output not found\n");
        return 0;
    }
    
    printf("Instrumented gcov-dump built successfully: %s\n", output_path);
    return 1;
}

/**
 * Create a simple C program to generate GCOV data
 */
int create_dummy_program(const char *dir) {
    char dummy_c_path[MAX_PATH];
    snprintf(dummy_c_path, sizeof(dummy_c_path), "%s/dummy.c", dir);
    
    FILE *fp = fopen(dummy_c_path, "w");
    if (!fp) {
        perror("Failed to create dummy.c");
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
    printf("Created dummy program at: %s\n", dummy_c_path);
    return 1;
}

/**
 * Compile and run dummy program to generate .gcda file
 */
int generate_gcda_file(const char *dir) {
    char cmd[MAX_CMD];
    char dummy_exe[MAX_PATH];
    char dummy_gcda[MAX_PATH];
    
    snprintf(dummy_exe, sizeof(dummy_exe), "%s/dummy_prog", dir);
    snprintf(dummy_gcda, sizeof(dummy_gcda), "%s/dummy.gcda", dir);
    
    // Remove existing .gcda file if present
    unlink(dummy_gcda);
    
    // Compile dummy program with coverage
    snprintf(cmd, sizeof(cmd),
        "gcc -O0 -fprofile-arcs -ftest-coverage %s/dummy.c -o %s 2>&1",
        dir, dummy_exe);
    
    printf("Compiling dummy program: %s\n", cmd);
    
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return 0;
    }
    
    // Run dummy program to generate .gcda
    printf("Running dummy program to generate .gcda...\n");
    if (system(dummy_exe) != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        return 0;
    }
    
    // Check if .gcda was created
    struct stat st;
    if (stat(dummy_gcda, &st) != 0) {
        fprintf(stderr, ".gcda file not generated: %s\n", dummy_gcda);
        return 0;
    }
    
    printf("Generated .gcda file: %s (size: %ld bytes)\n", dummy_gcda, (long)st.st_size);
    return 1;
}

/**
 * Run gcov-dump with specific arguments and merge coverage
 */
int run_gcov_dump_test(const char *gcov_dump_path, const char *gcda_path, 
                       const char *args, int expect_success) {
    char cmd[MAX_CMD];
    int result;
    
    printf("\n=== Testing: %s %s ===\n", gcov_dump_path, args);
    
    // Build the command
    if (strstr(args, ".gcda") == NULL && gcda_path != NULL) {
        // Add gcda path if not already in args
        snprintf(cmd, sizeof(cmd), "%s %s %s 2>&1", gcov_dump_path, args, gcda_path);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, args);
    }
    
    printf("Command: %s\n", cmd);
    
    // Execute the command
    result = system(cmd);
    
    // Check exit code
    int exit_code = WEXITSTATUS(result);
    printf("Exit code: %d\n", exit_code);
    
    if (expect_success && exit_code != 0) {
        fprintf(stderr, "ERROR: Expected success but got exit code %d\n", exit_code);
        return 0;
    }
    
    if (!expect_success && exit_code == 0) {
        fprintf(stderr, "ERROR: Expected failure but got exit code 0\n");
        return 0;
    }
    
    // Merge coverage data after each run
    char merge_cmd[MAX_CMD];
    snprintf(merge_cmd, sizeof(merge_cmd), "gcov -i %s 2>&1", gcov_dump_path);
    printf("Merging coverage...\n");
    system(merge_cmd);
    
    return 1;
}

/**
 * Generate final coverage report and check target lines
 */
void generate_coverage_report(const char *gcov_dump_path, const char *source_file) {
    char cmd[MAX_CMD];
    
    printf("\n=== Generating Coverage Report ===\n");
    
    // Generate human-readable coverage report
    snprintf(cmd, sizeof(cmd), "gcov -b %s 2>&1", gcov_dump_path);
    printf("Running: %s\n", cmd);
    system(cmd);
    
    // Look for gcov-dump.cc.gcov file
    char gcov_file[MAX_PATH];
    snprintf(gcov_file, sizeof(gcov_file), "%s.gcov", source_file);
    
    FILE *fp = fopen(gcov_file, "r");
    if (fp) {
        printf("\n=== Coverage Summary for Target Lines (111-130) ===\n");
        
        char line[1024];
        int line_num = 0;
        int target_lines_covered = 0;
        int target_lines_total = 0;
        
        while (fgets(line, sizeof(line), fp)) {
            // Parse gcov format: "    #####:   111:	case 'h':"
            if (strlen(line) > 10) {
                char *colon1 = strchr(line, ':');
                if (colon1) {
                    line_num = atoi(colon1 + 1);
                    
                    if (line_num >= 111 && line_num <= 130) {
                        target_lines_total++;
                        
                        // Check if line was executed
                        if (strstr(line, "#####") == NULL) {
                            target_lines_covered++;
                            printf("Line %3d: EXECUTED - %s", line_num, colon1 + 1);
                        } else {
                            printf("Line %3d: NOT EXECUTED - %s", line_num, colon1 + 1);
                        }
                    }
                }
            }
        }
        
        fclose(fp);
        
        printf("\nCoverage for target lines 111-130: %d/%d (%.1f%%)\n",
               target_lines_covered, target_lines_total,
               target_lines_total > 0 ? (100.0 * target_lines_covered / target_lines_total) : 0.0);
        
        if (target_lines_covered == target_lines_total) {
            printf("SUCCESS: All target lines covered!\n");
        } else {
            printf("WARNING: Not all target lines covered\n");
        }
    } else {
        printf("Could not open coverage file: %s\n", gcov_file);
    }
}

int main(int argc, char *argv[]) {
    char temp_dir[MAX_PATH];
    char gcov_dump_path[MAX_PATH];
    char gcda_path[MAX_PATH];
    char source_dir[MAX_PATH];
    int all_tests_passed = 1;
    
    printf("=== GCOV-Dump Coverage Test ===\n");
    
    // Determine source directory
    if (argc > 1) {
        strncpy(source_dir, argv[1], sizeof(source_dir));
    } else {
        // Try to find gcov-dump.cc in current directory
        strcpy(source_dir, ".");
    }
    
    // Create temporary directory for test files
    snprintf(temp_dir, sizeof(temp_dir), "/tmp/gcov_dump_test_%d", getpid());
    mkdir(temp_dir, 0755);
    printf("Using temp directory: %s\n", temp_dir);
    
    // Build instrumented gcov-dump
    snprintf(gcov_dump_path, sizeof(gcov_dump_path), "%s/gcov-dump-instrumented", temp_dir);
    if (!compile_instrumented_gcov_dump(source_dir, gcov_dump_path)) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    // Create and compile dummy program for GCOV data
    if (!create_dummy_program(temp_dir)) {
        fprintf(stderr, "Failed to create dummy program\n");
        return 1;
    }
    
    if (!generate_gcda_file(temp_dir)) {
        fprintf(stderr, "Failed to generate .gcda file\n");
        return 1;
    }
    
    snprintf(gcda_path, sizeof(gcda_path), "%s/dummy.gcda", temp_dir);
    
    // Clear any existing coverage data
    printf("\nClearing existing coverage data...\n");
    char clear_cmd[MAX_CMD];
    snprintf(clear_cmd, sizeof(clear_cmd), "rm -f *.gcda *.gcno 2>/dev/null");
    system(clear_cmd);
    
    // Test 1: Help flag (-h)
    if (!run_gcov_dump_test(gcov_dump_path, NULL, "-h", 1)) {
        all_tests_passed = 0;
    }
    
    // Test 2: Version flag (-v)
    if (!run_gcov_dump_test(gcov_dump_path, NULL, "-v", 1)) {
        all_tests_passed = 0;
    }
    
    // Test 3: Individual flags with gcda file
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-l", 1)) {
        all_tests_passed = 0;
    }
    
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-p", 1)) {
        all_tests_passed = 0;
    }
    
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-r", 1)) {
        all_tests_passed = 0;
    }
    
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-s", 1)) {
        all_tests_passed = 0;
    }
    
    // Test 4: Combined flags (space-separated)
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-l -p -r -s", 1)) {
        all_tests_passed = 0;
    }
    
    // Test 5: Concatenated flags
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-lprs", 1)) {
        all_tests_passed = 0;
    }
    
    // Test 6: Invalid flag (should trigger default case)
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-x", 0)) {
        all_tests_passed = 0;
    }
    
    // Test 7: Multiple gcda files (if available)
    // Create another gcda file
    char dummy2_gcda[MAX_PATH];
    snprintf(dummy2_gcda, sizeof(dummy2_gcda), "%s/dummy2.gcda", temp_dir);
    char copy_cmd[MAX_CMD];
    snprintf(copy_cmd, sizeof(copy_cmd), "cp %s %s", gcda_path, dummy2_gcda);
    system(copy_cmd);
    
    if (!run_gcov_dump_test(gcov_dump_path, NULL, "-l dummy.gcda dummy2.gcda", 1)) {
        all_tests_passed = 0;
    }
    
    // Generate final coverage report
    generate_coverage_report(gcov_dump_path, "gcov-dump.cc");
    
    // Cleanup
    printf("\n=== Test Complete ===\n");
    if (all_tests_passed) {
        printf("All tests passed successfully!\n");
    } else {
        printf("Some tests failed\n");
    }
    
    printf("Test files remain in: %s\n", temp_dir);
    printf("To clean up: rm -rf %s\n", temp_dir);
    
    return all_tests_passed ? 0 : 1;
}
