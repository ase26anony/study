/**
 * test_gcov_tool_overlap.c
 * 
 * Test driver for gcov-tool overlap command-line parsing.
 * Specifically targets the parse_overlap_options function in gcov-tool.cc
 * to cover lines 534-554.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define MAX_FILES 10
#define TEMP_DIR_PATTERN "/tmp/gcov_test_XXXXXX"

/* Test result structure */
typedef struct {
    char *command;
    int expected_exit;
    int actual_exit;
    int passed;
} test_result_t;

/* Function prototypes */
int create_temp_dir(char *template);
int compile_instrumented_program(const char *dir, const char *progname);
int generate_gcda_files(const char *dir, int count);
int run_gcov_tool(const char *command, int expected_exit);
void run_test_suite(const char *gcov_tool_path, const char *temp_dir, 
                    const char *gcda_file1, const char *gcda_file2);
void cleanup(const char *temp_dir);

int main(int argc, char *argv[]) {
    char temp_dir[256];
    char gcda_file1[512];
    char gcda_file2[512];
    
    printf("=== GCOV-TOOL Overlap Parser Test Suite ===\n\n");
    
    /* Create temporary directory for test files */
    if (create_temp_dir(temp_dir) != 0) {
        fprintf(stderr, "Failed to create temporary directory\n");
        return 1;
    }
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Compile a simple instrumented program */
    if (compile_instrumented_program(temp_dir, "test_prog") != 0) {
        cleanup(temp_dir);
        return 1;
    }
    
    /* Generate .gcda files */
    if (generate_gcda_files(temp_dir, 2) != 0) {
        cleanup(temp_dir);
        return 1;
    }
    
    /* Construct paths to .gcda files */
    snprintf(gcda_file1, sizeof(gcda_file1), "%s/test_prog.gcda", temp_dir);
    snprintf(gcda_file2, sizeof(gcda_file2), "%s/test_prog2.gcda", temp_dir);
    
    /* Determine gcov-tool path - use command line arg or default */
    const char *gcov_tool_path = (argc > 1) ? argv[1] : "gcov-tool";
    
    /* Run the test suite */
    run_test_suite(gcov_tool_path, temp_dir, gcda_file1, gcda_file2);
    
    /* Cleanup */
    cleanup(temp_dir);
    
    printf("\n=== Test Suite Complete ===\n");
    return 0;
}

int create_temp_dir(char *template) {
    char *result = mkdtemp(template);
    if (result == NULL) {
        perror("mkdtemp failed");
        return -1;
    }
    return 0;
}

int compile_instrumented_program(const char *dir, const char *progname) {
    char src_path[512];
    char exe_path[512];
    char compile_cmd[1024];
    FILE *src_file;
    
    /* Create source file path */
    snprintf(src_path, sizeof(src_path), "%s/%s.c", dir, progname);
    
    /* Create a simple C program */
    src_file = fopen(src_path, "w");
    if (!src_file) {
        perror("Failed to create source file");
        return -1;
    }
    
    fprintf(src_file, "#include <stdio.h>\n\n");
    fprintf(src_file, "int helper(int x) {\n");
    fprintf(src_file, "    return x * 2;\n");
    fprintf(src_file, "}\n\n");
    fprintf(src_file, "int main() {\n");
    fprintf(src_file, "    int result = 0;\n");
    fprintf(src_file, "    for (int i = 0; i < 10; i++) {\n");
    fprintf(src_file, "        result += helper(i);\n");
    fprintf(src_file, "    }\n");
    fprintf(src_file, "    printf(\"Result: %%d\\n\", result);\n");
    fprintf(src_file, "    return 0;\n");
    fprintf(src_file, "}\n");
    fclose(src_file);
    
    /* Create executable path */
    snprintf(exe_path, sizeof(exe_path), "%s/%s", dir, progname);
    
    /* Compile with GCOV instrumentation */
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s",
             exe_path, src_path);
    
    printf("Compiling: %s\n", compile_cmd);
    int ret = system(compile_cmd);
    if (ret != 0) {
        fprintf(stderr, "Compilation failed\n");
        return -1;
    }
    
    return 0;
}

int generate_gcda_files(const char *dir, int count) {
    char exe_path[512];
    char cmd[1024];
    
    snprintf(exe_path, sizeof(exe_path), "%s/test_prog", dir);
    
    /* Run the program to generate first .gcda file */
    printf("Running program to generate .gcda files...\n");
    
    /* First run */
    snprintf(cmd, sizeof(cmd), "%s > /dev/null 2>&1", exe_path);
    if (system(cmd) != 0) {
        fprintf(stderr, "First run failed\n");
        return -1;
    }
    
    /* Copy to create second .gcda file with different name */
    char gcda_src[512];
    char gcda_dst[512];
    
    snprintf(gcda_src, sizeof(gcda_src), "%s/test_prog.gcda", dir);
    snprintf(gcda_dst, sizeof(gcda_dst), "%s/test_prog2.gcda", dir);
    
    /* Use cp command to copy the file */
    snprintf(cmd, sizeof(cmd), "cp %s %s", gcda_src, gcda_dst);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to copy .gcda file\n");
        return -1;
    }
    
    printf("Generated .gcda files: %s, %s\n", gcda_src, gcda_dst);
    return 0;
}

int run_gcov_tool(const char *command, int expected_exit) {
    printf("\nRunning: %s\n", command);
    
    int status = system(command);
    int exit_code = WEXITSTATUS(status);
    
    printf("Exit code: %d (expected: %d)\n", exit_code, expected_exit);
    
    /* Check if exit code matches expectation */
    if (exit_code == expected_exit) {
        printf("✓ PASS\n");
        return 1;
    } else {
        printf("✗ FAIL\n");
        return 0;
    }
}

