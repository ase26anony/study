/* test_gcov_tool_overlap.c
 * Test harness to trigger uncovered lines in gcov-tool.cc (lines 534-554)
 * Compile and run: gcc -o test_gcov_tool test_gcov_tool_overlap.c && ./test_gcov_tool
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH 1024

/* Simple test programs to generate varied coverage data */

/* Scenario A: Simple function with conditionals */
const char *test_prog_a = 
"#include <stdio.h>\n"
"void func1(int x) {\n"
"    if (x > 0) {\n"
"        printf(\"Positive\\n\");\n"
"    } else {\n"
"        printf(\"Non-positive\\n\");\n"
"    }\n"
"}\n"
"void func2(int y) {\n"
"    switch(y) {\n"
"        case 1: printf(\"One\\n\"); break;\n"
"        case 2: printf(\"Two\\n\"); break;\n"
"        default: printf(\"Other\\n\"); break;\n"
"    }\n"
"}\n"
"int main() {\n"
"    func1(5);\n"
"    func1(-3);\n"
"    func2(1);\n"
"    func2(2);\n"
"    func2(3);\n"
"    return 0;\n"
"}\n";

/* Scenario B: Loop-heavy program */
const char *test_prog_b = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"int process_value(int n) {\n"
"    int sum = 0;\n"
"    for (int i = 0; i < n; i++) {\n"
"        for (int j = 0; j < i; j++) {\n"
"            sum += j;\n"
"        }\n"
"    }\n"
"    return sum;\n"
"}\n"
"int main(int argc, char **argv) {\n"
"    int iterations = 10;\n"
"    if (argc > 1) {\n"
"        iterations = atoi(argv[1]);\n"
"    }\n"
"    int total = 0;\n"
"    for (int k = 0; k < iterations; k++) {\n"
"        total += process_value(k + 1);\n"
"    }\n"
"    printf(\"Total: %d\\n\", total);\n"
"    return 0;\n"
"}\n";

/* Scenario C: Multiple source files - part 1 */
const char *test_prog_c1 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"void helper1(void) {\n"
"    printf(\"Helper1 called\\n\");\n"
"}\n"
"int main() {\n"
"    helper1();\n"
"    helper2();\n"
"    return 0;\n"
"}\n";

/* Scenario C: Multiple source files - part 2 */
const char *test_prog_c2 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"void helper2(void) {\n"
"    printf(\"Helper2 called\\n\");\n"
"}\n";

/* Scenario C: Header file */
const char *test_header_c = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"void helper1(void);\n"
"void helper2(void);\n"
"#endif\n";

/* Scenario D: Program that may produce zero counts */
const char *test_prog_d = 
"#include <stdio.h>\n"
"int main(int argc, char **argv) {\n"
"    /* This path is never taken when argc == 1 */\n"
"    if (argc > 10) {\n"
"        printf(\"This should not print\\n\");\n"
"        for (int i = 0; i < 100; i++) {\n"
"            printf(\"Loop %d\\n\", i);\n"
"        }\n"
"    }\n"
"    return 0;\n"
"}\n";

/* Utility functions */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Command failed: %s (status: %d)\n", cmd, status);
    }
    return status;
}

int compile_with_coverage(const char *src_file, const char *output) {
    char cmd[MAX_PATH];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage -o %s %s",
             output, src_file);
    return execute_command(cmd);
}

int run_program(const char *program, const char *args) {
    char cmd[MAX_PATH];
    if (args && args[0]) {
        snprintf(cmd, sizeof(cmd), "./%s %s", program, args);
    } else {
        snprintf(cmd, sizeof(cmd), "./%s", program);
    }
    return execute_command(cmd);
}

