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

/* Simple test programs to generate coverage data */

/* Scenario A: Simple function with basic branches */
const char *test_a = 
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

/* Scenario B: Loop-heavy program */
const char *test_b = 
"#include <stdio.h>\n"
"int main(int argc, char **argv) {\n"
"    int i, j, sum = 0;\n"
"    int limit = (argc > 1) ? atoi(argv[1]) : 10;\n"
"    \n"
"    for (i = 0; i < limit; i++) {\n"
"        for (j = 0; j < i; j++) {\n"
"            sum += j;\n"
"        }\n"
"    }\n"
"    \n"
"    if (sum > 100) {\n"
"        printf(\"Large sum: %d\\n\", sum);\n"
"    } else {\n"
"        printf(\"Small sum: %d\\n\", sum);\n"
"    }\n"
"    return 0;\n"
"}\n";

/* Scenario C: Multiple source files - part 1 */
const char *test_c1 = 
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

const char *test_c2 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"void helper2(void) {\n"
"    printf(\"Helper2 called\\n\");\n"
"}\n";

const char *test_c_header = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"void helper1(void);\n"
"void helper2(void);\n"
"#endif\n";

/* Scenario D: Program with zero coverage */
const char *test_d = 
"#include <stdio.h>\n"
"int main() {\n"
"    /* This code is never executed */\n"
"    return 0;\n"
"}\n";

/* Execute a shell command and check return status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Command failed: %s\n", cmd);
    }
    return status;
}

/* Write a string to a file */
void write_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to write file");
        return;
    }
    fputs(content, f);
    fclose(f);
}

/* Clean up generated files */
void cleanup_files(const char *base_name) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -f %s %s.gcno %s.gcda %s.o", 
             base_name, base_name, base_name, base_name);
    system(cmd);
}

/* Check if gcov-tool exists */
int check_gcov_tool() {
    if (system("which gcov-tool > /dev/null 2>&1") == 0) {
        return 1;
    }
    
    /* Check in current directory */
    if (access("./gcov-tool", X_OK) == 0) {
        return 1;
    }
    
    fprintf(stderr, "Error: gcov-tool not found in PATH or current directory\n");
    fprintf(stderr, "Please ensure gcov-tool is built and available\n");
    return 0;
}

