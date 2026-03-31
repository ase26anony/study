/**
 * Test harness for gcov-tool overlap command-line parsing coverage.
 * This program creates multiple test scenarios, generates coverage data,
 * and invokes gcov-tool with various flag combinations to cover lines 534-554
 * in gcov-tool.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH 1024
#define NUM_SCENARIOS 4

/* Function prototypes */
int compile_with_coverage(const char *source, const char *output);
int run_executable(const char *executable, const char *args);
int invoke_gcov_tool(const char *args, const char *gcda_file, const char *gcno_file);
void cleanup_files(const char *base_name);
void create_test_scenarios(void);

/* Test scenario source codes */
const char *scenario_a = 
"#include <stdio.h>\n"
"int func1(int x) {\n"
"    if (x > 0) return x * 2;\n"
"    else return x - 5;\n"
"}\n"
"void func2(void) {\n"
"    for (int i = 0; i < 10; i++) {\n"
"        printf(\"Loop: %d\\n\", i);\n"
"    }\n"
"}\n"
"int main(void) {\n"
"    func1(10);\n"
"    func1(-5);\n"
"    func2();\n"
"    return 0;\n"
"}\n";

const char *scenario_b = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"void nested_loops(int depth) {\n"
"    int count = 0;\n"
"    for (int i = 0; i < depth; i++) {\n"
"        for (int j = 0; j < depth; j++) {\n"
"            for (int k = 0; k < depth; k++) {\n"
"                count++;\n"
"            }\n"
"        }\n"
"    }\n"
"    printf(\"Count: %d\\n\", count);\n"
"}\n"
"int main(int argc, char **argv) {\n"
"    int depth = 3;\n"
"    if (argc > 1) depth = atoi(argv[1]);\n"
"    nested_loops(depth);\n"
"    nested_loops(depth + 1);\n"
"    return 0;\n"
"}\n";

const char *scenario_c1 = 
"#include <stdio.h>\n"
"#include \"test_header.h\"\n"
"void file1_func(void) {\n"
"    printf(\"File 1 function\\n\");\n"
"    shared_func();\n"
"}\n"
"int main(void) {\n"
"    file1_func();\n"
"    file2_func();\n"
"    return 0;\n"
"}\n";

const char *scenario_c2 = 
"#include <stdio.h>\n"
"#include \"test_header.h\"\n"
"void file2_func(void) {\n"
"    printf(\"File 2 function\\n\");\n"
"    shared_func();\n"
"}\n"
"void shared_func(void) {\n"
"    printf(\"Shared function\\n\");\n"
"}\n";

const char *scenario_c_header = 
"#ifndef TEST_HEADER_H\n"
"#define TEST_HEADER_H\n"
"void file2_func(void);\n"
"void shared_func(void);\n"
"#endif\n";

const char *scenario_d = 
"#include <stdio.h>\n"
"int never_called(void) {\n"
"    return 42;\n"
"}\n"
"int main(void) {\n"
"    /* Don't call any instrumented functions */\n"
"    printf(\"No coverage generated\\n\");\n"
"    return 0;\n"
"}\n";

/* Test cases for gcov-tool invocations */
typedef struct {
    const char *args;
    const char *description;
    int expect_success;
} gcov_test_case;

gcov_test_case test_cases[] = {
    {"-v", "Verbose flag", 1},
    {"-f", "Function level overlap", 1},
    {"-F", "Use fullname", 1},
    {"-o", "Object level overlap", 1},
    {"-h", "Hot only", 1},
    {"-t 0.5", "Hot threshold 0.5", 1},
    {"-t 0.75", "Hot threshold 0.75", 1},
    {"-t 1.0", "Hot threshold 1.0", 1},
    {"-f -o -v", "Combination of flags", 1},
    {"-F -h -t 0.3", "Another combination", 1},
    {"-z", "Invalid flag (triggers default case)", 0},
    {"-v -f -o -F -h -t 0.6", "All valid flags", 1},
    {"", "No flags (just overlap)", 1},
    {NULL, NULL, 0}
};

