/**
 * test_gcov_dump_flags.c
 * 
 * Comprehensive test to cover gcov-dump.cc lines 111-130
 * Tests all command-line flag parsing cases: -h, -v, -l, -p, -r, -s, and invalid flags
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_PATH 1024

/**
 * Compile a simple test program to generate GCOV data files
 */
int create_test_gcda_file(const char *gcda_path) {
    const char *dummy_c = "dummy_test.c";
    const char *dummy_exe = "dummy_test_prog";
    
    // Create a minimal C program
    FILE *fp = fopen(dummy_c, "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return 0;
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
    
    // Compile with coverage instrumentation
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s",
             dummy_c, dummy_exe);
    
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile dummy test program\n");
        return 0;
    }
    
    // Execute to generate .gcda file
    if (system(dummy_exe) != 0) {
        fprintf(stderr, "Failed to run dummy test program\n");
        return 0;
    }
    
    // The .gcda file will be named after the source file
    // Copy/move it to the desired location
    snprintf(cmd, sizeof(cmd), "cp dummy_test.gcda %s", gcda_path);
    system(cmd);
    
    // Cleanup intermediate files
    unlink(dummy_c);
    unlink(dummy_exe);
    
    return 1;
}

/**
 * Find or build an instrumented gcov-dump binary
 */
char* get_instrumented_gcov_dump() {
    static char path[MAX_PATH];
    
    // First, try to find an existing instrumented binary
    const char *candidates[] = {
        "./gcov-dump-instrumented",
        "../gcov-dump/gcov-dump-instrumented",
        "gcov-dump-instrumented",
        NULL
    };
    
    for (int i = 0; candidates[i]; i++) {
        struct stat st;
        if (stat(candidates[i], &st) == 0 && (st.st_mode & S_IXUSR)) {
            realpath(candidates[i], path);
            return path;
        }
    }
    
    // If not found, try to build it
    printf("Building instrumented gcov-dump...\n");
    
    // Look for gcov-dump source
    const char *source_candidates[] = {
        "./gcov-dump.cc",
        "../gcov-dump/gcov-dump.cc",
        "gcov-dump.cc",
        NULL
    };
    
    char *source_path = NULL;
    for (int i = 0; source_candidates[i]; i++) {
        struct stat st;
        if (stat(source_candidates[i], &st) == 0) {
            source_path = realpath(source_candidates[i], NULL);
            break;
        }
    }
    
    if (!source_path) {
        fprintf(stderr, "Could not find gcov-dump.cc source file\n");
        return NULL;
    }
    
    // Extract directory for includes
    char source_dir[MAX_PATH];
    strcpy(source_dir, source_path);
    char *last_slash = strrchr(source_dir, '/');
    if (last_slash) *last_slash = '\0';
    
    // Build command - adjust paths as needed for your GCC build
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage "
             "-I%s -I%s/../../include -I%s/../../libiberty "
             "%s %s/../../libiberty/libiberty.a "
             "-o gcov-dump-instrumented",
             source_dir, source_dir, source_dir,
             source_path, source_dir);
    
    printf("Build command: %s\n", cmd);
    
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        free(source_path);
        return NULL;
    }
    
    free(source_path);
    realpath("./gcov-dump-instrumented", path);
    return path;
}

/**
 * Run gcov-dump with specific arguments and merge coverage
 */
int run_gcov_dump_test(const char *gcov_dump_path, const char *gcda_path, 
                       const char *args, int expect_success) {
    char cmd[4096];
    int status;
    
    // Build the command
    if (strstr(args, gcda_path) == NULL) {
        // Arguments don't include the gcda file
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_path, args);
    } else {
        // Arguments already include the gcda file
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_path, args);
    }
    
    printf("Running: %s\n", cmd);
    
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
    
    // Merge coverage data
    snprintf(cmd, sizeof(cmd), "gcov -i %s", gcov_dump_path);
    system(cmd);
    
    return 1;
}

/**
 * Generate final coverage report and check target lines
 */
