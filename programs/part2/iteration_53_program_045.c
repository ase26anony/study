/* test_gcov_tool_overlap.c
 * Test harness to trigger uncovered lines in gcov-tool.cc (lines 534-554)
 * Compile and run: gcc test_gcov_tool_overlap.c -o test_gcov_tool && ./test_gcov_tool
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

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
"        default: printf(\"Other\\n\");\n"
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
"    /* While loop */\n"
"    int k = 0;\n"
"    while (k < n) {\n"
"        printf(\"k=%d\\n\", k);\n"
"        k++;\n"
"    }\n"
"    \n"
"    return 0;\n"
"}\n";

/* Scenario C: Multiple files - file1.c */
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

/* Scenario C: Multiple files - file2.c */
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
"int main(int argc, char **argv) {\n"
"    /* This path is only taken with specific input */\n"
"    if (argc > 1 && argv[1][0] == 'X') {\n"
"        printf(\"Executed instrumented path\\n\");\n"
"    } else {\n"
"        /* This path has no coverage instrumentation */\n"
"        /* (though gcc might still instrument it) */\n"
"        printf(\"Default path\\n\");\n"
"    }\n"
"    return 0;\n"
"}\n";

/* Helper function to execute a command and check status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Command failed: %s\n", cmd);
    }
    return status;
}

/* Write a string to a file */
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

