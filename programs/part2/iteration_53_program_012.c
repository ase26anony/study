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
"    func1(1);\n"
"    func1(-1);\n"
"    return 0;\n"
"}\n";

/* Scenario B: Loop-heavy program */
const char *test_prog_b = 
"#include <stdio.h>\n"
"int main(int argc, char **argv) {\n"
"    int i, j, iterations = 3;\n"
"    if (argc > 1) iterations = atoi(argv[1]);\n"
"    \n"
"    for (i = 0; i < iterations; i++) {\n"
"        for (j = 0; j < i; j++) {\n"
"            printf(\"i=%d, j=%d\\n\", i, j);\n"
"        }\n"
"    }\n"
"    \n"
"    int sum = 0;\n"
"    for (i = 0; i < 100; i++) {\n"
"        sum += i;\n"
"    }\n"
"    printf(\"Sum: %d\\n\", sum);\n"
"    return 0;\n"
"}\n";

/* Scenario C: Multiple source files - part 1 */
const char *test_prog_c1 = 
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
const char *test_prog_c2 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"void helper2(void) {\n"
"    printf(\"Helper2 called\\n\");\n"
"}\n";

/* Scenario C: Header file */
const char *test_header_c = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"void helper1(void);\n"
"void helper2(void);\n"
"#endif\n";

/* Scenario D: Program that may produce zero counts */
const char *test_prog_d = 
"#include <stdio.h>\n"
"int main(int argc, char **argv) {\n"
"    /* This path may not be executed */\n"
"    if (argc > 100) {  /* Never true */\n"
"        printf(\"This never runs\\n\");\n"
"        int i;\n"
"        for (i = 0; i < 10; i++) {\n"
"            printf(\"Loop %d\\n\", i);\n"
"        }\n"
"    }\n"
"    return 0;\n"
"}\n";

/* Function to write a string to a file */
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

