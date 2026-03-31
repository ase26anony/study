/**
 * test_gcov_tool_overlap.c
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

#define MAX_PATH 1024

/* Simple C program with basic conditional branches */
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

/* Loop-heavy program with nested loops */
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
"    int iterations = 3;\n"
"    if (argc > 1) {\n"
"        iterations = atoi(argv[1]);\n"
"    }\n"
"    nested_loops(iterations);\n"
"    return 0;\n"
"}\n";

/* First file of multi-file program */
const char *test_program_c1 = 
"#include <stdio.h>\n"
"#include \"test_header.h\"\n"
"void helper1(void) {\n"
"    printf(\"Helper1 called\\n\");\n"
"}\n"
"int main() {\n"
"    helper1();\n"
"    helper2();\n"
"    return 0;\n"
"}\n";

/* Second file of multi-file program */
const char *test_program_c2 = 
"#include <stdio.h>\n"
"#include \"test_header.h\"\n"
"void helper2(void) {\n"
"    printf(\"Helper2 called\\n\");\n"
"}\n";

/* Header file for multi-file program */
const char *test_header = 
"#ifndef TEST_HEADER_H\n"
"#define TEST_HEADER_H\n"
"void helper1(void);\n"
"void helper2(void);\n"
"#endif\n";

/* Program that may produce zero counts */
const char *test_program_d = 
"#include <stdio.h>\n"
"int main() {\n"
"    // This program is compiled with coverage but never executes\n"
"    // the instrumented code path due to early return\n"
"    return 0;\n"
"    // Unreachable code with coverage instrumentation\n"
"    printf(\"This never runs\\n\");\n"
"    if (1) {\n"
"        printf(\"This also never runs\\n\");\n"
"    }\n"
"}\n";

/* Function to execute a shell command and check return status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Command failed with status %d: %s\n", status, cmd);
    }
    return status;
}

/* Function to write a string to a file */
void write_to_file(const char *filename, const char *content) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fputs(content, fp);
    fclose(fp);
}

/* Function to compile a test program with coverage flags */
void compile_with_coverage(const char *source_file, const char *output_name) {
    char cmd[MAX_PATH];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage -o %s %s",
             output_name, source_file);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Failed to compile %s\n", source_file);
        exit(EXIT_FAILURE);
    }
}

/* Function to run a program to generate .gcda files */
void run_program(const char *program, const char *args) {
    char cmd[MAX_PATH];
    if (args && args[0] != '\0') {
        snprintf(cmd, sizeof(cmd), "./%s %s", program, args);
    } else {
        snprintf(cmd, sizeof(cmd), "./%s", program);
    }
    execute_command(cmd);
}

/* Function to invoke gcov-tool overlap with specific flags */
void test_gcov_tool_overlap(const char *gcda_file, const char *gcno_file, 
                           const char *flags, const char *description) {
    char cmd[MAX_PATH];
    printf("\n=== Testing: %s ===\n", description);
    
    /* First test with the specific flags */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap %s %s %s 2>&1",
             flags, gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Also test with the flags in reverse order to ensure parsing works */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap %s %s %s 2>&1",
             gcda_file, gcno_file, flags);
    execute_command(cmd);
}