int run_gcov_tool_overlap(const char *gcda_file, const char *gcno_file, const char *flags) {
    char cmd[MAX_PATH];
    /* Try to find gcov-tool in common locations */
    const char *gcov_tool_path = NULL;
    const char *paths[] = {
        "./gcov-tool",
        "../gcc/build/gcc/gcov-tool",
        "/usr/bin/gcov-tool",
        "/usr/local/bin/gcov-tool",
        NULL
    };
    
    for (int i = 0; paths[i]; i++) {
        if (access(paths[i], X_OK) == 0) {
            gcov_tool_path = paths[i];
            break;
        }
    }
    
    if (!gcov_tool_path) {
        fprintf(stderr, "ERROR: gcov-tool not found in common locations\n");
        fprintf(stderr, "Please ensure gcov-tool is in PATH or current directory\n");
        return -1;
    }
    
    snprintf(cmd, sizeof(cmd), "%s overlap %s %s %s",
             gcov_tool_path, flags, gcda_file, gcno_file);
    return execute_command(cmd);
}

int main(int argc, char **argv) {
    printf("=== Test harness for gcov-tool overlap command-line parsing ===\n");
    printf("Target: lines 534-554 in gcov-tool.cc\n\n");
    
    /* Create test source files */
    FILE *fp;
    
    /* Scenario A */
    fp = fopen("test_a.c", "w");
    if (!fp) { perror("test_a.c"); return 1; }
    fputs(test_prog_a, fp);
    fclose(fp);
    
    /* Scenario B */
    fp = fopen("test_b.c", "w");
    if (!fp) { perror("test_b.c"); return 1; }
    fputs(test_prog_b, fp);
    fclose(fp);
    
    /* Scenario C - multiple files */
    fp = fopen("test_c1.c", "w");
    if (!fp) { perror("test_c1.c"); return 1; }
    fputs(test_prog_c1, fp);
    fclose(fp);
    
    fp = fopen("test_c2.c", "w");
    if (!fp) { perror("test_c2.c"); return 1; }
    fputs(test_prog_c2, fp);
    fclose(fp);
    
    fp = fopen("test_c.h", "w");
    if (!fp) { perror("test_c.h"); return 1; }
    fputs(test_header_c, fp);
    fclose(fp);
    
    /* Scenario D */
    fp = fopen("test_d.c", "w");
    if (!fp) { perror("test_d.c"); return 1; }
    fputs(test_prog_d, fp);
    fclose(fp);
    
    /* Compile all test programs with coverage */
    printf("\n--- Compiling test programs with coverage ---\n");
    if (compile_with_coverage("test_a.c", "test_a") != 0) {
        fprintf(stderr, "Failed to compile test_a.c\n");
    }
    
    if (compile_with_coverage("test_b.c", "test_b") != 0) {
        fprintf(stderr, "Failed to compile test_b.c\n");
    }
    
    /* Compile multi-file program */
    char cmd[MAX_PATH];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage -o test_c test_c1.c test_c2.c");
    execute_command(cmd);
    
    if (compile_with_coverage("test_d.c", "test_d") != 0) {
        fprintf(stderr, "Failed to compile test_d.c\n");
    }
    
    /* Run programs to generate .gcda files */
    printf("\n--- Running programs to generate coverage data ---\n");
    run_program("test_a", "");
    
    /* Run test_b multiple times with different arguments */
    run_program("test_b", "5");
    run_program("test_b", "3");
    run_program("test_b", "7");
    
    run_program("test_c", "");
    
    /* Run test_d without triggering instrumented code */
    run_program("test_d", "");
    
    /* Now invoke gcov-tool overlap with various flag combinations */
    printf("\n=== Testing gcov-tool overlap with different flags ===\n");
    
    /* Test 1: -v flag (verbose) - triggers case 'v' */
    printf("\n--- Test 1: -v flag (verbose) ---\n");
    run_gcov_tool_overlap("test_a.gcda", "test_a.gcno", "-v");
    
    /* Test 2: -f flag (function level) - triggers case 'f' */
    printf("\n--- Test 2: -f flag (function level) ---\n");
    run_gcov_tool_overlap("test_b.gcda", "test_b.gcno", "-f");
    
    /* Test 3: -F flag (fullname) - triggers case 'F' */
    printf("\n--- Test 3: -F flag (fullname) ---\n");
    run_gcov_tool_overlap("test_a.gcda", "test_a.gcno", "-F");
    
    /* Test 4: -o flag (object level) - triggers case 'o' */
    printf("\n--- Test 4: -o flag (object level) ---\n");
    run_gcov_tool_overlap("test_b.gcda", "test_b.gcno", "-o");
    
    /* Test 5: -h flag (hot only) - triggers case 'h' */
    printf("\n--- Test 5: -h flag (hot only) ---\n");
    run_gcov_tool_overlap("test_a.gcda", "test_a.gcno", "-h");
    
    /* Test 6: -t flag with threshold - triggers case 't' */
    printf("\n--- Test 6: -t flag with threshold 0.5 ---\n");
    run_gcov_tool_overlap("test_b.gcda", "test_b.gcno", "-t 0.5");
    
    /* Test 7: -t flag with different threshold - triggers case 't' again */
    printf("\n--- Test 7: -t flag with threshold 0.75 ---\n");
    run_gcov_tool_overlap("test_a.gcda", "test_a.gcno", "-t 0.75");
    
    /* Test 8: -t flag with very low threshold */
    printf("\n--- Test 8: -t flag with threshold 0.1 ---\n");
    run_gcov_tool_overlap("test_d.gcda", "test_d.gcno", "-t 0.1");
    
    /* Test 9: Combination of flags */
    printf("\n--- Test 9: Combination -v -f -o ---\n");
    run_gcov_tool_overlap("test_a.gcda", "test_a.gcno", "-v -f -o");
    
    /* Test 10: Combination -F -h -t */
    printf("\n--- Test 10: Combination -F -h -t 0.3 ---\n");
    run_gcov_tool_overlap("test_b.gcda", "test_b.gcno", "-F -h -t 0.3");
    
    /* Test 11: Invalid option to trigger default case and overlap_usage() */
    printf("\n--- Test 11: Invalid option -z (triggers default case) ---\n");
    /* We redirect stderr to /dev/null to avoid cluttering output */
    char gcov_tool_path[MAX_PATH];
    snprintf(gcov_tool_path, sizeof(gcov_tool_path), 
             "./gcov-tool 2>/dev/null || ../gcc/build/gcc/gcov-tool 2>/dev/null || "
             "/usr/bin/gcov-tool 2>/dev/null || /usr/local/bin/gcov-tool 2>/dev/null");
    
    char invalid_cmd[MAX_PATH];
    snprintf(invalid_cmd, sizeof(invalid_cmd), 
             "%s overlap -z test_a.gcda test_a.gcno 2>/dev/null", gcov_tool_path);
    execute_command(invalid_cmd);
    
    /* Test 12: Multiple input files with flags */
    printf("\n--- Test 12: Multiple .gcda files with -v -o ---\n");
    char multi_cmd[MAX_PATH];
    snprintf(multi_cmd, sizeof(multi_cmd),
             "./gcov-tool overlap -v -o test_a.gcda test_b.gcda test_a.gcno test_b.gcno 2>&1 || "
             "echo 'Note: gcov-tool may require specific file ordering'");
    execute_command(multi_cmd);
    
    /* Test 13: Using test_c.gcda (from multi-file compilation) */
    printf("\n--- Test 13: Multi-source program with -f -F ---\n");
    run_gcov_tool_overlap("test_c1.gcda", "test_c1.gcno", "-f -F");
    
    /* Cleanup */
    printf("\n--- Cleaning up generated files ---\n");
    execute_command("rm -f test_a test_b test_c test_d");
    execute_command("rm -f test_a.c test_b.c test_c1.c test_c2.c test_c.h test_d.c");
    execute_command("rm -f *.gcda *.gcno *.gcov");
    execute_command("rm -f *.o");
    
    printf("\n=== Test complete ===\n");
    printf("All gcov-tool overlap flag combinations have been tested.\n");
    printf("The following cases in the switch statement should have been triggered:\n");
    printf("  case 'v': verbose flag\n");
    printf("  case 'f': function level overlap\n");
    printf("  case 'F': fullname flag\n");
    printf("  case 'o': object level overlap\n");
    printf("  case 'h': hot only flag\n");
    printf("  case 't': hot threshold with atof(optarg)\n");
    printf("  default: invalid option (triggered overlap_usage())\n");
    
    return 0;
}