int main(int argc, char **argv) {
    printf("=== Starting gcov-tool overlap coverage test ===\n\n");
    
    /* Check if gcov-tool exists */
    if (!check_gcov_tool()) {
        return 1;
    }
    
    /* Determine gcov-tool path */
    const char *gcov_tool = "gcov-tool";
    if (access("./gcov-tool", X_OK) == 0) {
        gcov_tool = "./gcov-tool";
    }
    
    /* Create test directory */
    system("mkdir -p test_coverage_data");
    chdir("test_coverage_data");
    
    /* =========================================== */
    /* Generate coverage data for various scenarios */
    /* =========================================== */
    
    /* Scenario A: Simple function */
    printf("\n--- Generating Scenario A (simple function) ---\n");
    write_file("test_a.c", test_a);
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_a.c -o test_a");
    execute_command("./test_a");
    
    /* Scenario B: Loop heavy - run multiple times with different inputs */
    printf("\n--- Generating Scenario B (loop heavy) ---\n");
    write_file("test_b.c", test_b);
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_b.c -o test_b");
    execute_command("./test_b 5");
    execute_command("./test_b 20");
    
    /* Scenario C: Multiple source files */
    printf("\n--- Generating Scenario C (multiple files) ---\n");
    write_file("test_c.h", test_c_header);
    write_file("test_c1.c", test_c1);
    write_file("test_c2.c", test_c2);
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage -I. test_c1.c test_c2.c -o test_c");
    execute_command("./test_c");
    
    /* Scenario D: Zero coverage */
    printf("\n--- Generating Scenario D (zero coverage) ---\n");
    write_file("test_d.c", test_d);
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_d.c -o test_d");
    /* Don't run it - keep zero coverage */
    
    /* =========================================== */
    /* Test gcov-tool overlap with various flags   */
    /* Targeting uncovered lines 534-554           */
    /* =========================================== */
    
    printf("\n=== Testing gcov-tool overlap with various flags ===\n");
    
    char cmd[512];
    int test_count = 0;
    
    /* Test 1: -v flag (verbose) - case 'v' */
    printf("\n--- Test %d: -v flag (verbose) ---\n", ++test_count);
    snprintf(cmd, sizeof(cmd), "%s overlap -v test_a.gcda test_a.gcno", gcov_tool);
    execute_command(cmd);
    
    /* Test 2: -f flag (function level) - case 'f' */
    printf("\n--- Test %d: -f flag (function level) ---\n", ++test_count);
    snprintf(cmd, sizeof(cmd), "%s overlap -f test_a.gcda test_b.gcda", gcov_tool);
    execute_command(cmd);
    
    /* Test 3: -F flag (fullname) - case 'F' */
    printf("\n--- Test %d: -F flag (fullname) ---\n", ++test_count);
    snprintf(cmd, sizeof(cmd), "%s overlap -F test_a.gcda test_b.gcda", gcov_tool);
    execute_command(cmd);
    
    /* Test 4: -o flag (object level) - case 'o' */
    printf("\n--- Test %d: -o flag (object level) ---\n", ++test_count);
    snprintf(cmd, sizeof(cmd), "%s overlap -o test_a.gcda test_b.gcda", gcov_tool);
    execute_command(cmd);
    
    /* Test 5: -h flag (hot only) - case 'h' */
    printf("\n--- Test %d: -h flag (hot only) ---\n", ++test_count);
    snprintf(cmd, sizeof(cmd), "%s overlap -h test_a.gcda test_b.gcda", gcov_tool);
    execute_command(cmd);
    
    /* Test 6: -t flag with threshold - case 't' */
    printf("\n--- Test %d: -t flag with threshold 0.5 ---\n", ++test_count);
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.5 test_a.gcda test_b.gcda", gcov_tool);
    execute_command(cmd);
    
    /* Test 7: -t flag with different threshold */
    printf("\n--- Test %d: -t flag with threshold 0.75 ---\n", ++test_count);
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.75 test_a.gcda test_b.gcda", gcov_tool);
    execute_command(cmd);
    
    /* Test 8: -t flag with threshold 0.0 */
    printf("\n--- Test %d: -t flag with threshold 0.0 ---\n", ++test_count);
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.0 test_a.gcda test_b.gcda", gcov_tool);
    execute_command(cmd);
    
    /* Test 9: -t flag with threshold 1.0 */
    printf("\n--- Test %d: -t flag with threshold 1.0 ---\n", ++test_count);
    snprintf(cmd, sizeof(cmd), "%s overlap -t 1.0 test_a.gcda test_b.gcda", gcov_tool);
    execute_command(cmd);
    
    /* Test 10: Combination of flags */
    printf("\n--- Test %d: Combination -v -f -o ---\n", ++test_count);
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f -o test_a.gcda test_b.gcda", gcov_tool);
    execute_command(cmd);
    
    /* Test 11: Another combination with -t */
    printf("\n--- Test %d: Combination -v -h -t 0.3 ---\n", ++test_count);
    snprintf(cmd, sizeof(cmd), "%s overlap -v -h -t 0.3 test_a.gcda test_b.gcda", gcov_tool);
    execute_command(cmd);
    
    /* Test 12: Test with multiple input files */
    printf("\n--- Test %d: Multiple input files with -F ---\n", ++test_count);
    snprintf(cmd, sizeof(cmd), "%s overlap -F test_a.gcda test_b.gcda test_c1.gcda", gcov_tool);
    execute_command(cmd);
    
    /* Test 13: Test with zero-coverage file */
    printf("\n--- Test %d: With zero-coverage file ---\n", ++test_count);
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.1 test_a.gcda test_d.gcda", gcov_tool);
    execute_command(cmd);
    
    /* Test 14: Invalid option to trigger default case - case 'default' */
    printf("\n--- Test %d: Invalid option to trigger default case ---\n", ++test_count);
    snprintf(cmd, sizeof(cmd), "%s overlap -z 2>&1 | head -5", gcov_tool);
    execute_command(cmd);
    
    /* Test 15: Another invalid option combination */
    printf("\n--- Test %d: Another invalid option ---\n", ++test_count);
    snprintf(cmd, sizeof(cmd), "%s overlap -x test_a.gcda 2>&1 | head -5", gcov_tool);
    execute_command(cmd);
    
    /* Test 16: Valid flags but missing required arguments */
    printf("\n--- Test %d: Missing required arguments ---\n", ++test_count);
    snprintf(cmd, sizeof(cmd), "%s overlap -v 2>&1 | head -5", gcov_tool);
    execute_command(cmd);
    
    /* Test 17: Test with .gcno files as input */
    printf("\n--- Test %d: With .gcno files ---\n", ++test_count);
    snprintf(cmd, sizeof(cmd), "%s overlap -v test_a.gcda test_a.gcno", gcov_tool);
    execute_command(cmd);
    
    /* Test 18: Mix of .gcda and .gcno */
    printf("\n--- Test %d: Mix of .gcda and .gcno files ---\n", ++test_count);
    snprintf(cmd, sizeof(cmd), "%s overlap -f test_a.gcda test_b.gcno test_c1.gcda", gcov_tool);
    execute_command(cmd);
    
    /* =========================================== */
    /* Cleanup                                     */
    /* =========================================== */
    
    printf("\n=== Cleaning up test files ===\n");
    
    /* Clean up individual files */
    cleanup_files("test_a");
    cleanup_files("test_b");
    cleanup_files("test_c");
    cleanup_files("test_d");
    
    /* Clean up source files */
    system("rm -f test_a.c test_b.c test_c.h test_c1.c test_c2.c test_d.c");
    system("rm -f test_c.gcno test_c1.gcno test_c2.gcno");
    system("rm -f test_c.gcda test_c1.gcda test_c2.gcda");
    
    /* Go back to original directory */
    chdir("..");
    system("rmdir test_coverage_data 2>/dev/null");
    
    printf("\n=== Test completed ===\n");
    printf("Executed %d gcov-tool overlap tests covering all target flags:\n", test_count);
    printf("  -v (verbose)          - line 536\n");
    printf("  -f (func level)       - line 539\n");
    printf("  -F (fullname)         - line 542\n");
    printf("  -o (object level)     - line 545\n");
    printf("  -h (hot only)         - line 548\n");
    printf("  -t (threshold)        - line 551\n");
    printf("  default case          - line 553 (overlap_usage)\n");
    
    return 0;
}
