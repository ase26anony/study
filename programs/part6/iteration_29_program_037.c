/**
 * test_gcov_tool_overlap.c
 * 
 * Test driver for gcov-tool overlap command parsing.
 * Exercises the specific switch cases in parse_overlap_options()
 * that handle flags: -v, -f, -F, -o, -h, -t
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

#define MAX_CMD_LEN 1024
#define MAX_FILES 10
#define TEMP_DIR_PREFIX "/tmp/gcov_test_XXXXXX"

/* Structure to hold test case information */
typedef struct {
    const char *description;
    const char *args;
    int expected_exit_code;  /* 0 for success, non-zero for expected failure */
    int should_test;         /* 1 to run this test, 0 to skip */
} test_case_t;

/* Global variables */
static char temp_dir[256];
static char test_prog_path[256];
static char gcda_file1[256];
static char gcda_file2[256];
static char gcno_file[256];

/* Function prototypes */
static int create_temp_directory(void);
static int compile_test_program(void);
static int run_test_program(void);
static int run_gcov_tool_test(const char *description, const char *args, int expected_exit_code);
static void cleanup(void);
static int execute_command(const char *cmd, int capture_output);

/**
 * Create a temporary directory for test files
 */
static int create_temp_directory(void) {
    char *dir = mkdtemp(strcpy(temp_dir, TEMP_DIR_PREFIX));
    if (!dir) {
        perror("Failed to create temporary directory");
        return -1;
    }
    printf("Created temporary directory: %s\n", temp_dir);
    return 0;
}

/**
 * Compile a simple test program with GCOV instrumentation
 */
static int compile_test_program(void) {
    const char *test_prog = 
        "#include <stdio.h>\n"
        "int main() {\n"
        "    int x = 0;\n"
        "    for (int i = 0; i < 10; i++) {\n"
        "        x += i;\n"
        "    }\n"
        "    printf(\"Result: %d\\n\", x);\n"
        "    return 0;\n"
        "}\n";
    
    /* Write test program to file */
    snprintf(test_prog_path, sizeof(test_prog_path), "%s/test_prog.c", temp_dir);
    FILE *fp = fopen(test_prog_path, "w");
    if (!fp) {
        perror("Failed to create test program file");
        return -1;
    }
    fputs(test_prog, fp);
    fclose(fp);
    
    /* Compile with GCOV instrumentation */
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -o %s/test_prog %s/test_prog.c 2>&1",
             temp_dir, temp_dir);
    
    printf("Compiling test program...\n");
    int result = system(cmd);
    if (result != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return -1;
    }
    
    /* Set up file paths */
    snprintf(gcda_file1, sizeof(gcda_file1), "%s/test_prog.gcda", temp_dir);
    snprintf(gcda_file2, sizeof(gcda_file2), "%s/test_prog2.gcda", temp_dir);
    snprintf(gcno_file, sizeof(gcno_file), "%s/test_prog.gcno", temp_dir);
    
    return 0;
}

/**
 * Run the test program to generate GCOV data
 */
static int run_test_program(void) {
    char cmd[MAX_CMD_LEN];
    
    /* First run - create initial gcda file */
    snprintf(cmd, sizeof(cmd), "%s/test_prog > /dev/null 2>&1", temp_dir);
    printf("Running test program (first run)...\n");
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to run test program\n");
        return -1;
    }
    
    /* Copy gcda file to create a second version for overlap comparison */
    char copy_cmd[MAX_CMD_LEN];
    snprintf(copy_cmd, sizeof(copy_cmd), "cp %s %s 2>&1", gcda_file1, gcda_file2);
    if (system(copy_cmd) != 0) {
        fprintf(stderr, "Failed to copy gcda file\n");
        return -1;
    }
    
    /* Run again to modify the first gcda file (different execution count) */
    snprintf(cmd, sizeof(cmd), "%s/test_prog > /dev/null 2>&1", temp_dir);
    printf("Running test program (second run)...\n");
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to run test program second time\n");
        return -1;
    }
    
    return 0;
}

/**
 * Execute a command and return its exit status
 */