int main(void) {
    printf("=== Starting gcov-tool overlap coverage test ===\n\n");
    
    /* Check if gcov-tool exists */
    if (system("which gcov-tool > /dev/null 2>&1") != 0) {
        fprintf(stderr, "Error: gcov-tool not found in PATH\n");
        fprintf(stderr, "Please ensure gcov-tool is built and in your PATH\n");
        return 1;
    }
    
    /* Create test scenarios */
    create_test_scenarios();
    
    /* Test each scenario with various gcov-tool invocations */
    for (int i = 0; i < NUM_SCENARIOS; i++) {
        char gcda_file[MAX_PATH];
        char gcno_file[MAX_PATH];
        char exec_file[MAX_PATH];
        
        snprintf(exec_file, sizeof(exec_file), "test_scenario_%d", i);
        snprintf(gcda_file, sizeof(gcda_file), "test_scenario_%d.gcda", i);
        snprintf(gcno_file, sizeof(gcno_file), "test_scenario_%d.gcno", i);
        
        printf("\n--- Testing Scenario %d ---\n", i);
        
        /* Run gcov-tool with various arguments */
        gcov_test_case *test = test_cases;
        while (test->args != NULL) {
            printf("  Testing: %s (%s)\n", test->args, test->description);
            
            /* Invoke gcov-tool with current test case */
            int result = invoke_gcov_tool(test->args, gcda_file, gcno_file);
            
            if (test->expect_success && result != 0) {
                printf("    WARNING: Expected success but got exit code %d\n", result);
            } else if (!test->expect_success && result == 0) {
                printf("    WARNING: Expected failure but got success\n");
            } else {
                printf("    OK\n");
            }
            
            test++;
        }
    }
    
    /* Cleanup */
    printf("\n=== Cleaning up test files ===\n");
    for (int i = 0; i < NUM_SCENARIOS; i++) {
        char base[MAX_PATH];
        snprintf(base, sizeof(base), "test_scenario_%d", i);
        cleanup_files(base);
    }
    
    /* Cleanup multi-file scenario */
    system("rm -f test_scenario_2_*.c test_scenario_2_*.gcda test_scenario_2_*.gcno test_header.h");
    
    printf("\n=== Test completed ===\n");
    printf("The following gcov-tool overlap flags were tested:\n");
    printf("  -v (verbose) - triggers gcov_set_verbose()\n");
    printf("  -f (function level) - sets overlap_func_level\n");
    printf("  -F (fullname) - sets overlap_use_fullname\n");
    printf("  -o (object level) - sets overlap_obj_level\n");
    printf("  -h (hot only) - sets overlap_hot_only\n");
    printf("  -t <value> (threshold) - sets overlap_hot_threshold\n");
    printf("  -z (invalid) - triggers default case and overlap_usage()\n");
    
    return 0;
}

int compile_with_coverage(const char *source, const char *output) {
    char cmd[MAX_PATH * 2];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage -o %s %s 2>&1",
             output, source);
    return system(cmd);
}

int run_executable(const char *executable, const char *args) {
    char cmd[MAX_PATH * 2];
    if (args && args[0]) {
        snprintf(cmd, sizeof(cmd), "./%s %s > /dev/null 2>&1", executable, args);
    } else {
        snprintf(cmd, sizeof(cmd), "./%s > /dev/null 2>&1", executable);
    }
    return system(cmd);
}

int invoke_gcov_tool(const char *args, const char *gcda_file, const char *gcno_file) {
    char cmd[MAX_PATH * 4];
    
    /* For invalid flag test, redirect stderr to suppress usage output */
    if (strstr(args, "-z") != NULL) {
        snprintf(cmd, sizeof(cmd), 
                 "gcov-tool overlap %s %s %s 2>/dev/null", 
                 args, gcda_file, gcno_file);
    } else {
        snprintf(cmd, sizeof(cmd), 
                 "gcov-tool overlap %s %s %s > /dev/null 2>&1", 
                 args, gcda_file, gcno_file);
    }
    
    return system(cmd);
}

void cleanup_files(const char *base_name) {
    char cmd[MAX_PATH * 2];
    snprintf(cmd, sizeof(cmd), 
             "rm -f %s %s.gcda %s.gcno %s.c", 
             base_name, base_name, base_name, base_name);
    system(cmd);
}

void create_test_scenarios(void) {
    printf("Creating test scenarios...\n");
    
    /* Scenario A: Simple function */
    printf("  Creating Scenario A (simple function)...\n");
    FILE *fp = fopen("test_scenario_0.c", "w");
    if (fp) {
        fputs(scenario_a, fp);
        fclose(fp);
        compile_with_coverage("test_scenario_0.c", "test_scenario_0");
        run_executable("test_scenario_0", "");
    }
    
    /* Scenario B: Loop heavy with different runs */
    printf("  Creating Scenario B (loop heavy)...\n");
    fp = fopen("test_scenario_1.c", "w");
    if (fp) {
        fputs(scenario_b, fp);
        fclose(fp);
        compile_with_coverage("test_scenario_1.c", "test_scenario_1");
        run_executable("test_scenario_1", "2");
        run_executable("test_scenario_1", "3");
        run_executable("test_scenario_1", "4");
    }
    
    /* Scenario C: Multiple source files */
    printf("  Creating Scenario C (multiple files)...\n");
    fp = fopen("test_scenario_2_1.c", "w");
    if (fp) {
        fputs(scenario_c1, fp);
        fclose(fp);
    }
    fp = fopen("test_scenario_2_2.c", "w");
    if (fp) {
        fputs(scenario_c2, fp);
        fclose(fp);
    }
    fp = fopen("test_header.h", "w");
    if (fp) {
        fputs(scenario_c_header, fp);
        fclose(fp);
    }
    system("gcc -O0 -fprofile-arcs -ftest-coverage test_scenario_2_1.c test_scenario_2_2.c -o test_scenario_2");
    run_executable("test_scenario_2", "");
    
    /* Scenario D: Empty/zero counts */
    printf("  Creating Scenario D (zero counts)...\n");
    fp = fopen("test_scenario_3.c", "w");
    if (fp) {
        fputs(scenario_d, fp);
        fclose(fp);
        compile_with_coverage("test_scenario_3.c", "test_scenario_3");
        run_executable("test_scenario_3", "");
    }
}
