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
"int main() {\n"
"    func1(5);\n"
"    func1(-3);\n"
"    return 0;\n"
"}\n";

/* Scenario B: Loop-heavy program with nested loops */
const char *test_prog_b = 
"#include <stdio.h>\n"
"int main(int argc, char *argv[]) {\n"
"    int iterations = argc > 1 ? atoi(argv[1]) : 3;\n"
"    int i, j, k;\n"
"    int sum = 0;\n"
"    \n"
"    for (i = 0; i < iterations; i++) {\n"
"        for (j = 0; j < 5; j++) {\n"
"            for (k = 0; k < 2; k++) {\n"
"                sum += i * j * k;\n"
"            }\n"
"        }\n"
"    }\n"
"    \n"
"    printf(\"Sum: %d\\n\", sum);\n"
"    \n"
"    /* Some conditional logic */\n"
"    if (sum > 100) {\n"
"        printf(\"Large sum\\n\");\n"
"    } else if (sum > 50) {\n"
"        printf(\"Medium sum\\n\");\n"
"    } else {\n"
"        printf(\"Small sum\\n\");\n"
"    }\n"
"    return 0;\n"
"}\n";

/* Scenario C: Multiple source files - File 1 */
const char *test_prog_c1 = 
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

/* Scenario C: Multiple source files - File 2 */
const char *test_prog_c2 = 
"#include <stdio.h>\n"
"#include \"test_header.h\"\n"
"void helper2(void) {\n"
"    printf(\"Helper2 called\\n\");\n"
"    if (1) { /* Always true, but creates branch */\n"
"        printf(\"Inside if\\n\");\n"
"    }\n"
"}\n";

/* Scenario C: Header file */
const char *test_header = 
"#ifndef TEST_HEADER_H\n"
"#define TEST_HEADER_H\n"
"void helper1(void);\n"
"void helper2(void);\n"
"#endif\n";

/* Scenario D: Program that may produce zero counts */
const char *test_prog_d = 
"#include <stdio.h>\n"
"int main(int argc, char *argv[]) {\n"
"    /* This path may not be taken if we don't provide right args */\n"
"    if (argc > 1 && atoi(argv[1]) > 100) {\n"
"        printf(\"High value\\n\");\n"
"        return 1;\n"
"    }\n"
"    /* Default path with minimal execution */\n"
"    return 0;\n"
"}\n";

/* Function to execute a command and check return status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        printf("Warning: Command returned non-zero: %s\n", cmd);
    }
    return status;
}

/* Function to write a string to a file */
void write_to_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to write file");
        exit(1);
    }
    fputs(content, f);
    fclose(f);
}

/* Function to check if gcov-tool exists */
int gcov_tool_exists() {
    return system("which gcov-tool > /dev/null 2>&1") == 0 ||
           system("command -v gcov-tool > /dev/null 2>&1") == 0;
}

