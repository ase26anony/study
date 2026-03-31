/* test_gcov_tool_overlap.c
 * 
 * This program tests the gcov-tool overlap command-line argument parsing
 * to cover lines 534-554 in gcov-tool.cc
 * 
 * Compile and run:
 *   gcc -o test_gcov_tool_overlap test_gcov_tool_overlap.c
 *   ./test_gcov_tool_overlap
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

/* Scenario B: Loop heavy program */
const char *test_program_b = 
"#include <stdio.h>\n"
"int main(int argc, char *argv[]) {\n"
"    int i, j, k;\n"
"    int iterations = (argc > 1) ? atoi(argv[1]) : 3;\n"
"    \n"
"    for (i = 0; i < iterations; i++) {\n"
"        printf(\"Outer loop: %d\\n\", i);\n"
"        for (j = 0; j < 2; j++) {\n"
"            printf(\"  Middle loop: %d\\n\", j);\n"
"            for (k = 0; k < 2; k++) {\n"
"                printf(\"    Inner loop: %d\\n\", k);\n"
"            }\n"
"        }\n"
"    }\n"
"    \n"
"    int sum = 0;\n"
"    while (sum < 10) {\n"
"        sum += 2;\n"
"        printf(\"While loop sum: %d\\n\", sum);\n"
"    }\n"
"    \n"
"    return 0;\n"
"}\n";

/* Scenario C: Multiple source files - part 1 */
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

/* Scenario C: Multiple source files - part 2 */
const char *test_program_c2 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"void helper2(void) {\n"
"    printf(\"Helper2 called\\n\");\n"
"    if (1) { /* Always true, but creates branch */\n"
"        printf(\"  Inside if\\n\");\n"
"    }\n"
"}\n";

/* Scenario C: Header file */
const char *test_header_c = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"void helper1(void);\n"
"void helper2(void);\n"
"#endif\n";

/* Scenario D: Empty/zero counts program */
const char *test_program_d = 
"#include <stdio.h>\n"
"int main() {\n"
"    /* This program has coverage instrumentation\n"
"     * but doesn't execute any instrumented paths\n"
"     * (except main entry/exit)\n"
"     */\n"
"    return 0;\n"
"}\n";

/* Function to execute a shell command and check return status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        printf("Warning: Command returned non-zero: %d\n", status);
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

/* Function to check if gcov-tool exists */
int gcov_tool_exists() {
    return system("which gcov-tool > /dev/null 2>&1") == 0 ||
           system("command -v gcov-tool > /dev/null 2>&1") == 0;
}

