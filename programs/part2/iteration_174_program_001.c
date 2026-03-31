/**
 * test_gcov_dump_coverage.c
 * 
 * This program tests the command-line argument parsing logic in gcov-dump.cc
 * to cover lines 111-130 (switch-case handling for flags -l, -p, -r, -s, -h, -v).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024

/**
 * Compile gcov-dump with coverage instrumentation
 */
int compile_gcov_dump_with_coverage(const char *source_dir, const char *output_path) {
    char cmd[2048];
    int status;
    
    printf("Compiling gcov-dump with coverage instrumentation...\n");
    
    // Try to find gcov-dump source in common locations
    const char *possible_paths[] = {
        ".",
        "../gcc",
        "../../gcc",
        source_dir,
        NULL
    };
    
    char source_path[MAX_PATH] = "";
    struct stat st;
    
    for (int i = 0; possible_paths[i] != NULL; i++) {
        snprintf(source_path, sizeof(source_path), "%s/gcov-dump.cc", possible_paths[i]);
        if (stat(source_path, &st) == 0) {
            printf("Found gcov-dump source at: %s\n", source_path);
            break;
        }
    }
    
    if (strlen(source_path) == 0 || stat(source_path, &st) != 0) {
        // Try to find pre-built gcov-dump
        printf("Looking for pre-built gcov-dump...\n");
        if (system("which gcov-dump > /dev/null 2>&1") == 0) {
            // Use system gcov-dump and hope it's instrumented
            snprintf(cmd, sizeof(cmd), "cp `which gcov-dump` %s", output_path);
            status = system(cmd);
            if (status == 0) {
                printf("Using system gcov-dump\n");
                return 0;
            }
        }
        fprintf(stderr, "Error: Could not find gcov-dump source\n");
        return -1;
    }
    
    // Build command to compile gcov-dump with coverage
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include -I../../libiberty "
        "%s ../../libiberty/libiberty.a -o %s",
        source_path, output_path);
    
    printf("Compilation command: %s\n", cmd);
    status = system(cmd);
    
    if (status != 0) {
        // Try simpler compilation
        snprintf(cmd, sizeof(cmd),
            "g++ -O0 -fprofile-arcs -ftest-coverage %s -o %s",
            source_path, output_path);
        printf("Trying simpler compilation: %s\n", cmd);
        status = system(cmd);
    }
    
    return (status == 0) ? 0 : -1;
}

/**
 * Create a simple test program to generate .gcda files
 */
int create_test_gcda_file(const char *dummy_source, const char *gcda_file) {
    char cmd[1024];
    int status;
    
    printf("Creating test program to generate .gcda file...\n");
    
    // Create dummy.c source file
    FILE *fp = fopen(dummy_source, "w");
    if (!fp) {
        perror("Error creating dummy.c");
        return -1;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        printf(\"Test: %%d\\n\", i);\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    fclose(fp);
    
    // Compile dummy program with coverage
    snprintf(cmd, sizeof(cmd), 
        "gcc -O0 -fprofile-arcs -ftest-coverage %s -o dummy_prog", dummy_source);
    status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Error compiling dummy program\n");
        return -1;
    }
    
    // Run dummy program to generate .gcda file
    status = system("./dummy_prog > /dev/null 2>&1");
    if (status != 0) {
        fprintf(stderr, "Error running dummy program\n");
        return -1;
    }
    
    // Find the generated .gcda file
    if (system("find . -name \"*.gcda\" -type f | head -1 > gcda_path.txt") == 0) {
        FILE *gcda_fp = fopen("gcda_path.txt", "r");
        if (gcda_fp) {
            char path[MAX_PATH];
            if (fgets(path, sizeof(path), gcda_fp)) {
                // Remove newline
                path[strcspn(path, "\n")] = 0;
                snprintf(cmd, sizeof(cmd), "cp \"%s\" \"%s\"", path, gcda_file);
                system(cmd);
            }
            fclose(gcda_fp);
            system("rm -f gcda_path.txt");
        }
    }
    
    return 0;
}

/**
 * Run gcov-dump with specific arguments and check exit code
 */
int run_gcov_dump(const char *gcov_dump_path, const char *gcda_file, 
                  const char *args, int expect_success) {
    char cmd[2048];
    int status;
    
    printf("Running: %s %s\n", gcov_dump_path, args);
    
    // Build command
    if (strstr(args, gcda_file) == NULL) {
        // Arguments don't contain gcda file, add it
        snprintf(cmd, sizeof(cmd), "%s %s %s 2>&1", gcov_dump_path, args, gcda_file);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, args);
    }
    
    status = system(cmd);
    
    if (expect_success) {
        if (status != 0) {
            fprintf(stderr, "ERROR: Command failed (expected success): %s\n", cmd);
            return -1;
        }
    } else {
        if (status == 0) {
            fprintf(stderr, "ERROR: Command succeeded (expected failure): %s\n", cmd);
            return -1;
        }
    }
    
    return 0;
}

