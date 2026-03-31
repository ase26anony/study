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

/* Function declarations */
int compile_with_coverage(const char *source, const char *output);
int run_executable(const char *executable, const char *args);
int invoke_gcov_tool(const char *args, const char *gcda_file, const char *gcno_file);
void create_test_scenarios(void);
void cleanup_files(void);

/* Test scenario source codes */
const char *scenario_a_src = 
"#include <stdio.h>\n"
"int func1(int x) {\n"
"    if (x > 0) return x * 2;\n"
"    else return x / 2;\n"
"}\n"
"void func2(void) {\n"
"    for (int i = 0; i < 5; i++) {\n"
"        printf(\"Loop iteration %d\\n\", i);\n"
"    }\n"
"}\n"
"int main(void) {\n"
"    func1(10);\n"
"    func1(-5);\n"
"    func2();\n"
"    return 0;\n"
"}\n";

const char *scenario_b_src = 
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
"    printf(\"Nested loops count: %d\\n\", count);\n"
"}\n"
"int main(int argc, char **argv) {\n"
"    int depth = 3;\n"
"    if (argc > 1) depth = atoi(argv[1]);\n"
"    nested_loops(depth);\n"
"    return 0;\n"
"}\n";

const char *scenario_c1_src = 
"#include <stdio.h>\n"
"#include \"scenario_c.h\"\n"
"void file1_func(void) {\n"
"    printf(\"Function from file 1\\n\");\n"
"    shared_function();\n"
"}\n"
"int main(void) {\n"
"    file1_func();\n"
"    file2_func();\n"
"    return 0;\n"
"}\n";

const char *scenario_c2_src = 
"#include <stdio.h>\n"
"#include \"scenario_c.h\"\n"
"void file2_func(void) {\n"
"    printf(\"Function from file 2\\n\");\n"
"    shared_function();\n"
"}\n"
"void shared_function(void) {\n"
"    printf(\"Shared function called\\n\");\n"
"}\n";

const char *scenario_c_header = 
"#ifndef SCENARIO_C_H\n"
"#define SCENARIO_C_H\n"
"void file1_func(void);\n"
"void file2_func(void);\n"
"void shared_function(void);\n"
"#endif\n";

const char *scenario_d_src = 
"#include <stdio.h>\n"
"int never_called_func(void) {\n"
"    return 42;\n"
"}\n"
"int main(void) {\n"
"    /* This main function does nothing that triggers coverage */\n"
"    /* The executable will be created but not run */\n"
"    return 0;\n"
"}\n";

/* Global arrays for generated files */
char gcda_files[NUM_TEST_SCENARIOS][MAX_PATH];
char gcno_files[NUM_TEST_SCENARIOS][MAX_PATH];
char exec_files[NUM_TEST_SCENARIOS][MAX_PATH];