int main(int argc, char *argv[]) {
    char cmd[1024];
    int i;
    
    printf("=== Test Harness for gcov-tool overlap coverage ===\n\n");
    
    /* Check if gcov-tool is available */
    if (!gcov_tool_exists()) {
        printf("Error: gcov-tool not found in PATH.\n");
        printf("Please ensure gcov-tool is built and in your PATH.\n");
        printf("You can build it with: gcc -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
        return 1;
    }
    
    /* Clean up any previous test files */
    system("rm -f test*.c test*.h test*.gcno test*.gcda test*.o a.out test_prog 2>/dev/null");
    
    /* ============================================
     * Generate coverage data from test programs
     * ============================================ */
    
    /* Scenario A: Simple function */
    printf("\n--- Generating Scenario A coverage data ---\n");
    write_to_file("test_a.c", test_prog_a);
    execute_command("gcc -fprofile-arcs -ftest-coverage -O0 test_a.c -o test_a");
    execute_command("./test_a");
    
    /* Scenario B: Loop heavy with different runs */
    printf("\n--- Generating Scenario B coverage data ---\n");
    write_to_file("test_b.c", test_prog_b);
    execute_command("gcc -fprofile-arcs -ftest-coverage -O0 test_b.c -o test_b");
    execute_command("./test_b 2");      /* Run with 2 iterations */
    execute_command("./test_b 10");     /* Run with 10 iterations */
    
    /* Scenario C: Multiple source files */
    printf("\n--- Generating Scenario C coverage data ---\n");
    write_to_file("test_c1.c", test_prog_c1);
    write_to_file("test_c2.c", test_prog_c2);
    write_to_file("test_header.h", test_header);
    execute_command("gcc -fprofile-arcs -ftest-coverage -O0 -I. test_c1.c test_c2.c -o test_c");
    execute_command("./test_c");
    
    /* Scenario D: Zero/empty counts */
    printf("\n--- Generating Scenario D coverage data ---\n");
    write_to_file("test_d.c", test_prog_d);
    execute_command("gcc -fprofile-arcs -ftest-coverage -O0 test_d.c -o test_d");
    /* Run with low value to potentially get zero counts in some paths */
    execute_command("./test_d 5");
    
    /* ============================================
     * Invoke gcov-tool overlap with various flags
     * Targeting uncovered lines 534-554 in gcov-tool.cc
     * ============================================ */
    
    printf("\n=== Testing gcov-tool overlap with various flags ===\n");
    
    /* Test 1: -v flag (verbose) - triggers case 'v' */
    printf("\n--- Test 1: Testing -v flag (verbose) ---\n");
    execute_command("gcov-tool overlap -v test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 2: -f flag (function level) - triggers case 'f' */
    printf("\n--- Test 2: Testing -f flag (function level) ---\n");
    execute_command("gcov-tool overlap -f test_a.gcda test_b.gcda 2>&1 | head -20");
    
    /* Test 3: -F flag (fullname) - triggers case 'F' */
    printf("\n--- Test 3: Testing -F flag (fullname) ---\n");
    execute_command("gcov-tool overlap -F test_a.gcda test_b.gcda 2>&1 | head -20");
    
    /* Test 4: -o flag (object level) - triggers case 'o' */
    printf("\n--- Test 4: Testing -o flag (object level) ---\n");
    execute_command("gcov-tool overlap -o test_a.gcda test_b.gcda 2>&1 | head -20");
    
    /* Test 5: -h flag (hot only) - triggers case 'h' */
    printf("\n--- Test 5: Testing -h flag (hot only) ---\n");
    execute_command("gcov-tool overlap -h test_a.gcda test_b.gcda 2>&1 | head -20");
    
    /* Test 6: -t flag with threshold - triggers case 't' */
    printf("\n--- Test 6: Testing -t flag with threshold 0.5 ---\n");
    execute_command("gcov-tool overlap -t 0.5 test_a.gcda test_b.gcda 2>&1 | head -20");
    
    /* Test 7: -t flag with different threshold - triggers case 't' */
    printf("\n--- Test 7: Testing -t flag with threshold 0.75 ---\n");
    execute_command("gcov-tool overlap -t 0.75 test_a.gcda test_b.gcda 2>&1 | head -20");
    
    /* Test 8: -t flag with extreme threshold - triggers case 't' */
    printf("\n--- Test 8: Testing -t flag with threshold 0.01 ---\n");
    execute_command("gcov-tool overlap -t 0.01 test_a.gcda test_b.gcda 2>&1 | head -20");
    
    /* Test 9: Combination of flags - triggers multiple cases */
    printf("\n--- Test 9: Testing combination -v -f -o ---\n");
    execute_command("gcov-tool overlap -v -f -o test_a.gcda test_b.gcda 2>&1 | head -20");
    
    /* Test 10: Another combination - triggers multiple cases */
    printf("\n--- Test 10: Testing combination -F -h -t 0.3 ---\n");
    execute_command("gcov-tool overlap -F -h -t 0.3 test_a.gcda test_b.gcda 2>&1 | head -20");
    
    /* Test 11: All flags together */
    printf("\n--- Test 11: Testing all flags together ---\n");
    execute_command("gcov-tool overlap -v -f -F -o -h -t 0.6 test_a.gcda test_b.gcda 2>&1 | head -20");
    
    /* Test 12: Invalid option -z to trigger default case and overlap_usage() */
    printf("\n--- Test 12: Testing invalid option -z (triggers default case) ---\n");
    execute_command("gcov-tool overlap -z 2>&1 | head -30");
    
    /* Test 13: Multiple input files with various flags */
    printf("\n--- Test 13: Testing with multiple .gcda files ---\n");
    execute_command("gcov-tool overlap -v test_a.gcda test_b.gcda test_c1.gcda test_c2.gcda 2>&1 | head -20");
    
    /* Test 14: With .gcno files as input */
    printf("\n--- Test 14: Testing with .gcno files ---\n");
    execute_command("gcov-tool overlap -f test_a.gcno test_b.gcno 2>&1 | head -20");
    
    /* Test 15: Mixed .gcda and .gcno with threshold */
    printf("\n--- Test 15: Testing mixed files with threshold ---\n");
    execute_command("gcov-tool overlap -t 0.8 test_a.gcda test_a.gcno test_b.gcda test_b.gcno 2>&1 | head -20");
    
    /* Test 16: Zero count files with hot threshold */
    printf("\n--- Test 16: Testing zero count files with hot threshold ---\n");
    execute_command("gcov-tool overlap -h -t 0.1 test_d.gcda test_a.gcda 2>&1 | head -20");
    
    /* ============================================
     * Additional edge cases
     * ============================================ */
    
    /* Test with same file twice */
    printf("\n--- Edge case: Same file twice ---\n");
    execute_command("gcov-tool overlap -v test_a.gcda test_a.gcda 2>&1 | head -20");
    
    /* Test with no files (should trigger usage) */
    printf("\n--- Edge case: No input files ---\n");
    execute_command("gcov-tool overlap -v 2>&1 | head -20");
    
    /* Test with just -v and no other args */
    printf("\n--- Edge case: Just -v flag ---\n");
    execute_command("gcov-tool overlap -v 2>&1 | head -20");
    
    /* Test invalid threshold (non-numeric) - should still parse -t flag */
    printf("\n--- Edge case: Invalid threshold value ---\n");
    execute_command("gcov-tool overlap -t invalid test_a.gcda test_b.gcda 2>&1 | head -20");
    
    /* ============================================
     * Cleanup
     * ============================================ */
    
    printf("\n=== Cleaning up test files ===\n");
    system("rm -f test*.c test*.h test*.gcno test*.gcda test_prog test_a test_b test_c test_d 2>/dev/null");
    system("rm -f *.gcov 2>/dev/null");
    
    printf("\n=== Test completed ===\n");
    printf("The gcov-tool overlap command has been invoked with various flag combinations.\n");
    printf("Check coverage of gcov-tool.cc lines 534-554 to verify execution.\n");
    
    return 0;
}
