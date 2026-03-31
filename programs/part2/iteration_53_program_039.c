/**
 * Test program to trigger uncovered lines in gcov-tool.cc (lines 534-554)
 * This program creates multiple test scenarios with coverage data and
 * invokes gcov-tool overlap with various flag combinations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH 1024

/* Function prototypes */
void create_test_scenarios(void);
void compile_with_coverage(const char *source, const char *output);
void run_executable(const char *executable, int argc, char *argv[]);
int execute_gcov_tool(const char *args);
void cleanup_files(void);

/* Test scenario source codes */
const char *scenario_a = 
    "#include <stdio.h>\n"
    "void func1(int x) {\n"
    "    if (x > 0) {\n"
    "        printf(\"Positive\\n\");\n"
    "    } else {\n"
    "        printf(\"Non-positive\\n\");\n"
    "    }\n"
    "}\n"
    "void func2(void) {\n"
    "    for (int i = 0; i < 3; i++) {\n"
    "        printf(\"Loop %d\\n\", i);\n"
    "    }\n"
    "}\n"
    "int main() {\n"
    "    func1(5);\n"
    "    func1(-2);\n"
    "    func2();\n"
    "    return 0;\n"
    "}\n";

const char *scenario_b = 
    "#include <stdio.h>\n"
    "#include <stdlib.h>\n"
    "void nested_loops(int n) {\n"
    "    int count = 0;\n"
    "    for (int i = 0; i < n; i++) {\n"
    "        for (int j = 0; j < n; j++) {\n"
    "            for (int k = 0; k < n; k++) {\n"
    "                count++;\n"
    "            }\n"
    "        }\n"
    "    }\n"
    "    printf(\"Count: %d\\n\", count);\n"
    "}\n"
    "int main(int argc, char *argv[]) {\n"
    "    int n = 3;\n"
    "    if (argc > 1) n = atoi(argv[1]);\n"
    "    nested_loops(n);\n"
    "    return 0;\n"
    "}\n";

const char *scenario_c1 = 
    "#include <stdio.h>\n"
    "#include \"test_c.h\"\n"
    "int main() {\n"
    "    printf(\"Main in file1\\n\");\n"
    "    helper_function(10);\n"
    "    return 0;\n"
    "}\n";

const char *scenario_c2 = 
    "#ifndef TEST_C_H\n"
    "#define TEST_C_H\n"
    "void helper_function(int x);\n"
    "#endif\n";

const char *scenario_c3 = 
    "#include <stdio.h>\n"
    "#include \"test_c.h\"\n"
    "void helper_function(int x) {\n"
    "    printf(\"Helper: %d\\n\", x);\n"
    "    if (x % 2 == 0) {\n"
    "        printf(\"Even\\n\");\n"
    "    } else {\n"
    "        printf(\"Odd\\n\");\n"
    "    }\n"
    "}\n";

const char *scenario_d = 
    "#include <stdio.h>\n"
    "int never_called(void) {\n"
    "    return 42;\n"
    "}\n"
    "int main() {\n"
    "    /* This main doesn't call any instrumented functions */\n"
    "    printf(\"No coverage generated\\n\");\n"
    "    return 0;\n"
    "}\n";

int main(int argc, char *argv[]) {
    printf("=== Starting gcov-tool overlap coverage test ===\n");
    
    /* Step 1: Create and compile test scenarios */
    create_test_scenarios();
    
    /* Step 2: Run executables to generate .gcda files */
    printf("\n--- Generating coverage data ---\n");
    
    /* Run scenario A */
    run_executable("test_a", 0, NULL);
    
    /* Run scenario B multiple times with different inputs */
    char *args1[] = {"test_b", "2", NULL};
    char *args2[] = {"test_b", "4", NULL};
    run_executable("test_b", 2, args1);
    run_executable("test_b", 2, args2);
    
    /* Run scenario C */
    run_executable("test_c", 0, NULL);
    
    /* Run scenario D (zero coverage) */
    run_executable("test_d", 0, NULL);
    
    /* Step 3: Invoke gcov-tool overlap with various flag combinations */
    printf("\n--- Testing gcov-tool overlap flags ---\n");
    
    /* Test case 1: -v flag (verbose) */
    printf("Test 1: -v flag\n");
    execute_gcov_tool("overlap -v test_a.gcda test_a.gcno");
    
    /* Test case 2: -f flag (function level) */
    printf("\nTest 2: -f flag\n");
    execute_gcov_tool("overlap -f test_a.gcda test_a.gcno");
    
    /* Test case 3: -F flag (fullname) */
    printf("\nTest 3: -F flag\n");
    execute_gcov_tool("overlap -F test_a.gcda test_a.gcno");
    
    /* Test case 4: -o flag (object level) */
    printf("\nTest 4: -o flag\n");
    execute_gcov_tool("overlap -o test_a.gcda test_a.gcno");
    
    /* Test case 5: -h flag (hot only) */
    printf("\nTest 5: -h flag\n");
    execute_gcov_tool("overlap -h test_a.gcda test_a.gcno");
    
    /* Test case 6: -t flag with threshold */
    printf("\nTest 6: -t flag with threshold 0.5\n");
    execute_gcov_tool("overlap -t 0.5 test_a.gcda test_a.gcno");
    
    /* Test case 7: -t flag with different threshold */
    printf("\nTest 7: -t flag with threshold 0.75\n");
    execute_gcov_tool("overlap -t 0.75 test_a.gcda test_a.gcno");
    
    /* Test case 8: Multiple flags combined */
    printf("\nTest 8: Combined flags -v -f -o\n");
    execute_gcov_tool("overlap -v -f -o test_a.gcda test_a.gcno");
    
    /* Test case 9: Different combination */
    printf("\nTest 9: Combined flags -F -h -t 0.3\n");
    execute_gcov_tool("overlap -F -h -t 0.3 test_a.gcda test_a.gcno");
    
    /* Test case 10: With multiple input files */
    printf("\nTest 10: Multiple input files with -v flag\n");
    execute_gcov_tool("overlap -v test_a.gcda test_b.gcda test_a.gcno test_b.gcno");
    
    /* Test case 11: With scenario C (multiple source files) */
    printf("\nTest 11: Multiple source files with -f -o flags\n");
    execute_gcov_tool("overlap -f -o test_c.gcda file1.gcda file2.gcda test_c.gcno file1.gcno file2.gcno");
    
    /* Test case 12: With zero-coverage file */
    printf("\nTest 12: Zero coverage file with -t flag\n");
    execute_gcov_tool("overlap -t 0.1 test_d.gcda test_d.gcno");
    
    /* Test case 13: Invalid option to trigger default case */
    printf("\nTest 13: Invalid option to trigger default case\n");
    execute_gcov_tool("overlap -z test_a.gcda test_a.gcno");
    
    /* Test case 14: Another invalid combination */
    printf("\nTest 14: Another invalid option\n");
    execute_gcov_tool("overlap -x -y test_a.gcda test_a.gcno");
    
    /* Step 4: Cleanup */
    printf("\n--- Cleaning up generated files ---\n");
    cleanup_files();
    
    printf("\n=== gcov-tool overlap coverage test completed ===\n");
    printf("All flag combinations have been tested.\n");
    printf("Check gcov-tool's coverage report to verify lines 534-554 are covered.\n");
    
    return 0;
}