int verify_coverage(const char *gcov_dump_path) {
    char cmd[1024];
    char report_file[1024];
    
    // Get the source file name from binary path
    const char *binary_name = strrchr(gcov_dump_path, '/');
    if (!binary_name) binary_name = gcov_dump_path;
    else binary_name++;
    
    // Generate coverage report
    snprintf(cmd, sizeof(cmd), "gcov -b %s", gcov_dump_path);
    printf("\nGenerating coverage report...\n");
    system(cmd);
    
    // The coverage file will be gcov-dump.cc.gcov
    // Check if our target lines are covered
    FILE *gcov_file = fopen("gcov-dump.cc.gcov", "r");
    if (!gcov_file) {
        fprintf(stderr, "Could not open coverage report\n");
        return 0;
    }
    
    int target_lines_covered = 0;
    char line[1024];
    int line_num;
    int count;
    char code[1024];
    
    printf("\nChecking coverage for target lines 111-130:\n");
    
    while (fgets(line, sizeof(line), gcov_file)) {
        if (sscanf(line, "%d:%d:%s", &count, &line_num, code) >= 2) {
            if (line_num >= 111 && line_num <= 130) {
                printf("Line %d: count = %d\n", line_num, count);
                if (count > 0) {
                    target_lines_covered++;
                }
            }
        }
    }
    
    fclose(gcov_file);
    
    int total_target_lines = 20; // Lines 111-130 inclusive
    printf("\nCovered %d of %d target lines\n", 
           target_lines_covered, total_target_lines);
    
    return target_lines_covered == total_target_lines;
}

int main() {
    char gcda_path[MAX_PATH];
    char *gcov_dump_path;
    int all_tests_passed = 1;
    
    printf("=== GCOV-DUMP Flag Coverage Test ===\n\n");
    
    // Step 1: Get instrumented gcov-dump binary
    gcov_dump_path = get_instrumented_gcov_dump();
    if (!gcov_dump_path) {
        fprintf(stderr, "Failed to get instrumented gcov-dump binary\n");
        return 1;
    }
    
    printf("Using gcov-dump: %s\n\n", gcov_dump_path);
    
    // Step 2: Create test GCOV data file
    snprintf(gcda_path, sizeof(gcda_path), "./test_data.gcda");
    if (!create_test_gcda_file(gcda_path)) {
        fprintf(stderr, "Failed to create test GCOV data file\n");
        return 1;
    }
    
    printf("Created test GCOV data: %s\n\n", gcda_path);
    
    // Step 3: Execute comprehensive flag tests
    
    // Test 1: Help flag (-h)
    printf("Test 1: Testing -h flag\n");
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-h", 1)) {
        all_tests_passed = 0;
    }
    
    // Test 2: Version flag (-v)
    printf("\nTest 2: Testing -v flag\n");
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-v", 1)) {
        all_tests_passed = 0;
    }
    
    // Test 3: Individual flags with data file
    printf("\nTest 3: Testing individual flags\n");
    
    printf("  Testing -l flag\n");
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-l test_data.gcda", 1)) {
        all_tests_passed = 0;
    }
    
    printf("  Testing -p flag\n");
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-p test_data.gcda", 1)) {
        all_tests_passed = 0;
    }
    
    printf("  Testing -r flag\n");
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-r test_data.gcda", 1)) {
        all_tests_passed = 0;
    }
    
    printf("  Testing -s flag\n");
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, "-s test_data.gcda", 1)) {
        all_tests_passed = 0;
    }
    
    // Test 4: Combined flags (space-separated)
    printf("\nTest 4: Testing combined flags (space-separated)\n");
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, 
                           "-l -p -r -s test_data.gcda", 1)) {
        all_tests_passed = 0;
    }
    
    // Test 5: Concatenated flags
    printf("\nTest 5: Testing concatenated flags\n");
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, 
                           "-lprs test_data.gcda", 1)) {
        all_tests_passed = 0;
    }
    
    // Test 6: Invalid flag (to trigger default case)
    printf("\nTest 6: Testing invalid flag (should fail)\n");
    if (!run_gcov_dump_test(gcov_dump_path, gcda_path, 
                           "-x test_data.gcda", 0)) {
        all_tests_passed = 0;
    }
    
    // Step 4: Verify coverage
    printf("\n=== Verifying Coverage ===\n");
    if (verify_coverage(gcov_dump_path)) {
        printf("\n✓ All target lines covered!\n");
    } else {
        printf("\n✗ Some target lines not covered\n");
        all_tests_passed = 0;
    }
    
    // Cleanup
    unlink(gcda_path);
    unlink("dummy_test.gcda");
    unlink("dummy_test.gcno");
    
    if (all_tests_passed) {
        printf("\n=== All tests passed ===\n");
        return 0;
    } else {
        printf("\n=== Some tests failed ===\n");
        return 1;
    }
}
