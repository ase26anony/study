/**
 * Test harness for gcov-tool overlap command-line parsing coverage.
 * This program creates multiple test scenarios, generates coverage data,
 * and invokes gcov-tool overlap with different flag combinations.
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
int run_program(const char *program, const char *args);
int invoke_gcov_tool(const char *gcda, const char *gcno, const char *flags);
void cleanup_files(const char *base_name);

/* Test scenario 1: Simple function with conditionals */
const char *scenario1_source = 
"#include <stdio.h>\n"
"void func1(int x) {\n"
"    if (x > 0) {\n"
"        printf(\"Positive\\n\");\n"
"    } else {\n"
"        printf(\"Non-positive\\n\");\n"
"    }\n"
"}\n"
"int main() {\n"
"    func1(5);\n"
"    func1(-3);\n"
"    return 0;\n"
"}\n";

/* Test scenario 2: Loop-heavy program */
const char *scenario2_source = 
"#include <stdio.h>\n"
"void nested_loops(int n) {\n"
"    int i, j;\n"
"    for (i = 0; i < n; i++) {\n"
"        for (j = 0; j < i; j++) {\n"
"            printf(\".\");\n"
"        }\n"
"        printf(\"\\n\");\n"
"    }\n"
"}\n"
"int main(int argc, char **argv) {\n"
"    int iterations = 3;\n"
"    if (argc > 1) iterations = atoi(argv[1]);\n"
"    nested_loops(iterations);\n"
"    return 0;\n"
"}\n";

/* Test scenario 3: Multiple source files */
const char *scenario3a_source = 
"#include <stdio.h>\n"
"#include \"scenario3.h\"\n"
"int helper1(int x) {\n"
"    return x * 2;\n"
"}\n"
"int main() {\n"
"    int result = helper1(5) + helper2(3);\n"
"    printf(\"Result: %d\\n\", result);\n"
"    return 0;\n"
"}\n";

const char *scenario3b_source = 
"#include \"scenario3.h\"\n"
"int helper2(int y) {\n"
"    if (y > 0) return y + 1;\n"
"    return y - 1;\n"
"}\n";

const char *scenario3_header = 
"#ifndef SCENARIO3_H\n"
"#define SCENARIO3_H\n"
"int helper1(int x);\n"
"int helper2(int y);\n"
"#endif\n";

/* Test scenario 4: Zero coverage (never executed) */
const char *scenario4_source = 
"#include <stdio.h>\n"
"void unused_function() {\n"
"    printf(\"This is never called\\n\");\n"
"}\n"
"int main() {\n"
"    /* Intentionally empty - no instrumented code executed */\n"
"    return 0;\n"
"}\n";

