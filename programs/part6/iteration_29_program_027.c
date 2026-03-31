/**
 * test_gcov_tool_overlap.c
 * 
 * Test driver to exercise the uncovered lines in gcov-tool.cc's
 * parse_overlap_options function (lines 534-554).
 * 
 * Compile with: gcc -O0 -Wall -Wextra test_gcov_tool_overlap.c -o test_gcov_tool_overlap
 * 
 * Requirements:
 * 1. gcov-tool must be built with coverage instrumentation (--enable-coverage)
 * 2. The test program should be run from a directory with write permissions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#define MAX_CMD_LEN 1024
#define MAX_PATH_LEN 512
#define TEMP_DIR_TEMPLATE "/tmp/gcov_test_XXXXXX"

/* Global test counters */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

/* Test result structure */
typedef struct {
    char *description;
    char *command;
    int expected_exit_code;
    int actual_exit_code;
    int passed;
} test_result_t;

/* Function prototypes */
static int run_command(const char *cmd, int *exit_code);
static int create_test_program(const char *dir_path);
static int generate_gcda_files(const char *dir_path, int num_files);
static void run_test_suite(const char *gcov_tool_path, const char *test_dir);
static void print_summary(void);
static void cleanup_temp_dir(const char *dir_path);

/**
 * Execute a shell command and capture its exit code
 */
static int run_command(const char *cmd, int *exit_code) {
    int status;
    
    printf("Running: %s\n", cmd);
    fflush(stdout);
    
    status = system(cmd);
    
    if (WIFEXITED(status)) {
        *exit_code = WEXITSTATUS(status);
        return 0;
    } else if (WIFSIGNALED(status)) {
        *exit_code = WTERMSIG(status);
        return -1;
    } else {
        *exit_code = -1;
        return -1;
    }
}

/**
 * Create a minimal C program for GCOV instrumentation
 */
