/**
 * Test harness for gcov-tool overlap command-line parsing coverage.
 * This program creates multiple test scenarios, generates coverage data,
 * and invokes gcov-tool overlap with various flag combinations to cover
 * the switch statement cases in gcov-tool.cc lines 534-554.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH 1024
#define NUM_TEST_SCENARIOS 4

/* Function prototypes */
int compile_with_coverage(const char *source, const char *output);
int run_program(const char *program, const char *args);
int invoke_gcov_tool(const char *gcda, const char *gcno, const char *flags);
void cleanup_files(const char *base_name);
int file_exists(const char *path);

/* Test scenario source codes */
const char *scenario_a_src = 
"#include <stdio.h>\n"
"int func1(int x) {\n"
"    if (x > 0) return x * 2;\n"
"    else return x - 1;\n"
"}\n"
"void func2() {\n"
"    printf(\"Hello from func2\\n\");\n"
"}\n"
"int main() {\n"
"    func1(5);\n"
"    func1(-3);\n"
"    func2();\n"
"    return 0;\n"
"}\n";

const char *scenario_b_src = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"int process_loop(int iterations) {\n"
"    int sum = 0;\n"
"    for (int i = 0; i < iterations; i++) {\n"
"        for (int j = 0; j < i; j++) {\n"
"            sum += j;\n"
"        }\n"
"    }\n"
"    return sum;\n"
"}\n"
"int main(int argc, char **argv) {\n"
"    int iterations = 10;\n"
"    if (argc > 1) iterations = atoi(argv[1]);\n"
"    \n"
"    int result1 = process_loop(iterations);\n"
"    int result2 = process_loop(iterations / 2);\n"
"    \n"
"    printf(\"Results: %d, %d\\n\", result1, result2);\n"
"    return 0;\n"
"}\n";

const char *scenario_c1_src = 
"#include \"scenario_c.h\"\n"
"int multiply(int a, int b) {\n"
"    return a * b;\n"
"}\n"
"float divide(float a, float b) {\n"
"    if (b != 0.0f) return a / b;\n"
"    return 0.0f;\n"
"}\n";

const char *scenario_c2_src = 
"#include \"scenario_c.h\"\n"
"#include <stdio.h>\n"
"int main() {\n"
"    int prod = multiply(7, 8);\n"
"    float quot = divide(15.0f, 3.0f);\n"
"    printf(\"Product: %d, Quotient: %.2f\\n\", prod, quot);\n"
"    return 0;\n"
"}\n";

const char *scenario_c_header = 
"#ifndef SCENARIO_C_H\n"
"#define SCENARIO_C_H\n"
"int multiply(int a, int b);\n"
"float divide(float a, float b);\n"
"#endif\n";

const char *scenario_d_src = 
"#include <stdio.h>\n"
"int unused_function(int x) {\n"
"    if (x > 100) return x * 10;\n"
"    return x;\n"
"}\n"
"int main() {\n"
"    /* This main does nothing that triggers coverage */\n"
"    printf(\"Program executed but no instrumented code paths taken\\n\");\n"
"    return 0;\n"
"}\n";

/* Test cases for gcov-tool overlap invocations */
typedef struct {
    const char *description;
    const char *flags;
    int expect_success;
} gcov_test_case;

gcov_test_case test_cases[] = {
    {"Test verbose flag (-v)", "-v", 1},
    {"Test function level overlap (-f)", "-f", 1},
    {"Test fullname flag (-F)", "-F", 1},
    {"Test object level overlap (-o)", "-o", 1},
    {"Test hot only flag (-h)", "-h", 1},
    {"Test threshold flag (-t 0.5)", "-t 0.5", 1},
    {"Test threshold flag with different value (-t 0.75)", "-t 0.75", 1},
    {"Test threshold flag with high value (-t 0.9)", "-t 0.9", 1},
    {"Test combination of flags (-v -f -o)", "-v -f -o", 1},
    {"Test another combination (-F -h -t 0.6)", "-F -h -t 0.6", 1},
    {"Test invalid flag to trigger default case (-z)", "-z", 0},
    {"Test multiple files with verbose", "-v", 1},
    {NULL, NULL, 0}
};

