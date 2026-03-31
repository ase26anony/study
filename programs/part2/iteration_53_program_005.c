/* test_gcov_tool_overlap.c - Test harness for gcov-tool overlap command parsing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Simple test programs to generate varied coverage data */

/* Scenario A: Simple function with conditional branches */
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

/* Scenario B: Loop-heavy program with nested loops */
const char *test_prog_b = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"int process_matrix(int size, int iterations) {\n"
"    int total = 0;\n"
"    for (int iter = 0; iter < iterations; iter++) {\n"
"        for (int i = 0; i < size; i++) {\n"
"            for (int j = 0; j < size; j++) {\n"
"                total += i * j + iter;\n"
"            }\n"
"        }\n"
"    }\n"
"    return total;\n"
"}\n"
"int main(int argc, char **argv) {\n"
"    int size = 5;\n"
"    int iterations = 3;\n"
"    if (argc > 1) size = atoi(argv[1]);\n"
"    if (argc > 2) iterations = atoi(argv[2]);\n"
"    int result = process_matrix(size, iterations);\n"
"    printf(\"Result: %d\\n\", result);\n"
"    return 0;\n"
"}\n";

/* Scenario C: Multiple source files (main file) */
const char *test_prog_c1 = 
"#include <stdio.h>\n"
"#include \"test_utils.h\"\n"
"int main() {\n"
"    int a = 10, b = 20;\n"
"    printf(\"Sum: %d\\n\", add(a, b));\n"
"    printf(\"Diff: %d\\n\", subtract(a, b));\n"
"    printf(\"Max: %d\\n\", max(a, b));\n"
"    return 0;\n"
"}\n";

const char *test_prog_c2 = 
"#include \"test_utils.h\"\n"
"int add(int x, int y) {\n"
"    return x + y;\n"
"}\n"
"int subtract(int x, int y) {\n"
"    return x - y;\n"
"}\n"
"int max(int x, int y) {\n"
"    return (x > y) ? x : y;\n"
"}\n";

const char *test_header = 
"#ifndef TEST_UTILS_H\n"
"#define TEST_UTILS_H\n"
"int add(int x, int y);\n"
"int subtract(int x, int y);\n"
"int max(int x, int y);\n"
"#endif\n";

/* Scenario D: Program that may produce zero counts */
const char *test_prog_d = 
"#include <stdio.h>\n"
"int main(int argc, char **argv) {\n"
"    /* This path may not be executed if we don't run the program */\n"
"    if (argc > 1) {\n"
"        printf(\"Executed with argument\\n\");\n"
"        for (int i = 0; i < 5; i++) {\n"
"            printf(\"Loop %d\\n\", i);\n"
"        }\n"
"    }\n"
"    return 0;\n"
"}\n";

/* Helper function to execute a command and check status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Command failed: %s (status: %d)\n", cmd, status);
    }
    return status;
}

/* Helper to write a string to a file */
void write_to_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    fputs(content, f);
    fclose(f);
}

