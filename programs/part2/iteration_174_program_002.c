#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024

// Function to check if a file exists
int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

// Function to execute a command and check its return code
int execute_command(const char *cmd, int expect_success) {
    printf("Executing: %s\n", cmd);
    int ret = system(cmd);
    if (expect_success && ret != 0) {
        fprintf(stderr, "Command failed with code %d: %s\n", ret, cmd);
        return 0;
    }
    if (!expect_success && ret == 0) {
        fprintf(stderr, "Command should have failed but succeeded: %s\n", cmd);
        return 0;
    }
    return 1;
}

// Function to compile gcov-dump with coverage instrumentation
int compile_gcov_dump(const char *source_dir, const char *build_dir) {
    char cmd[MAX_PATH * 4];
    
    // First, check if we're in a GCC source tree
    char gcov_dump_source[MAX_PATH];
    snprintf(gcov_dump_source, sizeof(gcov_dump_source), "%s/gcov-dump.cc", source_dir);
    
    if (!file_exists(gcov_dump_source)) {
        fprintf(stderr, "Could not find gcov-dump.cc at %s\n", gcov_dump_source);
        return 0;
    }
    
    // Create build directory if it doesn't exist
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", build_dir);
    system(cmd);
    
    // Compile gcov-dump with coverage instrumentation
    // Note: This assumes a typical GCC source tree structure
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-I%s -I%s/../../include -I%s/../../libiberty "
        "%s "
        "%s/../../libiberty/libiberty.a "
        "-o %s/gcov-dump-instrumented",
        source_dir, source_dir, source_dir,
        gcov_dump_source,
        source_dir,
        build_dir);
    
    return execute_command(cmd, 1);
}

// Function to create and compile a dummy program for generating .gcda files
int create_dummy_program(const char *build_dir) {
    char dummy_c[MAX_PATH];
    snprintf(dummy_c, sizeof(dummy_c), "%s/dummy.c", build_dir);
    
    // Create a simple C program
    FILE *fp = fopen(dummy_c, "w");
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
    
    // Compile it with coverage
    char cmd[MAX_PATH * 2];
    snprintf(cmd, sizeof(cmd),
        "gcc -O0 -fprofile-arcs -ftest-coverage %s/dummy.c -o %s/dummy_prog",
        build_dir, build_dir);
    
    if (!execute_command(cmd, 1)) {
        return 0;
    }
    
    // Run it to generate .gcda file
    snprintf(cmd, sizeof(cmd), "cd %s && ./dummy_prog > /dev/null", build_dir);
    return execute_command(cmd, 1);
}

// Function to merge coverage data after each gcov-dump invocation
void merge_coverage(const char *build_dir, const char *source_dir) {
    char cmd[MAX_PATH * 4];
    
    // Method 1: Use gcov -i to merge intermediate files
    snprintf(cmd, sizeof(cmd),
        "cd %s && gcov -i gcov-dump-instrumented 2>/dev/null || true",
        build_dir);
    system(cmd);
    
    // Method 2: Copy .gcda files to a known location
    // This handles cases where gcov-dump creates .gcda files in different directories
    snprintf(cmd, sizeof(cmd),
        "find %s -name '*.gcda' -exec cp {} %s/ 2>/dev/null \\; || true",
        source_dir, build_dir);
    system(cmd);
}

