/**
 * test_gcov_dump_coverage.c
 * 
 * This program systematically tests gcov-dump's command-line argument parsing
 * to achieve coverage of lines 111-130 in gcov-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024

/**
 * Compile gcov-dump with coverage instrumentation
 */
int compile_instrumented_gcov_dump(const char *source_dir, const char *output_path) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
        "cd '%s' && "
        "g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include -I../../libiberty "
        "gcov-dump.cc ../../libiberty/libiberty.a -o '%s'",
        source_dir, output_path);
    
    printf("Compiling instrumented gcov-dump: %s\n", cmd);
    return system(cmd);
}

/**
 * Create a simple test program to generate GCOV data
 */
int create_test_program(const char *dir) {
    char dummy_c_path[MAX_PATH];
    snprintf(dummy_c_path, sizeof(dummy_c_path), "%s/dummy.c", dir);
    
    FILE *f = fopen(dummy_c_path, "w");
    if (!f) {
        perror("Failed to create dummy.c");
        return -1;
    }
    
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main() {\n");
    fprintf(f, "    int i;\n");
    fprintf(f, "    for (i = 0; i < 10; i++) {\n");
    fprintf(f, "        printf(\"Test: %%d\\n\", i);\n");
    fprintf(f, "    }\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    fclose(f);
    
    return 0;
}

/**
 * Compile and run the test program to generate .gcda file
 */
int generate_gcda_file(const char *dir) {
    char cmd[1024];
    
    // Compile dummy program with coverage
    snprintf(cmd, sizeof(cmd),
        "cd '%s' && "
        "gcc -O0 -fprofile-arcs -ftest-coverage dummy.c -o dummy_prog",
        dir);
    
    printf("Compiling dummy program: %s\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return -1;
    }
    
    // Run it to generate .gcda file
    snprintf(cmd, sizeof(cmd), "cd '%s' && ./dummy_prog", dir);
    printf("Running dummy program: %s\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        return -1;
    }
    
    // Verify .gcda file was created
    char gcda_path[MAX_PATH];
    snprintf(gcda_path, sizeof(gcda_path), "%s/dummy.gcda", dir);
    struct stat st;
    if (stat(gcda_path, &st) != 0) {
        fprintf(stderr, "Failed to create dummy.gcda\n");
        return -1;
    }
    
    printf("Generated dummy.gcda (%ld bytes)\n", (long)st.st_size);
    return 0;
}

/**
 * Merge coverage data from gcov-dump runs
 */
void merge_coverage(const char *gcov_dump_path, const char *source_file) {
    char cmd[1024];
    
    // First, ensure we have the .gcno file
    snprintf(cmd, sizeof(cmd), 
        "cd '%s' && "
        "gcov -i %s > /dev/null 2>&1",
        gcov_dump_path, source_file);
    
    printf("Merging coverage data: %s\n", cmd);
    system(cmd);
}

/**
 * Run gcov-dump with specific arguments and check exit code
 */
int run_gcov_dump(const char *gcov_dump_bin, const char *gcda_file, 
                  const char *args, int expect_success) {
    char cmd[2048];
    
    if (gcda_file) {
        snprintf(cmd, sizeof(cmd), "%s %s '%s'", gcov_dump_bin, args, gcda_file);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_bin, args);
    }
    
    printf("\nRunning: %s\n", cmd);
    
    int status = system(cmd);
    int exit_code = WEXITSTATUS(status);
    
    if (expect_success) {
        if (exit_code != 0) {
            fprintf(stderr, "ERROR: Expected success but got exit code %d\n", exit_code);
            return -1;
        }
    } else {
        if (exit_code == 0) {
            fprintf(stderr, "ERROR: Expected failure but got success\n");
            return -1;
        }
    }
    
    return 0;
}

/**
 * Generate final coverage report
 */
void generate_coverage_report(const char *gcov_dump_path, const char *source_file) {
    char cmd[1024];
    
    printf("\n=== Generating coverage report for %s ===\n", source_file);
    
    snprintf(cmd, sizeof(cmd),
        "cd '%s' && "
        "gcov -b %s",
        gcov_dump_path, source_file);
    
    system(cmd);
    
    // Also show the specific lines we care about
    printf("\n=== Checking coverage for target lines 111-130 ===\n");
    snprintf(cmd, sizeof(cmd),
        "cd '%s' && "
        "gcov -l %s | grep -A 20 '^111:'",
        gcov_dump_path, source_file);
    
    system(cmd);
}

int main(int argc, char *argv[]) {
    char work_dir[MAX_PATH];
    char gcov_dump_bin[MAX_PATH];
    char gcda_file[MAX_PATH];
    char gcov_dump_source_dir[MAX_PATH];
    
    // Get current directory
    if (getcwd(work_dir, sizeof(work_dir)) == NULL) {
        perror("getcwd failed");
        return 1;
    }
    
    // Determine paths
    snprintf(gcov_dump_bin, sizeof(gcov_dump_bin), "%s/gcov-dump-instrumented", work_dir);
    snprintf(gcda_file, sizeof(gcda_file), "%s/dummy.gcda", work_dir);
    
    // Assume gcov-dump source is in parent directory for this example
    // Adjust as needed for your environment
    snprintf(gcov_dump_source_dir, sizeof(gcov_dump_source_dir), "%s/..", work_dir);
    
    printf("Work directory: %s\n", work_dir);
    printf("GCOV dump binary: %s\n", gcov_dump_bin);
    printf("GCOV dump source dir: %s\n", gcov_dump_source_dir);
    
    // Step 1: Compile instrumented gcov-dump if needed
    struct stat st;
    if (stat(gcov_dump_bin, &st) != 0) {
        printf("Instrumented gcov-dump not found, compiling...\n");
        if (compile_instrumented_gcov_dump(gcov_dump_source_dir, gcov_dump_bin) != 0) {
            fprintf(stderr, "Failed to compile instrumented gcov-dump\n");
            return 1;
        }
    } else {
        printf("Using existing instrumented gcov-dump\n");
    }
    
    // Step 2: Create test program and generate .gcda file
    printf("\n=== Generating test GCOV data ===\n");
    if (create_test_program(work_dir) != 0) {
        return 1;
    }
    
    if (generate_gcda_file(work_dir) != 0) {
        return 1;
    }
    
    // Step 3: Execute comprehensive flag testing
    printf("\n=== Testing gcov-dump flag combinations ===\n");
    
    // Test individual flags (lines 111-130)
    printf("\n--- Testing individual flags ---\n");
    
    // -h (help) - no gcda file needed
    run_gcov_dump(gcov_dump_bin, NULL, "-h", 1);
    merge_coverage(work_dir, "gcov-dump.cc");
    
    // -v (version) - no gcda file needed
    run_gcov_dump(gcov_dump_bin, NULL, "-v", 1);
    merge_coverage(work_dir, "gcov-dump.cc");
    
    // -l (dump contents)
    run_gcov_dump(gcov_dump_bin, gcda_file, "-l", 1);
    merge_coverage(work_dir, "gcov-dump.cc");
    
    // -p (dump positions)
    run_gcov_dump(gcov_dump_bin, gcda_file, "-p", 1);
    merge_coverage(work_dir, "gcov-dump.cc");
    
    // -r (dump raw)
    run_gcov_dump(gcov_dump_bin, gcda_file, "-r", 1);
    merge_coverage(work_dir, "gcov-dump.cc");
    
    // -s (dump stable)
    run_gcov_dump(gcov_dump_bin, gcda_file, "-s", 1);
    merge_coverage(work_dir, "gcov-dump.cc");
    
    // Test combined flags (space-separated)
    printf("\n--- Testing combined flags (space-separated) ---\n");
    run_gcov_dump(gcov_dump_bin, gcda_file, "-l -p -r -s", 1);
    merge_coverage(work_dir, "gcov-dump.cc");
    
    // Test concatenated flags
    printf("\n--- Testing concatenated flags ---\n");
    run_gcov_dump(gcov_dump_bin, gcda_file, "-lprs", 1);
    merge_coverage(work_dir, "gcov-dump.cc");
    
    // Test invalid flag (should trigger default case)
    printf("\n--- Testing invalid flag (should trigger default case) ---\n");
    run_gcov_dump(gcov_dump_bin, gcda_file, "-x", 0);
    merge_coverage(work_dir, "gcov-dump.cc");
    
    // Test mixed valid and invalid flags
    printf("\n--- Testing mixed valid and invalid flags ---\n");
    run_gcov_dump(gcov_dump_bin, gcda_file, "-lp -x", 0);
    merge_coverage(work_dir, "gcov-dump.cc");
    
    // Step 4: Generate final coverage report
    printf("\n=== Final coverage analysis ===\n");
    generate_coverage_report(work_dir, "gcov-dump.cc");
    
    // Cleanup
    printf("\n=== Cleaning up ===\n");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "cd '%s' && rm -f dummy.c dummy_prog *.gcda *.gcno", work_dir);
    system(cmd);
    
    printf("\n=== Test completed ===\n");
    printf("Coverage data for gcov-dump.cc should now show lines 111-130 as executed.\n");
    printf("Check gcov-dump.cc.gcov for detailed coverage information.\n");
    
    return 0;
}