int main(int argc, char **argv) {
    printf("=== Starting gcov-tool overlap command-line parsing coverage test ===\n\n");
    
    /* Check if gcov-tool exists */
    printf("Checking for gcov-tool...\n");
    if (system("which gcov-tool > /dev/null 2>&1") != 0) {
        /* Try in current directory */
        if (!file_exists("./gcov-tool")) {
            fprintf(stderr, "Error: gcov-tool not found in PATH or current directory.\n");
            fprintf(stderr, "Please build gcov-tool with coverage flags first:\n");
            fprintf(stderr, "  g++ -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
            return 1;
        }
    }
    
    /* Create test directory */
    if (system("mkdir -p gcov_test_data") != 0) {
        perror("Failed to create test directory");
        return 1;
    }
    
    /* Change to test directory */
    if (chdir("gcov_test_data") != 0) {
        perror("Failed to change to test directory");
        return 1;
    }
    
    /* Scenario A: Simple function */
    printf("\n--- Scenario A: Simple function with branches ---\n");
    FILE *fp = fopen("scenario_a.c", "w");
    if (!fp) {
        perror("Failed to create scenario_a.c");
        return 1;
    }
    fputs(scenario_a_src, fp);
    fclose(fp);
    
    if (!compile_with_coverage("scenario_a.c", "scenario_a")) {
        fprintf(stderr, "Failed to compile scenario A\n");
        return 1;
    }
    
    if (!run_program("./scenario_a", NULL)) {
        fprintf(stderr, "Failed to run scenario A\n");
        return 1;
    }
    
    /* Scenario B: Loop heavy */
    printf("\n--- Scenario B: Loop heavy program ---\n");
    fp = fopen("scenario_b.c", "w");
    if (!fp) {
        perror("Failed to create scenario_b.c");
        return 1;
    }
    fputs(scenario_b_src, fp);
    fclose(fp);
    
    if (!compile_with_coverage("scenario_b.c", "scenario_b")) {
        fprintf(stderr, "Failed to compile scenario B\n");
        return 1;
    }
    
    /* Run multiple times with different arguments */
    run_program("./scenario_b", "5");
    run_program("./scenario_b", "20");
    run_program("./scenario_b", "8");
    
    /* Scenario C: Multiple source files */
    printf("\n--- Scenario C: Multiple source files ---\n");
    fp = fopen("scenario_c.h", "w");
    if (!fp) {
        perror("Failed to create scenario_c.h");
        return 1;
    }
    fputs(scenario_c_header, fp);
    fclose(fp);
    
    fp = fopen("scenario_c1.c", "w");
    if (!fp) {
        perror("Failed to create scenario_c1.c");
        return 1;
    }
    fputs(scenario_c1_src, fp);
    fclose(fp);
    
    fp = fopen("scenario_c2.c", "w");
    if (!fp) {
        perror("Failed to create scenario_c2.c");
        return 1;
    }
    fputs(scenario_c2_src, fp);
    fclose(fp);
    
    /* Compile multiple files together */
    char cmd[MAX_PATH];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage scenario_c1.c scenario_c2.c -o scenario_c");
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile scenario C\n");
        return 1;
    }
    
    run_program("./scenario_c", NULL);
    
    /* Scenario D: Empty/zero counts */
    printf("\n--- Scenario D: Program with zero coverage counts ---\n");
    fp = fopen("scenario_d.c", "w");
    if (!fp) {
        perror("Failed to create scenario_d.c");
        return 1;
    }
    fputs(scenario_d_src, fp);
    fclose(fp);
    
    if (!compile_with_coverage("scenario_d.c", "scenario_d")) {
        fprintf(stderr, "Failed to compile scenario D\n");
        return 1;
    }
    
    run_program("./scenario_d", NULL);
    
    /* Now invoke gcov-tool overlap with various flags */
    printf("\n=== Invoking gcov-tool overlap with various flags ===\n\n");
    
    /* Test with each scenario's coverage data */
    const char *scenarios[] = {"scenario_a", "scenario_b", "scenario_c", "scenario_d", NULL};
    
    for (int i = 0; scenarios[i] != NULL; i++) {
        char gcda_file[MAX_PATH], gcno_file[MAX_PATH];
        snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", scenarios[i]);
        snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", scenarios[i]);
        
        if (file_exists(gcda_file) && file_exists(gcno_file)) {
            printf("\n--- Testing with %s coverage data ---\n", scenarios[i]);
            
            /* Run a subset of test cases for each scenario */
            for (int j = 0; test_cases[j].description != NULL; j++) {
                /* Only run certain tests for each scenario to keep it manageable */
                if ((j % (i + 2)) == 0 || j == 10) {  /* Include the invalid flag test (j=10) */
                    printf("  %s\n", test_cases[j].description);
                    
                    /* For multiple files test, use two different gcda files */
                    if (strstr(test_cases[j].description, "multiple files")) {
                        char cmd[MAX_PATH * 2];
                        if (i == 0 && scenarios[1] != NULL) {
                            /* Use scenario_a and scenario_b gcda files */
                            snprintf(cmd, sizeof(cmd),
                                     "gcov-tool overlap %s %s.gcda %s.gcda 2>&1",
                                     test_cases[j].flags, scenarios[0], scenarios[1]);
                            system(cmd);
                        }
                    } else {
                        invoke_gcov_tool(gcda_file, gcno_file, test_cases[j].flags);
                    }
                }
            }
        }
    }
    
    /* Additional targeted tests to ensure all cases are hit */
    printf("\n=== Targeted tests for specific uncovered lines ===\n\n");
    
    /* Test -v flag specifically */
    printf("1. Testing -v flag (verbose mode):\n");
    invoke_gcov_tool("scenario_a.gcda", "scenario_a.gcno", "-v");
    
    /* Test -f flag specifically */
    printf("\n2. Testing -f flag (function level):\n");
    invoke_gcov_tool("scenario_b.gcda", "scenario_b.gcno", "-f");
    
    /* Test -F flag specifically */
    printf("\n3. Testing -F flag (fullname):\n");
    invoke_gcov_tool("scenario_c.gcda", "scenario_c.gcno", "-F");
    
    /* Test -o flag specifically */
    printf("\n4. Testing -o flag (object level):\n");
    invoke_gcov_tool("scenario_a.gcda", "scenario_a.gcno", "-o");
    
    /* Test -h flag specifically */
    printf("\n5. Testing -h flag (hot only):\n");
    invoke_gcov_tool("scenario_b.gcda", "scenario_b.gcno", "-h");
    
    /* Test -t flag with various values */
    printf("\n6. Testing -t flag with value 0.5:\n");
    invoke_gcov_tool("scenario_a.gcda", "scenario_a.gcno", "-t 0.5");
    
    printf("\n7. Testing -t flag with value 0.75:\n");
    invoke_gcov_tool("scenario_b.gcda", "scenario_b.gcno", "-t 0.75");
    
    printf("\n8. Testing -t flag with value 0.9:\n");
    invoke_gcov_tool("scenario_d.gcda", "scenario_d.gcno", "-t 0.9");
    
    /* Test combination of flags */
    printf("\n9. Testing combination -v -f -o:\n");
    invoke_gcov_tool("scenario_a.gcda", "scenario_a.gcno", "-v -f -o");
    
    printf("\n10. Testing combination -F -h -t 0.6:\n");
    invoke_gcov_tool("scenario_b.gcda", "scenario_b.gcno", "-F -h -t 0.6");
    
    /* Test invalid flag to trigger default case and overlap_usage() */
    printf("\n11. Testing invalid flag -z (should trigger default case):\n");
    system("gcov-tool overlap -z 2>&1 | head -20");
    
    /* Test with two input files */
    printf("\n12. Testing with two gcda files and verbose flag:\n");
    system("gcov-tool overlap -v scenario_a.gcda scenario_b.gcda 2>&1 | head -30");
    
    /* Cleanup */
    printf("\n=== Cleaning up test files ===\n");
    chdir("..");
    system("rm -rf gcov_test_data");
    
    printf("\n=== Test completed ===\n");
    printf("The gcov-tool overlap command-line parser should now have executed\n");
    printf("all code paths in the target switch statement (lines 534-554).\n");
    
    return 0;
}

/* Helper function implementations */
int compile_with_coverage(const char *source, const char *output) {
    char cmd[MAX_PATH * 2];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s",
             source, output);
    printf("  Compiling: %s\n", cmd);
    return system(cmd) == 0;
}

int run_program(const char *program, const char *args) {
    char cmd[MAX_PATH * 2];
    if (args) {
        snprintf(cmd, sizeof(cmd), "%s %s", program, args);
    } else {
        snprintf(cmd, sizeof(cmd), "%s", program);
    }
    printf("  Running: %s\n", cmd);
    return system(cmd) == 0;
}

int invoke_gcov_tool(const char *gcda, const char *gcno, const char *flags) {
    char cmd[MAX_PATH * 3];
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap %s %s %s 2>&1", flags, gcda, gcno);
    
    /* For invalid flags, we expect failure */
    if (strstr(flags, "-z")) {
        /* Let the output go to stderr to see usage message */
        return system(cmd);
    }
    
    /* For valid commands, capture and limit output */
    char full_cmd[MAX_PATH * 4];
    snprintf(full_cmd, sizeof(full_cmd), "%s | head -5", cmd);
    return system(full_cmd);
}

int file_exists(const char *path) {
    return access(path, F_OK) == 0;
}
