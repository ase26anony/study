/* test_gcov_tool_overlap.c
 * 
 * This program tests the gcov-tool overlap command-line argument parsing
 * by generating coverage data and invoking gcov-tool with various flags.
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

#define MAX_PATH 1024
#define MAX_CMD 4096

/* Simple test programs to generate coverage data */

/* Scenario A: Simple function with basic branches */
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
"    int sum = 0;\n"
"    for (i = 0; i < n; i++) {\n"
"        for (j = 0; j < n; j++) {\n"
"            sum += i * j;\n"
"        }\n"
"    }\n"
"    \n"
"    int k = 0;\n"
"    while (k < n) {\n"
"        printf(\"k=%d\\n\", k);\n"
"        k++;\n"
"    }\n"
"    \n"
"    printf(\"Sum: %d\\n\", sum);\n"
"    return 0;\n"
"}\n";

/* Scenario C: Multiple source files (part 1) */
const char *test_prog_c1 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"void helper1(int x) {\n"
"    if (x % 2 == 0) {\n"
"        printf(\"Even\\n\");\n"
"    } else {\n"
"        printf(\"Odd\\n\");\n"
"    }\n"
"}\n"
"int main() {\n"
"    helper1(4);\n"
"    helper1(7);\n"
"    helper2(10);\n"
"    return 0;\n"
"}\n";

const char *test_prog_c2 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"void helper2(int y) {\n"
"    for (int i = 0; i < y; i++) {\n"
"        if (i < 5) {\n"
"            printf(\"Small: %d\\n\", i);\n"
"        }\n"
"    }\n"
"}\n";

const char *test_prog_c_header = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"void helper1(int x);\n"
"void helper2(int y);\n"
"#endif\n";

/* Scenario D: Program that may produce zero counts */
const char *test_prog_d = 
"#include <stdio.h>\n"
"int main(int argc, char **argv) {\n"
"    /* This path is never taken when argc == 1 */\n"
"    if (argc > 10) {  /* Never true in our test */\n"
"        printf(\"This should not print\\n\");\n"
"        int x = 0;\n"
"        while (x < 5) {\n"
"            printf(\"Loop: %d\\n\", x);\n"
"            x++;\n"
"        }\n"
"    }\n"
"    return 0;\n"
"}\n";

/* Helper function to execute a command and check status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Command failed with status %d: %s\n", status, cmd);
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
    static char path[MAX_PATH];
    
    /* Check current directory first */
    if (access("./gcov-tool", X_OK) == 0) {
        strcpy(path, "./gcov-tool");
        return path;
    }
    
    /* Check in PATH */
    char *path_env = getenv("PATH");
    if (path_env) {
        char *path_copy = strdup(path_env);
        char *dir = strtok(path_copy, ":");
        
        while (dir) {
            snprintf(path, MAX_PATH, "%s/gcov-tool", dir);
            if (access(path, X_OK) == 0) {
                free(path_copy);
                return path;
            }
            dir = strtok(NULL, ":");
        }
        free(path_copy);
    }
    
    /* Not found */
    return NULL;
}