// Function to run all test cases
int run_test_cases(const char *build_dir) {
    char gcov_dump_path[MAX_PATH];
    char dummy_gcda[MAX_PATH];
    char cmd[MAX_PATH * 4];
    
    snprintf(gcov_dump_path, sizeof(gcov_dump_path), "%s/gcov-dump-instrumented", build_dir);
    snprintf(dummy_gcda, sizeof(dummy_gcda), "%s/dummy.gcda", build_dir);
    
    int all_passed = 1;
    
    // Test 1: Help flag (-h)
    printf("\n=== Test 1: Help flag (-h) ===\n");
    snprintf(cmd, sizeof(cmd), "%s -h", gcov_dump_path);
    all_passed &= execute_command(cmd, 1);
    merge_coverage(build_dir, ".");
    
    // Test 2: Version flag (-v)
    printf("\n=== Test 2: Version flag (-v) ===\n");
    snprintf(cmd, sizeof(cmd), "%s -v", gcov_dump_path);
    all_passed &= execute_command(cmd, 1);
    merge_coverage(build_dir, ".");
    
    // Test 3: Dump contents flag (-l)
    printf("\n=== Test 3: Dump contents flag (-l) ===\n");
    snprintf(cmd, sizeof(cmd), "%s -l %s", gcov_dump_path, dummy_gcda);
    all_passed &= execute_command(cmd, 1);
    merge_coverage(build_dir, ".");
    
    // Test 4: Dump positions flag (-p)
    printf("\n=== Test 4: Dump positions flag (-p) ===\n");
    snprintf(cmd, sizeof(cmd), "%s -p %s", gcov_dump_path, dummy_gcda);
    all_passed &= execute_command(cmd, 1);
    merge_coverage(build_dir, ".");
    
    // Test 5: Dump raw flag (-r)
    printf("\n=== Test 5: Dump raw flag (-r) ===\n");
    snprintf(cmd, sizeof(cmd), "%s -r %s", gcov_dump_path, dummy_gcda);
    all_passed &= execute_command(cmd, 1);
    merge_coverage(build_dir, ".");
    
    // Test 6: Dump stable flag (-s)
    printf("\n=== Test 6: Dump stable flag (-s) ===\n");
    snprintf(cmd, sizeof(cmd), "%s -s %s", gcov_dump_path, dummy_gcda);
    all_passed &= execute_command(cmd, 1);
    merge_coverage(build_dir, ".");
    
    // Test 7: Combined flags (space-separated)
    printf("\n=== Test 7: Combined flags (space-separated) ===\n");
    snprintf(cmd, sizeof(cmd), "%s -l -p -r -s %s", gcov_dump_path, dummy_gcda);
    all_passed &= execute_command(cmd, 1);
    merge_coverage(build_dir, ".");
    
    // Test 8: Combined flags (concatenated)
    printf("\n=== Test 8: Combined flags (concatenated) ===\n");
    snprintf(cmd, sizeof(cmd), "%s -lprs %s", gcov_dump_path, dummy_gcda);
    all_passed &= execute_command(cmd, 1);
    merge_coverage(build_dir, ".");
    
    // Test 9: Invalid flag (should trigger default case)
    printf("\n=== Test 9: Invalid flag (should trigger default case) ===\n");
    snprintf(cmd, sizeof(cmd), "%s -x %s 2>&1 | grep -q 'unknown flag'", 
             gcov_dump_path, dummy_gcda);
    all_passed &= execute_command(cmd, 1);
    merge_coverage(build_dir, ".");
    
    return all_passed;
}

// Function to generate and check final coverage report
void check_coverage(const char *build_dir, const char *source_dir) {
    char cmd[MAX_PATH * 4];
    char gcov_file[MAX_PATH];
    
    // First, copy the source file to build directory for gcov
    snprintf(cmd, sizeof(cmd), "cp %s/gcov-dump.cc %s/", source_dir, build_dir);
    system(cmd);
    
    // Generate coverage report
    printf("\n=== Generating coverage report ===\n");
    snprintf(cmd, sizeof(cmd), 
        "cd %s && gcov -b gcov-dump-instrumented.gcda gcov-dump.cc 2>&1",
        build_dir);
    system(cmd);
    
    // Check if coverage file was generated
    snprintf(gcov_file, sizeof(gcov_file), "%s/gcov-dump.cc.gcov", build_dir);
    if (file_exists(gcov_file)) {
        printf("\n=== Coverage report summary ===\n");
        // Extract and display coverage for lines 111-130
        snprintf(cmd, sizeof(cmd),
            "awk 'NR >= 111 && NR <= 130 {printf \"Line %%d: %%s\\n\", NR, $0}' %s",
            gcov_file);
        system(cmd);
        
        // Check specifically for the switch case lines
        printf("\n=== Checking target switch-case coverage ===\n");
        snprintf(cmd, sizeof(cmd),
            "grep -n -E \"case '[hlprsv]'|default:\" %s | head -20",
            gcov_file);
        system(cmd);
    } else {
        printf("Warning: No .gcov file generated\n");
    }
}

int main(int argc, char *argv[]) {
    const char *source_dir = ".";
    const char *build_dir = "./gcov_dump_test_build";
    
    printf("=== Starting gcov-dump coverage test ===\n");
    
    // Step 1: Compile instrumented gcov-dump
    printf("\nStep 1: Compiling instrumented gcov-dump...\n");
    if (!compile_gcov_dump(source_dir, build_dir)) {
        fprintf(stderr, "Failed to compile gcov-dump\n");
        return 1;
    }
    
    // Step 2: Create dummy program for generating .gcda files
    printf("\nStep 2: Creating dummy program for test data...\n");
    if (!create_dummy_program(build_dir)) {
        fprintf(stderr, "Failed to create dummy program\n");
        return 1;
    }
    
    // Step 3: Run all test cases
    printf("\nStep 3: Running test cases...\n");
    if (!run_test_cases(build_dir)) {
        fprintf(stderr, "Some test cases failed\n");
        // Continue anyway to check coverage
    }
    
    // Step 4: Check coverage
    printf("\nStep 4: Checking coverage results...\n");
    check_coverage(build_dir, source_dir);
    
    printf("\n=== Test complete ===\n");
    printf("Coverage data available in: %s\n", build_dir);
    printf("Check gcov-dump.cc.gcov for detailed coverage information\n");
    
    return 0;
}
