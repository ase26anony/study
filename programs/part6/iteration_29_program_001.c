/**
 * test_gcov_tool_overlap.c
 * 
 * Test driver for gcov-tool overlap command parsing.
 * Exercises the uncovered switch cases in parse_overlap_options()
 * in gcov-tool.cc lines 534-554.
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
#define TEMP_DIR "/tmp/gcov_test_XXXXXX"

/* Global variables to track test results */
typedef struct {
    const char *description;
    const char *command;
    int expected_exit_code;
    int actual_exit_code;
    int executed;
} test_case_t;

/* Function prototypes */
int create_temp_dir(char *template);
int compile_instrumented_program(const char *dir);
int generate_gcda_files(const char *dir, int count);
int run_gcov_tool(const char *command);
void run_test_suite(const char *gcov_tool_path, const char *gcda_file1, const char *gcda_file2);
void print_test_summary(test_case_t *tests, int num_tests);

int main(int argc, char *argv[]) {
    char temp_dir[256];
    char gcda_file1[512];
    char gcda_file2[512];
    char gcov_tool_path[512] = "./gcov-tool";
    
    /* Allow overriding gcov-tool path via environment variable */
    const char *env_path = getenv("GCOV_TOOL_PATH");
    if (env_path != NULL) {
        strncpy(gcov_tool_path, env_path, sizeof(gcov_tool_path) - 1);
        gcov_tool_path[sizeof(gcov_tool_path) - 1] = '\0';
    }
    
    printf("=== GCOV-TOOL Overlap Parser Test ===\n");
    printf("Testing gcov-tool at: %s\n", gcov_tool_path);
    
    /* Create temporary directory for test files */
    if (create_temp_dir(temp_dir) != 0) {
        fprintf(stderr, "Failed to create temporary directory\n");
        return 1;
    }
    
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Compile a simple instrumented program */
    if (compile_instrumented_program(temp_dir) != 0) {
        fprintf(stderr, "Failed to compile instrumented program\n");
        return 1;
    }
    
    /* Generate multiple .gcda files for overlap analysis */
    if (generate_gcda_files(temp_dir, 2) != 0) {
        fprintf(stderr, "Failed to generate .gcda files\n");
        return 1;
    }
    
    /* Construct paths to generated .gcda files */
    snprintf(gcda_file1, sizeof(gcda_file1), "%s/test_prog.gcda", temp_dir);
    snprintf(gcda_file2, sizeof(gcda_file2), "%s/test_prog2.gcda", temp_dir);
    
    /* Run the test suite */
    run_test_suite(gcov_tool_path, gcda_file1, gcda_file2);
    
    /* Cleanup */
    printf("\nCleaning up temporary directory: %s\n", temp_dir);
    char cleanup_cmd[512];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
    system(cleanup_cmd);
    
    return 0;
}

/**
 * Create a temporary directory for test files
 */
int create_temp_dir(char *template) {
    char *result = mkdtemp(template);
    if (result == NULL) {
        perror("mkdtemp failed");
        return -1;
    }
    return 0;
}

/**
 * Compile a simple C program with GCOV instrumentation
 */
int compile_instrumented_program(const char *dir) {
    char source_path[512];
    char exec_path[512];
    char compile_cmd[1024];
    
    /* Create source file */
    snprintf(source_path, sizeof(source_path), "%s/test_prog.c", dir);
    FILE *src = fopen(source_path, "w");
    if (src == NULL) {
        perror("Failed to create source file");
        return -1;
    }
    
    /* Write a simple instrumented program */
    fprintf(src, "#include <stdio.h>\n");
    fprintf(src, "#include <stdlib.h>\n\n");
    fprintf(src, "void func1() {\n");
    fprintf(src, "    printf(\"func1 called\\n\");\n");
    fprintf(src, "}\n\n");
    fprintf(src, "void func2(int x) {\n");
    fprintf(src, "    if (x > 0) {\n");
    fprintf(src, "        printf(\"x is positive\\n\");\n");
    fprintf(src, "    } else {\n");
    fprintf(src, "        printf(\"x is non-positive\\n\");\n");
    fprintf(src, "    }\n");
    fprintf(src, "}\n\n");
    fprintf(src, "int main(int argc, char *argv[]) {\n");
    fprintf(src, "    printf(\"Test program running\\n\");\n");
    fprintf(src, "    func1();\n");
    fprintf(src, "    func2(argc);\n");
    fprintf(src, "    return 0;\n");
    fprintf(src, "}\n");
    fclose(src);
    
    /* Compile with GCOV instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", dir);
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s",
             exec_path, source_path);
    
    printf("Compiling: %s\n", compile_cmd);
    int ret = system(compile_cmd);
    if (ret != 0) {
        fprintf(stderr, "Compilation failed\n");
        return -1;
    }
    
    return 0;
}

/**
 * Generate .gcda files by running the instrumented program
 */
int generate_gcda_files(const char *dir, int count) {
    char exec_path[512];
    char gcda_copy_cmd[1024];
    
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", dir);
    
    /* Run the program to generate initial .gcda file */
    printf("Running instrumented program to generate .gcda files\n");
    
    /* First run - creates test_prog.gcda */
    char run_cmd[512];
    snprintf(run_cmd, sizeof(run_cmd), "%s > /dev/null 2>&1", exec_path);
    system(run_cmd);
    
    /* Create multiple copies with different names for overlap analysis */
    for (int i = 2; i <= count; i++) {
        snprintf(gcda_copy_cmd, sizeof(gcda_copy_cmd),
                 "cp %s/test_prog.gcda %s/test_prog%d.gcda",
                 dir, dir, i);
        system(gcda_copy_cmd);
    }
    
    /* Verify files were created */
    struct stat st;
    char gcda_path[512];
    snprintf(gcda_path, sizeof(gcda_path), "%s/test_prog.gcda", dir);
    
    if (stat(gcda_path, &st) != 0) {
        fprintf(stderr, "Failed to create .gcda file\n");
        return -1;
    }
    
    printf("Generated %d .gcda files\n", count);
    return 0;
}

/**
 * Execute gcov-tool with given command and return exit code
 */
int run_gcov_tool(const char *command) {
    printf("Executing: %s\n", command);
    
    int status = system(command);
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    } else {
        return -1;  /* Command didn't exit normally */
    }
}