/**
 * Merge coverage data after each run
 */
void merge_coverage_data(const char *gcov_dump_source) {
    char cmd[1024];
    
    // Try different methods to merge coverage
    printf("Merging coverage data...\n");
    
    // Method 1: Use gcov -i
    snprintf(cmd, sizeof(cmd), "gcov -i %s > /dev/null 2>&1", gcov_dump_source);
    system(cmd);
    
    // Method 2: Copy .gcda files to common location
    system("find . -name \"*.gcda\" -exec cp {} . \\; 2>/dev/null");
    
    // Method 3: Use lcov to capture data
    system("lcov --capture --directory . --output-file coverage.info 2>/dev/null");
}

/**
 * Check if target lines are covered
 */
int check_coverage(const char *gcov_dump_source) {
    char cmd[1024];
    char line[256];
    FILE *fp;
    int target_lines_covered = 0;
    
    printf("\n=== Checking Coverage Results ===\n");
    
    // Generate coverage report
    snprintf(cmd, sizeof(cmd), "gcov -b %s", gcov_dump_source);
    fp = popen(cmd, "r");
    if (!fp) {
        perror("Error running gcov");
        return -1;
    }
    
    // Look for coverage of our target lines
    while (fgets(line, sizeof(line), fp)) {
        // Check for lines around 111-130
        if (strstr(line, "111:") || strstr(line, "112:") || 
            strstr(line, "120:") || strstr(line, "130:")) {
            printf("%s", line);
            if (strstr(line, "#####") == NULL) {
                target_lines_covered = 1;
            }
        }
        // Also print summary
        if (strstr(line, "Lines executed:") || 
            strstr(line, "Branches executed:") ||
            strstr(line, "Taken at least once:")) {
            printf("%s", line);
        }
    }
    
    pclose(fp);
    
    // Also check with lcov if available
    if (system("which lcov > /dev/null 2>&1") == 0) {
        system("genhtml coverage.info --output-directory coverage_report 2>/dev/null");
        printf("\nDetailed HTML report generated in coverage_report/\n");
    }
    
    return target_lines_covered ? 0 : -1;
}