static int create_test_program(const char *dir_path) {
    char source_path[MAX_PATH_LEN];
    char compile_cmd[MAX_CMD_LEN];
    int exit_code;
    
    /* Create source file path */
    snprintf(source_path, sizeof(source_path), "%s/test_prog.c", dir_path);
    
    /* Write simple C program */
    FILE *fp = fopen(source_path, "w");
    if (!fp) {
        perror("Failed to create test program");
        return -1;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int helper(int x) {\n");
    fprintf(fp, "    if (x > 0) {\n");
    fprintf(fp, "        return x * 2;\n");
    fprintf(fp, "    } else {\n");
    fprintf(fp, "        return x + 1;\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "}\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int result = 0;\n");
    fprintf(fp, "    for (int i = 0; i < 10; i++) {\n");
    fprintf(fp, "        result += helper(i);\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    printf(\"Result: %%d\\n\", result);\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    
    /* Compile with GCOV instrumentation */
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s/test_prog %s/test_prog.c",
             dir_path, dir_path);
    
    if (run_command(compile_cmd, &exit_code) != 0 || exit_code != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return -1;
    }
    
    return 0;
}

/**
 * Generate multiple .gcda files by running the test program
 */
static int generate_gcda_files(const char *dir_path, int num_files) {
    char run_cmd[MAX_CMD_LEN];
    char copy_cmd[MAX_CMD_LEN];
    int exit_code;
    
    if (num_files < 1) {
        return -1;
    }
    
    /* Run the program once to generate initial .gcda file */
    snprintf(run_cmd, sizeof(run_cmd), "cd %s && ./test_prog", dir_path);
    if (run_command(run_cmd, &exit_code) != 0 || exit_code != 0) {
        fprintf(stderr, "Failed to run test program\n");
        return -1;
    }
    
    /* Create additional copies with different names */
    for (int i = 2; i <= num_files; i++) {
        snprintf(copy_cmd, sizeof(copy_cmd),
                 "cp %s/test_prog.gcda %s/test_prog%d.gcda",
                 dir_path, dir_path, i);
        
        if (run_command(copy_cmd, &exit_code) != 0 || exit_code != 0) {
            fprintf(stderr, "Failed to copy gcda file %d\n", i);
            return -1;
        }
    }
    
    return 0;
}

/**
 * Run a single test case and record results
 */
static test_result_t run_test_case(const char *description, 
                                   const char *command, 
                                   int expected_exit_code) {
    test_result_t result;
    int exit_code;
    
    result.description = strdup(description);
    result.command = strdup(command);
    result.expected_exit_code = expected_exit_code;
    
    tests_run++;
    
    if (run_command(command, &exit_code) == 0) {
        result.actual_exit_code = exit_code;
        result.passed = (exit_code == expected_exit_code);
        
        if (result.passed) {
            tests_passed++;
            printf("  ✓ PASS: %s (exit code: %d)\n", description, exit_code);
        } else {
            tests_failed++;
            printf("  ✗ FAIL: %s (expected %d, got %d)\n", 
                   description, expected_exit_code, exit_code);
        }
    } else {
        result.actual_exit_code = -1;
        result.passed = 0;
        tests_failed++;
        printf("  ✗ FAIL: %s (command execution failed)\n", description);
    }
    
    printf("\n");
    return result;
}

/**
 * Main test suite
 */
static void run_test_suite(const char *gcov_tool_path, const char *test_dir) {
    char cmd[MAX_CMD_LEN];
    test_result_t *results = NULL;
    int num_results = 0;
    
    printf("========================================\n");
    printf("Testing gcov-tool overlap argument parsing\n");
    printf("========================================\n\n");
    
    /* Test 1: Basic overlap with all uncovered flags */
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f -F -o -h -t 0.75 %s/test_prog.gcda %s/test_prog2.gcda",
             gcov_tool_path, test_dir, test_dir);
    run_test_case("All uncovered flags combined", cmd, 0);
    
    /* Test 2: Different order of flags */
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.5 -h -o -F -f -v %s/test_prog.gcda %s/test_prog2.gcda",
             gcov_tool_path, test_dir, test_dir);
    run_test_case("Flags in reverse order", cmd, 0);
    
    /* Test 3: Single flag -v (verbose) */
    snprintf(cmd, sizeof(cmd), "%s overlap -v %s/test_prog.gcda %s/test_prog2.gcda",
             gcov_tool_path, test_dir, test_dir);
    run_test_case("Verbose flag only", cmd, 0);
    
    /* Test 4: Single flag -f (function level) */
    snprintf(cmd, sizeof(cmd), "%s overlap -f %s/test_prog.gcda %s/test_prog2.gcda",
             gcov_tool_path, test_dir, test_dir);
    run_test_case("Function level flag only", cmd, 0);
    
    /* Test 5: Single flag -F (full filename) */
    snprintf(cmd, sizeof(cmd), "%s overlap -F %s/test_prog.gcda %s/test_prog2.gcda",
             gcov_tool_path, test_dir, test_dir);
    run_test_case("Full filename flag only", cmd, 0);
    
    /* Test 6: Single flag -o (object level) */
    snprintf(cmd, sizeof(cmd), "%s overlap -o %s/test_prog.gcda %s/test_prog2.gcda",
             gcov_tool_path, test_dir, test_dir);
    run_test_case("Object level flag only", cmd, 0);
    
    /* Test 7: Single flag -h (hot only) */
    snprintf(cmd, sizeof(cmd), "%s overlap -h %s/test_prog.gcda %s/test_prog2.gcda",
             gcov_tool_path, test_dir, test_dir);
    run_test_case("Hot only flag only", cmd, 0);
    
    /* Test 8: Flag -t with different threshold values */
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.0 %s/test_prog.gcda %s/test_prog2.gcda",
             gcov_tool_path, test_dir, test_dir);
    run_test_case("Threshold 0.0", cmd, 0);
    
    snprintf(cmd, sizeof(cmd), "%s overlap -t 1.0 %s/test_prog.gcda %s/test_prog2.gcda",
             gcov_tool_path, test_dir, test_dir);
    run_test_case("Threshold 1.0", cmd, 0);
    
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.25 %s/test_prog.gcda %s/test_prog2.gcda",
             gcov_tool_path, test_dir, test_dir);
    run_test_case("Threshold 0.25", cmd, 0);
    
    /* Test 9: Flag combinations */
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f -F %s/test_prog.gcda %s/test_prog2.gcda",
             gcov_tool_path, test_dir, test_dir);
    run_test_case("Verbose + function + fullname", cmd, 0);
    
    snprintf(cmd, sizeof(cmd), "%s overlap -o -h -t 0.8 %s/test_prog.gcda %s/test_prog2.gcda",
             gcov_tool_path, test_dir, test_dir);
    run_test_case("Object + hot + threshold", cmd, 0);
    
    /* Test 10: Repeated flags */
    snprintf(cmd, sizeof(cmd), "%s overlap -v -v -v %s/test_prog.gcda %s/test_prog2.gcda",
             gcov_tool_path, test_dir, test_dir);
    run_test_case("Repeated verbose flags", cmd, 0);
    
    /* Test 11: Edge case - invalid argument for -t */
    snprintf(cmd, sizeof(cmd), "%s overlap -t not_a_number %s/test_prog.gcda %s/test_prog2.gcda",
             gcov_tool_path, test_dir, test_dir);
    run_test_case("Invalid threshold (non-numeric)", cmd, 1); /* Should fail */
    
    /* Test 12: Edge case - missing argument for -t (at end) */
    snprintf(cmd, sizeof(cmd), "%s overlap -t %s/test_prog.gcda %s/test_prog2.gcda",
             gcov_tool_path, test_dir, test_dir);
    run_test_case("Missing threshold argument", cmd, 1); /* Should fail */
    
    /* Test 13: Edge case - unknown flag (triggers default case) */
    snprintf(cmd, sizeof(cmd), "%s overlap -x %s/test_prog.gcda %s/test_prog2.gcda",
             gcov_tool_path, test_dir, test_dir);
    run_test_case("Unknown flag -x (triggers usage)", cmd, 1); /* Should fail */
    
    /* Test 14: Multiple input files */
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f %s/test_prog.gcda %s/test_prog2.gcda %s/test_prog3.gcda",
             gcov_tool_path, test_dir, test_dir, test_dir);
    run_test_case("Three input files", cmd, 0);
    
    /* Test 15: Absolute paths */
    char abs_path1[MAX_PATH_LEN], abs_path2[MAX_PATH_LEN];
    snprintf(abs_path1, sizeof(abs_path1), "%s/test_prog.gcda", test_dir);
    snprintf(abs_path2, sizeof(abs_path2), "%s/test_prog2.gcda", test_dir);
    snprintf(cmd, sizeof(cmd), "%s overlap -v -F %s %s",
             gcov_tool_path, abs_path1, abs_path2);
    run_test_case("Absolute paths with fullname flag", cmd, 0);
    
    /* Test 16: Mixed flag ordering with files in between */
    snprintf(cmd, sizeof(cmd), "%s overlap -v %s/test_prog.gcda -f %s/test_prog2.gcda -t 0.5",
             gcov_tool_path, test_dir, test_dir);
    run_test_case("Flags interspersed with filenames", cmd, 0);
    
    /* Test 17: Only threshold with extreme value */
    snprintf(cmd, sizeof(cmd), "%s overlap -t 100.5 %s/test_prog.gcda %s/test_prog2.gcda",
             gcov_tool_path, test_dir, test_dir);
    run_test_case("Large threshold value", cmd, 0);
    
    /* Test 18: Negative threshold */
    snprintf(cmd, sizeof(cmd), "%s overlap -t -0.5 %s/test_prog.gcda %s/test_prog2.gcda",
             gcov_tool_path, test_dir, test_dir);
    run_test_case("Negative threshold", cmd, 0);
    
    /* Test 19: Scientific notation threshold */
    snprintf(cmd, sizeof(cmd), "%s overlap -t 1e-3 %s/test_prog.gcda %s/test_prog2.gcda",
             gcov_tool_path, test_dir, test_dir);
    run_test_case("Scientific notation threshold", cmd, 0);
}

