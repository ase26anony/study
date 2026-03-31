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
#define TEMP_DIR "/tmp/gcov_test_XXXXXX"

typedef struct {
    char *cmd;
    int expected_exit;
    char *description;
} test_case_t;

/* Global variables to prevent optimization */
volatile int use_verbose = 1;
volatile int use_func = 1;
volatile int use_fullname = 1;
volatile int use_obj = 1;
volatile int use_hot = 1;
volatile float threshold_val = 0.75;

/* Create a minimal C program for GCOV instrumentation */
const char *test_program = 
    "#include <stdio.h>\n"
    "int helper(int x) { return x * 2; }\n"
    "int main() {\n"
    "    int i;\n"
    "    for (i = 0; i < 10; i++) {\n"
    "        printf(\"%d\\n\", helper(i));\n"
    "    }\n"
    "    return 0;\n"
    "}\n";

/* Create a second program for overlap comparison */
const char *test_program2 = 
    "#include <stdio.h>\n"
    "int helper2(int x) { return x * 3; }\n"
    "int main() {\n"
    "    int i;\n"
    "    for (i = 0; i < 5; i++) {\n"
    "        printf(\"%d\\n\", helper2(i));\n"
    "    }\n"
    "    return 0;\n"
    "}\n";

/* Execute a shell command and return exit status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    fflush(stdout);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Create a temporary directory */
char *create_temp_dir() {
    char *template = strdup(TEMP_DIR);
    if (mkdtemp(template) == NULL) {
        perror("mkdtemp failed");
        free(template);
        return NULL;
    }
    return template;
}

/* Clean up temporary directory */
void cleanup_temp_dir(const char *dir) {
    if (dir) {
        char cmd[MAX_CMD_LEN];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", dir);
        system(cmd);
    }
}

/* Compile a test program with GCOV instrumentation */
int compile_with_gcov(const char *dir, const char *prog_name, const char *source) {
    char source_path[MAX_CMD_LEN];
    char exe_path[MAX_CMD_LEN];
    
    snprintf(source_path, sizeof(source_path), "%s/%s.c", dir, prog_name);
    snprintf(exe_path, sizeof(exe_path), "%s/%s", dir, prog_name);
    
    /* Write source file */
    FILE *fp = fopen(source_path, "w");
    if (!fp) {
        perror("fopen source");
        return 0;
    }
    fputs(source, fp);
    fclose(fp);
    
    /* Compile with GCOV instrumentation */
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s 2>/dev/null",
             exe_path, source_path);
    
    return execute_command(cmd) == 0;
}

/* Run a program to generate .gcda file */
int run_program(const char *dir, const char *prog_name) {
    char exe_path[MAX_CMD_LEN];
    snprintf(exe_path, sizeof(exe_path), "%s/%s", dir, prog_name);
    
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "cd %s && ./%s > /dev/null 2>&1", dir, prog_name);
    
    return execute_command(cmd) == 0;
}

/* Generate test cases for gcov-tool overlap command */
test_case_t *generate_test_cases(const char *dir, int *num_cases) {
    /* Base paths for gcda files */
    char gcda1[MAX_CMD_LEN], gcda2[MAX_CMD_LEN];
    snprintf(gcda1, sizeof(gcda1), "%s/test1.gcda", dir);
    snprintf(gcda2, sizeof(gcda2), "%s/test2.gcda", dir);
    
    /* Allocate test cases */
    test_case_t *tests = malloc(50 * sizeof(test_case_t));
    int idx = 0;
    
    /* Test 1: All flags combined (main test for uncovered lines) */
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN,
             "gcov-tool overlap -v -f -F -o -h -t %.2f %s %s",
             threshold_val, gcda1, gcda2);
    tests[idx].expected_exit = 0;
    tests[idx].description = "All flags combined (-v -f -F -o -h -t)";
    idx++;
    
    /* Test 2: Different order of flags */
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN,
             "gcov-tool overlap -t 0.5 -h -o -F -f -v %s %s",
             gcda1, gcda2);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Flags in reverse order";
    idx++;
    
    /* Test 3: Only -v flag */
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN,
             "gcov-tool overlap -v %s %s", gcda1, gcda2);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Only verbose flag (-v)";
    idx++;
    
    /* Test 4: Only -f flag */
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN,
             "gcov-tool overlap -f %s %s", gcda1, gcda2);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Only function level flag (-f)";
    idx++;
    
    /* Test 5: Only -F flag */
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN,
             "gcov-tool overlap -F %s %s", gcda1, gcda2);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Only fullname flag (-F)";
    idx++;
    
    /* Test 6: Only -o flag */
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN,
             "gcov-tool overlap -o %s %s", gcda1, gcda2);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Only object level flag (-o)";
    idx++;
    
    /* Test 7: Only -h flag */
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN,
             "gcov-tool overlap -h %s %s", gcda1, gcda2);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Only hot only flag (-h)";
    idx++;
    
    /* Test 8: Only -t flag with different values */
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN,
             "gcov-tool overlap -t 1.0 %s %s", gcda1, gcda2);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Only threshold flag (-t 1.0)";
    idx++;
    
    /* Test 9: Combination without threshold */
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN,
             "gcov-tool overlap -v -f -F -o -h %s %s", gcda1, gcda2);
    tests[idx].expected_exit = 0;
    tests[idx].description = "All flags except threshold";
    idx++;
    
    /* Test 10: Repeated flags */
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN,
             "gcov-tool overlap -v -v -f -f -t 0.25 %s %s", gcda1, gcda2);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Repeated flags (-v -v -f -f)";
    idx++;
    
    /* Test 11: Edge case - invalid argument for -t */
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN,
             "gcov-tool overlap -t not_a_number %s %s", gcda1, gcda2);
    tests[idx].expected_exit = 1;  /* Should fail */
    tests[idx].description = "Invalid threshold value";
    idx++;
    
    /* Test 12: Edge case - missing argument for -t */
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN,
             "gcov-tool overlap -t %s %s", gcda1, gcda2);
    tests[idx].expected_exit = 1;  /* Should fail */
    tests[idx].description = "Missing threshold value";
    idx++;
    
    /* Test 13: Edge case - unknown flag (triggers default case) */
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN,
             "gcov-tool overlap -x %s %s", gcda1, gcda2);
    tests[idx].expected_exit = 1;  /* Should fail */
    tests[idx].description = "Unknown flag (-x) triggers default case";
    idx++;
    
    /* Test 14: Single input file */
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN,
             "gcov-tool overlap -v -f %s", gcda1);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Single input file";
    idx++;
    
    /* Test 15: Three input files */
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN,
             "gcov-tool overlap -v %s %s %s", gcda1, gcda2, gcda1);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Three input files";
    idx++;
    
    /* Test 16: Different threshold values */
    float thresholds[] = {0.1, 0.25, 0.5, 0.75, 0.99, 1.0, 1.5, 2.0};
    for (int i = 0; i < 8; i++) {
        tests[idx].cmd = malloc(MAX_CMD_LEN);
        snprintf(tests[idx].cmd, MAX_CMD_LEN,
                 "gcov-tool overlap -t %.2f %s %s", thresholds[i], gcda1, gcda2);
        tests[idx].expected_exit = 0;
        tests[idx].description = "Various threshold values";
        idx++;
    }
    
    /* Test 17: Mixed valid and invalid flags */
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN,
             "gcov-tool overlap -v -x -f %s %s", gcda1, gcda2);
    tests[idx].expected_exit = 1;  /* Should fail due to -x */
    tests[idx].description = "Mixed valid and invalid flags";
    idx++;
    
    /* Test 18: Flags with no input files */
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN,
             "gcov-tool overlap -v -f");
    tests[idx].expected_exit = 1;  /* Should fail */
    tests[idx].description = "Flags without input files";
    idx++;
    
    *num_cases = idx;
    return tests;
}

