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
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH 1024
#define MAX_CMD 4096

/**
 * Execute a shell command and return its exit status.
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
 * Check if a file exists.
 */
int file_exists(const char *path) {
    return access(path, F_OK) == 0;
}

/**
 * Create a simple C program to generate GCOV data.
 */
void create_dummy_program(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        exit(1);
    }
    
    fprintf(fp, "/* dummy.c - Simple program to generate GCOV data */\n");
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        printf(\"Value: %%d\\n\", i);\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
}

/**
 * Build gcov-dump with coverage instrumentation.
 * Returns 1 on success, 0 on failure.
 */
int build_instrumented_gcov_dump(const char *source_dir, const char *output_path) {
    char cmd[MAX_CMD];
    
    // First check if we can find gcov-dump source
    char source_path[MAX_PATH];
    snprintf(source_path, sizeof(source_path), "%s/gcov-dump.cc", source_dir);
    
    if (!file_exists(source_path)) {
        printf("Looking for gcov-dump source in common locations...\n");
        
        // Try common locations
        const char *locations[] = {
            ".",
            "..",
            "../gcc",
            "../../gcc",
            "/usr/src/gcc",
            NULL
        };
        
        for (int i = 0; locations[i]; i++) {
            snprintf(source_path, sizeof(source_path), "%s/gcov-dump.cc", locations[i]);
            if (file_exists(source_path)) {
                source_dir = locations[i];
                break;
            }
        }
    }
    
    if (!file_exists(source_path)) {
        printf("ERROR: Could not find gcov-dump.cc source file\n");
        printf("Please specify the path to GCC source directory\n");
        return 0;
    }
    
    printf("Found gcov-dump source at: %s\n", source_path);
    
    // Try to build with coverage instrumentation
    // This is a simplified build command - adjust based on your GCC build setup
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage -I%s -I%s/../include -I%s/../../include "
        "-I%s/../libiberty -I%s/../../libiberty %s "
        "-L%s/../libiberty -L%s/../../libiberty -liberty -o %s 2>&1",
        source_dir, source_dir, source_dir, source_dir, source_dir,
        source_path, source_dir, source_dir, output_path);
    
    printf("Building instrumented gcov-dump...\n");
    int result = execute_command(cmd);
    
    if (result != 0 || !file_exists(output_path)) {
        printf("Build failed, trying alternative approach...\n");
        
        // Try simpler build if the complex one fails
        snprintf(cmd, sizeof(cmd),
            "g++ -O0 -fprofile-arcs -ftest-coverage %s -liberty -o %s 2>&1",
            source_path, output_path);
        
        result = execute_command(cmd);
    }
    
    return file_exists(output_path);
}

/**
 * Run a single gcov-dump test with given arguments.
 * Returns exit code from gcov-dump.
 */
int run_gcov_dump_test(const char *gcov_dump_path, const char *gcda_file, 
                       const char *args, int expect_success) {
    char cmd[MAX_CMD];
    int exit_code;
    
    if (gcda_file && strlen(gcda_file) > 0) {
        snprintf(cmd, sizeof(cmd), "%s %s %s 2>&1", gcov_dump_path, args, gcda_file);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, args);
    }
    
    printf("\n=== Testing: %s ===\n", args);
    exit_code = execute_command(cmd);
    
    if (expect_success) {
        if (exit_code != 0) {
            printf("WARNING: Expected success but got exit code %d\n", exit_code);
        }
    } else {
        if (exit_code == 0) {
            printf("WARNING: Expected failure but got success\n");
        }
    }
    
    return exit_code;
}

/**
 * Merge coverage data after a test run.
 */
void merge_coverage_data(const char *gcov_dump_path) {
    char cmd[MAX_CMD];
    
    // First, find the .gcda file for gcov-dump
    // It should be in the same directory as the binary
    char gcda_pattern[MAX_PATH];
    snprintf(gcda_pattern, sizeof(gcda_pattern), "%s*.gcda", gcov_dump_path);
    
    // Use gcov to process the coverage data
    snprintf(cmd, sizeof(cmd), "gcov -i %s 2>&1 | head -20", gcov_dump_path);
    printf("Merging coverage data...\n");
    execute_command(cmd);
}

/**
 * Generate final coverage report.
 */
