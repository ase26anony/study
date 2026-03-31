/*
 * gcov_tool_overlap_test.c
 * 
 * Test harness to trigger uncovered lines in gcov-tool.cc (lines 534-554)
 * Specifically tests the 'overlap' subcommand argument parsing.
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
const char *test_program_a = 
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
const char *test_program_b = 
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
const char *test_program_c1 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"int main() {\n"
"    int result = add(5, 3);\n"
"    result = multiply(result, 2);\n"
"    printf(\"Result: %d\\n\", result);\n"
"    return 0;\n"
"}\n";

/* Scenario C: Multiple source files - part 2 */
const char *test_program_c2 = 
"int add(int a, int b) {\n"
"    return a + b;\n"
"}\n"
"int multiply(int a, int b) {\n"
"    return a * b;\n"
"}\n";

/* Scenario C: Header file */
const char *test_program_ch = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"int add(int a, int b);\n"
"int multiply(int a, int b);\n"
"#endif\n";

/* Scenario D: Program that may produce zero counts */
const char *test_program_d = 
"#include <stdio.h>\n"
"int main(int argc, char **argv) {\n"
"    /* This code path may not be executed */\n"
"    if (argc > 100) {  /* Never true */\n"
"        printf(\"This never runs\\n\");\n"
"        int i;\n"
"        for (i = 0; i < 10; i++) {\n"
"            printf(\"Loop %d\\n\", i);\n"
"        }\n"
"    }\n"
"    return 0;\n"
"}\n";

/* Helper function to write a string to a file */
int write_to_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    fputs(content, f);
    fclose(f);
    return 1;
}

/* Execute a command and check return status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        printf("Command failed with status %d: %s\n", status, cmd);
    }
    return (status == 0);
}

/* Find gcov-tool in common locations */
const char *find_gcov_tool() {
    /* Try common locations */
    const char *locations[] = {
        "./gcov-tool",
        "../gcov-tool",
        "../../gcov-tool",
        "/usr/bin/gcov-tool",
        "/usr/local/bin/gcov-tool",
        "gcov-tool",  /* Try PATH */
        NULL
    };
    
    for (int i = 0; locations[i]; i++) {
        if (access(locations[i], X_OK) == 0) {
            return locations[i];
        }
    }
    
    return NULL;
}

/* Test a specific flag combination */
void test_flag_combination(const char *gcov_tool, const char *gcda_file, 
                          const char *gcno_file, const char *flags) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s overlap %s %s %s", 
             gcov_tool, flags, gcda_file, gcno_file);
    execute_command(cmd);
}

