/* test_gcov_tool_overlap.c
 * 
 * This program tests the gcov-tool overlap subcommand by:
 * 1. Creating multiple C test programs with different coverage patterns
 * 2. Compiling them with coverage flags
 * 3. Running them to generate .gcda files
 * 4. Invoking gcov-tool overlap with various flag combinations
 *    to trigger the uncovered switch cases in gcov-tool.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Function prototypes */
int compile_with_coverage(const char *source, const char *output);
int run_program(const char *program, const char *args);
int run_gcov_tool_overlap(const char *args, const char *gcda_file, const char *gcno_file);
void cleanup_files(const char *base_name);

/* Test C programs */
const char *test_program_a = 
"#include <stdio.h>\n"
"void func1(int x) {\n"
"    if (x > 0) {\n"
"        printf(\"Positive\\n\");\n"
"    } else {\n"
"        printf(\"Non-positive\\n\");\n"
"    }\n"
"}\n"
"void func2(int y) {\n"
"    for (int i = 0; i < y; i++) {\n"
"        printf(\"Iteration %d\\n\", i);\n"
"    }\n"
"}\n"
"int main() {\n"
"    func1(5);\n"
"    func1(-3);\n"
"    func2(3);\n"
"    return 0;\n"
"}\n";

const char *test_program_b = 
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
"    int n = 2;\n"
"    if (argc > 1) {\n"
"        n = atoi(argv[1]);\n"
"    }\n"
"    nested_loops(n);\n"
"    return 0;\n"
"}\n";

const char *test_program_c1 = 
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

const char *test_program_c2 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"void helper2(void) {\n"
"    printf(\"Helper2 called\\n\");\n"
"}\n";

const char *test_header_c = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"void helper1(void);\n"
"void helper2(void);\n"
"#endif\n";

const char *test_program_d = 
"#include <stdio.h>\n"
"int main() {\n"
"    /* This program has coverage instrumentation\n"
"     * but we won't execute the instrumented paths */\n"
"    return 0;\n"
"}\n";

/* Helper function to write a string to a file */
int write_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    fputs(content, f);
    fclose(f);
    return 1;
}

/* Compile a C program with coverage flags */
int compile_with_coverage(const char *source, const char *output) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage -o %s %s 2>/dev/null",
             output, source);
    return system(cmd) == 0;
}

/* Run a program */
int run_program(const char *program, const char *args) {
    char cmd[512];
    if (args && args[0]) {
        snprintf(cmd, sizeof(cmd), "./%s %s >/dev/null 2>&1", program, args);
    } else {
        snprintf(cmd, sizeof(cmd), "./%s >/dev/null 2>&1", program);
    }
    return system(cmd) == 0;
}

/* Run gcov-tool overlap with specific arguments */
int run_gcov_tool_overlap(const char *args, const char *gcda_file, const char *gcno_file) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap %s %s %s 2>&1",
             args, gcda_file, gcno_file);
    printf("Executing: %s\n", cmd);
    return system(cmd);
}

/* Clean up generated files */
void cleanup_files(const char *base_name) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -f %s %s.gcda %s.gcno %s.c", 
             base_name, base_name, base_name, base_name);
    system(cmd);
}

