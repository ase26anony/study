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
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_PATH 1024
#define MAX_CMD 4096

/**
 * Compile a simple C program with coverage instrumentation
 * to generate a valid .gcda file for gcov-dump to process
 */
int create_test_gcda_file(const char *temp_dir) {
    char dummy_c_path[MAX_PATH];
    char dummy_exe_path[MAX_PATH];
    char cmd[MAX_CMD];
    
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
    fprintf(fp, "        printf(\"Hello %%d\\n\", i);\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    fclose(fp);
    
    // Compile dummy.c with coverage
    snprintf(dummy_exe_path, sizeof(dummy_exe_path), "%s/dummy", temp_dir);
    snprintf(cmd, sizeof(cmd), "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s", 
             dummy_c_path, dummy_exe_path);
    
    printf("Compiling test program: %s\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile dummy.c\n");
        return 0;
    }
    
    // Run the dummy program to generate .gcda file
    printf("Running test program to generate .gcda file\n");
    if (system(dummy_exe_path) != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        return 0;
    }
    
    // Verify .gcda file was created
    char gcda_path[MAX_PATH];
    snprintf(gcda_path, sizeof(gcda_path), "%s/dummy.gcda", temp_dir);
    struct stat st;
    if (stat(gcda_path, &st) != 0 || st.st_size == 0) {
        fprintf(stderr, "Failed to generate .gcda file\n");
        return 0;
    }
    
    printf("Successfully created .gcda file: %s\n", gcda_path);
    return 1;
}

/**
 * Execute gcov-dump with given arguments and merge coverage
 */
int run_gcov_dump(const char *gcov_dump_path, const char *gcda_path, 
                  const char *args, int expect_success) {
    char cmd[MAX_CMD];
    int status;
    
    // Build the command
    if (gcda_path && strlen(gcda_path) > 0) {
        snprintf(cmd, sizeof(cmd), "%s %s %s", gcov_dump_path, args, gcda_path);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_path, args);
    }
    
    printf("Executing: %s\n", cmd);
    
    // Execute the command
    status = system(cmd);
    
    // Check exit status
    if (expect_success) {
        if (WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Command failed (expected success): %s\n", cmd);
            return 0;
        }
    } else {
        if (WEXITSTATUS(status) == 0) {
            fprintf(stderr, "Command succeeded (expected failure): %s\n", cmd);
            return 0;
        }
    }
    
    return 1;
}

/**
 * Merge coverage data for gcov-dump.cc
 */
void merge_coverage(const char *gcov_dump_dir) {
    char cmd[MAX_CMD];
    
    // Change to directory containing gcov-dump's .gcda files
    chdir(gcov_dump_dir);
    
    // Merge coverage using gcov
    printf("Merging coverage data...\n");
    snprintf(cmd, sizeof(cmd), "gcov -i gcov-dump.cc 2>/dev/null || true");
    system(cmd);
    
    // Also try lcov if available for better reporting
    snprintf(cmd, sizeof(cmd), "lcov --capture --directory . --output-file gcov_dump.info 2>/dev/null || true");
    system(cmd);
}

/**
 * Check if lines 111-130 are covered in the gcov output
 */