int main(int argc, char *argv[]) {
    printf("=== Starting gcov-tool overlap argument parsing tests ===\n");
    
    /* Check if gcov-tool exists */
    if (system("which gcov-tool > /dev/null 2>&1") != 0) {
        fprintf(stderr, "Error: gcov-tool not found in PATH\n");
        fprintf(stderr, "Please ensure gcov-tool is built and in your PATH\n");
        fprintf(stderr, "You can build it with: gcc -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
        return EXIT_FAILURE;
    }
    
    /* Create test directory */
    execute_command("mkdir -p test_coverage_data");
    execute_command("cd test_coverage_data");
    
    /* Clean up any existing test files */
    execute_command("rm -f test*.c test*.h test*.gcda test*.gcno test*_exec");
    
    /* ============================================
     * Scenario A: Simple function with branches
     * ============================================ */
    printf("\n--- Scenario A: Simple function with branches ---\n");
    write_to_file("test_a.c", test_program_a);
    compile_with_coverage("test_a.c", "test_a_exec");
    run_program("test_a_exec", "");
    
    /* ============================================
     * Scenario B: Loop-heavy program
     * ============================================ */
    printf("\n--- Scenario B: Loop-heavy program ---\n");
    write_to_file("test_b.c", test_program_b);
    compile_with_coverage("test_b.c", "test_b_exec");
    /* Run multiple times with different arguments for varied counts */
    run_program("test_b_exec", "2");
    run_program("test_b_exec", "3");
    run_program("test_b_exec", "1");
    
    /* ============================================
     * Scenario C: Multiple source files
     * ============================================ */
    printf("\n--- Scenario C: Multiple source files ---\n");
    write_to_file("test_header.h", test_header);
    write_to_file("test_c1.c", test_program_c1);
    write_to_file("test_c2.c", test_program_c2);
    /* Compile both files together */
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage -o test_c_exec test_c1.c test_c2.c");
    run_program("test_c_exec", "");
    
    /* ============================================
     * Scenario D: Program with zero counts
     * ============================================ */
    printf("\n--- Scenario D: Program with zero counts ---\n");
    write_to_file("test_d.c", test_program_d);
    compile_with_coverage("test_d.c", "test_d_exec");
    /* Don't run it to get zero counts, or run it but it returns early */
    run_program("test_d_exec", "");
    
    /* ============================================
     * Test gcov-tool overlap with various flags
     * Targeting lines 534-554 in gcov-tool.cc
     * ============================================ */
    
    printf("\n=== Testing gcov-tool overlap argument parsing ===\n");
    
    /* Test 1: -v flag (verbose mode) - triggers case 'v' */
    test_gcov_tool_overlap("test_a.gcda", "test_a.gcno", "-v", 
                          "Verbose flag (-v)");
    
    /* Test 2: -f flag (function level) - triggers case 'f' */
    test_gcov_tool_overlap("test_a.gcda", "test_a.gcno", "-f", 
                          "Function level flag (-f)");
    
    /* Test 3: -F flag (fullname) - triggers case 'F' */
    test_gcov_tool_overlap("test_a.gcda", "test_a.gcno", "-F", 
                          "Fullname flag (-F)");
    
    /* Test 4: -o flag (object level) - triggers case 'o' */
    test_gcov_tool_overlap("test_a.gcda", "test_a.gcno", "-o", 
                          "Object level flag (-o)");
    
    /* Test 5: -h flag (hot only) - triggers case 'h' */
    test_gcov_tool_overlap("test_a.gcda", "test_a.gcno", "-h", 
                          "Hot only flag (-h)");
    
    /* Test 6: -t flag with threshold - triggers case 't' */
    test_gcov_tool_overlap("test_a.gcda", "test_a.gcno", "-t 0.5", 
                          "Threshold flag (-t 0.5)");
    
    /* Test 7: -t flag with different threshold values */
    test_gcov_tool_overlap("test_b.gcda", "test_b.gcno", "-t 0.75", 
                          "Threshold flag (-t 0.75)");
    test_gcov_tool_overlap("test_b.gcda", "test_b.gcno", "-t 0.25", 
                          "Threshold flag (-t 0.25)");
    test_gcov_tool_overlap("test_b.gcda", "test_b.gcno", "-t 1.0", 
                          "Threshold flag (-t 1.0)");
    test_gcov_tool_overlap("test_b.gcda", "test_b.gcno", "-t 0.0", 
                          "Threshold flag (-t 0.0)");
    
    /* Test 8: Combination of flags */
    test_gcov_tool_overlap("test_a.gcda", "test_a.gcno", "-v -f -o", 
                          "Combination of -v, -f, -o flags");
    test_gcov_tool_overlap("test_a.gcda", "test_a.gcno", "-F -h -t 0.3", 
                          "Combination of -F, -h, -t flags");
    test_gcov_tool_overlap("test_a.gcda", "test_a.gcno", "-v -f -F -o -h -t 0.6", 
                          "All flags combined");
    
    /* Test 9: Test with multiple input files */
    printf("\n=== Testing with multiple input files ===\n");
    execute_command("gcov-tool overlap -v test_a.gcda test_b.gcda test_a.gcno test_b.gcno 2>&1");
    execute_command("gcov-tool overlap -f -o test_a.gcda test_b.gcda test_c1.gcda test_a.gcno test_b.gcno test_c1.gcno 2>&1");
    
    /* Test 10: Test with zero-count files (Scenario D) */
    printf("\n=== Testing with zero-count files ===\n");
    test_gcov_tool_overlap("test_d.gcda", "test_d.gcno", "-t 0.5", 
                          "Zero-count file with threshold");
    test_gcov_tool_overlap("test_d.gcda", "test_d.gcno", "-h", 
                          "Zero-count file with hot-only");
    
    /* Test 11: Invalid option to trigger default case and overlap_usage() */
    printf("\n=== Testing invalid option to trigger default case ===\n");
    execute_command("gcov-tool overlap -z 2>&1 | head -20");
    execute_command("gcov-tool overlap --invalid-flag 2>&1 | head -20");
    execute_command("gcov-tool overlap -v -z -f 2>&1 | head -20");
    
    /* Test 12: Edge cases for -t flag */
    printf("\n=== Testing edge cases for -t flag ===\n");
    /* Very small threshold */
    test_gcov_tool_overlap("test_a.gcda", "test_a.gcno", "-t 0.001", 
                          "Very small threshold");
    /* Threshold that might be exactly at some execution count */
    test_gcov_tool_overlap("test_b.gcda", "test_b.gcno", "-t 0.333", 
                          "Fractional threshold");
    
    /* Test 13: Different order of arguments */
    printf("\n=== Testing different argument orders ===\n");
    execute_command("gcov-tool overlap test_a.gcda -v test_a.gcno 2>&1");
    execute_command("gcov-tool overlap -t 0.5 -h test_b.gcda test_b.gcno 2>&1");
    execute_command("gcov-tool overlap test_c1.gcda -f -F test_c1.gcno 2>&1");
    
    /* ============================================
     * Cleanup
     * ============================================ */
    printf("\n=== Cleaning up test files ===\n");
    execute_command("cd ..");
    execute_command("rm -rf test_coverage_data");
    
    printf("\n=== Test completed ===\n");
    printf("All gcov-tool overlap argument parsing tests have been executed.\n");
    printf("The following cases from lines 534-554 in gcov-tool.cc were targeted:\n");
    printf("  - case 'v': verbose mode\n");
    printf("  - case 'f': function level overlap\n");
    printf("  - case 'F': use fullname\n");
    printf("  - case 'o': object level\n");
    printf("  - case 'h': hot only\n");
    printf("  - case 't': hot threshold with various values\n");
    printf("  - default: invalid option (triggers overlap_usage())\n");
    
    return EXIT_SUCCESS;
}