/* Execute a command and check return status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        printf("Command failed with status %d: %s\n", status, cmd);
        return 0;
    }
    return 1;
}

/* Check if gcov-tool exists in PATH or current directory */
int check_gcov_tool() {
    if (system("which gcov-tool > /dev/null 2>&1") == 0) {
        return 1;
    }
    
    /* Check in current directory */
    if (access("./gcov-tool", X_OK) == 0) {
        return 1;
    }
    
    printf("Error: gcov-tool not found in PATH or current directory\n");
    printf("Please build gcov-tool with: gcc -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
    return 0;
}

/* Main test function */
int main(int argc, char **argv) {
    printf("=== Starting gcov-tool overlap argument parsing test ===\n\n");
    
    /* Check if gcov-tool is available */
    if (!check_gcov_tool()) {
        return 1;
    }
    
    /* Determine gcov-tool path */
    char gcov_tool_cmd[1024] = "gcov-tool";
    if (access("./gcov-tool", X_OK) == 0) {
        strcpy(gcov_tool_cmd, "./gcov-tool");
    }
    
    /* Create test directory */
    if (system("mkdir -p test_coverage_data") != 0) {
        printf("Failed to create test directory\n");
        return 1;
    }
    
    /* Change to test directory */
    if (chdir("test_coverage_data") != 0) {
        perror("chdir");
        return 1;
    }
    
    printf("--- Generating test coverage data ---\n");
    
    /* Scenario A: Simple function */
    printf("\nScenario A: Simple function with branches\n");
    if (!write_file("test_a.c", test_prog_a)) return 1;
    if (!execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_a.c -o test_a")) return 1;
    if (!execute_command("./test_a")) return 1;
    
    /* Scenario B: Loop-heavy program */
    printf("\nScenario B: Loop-heavy program\n");
    if (!write_file("test_b.c", test_prog_b)) return 1;
    if (!execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_b.c -o test_b")) return 1;
    if (!execute_command("./test_b 5")) return 1;  /* Run with iterations=5 */
    if (!execute_command("./test_b 2")) return 1;  /* Run again with different count */
    
    /* Scenario C: Multiple source files */
    printf("\nScenario C: Multiple source files\n");
    if (!write_file("test_c.h", test_header_c)) return 1;
    if (!write_file("test_c1.c", test_prog_c1)) return 1;
    if (!write_file("test_c2.c", test_prog_c2)) return 1;
    if (!execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_c1.c test_c2.c -o test_c -I.")) return 1;
    if (!execute_command("./test_c")) return 1;
    
    /* Scenario D: Program with potential zero counts */
    printf("\nScenario D: Program that may produce zero counts\n");
    if (!write_file("test_d.c", test_prog_d)) return 1;
    if (!execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_d.c -o test_d")) return 1;
    /* Don't run it - this will produce .gcda files with zero counts */
    
    printf("\n--- Testing gcov-tool overlap with various flags ---\n");
    
    /* Test 1: -v flag (verbose) - triggers case 'v' */
    printf("\nTest 1: Testing -v flag (verbose)\n");
    if (!execute_command(gcov_tool_cmd " overlap -v test_a.gcda test_a.gcno")) return 1;
    
    /* Test 2: -f flag (function level) - triggers case 'f' */
    printf("\nTest 2: Testing -f flag (function level)\n");
    if (!execute_command(gcov_tool_cmd " overlap -f test_a.gcda test_b.gcda")) return 1;
    
    /* Test 3: -F flag (fullname) - triggers case 'F' */
    printf("\nTest 3: Testing -F flag (fullname)\n");
    if (!execute_command(gcov_tool_cmd " overlap -F test_a.gcda test_b.gcda")) return 1;
    
    /* Test 4: -o flag (object level) - triggers case 'o' */
    printf("\nTest 4: Testing -o flag (object level)\n");
    if (!execute_command(gcov_tool_cmd " overlap -o test_a.gcda test_b.gcda")) return 1;
    
    /* Test 5: -h flag (hot only) - triggers case 'h' */
    printf("\nTest 5: Testing -h flag (hot only)\n");
    if (!execute_command(gcov_tool_cmd " overlap -h test_a.gcda test_b.gcda")) return 1;
    
    /* Test 6: -t flag with threshold - triggers case 't' */
    printf("\nTest 6: Testing -t flag with threshold 0.5\n");
    if (!execute_command(gcov_tool_cmd " overlap -t 0.5 test_a.gcda test_b.gcda")) return 1;
    
    /* Test 7: -t flag with different threshold */
    printf("\nTest 7: Testing -t flag with threshold 0.75\n");
    if (!execute_command(gcov_tool_cmd " overlap -t 0.75 test_a.gcda test_b.gcda")) return 1;
    
    /* Test 8: -t flag with threshold 0.0 */
    printf("\nTest 8: Testing -t flag with threshold 0.0\n");
    if (!execute_command(gcov_tool_cmd " overlap -t 0.0 test_a.gcda test_b.gcda")) return 1;
    
    /* Test 9: -t flag with threshold 1.0 */
    printf("\nTest 9: Testing -t flag with threshold 1.0\n");
    if (!execute_command(gcov_tool_cmd " overlap -t 1.0 test_a.gcda test_b.gcda")) return 1;
    
    /* Test 10: Combination of flags */
    printf("\nTest 10: Testing combination -v -f -o\n");
    if (!execute_command(gcov_tool_cmd " overlap -v -f -o test_a.gcda test_b.gcda")) return 1;
    
    /* Test 11: Another combination */
    printf("\nTest 11: Testing combination -F -h -t 0.3\n");
    if (!execute_command(gcov_tool_cmd " overlap -F -h -t 0.3 test_a.gcda test_b.gcda")) return 1;
    
    /* Test 12: With multiple input files */
    printf("\nTest 12: Testing with multiple .gcda files\n");
    if (!execute_command(gcov_tool_cmd " overlap -v test_a.gcda test_b.gcda test_c1.gcda")) return 1;
    
    /* Test 13: With .gcno files (different input type) */
    printf("\nTest 13: Testing with .gcno files\n");
    if (!execute_command(gcov_tool_cmd " overlap -f test_a.gcno test_b.gcno")) return 1;
    
    /* Test 14: Mixed .gcda and .gcno files */
    printf("\nTest 14: Testing with mixed .gcda and .gcno files\n");
    if (!execute_command(gcov_tool_cmd " overlap -o test_a.gcda test_b.gcno")) return 1;
    
    /* Test 15: Using program with zero counts (test_d) */
    printf("\nTest 15: Testing with zero-count coverage data\n");
    if (!execute_command(gcov_tool_cmd " overlap -t 0.1 test_a.gcda test_d.gcda")) return 1;
    
    /* Test 16: Invalid option - triggers default case and overlap_usage() */
    printf("\nTest 16: Testing invalid option -z (triggers default case)\n");
    /* We expect this to fail, so we don't check the return status */
    system(gcov_tool_cmd " overlap -z 2>&1");
    
    /* Test 17: Another invalid option */
    printf("\nTest 17: Testing invalid option --invalid (triggers default case)\n");
    system(gcov_tool_cmd " overlap --invalid 2>&1");
    
    /* Test 18: Valid flag with invalid argument for -t */
    printf("\nTest 18: Testing -t with non-numeric argument (may trigger error handling)\n");
    system(gcov_tool_cmd " overlap -t not_a_number test_a.gcda test_b.gcda 2>&1");
    
    /* Test 19: Edge case - empty argument list */
    printf("\nTest 19: Testing with no input files\n");
    system(gcov_tool_cmd " overlap -v 2>&1");
    
    /* Test 20: All flags together */
    printf("\nTest 20: Testing all flags together\n");
    if (!execute_command(gcov_tool_cmd " overlap -v -f -F -o -h -t 0.5 test_a.gcda test_b.gcda")) return 1;
    
    printf("\n--- Cleaning up ---\n");
    
    /* Change back to parent directory */
    chdir("..");
    
    /* Optional: Remove test directory */
    char cleanup;
    printf("\nRemove test_coverage_data directory? (y/n): ");
    cleanup = getchar();
    if (cleanup == 'y' || cleanup == 'Y') {
        if (system("rm -rf test_coverage_data") == 0) {
            printf("Test directory removed.\n");
        }
    } else {
        printf("Test files preserved in test_coverage_data/\n");
    }
    
    printf("\n=== Test completed ===\n");
    printf("The following gcov-tool overlap flags were tested:\n");
    printf("  -v (verbose)           - triggers case 'v' and gcov_set_verbose()\n");
    printf("  -f (func level)        - triggers case 'f' and sets overlap_func_level\n");
    printf("  -F (fullname)          - triggers case 'F' and sets overlap_use_fullname\n");
    printf("  -o (object level)      - triggers case 'o' and sets overlap_obj_level\n");
    printf("  -h (hot only)          - triggers case 'h' and sets overlap_hot_only\n");
    printf("  -t (threshold)         - triggers case 't' and sets overlap_hot_threshold\n");
    printf("  -z (invalid)           - triggers default case and overlap_usage()\n");
    
    return 0;
}