int main(int argc, char **argv) {
    char cmd[MAX_CMD];
    char *gcov_tool_path;
    int status;
    
    printf("=== Testing gcov-tool overlap command-line parsing ===\n");
    
    /* Find gcov-tool */
    gcov_tool_path = find_gcov_tool();
    if (!gcov_tool_path) {
        fprintf(stderr, "Error: gcov-tool not found in PATH or current directory\n");
        fprintf(stderr, "Please build gcov-tool with coverage flags first:\n");
        fprintf(stderr, "  g++ -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
        return 1;
    }
    printf("Using gcov-tool at: %s\n", gcov_tool_path);
    
    /* Create test directory */
    execute_command("mkdir -p test_coverage_data");
    
    /* ============================================================
     * Generate coverage data for different scenarios
     * ============================================================ */
    
    /* Scenario A: Simple function */
    printf("\n--- Generating Scenario A coverage data ---\n");
    write_to_file("test_coverage_data/test_a.c", test_prog_a);
    snprintf(cmd, MAX_CMD, 
             "cd test_coverage_data && "
             "gcc -O0 -fprofile-arcs -ftest-coverage test_a.c -o test_a");
    execute_command(cmd);
    execute_command("cd test_coverage_data && ./test_a");
    
    /* Scenario B: Loop heavy (run multiple times with different inputs) */
    printf("\n--- Generating Scenario B coverage data ---\n");
    write_to_file("test_coverage_data/test_b.c", test_prog_b);
    snprintf(cmd, MAX_CMD,
             "cd test_coverage_data && "
             "gcc -O0 -fprofile-arcs -ftest-coverage test_b.c -o test_b");
    execute_command(cmd);
    execute_command("cd test_coverage_data && ./test_b 2");
    execute_command("cd test_coverage_data && ./test_b 5");
    execute_command("cd test_coverage_data && ./test_b 1");
    
    /* Scenario C: Multiple source files */
    printf("\n--- Generating Scenario C coverage data ---\n");
    write_to_file("test_coverage_data/test_c1.c", test_prog_c1);
    write_to_file("test_coverage_data/test_c2.c", test_prog_c2);
    write_to_file("test_coverage_data/test_c.h", test_prog_c_header);
    snprintf(cmd, MAX_CMD,
             "cd test_coverage_data && "
             "gcc -O0 -fprofile-arcs -ftest-coverage "
             "-I. test_c1.c test_c2.c -o test_c");
    execute_command(cmd);
    execute_command("cd test_coverage_data && ./test_c");
    
    /* Scenario D: Zero counts */
    printf("\n--- Generating Scenario D coverage data ---\n");
    write_to_file("test_coverage_data/test_d.c", test_prog_d);
    snprintf(cmd, MAX_CMD,
             "cd test_coverage_data && "
             "gcc -O0 -fprofile-arcs -ftest-coverage test_d.c -o test_d");
    execute_command(cmd);
    /* Run with argc=1 (no arguments) to skip all instrumented code */
    execute_command("cd test_coverage_data && ./test_d");
    
    /* ============================================================
     * Test gcov-tool overlap with various flag combinations
     * Targeting the uncovered switch cases in gcov-tool.cc
     * ============================================================ */
    
    printf("\n=== Testing gcov-tool overlap with various flags ===\n");
    
    /* Test 1: -v flag (verbose) - triggers case 'v' */
    printf("\n--- Test 1: Testing -v flag (verbose) ---\n");
    snprintf(cmd, MAX_CMD,
             "%s overlap -v test_coverage_data/test_a.gcda test_coverage_data/test_a.gcno",
             gcov_tool_path);
    execute_command(cmd);
    
    /* Test 2: -f flag (function level) - triggers case 'f' */
    printf("\n--- Test 2: Testing -f flag (function level) ---\n");
    snprintf(cmd, MAX_CMD,
             "%s overlap -f test_coverage_data/test_a.gcda test_coverage_data/test_b.gcda",
             gcov_tool_path);
    execute_command(cmd);
    
    /* Test 3: -F flag (fullname) - triggers case 'F' */
    printf("\n--- Test 3: Testing -F flag (fullname) ---\n");
    snprintf(cmd, MAX_CMD,
             "%s overlap -F test_coverage_data/test_a.gcda test_coverage_data/test_b.gcda",
             gcov_tool_path);
    execute_command(cmd);
    
    /* Test 4: -o flag (object level) - triggers case 'o' */
    printf("\n--- Test 4: Testing -o flag (object level) ---\n");
    snprintf(cmd, MAX_CMD,
             "%s overlap -o test_coverage_data/test_a.gcda test_coverage_data/test_b.gcda",
             gcov_tool_path);
    execute_command(cmd);
    
    /* Test 5: -h flag (hot only) - triggers case 'h' */
    printf("\n--- Test 5: Testing -h flag (hot only) ---\n");
    snprintf(cmd, MAX_CMD,
             "%s overlap -h test_coverage_data/test_a.gcda test_coverage_data/test_b.gcda",
             gcov_tool_path);
    execute_command(cmd);
    
    /* Test 6: -t flag with threshold - triggers case 't' */
    printf("\n--- Test 6: Testing -t flag with threshold ---\n");
    snprintf(cmd, MAX_CMD,
             "%s overlap -t 0.5 test_coverage_data/test_a.gcda test_coverage_data/test_b.gcda",
             gcov_tool_path);
    execute_command(cmd);
    
    /* Test 7: -t flag with different threshold value */
    printf("\n--- Test 7: Testing -t flag with different threshold ---\n");
    snprintf(cmd, MAX_CMD,
             "%s overlap -t 0.75 test_coverage_data/test_b.gcda test_coverage_data/test_c1.gcda",
             gcov_tool_path);
    execute_command(cmd);
    
    /* Test 8: Combination of multiple flags */
    printf("\n--- Test 8: Testing combination of flags ---\n");
    snprintf(cmd, MAX_CMD,
             "%s overlap -v -f -o test_coverage_data/test_a.gcda test_coverage_data/test_b.gcda",
             gcov_tool_path);
    execute_command(cmd);
    
    /* Test 9: Another combination with -t and -h */
    printf("\n--- Test 9: Testing -t and -h combination ---\n");
    snprintf(cmd, MAX_CMD,
             "%s overlap -h -t 0.3 test_coverage_data/test_c1.gcda test_coverage_data/test_c2.gcda",
             gcov_tool_path);
    execute_command(cmd);
    
    /* Test 10: Invalid option to trigger default case and overlap_usage() */
    printf("\n--- Test 10: Testing invalid option (triggers default case) ---\n");
    snprintf(cmd, MAX_CMD,
             "%s overlap -z 2>&1 | head -20",  /* Capture stderr, show first 20 lines */
             gcov_tool_path);
    execute_command(cmd);
    
    /* Test 11: Test with zero-count data (scenario D) */
    printf("\n--- Test 11: Testing with zero-count coverage data ---\n");
    snprintf(cmd, MAX_CMD,
             "%s overlap -t 0.1 test_coverage_data/test_d.gcda test_coverage_data/test_a.gcda",
             gcov_tool_path);
    execute_command(cmd);
    
    /* Test 12: Test with multiple .gcda files */
    printf("\n--- Test 12: Testing with multiple input files ---\n");
    snprintf(cmd, MAX_CMD,
             "%s overlap -v -f test_coverage_data/test_a.gcda "
             "test_coverage_data/test_b.gcda test_coverage_data/test_c1.gcda",
             gcov_tool_path);
    execute_command(cmd);
    
    /* Test 13: Test with .gcno files as well */
    printf("\n--- Test 13: Testing with .gcno files ---\n");
    snprintf(cmd, MAX_CMD,
             "%s overlap -F -o test_coverage_data/test_a.gcda test_coverage_data/test_a.gcno "
             "test_coverage_data/test_b.gcda test_coverage_data/test_b.gcno",
             gcov_tool_path);
    execute_command(cmd);
    
    /* ============================================================
     * Cleanup
     * ============================================================ */
    
    printf("\n=== Cleaning up test files ===\n");
    
    /* Optionally keep the coverage data for inspection */
    char keep_files = 0;
    if (argc > 1 && strcmp(argv[1], "--keep") == 0) {
        keep_files = 1;
        printf("Keeping test files in test_coverage_data/\n");
    } else {
        snprintf(cmd, MAX_CMD, "rm -rf test_coverage_data");
        execute_command(cmd);
        printf("Test directory cleaned up\n");
    }
    
    printf("\n=== Test completed ===\n");
    printf("The following gcov-tool overlap flags were tested:\n");
    printf("  -v (verbose)           - case 'v'\n");
    printf("  -f (function level)    - case 'f'\n");
    printf("  -F (fullname)          - case 'F'\n");
    printf("  -o (object level)      - case 'o'\n");
    printf("  -h (hot only)          - case 'h'\n");
    printf("  -t (threshold)         - case 't' with atof(optarg)\n");
    printf("  -z (invalid)           - default case -> overlap_usage()\n");
    
    return 0;
}