void generate_coverage_report(const char *source_file) {
    char cmd[MAX_CMD];
    
    printf("\n=== Generating Coverage Report ===\n");
    
    // Generate human-readable coverage report
    snprintf(cmd, sizeof(cmd), "gcov -b %s 2>&1", source_file);
    execute_command(cmd);
    
    // Also show summary
    printf("\n=== Coverage Summary ===\n");
    snprintf(cmd, sizeof(cmd), "gcov %s 2>&1 | grep -A5 'Lines executed'", source_file);
    execute_command(cmd);
}

/**
 * Check if target lines are covered.
 */
void check_target_lines_coverage(const char *source_file) {
    char cmd[MAX_CMD];
    FILE *fp;
    char line[1024];
    
    printf("\n=== Checking Coverage for Target Lines (111-130) ===\n");
    
    // First, let's see the actual source lines
    snprintf(cmd, sizeof(cmd), "sed -n '111,130p' %s", source_file);
    fp = popen(cmd, "r");
    if (fp) {
        printf("Target lines in source:\n");
        int line_num = 111;
        while (fgets(line, sizeof(line), fp)) {
            printf("%4d: %s", line_num++, line);
        }
        pclose(fp);
    }
    
    // Check gcov output for these lines
    snprintf(cmd, sizeof(cmd), "gcov %s 2>&1 | grep -n ':' | grep -A20 '^111:'", source_file);
    fp = popen(cmd, "r");
    if (fp) {
        printf("\nCoverage information for target lines:\n");
        while (fgets(line, sizeof(line), fp)) {
            printf("%s", line);
        }
        pclose(fp);
    }
}

