/**
 * test_gcov_tool_overlap.c
 * 
 * Test driver to exercise the parse_overlap_options function in gcov-tool.cc
 * Specifically targets lines 534-554 handling flags: -v, -f, -F, -o, -h, -t
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

typedef struct {
    char *cmd;
    int expected_exit;
    char *description;
} test_case_t;

/* Global variables to track test results */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

/* Function prototypes */
static int run_gcov_tool(const char *cmd);
static int create_test_gcda_files(const char *dir, char **files, int count);
static void cleanup_temp_dir(const char *dir);
static void run_test_case(const test_case_t *test);
static void print_summary(void);

/* Simple C program to generate GCOV data */
static const char *test_program = 
    "#include <stdio.h>\n"
    "int main() {\n"
    "    int x = 0;\n"
    "    for (int i = 0; i < 10; i++) {\n"
    "        x += i;\n"
    "    }\n"
    "    printf(\"Result: %d\\n\", x);\n"
    "    return 0;\n"
    "}\n";

int main(int argc, char *argv[]) {
    char temp_dir[256];
    char *gcda_files[MAX_FILES];
    char gcda_paths[MAX_FILES][256];
    int num_files = 2;  /* Use 2 files for overlap comparison */
    
    printf("=== GCOV-TOOL Overlap Parser Test ===\n\n");
    
    /* Create temporary directory for test files */
    strcpy(temp_dir, TEMP_DIR_PATTERN);
    if (mkdtemp(temp_dir) == NULL) {
        perror("Failed to create temp directory");
        return 1;
    }
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Create test source file */
    char src_path[256];
    snprintf(src_path, sizeof(src_path), "%s/test.c", temp_dir);
    FILE *src = fopen(src_path, "w");
    if (!src) {
        perror("Failed to create test source");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    fputs(test_program, src);
    fclose(src);
    
    /* Compile test program with GCOV instrumentation */
    printf("Compiling test program with GCOV instrumentation...\n");
    char compile_cmd[512];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -o %s/test_prog %s/test.c",
             temp_dir, temp_dir);
    
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Generate multiple .gcda files by running the program multiple times */
    for (int i = 0; i < num_files; i++) {
        char run_cmd[256];
        snprintf(run_cmd, sizeof(run_cmd), "cd %s && ./test_prog > /dev/null 2>&1", temp_dir);
        system(run_cmd);
        
        /* Rename gcda file to create multiple versions */
        char old_gcda[256], new_gcda[256];
        snprintf(old_gcda, sizeof(old_gcda), "%s/test.gcda", temp_dir);
        snprintf(new_gcda, sizeof(new_gcda), "%s/test%d.gcda", temp_dir, i);
        
        if (rename(old_gcda, new_gcda) == 0) {
            gcda_files[i] = strdup(new_gcda);
            strcpy(gcda_paths[i], new_gcda);
        } else {
            /* If rename fails, use the original */
            gcda_files[i] = strdup(old_gcda);
            strcpy(gcda_paths[i], old_gcda);
        }
    }
    
    printf("Generated %d .gcda files\n\n", num_files);
    
    /* =====================================================================
       TEST CASES - Targeting the uncovered switch statement in parse_overlap_options
       ===================================================================== */
    
    /* Test 1: All flags combined (covers all case statements) */
    test_case_t test1 = {
        .cmd = NULL,  /* Will be constructed dynamically */
        .expected_exit = 0,
        .description = "All flags combined: -v -f -F -o -h -t 0.75"
    };
    
    /* Test 2: Different order of flags */
    test_case_t test2 = {
        .cmd = NULL,
        .expected_exit = 0,
        .description = "Different flag order: -t 0.5 -h -o -F -f -v"
    };
    
    /* Test 3: Single flags individually (run multiple tests) */
    test_case_t test3[] = {
        {NULL, 0, "Verbose flag only: -v"},
        {NULL, 0, "Function level flag: -f"},
        {NULL, 0, "Fullname flag: -F"},
        {NULL, 0, "Object level flag: -o"},
        {NULL, 0, "Hot only flag: -h"},
        {NULL, 0, "Threshold flag: -t 1.0"},
    };
    
    /* Test 4: Flag combinations */
    test_case_t test4[] = {
        {NULL, 0, "Verbose + function level: -v -f"},
        {NULL, 0, "Fullname + object level: -F -o"},
        {NULL, 0, "Hot only with threshold: -h -t 0.9"},
        {NULL, 0, "All except threshold: -v -f -F -o -h"},
    };
    
    /* Test 5: Edge cases and error conditions */
    test_case_t test5[] = {
        {NULL, 1, "Invalid threshold (non-numeric): -t not_a_number"},
        {NULL, 1, "Missing threshold value: -t"},
        {NULL, 1, "Unknown flag: -x"},
        {NULL, 0, "Repeated flags: -v -v -f -f"},
        {NULL, 0, "Threshold with scientific notation: -t 1.5e-1"},
    };
    
    /* Build command strings dynamically with actual file paths */
    char base_cmd[512];
    snprintf(base_cmd, sizeof(base_cmd), "gcov-tool overlap %s %s", 
             gcda_paths[0], gcda_paths[1]);
    
    /* Test 1: All flags */
    char cmd1[MAX_CMD_LEN];
    snprintf(cmd1, sizeof(cmd1), "%s -v -f -F -o -h -t 0.75", base_cmd);
    test1.cmd = cmd1;
    
    /* Test 2: Different order */
    char cmd2[MAX_CMD_LEN];
    snprintf(cmd2, sizeof(cmd2), "%s -t 0.5 -h -o -F -f -v", base_cmd);
    test2.cmd = cmd2;
    
    /* Run the tests */
    printf("Running comprehensive overlap option tests...\n\n");
    
    printf("--- Test 1: All flags combined ---\n");
    run_test_case(&test1);
    
    printf("\n--- Test 2: Different flag order ---\n");
    run_test_case(&test2);
    
    printf("\n--- Test 3: Individual flags ---\n");
    for (size_t i = 0; i < sizeof(test3)/sizeof(test3[0]); i++) {
        char cmd[MAX_CMD_LEN];
        const char *flag;
        switch (i) {
            case 0: flag = "-v"; break;
            case 1: flag = "-f"; break;
            case 2: flag = "-F"; break;
            case 3: flag = "-o"; break;
            case 4: flag = "-h"; break;
            case 5: flag = "-t 1.0"; break;
            default: flag = "";
        }
        snprintf(cmd, sizeof(cmd), "%s %s", base_cmd, flag);
        test3[i].cmd = strdup(cmd);
        run_test_case(&test3[i]);
        free(test3[i].cmd);
    }
    
    printf("\n--- Test 4: Flag combinations ---\n");
    const char *combinations[] = {
        "-v -f",
        "-F -o", 
        "-h -t 0.9",
        "-v -f -F -o -h"
    };
    for (size_t i = 0; i < sizeof(test4)/sizeof(test4[0]); i++) {
        char cmd[MAX_CMD_LEN];
        snprintf(cmd, sizeof(cmd), "%s %s", base_cmd, combinations[i]);
        test4[i].cmd = strdup(cmd);
        run_test_case(&test4[i]);
        free(test4[i].cmd);
    }
    
    printf("\n--- Test 5: Edge cases ---\n");
    const char *edge_cases[] = {
        "-t not_a_number",
        "-t",
        "-x",
        "-v -v -f -f",
        "-t 1.5e-1"
    };
    for (size_t i = 0; i < sizeof(test5)/sizeof(test5[0]); i++) {
        char cmd[MAX_CMD_LEN];
        snprintf(cmd, sizeof(cmd), "%s %s", base_cmd, edge_cases[i]);
        test5[i].cmd = strdup(cmd);
        run_test_case(&test5[i]);
        free(test5[i].cmd);
    }
    
    /* Additional permutation tests */
    printf("\n--- Test 6: Additional permutations ---\n");
    
    /* Generate permutations of the 6 main flags */
    const char *flags[] = {"-v", "-f", "-F", "-o", "-h", "-t 0.8"};
    int flag_indices[] = {0, 1, 2, 3, 4, 5};
    
    /* Test a few permutations (not all 720) */
    int permutations[][6] = {
        {0, 1, 2, 3, 4, 5},  /* Original order */
        {5, 4, 3, 2, 1, 0},  /* Reverse order */
        {2, 0, 4, 1, 5, 3},  /* Random permutation 1 */
        {4, 2, 0, 5, 3, 1},  /* Random permutation 2 */
    };
    
    for (int p = 0; p < 4; p++) {
        char perm_cmd[MAX_CMD_LEN];
        char flag_str[256] = "";
        
        for (int i = 0; i < 6; i++) {
            strcat(flag_str, " ");
            strcat(flag_str, flags[permutations[p][i]]);
        }
        
        snprintf(perm_cmd, sizeof(perm_cmd), "%s%s", base_cmd, flag_str);
        
        char desc[100];
        snprintf(desc, sizeof(desc), "Flag permutation %d", p+1);
        
        test_case_t perm_test = {
            .cmd = perm_cmd,
            .expected_exit = 0,
            .description = desc
        };
        
        run_test_case(&perm_test);
    }
    
    /* Test with different threshold values */
    printf("\n--- Test 7: Different threshold values ---\n");
    const char *thresholds[] = {"0.0", "0.25", "0.5", "0.75", "1.0", "2.5", "100.0"};
    for (size_t i = 0; i < sizeof(thresholds)/sizeof(thresholds[0]); i++) {
        char cmd[MAX_CMD_LEN];
        char desc[100];
        snprintf(cmd, sizeof(cmd), "%s -t %s", base_cmd, thresholds[i]);
        snprintf(desc, sizeof(desc), "Threshold value: %s", thresholds[i]);
        
        test_case_t thresh_test = {
            .cmd = cmd,
            .expected_exit = 0,
            .description = desc
        };
        
        run_test_case(&thresh_test);
    }
    
    /* Test with different numbers of input files */
    printf("\n--- Test 8: Different numbers of input files ---\n");
    
    /* Single file (should still parse options) */
    char single_file_cmd[MAX_CMD_LEN];
    snprintf(single_file_cmd, sizeof(single_file_cmd), 
             "gcov-tool overlap -v -f %s", gcda_paths[0]);
    
    test_case_t single_file_test = {
        .cmd = single_file_cmd,
        .expected_exit = 0,  /* May fail if overlap needs >1 file, but options still parsed */
        .description = "Single input file with flags"
    };
    run_test_case(&single_file_test);
    
    /* Three files */
    if (num_files >= 3) {
        char three_files_cmd[MAX_CMD_LEN];
        snprintf(three_files_cmd, sizeof(three_files_cmd),
                 "gcov-tool overlap -v -f %s %s %s", 
                 gcda_paths[0], gcda_paths[1], gcda_paths[2]);
        
        test_case_t three_files_test = {
            .cmd = three_files_cmd,
            .expected_exit = 0,
            .description = "Three input files with flags"
        };
        run_test_case(&three_files_test);
    }
    
    /* Clean up */
    for (int i = 0; i < num_files; i++) {
        free(gcda_files[i]);
    }
    
    cleanup_temp_dir(temp_dir);
    
    /* Print final summary */
    print_summary();
    
    return tests_failed > 0 ? 1 : 0;
}