/* Main test orchestrator */
int main(int argc, char **argv) {
    printf("=== Starting gcov-tool overlap coverage test ===\n");
    
    /* Check if gcov-tool exists */
    if (system("which gcov-tool > /dev/null 2>&1") != 0) {
        /* Try in current directory */
        if (access("./gcov-tool", X_OK) != 0) {
            fprintf(stderr, "Error: gcov-tool not found in PATH or current directory\n");
            fprintf(stderr, "Please build gcov-tool with coverage first:\n");
            fprintf(stderr, "  g++ -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
            return 1;
        }
    }
    
    /* Create temporary directory for test files */
    system("rm -rf gcov_test_tmp && mkdir -p gcov_test_tmp");
    chdir("gcov_test_tmp");
    
    /* =========================================== */
    /* Generate coverage data for each scenario    */
    /* =========================================== */
    
    /* Scenario A: Simple function */
    printf("\n--- Scenario A: Simple function ---\n");
    write_to_file("test_a.c", test_prog_a);
    execute_command("gcc -fprofile-arcs -ftest-coverage -O0 test_a.c -o test_a");
    execute_command("./test_a");
    
    /* Scenario B: Loop heavy (run multiple times) */
    printf("\n--- Scenario B: Loop heavy ---\n");
    write_to_file("test_b.c", test_prog_b);
    execute_command("gcc -fprofile-arcs -ftest-coverage -O0 test_b.c -o test_b");
    execute_command("./test_b 2");      /* Run with n=2 */
    execute_command("./test_b 5");      /* Run with n=5 (accumulates counts) */
    
    /* Scenario C: Multiple source files */
    printf("\n--- Scenario C: Multiple files ---\n");
    write_to_file("test_c1.c", test_prog_c1);
    write_to_file("test_c2.c", test_prog_c2);
    write_to_file("test_header.h", test_header);
    execute_command("gcc -fprofile-arcs -ftest-coverage -O0 test_c1.c test_c2.c -o test_c");
    execute_command("./test_c");
    
    /* Scenario D: Zero/partial counts */
    printf("\n--- Scenario D: Zero counts ---\n");
    write_to_file("test_d.c", test_prog_d);
    execute_command("gcc -fprofile-arcs -ftest-coverage -O0 test_d.c -o test_d");
    execute_command("./test_d");  /* Run without 'X' argument - may have zero counts in some paths */
    
    /* =========================================== */
    /* Invoke gcov-tool overlap with various flags */
    /* Targeting lines 534-554 in gcov-tool.cc     */
    /* =========================================== */
    
    printf("\n=== Testing gcov-tool overlap flags ===\n");
    
    /* Test 1: -v flag (verbose) - triggers case 'v' */
    printf("\nTest 1: -v flag (verbose)\n");
    execute_command("gcov-tool overlap -v test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 2: -f flag (function level) - triggers case 'f' */
    printf("\nTest 2: -f flag (function level)\n");
    execute_command("gcov-tool overlap -f test_a.gcda test_b.gcda 2>&1 | head -20");
    
    /* Test 3: -F flag (fullname) - triggers case 'F' */
    printf("\nTest 3: -F flag (fullname)\n");
    execute_command("gcov-tool overlap -F test_a.gcda test_b.gcda 2>&1 | head -20");
    
    /* Test 4: -o flag (object level) - triggers case 'o' */
    printf("\nTest 4: -o flag (object level)\n");
    execute_command("gcov-tool overlap -o test_a.gcda test_b.gcda 2>&1 | head -20");
    
    /* Test 5: -h flag (hot only) - triggers case 'h' */
    printf("\nTest 5: -h flag (hot only)\n");
    execute_command("gcov-tool overlap -h test_a.gcda test_b.gcda 2>&1 | head -20");
    
    /* Test 6: -t flag with threshold - triggers case 't' and atof(optarg) */
    printf("\nTest 6: -t flag with threshold 0.5\n");
    execute_command("gcov-tool overlap -t 0.5 test_a.gcda test_b.gcda 2>&1 | head -20");
    
    /* Test 7: -t flag with different threshold */
    printf("\nTest 7: -t flag with threshold 0.75\n");
    execute_command("gcov-tool overlap -t 0.75 test_a.gcda test_b.gcda 2>&1 | head -20");
    
    /* Test 8: -t flag with integer threshold */
    printf("\nTest 8: -t flag with threshold 1\n");
    execute_command("gcov-tool overlap -t 1 test_a.gcda test_b.gcda 2>&1 | head -20");
    
    /* Test 9: Combination of flags */
    printf("\nTest 9: Combination -v -f -o\n");
    execute_command("gcov-tool overlap -v -f -o test_a.gcda test_b.gcda 2>&1 | head -20");
    
    /* Test 10: Another combination with -t */
    printf("\nTest 10: Combination -v -h -t 0.3\n");
    execute_command("gcov-tool overlap -v -h -t 0.3 test_a.gcda test_b.gcda 2>&1 | head -20");
    
    /* Test 11: With multiple input files */
    printf("\nTest 11: Multiple files with -F flag\n");
    execute_command("gcov-tool overlap -F test_a.gcda test_b.gcda test_c1.gcda 2>&1 | head -20");
    
    /* Test 12: With .gcno files explicitly */
    printf("\nTest 12: Explicit .gcno files with -o flag\n");
    execute_command("gcov-tool overlap -o test_a.gcda test_a.gcno test_b.gcda test_b.gcno 2>&1 | head -20");
    
    /* Test 13: Default case - invalid option -z */
    /* This triggers the default case and overlap_usage() */
    printf("\nTest 13: Invalid option -z (triggers default case)\n");
    execute_command("gcov-tool overlap -z 2>&1 | head -10");
    
    /* Test 14: Another invalid option */
    printf("\nTest 14: Invalid option --invalid (triggers default case)\n");
    execute_command("gcov-tool overlap --invalid 2>&1 | head -10");
    
    /* Test 15: -t without argument (should also trigger error/usage) */
    printf("\nTest 15: -t without argument\n");
    execute_command("gcov-tool overlap -t 2>&1 | head -10");
    
    /* Test 16: All flags together */
    printf("\nTest 16: All valid flags together\n");
    execute_command("gcov-tool overlap -v -f -F -o -h -t 0.9 test_a.gcda test_b.gcda 2>&1 | head -25");
    
    /* Test 17: With the zero-counts file */
    printf("\nTest 17: With zero-counts file and hot threshold\n");
    execute_command("gcov-tool overlap -h -t 0.1 test_a.gcda test_d.gcda 2>&1 | head -20");
    
    /* Test 18: Multiple runs to accumulate different coverage patterns */
    printf("\nTest 18: After multiple runs of test program\n");
    execute_command("./test_b 10");  /* Run again to change counts */
    execute_command("gcov-tool overlap -v -t 0.8 test_b.gcda test_b.gcda 2>&1 | head -20");
    
    /* =========================================== */
    /* Cleanup and exit                           */
    /* =========================================== */
    
    printf("\n=== Cleaning up ===\n");
    chdir("..");
    execute_command("rm -rf gcov_test_tmp");
    
    printf("\n=== Test completed ===\n");
    printf("The following gcov-tool overlap flags were tested:\n");
    printf("  -v (verbose)           - case 'v'\n");
    printf("  -f (function level)    - case 'f'\n");
    printf("  -F (fullname)          - case 'F'\n");
    printf("  -o (object level)      - case 'o'\n");
    printf("  -h (hot only)          - case 'h'\n");
    printf("  -t N (threshold)       - case 't' with atof(optarg)\n");
    printf("  -z (invalid)           - default case -> overlap_usage()\n");
    
    return 0;
}