void check_coverage() {
    FILE *fp;
    char line[1024];
    int target_lines_covered = 0;
    int in_target_section = 0;
    
    printf("\n=== Checking Coverage for gcov-dump.cc ===\n");
    
    // Read the gcov output file
    fp = fopen("gcov-dump.cc.gcov", "r");
    if (!fp) {
        printf("Could not open gcov-dump.cc.gcov\n");
        return;
    }
    
    while (fgets(line, sizeof(line), fp)) {
        // Look for lines 111-130
        int line_num;
        if (sscanf(line, "%d:", &line_num) == 1) {
            if (line_num >= 111 && line_num <= 130) {
                // Check if line is executed (starts with a number > 0)
                if (line[0] >= '0' && line[0] <= '9') {
                    int count = atoi(line);
                    if (count > 0) {
                        printf("Line %d: EXECUTED (%d times)\n", line_num, count);
                        target_lines_covered++;
                    } else {
                        printf("Line %d: NOT EXECUTED\n", line_num);
                    }
                } else if (strstr(line, "#####")) {
                    printf("Line %d: NOT EXECUTED (#####)\n", line_num);
                }
            }
        }
    }
    
    fclose(fp);
    
    printf("\n=== Coverage Summary ===\n");
    printf("Target lines (111-130): 20 lines total\n");
    printf("Lines executed: %d\n", target_lines_covered);
    printf("Coverage percentage: %.1f%%\n", (target_lines_covered / 20.0) * 100);
    
    if (target_lines_covered == 20) {
        printf("SUCCESS: All target lines covered!\n");
    } else {
        printf("WARNING: Not all target lines covered\n");
    }
}