int main(void) {
    printf("=== Starting gcov-tool overlap command-line parsing coverage test ===\n\n");
    
    /* Step 1: Create test scenarios and generate coverage data */
    create_test_scenarios();
    
    /* Step 2: Invoke gcov-tool overlap with various flag combinations */
    printf("\n=== Testing gcov-tool overlap with different flags ===\n\n");
    
    /* Test case 1: -v flag (verbose) */
    printf("Test 1: Testing -v flag (verbose mode)\n");
    invoke_gcov_tool("-v", gcda_files[0], gcno_files[0]);
    
    /* Test case 2: -f flag (function level overlap) */
    printf("\nTest 2: Testing -f flag (function level)\n");
    invoke_gcov_tool("-f", gcda_files[0], gcno_files[0]);
    
    /* Test case 3: -F flag (use fullname) */
    printf("\nTest 3: Testing -F flag (use fullname)\n");
    invoke_gcov_tool("-F", gcda_files[1], gcno_files[1]);
    
    /* Test case 4: -o flag (object level) */
    printf("\nTest 4: Testing -o flag (object level)\n");
    invoke_gcov_tool("-o", gcda_files[1], gcno_files[1]);
    
    /* Test case 5: -h flag (hot only) */
    printf("\nTest 5: Testing -h flag (hot only)\n");
    invoke_gcov_tool("-h", gcda_files[0], gcno_files[0]);
    
    /* Test case 6: -t flag with threshold value */
    printf("\nTest 6: Testing -t flag with threshold 0.5\n");
    invoke_gcov_tool("-t 0.5", gcda_files[0], gcno_files[0]);
    
    /* Test case 7: -t flag with different threshold */
    printf("\nTest 7: Testing -t flag with threshold 0.75\n");
    invoke_gcov_tool("-t 0.75", gcda_files[1], gcno_files[1]);
    
    /* Test case 8: Combination of flags */
    printf("\nTest 8: Testing combination -v -f -o\n");
    invoke_gcov_tool("-v -f -o", gcda_files[0], gcno_files[0]);
    
    /* Test case 9: Another combination */
    printf("\nTest 9: Testing combination -F -h -t 0.3\n");
    invoke_gcov_tool("-F -h -t 0.3", gcda_files[1], gcno_files[1]);
    
    /* Test case 10: Invalid option to trigger default case */
    printf("\nTest 10: Testing invalid option -z (should trigger default case)\n");
    invoke_gcov_tool("-z", gcda_files[0], gcno_files[0]);
    
    /* Test case 11: Multiple input files */
    printf("\nTest 11: Testing with multiple input files\n");
    char multi_args[MAX_PATH * 2];
    snprintf(multi_args, sizeof(multi_args), "-v %s %s %s %s", 
             gcda_files[0], gcno_files[0], gcda_files[1], gcno_files[1]);
    invoke_gcov_tool(multi_args, NULL, NULL);
    
    /* Test case 12: Empty/zero counts file */
    printf("\nTest 12: Testing with zero-count coverage data\n");
    invoke_gcov_tool("-t 0.1", gcda_files[3], gcno_files[3]);
    
    printf("\n=== All gcov-tool overlap tests completed ===\n");
    
    /* Cleanup */
    cleanup_files();
    
    return 0;
}

int compile_with_coverage(const char *source, const char *output) {
    char cmd[MAX_PATH * 3];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage -o %s %s 2>/dev/null",
             output, source);
    
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile %s\n", source);
        return -1;
    }
    return 0;
}

int run_executable(const char *executable, const char *args) {
    char cmd[MAX_PATH * 2];
    if (args) {
        snprintf(cmd, sizeof(cmd), "./%s %s >/dev/null 2>&1", executable, args);
    } else {
        snprintf(cmd, sizeof(cmd), "./%s >/dev/null 2>&1", executable);
    }
    
    return system(cmd);
}

int invoke_gcov_tool(const char *args, const char *gcda_file, const char *gcno_file) {
    char cmd[MAX_PATH * 4];
    char *gcov_tool_path = "gcov-tool";
    
    /* Check if gcov-tool exists in PATH */
    if (system("which gcov-tool >/dev/null 2>&1") != 0) {
        /* Try local build directory */
        if (access("./gcov-tool", X_OK) == 0) {
            gcov_tool_path = "./gcov-tool";
        } else {
            fprintf(stderr, "ERROR: gcov-tool not found in PATH or current directory\n");
            return -1;
        }
    }
    
    if (gcda_file && gcno_file) {
        snprintf(cmd, sizeof(cmd), "%s overlap %s %s %s 2>&1", 
                 gcov_tool_path, args, gcda_file, gcno_file);
    } else {
        snprintf(cmd, sizeof(cmd), "%s overlap %s 2>&1", gcov_tool_path, args);
    }
    
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    printf("Exit status: %d\n\n", WEXITSTATUS(status));
    
    return 0;
}

