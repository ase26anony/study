/**
 * test_gcov_tool_overlap.c
 * 
 * Test driver to exercise the uncovered switch cases in gcov-tool.cc
 * Specifically targets the parse_overlap_options function (lines 534-554)
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
#define TEMP_DIR_PREFIX "/tmp/gcov_test_XXXXXX"

/* Global variables to track test results */
typedef struct {
    char *description;
    char *command;
    int expected_exit;
    int actual_exit;
    int passed;
} test_case_t;

/* Function prototypes */
int create_temp_dir(char *template);
int compile_instrumented_program(const char *dir, const char *prog_name);
int generate_gcda_files(const char *dir, const char *prog_name, int count);
int run_gcov_tool(const char *command, int *exit_status);
void run_test_suite(const char *gcov_tool_path, const char *temp_dir, 
                    const char *prog_name, test_case_t *tests, int num_tests);
void cleanup(const char *dir);
void print_summary(test_case_t *tests, int num_tests);

int main(int argc, char *argv[]) {
    char temp_dir[256];
    char gcov_tool_path[256] = "./gcov-tool";
    char prog_name[] = "test_prog";
    int num_tests = 0;
    
    /* Use provided gcov-tool path if given */
    if (argc > 1) {
        strncpy(gcov_tool_path, argv[1], sizeof(gcov_tool_path) - 1);
    }
    
    printf("=== GCOV-TOOL Overlap Parser Test Suite ===\n");
    printf("Testing gcov-tool at: %s\n\n", gcov_tool_path);
    
    /* Create temporary directory */
    strcpy(temp_dir, TEMP_DIR_PREFIX);
    if (create_temp_dir(temp_dir) != 0) {
        fprintf(stderr, "Failed to create temp directory\n");
        return 1;
    }
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Compile instrumented test program */
    if (compile_instrumented_program(temp_dir, prog_name) != 0) {
        cleanup(temp_dir);
        return 1;
    }
    
    /* Generate multiple .gcda files for overlap analysis */
    if (generate_gcda_files(temp_dir, prog_name, 3) != 0) {
        cleanup(temp_dir);
        return 1;
    }
    
    /* Construct file paths for test commands */
    char gcda1[512], gcda2[512], gcda3[512];
    snprintf(gcda1, sizeof(gcda1), "%s/%s.gcda", temp_dir, prog_name);
    snprintf(gcda2, sizeof(gcda2), "%s/%s_2.gcda", temp_dir, prog_name);
    snprintf(gcda3, sizeof(gcda3), "%s/%s_3.gcda", temp_dir, prog_name);
    
    /* Define test cases targeting the uncovered switch cases */
    test_case_t tests[] = {
        /* Basic flag tests - each flag individually */
        {"Single -v flag", "", 0, 0, 0},
        {"Single -f flag", "", 0, 0, 0},
        {"Single -F flag", "", 0, 0, 0},
        {"Single -o flag", "", 0, 0, 0},
        {"Single -h flag", "", 0, 0, 0},
        {"Single -t flag with value", "", 0, 0, 0},
        
        /* Combined flags - all uncovered flags together */
        {"All flags combined", "", 0, 0, 0},
        
        /* Flag permutations - different orders */
        {"Flags in reverse order", "", 0, 0, 0},
        {"Flags mixed order 1", "", 0, 0, 0},
        {"Flags mixed order 2", "", 0, 0, 0},
        
        /* Edge cases */
        {"Repeated -v flag", "", 0, 0, 0},
        {"-t with high threshold", "", 0, 0, 0},
        {"-t with low threshold", "", 0, 0, 0},
        {"-t with integer threshold", "", 0, 0, 0},
        
        /* Error cases */
        {"-t without argument (should fail)", "", 1, 0, 0},
        {"-t with invalid argument (should fail)", "", 1, 0, 0},
        {"Unknown flag -x (should fail)", "", 1, 0, 0},
        
        /* With different numbers of input files */
        {"Flags with single input file", "", 0, 0, 0},
        {"Flags with multiple input files", "", 0, 0, 0},
        
        /* With absolute paths */
        {"Flags with absolute paths", "", 0, 0, 0},
    };
    
    num_tests = sizeof(tests) / sizeof(tests[0]);
    
    /* Fill in the command strings dynamically */
    char abs_gcda1[512];
    realpath(gcda1, abs_gcda1);
    
    /* Test 0: Single -v flag */
    snprintf(tests[0].command, MAX_CMD_LEN, "%s overlap -v %s %s", 
             gcov_tool_path, gcda1, gcda2);
    tests[0].expected_exit = 0;
    
    /* Test 1: Single -f flag */
    snprintf(tests[1].command, MAX_CMD_LEN, "%s overlap -f %s %s", 
             gcov_tool_path, gcda1, gcda2);
    tests[1].expected_exit = 0;
    
    /* Test 2: Single -F flag */
    snprintf(tests[2].command, MAX_CMD_LEN, "%s overlap -F %s %s", 
             gcov_tool_path, gcda1, gcda2);
    tests[2].expected_exit = 0;
    
    /* Test 3: Single -o flag */
    snprintf(tests[3].command, MAX_CMD_LEN, "%s overlap -o %s %s", 
             gcov_tool_path, gcda1, gcda2);
    tests[3].expected_exit = 0;
    
    /* Test 4: Single -h flag */
    snprintf(tests[4].command, MAX_CMD_LEN, "%s overlap -h %s %s", 
             gcov_tool_path, gcda1, gcda2);
    tests[4].expected_exit = 0;
    
    /* Test 5: Single -t flag with value */
    snprintf(tests[5].command, MAX_CMD_LEN, "%s overlap -t 0.75 %s %s", 
             gcov_tool_path, gcda1, gcda2);
    tests[5].expected_exit = 0;
    
    /* Test 6: All flags combined */
    snprintf(tests[6].command, MAX_CMD_LEN, "%s overlap -v -f -F -o -h -t 0.5 %s %s", 
             gcov_tool_path, gcda1, gcda2);
    tests[6].expected_exit = 0;
    
    /* Test 7: Flags in reverse order */
    snprintf(tests[7].command, MAX_CMD_LEN, "%s overlap -t 0.3 -h -o -F -f -v %s %s", 
             gcov_tool_path, gcda1, gcda2);
    tests[7].expected_exit = 0;
    
    /* Test 8: Flags mixed order 1 */
    snprintf(tests[8].command, MAX_CMD_LEN, "%s overlap -f -v -o -t 0.8 -F -h %s %s", 
             gcov_tool_path, gcda1, gcda2);
    tests[8].expected_exit = 0;
    
    /* Test 9: Flags mixed order 2 */
    snprintf(tests[9].command, MAX_CMD_LEN, "%s overlap -F -h -t 0.25 -v -f -o %s %s", 
             gcov_tool_path, gcda1, gcda2);
    tests[9].expected_exit = 0;
    
    /* Test 10: Repeated -v flag */
    snprintf(tests[10].command, MAX_CMD_LEN, "%s overlap -v -v -v %s %s", 
             gcov_tool_path, gcda1, gcda2);
    tests[10].expected_exit = 0;
    
    /* Test 11: -t with high threshold */
    snprintf(tests[11].command, MAX_CMD_LEN, "%s overlap -t 99.9 %s %s", 
             gcov_tool_path, gcda1, gcda2);
    tests[11].expected_exit = 0;
    
    /* Test 12: -t with low threshold */
    snprintf(tests[12].command, MAX_CMD_LEN, "%s overlap -t 0.001 %s %s", 
             gcov_tool_path, gcda1, gcda2);
    tests[12].expected_exit = 0;
    
    /* Test 13: -t with integer threshold */
    snprintf(tests[13].command, MAX_CMD_LEN, "%s overlap -t 1 %s %s", 
             gcov_tool_path, gcda1, gcda2);
    tests[13].expected_exit = 0;
    
    /* Test 14: -t without argument (should trigger error) */
    snprintf(tests[14].command, MAX_CMD_LEN, "%s overlap -t %s %s", 
             gcov_tool_path, gcda1, gcda2);
    tests[14].expected_exit = 1;
    
    /* Test 15: -t with invalid argument (should trigger error) */
    snprintf(tests[15].command, MAX_CMD_LEN, "%s overlap -t not_a_number %s %s", 
             gcov_tool_path, gcda1, gcda2);
    tests[15].expected_exit = 1;
    
    /* Test 16: Unknown flag -x (should trigger default case) */
    snprintf(tests[16].command, MAX_CMD_LEN, "%s overlap -x %s %s", 
             gcov_tool_path, gcda1, gcda2);
    tests[16].expected_exit = 1;
    
    /* Test 17: Flags with single input file */
    snprintf(tests[17].command, MAX_CMD_LEN, "%s overlap -v -f -F %s", 
             gcov_tool_path, gcda1);
    tests[17].expected_exit = 0;
    
    /* Test 18: Flags with multiple input files */
    snprintf(tests[18].command, MAX_CMD_LEN, "%s overlap -v -f -F %s %s %s", 
             gcov_tool_path, gcda1, gcda2, gcda3);
    tests[18].expected_exit = 0;
    
    /* Test 19: Flags with absolute paths */
    snprintf(tests[19].command, MAX_CMD_LEN, "%s overlap -v -f -F %s %s", 
             gcov_tool_path, abs_gcda1, gcda2);
    tests[19].expected_exit = 0;
    
    /* Allocate memory for command strings */
    for (int i = 0; i < num_tests; i++) {
        tests[i].command = malloc(MAX_CMD_LEN);
        if (!tests[i].command) {
            fprintf(stderr, "Memory allocation failed\n");
            cleanup(temp_dir);
            return 1;
        }
    }
    
    /* Run the test suite */
    run_test_suite(gcov_tool_path, temp_dir, prog_name, tests, num_tests);
    
    /* Print summary */
    print_summary(tests, num_tests);
    
    /* Cleanup */
    cleanup(temp_dir);
    
    for (int i = 0; i < num_tests; i++) {
        free(tests[i].command);
    }
    
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

int compile_instrumented_program(const char *dir, const char *prog_name) {
    char src_path[512];
    char exe_path[512];
    char cmd[1024];
    
    snprintf(src_path, sizeof(src_path), "%s/%s.c", dir, prog_name);
    snprintf(exe_path, sizeof(exe_path), "%s/%s", dir, prog_name);
    
    /* Create a simple C program for testing */
    FILE *src = fopen(src_path, "w");
    if (!src) {
        perror("Failed to create source file");
        return -1;
    }
    
    fprintf(src, "#include <stdio.h>\n\n");
    fprintf(src, "int func1(int x) {\n");
    fprintf(src, "    if (x > 0) {\n");
    fprintf(src, "        return x * 2;\n");
    fprintf(src, "    } else {\n");
    fprintf(src, "        return x / 2;\n");
    fprintf(src, "    }\n");
    fprintf(src, "}\n\n");
    fprintf(src, "int func2(int y) {\n");
    fprintf(src, "    for (int i = 0; i < y; i++) {\n");
    fprintf(src, "        printf(\"%%d \", i);\n");
    fprintf(src, "    }\n");
    fprintf(src, "    return y * y;\n");
    fprintf(src, "}\n\n");
    fprintf(src, "int main() {\n");
    fprintf(src, "    int result1 = func1(10);\n");
    fprintf(src, "    int result2 = func2(5);\n");
    fprintf(src, "    printf(\"Results: %%d, %%d\\n\", result1, result2);\n");
    fprintf(src, "    return 0;\n");
    fprintf(src, "}\n");
    
    fclose(src);
    
    /* Compile with GCOV instrumentation */
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s 2>&1", 
             exe_path, src_path);
    
    printf("Compiling instrumented program...\n");
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Compilation failed\n");
        return -1;
    }
    
    return 0;
}