int main(int argc, char *argv[]) {
    char temp_dir[MAX_PATH];
    char gcov_dump_path[MAX_PATH];
    char gcda_path[MAX_PATH];
    char cwd[MAX_PATH];
    int all_tests_passed = 1;
    
    // Get current directory
    if (!getcwd(cwd, sizeof(cwd))) {
        perror("getcwd failed");
        return 1;
    }
    
    // Create temporary directory for test files
    snprintf(temp_dir, sizeof(temp_dir), "%s/gcov_dump_test_%d", cwd, getpid());
    printf("Creating test directory: %s\n", temp_dir);
    
    if (mkdir(temp_dir, 0755) != 0) {
        perror("Failed to create test directory");
        return 1;
    }
    
    // Path to instrumented gcov-dump
    // First check if provided as argument, otherwise use default
    if (argc > 1) {
        strncpy(gcov_dump_path, argv[1], sizeof(gcov_dump_path));
    } else {
        // Try to find gcov-dump in common locations
        const char *possible_paths[] = {
            "./gcov-dump-instrumented",
            "../gcov/gcov-dump-instrumented",
            "gcov-dump",
            "/usr/bin/gcov-dump",
            NULL
        };
        
        int found = 0;
        for (int i = 0; possible_paths[i]; i++) {
            if (access(possible_paths[i], X_OK) == 0) {
                strncpy(gcov_dump_path, possible_paths[i], sizeof(gcov_dump_path));
                found = 1;
                break;
            }
        }
        
        if (!found) {
            fprintf(stderr, "Could not find gcov-dump executable.\n");
            fprintf(stderr, "Please build it with: g++ -O0 -fprofile-arcs -ftest-coverage gcov-dump.cc -o gcov-dump-instrumented\n");
            fprintf(stderr, "Or specify path as argument: %s /path/to/gcov-dump\n", argv[0]);
            rmdir(temp_dir);
            return 1;
        }
    }
    
    printf("Using gcov-dump: %s\n", gcov_dump_path);
    
    // Create test .gcda file
    if (!create_test_gcda_file(temp_dir)) {
        fprintf(stderr, "Failed to create test .gcda file\n");
        rmdir(temp_dir);
        return 1;
    }
    
    snprintf(gcda_path, sizeof(gcda_path), "%s/dummy.gcda", temp_dir);
    
    // Save original directory
    char original_dir[MAX_PATH];
    getcwd(original_dir, sizeof(original_dir));
    
    // Change to directory containing gcov-dump to ensure .gcda files are created there
    char gcov_dump_dir[MAX_PATH];
    strncpy(gcov_dump_dir, gcov_dump_path, sizeof(gcov_dump_dir));
    char *last_slash = strrchr(gcov_dump_dir, '/');
    if (last_slash) {
        *last_slash = '\0';
        if (strlen(gcov_dump_dir) == 0) {
            strcpy(gcov_dump_dir, ".");
        }
    } else {
        strcpy(gcov_dump_dir, ".");
    }
    
    printf("gcov-dump directory: %s\n", gcov_dump_dir);
    
    // Test 1: Help flag (-h)
    printf("\n=== Test 1: Help flag (-h) ===\n");
    if (!run_gcov_dump(gcov_dump_path, NULL, "-h", 1)) {
        all_tests_passed = 0;
    }
    merge_coverage(gcov_dump_dir);
    
    // Test 2: Version flag (-v)
    printf("\n=== Test 2: Version flag (-v) ===\n");
    if (!run_gcov_dump(gcov_dump_path, NULL, "-v", 1)) {
        all_tests_passed = 0;
    }
    merge_coverage(gcov_dump_dir);
    
    // Test 3: Contents flag (-l)
    printf("\n=== Test 3: Contents flag (-l) ===\n");
    if (!run_gcov_dump(gcov_dump_path, gcda_path, "-l", 1)) {
        all_tests_passed = 0;
    }
    merge_coverage(gcov_dump_dir);
    
    // Test 4: Positions flag (-p)
    printf("\n=== Test 4: Positions flag (-p) ===\n");
    if (!run_gcov_dump(gcov_dump_path, gcda_path, "-p", 1)) {
        all_tests_passed = 0;
    }
    merge_coverage(gcov_dump_dir);
    
    // Test 5: Raw flag (-r)
    printf("\n=== Test 5: Raw flag (-r) ===\n");
    if (!run_gcov_dump(gcov_dump_path, gcda_path, "-r", 1)) {
        all_tests_passed = 0;
    }
    merge_coverage(gcov_dump_dir);
    
    // Test 6: Stable flag (-s)
    printf("\n=== Test 6: Stable flag (-s) ===\n");
    if (!run_gcov_dump(gcov_dump_path, gcda_path, "-s", 1)) {
        all_tests_passed = 0;
    }
    merge_coverage(gcov_dump_dir);
    
    // Test 7: Combined flags (space-separated)
    printf("\n=== Test 7: Combined flags (space-separated) ===\n");
    if (!run_gcov_dump(gcov_dump_path, gcda_path, "-l -p -r -s", 1)) {
        all_tests_passed = 0;
    }
    merge_coverage(gcov_dump_dir);
    
    // Test 8: Concatenated flags
    printf("\n=== Test 8: Concatenated flags ===\n");
    if (!run_gcov_dump(gcov_dump_path, gcda_path, "-lprs", 1)) {
        all_tests_passed = 0;
    }
    merge_coverage(gcov_dump_dir);
    
    // Test 9: Invalid flag (to trigger default case)
    printf("\n=== Test 9: Invalid flag (-x) ===\n");
    if (!run_gcov_dump(gcov_dump_path, gcda_path, "-x", 0)) {
        all_tests_passed = 0;
    }
    merge_coverage(gcov_dump_dir);
    
    // Test 10: Multiple combinations with data file
    printf("\n=== Test 10: Various flag combinations ===\n");
    const char *combinations[] = {
        "-lp",
        "-lr",
        "-ls",
        "-pr",
        "-ps",
        "-rs",
        "-lpr",
        "-lps",
        "-lrs",
        "-prs",
        NULL
    };
    
    for (int i = 0; combinations[i]; i++) {
        printf("Testing: %s\n", combinations[i]);
        if (!run_gcov_dump(gcov_dump_path, gcda_path, combinations[i], 1)) {
            all_tests_passed = 0;
        }
        merge_coverage(gcov_dump_dir);
    }
    
    // Return to original directory and check coverage
    chdir(original_dir);
    check_coverage();
    
    // Cleanup
    printf("\nCleaning up test directory: %s\n", temp_dir);
    char cleanup_cmd[MAX_CMD];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
    system(cleanup_cmd);
    
    // Also clean up coverage files in gcov-dump directory
    chdir(gcov_dump_dir);
    system("rm -f *.gcda *.gcov *.info 2>/dev/null || true");
    chdir(original_dir);
    
    if (all_tests_passed) {
        printf("\nAll tests passed!\n");
        return 0;
    } else {
        printf("\nSome tests failed\n");
        return 1;
    }
}
