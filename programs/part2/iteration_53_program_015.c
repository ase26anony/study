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
"int main() {\n"
"    func1(5);\n"
"    func1(-3);\n"
"    return 0;\n"
"}\n";

/* Scenario B: Loop-heavy program */
const char *test_prog_b = 
"#include <stdio.h>\n"
"int main(int argc, char **argv) {\n"
"    int i, j, sum = 0;\n"
"    int limit = (argc > 1) ? atoi(argv[1]) : 10;\n"
"    \n"
"    for (i = 0; i < limit; i++) {\n"
"        for (j = 0; j < i; j++) {\n"
"            sum += i * j;\n"
"        }\n"
"    }\n"
"    \n"
"    printf(\"Sum: %d\\n\", sum);\n"
"    return 0;\n"
"}\n";

/* Scenario C: Multiple source files - part 1 */
const char *test_prog_c1 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"int main() {\n"
"    int a = 5, b = 3;\n"
"    printf(\"Add: %d\\n\", add(a, b));\n"
"    printf(\"Sub: %d\\n\", subtract(a, b));\n"
"    return 0;\n"
"}\n";

const char *test_prog_c2 = 
"int add(int x, int y) {\n"
"    return x + y;\n"
"}\n"
"int subtract(int x, int y) {\n"
"    return x - y;\n"
"}\n";

const char *test_prog_c_header = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"int add(int x, int y);\n"
"int subtract(int x, int y);\n"
"#endif\n";

/* Scenario D: Program with zero coverage */
const char *test_prog_d = 
"#include <stdio.h>\n"
"int main() {\n"
"    /* This program is compiled but never executed */\n"
"    /* Or executed with a path that skips instrumented code */\n"
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

/* Find gcov-tool in PATH or current directory */
char* find_gcov_tool() {
    static char path[1024];
    
    /* Check current directory first */
    if (access("./gcov-tool", X_OK) == 0) {
        strcpy(path, "./gcov-tool");
        return path;
    }
    
    /* Check in PATH */
    char *path_env = getenv("PATH");
    if (path_env) {
        char *token = strtok(path_env, ":");
        while (token) {
            snprintf(path, sizeof(path), "%s/gcov-tool", token);
            if (access(path, X_OK) == 0) {
                return path;
            }
            token = strtok(NULL, ":");
        }
    }
    
    /* Not found */
    return NULL;
}

