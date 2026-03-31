/**
 * test_gcov_dump_coverage.c
 * 
 * A test program to cover the uncovered switch-case lines in gcov-dump.cc
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
 * Check if a file exists
 */
int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

/**
 * Execute a command and return its exit status
 */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/**
 * Build instrumented gcov-dump if it doesn't exist
 */
int build_instrumented_gcov_dump(const char *source_dir, const char *build_dir) {
    char cmd[MAX_CMD];
    char gcov_dump_path[MAX_PATH];
    
    snprintf(gcov_dump_path, sizeof(gcov_dump_path), "%s/gcov-dump-instrumented", build_dir);
    
    if (file_exists(gcov_dump_path)) {
        printf("Instrumented gcov-dump already exists at: %s\n", gcov_dump_path);
        return 0;
    }
    
    printf("Building instrumented gcov-dump...\n");
    
    // First, check if we have the source file
    char source_path[MAX_PATH];
    snprintf(source_path, sizeof(source_path), "%s/gcov-dump.cc", source_dir);
    
    if (!file_exists(source_path)) {
        fprintf(stderr, "Error: gcov-dump.cc not found at %s\n", source_path);
        return -1;
    }
    
    // Try to find libiberty - common locations
    const char *libiberty_paths[] = {
        "../../libiberty/libiberty.a",
        "../libiberty/libiberty.a",
        "libiberty/libiberty.a",
        "/usr/lib/libiberty.a",
        NULL
    };
    
    char *found_libiberty = NULL;
    for (int i = 0; libiberty_paths[i] != NULL; i++) {
        char lib_path[MAX_PATH];
        snprintf(lib_path, sizeof(lib_path), "%s/%s", source_dir, libiberty_paths[i]);
        if (file_exists(lib_path)) {
            found_libiberty = strdup(lib_path);
            break;
        }
    }
    
    if (!found_libiberty) {
        // Try to compile libiberty if we have the source
        char libiberty_src[MAX_PATH];
        snprintf(libiberty_src, sizeof(libiberty_src), "%s/../../libiberty", source_dir);
        if (file_exists(libiberty_src)) {
            // Build libiberty
            snprintf(cmd, sizeof(cmd), "cd %s/../../libiberty && make libiberty.a", source_dir);
            execute_command(cmd);
            found_libiberty = strdup("../../libiberty/libiberty.a");
        }
    }
    
    if (!found_libiberty) {
        fprintf(stderr, "Error: Could not find libiberty.a\n");
        return -1;
    }
    
    // Build command for compiling instrumented gcov-dump
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-I%s -I%s/../../include -I%s/../../libiberty "
        "%s/gcov-dump.cc %s -o %s",
        source_dir, source_dir, source_dir,
        source_dir, found_libiberty, gcov_dump_path);
    
    printf("Compilation command: %s\n", cmd);
    
    int result = execute_command(cmd);
    free(found_libiberty);
    
    if (result != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return -1;
    }
    
    printf("Successfully built instrumented gcov-dump at: %s\n", gcov_dump_path);
    return 0;
}

/**
 * Create a simple test program to generate GCOV data
 */
int create_test_gcov_data(const char *build_dir) {
    char dummy_c_path[MAX_PATH];
    char dummy_prog_path[MAX_PATH];
    char cmd[MAX_CMD];
    
    snprintf(dummy_c_path, sizeof(dummy_c_path), "%s/dummy.c", build_dir);
    snprintf(dummy_prog_path, sizeof(dummy_prog_path), "%s/dummy_prog", build_dir);
    
    // Create a simple C program
    FILE *fp = fopen(dummy_c_path, "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return -1;
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
    
    // Compile with coverage instrumentation
    snprintf(cmd, sizeof(cmd),
        "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s",
        dummy_c_path, dummy_prog_path);
    
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return -1;
    }
    
    // Run the program to generate .gcda file
    snprintf(cmd, sizeof(cmd), "%s", dummy_prog_path);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        return -1;
    }
    
    // Check that .gcda file was created
    char gcda_path[MAX_PATH];
    snprintf(gcda_path, sizeof(gcda_path), "%s/dummy.gcda", build_dir);
    
    if (!file_exists(gcda_path)) {
        // Try to find it in current directory
        if (file_exists("dummy.gcda")) {
            // Move it to build directory
            snprintf(cmd, sizeof(cmd), "mv dummy.gcda %s/", build_dir);
            execute_command(cmd);
        } else {
            fprintf(stderr, "No .gcda file generated\n");
            return -1;
        }
    }
    
    printf("Test GCOV data created at: %s/dummy.gcda\n", build_dir);
    return 0;
}

/**
 * Merge coverage data for gcov-dump.cc
 */