/**
 * Run gcov-tool with the given command and check exit code
 */
static int run_gcov_tool(const char *cmd) {
    int status = system(cmd);
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    } else {
        return -1;  /* Command didn't exit normally */
    }
}

/**
 * Run a single test case
 */
static void run_test_case(const test_case_t *test) {
    tests_run++;
    
    printf("Test: %s\n", test->description);
    printf("Command: %s\n", test->cmd);
    
    int exit_code = run_gcov_tool(test->cmd);
    
    if (exit_code == test->expected_exit) {
        printf("✓ PASSED (exit code: %d)\n", exit_code);
        tests_passed++;
    } else {
        printf("✗ FAILED - Expected exit code %d, got %d\n", 
               test->expected_exit, exit_code);
        tests_failed++;
    }
    
    printf("\n");
}

/**
 * Clean up temporary directory
 */
static void cleanup_temp_dir(const char *dir) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dir);
    system(cmd);
}

/**
 * Print test summary
 */
static void print_summary(void) {
    printf("\n=== TEST SUMMARY ===\n");
    printf("Total tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    printf("Pass rate: %.1f%%\n", 
           tests_run > 0 ? (100.0 * tests_passed / tests_run) : 0.0);
    
    if (tests_failed == 0) {
        printf("\n✓ All tests passed! The uncovered lines in parse_overlap_options\n");
        printf("  should now be executed and covered.\n");
    } else {
        printf("\n⚠ Some tests failed. This may be expected for error cases.\n");
        printf("  The important thing is that the option parsing code was exercised.\n");
    }
    
    printf("\nTo generate coverage data for gcov-tool.cc:\n");
    printf("1. Ensure gcov-tool was built with --enable-coverage\n");
    printf("2. Run: gcov gcov-tool.cc\n");
    printf("3. Check that lines 534-554 show execution counts > 0\n");
}
