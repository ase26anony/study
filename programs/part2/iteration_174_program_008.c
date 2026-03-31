/**
 * test_gcov_dump_coverage.c
 * 
 * This program tests the uncovered lines in gcov-dump.cc (lines 111-130)
 * by building an instrumented version of gcov-dump and executing it
 * with various command-line flag combinations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_PATH 1024
#define MAX_CMD 4096

/**
 * Creates a simple C program that will be used to generate GCOV data
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
    fprintf(fp, "        printf(\"Iteration %%d\\n\", i);\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    printf("Created dummy program: %s\n", filename);
}

/**
 * Builds an instrumented version of gcov-dump
 * Assumes gcov-dump.cc and necessary headers are available
 */
int build_instrumented_gcov_dump(const char *source_dir, const char *output_path) {
    char cmd[MAX_CMD];
    int result;
    
    printf("Building instrumented gcov-dump...\n");
    
    // Try to find gcov-dump source
    char gcov_dump_source[MAX_PATH];
    snprintf(gcov_dump_source, sizeof(gcov_dump_source), "%s/gcov-dump.cc", source_dir);
    
    struct stat st;
    if (stat(gcov_dump_source, &st) != 0) {
        // Try alternative location
        snprintf(gcov_dump_source, sizeof(gcov_dump_source), "%s/../gcc/gcov-dump.cc", source_dir);
        if (stat(gcov_dump_source, &st) != 0) {
            fprintf(stderr, "Could not find gcov-dump.cc in %s\n", source_dir);
            return 0;
        }
    }
    
    // Build command for instrumented gcov-dump
    // This assumes GCC source tree structure
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-I%s -I%s/../../include -I%s/../../libiberty "
        "%s %s/../../libiberty/libiberty.a "
        "-o %s",
        source_dir, source_dir, source_dir,
        gcov_dump_source, source_dir,
        output_path);
    
    printf("Compilation command: %s\n", cmd);
    result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 0;
    }
    
    printf("Successfully built instrumented gcov-dump: %s\n", output_path);
    return 1;
}

/**
 * Runs a command and checks its exit status
 */
int run_command(const char *cmd, int expect_success) {
    printf("Running: %s\n", cmd);
    int result = system(cmd);
    
    if (expect_success && result != 0) {
        fprintf(stderr, "Command failed with exit code %d: %s\n", result, cmd);
        return 0;
    }
    
    if (!expect_success && result == 0) {
        fprintf(stderr, "Command succeeded unexpectedly: %s\n", cmd);
        return 0;
    }
    
    return 1;
}

/**
 * Merges coverage data for gcov-dump.cc
 */
void merge_coverage(const char *gcov_dump_source, const char *build_dir) {
    char cmd[MAX_CMD];
    
    // Change to build directory to find .gcda files
    char old_cwd[MAX_PATH];
    getcwd(old_cwd, sizeof(old_cwd));
    
    if (chdir(build_dir) == 0) {
        // Use gcov to generate intermediate coverage data
        snprintf(cmd, sizeof(cmd), "gcov -i %s > /dev/null 2>&1", gcov_dump_source);
        system(cmd);
        
        // Return to original directory
        chdir(old_cwd);
    }
}

/**
 * Main test function
 */