void run_test_suite(const char *gcov_tool_path, const char *temp_dir,
                    const char *gcda_file1, const char *gcda_file2) {
    int total_tests = 0;
    int passed_tests = 0;
    char command[MAX_CMD_LEN];
    
    printf("\n=== Testing Basic Flag Combinations ===\n");
    
    /* Test 1: All flags together (main target for coverage) */
    total_tests++;
    snprintf(command, sizeof(command),
             "%s overlap -v -f -F -o -h -t 0.75 %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    passed_tests += run_gcov_tool(command, 0);
    
    /* Test 2: Different order of flags */
    total_tests++;
    snprintf(command, sizeof(command),
             "%s overlap -t 1.0 -h -o -F -f -v %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    passed_tests += run_gcov_tool(command, 0);
    
    /* Test 3: Only -v flag (verbosity) */
    total_tests++;
    snprintf(command, sizeof(command),
             "%s overlap -v %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    passed_tests += run_gcov_tool(command, 0);
    
    /* Test 4: Only -f flag (function level) */
    total_tests++;
    snprintf(command, sizeof(command),
             "%s overlap -f %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    passed_tests += run_gcov_tool(command, 0);
    
    /* Test 5: Only -F flag (full filename) */
    total_tests++;
    snprintf(command, sizeof(command),
             "%s overlap -F %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    passed_tests += run_gcov_tool(command, 0);
    
    /* Test 6: Only -o flag (object level) */
    total_tests++;
    snprintf(command, sizeof(command),
             "%s overlap -o %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    passed_tests += run_gcov_tool(command, 0);
    
    /* Test 7: Only -h flag (hot only) */
    total_tests++;
    snprintf(command, sizeof(command),
             "%s overlap -h %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    passed_tests += run_gcov_tool(command, 0);
    
    /* Test 8: Only -t flag with threshold */
    total_tests++;
    snprintf(command, sizeof(command),
             "%s overlap -t 0.5 %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    passed_tests += run_gcov_tool(command, 0);
    
    /* Test 9: Combination without threshold */
    total_tests++;
    snprintf(command, sizeof(command),
             "%s overlap -v -f -F -o -h %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    passed_tests += run_gcov_tool(command, 0);
    
    /* Test 10: Combination with different threshold value */
    total_tests++;
    snprintf(command, sizeof(command),
             "%s overlap -v -f -t 0.25 %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    passed_tests += run_gcov_tool(command, 0);
    
    printf("\n=== Testing Edge Cases ===\n");
    
    /* Test 11: Invalid threshold (non-numeric) - should fail */
    total_tests++;
    snprintf(command, sizeof(command),
             "%s overlap -t not_a_number %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    /* Expect non-zero exit for invalid argument */
    passed_tests += run_gcov_tool(command, 1); /* 1 indicates we expect failure */
    
    /* Test 12: Missing argument for -t (last argument) */
    total_tests++;
    snprintf(command, sizeof(command),
             "%s overlap -t %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    /* gcov-tool should handle missing argument */
    passed_tests += run_gcov_tool(command, 1);
    
    /* Test 13: Unknown flag (should trigger default case) */
    total_tests++;
    snprintf(command, sizeof(command),
             "%s overlap -x %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    passed_tests += run_gcov_tool(command, 1);
    
    /* Test 14: Repeated flags */
    total_tests++;
    snprintf(command, sizeof(command),
             "%s overlap -v -v -f -f %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    passed_tests += run_gcov_tool(command, 0);
    
    /* Test 15: Flags with single file (minimum valid case) */
    total_tests++;
    snprintf(command, sizeof(command),
             "%s overlap -v %s",
             gcov_tool_path, gcda_file1);
    passed_tests += run_gcov_tool(command, 0);
    
    /* Test 16: Using absolute paths */
    total_tests++;
    char abs_path1[1024];
    char abs_path2[1024];
    realpath(gcda_file1, abs_path1);
    realpath(gcda_file2, abs_path2);
    snprintf(command, sizeof(command),
             "%s overlap -v -F %s %s",
             gcov_tool_path, abs_path1, abs_path2);
    passed_tests += run_gcov_tool(command, 0);
    
    /* Test 17: Mix of valid and invalid flags */
    total_tests++;
    snprintf(command, sizeof(command),
             "%s overlap -v -x -f %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    passed_tests += run_gcov_tool(command, 1);
    
    /* Test 18: Threshold with scientific notation */
    total_tests++;
    snprintf(command, sizeof(command),
             "%s overlap -t 1e-1 %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    passed_tests += run_gcov_tool(command, 0);
    
    /* Test 19: All flags with very high threshold */
    total_tests++;
    snprintf(command, sizeof(command),
             "%s overlap -v -f -F -o -h -t 99.999 %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    passed_tests += run_gcov_tool(command, 0);
    
    /* Test 20: Flags in different grouping styles */
    total_tests++;
    snprintf(command, sizeof(command),
             "%s overlap -vfF -o -h -t 0.3 %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    passed_tests += run_gcov_tool(command, 0);
    
    printf("\n=== Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    printf("Success rate: %.1f%%\n", 
           (total_tests > 0) ? (100.0 * passed_tests / total_tests) : 0.0);
}

void cleanup(const char *temp_dir) {
    char cmd[1024];
    
    printf("\nCleaning up temporary directory: %s\n", temp_dir);
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
    system(cmd);
}