/**
 * Print test summary
 */
static void print_summary(void) {
    printf("\n========================================\n");
    printf("Test Summary\n");
    printf("========================================\n");
    printf("Tests Run:    %d\n", tests_run);
    printf("Tests Passed: %d\n", tests_passed);
    printf("Tests Failed: %d\n", tests_failed);
    printf("Pass Rate:    %.1f%%\n", 
           tests_run > 0 ? (100.0 * tests_passed / tests_run) : 0.0);
    
    if (tests_failed == 0) {
        printf("\n✅ All tests passed!\n");
    } else {
        printf("\n❌ Some tests failed\n");
    }
}

/**
 * Clean up temporary directory
 */
static void cleanup_temp_dir(const char *dir_path) {
    char cmd[MAX_CMD_LEN];
    int exit_code;
    
    if (dir_path && strstr(dir_path, "/tmp/gcov_test_") == dir_path) {
        snprintf(cmd, sizeof(cmd), "rm -rf %s", dir_path);
        run_command(cmd, &exit_code);
        printf("Cleaned up temporary directory: %s\n", dir_path);
    }
}

/**
 * Main function
 */
int main(int argc, char *argv[]) {
    char temp_dir[MAX_PATH_LEN];
    char *gcov_tool_path = "gcov-tool";  /* Default path */
    
    /* Allow overriding gcov-tool path via command line */
    if (argc > 1) {
        gcov_tool_path = argv[1];
    }
    
    printf("Using gcov-tool at: %s\n", gcov_tool_path);
    
    /* Create temporary directory */
    strncpy(temp_dir, TEMP_DIR_TEMPLATE, sizeof(temp_dir));
    if (mkdtemp(temp_dir) == NULL) {
        perror("Failed to create temporary directory");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", temp_dir);
    
    /* Create test program and generate GCOV data */
    if (create_test_program(temp_dir) != 0) {
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    if (generate_gcda_files(temp_dir, 3) != 0) {
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Run the test suite */
    run_test_suite(gcov_tool_path, temp_dir);
    
    /* Print summary */
    print_summary();
    
    /* Clean up */
    cleanup_temp_dir(temp_dir);
    
    return (tests_failed == 0) ? 0 : 1;
}