int main(int argc, char *argv[]) {
    const char *gcov_dump_instrumented = "./gcov-dump-instrumented";
    const char *dummy_source = "./dummy.c";
    const char *gcda_file = "./test.gcda";
    const char *gcov_dump_source = "gcov-dump.cc";
    int all_tests_passed = 1;
    
    printf("=== GCOV-Dump Coverage Test ===\n\n");
    
    // Step 1: Build/Locate instrumented gcov-dump
    if (compile_gcov_dump_with_coverage((argc > 1) ? argv[1] : ".", 
                                        gcov_dump_instrumented) != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    // Check if the binary was created
    if (access(gcov_dump_instrumented, X_OK) != 0) {
        fprintf(stderr, "Instrumented gcov-dump not found or not executable\n");
        return 1;
    }
    
    // Step 2: Generate test GCOV data file
    if (create_test_gcda_file(dummy_source, gcda_file) != 0) {
        fprintf(stderr, "Failed to create test .gcda file\n");
        return 1;
    }
    
    // Verify gcda file exists
    if (access(gcda_file, R_OK) != 0) {
        fprintf(stderr, "Test .gcda file not found: %s\n", gcda_file);
        // Try to find any .gcda file
        system("find . -name \"*.gcda\" -type f | head -5");
        return 1;
    }
    
    printf("\n=== Running Flag Coverage Tests ===\n");
    
    // Step 3: Execute comprehensive flag combinations
    
    // Test 1: Help flag (-h) - should succeed without gcda file
    printf("\n1. Testing -h (help flag):\n");
    if (run_gcov_dump(gcov_dump_instrumented, "", "-h", 1) != 0) {
        all_tests_passed = 0;
    }
    merge_coverage_data(gcov_dump_source);
    
    // Test 2: Version flag (-v)
    printf("\n2. Testing -v (version flag):\n");
    if (run_gcov_dump(gcov_dump_instrumented, "", "-v", 1) != 0) {
        all_tests_passed = 0;
    }
    merge_coverage_data(gcov_dump_source);
    
    // Test 3: Individual flags with gcda file
    printf("\n3. Testing individual flags with gcda file:\n");
    
    printf("  3a. Testing -l:\n");
    if (run_gcov_dump(gcov_dump_instrumented, gcda_file, "-l", 1) != 0) {
        all_tests_passed = 0;
    }
    merge_coverage_data(gcov_dump_source);
    
    printf("  3b. Testing -p:\n");
    if (run_gcov_dump(gcov_dump_instrumented, gcda_file, "-p", 1) != 0) {
        all_tests_passed = 0;
    }
    merge_coverage_data(gcov_dump_source);
    
    printf("  3c. Testing -r:\n");
    if (run_gcov_dump(gcov_dump_instrumented, gcda_file, "-r", 1) != 0) {
        all_tests_passed = 0;
    }
    merge_coverage_data(gcov_dump_source);
    
    printf("  3d. Testing -s:\n");
    if (run_gcov_dump(gcov_dump_instrumented, gcda_file, "-s", 1) != 0) {
        all_tests_passed = 0;
    }
    merge_coverage_data(gcov_dump_source);
    
    // Test 4: Combined flags (space-separated)
    printf("\n4. Testing combined flags (space-separated):\n");
    if (run_gcov_dump(gcov_dump_instrumented, gcda_file, "-l -p -r -s", 1) != 0) {
        all_tests_passed = 0;
    }
    merge_coverage_data(gcov_dump_source);
    
    // Test 5: Concatenated flags
    printf("\n5. Testing concatenated flags:\n");
    if (run_gcov_dump(gcov_dump_instrumented, gcda_file, "-lprs", 1) != 0) {
        all_tests_passed = 0;
    }
    merge_coverage_data(gcov_dump_source);
    
    // Test 6: Invalid flag (should trigger default case)
    printf("\n6. Testing invalid flag (should trigger default case):\n");
    if (run_gcov_dump(gcov_dump_instrumented, gcda_file, "-x", 0) != 0) {
        all_tests_passed = 0;
    }
    merge_coverage_data(gcov_dump_source);
    
    // Test 7: Mixed valid and invalid (edge case)
    printf("\n7. Testing mixed valid and invalid flags:\n");
    if (run_gcov_dump(gcov_dump_instrumented, gcda_file, "-lp -x", 0) != 0) {
        all_tests_passed = 0;
    }
    merge_coverage_data(gcov_dump_source);
    
    // Step 4: Final coverage check
    printf("\n=== Final Coverage Analysis ===\n");
    if (check_coverage(gcov_dump_source) != 0) {
        printf("\nWARNING: Target lines may not be fully covered\n");
        all_tests_passed = 0;
    } else {
        printf("\nSUCCESS: Target lines appear to be covered!\n");
    }
    
    // Cleanup
    printf("\n=== Cleaning up ===\n");
    system("rm -f dummy.c dummy_prog *.gcda *.gcno *.gcov coverage.info 2>/dev/null");
    system("rm -rf coverage_report 2>/dev/null");
    
    if (all_tests_passed) {
        printf("\n✅ All tests passed!\n");
        return 0;
    } else {
        printf("\n❌ Some tests failed\n");
        return 1;
    }
}