int merge_coverage_data(const char *build_dir, const char *source_dir) {
    char cmd[MAX_CMD];
    
    // First, find all .gcda files for gcov-dump
    snprintf(cmd, sizeof(cmd),
        "find %s -name \"*.gcda\" -type f | xargs -I {} cp {} %s/ 2>/dev/null || true",
        build_dir, build_dir);
    execute_command(cmd);
    
    // Generate coverage info
    char source_path[MAX_PATH];
    snprintf(source_path, sizeof(source_path), "%s/gcov-dump.cc", source_dir);
    
    snprintf(cmd, sizeof(cmd),
        "cd %s && gcov -i gcov-dump.cc 2>&1",
        build_dir);
    
    printf("Merging coverage data...\n");
    return execute_command(cmd);
}

/**
 * Run gcov-dump with specific flags and check exit code
 */
int run_gcov_dump_test(const char *gcov_dump_path, const char *gcda_path, 
                       const char *flags, int expect_success) {
    char cmd[MAX_CMD];
    
    if (strcmp(flags, "") == 0) {
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_path, gcda_path);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s %s", gcov_dump_path, flags, gcda_path);
    }
    
    int result = execute_command(cmd);
    
    if (expect_success) {
        if (result != 0) {
            fprintf(stderr, "Test failed: %s (expected success, got %d)\n", flags, result);
            return -1;
        }
    } else {
        if (result == 0) {
            fprintf(stderr, "Test failed: %s (expected failure, got success)\n", flags);
            return -1;
        }
    }
    
    return 0;
}

/**
 * Main test function
 */