int main(int argc, char *argv[]) {
    printf("=== Testing gcov-tool overlap command-line parsing ===\n");
    printf("Target: Lines 534-554 in gcov-tool.cc\n\n");
    
    /* Check if gcov-tool is available */
    if (!gcov_tool_exists()) {
        printf("Error: gcov-tool not found in PATH.\n");
        printf("Please ensure gcov-tool is built and available.\n");
        printf("You may need to specify full path to gcov-tool.\n");
        return EXIT_FAILURE;
    }
    
    /* Create test directory */
    execute_command("mkdir -p test_coverage_data");
    execute_command("cd test_coverage_data");
    
    /* ============================================================
     * Generate coverage data for Scenario A
     * ============================================================ */
    printf("\n--- Generating Scenario A (Simple function) ---\n");
    write_to_file("test_a.c", test_program_a);
    
    /* Compile with coverage */
    execute_command("gcc -fprofile-arcs -ftest-coverage -O0 -o test_a test_a.c");
    
    /* Run to generate .gcda file */
    execute_command("./test_a");
    
    /* ============================================================
     * Generate coverage data for Scenario B
     * ============================================================ */
    printf("\n--- Generating Scenario B (Loop heavy) ---\n");
    write_to_file("test_b.c", test_program_b);
    
    /* Compile with coverage */
    execute_command("gcc -fprofile-arcs -ftest-coverage -O0 -o test_b test_b.c");
    
    /* Run multiple times with different inputs */
    execute_command("./test_b 2");      /* Run with argument 2 */
    execute_command("./test_b 5");      /* Run with argument 5 */
    
    /* ============================================================
     * Generate coverage data for Scenario C (multiple files)
     * ============================================================ */
    printf("\n--- Generating Scenario C (Multiple source files) ---\n");
    write_to_file("test_c1.c", test_program_c1);
    write_to_file("test_c2.c", test_program_c2);
    write_to_file("test_c.h", test_header_c);
    
    /* Compile all files together with coverage */
    execute_command("gcc -fprofile-arcs -ftest-coverage -O0 -o test_c test_c1.c test_c2.c");
    
    /* Run to generate .gcda files */
    execute_command("./test_c");
    
    /* ============================================================
     * Generate coverage data for Scenario D (zero counts)
     * ============================================================ */
    printf("\n--- Generating Scenario D (Empty/zero counts) ---\n");
    write_to_file("test_d.c", test_program_d);
    
    /* Compile with coverage */
    execute_command("gcc -fprofile-arcs -ftest-coverage -O0 -o test_d test_d.c");
    
    /* Don't run it - this will create .gcda files with zero counts
     * when the program is eventually run, or we can run it once */
    execute_command("./test_d");  /* Run once but it has minimal coverage */
    
    /* ============================================================
     * Now test gcov-tool overlap with various command-line flags
     * Targeting the switch cases in lines 534-554
     * ============================================================ */
    printf("\n=== Testing gcov-tool overlap command-line arguments ===\n");
    
    /* Test 1: -v flag (verbose) - case 'v' */
    printf("\nTest 1: Testing -v flag (case 'v')\n");
    execute_command("gcov-tool overlap -v test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 2: -f flag (overlap_func_level) - case 'f' */
    printf("\nTest 2: Testing -f flag (case 'f')\n");
    execute_command("gcov-tool overlap -f test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 3: -F flag (overlap_use_fullname) - case 'F' */
    printf("\nTest 3: Testing -F flag (case 'F')\n");
    execute_command("gcov-tool overlap -F test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 4: -o flag (overlap_obj_level) - case 'o' */
    printf("\nTest 4: Testing -o flag (case 'o')\n");
    execute_command("gcov-tool overlap -o test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 5: -h flag (overlap_hot_only) - case 'h' */
    printf("\nTest 5: Testing -h flag (case 'h')\n");
    execute_command("gcov-tool overlap -h test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 6: -t flag with argument (overlap_hot_threshold) - case 't' */
    printf("\nTest 6: Testing -t flag with argument 0.5 (case 't')\n");
    execute_command("gcov-tool overlap -t 0.5 test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 7: -t flag with different argument */
    printf("\nTest 7: Testing -t flag with argument 0.75\n");
    execute_command("gcov-tool overlap -t 0.75 test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 8: -t flag with argument 0.0 */
    printf("\nTest 8: Testing -t flag with argument 0.0\n");
    execute_command("gcov-tool overlap -t 0.0 test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 9: -t flag with argument 1.0 */
    printf("\nTest 9: Testing -t flag with argument 1.0\n");
    execute_command("gcov-tool overlap -t 1.0 test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 10: Combination of flags */
    printf("\nTest 10: Testing combination -v -f -o\n");
    execute_command("gcov-tool overlap -v -f -o test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 11: Another combination */
    printf("\nTest 11: Testing combination -F -h -t 0.3\n");
    execute_command("gcov-tool overlap -F -h -t 0.3 test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 12: Test with multiple .gcda files */
    printf("\nTest 12: Testing with multiple .gcda files\n");
    execute_command("gcov-tool overlap -v test_a.gcda test_b.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 13: Test with zero-count .gcda file (Scenario D) */
    printf("\nTest 13: Testing with zero-count .gcda file\n");
    execute_command("gcov-tool overlap -t 0.5 test_d.gcda test_d.gcno 2>&1 | head -20");
    
    /* Test 14: Test with multiple source files (Scenario C) */
    printf("\nTest 14: Testing with multiple source files\n");
    execute_command("gcov-tool overlap -v test_c1.gcda test_c2.gcda test_c1.gcno test_c2.gcno 2>&1 | head -20");
    
    /* Test 15: Invalid option to trigger default case and overlap_usage() */
    printf("\nTest 15: Testing invalid option -z (trigger default case)\n");
    execute_command("gcov-tool overlap -z 2>&1 | head -30");
    
    /* Test 16: Another invalid option */
    printf("\nTest 16: Testing invalid option --invalid (trigger default case)\n");
    execute_command("gcov-tool overlap --invalid 2>&1 | head -30");
    
    /* Test 17: -t without argument (should trigger error/usage) */
    printf("\nTest 17: Testing -t without argument\n");
    execute_command("gcov-tool overlap -t 2>&1 | head -30");
    
    /* Test 18: All flags together */
    printf("\nTest 18: Testing all valid flags together\n");
    execute_command("gcov-tool overlap -v -f -F -o -h -t 0.25 test_a.gcda test_a.gcno 2>&1 | head -30");
    
    /* ============================================================
     * Cleanup
     * ============================================================ */
    printf("\n=== Cleaning up test files ===\n");
    
    /* Go back to original directory */
    execute_command("cd ..");
    
    /* Remove test directory and all generated files */
    execute_command("rm -rf test_coverage_data");
    
    printf("\n=== Test completed ===\n");
    printf("The gcov-tool overlap command-line parsing (lines 534-554) should now be covered.\n");
    printf("Check coverage with: gcov -b gcov-tool.cc\n");
    
    return EXIT_SUCCESS;
}