int main(int argc, char **argv) {
    char cmd[2048];
    char *gcov_tool;
    int ret = 0;
    
    printf("=== Test Harness for gcov-tool overlap command ===\n\n");
    
    /* Find gcov-tool binary */
    gcov_tool = find_gcov_tool();
    if (!gcov_tool) {
        fprintf(stderr, "ERROR: gcov-tool not found in PATH or current directory\n");
        fprintf(stderr, "Please build gcov-tool with: gcc -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
        return 1;
    }
    printf("Using gcov-tool: %s\n\n", gcov_tool);
    
    /* Create test directory */
    execute_command("mkdir -p test_coverage_data");
    execute_command("cd test_coverage_data");
    
    /* ============================================
     * Scenario A: Simple function with branches
     * ============================================ */
    printf("\n--- Scenario A: Simple function ---\n");
    write_to_file("test_coverage_data/test_a.c", test_prog_a);
    
    /* Compile with coverage */
    execute_command("cd test_coverage_data && gcc -O0 -fprofile-arcs -ftest-coverage test_a.c -o test_a");
    
    /* Run to generate .gcda file */
    execute_command("cd test_coverage_data && ./test_a");
    
    /* ============================================
     * Scenario B: Loop-heavy program
     * ============================================ */
    printf("\n--- Scenario B: Loop-heavy program ---\n");
    write_to_file("test_coverage_data/test_b.c", test_prog_b);
    
    /* Compile with coverage */
    execute_command("cd test_coverage_data && gcc -O0 -fprofile-arcs -ftest-coverage test_b.c -o test_b");
    
    /* Run multiple times with different inputs */
    execute_command("cd test_coverage_data && ./test_b 5");
    execute_command("cd test_coverage_data && ./test_b 10");
    execute_command("cd test_coverage_data && ./test_b 3");
    
    /* ============================================
     * Scenario C: Multiple source files
     * ============================================ */
    printf("\n--- Scenario C: Multiple source files ---\n");
    write_to_file("test_coverage_data/test_c_main.c", test_prog_c1);
    write_to_file("test_coverage_data/test_c_lib.c", test_prog_c2);
    write_to_file("test_coverage_data/test_c.h", test_prog_c_header);
    
    /* Compile all files together with coverage */
    execute_command("cd test_coverage_data && gcc -O0 -fprofile-arcs -ftest-coverage test_c_main.c test_c_lib.c -o test_c");
    
    /* Run to generate .gcda files for both sources */
    execute_command("cd test_coverage_data && ./test_c");
    
    /* ============================================
     * Scenario D: Zero coverage program
     * ============================================ */
    printf("\n--- Scenario D: Zero coverage program ---\n");
    write_to_file("test_coverage_data/test_d.c", test_prog_d);
    
    /* Compile but don't run (or run with minimal path) */
    execute_command("cd test_coverage_data && gcc -O0 -fprofile-arcs -ftest-coverage test_d.c -o test_d");
    /* Optionally run once to generate empty coverage */
    execute_command("cd test_coverage_data && ./test_d");
    
    /* ============================================
     * Now invoke gcov-tool overlap with various flags
     * Targeting the uncovered switch cases in gcov-tool.cc
     * ============================================ */
    
    printf("\n=== Invoking gcov-tool overlap with various flags ===\n\n");
    
    /* Test 1: -v flag (verbose) - triggers case 'v' */
    printf("Test 1: -v flag (verbose)\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v test_coverage_data/test_a.gcda test_coverage_data/test_a.gcno 2>&1 | head -20", 
             gcov_tool);
    execute_command(cmd);
    printf("\n");
    
    /* Test 2: -f flag (function level) - triggers case 'f' */
    printf("Test 2: -f flag (function level)\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -f test_coverage_data/test_a.gcda test_coverage_data/test_a.gcno 2>&1 | head -20", 
             gcov_tool);
    execute_command(cmd);
    printf("\n");
    
    /* Test 3: -F flag (fullname) - triggers case 'F' */
    printf("Test 3: -F flag (fullname)\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -F test_coverage_data/test_a.gcda test_coverage_data/test_a.gcno 2>&1 | head -20", 
             gcov_tool);
    execute_command(cmd);
    printf("\n");
    
    /* Test 4: -o flag (object level) - triggers case 'o' */
    printf("Test 4: -o flag (object level)\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -o test_coverage_data/test_a.gcda test_coverage_data/test_a.gcno 2>&1 | head -20", 
             gcov_tool);
    execute_command(cmd);
    printf("\n");
    
    /* Test 5: -h flag (hot only) - triggers case 'h' */
    printf("Test 5: -h flag (hot only)\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -h test_coverage_data/test_a.gcda test_coverage_data/test_a.gcno 2>&1 | head -20", 
             gcov_tool);
    execute_command(cmd);
    printf("\n");
    
    /* Test 6: -t flag with threshold - triggers case 't' */
    printf("Test 6: -t flag with threshold 0.5\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.5 test_coverage_data/test_a.gcda test_coverage_data/test_a.gcno 2>&1 | head -20", 
             gcov_tool);
    execute_command(cmd);
    printf("\n");
    
    /* Test 7: -t flag with different threshold - triggers case 't' again */
    printf("Test 7: -t flag with threshold 0.75\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.75 test_coverage_data/test_b.gcda test_coverage_data/test_b.gcno 2>&1 | head -20", 
             gcov_tool);
    execute_command(cmd);
    printf("\n");
    
    /* Test 8: Combination of multiple flags */
    printf("Test 8: Combination -v -f -o flags\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f -o test_coverage_data/test_b.gcda test_coverage_data/test_b.gcno 2>&1 | head -20", 
             gcov_tool);
    execute_command(cmd);
    printf("\n");
    
    /* Test 9: Another combination with -F and -h */
    printf("Test 9: Combination -F -h flags\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -F -h test_coverage_data/test_c_main.gcda test_coverage_data/test_c_main.gcno 2>&1 | head -20", 
             gcov_tool);
    execute_command(cmd);
    printf("\n");
    
    /* Test 10: Full combination with threshold */
    printf("Test 10: Full combination -v -f -F -o -h -t 0.3\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f -F -o -h -t 0.3 test_coverage_data/test_c_lib.gcda test_coverage_data/test_c_lib.gcno 2>&1 | head -20", 
             gcov_tool);
    execute_command(cmd);
    printf("\n");
    
    /* Test 11: Invalid option to trigger default case and overlap_usage() */
    printf("Test 11: Invalid option -z (triggers default case)\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -z 2>&1", gcov_tool);
    execute_command(cmd);
    printf("\n");
    
    /* Test 12: Another invalid option combination */
    printf("Test 12: Invalid option -x with valid files (triggers default case)\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -x test_coverage_data/test_d.gcda test_coverage_data/test_d.gcno 2>&1", 
             gcov_tool);
    execute_command(cmd);
    printf("\n");
    
    /* Test 13: Overlap with multiple .gcda files */
    printf("Test 13: Overlap with multiple .gcda files and -v flag\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v test_coverage_data/test_a.gcda test_coverage_data/test_b.gcda test_coverage_data/test_a.gcno test_coverage_data/test_b.gcno 2>&1 | head -30", 
             gcov_tool);
    execute_command(cmd);
    printf("\n");
    
    /* Test 14: Overlap with zero-coverage file and threshold */
    printf("Test 14: Zero-coverage file with -t 0.1\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.1 test_coverage_data/test_d.gcda test_coverage_data/test_d.gcno 2>&1 | head -20", 
             gcov_tool);
    execute_command(cmd);
    
    /* ============================================
     * Cleanup
     * ============================================ */
    printf("\n=== Cleaning up test files ===\n");
    
    /* Remove test directory and all generated files */
    execute_command("rm -rf test_coverage_data");
    
    printf("\n=== Test completed ===\n");
    printf("All gcov-tool overlap invocations have been executed.\n");
    printf("The following switch cases in gcov-tool.cc should now be covered:\n");
    printf("  - case 'v': verbose mode\n");
    printf("  - case 'f': function level overlap\n");
    printf("  - case 'F': fullname mode\n");
    printf("  - case 'o': object level overlap\n");
    printf("  - case 'h': hot only mode\n");
    printf("  - case 't': threshold with atof()\n");
    printf("  - default: invalid option (triggers overlap_usage())\n");
    
    return ret;
}