int generate_gcda_files(const char *dir, const char *prog_name, int count) {
    char exe_path[512];
    char gcda_path[512];
    char cmd[1024];
    
    snprintf(exe_path, sizeof(exe_path), "%s/%s", dir, prog_name);
    
    /* Run the program multiple times with different arguments 
       to generate different profile data */
    for (int i = 0; i < count; i++) {
        printf("Generating gcda file %d...\n", i + 1);
        
        /* Run the program */
        snprintf(cmd, sizeof(cmd), "cd %s && ./%s > /dev/null 2>&1", dir, prog_name);
        system(cmd);
        
        /* Rename gcda file for multiple profiles */
        if (i > 0) {
            snprintf(gcda_path, sizeof(gcda_path), "%s/%s.gcda", dir, prog_name);
            char new_gcda_path[512];
            snprintf(new_gcda_path, sizeof(new_gcda_path), "%s/%s_%d.gcda", dir, prog_name, i + 1);
            rename(gcda_path, new_gcda_path);
        }
    }
    
    return 0;
}

int run_gcov_tool(const char *command, int *exit_status) {
    printf("Running: %s\n", command);
    
    int status = system(command);
    
    if (WIFEXITED(status)) {
        *exit_status = WEXITSTATUS(status);
        return 0;
    } else {
        *exit_status = -1;
        return -1;
    }
}