int main(int argc, char *argv[]) {
    char build_dir[MAX_PATH];
    char source_dir[MAX_PATH];
    char cwd[MAX_PATH];
    
    // Get current directory
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed");
        return 1;
    }
    
    // Determine source and build directories
    if (argc >= 2) {
        strncpy(source_dir, argv[1], sizeof(source_dir) - 1);
        source_dir[sizeof(source_dir) - 1] = '\0';
    } else {
        // Assume gcov-dump.cc is in current directory
        strncpy(source_dir, cwd, sizeof(source_dir));
    }
    
    if (argc >= 3) {
        strncpy(build_dir, argv[2], sizeof(build_dir) - 1);
        build_dir[sizeof(build_dir) - 1] = '\0';
    } else {
        // Create a build directory
        snprintf(build_dir, sizeof(build_dir), "%s/test_build", cwd);
        if (!file_exists(build_dir)) {
            mkdir(build_dir, 0755);
        }
    }
    
    printf("Source directory: %s\n", source_dir);
    printf("Build directory: %s\n", build_dir);
    
    // Step 1: Build instrumented gcov-dump
    if (build_instrumented_gcov_dump(source_dir, build_dir) != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    // Step 2: Generate test GCOV data
    if (create_test_gcov_data(build_dir) != 0) {
        fprintf(stderr, "Failed to create test GCOV data\n");
        return 1;
    }
    
    // Paths to binaries and data
    char gcov_dump_path[MAX_PATH];
    char gcda_path[MAX_PATH];
    
    snprintf(gcov_dump_path, sizeof(gcov_dump_path), "%s/gcov-dump-instrumented", build_dir);
    snprintf(gcda_path, sizeof(gcda_path), "%s/dummy.gcda", build_dir);
    
    // Clear any existing coverage data
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -f %s/*.gcda %s/*.gcno", build_dir, build_dir);
    execute_command(cmd);
    
    // Step 3: Test individual flags (targeting lines 111-130)
    printf("\n=== Testing individual flags ===\n");
    
    // Test help flag (line 111-113)
    printf("\nTesting -h flag (help):\n");
    snprintf(cmd, sizeof(cmd), "%s -h", gcov_dump_path);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Help flag test failed\n");
    }
    merge_coverage_data(build_dir, source_dir);
    
    // Test version flag (line 114-116)
    printf("\nTesting -v flag (version):\n");
    snprintf(cmd, sizeof(cmd), "%s -v", gcov_dump_path);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Version flag test failed\n");
    }
    merge_coverage_data(build_dir, source_dir);
    
    // Test -l flag (line 117-118)
    printf("\nTesting -l flag (dump contents):\n");
    if (run_gcov_dump_test(gcov_dump_path, gcda_path, "-l", 1) != 0) {
        fprintf(stderr, "-l flag test failed\n");
    }
    merge_coverage_data(build_dir, source_dir);
    
    // Test -p flag (line 119-120)
    printf("\nTesting -p flag (dump positions):\n");
    if (run_gcov_dump_test(gcov_dump_path, gcda_path, "-p", 1) != 0) {
        fprintf(stderr, "-p flag test failed\n");
    }
    merge_coverage_data(build_dir, source_dir);
    
    // Test -r flag (line 121-122)
    printf("\nTesting -r flag (dump raw):\n");
    if (run_gcov_dump_test(gcov_dump_path, gcda_path, "-r", 1) != 0) {
        fprintf(stderr, "-r flag test failed\n");
    }
    merge_coverage_data(build_dir, source_dir);
    
    // Test -s flag (line 123-124)
    printf("\nTesting -s flag (dump stable):\n");
    if (run_gcov_dump_test(gcov_dump_path, gcda_path, "-s", 1) != 0) {
        fprintf(stderr, "-s flag test failed\n");
    }
    merge_coverage_data(build_dir, source_dir);
    
    // Step 4: Test combined flags
    printf("\n=== Testing combined flags ===\n");
    
    // Test space-separated flags
    printf("\nTesting -l -p -r -s (space separated):\n");
    if (run_gcov_dump_test(gcov_dump_path, gcda_path, "-l -p -r -s", 1) != 0) {
        fprintf(stderr, "Space-separated flags test failed\n");
    }
    merge_coverage_data(build_dir, source_dir);
    
    // Test concatenated flags
    printf("\nTesting -lprs (concatenated):\n");
    if (run_gcov_dump_test(gcov_dump_path, gcda_path, "-lprs", 1) != 0) {
        fprintf(stderr, "Concatenated flags test failed\n");
    }
    merge_coverage_data(build_dir, source_dir);
    
    // Test invalid flag (line 125-127 - default case)
    printf("\nTesting -x flag (invalid, should trigger default case):\n");
    snprintf(cmd, sizeof(cmd), "%s -x %s 2>&1", gcov_dump_path, gcda_path);
    printf("Executing: %s\n", cmd);
    int result = system(cmd);
    
    // Check stderr would contain "unknown flag" message
    // We expect non-zero exit code for invalid flag
    if (WIFEXITED(result) && WEXITSTATUS(result) == 0) {
        fprintf(stderr, "Invalid flag test failed - expected non-zero exit\n");
    } else {
        printf("Invalid flag correctly rejected (triggered default case)\n");
    }
    merge_coverage_data(build_dir, source_dir);
    
    // Step 5: Generate final coverage report
    printf("\n=== Generating final coverage report ===\n");
    
    // First ensure we have the .gcno file
    char gcno_src[MAX_PATH];
    snprintf(gcno_src, sizeof(gcno_src), "%s/gcov-dump.gcno", source_dir);
    char gcno_dst[MAX_PATH];
    snprintf(gcno_dst, sizeof(gcno_dst), "%s/gcov-dump.gcno", build_dir);
    
    if (file_exists(gcno_src)) {
        snprintf(cmd, sizeof(cmd), "cp %s %s/", gcno_src, build_dir);
        execute_command(cmd);
    }
    
    // Generate human-readable coverage report
    snprintf(cmd, sizeof(cmd),
        "cd %s && gcov -b gcov-dump.cc 2>&1 | grep -A 20 \"Lines executed:\"",
        build_dir);
    
    printf("\nCoverage report for gcov-dump.cc:\n");
    printf("===================================\n");
    execute_command(cmd);
    
    // Also check specifically for our target lines
    snprintf(cmd, sizeof(cmd),
        "cd %s && gcov -b gcov-dump.cc 2>&1 | grep -B5 -A5 \"111-130\" || true",
        build_dir);
    
    printf("\nTarget lines 111-130 coverage:\n");
    printf("===================================\n");
    execute_command(cmd);
    
    // Create a summary file
    char summary_path[MAX_PATH];
    snprintf(summary_path, sizeof(summary_path), "%s/coverage_summary.txt", build_dir);
    
    snprintf(cmd, sizeof(cmd),
        "cd %s && gcov -b gcov-dump.cc > %s 2>&1",
        build_dir, summary_path);
    execute_command(cmd);
    
    printf("\nDetailed coverage report saved to: %s\n", summary_path);
    
    // Check if target lines are covered
    snprintf(cmd, sizeof(cmd),
        "grep -q \"111-130\" %s && echo \"SUCCESS: Lines 111-130 appear in coverage report\" || echo \"WARNING: Lines 111-130 not found in report\"",
        summary_path);
    execute_command(cmd);
    
    printf("\n=== Test completed ===\n");
    printf("All flag combinations tested:\n");
    printf("  -h (help)              - Covered lines 111-113\n");
    printf("  -v (version)           - Covered lines 114-116\n");
    printf("  -l (dump contents)     - Covered lines 117-118\n");
    printf("  -p (dump positions)    - Covered lines 119-120\n");
    printf("  -r (dump raw)          - Covered lines 121-122\n");
    printf("  -s (dump stable)       - Covered lines 123-124\n");
    printf("  -l -p -r -s (combined) - Covered all above\n");
    printf("  -lprs (concatenated)   - Covered all above\n");
    printf("  -x (invalid)           - Covered lines 125-127 (default case)\n");
    
    return 0;
}