void create_test_scenarios(void) {
    printf("=== Creating test scenarios and generating coverage data ===\n\n");
    
    /* Scenario A: Simple function with branches */
    printf("Creating Scenario A (simple function with branches)...\n");
    FILE *fp = fopen("test_scenario_a.c", "w");
    if (!fp) {
        perror("Failed to create test_scenario_a.c");
        exit(1);
    }
    fputs(scenario_a_src, fp);
    fclose(fp);
    
    if (compile_with_coverage("test_scenario_a.c", "test_a") == 0) {
        strcpy(exec_files[0], "test_a");
        strcpy(gcda_files[0], "test_scenario_a.gcda");
        strcpy(gcno_files[0], "test_scenario_a.gcno");
        run_executable("test_a", NULL);
        printf("  Generated: test_a, test_scenario_a.gcda, test_scenario_a.gcno\n");
    }
    
    /* Scenario B: Loop heavy with different runs */
    printf("\nCreating Scenario B (loop heavy with multiple runs)...\n");
    fp = fopen("test_scenario_b.c", "w");
    if (!fp) {
        perror("Failed to create test_scenario_b.c");
        exit(1);
    }
    fputs(scenario_b_src, fp);
    fclose(fp);
    
    if (compile_with_coverage("test_scenario_b.c", "test_b") == 0) {
        strcpy(exec_files[1], "test_b");
        strcpy(gcda_files[1], "test_scenario_b.gcda");
        strcpy(gcno_files[1], "test_scenario_b.gcno");
        
        /* Run multiple times with different parameters */
        run_executable("test_b", "2");
        run_executable("test_b", "3");
        run_executable("test_b", "4");
        printf("  Generated: test_b, test_scenario_b.gcda, test_scenario_b.gcno\n");
    }
    
    /* Scenario C: Multiple source files */
    printf("\nCreating Scenario C (multiple source files)...\n");
    fp = fopen("scenario_c.h", "w");
    if (!fp) {
        perror("Failed to create scenario_c.h");
        exit(1);
    }
    fputs(scenario_c_header, fp);
    fclose(fp);
    
    fp = fopen("test_scenario_c1.c", "w");
    if (!fp) {
        perror("Failed to create test_scenario_c1.c");
        exit(1);
    }
    fputs(scenario_c1_src, fp);
    fclose(fp);
    
    fp = fopen("test_scenario_c2.c", "w");
    if (!fp) {
        perror("Failed to create test_scenario_c2.c");
        exit(1);
    }
    fputs(scenario_c2_src, fp);
    fclose(fp);
    
    /* Compile both files together */
    char cmd[MAX_PATH];
    snprintf(cmd, sizeof(cmd),
             "gcc -O0 -fprofile-arcs -ftest-coverage test_scenario_c1.c test_scenario_c2.c -o test_c 2>/dev/null");
    
    if (system(cmd) == 0) {
        strcpy(exec_files[2], "test_c");
        strcpy(gcda_files[2], "test_scenario_c1.gcda");
        strcpy(gcno_files[2], "test_scenario_c1.gcno");
        run_executable("test_c", NULL);
        printf("  Generated: test_c, multiple .gcda/.gcno files\n");
    }
    
    /* Scenario D: Empty/zero counts */
    printf("\nCreating Scenario D (empty/zero counts)...\n");
    fp = fopen("test_scenario_d.c", "w");
    if (!fp) {
        perror("Failed to create test_scenario_d.c");
        exit(1);
    }
    fputs(scenario_d_src, fp);
    fclose(fp);
    
    if (compile_with_coverage("test_scenario_d.c", "test_d") == 0) {
        strcpy(exec_files[3], "test_d");
        strcpy(gcda_files[3], "test_scenario_d.gcda");
        strcpy(gcno_files[3], "test_scenario_d.gcno");
        /* Intentionally NOT running the executable to get zero counts */
        printf("  Generated: test_d, test_scenario_d.gcda (zero counts), test_scenario_d.gcno\n");
    }
    
    printf("\n=== All test scenarios created ===\n");
}

void cleanup_files(void) {
    printf("\n=== Cleaning up generated files ===\n");
    
    /* Remove source files */
    remove("test_scenario_a.c");
    remove("test_scenario_b.c");
    remove("test_scenario_c1.c");
    remove("test_scenario_c2.c");
    remove("scenario_c.h");
    remove("test_scenario_d.c");
    
    /* Remove executables */
    for (int i = 0; i < NUM_TEST_SCENARIOS; i++) {
        if (exec_files[i][0]) {
            remove(exec_files[i]);
        }
    }
    
    /* Remove coverage files */
    system("rm -f *.gcda *.gcno *.gcov 2>/dev/null");
    
    printf("Cleanup completed.\n");
}