void create_test_scenarios(void) {
    FILE *fp;
    
    printf("--- Creating test scenarios ---\n");
    
    /* Scenario A: Simple function */
    fp = fopen("test_a.c", "w");
    if (fp) {
        fputs(scenario_a, fp);
        fclose(fp);
        printf("Created test_a.c\n");
        compile_with_coverage("test_a.c", "test_a");
    }
    
    /* Scenario B: Loop heavy */
    fp = fopen("test_b.c", "w");
    if (fp) {
        fputs(scenario_b, fp);
        fclose(fp);
        printf("Created test_b.c\n");
        compile_with_coverage("test_b.c", "test_b");
    }
    
    /* Scenario C: Multiple source files */
    fp = fopen("file1.c", "w");
    if (fp) {
        fputs(scenario_c1, fp);
        fclose(fp);
        printf("Created file1.c\n");
    }
    
    fp = fopen("test_c.h", "w");
    if (fp) {
        fputs(scenario_c2, fp);
        fclose(fp);
        printf("Created test_c.h\n");
    }
    
    fp = fopen("file2.c", "w");
    if (fp) {
        fputs(scenario_c3, fp);
        fclose(fp);
        printf("Created file2.c\n");
    }
    
    /* Compile scenario C */
    char cmd[MAX_PATH];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage file1.c file2.c -o test_c");
    system(cmd);
    printf("Compiled test_c (multiple files)\n");
    
    /* Scenario D: Empty/zero counts */
    fp = fopen("test_d.c", "w");
    if (fp) {
        fputs(scenario_d, fp);
        fclose(fp);
        printf("Created test_d.c\n");
        compile_with_coverage("test_d.c", "test_d");
    }
}

void compile_with_coverage(const char *source, const char *output) {
    char cmd[MAX_PATH];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s", 
             source, output);
    
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Failed to compile %s\n", source);
    } else {
        printf("Compiled %s -> %s\n", source, output);
    }
}

void run_executable(const char *executable, int argc, char *argv[]) {
    char cmd[MAX_PATH];
    
    if (argc == 0) {
        snprintf(cmd, sizeof(cmd), "./%s", executable);
    } else {
        /* Build command with arguments */
        snprintf(cmd, sizeof(cmd), "./%s", executable);
        for (int i = 1; i < argc; i++) {
            strcat(cmd, " ");
            strcat(cmd, argv[i]);
        }
    }
    
    printf("Running: %s\n", cmd);
    system(cmd);
}

int execute_gcov_tool(const char *args) {
    char cmd[MAX_PATH];
    int ret;
    
    /* First try gcov-tool in current directory */
    snprintf(cmd, sizeof(cmd), "./gcov-tool %s", args);
    ret = system(cmd);
    
    if (ret != 0) {
        /* Try system gcov-tool */
        snprintf(cmd, sizeof(cmd), "gcov-tool %s", args);
        ret = system(cmd);
        
        if (ret != 0) {
            fprintf(stderr, "Warning: gcov-tool execution failed for: %s\n", args);
            fprintf(stderr, "Make sure gcov-tool is in PATH or current directory\n");
        }
    }
    
    return ret;
}

void cleanup_files(void) {
    /* Remove source files */
    remove("test_a.c");
    remove("test_b.c");
    remove("file1.c");
    remove("file2.c");
    remove("test_c.h");
    remove("test_d.c");
    
    /* Remove executables */
    remove("test_a");
    remove("test_b");
    remove("test_c");
    remove("test_d");
    
    /* Remove coverage data files */
    system("rm -f *.gcda *.gcno *.gcov 2>/dev/null");
    
    printf("Cleaned up temporary files\n");
}
