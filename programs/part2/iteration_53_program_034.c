/* test_gcov_tool_overlap.c
 * 
 * This program tests the gcov-tool overlap command-line argument parsing
 * by generating various coverage data files and invoking gcov-tool with
 * different flag combinations to cover lines 534-554 in gcov-tool.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Simple test programs to generate coverage data */

/* Scenario A: Simple function with basic conditional branches */
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
"    func2(3);\n"
"    return 0;\n"
"}\n";

/* Scenario B: Loop-heavy program */
const char *test_prog_b = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"int main(int argc, char **argv) {\n"
"    int i, j, n = 3;\n"
"    if (argc > 1) n = atoi(argv[1]);\n"
"    \n"
"    /* Nested loops */\n"
"    for (i = 0; i < n; i++) {\n"
"        for (j = 0; j < n; j++) {\n"
"            printf(\"i=%d, j=%d\\n\", i, j);\n"
"        }\n"
"    }\n"
"    \n"
"    /* While loop with condition */\n"
"    int k = 0;\n"
"    while (k < n) {\n"
"        if (k % 2 == 0) {\n"
"            printf(\"Even: %d\\n\", k);\n"
"        } else {\n"
"            printf(\"Odd: %d\\n\", k);\n"
"        }\n"
"        k++;\n"
"    }\n"
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

/* Scenario D: Program with zero/empty coverage */
const char *test_prog_d = 
"#include <stdio.h>\n"
"int main() {\n"
"    /* This program is compiled with coverage but never executed */\n"
"    /* Or executed with a path that skips instrumented code */\n"
"    return 0;\n"
"}\n";

/* Function to execute a command and check status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Function to write a string to a file */
int write_to_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return -1;
    }
    fputs(content, f);
    fclose(f);
    return 0;
}

/* Function to compile a test program with coverage */
int compile_with_coverage(const char *src_file, const char *exe_name) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage -o %s %s",
             exe_name, src_file);
    return execute_command(cmd);
}

/* Function to run a test program */
int run_test_program(const char *exe_name, const char *args) {
    char cmd[512];
    if (args && args[0]) {
        snprintf(cmd, sizeof(cmd), "./%s %s", exe_name, args);
    } else {
        snprintf(cmd, sizeof(cmd), "./%s", exe_name);
    }
    return execute_command(cmd);
}

/* Function to test gcov-tool overlap with specific flags */
void test_gcov_overlap(const char *desc, const char *gcda_file, 
                       const char *gcno_file, const char *flags) {
    printf("\n=== Testing: %s ===\n", desc);
    
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap %s %s %s", 
             flags, gcda_file, gcno_file);
    
    /* Execute and capture output */
    printf("Command: %s\n", cmd);
    int status = system(cmd);
    
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
    }
}