int main(int argc, char *argv[]) {
    char cwd[MAX_PATH];
    char gcov_dump_path[MAX_PATH];
    char dummy_source[MAX_PATH];
    char dummy_binary[MAX_PATH];
    char dummy_gcda[MAX_PATH];
    char source_dir[MAX_PATH] = ".";
    
    printf("=== GCOV-Dump Coverage Test Program ===\n");
    
    // Get current directory
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed");
        return 1;
    }
    
    // Parse command line arguments
    if (argc > 1) {
        strncpy(source_dir, argv[1], sizeof(source_dir) - 1);
    }
    
    // Set paths
    snprintf(gcov_dump_path, sizeof(gcov_dump_path), "%s/gcov-dump-instrumented", cwd);
    snprintf(dummy_source, sizeof(dummy_source), "%s/dummy.c", cwd);
    snprintf(dummy_binary, sizeof(dummy_binary), "%s/dummy_prog", cwd);
    snprintf(dummy_gcda, sizeof(dummy_gcda), "%s/dummy.gcda", cwd);
    
    // Step 1: Build or locate instrumented gcov-dump
    printf("\n--- Step 1: Preparing instrumented gcov-dump ---\n");
    
    if (file_exists(gcov_dump_path)) {
        printf("Found existing instrumented gcov-dump at: %s\n", gcov_dump_path);
    } else {
        printf("Building instrumented gcov-dump...\n");
        if (!build_instrumented_gcov_dump(source_dir, gcov_dump_path)) {
            printf("ERROR: Failed to build instrumented gcov-dump\n");
            printf("Please build it manually with:\n");
            printf("  g++ -O0 -fprofile-arcs -ftest-coverage gcov-dump.cc -liberty -o gcov-dump-instrumented\n");
            return 1;
        }
    }
    
    if (!file_exists(gcov_dump_path)) {
        printf("ERROR: gcov-dump binary not found at: %s\n", gcov_dump_path);
        return 1;
    }
    
    // Step 2: Generate test GCOV data
    printf("\n--- Step 2: Generating test GCOV data ---\n");
    
    // Create dummy program
    create_dummy_program(dummy_source);
    printf("Created dummy program: %s\n", dummy_source);
    
    // Compile dummy program with coverage
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s",
             dummy_source, dummy_binary);
    
    if (execute_command(cmd) != 0) {
        printf("ERROR: Failed to compile dummy program\n");
        return 1;
    }
    
    // Run dummy program to generate .gcda file
    printf("Running dummy program to generate .gcda file...\n");
    if (execute_command(dummy_binary) != 0) {
        printf("ERROR: Failed to run dummy program\n");
        return 1;
    }
    
    if (!file_exists(dummy_gcda)) {
        printf("ERROR: .gcda file not created: %s\n", dummy_gcda);
        printf("Looking for .gcda files...\n");
        execute_command("find . -name '*.gcda' -type f 2>/dev/null");
        return 1;
    }
    
    printf("Generated GCOV data file: %s\n", dummy_gcda);
    
    // Step 3: Execute comprehensive flag tests
    printf("\n--- Step 3: Executing flag coverage tests ---\n");
    
    // Clear any existing coverage data for gcov-dump
    snprintf(cmd, sizeof(cmd), "rm -f %s*.gcda %s*.gcno 2>/dev/null", 
             gcov_dump_path, gcov_dump_path);
    execute_command(cmd);
    
    // Test individual flags (cases 'h', 'v', 'l', 'p', 'r', 's')
    printf("\n>>> Testing individual flags:\n");
    
    // -h (help) - doesn't need gcda file
    run_gcov_dump_test(gcov_dump_path, NULL, "-h", 1);
    merge_coverage_data(gcov_dump_path);
    
    // -v (version) - doesn't need gcda file
    run_gcov_dump_test(gcov_dump_path, NULL, "-v", 1);
    merge_coverage_data(gcov_dump_path);
    
    // -l (dump contents) - needs gcda file
    run_gcov_dump_test(gcov_dump_path, dummy_gcda, "-l", 1);
    merge_coverage_data(gcov_dump_path);
    
    // -p (dump positions) - needs gcda file
    run_gcov_dump_test(gcov_dump_path, dummy_gcda, "-p", 1);
    merge_coverage_data(gcov_dump_path);
    
    // -r (dump raw) - needs gcda file
    run_gcov_dump_test(gcov_dump_path, dummy_gcda, "-r", 1);
    merge_coverage_data(gcov_dump_path);
    
    // -s (dump stable) - needs gcda file
    run_gcov_dump_test(gcov_dump_path, dummy_gcda, "-s", 1);
    merge_coverage_data(gcov_dump_path);
    
    // Test combined flags (space-separated)
    printf("\n>>> Testing combined flags (space-separated):\n");
    run_gcov_dump_test(gcov_dump_path, dummy_gcda, "-l -p -r -s", 1);
    merge_coverage_data(gcov_dump_path);
    
    // Test concatenated flags
    printf("\n>>> Testing concatenated flags:\n");
    run_gcov_dump_test(gcov_dump_path, dummy_gcda, "-lprs", 1);
    merge_coverage_data(gcov_dump_path);
    
    // Test invalid flag (should trigger default case)
    printf("\n>>> Testing invalid flag (should trigger default case):\n");
    run_gcov_dump_test(gcov_dump_path, dummy_gcda, "-x", 0);
    merge_coverage_data(gcov_dump_path);
    
    // Test mixed valid and invalid flags
    printf("\n>>> Testing mixed valid and invalid flags:\n");
    run_gcov_dump_test(gcov_dump_path, dummy_gcda, "-l -x -p", 0);
    merge_coverage_data(gcov_dump_path);
    
    // Step 4: Generate final coverage report
    printf("\n--- Step 4: Final coverage analysis ---\n");
    
    // Find the actual source file
    char source_file[MAX_PATH];
    snprintf(source_file, sizeof(source_file), "%s/gcov-dump.cc", source_dir);
    
    if (!file_exists(source_file)) {
        // Try to find it in current directory
        snprintf(source_file, sizeof(source_file), "gcov-dump.cc");
        if (!file_exists(source_file)) {
            printf("WARNING: Could not find gcov-dump.cc for coverage report\n");
            printf("Looking for it...\n");
            execute_command("find . -name 'gcov-dump.cc' -type f 2>/dev/null");
            // Use whatever we can find
            strcpy(source_file, "gcov-dump.cc");
        }
    }
    
    generate_coverage_report(source_file);
    check_target_lines_coverage(source_file);
    
    // Step 5: Verify coverage
    printf("\n--- Step 5: Verification ---\n");
    
    // Check if we have coverage data
    snprintf(cmd, sizeof(cmd), "ls -la *.gcda *.gcno 2>/dev/null | head -10");
    execute_command(cmd);
    
    printf("\n=== Test Complete ===\n");
    printf("Coverage data has been generated for gcov-dump.cc\n");
    printf("Check the gcov output above to verify lines 111-130 are covered.\n");
    printf("\nTo view detailed coverage:\n");
    printf("  gcov -b %s\n", source_file);
    printf("  gcov -i %s\n", source_file);
    
    return 0;
}