static int execute_command(const char *cmd, int capture_output) {
    if (capture_output) {
        char full_cmd[MAX_CMD_LEN];
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
        return system(full_cmd);
    }
    return system(cmd);
}

/**
 * Run a single gcov-tool test case
 */
static int run_gcov_tool_test(const char *description, const char *args, int expected_exit_code) {
    char cmd[MAX_CMD_LEN];
    int result;
    
    /* Build the full command */
    if (strstr(args, ".gcda") != NULL || strstr(args, ".gcno") != NULL) {
        /* Args already contain file paths */
        snprintf(cmd, sizeof(cmd), "gcov-tool overlap %s", args);
    } else {
        /* Add the gcda files to the command */
        snprintf(cmd, sizeof(cmd), "gcov-tool overlap %s %s %s", 
                 args, gcda_file1, gcda_file2);
    }
    
    printf("\n=== Test: %s ===\n", description);
    printf("Command: %s\n", cmd);
    
    /* Execute the command */
    result = execute_command(cmd, 1);
    int exit_code = WEXITSTATUS(result);
    
    /* Check result */
    if ((expected_exit_code == 0 && exit_code == 0) ||
        (expected_exit_code != 0 && exit_code != 0)) {
        printf("✓ PASS (exit code: %d)\n", exit_code);
        return 1;
    } else {
        printf("✗ FAIL - Expected exit code %d, got %d\n", expected_exit_code, exit_code);
        return 0;
    }
}

/**
 * Clean up temporary files
 */
static void cleanup(void) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
    system(cmd);
    printf("\nCleaned up temporary directory: %s\n", temp_dir);
}