void run_test_suite(const char *gcov_tool_path, const char *temp_dir, 
                    const char *prog_name, test_case_t *tests, int num_tests) {
    printf("\n=== Running Test Suite ===\n");
    
    for (int i = 0; i < num_tests; i++) {
        printf("\nTest %d: %s\n", i + 1, tests[i].description);
        printf("Command: %s\n", tests[i].command);
        
        int exit_status;
        if (run_gcov_tool(tests[i].command, &exit_status) == 0) {
            tests[i].actual_exit = exit_status;
            tests[i].passed = (exit_status == tests[i].expected_exit);
            
            if (tests[i].passed) {
                printf("✓ PASSED (exit: %d)\n", exit_status);
            } else {
                printf("✗ FAILED (expected: %d, got: %d)\n", 
                       tests[i].expected_exit, exit_status);
            }
        } else {
            tests[i].passed = 0;
            printf("✗ FAILED (system call error)\n");
        }
    }
}

void cleanup(const char *dir) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dir);
    system(cmd);
    printf("\nCleaned up temp directory: %s\n", dir);
}

void print_summary(test_case_t *tests, int num_tests) {
    printf("\n=== Test Summary ===\n");
    
    int passed = 0;
    for (int i = 0; i < num_tests; i++) {
        if (tests[i].passed) {
            passed++;
        }
    }
    
    printf("Passed: %d/%d (%.1f%%)\n", passed, num_tests, 
           (float)passed / num_tests * 100);
    
    if (passed < num_tests) {
        printf("\nFailed tests:\n");
        for (int i = 0; i < num_tests; i++) {
            if (!tests[i].passed) {
                printf("  %d. %s\n", i + 1, tests[i].description);
                printf("     Command: %s\n", tests[i].command);
                printf("     Expected exit: %d, Got: %d\n", 
                       tests[i].expected_exit, tests[i].actual_exit);
            }
        }
    }
    
    printf("\n=== Lines Targeted ===\n");
    printf("The following switch cases in parse_overlap_options were exercised:\n");
    printf("  case 'v': verbose = true; gcov_set_verbose();\n");
    printf("  case 'f': overlap_func_level = 1;\n");
    printf("  case 'F': overlap_use_fullname = 1;\n");
    printf("  case 'o': overlap_obj_level = 1;\n");
    printf("  case 'h': overlap_hot_only = 1;\n");
    printf("  case 't': overlap_hot_threshold = atof(optarg);\n");
    printf("  default: overlap_usage();\n");
}