/**
 * Run comprehensive test suite for overlap command parsing
 */
void run_test_suite(const char *gcov_tool_path, const char *gcda_file1, const char *gcda_file2) {
    test_case_t tests[50];
    int test_count = 0;
    char cmd[MAX_CMD_LEN];
    
    printf("\n=== Running Overlap Parser Tests ===\n");
    
    /* Test 1: Basic overlap command with all uncovered flags */
    tests[test_count].description = "All uncovered flags combined";
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f -F -o -h -t 0.75 %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    tests[test_count].command = strdup(cmd);
    tests[test_count].expected_exit_code = 0;
    test_count++;
    
    /* Test 2: Different order of flags */
    tests[test_count].description = "Different flag order";
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.5 -h -o -F -f -v %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    tests[test_count].command = strdup(cmd);
    tests[test_count].expected_exit_code = 0;
    test_count++;
    
    /* Test 3: Only -v flag (verbose) */
    tests[test_count].description = "Only -v flag";
    snprintf(cmd, sizeof(cmd), "%s overlap -v %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    tests[test_count].command = strdup(cmd);
    tests[test_count].expected_exit_code = 0;
    test_count++;
    
    /* Test 4: Only -f flag (function level) */
    tests[test_count].description = "Only -f flag";
    snprintf(cmd, sizeof(cmd), "%s overlap -f %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    tests[test_count].command = strdup(cmd);
    tests[test_count].expected_exit_code = 0;
    test_count++;
    
    /* Test 5: Only -F flag (full filename) */
    tests[test_count].description = "Only -F flag";
    snprintf(cmd, sizeof(cmd), "%s overlap -F %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    tests[test_count].command = strdup(cmd);
    tests[test_count].expected_exit_code = 0;
    test_count++;
    
    /* Test 6: Only -o flag (object level) */
    tests[test_count].description = "Only -o flag";
    snprintf(cmd, sizeof(cmd), "%s overlap -o %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    tests[test_count].command = strdup(cmd);
    tests[test_count].expected_exit_code = 0;
    test_count++;
    
    /* Test 7: Only -h flag (hot only) */
    tests[test_count].description = "Only -h flag";
    snprintf(cmd, sizeof(cmd), "%s overlap -h %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    tests[test_count].command = strdup(cmd);
    tests[test_count].expected_exit_code = 0;
    test_count++;
    
    /* Test 8: Only -t flag with threshold */
    tests[test_count].description = "Only -t flag with threshold";
    snprintf(cmd, sizeof(cmd), "%s overlap -t 1.0 %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    tests[test_count].command = strdup(cmd);
    tests[test_count].expected_exit_code = 0;
    test_count++;
    
    /* Test 9: -t with different threshold values */
    tests[test_count].description = "-t with threshold 0.25";
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.25 %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    tests[test_count].command = strdup(cmd);
    tests[test_count].expected_exit_code = 0;
    test_count++;
    
    tests[test_count].description = "-t with threshold 0.0";
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.0 %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    tests[test_count].command = strdup(cmd);
    tests[test_count].expected_exit_code = 0;
    test_count++;
    
    tests[test_count].description = "-t with threshold 100.0";
    snprintf(cmd, sizeof(cmd), "%s overlap -t 100.0 %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    tests[test_count].command = strdup(cmd);
    tests[test_count].expected_exit_code = 0;
    test_count++;
    
    /* Test 10: Combination of some flags */
    tests[test_count].description = "Combination -v -f -t 0.5";
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f -t 0.5 %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    tests[test_count].command = strdup(cmd);
    tests[test_count].expected_exit_code = 0;
    test_count++;
    
    tests[test_count].description = "Combination -F -o -h";
    snprintf(cmd, sizeof(cmd), "%s overlap -F -o -h %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    tests[test_count].command = strdup(cmd);
    tests[test_count].expected_exit_code = 0;
    test_count++;
    
    /* Test 11: Repeated flags */
    tests[test_count].description = "Repeated -v flag";
    snprintf(cmd, sizeof(cmd), "%s overlap -v -v %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    tests[test_count].command = strdup(cmd);
    tests[test_count].expected_exit_code = 0;
    test_count++;
    
    /* Test 12: Flags with single gcda file (edge case) */
    tests[test_count].description = "Single gcda file with flags";
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f %s",
             gcov_tool_path, gcda_file1);
    tests[test_count].command = strdup(cmd);
    tests[test_count].expected_exit_code = 0;
    test_count++;
    
    /* Test 13: Invalid threshold (non-numeric) - should trigger atof */
    tests[test_count].description = "Invalid threshold (non-numeric)";
    snprintf(cmd, sizeof(cmd), "%s overlap -t not_a_number %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    tests[test_count].command = strdup(cmd);
    tests[test_count].expected_exit_code = 1;  /* Expected to fail */
    test_count++;
    
    /* Test 14: Missing argument for -t (edge case) */
    tests[test_count].description = "Missing argument for -t";
    snprintf(cmd, sizeof(cmd), "%s overlap -t %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    tests[test_count].command = strdup(cmd);
    tests[test_count].expected_exit_code = 1;  /* Expected to fail */
    test_count++;
    
    /* Test 15: Unknown flag to trigger default case */
    tests[test_count].description = "Unknown flag -x (trigger default)";
    snprintf(cmd, sizeof(cmd), "%s overlap -x %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    tests[test_count].command = strdup(cmd);
    tests[test_count].expected_exit_code = 1;  /* Expected to fail */
    test_count++;
    
    /* Test 16: Mix of valid and invalid flags */
    tests[test_count].description = "Mix valid flags with invalid";
    snprintf(cmd, sizeof(cmd), "%s overlap -v -x -f %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    tests[test_count].command = strdup(cmd);
    tests[test_count].expected_exit_code = 1;  /* Expected to fail */
    test_count++;
    
    /* Test 17: Empty flags (just overlap command) */
    tests[test_count].description = "No flags, just overlap";
    snprintf(cmd, sizeof(cmd), "%s overlap %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    tests[test_count].command = strdup(cmd);
    tests[test_count].expected_exit_code = 0;
    test_count++;
    
    /* Test 18: Flags separated (not combined) */
    tests[test_count].description = "Flags separated with spaces";
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f -F -o -h -t 0.33 %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    tests[test_count].command = strdup(cmd);
    tests[test_count].expected_exit_code = 0;
    test_count++;
    
    /* Test 19: Using absolute paths for gcda files */
    char abs_gcda1[1024];
    char abs_gcda2[1024];
    realpath(gcda_file1, abs_gcda1);
    realpath(gcda_file2, abs_gcda2);
    
    tests[test_count].description = "Absolute paths with flags";
    snprintf(cmd, sizeof(cmd), "%s overlap -v -F -t 0.5 %s %s",
             gcov_tool_path, abs_gcda1, abs_gcda2);
    tests[test_count].command = strdup(cmd);
    tests[test_count].expected_exit_code = 0;
    test_count++;
    
    /* Test 20: Threshold with scientific notation */
    tests[test_count].description = "Threshold scientific notation";
    snprintf(cmd, sizeof(cmd), "%s overlap -t 1e-2 %s %s",
             gcov_tool_path, gcda_file1, gcda_file2);
    tests[test_count].command = strdup(cmd);
    tests[test_count].expected_exit_code = 0;
    test_count++;
    
    /* Execute all tests */
    for (int i = 0; i < test_count; i++) {
        printf("\n--- Test %d: %s ---\n", i + 1, tests[i].description);
        tests[i].actual_exit_code = run_gcov_tool(tests[i].command);
        tests[i].executed = 1;
        
        /* Small delay to avoid overwhelming the system */
        usleep(100000);
    }
    
    /* Print summary */
    print_test_summary(tests, test_count);
    
    /* Free allocated command strings */
    for (int i = 0; i < test_count; i++) {
        free((void*)tests[i].command);
    }
}

/**
 * Print test execution summary
 */
void print_test_summary(test_case_t *tests, int num_tests) {
    printf("\n=== Test Execution Summary ===\n");
    printf("%-40s %-12s %-12s %s\n", 
           "Test Description", "Expected", "Actual", "Status");
    printf("%-40s %-12s %-12s %s\n",
           "----------------", "--------", "------", "------");
    
    int passed = 0;
    for (int i = 0; i < num_tests; i++) {
        if (!tests[i].executed) continue;
        
        const char *status = "FAIL";
        if (tests[i].actual_exit_code == tests[i].expected_exit_code) {
            status = "PASS";
            passed++;
        }
        
        printf("%-40s %-12d %-12d %s\n",
               tests[i].description,
               tests[i].expected_exit_code,
               tests[i].actual_exit_code,
               status);
    }
    
    printf("\nTotal Tests: %d, Passed: %d, Failed: %d\n",
           num_tests, passed, num_tests - passed);
}