int main(int argc, char **argv) {
    printf("=== Starting gcov-tool overlap coverage test ===\n\n");
    
    /* Check if gcov-tool exists */
    if (system("which gcov-tool > /dev/null 2>&1") != 0) {
        fprintf(stderr, "Error: gcov-tool not found in PATH\n");
        fprintf(stderr, "Please ensure gcov-tool is built and in your PATH\n");
        return 1;
    }
    
    /* Create test directory */
    if (system("mkdir -p test_coverage_data") != 0) {
        perror("Failed to create test directory");
        return 1;
    }
    
    /* Change to test directory */
    if (chdir("test_coverage_data") != 0) {
        perror("Failed to change to test directory");
        return 1;
    }
    
    /* Scenario 1: Simple function */
    printf("=== Scenario 1: Simple function with conditionals ===\n");
    FILE *fp = fopen("test1.c", "w");
    if (!fp) {
        perror("Failed to create test1.c");
        return 1;
    }
    fputs(scenario1_source, fp);
    fclose(fp);
    
    if (!compile_with_coverage("test1.c", "test1")) {
        fprintf(stderr, "Failed to compile test1.c\n");
        return 1;
    }
    
    if (!run_program("./test1", NULL)) {
        fprintf(stderr, "Failed to run test1\n");
        return 1;
    }
    
    /* Scenario 2: Loop heavy */
    printf("\n=== Scenario 2: Loop-heavy program ===\n");
    fp = fopen("test2.c", "w");
    if (!fp) {
        perror("Failed to create test2.c");
        return 1;
    }
    fputs(scenario2_source, fp);
    fclose(fp);
    
    if (!compile_with_coverage("test2.c", "test2")) {
        fprintf(stderr, "Failed to compile test2.c\n");
        return 1;
    }
    
    /* Run multiple times with different inputs */
    run_program("./test2", "2");
    run_program("./test2", "4");
    run_program("./test2", "1");
    
    /* Scenario 3: Multiple source files */
    printf("\n=== Scenario 3: Multiple source files ===\n");
    fp = fopen("scenario3.h", "w");
    if (!fp) {
        perror("Failed to create scenario3.h");
        return 1;
    }
    fputs(scenario3_header, fp);
    fclose(fp);
    
    fp = fopen("test3a.c", "w");
    if (!fp) {
        perror("Failed to create test3a.c");
        return 1;
    }
    fputs(scenario3a_source, fp);
    fclose(fp);
    
    fp = fopen("test3b.c", "w");
    if (!fp) {
        perror("Failed to create test3b.c");
        return 1;
    }
    fputs(scenario3b_source, fp);
    fclose(fp);
    
    /* Compile multiple source files */
    char compile_cmd[MAX_PATH];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -O0 -fprofile-arcs -ftest-coverage test3a.c test3b.c -o test3");
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile test3\n");
        return 1;
    }
    
    run_program("./test3", NULL);
    
    /* Scenario 4: Zero coverage */
    printf("\n=== Scenario 4: Zero coverage program ===\n");
    fp = fopen("test4.c", "w");
    if (!fp) {
        perror("Failed to create test4.c");
        return 1;
    }
    fputs(scenario4_source, fp);
    fclose(fp);
    
    if (!compile_with_coverage("test4.c", "test4")) {
        fprintf(stderr, "Failed to compile test4.c\n");
        return 1;
    }
    
    run_program("./test4", NULL);
    
    /* Now invoke gcov-tool overlap with various flag combinations */
    printf("\n=== Invoking gcov-tool overlap with different flags ===\n\n");
    
    /* Test 1: -v flag (verbose) */
    printf("Test 1: Testing -v flag (verbose)\n");
    invoke_gcov_tool("test1.gcda", "test1.gcno", "-v");
    
    /* Test 2: -f flag (function level) */
    printf("\nTest 2: Testing -f flag (function level)\n");
    invoke_gcov_tool("test1.gcda", "test1.gcno", "-f");
    
    /* Test 3: -F flag (fullname) */
    printf("\nTest 3: Testing -F flag (fullname)\n");
    invoke_gcov_tool("test1.gcda", "test1.gcno", "-F");
    
    /* Test 4: -o flag (object level) */
    printf("\nTest 4: Testing -o flag (object level)\n");
    invoke_gcov_tool("test1.gcda", "test1.gcno", "-o");
    
    /* Test 5: -h flag (hot only) */
    printf("\nTest 5: Testing -h flag (hot only)\n");
    invoke_gcov_tool("test1.gcda", "test1.gcno", "-h");
    
    /* Test 6: -t flag with threshold */
    printf("\nTest 6: Testing -t flag with threshold 0.5\n");
    invoke_gcov_tool("test1.gcda", "test1.gcno", "-t 0.5");
    
    /* Test 7: -t flag with different threshold */
    printf("\nTest 7: Testing -t flag with threshold 0.75\n");
    invoke_gcov_tool("test1.gcda", "test1.gcno", "-t 0.75");
    
    /* Test 8: Multiple flags combined */
    printf("\nTest 8: Testing -v -f -o flags combined\n");
    invoke_gcov_tool("test1.gcda", "test1.gcno", "-v -f -o");
    
    /* Test 9: -f -F -o combined */
    printf("\nTest 9: Testing -f -F -o flags combined\n");
    invoke_gcov_tool("test1.gcda", "test1.gcno", "-f -F -o");
    
    /* Test 10: -h -t combined */
    printf("\nTest 10: Testing -h -t 0.3 flags combined\n");
    invoke_gcov_tool("test1.gcda", "test1.gcno", "-h -t 0.3");
    
    /* Test 11: With multiple input files */
    printf("\nTest 11: Testing with multiple .gcda files\n");
    char cmd[MAX_PATH];
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -v test1.gcda test2.gcda test1.gcno test2.gcno 2>&1 | head -20");
    system(cmd);
    
    /* Test 12: Trigger default case with invalid option */
    printf("\nTest 12: Testing invalid option -z (should trigger default case)\n");
    system("gcov-tool overlap -z 2>&1 | head -5");
    
    /* Test 13: Test with zero-coverage file */
    printf("\nTest 13: Testing with zero-coverage file\n");
    invoke_gcov_tool("test4.gcda", "test4.gcno", "-v -t 0.1");
    
    /* Test 14: Test with multiple source files scenario */
    printf("\nTest 14: Testing with multiple source files\n");
    invoke_gcov_tool("test3a.gcda", "test3a.gcno", "-v -f");
    
    /* Cleanup */
    printf("\n=== Cleaning up test files ===\n");
    cleanup_files("test1");
    cleanup_files("test2");
    cleanup_files("test3");
    cleanup_files("test4");
    system("rm -f test1.c test2.c test3a.c test3b.c scenario3.h");
    system("rm -f *.gcov");
    
    chdir("..");
    system("rmdir test_coverage_data");
    
    printf("\n=== Test completed successfully ===\n");
    printf("All gcov-tool overlap flag combinations have been tested.\n");
    printf("The target switch statement in gcov-tool.cc should now be covered.\n");
    
    return 0;
}

int compile_with_coverage(const char *source, const char *output) {
    char cmd[MAX_PATH];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s", 
             source, output);
    printf("Compiling: %s\n", cmd);
    return system(cmd) == 0;
}

int run_program(const char *program, const char *args) {
    char cmd[MAX_PATH];
    if (args) {
        snprintf(cmd, sizeof(cmd), "%s %s > /dev/null", program, args);
    } else {
        snprintf(cmd, sizeof(cmd), "%s > /dev/null", program);
    }
    printf("Running: %s\n", cmd);
    return system(cmd) == 0;
}

int invoke_gcov_tool(const char *gcda, const char *gcno, const char *flags) {
    char cmd[MAX_PATH];
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap %s %s %s 2>&1 | head -10", 
             flags, gcda, gcno);
    printf("Command: %s\n", cmd);
    return system(cmd);
}

void cleanup_files(const char *base_name) {
    char cmd[MAX_PATH];
    snprintf(cmd, sizeof(cmd), "rm -f %s %s.gcda %s.gcno", base_name, base_name, base_name);
    system(cmd);
}