int main(int argc, char *argv[]) {
    printf("=== Testing gcov-tool overlap command-line parsing ===\n\n");
    
    /* Check if gcov-tool exists */
    if (system("which gcov-tool >/dev/null 2>&1") != 0) {
        printf("Warning: gcov-tool not found in PATH. Trying ./gcov-tool...\n");
        if (system("test -x ./gcov-tool") != 0) {
            fprintf(stderr, "Error: gcov-tool not found. Please build gcov-tool first.\n");
            fprintf(stderr, "Build with: gcc -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
            return 1;
        }
    }
    
    /* Test Scenario A: Simple function with branches */
    printf("--- Scenario A: Simple function with branches ---\n");
    if (!write_file("test_a.c", test_program_a)) {
        fprintf(stderr, "Failed to write test_a.c\n");
        return 1;
    }
    
    if (!compile_with_coverage("test_a.c", "test_a")) {
        fprintf(stderr, "Failed to compile test_a.c\n");
        return 1;
    }
    
    if (!run_program("test_a", "")) {
        fprintf(stderr, "Failed to run test_a\n");
    }
    
    /* Test Scenario B: Loop-heavy program */
    printf("\n--- Scenario B: Loop-heavy program ---\n");
    if (!write_file("test_b.c", test_program_b)) {
        fprintf(stderr, "Failed to write test_b.c\n");
        return 1;
    }
    
    if (!compile_with_coverage("test_b.c", "test_b")) {
        fprintf(stderr, "Failed to compile test_b.c\n");
        return 1;
    }
    
    /* Run multiple times with different arguments */
    run_program("test_b", "1");
    run_program("test_b", "3");
    run_program("test_b", "2");
    
    /* Test Scenario C: Multiple source files */
    printf("\n--- Scenario C: Multiple source files ---\n");
    if (!write_file("test_c.h", test_header_c)) {
        fprintf(stderr, "Failed to write test_c.h\n");
        return 1;
    }
    
    if (!write_file("test_c1.c", test_program_c1)) {
        fprintf(stderr, "Failed to write test_c1.c\n");
        return 1;
    }
    
    if (!write_file("test_c2.c", test_program_c2)) {
        fprintf(stderr, "Failed to write test_c2.c\n");
        return 1;
    }
    
    /* Compile both files together */
    if (system("gcc -O0 -fprofile-arcs -ftest-coverage test_c1.c test_c2.c -o test_c 2>/dev/null") != 0) {
        fprintf(stderr, "Failed to compile test_c program\n");
        return 1;
    }
    
    run_program("test_c", "");
    
    /* Test Scenario D: Program with zero counts */
    printf("\n--- Scenario D: Program with zero counts ---\n");
    if (!write_file("test_d.c", test_program_d)) {
        fprintf(stderr, "Failed to write test_d.c\n");
        return 1;
    }
    
    if (!compile_with_coverage("test_d.c", "test_d")) {
        fprintf(stderr, "Failed to compile test_d.c\n");
        return 1;
    }
    
    /* Don't run it - keep zero counts */
    
    /* Now invoke gcov-tool overlap with various flags to trigger the switch cases */
    printf("\n=== Invoking gcov-tool overlap with various flags ===\n");
    
    /* Case 'v': verbose flag */
    printf("\n1. Testing -v flag (case 'v'):\n");
    run_gcov_tool_overlap("-v", "test_a.gcda", "test_a.gcno");
    
    /* Case 'f': function level overlap */
    printf("\n2. Testing -f flag (case 'f'):\n");
    run_gcov_tool_overlap("-f", "test_a.gcda", "test_a.gcno");
    
    /* Case 'F': use fullname */
    printf("\n3. Testing -F flag (case 'F'):\n");
    run_gcov_tool_overlap("-F", "test_a.gcda", "test_a.gcno");
    
    /* Case 'o': object level */
    printf("\n4. Testing -o flag (case 'o'):\n");
    run_gcov_tool_overlap("-o", "test_a.gcda", "test_a.gcno");
    
    /* Case 'h': hot only */
    printf("\n5. Testing -h flag (case 'h'):\n");
    run_gcov_tool_overlap("-h", "test_b.gcda", "test_b.gcno");
    
    /* Case 't': hot threshold with argument */
    printf("\n6. Testing -t flag with argument (case 't'):\n");
    run_gcov_tool_overlap("-t 0.5", "test_b.gcda", "test_b.gcno");
    
    /* Test with different threshold values */
    printf("\n7. Testing -t flag with different thresholds:\n");
    run_gcov_tool_overlap("-t 0.1", "test_b.gcda", "test_b.gcno");
    run_gcov_tool_overlap("-t 0.75", "test_b.gcda", "test_b.gcno");
    run_gcov_tool_overlap("-t 1.0", "test_b.gcda", "test_b.gcno");
    
    /* Test with multiple flags combined */
    printf("\n8. Testing multiple flags combined:\n");
    run_gcov_tool_overlap("-v -f -o", "test_a.gcda", "test_a.gcno");
    run_gcov_tool_overlap("-F -h -t 0.3", "test_b.gcda", "test_b.gcno");
    
    /* Test with program that has zero counts */
    printf("\n9. Testing with zero-count program:\n");
    run_gcov_tool_overlap("-v -h -t 0.01", "test_d.gcda", "test_d.gcno");
    
    /* Test with multiple .gcda files */
    printf("\n10. Testing with multiple input files:\n");
    system("gcov-tool overlap -v test_a.gcda test_b.gcda test_a.gcno test_b.gcno 2>&1");
    
    /* Test with the multi-file program */
    printf("\n11. Testing with multi-source program:\n");
    run_gcov_tool_overlap("-v -f", "test_c1.gcda", "test_c1.gcno");
    run_gcov_tool_overlap("-v -f", "test_c2.gcda", "test_c2.gcno");
    
    /* Default case: invalid option to trigger overlap_usage() */
    printf("\n12. Testing invalid option (default case, triggers overlap_usage):\n");
    system("gcov-tool overlap -z 2>&1");
    
    /* Test with no arguments (should also show usage) */
    printf("\n13. Testing with no arguments:\n");
    system("gcov-tool overlap 2>&1");
    
    /* Cleanup */
    printf("\n=== Cleaning up generated files ===\n");
    cleanup_files("test_a");
    cleanup_files("test_b");
    cleanup_files("test_c");
    cleanup_files("test_d");
    system("rm -f test_c1.c test_c2.c test_c.h test_c.gcda test_c1.gcda test_c2.gcda test_c1.gcno test_c2.gcno");
    
    printf("\n=== Test completed ===\n");
    printf("The gcov-tool overlap command-line parser should have executed all switch cases:\n");
    printf("  - Case 'v': verbose mode\n");
    printf("  - Case 'f': function-level overlap\n");
    printf("  - Case 'F': use fullname\n");
    printf("  - Case 'o': object-level\n");
    printf("  - Case 'h': hot-only\n");
    printf("  - Case 't': hot threshold (with various values)\n");
    printf("  - Default case: invalid option\n");
    
    return 0;
}