int main(int argc, char **argv) {
    printf("=== Testing gcov-tool overlap command parsing ===\n");
    
    /* Check if gcov-tool exists */
    if (system("which gcov-tool > /dev/null 2>&1") != 0) {
        fprintf(stderr, "Error: gcov-tool not found in PATH\n");
        fprintf(stderr, "Try: ./gcov-tool or build/gcov-tool\n");
        return 1;
    }
    
    /* Create test directory */
    execute_command("mkdir -p test_coverage_data");
    chdir("test_coverage_data");
    
    /* Clean up any previous test files */
    execute_command("rm -f *.gcda *.gcno *.o a.out test*");
    
    /* =========================================== */
    /* Generate coverage data for different scenarios */
    /* =========================================== */
    
    /* Scenario A: Simple function */
    printf("\n--- Generating Scenario A coverage data ---\n");
    write_to_file("test_a.c", test_prog_a);
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_a.c -o test_a");
    execute_command("./test_a");
    
    /* Scenario B: Loop-heavy with multiple runs */
    printf("\n--- Generating Scenario B coverage data ---\n");
    write_to_file("test_b.c", test_prog_b);
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_b.c -o test_b");
    execute_command("./test_b 3 2");
    execute_command("./test_b 4 1");
    execute_command("./test_b 2 5");
    
    /* Scenario C: Multiple source files */
    printf("\n--- Generating Scenario C coverage data ---\n");
    write_to_file("test_utils.h", test_header);
    write_to_file("test_c1.c", test_prog_c1);
    write_to_file("test_c2.c", test_prog_c2);
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage -I. test_c1.c test_c2.c -o test_c");
    execute_command("./test_c");
    
    /* Scenario D: Zero-count coverage (don't run the executable) */
    printf("\n--- Generating Scenario D coverage data (zero counts) ---\n");
    write_to_file("test_d.c", test_prog_d);
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_d.c -o test_d");
    /* Intentionally NOT running test_d to get zero counts */
    
    /* =========================================== */
    /* Test gcov-tool overlap with various flags  */
    /* Targeting the uncovered switch cases       */
    /* =========================================== */
    
    printf("\n=== Testing gcov-tool overlap flag parsing ===\n");
    
    /* Test 1: -v flag (verbose) - case 'v' */
    printf("\n--- Test 1: -v flag (verbose) ---\n");
    execute_command("gcov-tool overlap -v test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 2: -f flag (function level) - case 'f' */
    printf("\n--- Test 2: -f flag (function level) ---\n");
    execute_command("gcov-tool overlap -f test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 3: -F flag (fullname) - case 'F' */
    printf("\n--- Test 3: -F flag (fullname) ---\n");
    execute_command("gcov-tool overlap -F test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 4: -o flag (object level) - case 'o' */
    printf("\n--- Test 4: -o flag (object level) ---\n");
    execute_command("gcov-tool overlap -o test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 5: -h flag (hot only) - case 'h' */
    printf("\n--- Test 5: -h flag (hot only) ---\n");
    execute_command("gcov-tool overlap -h test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 6: -t flag with threshold - case 't' */
    printf("\n--- Test 6: -t flag with threshold 0.5 ---\n");
    execute_command("gcov-tool overlap -t 0.5 test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 7: -t flag with different threshold */
    printf("\n--- Test 7: -t flag with threshold 0.75 ---\n");
    execute_command("gcov-tool overlap -t 0.75 test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 8: -t flag with threshold 0.0 */
    printf("\n--- Test 8: -t flag with threshold 0.0 ---\n");
    execute_command("gcov-tool overlap -t 0.0 test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 9: -t flag with threshold 1.0 */
    printf("\n--- Test 9: -t flag with threshold 1.0 ---\n");
    execute_command("gcov-tool overlap -t 1.0 test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 10: Combination of flags */
    printf("\n--- Test 10: Combination -v -f -o ---\n");
    execute_command("gcov-tool overlap -v -f -o test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 11: Another combination with -F and -h */
    printf("\n--- Test 11: Combination -F -h -t 0.3 ---\n");
    execute_command("gcov-tool overlap -F -h -t 0.3 test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 12: Test with multiple .gcda files */
    printf("\n--- Test 12: Multiple files with -v flag ---\n");
    execute_command("gcov-tool overlap -v test_a.gcda test_b.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 13: Test with zero-count .gcda file */
    printf("\n--- Test 13: Zero-count file with -t flag ---\n");
    execute_command("gcov-tool overlap -t 0.5 test_d.gcda test_d.gcno 2>&1 | head -20");
    
    /* Test 14: Test with multiple source files scenario */
    printf("\n--- Test 14: Multi-source with -f -o flags ---\n");
    execute_command("gcov-tool overlap -f -o test_c1.gcda test_c2.gcda test_c1.gcno 2>&1 | head -20");
    
    /* Test 15: Invalid option to trigger default case */
    printf("\n--- Test 15: Invalid option -z (trigger default case) ---\n");
    execute_command("gcov-tool overlap -z 2>&1 | head -10");
    
    /* Test 16: Another invalid option combination */
    printf("\n--- Test 16: Valid + invalid option mix ---\n");
    execute_command("gcov-tool overlap -v -z -f 2>&1 | head -10");
    
    /* Test 17: Missing argument for -t flag (should also trigger error) */
    printf("\n--- Test 17: -t without argument ---\n");
    execute_command("gcov-tool overlap -t 2>&1 | head -10");
    
    /* Test 18: Edge case - very small threshold */
    printf("\n--- Test 18: Very small threshold 0.001 ---\n");
    execute_command("gcov-tool overlap -t 0.001 test_b.gcda test_b.gcno 2>&1 | head -20");
    
    /* Test 19: Edge case - threshold > 1.0 */
    printf("\n--- Test 19: Threshold 1.5 ---\n");
    execute_command("gcov-tool overlap -t 1.5 test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 20: All flags together */
    printf("\n--- Test 20: All valid flags together ---\n");
    execute_command("gcov-tool overlap -v -f -F -o -h -t 0.6 test_a.gcda test_a.gcno 2>&1 | head -25");
    
    printf("\n=== Testing complete ===\n");
    
    /* Cleanup */
    printf("\nCleaning up test files...\n");
    chdir("..");
    execute_command("rm -rf test_coverage_data");
    
    printf("\nAll gcov-tool overlap flag parsing tests executed.\n");
    printf("The following switch cases in gcov-tool.cc should now be covered:\n");
    printf("  case 'v': (verbose)\n");
    printf("  case 'f': (overlap_func_level)\n");
    printf("  case 'F': (overlap_use_fullname)\n");
    printf("  case 'o': (overlap_obj_level)\n");
    printf("  case 'h': (overlap_hot_only)\n");
    printf("  case 't': (overlap_hot_threshold)\n");
    printf("  default:  (overlap_usage)\n");
    
    return 0;
}