int main(int argc, char **argv) {
    const char *gcov_tool = find_gcov_tool();
    if (!gcov_tool) {
        fprintf(stderr, "Error: gcov-tool not found in common locations or PATH\n");
        fprintf(stderr, "Please build gcov-tool with coverage flags first:\n");
        fprintf(stderr, "  g++ -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
        return 1;
    }
    
    printf("Using gcov-tool at: %s\n", gcov_tool);
    
    /* Create test directory */
    if (mkdir("gcov_test_dir", 0755) != 0 && errno != EEXIST) {
        perror("mkdir");
        return 1;
    }
    chdir("gcov_test_dir");
    
    /* Clean up any previous test files */
    system("rm -f *.gcda *.gcno *.o *.exe a.out test_* *.h 2>/dev/null");
    
    /* ============================================
     * Generate coverage data from various scenarios
     * ============================================ */
    
    /* Scenario A: Simple function */
    printf("\n=== Generating Scenario A (Simple function) ===\n");
    if (!write_to_file("test_a.c", test_program_a)) return 1;
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_a.c -o test_a");
    execute_command("./test_a");
    
    /* Scenario B: Loop heavy - run multiple times with different inputs */
    printf("\n=== Generating Scenario B (Loop heavy) ===\n");
    if (!write_to_file("test_b.c", test_program_b)) return 1;
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_b.c -o test_b");
    execute_command("./test_b 5");
    execute_command("./test_b 8");
    execute_command("./test_b 3");
    
    /* Scenario C: Multiple source files */
    printf("\n=== Generating Scenario C (Multiple files) ===\n");
    if (!write_to_file("test_c.h", test_program_ch)) return 1;
    if (!write_to_file("test_c1.c", test_program_c1)) return 1;
    if (!write_to_file("test_c2.c", test_program_c2)) return 1;
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_c1.c test_c2.c -o test_c");
    execute_command("./test_c");
    
    /* Scenario D: Zero counts */
    printf("\n=== Generating Scenario D (Zero counts) ===\n");
    if (!write_to_file("test_d.c", test_program_d)) return 1;
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_d.c -o test_d");
    /* Don't run it - or run without triggering the instrumented code */
    execute_command("./test_d");
    
    /* ============================================
     * Test gcov-tool overlap with various flags
     * Targeting lines 534-554 in gcov-tool.cc
     * ============================================ */
    
    printf("\n=== Testing gcov-tool overlap flags ===\n");
    
    /* Test 1: -v flag (verbose) - triggers case 'v' */
    printf("\n--- Test 1: -v flag (verbose) ---\n");
    test_flag_combination(gcov_tool, "test_a.gcda", "test_a.gcno", "-v");
    
    /* Test 2: -f flag (function level) - triggers case 'f' */
    printf("\n--- Test 2: -f flag (function level) ---\n");
    test_flag_combination(gcov_tool, "test_b.gcda", "test_b.gcno", "-f");
    
    /* Test 3: -F flag (fullname) - triggers case 'F' */
    printf("\n--- Test 3: -F flag (fullname) ---\n");
    test_flag_combination(gcov_tool, "test_c1.gcda", "test_c1.gcno", "-F");
    
    /* Test 4: -o flag (object level) - triggers case 'o' */
    printf("\n--- Test 4: -o flag (object level) ---\n");
    test_flag_combination(gcov_tool, "test_a.gcda", "test_a.gcno", "-o");
    
    /* Test 5: -h flag (hot only) - triggers case 'h' */
    printf("\n--- Test 5: -h flag (hot only) ---\n");
    test_flag_combination(gcov_tool, "test_b.gcda", "test_b.gcno", "-h");
    
    /* Test 6: -t flag with threshold - triggers case 't' */
    printf("\n--- Test 6: -t flag with threshold ---\n");
    test_flag_combination(gcov_tool, "test_b.gcda", "test_b.gcno", "-t 0.5");
    test_flag_combination(gcov_tool, "test_b.gcda", "test_b.gcno", "-t 0.75");
    test_flag_combination(gcov_tool, "test_b.gcda", "test_b.gcno", "-t 0.0");
    test_flag_combination(gcov_tool, "test_b.gcda", "test_b.gcno", "-t 1.0");
    
    /* Test 7: Combination of flags */
    printf("\n--- Test 7: Combination of flags ---\n");
    test_flag_combination(gcov_tool, "test_a.gcda", "test_a.gcno", "-v -f -o");
    test_flag_combination(gcov_tool, "test_c1.gcda", "test_c1.gcno", "-F -h -t 0.3");
    
    /* Test 8: Invalid option - triggers default case and overlap_usage() */
    printf("\n--- Test 8: Invalid option (triggers default case) ---\n");
    char invalid_cmd[1024];
    snprintf(invalid_cmd, sizeof(invalid_cmd), "%s overlap -z test_a.gcda test_a.gcno 2>&1", gcov_tool);
    execute_command(invalid_cmd);
    
    /* Test 9: Multiple input files with flags */
    printf("\n--- Test 9: Multiple input files with flags ---\n");
    char multi_cmd[2048];
    snprintf(multi_cmd, sizeof(multi_cmd), 
             "%s overlap -v -f test_a.gcda test_b.gcda test_c1.gcda test_a.gcno test_b.gcno test_c1.gcno",
             gcov_tool);
    execute_command(multi_cmd);
    
    /* Test 10: Test with zero-count files */
    printf("\n--- Test 10: Zero-count files with hot threshold ---\n");
    test_flag_combination(gcov_tool, "test_d.gcda", "test_d.gcno", "-t 0.1");
    
    /* ============================================
     * Cleanup (optional - comment out for debugging)
     * ============================================ */
    printf("\n=== Cleaning up test files ===\n");
    chdir("..");
    system("rm -rf gcov_test_dir");
    
    printf("\n=== Test completed ===\n");
    printf("The following gcov-tool overlap flags were tested:\n");
    printf("  -v (verbose)           - case 'v'\n");
    printf("  -f (function level)    - case 'f'\n");
    printf("  -F (fullname)          - case 'F'\n");
    printf("  -o (object level)      - case 'o'\n");
    printf("  -h (hot only)          - case 'h'\n");
    printf("  -t (threshold)         - case 't'\n");
    printf("  -z (invalid)           - default case\n");
    
    return 0;
}