int main(int argc, char *argv[]) {
    char cwd[MAX_PATH];
    char build_dir[MAX_PATH];
    char dummy_source[MAX_PATH];
    char dummy_binary[MAX_PATH];
    char gcov_dump_instrumented[MAX_PATH];
    char gcov_data_file[MAX_PATH];
    
    // Get current directory
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed");
        return 1;
    }
    
    // Create build directory
    snprintf(build_dir, sizeof(build_dir), "%s/gcov_test_build", cwd);
    mkdir(build_dir, 0755);
    
    // Create paths
    snprintf(dummy_source, sizeof(dummy_source), "%s/dummy.c", build_dir);
    snprintf(dummy_binary, sizeof(dummy_binary), "%s/dummy_prog", build_dir);
    snprintf(gcov_dump_instrumented, sizeof(gcov_dump_instrumented), 
             "%s/gcov-dump-instrumented", build_dir);
    snprintf(gcov_data_file, sizeof(gcov_data_file), 
             "%s/dummy.gcda", build_dir);
    
    // Step 1: Create dummy program
    printf("=== Step 1: Creating dummy program ===\n");
    create_dummy_program(dummy_source);
    
    // Step 2: Build instrumented gcov-dump
    printf("\n=== Step 2: Building instrumented gcov-dump ===\n");
    if (!build_instrumented_gcov_dump(cwd, gcov_dump_instrumented)) {
        // Try alternative: use existing gcov-dump if available
        printf("Trying to use existing gcov-dump...\n");
        snprintf(gcov_dump_instrumented, sizeof(gcov_dump_instrumented), "gcov-dump");
    }
    
    // Step 3: Generate test GCOV data
    printf("\n=== Step 3: Generating test GCOV data ===\n");
    char cmd[MAX_CMD];
    
    // Compile dummy program with coverage
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s",
             dummy_source, dummy_binary);
    if (!run_command(cmd, 1)) {
        return 1;
    }
    
    // Run dummy program to generate .gcda file
    snprintf(cmd, sizeof(cmd), "%s", dummy_binary);
    if (!run_command(cmd, 1)) {
        return 1;
    }
    
    printf("Generated GCOV data file: %s\n", gcov_data_file);
    
    // Step 4: Execute flag coverage series
    printf("\n=== Step 4: Testing flag combinations ===\n");
    
    // Test individual flags
    printf("\n--- Testing individual flags ---\n");
    
    // -h (help) - doesn't need data file
    snprintf(cmd, sizeof(cmd), "%s -h 2>&1 | head -5", gcov_dump_instrumented);
    run_command(cmd, 1);
    merge_coverage("gcov-dump.cc", build_dir);
    
    // -v (version) - doesn't need data file
    snprintf(cmd, sizeof(cmd), "%s -v", gcov_dump_instrumented);
    run_command(cmd, 1);
    merge_coverage("gcov-dump.cc", build_dir);
    
    // -l (dump contents) with data file
    snprintf(cmd, sizeof(cmd), "%s -l %s", gcov_dump_instrumented, gcov_data_file);
    run_command(cmd, 1);
    merge_coverage("gcov-dump.cc", build_dir);
    
    // -p (dump positions) with data file
    snprintf(cmd, sizeof(cmd), "%s -p %s", gcov_dump_instrumented, gcov_data_file);
    run_command(cmd, 1);
    merge_coverage("gcov-dump.cc", build_dir);
    
    // -r (dump raw) with data file
    snprintf(cmd, sizeof(cmd), "%s -r %s", gcov_dump_instrumented, gcov_data_file);
    run_command(cmd, 1);
    merge_coverage("gcov-dump.cc", build_dir);
    
    // -s (dump stable) with data file
    snprintf(cmd, sizeof(cmd), "%s -s %s", gcov_dump_instrumented, gcov_data_file);
    run_command(cmd, 1);
    merge_coverage("gcov-dump.cc", build_dir);
    
    // Test combined flags (space-separated)
    printf("\n--- Testing combined flags (space-separated) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -l -p -r -s %s", 
             gcov_dump_instrumented, gcov_data_file);
    run_command(cmd, 1);
    merge_coverage("gcov-dump.cc", build_dir);
    
    // Test concatenated flags
    printf("\n--- Testing concatenated flags ---\n");
    snprintf(cmd, sizeof(cmd), "%s -lprs %s", 
             gcov_dump_instrumented, gcov_data_file);
    run_command(cmd, 1);
    merge_coverage("gcov-dump.cc", build_dir);
    
    // Test invalid flag (should trigger default case)
    printf("\n--- Testing invalid flag (should trigger default case) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -x %s 2>&1", 
             gcov_dump_instrumented, gcov_data_file);
    // This should fail, so expect_success = 0
    run_command(cmd, 0);
    merge_coverage("gcov-dump.cc", build_dir);
    
    // Step 5: Generate final coverage report
    printf("\n=== Step 5: Generating coverage report ===\n");
    
    // Change to build directory to find coverage files
    char old_cwd[MAX_PATH];
    getcwd(old_cwd, sizeof(old_cwd));
    
    if (chdir(build_dir) == 0) {
        // Generate human-readable coverage report
        printf("\nCoverage report for gcov-dump.cc:\n");
        snprintf(cmd, sizeof(cmd), "gcov -b gcov-dump.cc 2>&1 | grep -A 30 'Lines executed:'");
        system(cmd);
        
        // Specifically check for our target lines
        printf("\n--- Checking target lines (111-130) ---\n");
        snprintf(cmd, sizeof(cmd), 
                 "gcov -b gcov-dump.cc 2>&1 | "
                 "sed -n '/^ *111:/,/^ *131:/p' | "
                 "grep -E '^ *[0-9]+:|^ *[#|-]'");
        system(cmd);
        
        // Return to original directory
        chdir(old_cwd);
    }
    
    // Cleanup
    printf("\n=== Cleanup ===\n");
    printf("Test files are in: %s\n", build_dir);
    printf("To remove them, run: rm -rf %s\n", build_dir);
    
    printf("\n=== Test completed ===\n");
    printf("The target lines in gcov-dump.cc (111-130) should now be covered.\n");
    printf("Check the coverage output above to verify execution counts.\n");
    
    return 0;
}