/* Run all test cases */
void run_tests(test_case_t *tests, int num_cases) {
    int passed = 0;
    int failed = 0;
    
    printf("\n=== Running gcov-tool overlap tests ===\n\n");
    
    for (int i = 0; i < num_cases; i++) {
        printf("Test %d: %s\n", i + 1, tests[i].description);
        printf("Command: %s\n", tests[i].cmd);
        
        int exit_code = execute_command(tests[i].cmd);
        
        if ((tests[i].expected_exit == 0 && exit_code == 0) ||
            (tests[i].expected_exit != 0 && exit_code != 0)) {
            printf("✓ PASSED (exit code: %d)\n\n", exit_code);
            passed++;
        } else {
            printf("✗ FAILED - Expected exit code %d, got %d\n\n", 
                   tests[i].expected_exit, exit_code);
            failed++;
        }
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", num_cases);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    printf("Coverage lines targeted: 534-554 in gcov-tool.cc\n");
    printf("Flags exercised: -v, -f, -F, -o, -h, -t\n");
}

/* Main test driver */
int main(int argc, char *argv[]) {
    char *temp_dir = NULL;
    int ret = 0;
    
    printf("=== GCOV Tool Overlap Parser Test ===\n");
    
    /* Create temporary directory */
    temp_dir = create_temp_dir();
    if (!temp_dir) {
        fprintf(stderr, "Failed to create temp directory\n");
        return 1;
    }
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Compile and run test programs to generate gcda files */
    printf("\nGenerating GCOV data files...\n");
    
    if (!compile_with_gcov(temp_dir, "test1", test_program)) {
        fprintf(stderr, "Failed to compile test1\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    if (!compile_with_gcov(temp_dir, "test2", test_program2)) {
        fprintf(stderr, "Failed to compile test2\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    if (!run_program(temp_dir, "test1")) {
        fprintf(stderr, "Failed to run test1\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    if (!run_program(temp_dir, "test2")) {
        fprintf(stderr, "Failed to run test2\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Copy gcda files to predictable names */
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "cp %s/test1.gcda %s/test1.gcda.bak 2>/dev/null", temp_dir, temp_dir);
    system(cmd);
    snprintf(cmd, sizeof(cmd), "cp %s/test2.gcda %s/test2.gcda.bak 2>/dev/null", temp_dir, temp_dir);
    system(cmd);
    
    /* Run test1 multiple times to get different coverage data */
    for (int i = 0; i < 3; i++) {
        run_program(temp_dir, "test1");
    }
    
    /* Generate and run test cases */
    int num_cases = 0;
    test_case_t *tests = generate_test_cases(temp_dir, &num_cases);
    
    if (tests && num_cases > 0) {
        run_tests(tests, num_cases);
        
        /* Free allocated memory */
        for (int i = 0; i < num_cases; i++) {
            free(tests[i].cmd);
        }
        free(tests);
    }
    
    /* Cleanup */
    printf("\nCleaning up temp directory: %s\n", temp_dir);
    cleanup_temp_dir(temp_dir);
    free(temp_dir);
    
    printf("\n=== Test Complete ===\n");
    printf("To collect coverage data for gcov-tool.cc:\n");
    printf("1. Ensure gcov-tool was built with --enable-coverage\n");
    printf("2. Run: gcov gcov-tool.cc\n");
    printf("3. Check gcov-tool.cc.gcov for coverage of lines 534-554\n");
    
    return ret;
}