/* Main test orchestrator */
int main(int argc, char **argv) {
    printf("=== Starting gcov-tool overlap argument parsing test ===\n");
    
    /* Check if gcov-tool exists */
    if (system("which gcov-tool > /dev/null 2>&1") != 0) {
        /* Try in current directory */
        if (access("./gcov-tool", X_OK) != 0) {
            fprintf(stderr, "Error: gcov-tool not found in PATH or current directory\n");
            fprintf(stderr, "Please build gcov-tool with: gcc -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
            return 1;
        }
    }
    
    /* Create test directory */
    system("mkdir -p test_coverage_data");
    chdir("test_coverage_data");
    
    /* Clean up any previous test files */
    system("rm -f *.gcda *.gcno *.c *.h test_* a.out");
    
    /* =========================================== */
    /* Generate Scenario A: Simple function */
    /* =========================================== */
    printf("\n--- Generating Scenario A (Simple function) ---\n");
    write_to_file("test_a.c", test_prog_a);
    compile_with_coverage("test_a.c", "test_a");
    run_test_program("test_a", "");
    
    /* =========================================== */
    /* Generate Scenario B: Loop heavy */
    /* =========================================== */
    printf("\n--- Generating Scenario B (Loop heavy) ---\n");
    write_to_file("test_b.c", test_prog_b);
    compile_with_coverage("test_b.c", "test_b");
    /* Run multiple times with different inputs */
    run_test_program("test_b", "2");
    run_test_program("test_b", "5");
    run_test_program("test_b", "1");
    
    /* =========================================== */
    /* Generate Scenario C: Multiple source files */
    /* =========================================== */
    printf("\n--- Generating Scenario C (Multiple source files) ---\n");
    write_to_file("test_c.h", test_header_c);
    write_to_file("test_c1.c", test_prog_c1);
    write_to_file("test_c2.c", test_prog_c2);
    /* Compile both files together */
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage -o test_c test_c1.c test_c2.c");
    run_test_program("test_c", "");
    
    /* =========================================== */
    /* Generate Scenario D: Zero coverage */
    /* =========================================== */
    printf("\n--- Generating Scenario D (Zero coverage) ---\n");
    write_to_file("test_d.c", test_prog_d);
    compile_with_coverage("test_d.c", "test_d");
    /* Don't run it to keep zero counts */
    
    /* =========================================== */
    /* Now test gcov-tool overlap with various flags */
    /* Targeting lines 534-554 in gcov-tool.cc */
    /* =========================================== */
    
    printf("\n=== Testing gcov-tool overlap argument parsing ===\n");
    
    /* Test 1: -v flag (verbose) - case 'v' */
    test_gcov_overlap("Verbose flag (-v)", "test_a.gcda", "test_a.gcno", "-v");
    
    /* Test 2: -f flag (function level) - case 'f' */
    test_gcov_overlap("Function level flag (-f)", "test_a.gcda", "test_a.gcno", "-f");
    
    /* Test 3: -F flag (fullname) - case 'F' */
    test_gcov_overlap("Fullname flag (-F)", "test_a.gcda", "test_a.gcno", "-F");
    
    /* Test 4: -o flag (object level) - case 'o' */
    test_gcov_overlap("Object level flag (-o)", "test_a.gcda", "test_a.gcno", "-o");
    
    /* Test 5: -h flag (hot only) - case 'h' */
    test_gcov_overlap("Hot only flag (-h)", "test_a.gcda", "test_a.gcno", "-h");
    
    /* Test 6: -t flag with threshold - case 't' */
    test_gcov_overlap("Threshold flag (-t 0.5)", "test_b.gcda", "test_b.gcno", "-t 0.5");
    test_gcov_overlap("Threshold flag (-t 0.75)", "test_b.gcda", "test_b.gcno", "-t 0.75");
    test_gcov_overlap("Threshold flag (-t 0.1)", "test_b.gcda", "test_b.gcno", "-t 0.1");
    
    /* Test 7: Combination of flags */
    test_gcov_overlap("Combination (-v -f -o)", "test_a.gcda", "test_a.gcno", "-v -f -o");
    test_gcov_overlap("Combination (-F -h -t 0.3)", "test_b.gcda", "test_b.gcno", "-F -h -t 0.3");
    
    /* Test 8: Multiple input files */
    printf("\n=== Testing with multiple input files ===\n");
    execute_command("gcov-tool overlap -v test_a.gcda test_b.gcda test_a.gcno test_b.gcno 2>&1 | head -20");
    
    /* Test 9: Default case (invalid option) - should trigger overlap_usage() */
    printf("\n=== Testing default case (invalid option) ===\n");
    printf("This should trigger overlap_usage() function\n");
    execute_command("gcov-tool overlap -z 2>&1 | head -10");
    
    /* Test 10: With zero-coverage file */
    test_gcov_overlap("Zero coverage file with -t 0.5", "test_d.gcda", "test_d.gcno", "-t 0.5");
    
    /* Test 11: Multiple source files scenario */
    test_gcov_overlap("Multiple source files (-v)", "test_c1.gcda test_c2.gcda", "test_c1.gcno test_c2.gcno", "-v");
    
    /* Test 12: Edge case threshold values */
    test_gcov_overlap("Threshold 0.0", "test_a.gcda", "test_a.gcno", "-t 0.0");
    test_gcov_overlap("Threshold 1.0", "test_a.gcda", "test_a.gcno", "-t 1.0");
    test_gcov_overlap("Threshold 100.0", "test_a.gcda", "test_a.gcno", "-t 100.0");
    
    /* Clean up */
    printf("\n=== Cleaning up test files ===\n");
    chdir("..");
    system("rm -rf test_coverage_data");
    
    printf("\n=== Test completed ===\n");
    printf("The following cases from lines 534-554 should have been executed:\n");
    printf("  - case 'v': verbose flag\n");
    printf("  - case 'f': function level overlap\n");
    printf("  - case 'F': fullname flag\n");
    printf("  - case 'o': object level flag\n");
    printf("  - case 'h': hot only flag\n");
    printf("  - case 't': threshold with various values\n");
    printf("  - default: invalid option '-z'\n");
    
    return 0;
}