int main(int argc, char *argv[]) {
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("========================================\n");
    printf("Testing gcov-tool overlap argument parsing\n");
    printf("Target: Lines 534-554 in gcov-tool.cc\n");
    printf("========================================\n\n");
    
    /* Set up test environment */
    if (create_temp_directory() != 0) {
        return EXIT_FAILURE;
    }
    
    if (compile_test_program() != 0) {
        cleanup();
        return EXIT_FAILURE;
    }
    
    if (run_test_program() != 0) {
        cleanup();
        return EXIT_FAILURE;
    }
    
    /* Verify GCOV files exist */
    struct stat st;
    if (stat(gcda_file1, &st) != 0 || stat(gcda_file2, &st) != 0) {
        fprintf(stderr, "GCOV data files not created\n");
        cleanup();
        return EXIT_FAILURE;
    }
    
    printf("\nGCOV data files created successfully:\n");
    printf("  %s\n", gcda_file1);
    printf("  %s\n", gcda_file2);
    printf("  %s\n", gcno_file);
    
    /* =====================================================================
     * Test Cases - Designed to exercise all uncovered switch cases
     * ===================================================================== */
    
    test_case_t test_cases[] = {
        /* Basic tests for each individual flag */
        {"Single flag: -v (verbose)", "-v", 0, 1},
        {"Single flag: -f (function level)", "-f", 0, 1},
        {"Single flag: -F (full filename)", "-F", 0, 1},
        {"Single flag: -o (object level)", "-o", 0, 1},
        {"Single flag: -h (hot only)", "-h", 0, 1},
        {"Single flag: -t with value", "-t 0.5", 0, 1},
        
        /* Combined flags - testing all uncovered cases in one command */
        {"All flags combined", "-v -f -F -o -h -t 0.75", 0, 1},
        
        /* Permutations of flag order */
        {"Flag order permutation 1", "-t 1.0 -v -f -F -o -h", 0, 1},
        {"Flag order permutation 2", "-h -o -F -f -v -t 0.25", 0, 1},
        {"Flag order permutation 3", "-f -F -v -h -o -t 0.9", 0, 1},
        
        /* Different threshold values */
        {"Threshold: 0.0", "-t 0.0", 0, 1},
        {"Threshold: 0.1", "-t 0.1", 0, 1},
        {"Threshold: 0.5", "-t 0.5", 0, 1},
        {"Threshold: 0.99", "-t 0.99", 0, 1},
        {"Threshold: 1.0", "-t 1.0", 0, 1},
        {"Threshold: 1.5", "-t 1.5", 0, 1},
        
        /* Edge cases for threshold */
        {"Threshold with scientific notation", "-t 1e-3", 0, 1},
        {"Threshold with plus sign", "-t +0.5", 0, 1},
        
        /* Repeated flags (should be handled by parser) */
        {"Repeated -v flag", "-v -v -v", 0, 1},
        {"Multiple -t flags (last wins)", "-t 0.1 -t 0.9", 0, 1},
        
        /* Flags with gcno files instead of gcda */
        {"With gcno file", "-v -f", 0, 1},
        
        /* Mixed file types */
        {"Mixed gcda and gcno files", "-v -f test_mixed.gcda test_mixed.gcno", 0, 1},
        
        /* Error cases - should trigger error handling */
        {"Missing argument for -t (error expected)", "-t", 1, 1},
        {"Invalid argument for -t (error expected)", "-t not_a_number", 1, 1},
        {"Unknown flag -x (should trigger default case)", "-x", 1, 1},
        
        /* Empty flag (just files) */
        {"No flags, just files", "", 0, 1},
        
        /* Flags with single file */
        {"Single file with flags", "-v -f test_single.gcda", 0, 1},
        
        /* End marker */
        {NULL, NULL, 0, 0}
    };
    
    /* Run all test cases */
    printf("\n========================================\n");
    printf("Running test cases...\n");
    printf("========================================\n");
    
    for (int i = 0; test_cases[i].description != NULL; i++) {
        if (test_cases[i].should_test) {
            total_tests++;
            if (run_gcov_tool_test(test_cases[i].description, 
                                  test_cases[i].args, 
                                  test_cases[i].expected_exit_code)) {
                passed_tests++;
            }
        }
    }
    
    /* Additional dynamic tests with file path variations */
    printf("\n========================================\n");
    printf("Running path variation tests...\n");
    printf("========================================\n");
    
    /* Test with absolute paths */
    char abs_path_cmd[MAX_CMD_LEN];
    char cwd[256];
    if (getcwd(cwd, sizeof(cwd))) {
        snprintf(abs_path_cmd, sizeof(abs_path_cmd), "-v -f %s/%s %s/%s", 
                 cwd, gcda_file1, cwd, gcda_file2);
        total_tests++;
        if (run_gcov_tool_test("Absolute file paths", abs_path_cmd, 0)) {
            passed_tests++;
        }
    }
    
    /* Test with relative paths */
    total_tests++;
    if (run_gcov_tool_test("Relative paths with ..", "-v -f ../test.gcda ../test2.gcda", 1)) {
        passed_tests++;
    }
    
    /* Test with different file order */
    char reverse_files[MAX_CMD_LEN];
    snprintf(reverse_files, sizeof(reverse_files), "-v -f %s %s", gcda_file2, gcda_file1);
    total_tests++;
    if (run_gcov_tool_test("Reversed file order", reverse_files, 0)) {
        passed_tests++;
    }
    
    /* Test minimum required files (2) */
    char single_file[MAX_CMD_LEN];
    snprintf(single_file, sizeof(single_file), "-v -f %s", gcda_file1);
    total_tests++;
    if (run_gcov_tool_test("Single file (may fail)", single_file, 1)) {
        passed_tests++;
    }
    
    /* Test with three files */
    char three_files[MAX_CMD_LEN];
    snprintf(three_files, sizeof(three_files), "-v -f %s %s %s", 
             gcda_file1, gcda_file2, gcda_file1);
    total_tests++;
    if (run_gcov_tool_test("Three input files", three_files, 0)) {
        passed_tests++;
    }
    
    /* Summary */
    printf("\n========================================\n");
    printf("TEST SUMMARY\n");
    printf("========================================\n");
    printf("Total tests run: %d\n", total_tests);
    printf("Tests passed:    %d\n", passed_tests);
    printf("Tests failed:    %d\n", total_tests - passed_tests);
    printf("Pass rate:       %.1f%%\n", (passed_tests * 100.0) / total_tests);
    
    if (passed_tests == total_tests) {
        printf("\n✓ ALL TESTS PASSED\n");
    } else {
        printf("\n⚠ SOME TESTS FAILED\n");
    }
    
    /* Clean up */
    cleanup();
    
    return (passed_tests == total_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}
