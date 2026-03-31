#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define TEST_SOURCE "dummy.c"
#define TEST_PROGRAM "dummy_prog"
#define INSTRUMENTED_GCOV_DUMP "gcov-dump-instrumented"
#define COVERAGE_DIR "coverage_data"

// Simple test program to generate .gcda files
const char* dummy_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    int i;\n"
"    for (i = 0; i < 10; i++) {\n"
"        printf(\"Value: %d\\n\", i);\n"
"    }\n"
"    return 0;\n"
"}";

void cleanup_files() {
    // Remove generated files
    remove(TEST_SOURCE);
    remove(TEST_PROGRAM);
    remove("dummy.gcda");
    remove("dummy.gcno");
    system("rm -rf " COVERAGE_DIR);
}

int compile_instrumented_gcov_dump() {
    printf("Compiling instrumented gcov-dump...\n");
    
    // Check if we have the source file
    if (access("gcov-dump.cc", R_OK) != 0) {
        fprintf(stderr, "Error: gcov-dump.cc not found in current directory\n");
        return 0;
    }
    
    // Try to compile with coverage instrumentation
    // This assumes GCC source structure - adjust paths as needed
    char compile_cmd[1024];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage "
             "-I. -I../../include -I../../libiberty "
             "gcov-dump.cc ../../libiberty/libiberty.a "
             "-o %s 2>&1", INSTRUMENTED_GCOV_DUMP);
    
    printf("Running: %s\n", compile_cmd);
    int result = system(compile_cmd);
    
    if (result != 0 || access(INSTRUMENTED_GCOV_DUMP, X_OK) != 0) {
        fprintf(stderr, "Failed to compile instrumented gcov-dump\n");
        return 0;
    }
    
    return 1;
}

int generate_test_gcda() {
    printf("Generating test GCOV data file...\n");
    
    // Write dummy test program
    FILE* fp = fopen(TEST_SOURCE, "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return 0;
    }
    fputs(dummy_source, fp);
    fclose(fp);
    
    // Compile with coverage
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s",
             TEST_SOURCE, TEST_PROGRAM);
    
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return 0;
    }
    
    // Run to generate .gcda file
    if (system("./" TEST_PROGRAM) != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        return 0;
    }
    
    // Verify .gcda was created
    if (access("dummy.gcda", R_OK) != 0) {
        fprintf(stderr, "No dummy.gcda file generated\n");
        return 0;
    }
    
    return 1;
}

void merge_coverage_data(int run_num) {
    // Create coverage directory if it doesn't exist
    mkdir(COVERAGE_DIR, 0755);
    
    // Copy/move coverage data to preserve it
    char cmd[512];
    snprintf(cmd, sizeof(cmd), 
             "cp -f %s.gcda %s/%s_%d.gcda 2>/dev/null || true",
             INSTRUMENTED_GCOV_DUMP, COVERAGE_DIR, INSTRUMENTED_GCOV_DUMP, run_num);
    system(cmd);
    
    // Also copy the .gcno file on first run
    if (run_num == 0) {
        snprintf(cmd, sizeof(cmd),
                 "cp -f %s.gcno %s/ 2>/dev/null || true",
                 INSTRUMENTED_GCOV_DUMP, COVERAGE_DIR);
        system(cmd);
    }
}

void run_gcov_dump_test(const char* args, const char* gcda_file, int test_num) {
    printf("\nTest %d: Running gcov-dump %s %s\n", test_num, args, gcda_file ? gcda_file : "");
    
    char cmd[1024];
    if (gcda_file) {
        snprintf(cmd, sizeof(cmd), "./%s %s %s 2>&1", 
                 INSTRUMENTED_GCOV_DUMP, args, gcda_file);
    } else {
        snprintf(cmd, sizeof(cmd), "./%s %s 2>&1", 
                 INSTRUMENTED_GCOV_DUMP, args);
    }
    
    int result = system(cmd);
    printf("Exit code: %d\n", result >> 8);
    
    // Merge coverage data after each run
    merge_coverage_data(test_num);
}

void generate_final_coverage_report() {
    printf("\n=== Generating final coverage report ===\n");
    
    // First, merge all coverage data
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "cd %s && gcov -i ../%s.gcno 2>&1",
             COVERAGE_DIR, INSTRUMENTED_GCOV_DUMP);
    system(cmd);
    
    // Generate human-readable report
    snprintf(cmd, sizeof(cmd),
             "cd %s && gcov -b ../gcov-dump.cc 2>&1 | "
             "grep -A5 -B5 'Lines 111-130' || true",
             COVERAGE_DIR);
    system(cmd);
    
    // Also show summary
    printf("\n=== Coverage summary ===\n");
    snprintf(cmd, sizeof(cmd),
             "cd %s && gcov ../gcov-dump.cc 2>&1 | "
             "tail -20",
             COVERAGE_DIR);
    system(cmd);
}

int main() {
    printf("=== GCOV-Dump Coverage Test ===\n");
    
    // Clean up any previous test files
    cleanup_files();
    
    // Step 1: Build instrumented gcov-dump
    if (!compile_instrumented_gcov_dump()) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    // Step 2: Generate test GCOV data file
    if (!generate_test_gcda()) {
        fprintf(stderr, "Failed to generate test GCOV data\n");
        return 1;
    }
    
    // Step 3: Run comprehensive flag tests
    int test_num = 0;
    
    // Test help flag (-h)
    run_gcov_dump_test("-h", NULL, test_num++);
    
    // Test version flag (-v)
    run_gcov_dump_test("-v", NULL, test_num++);
    
    // Test individual flags with GCOV data file
    run_gcov_dump_test("-l", "dummy.gcda", test_num++);  // flag_dump_contents
    run_gcov_dump_test("-p", "dummy.gcda", test_num++);  // flag_dump_positions
    run_gcov_dump_test("-r", "dummy.gcda", test_num++);  // flag_dump_raw
    run_gcov_dump_test("-s", "dummy.gcda", test_num++);  // flag_dump_stable
    
    // Test combined flags (space-separated)
    run_gcov_dump_test("-l -p -r -s", "dummy.gcda", test_num++);
    
    // Test concatenated flags
    run_gcov_dump_test("-lprs", "dummy.gcda", test_num++);
    
    // Test invalid flag to trigger default case
    run_gcov_dump_test("-x", "dummy.gcda", test_num++);
    
    // Step 4: Generate final coverage report
    generate_final_coverage_report();
    
    // Step 5: Cleanup
    printf("\n=== Test complete ===\n");
    printf("Coverage data preserved in directory: %s\n", COVERAGE_DIR);
    
    return 0;
}
