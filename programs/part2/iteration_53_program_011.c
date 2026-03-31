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

#define MAX_PATH 1024
#define MAX_CMD 4096

/* Utility function to execute a shell command and check status */
static int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Command failed with status %d: %s\n", status, cmd);
    }
    return status;
}

/* Create and compile test C programs with coverage instrumentation */
static void create_and_compile_test_programs(void) {
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
        "    func1(1);\n"
        "    func1(-1);\n"
        "    return 0;\n"
        "}\n";
    
    FILE *fp = fopen("test_a.c", "w");
    if (fp) {
        fputs(test_a, fp);
        fclose(fp);
        execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_a.c -o test_a");
    }
    
    /* Scenario B: Loop-heavy program */
    const char *test_b = 
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "int process_value(int val) {\n"
        "    int sum = 0;\n"
        "    for (int i = 0; i < val; i++) {\n"
        "        for (int j = 0; j < i; j++) {\n"
        "            sum += j;\n"
        "        }\n"
        "    }\n"
        "    return sum;\n"
        "}\n"
        "int main(int argc, char **argv) {\n"
        "    int iterations = (argc > 1) ? atoi(argv[1]) : 5;\n"
        "    int total = 0;\n"
        "    for (int k = 0; k < iterations; k++) {\n"
        "        total += process_value(k + 1);\n"
        "    }\n"
        "    printf(\"Total: %d\\n\", total);\n"
        "    return 0;\n"
        "}\n";
    
    fp = fopen("test_b.c", "w");
    if (fp) {
        fputs(test_b, fp);
        fclose(fp);
        execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_b.c -o test_b");
    }
    
    /* Scenario C: Multiple source files */
    const char *test_c1 = 
        "#include <stdio.h>\n"
        "#include \"test_c.h\"\n"
        "int helper1(int x) {\n"
        "    return x * 2;\n"
        "}\n"
        "int main() {\n"
        "    int val = helper1(5);\n"
        "    val = helper2(val);\n"
        "    printf(\"Result: %d\\n\", val);\n"
        "    return 0;\n"
        "}\n";
    
    const char *test_c2 = 
        "#include \"test_c.h\"\n"
        "int helper2(int y) {\n"
        "    if (y > 10) {\n"
        "        return y / 2;\n"
        "    }\n"
        "    return y + 5;\n"
        "}\n";
    
    const char *test_c_header = 
        "#ifndef TEST_C_H\n"
        "#define TEST_C_H\n"
        "int helper1(int x);\n"
        "int helper2(int y);\n"
        "#endif\n";
    
    fp = fopen("test_c1.c", "w");
    if (fp) {
        fputs(test_c1, fp);
        fclose(fp);
    }
    
    fp = fopen("test_c2.c", "w");
    if (fp) {
        fputs(test_c2, fp);
        fclose(fp);
    }
    
    fp = fopen("test_c.h", "w");
    if (fp) {
        fputs(test_c_header, fp);
        fclose(fp);
    }
    
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_c1.c test_c2.c -o test_c");
    
    /* Scenario D: Program with zero counts (not executed) */
    const char *test_d = 
        "#include <stdio.h>\n"
        "int unused_function(int x) {\n"
        "    if (x > 100) {\n"
        "        return x * 2;\n"
        "    } else {\n"
        "        return x / 2;\n"
        "    }\n"
        "}\n"
        "int main() {\n"
        "    /* Don't call unused_function */\n"
        "    printf(\"Nothing instrumented called\\n\");\n"
        "    return 0;\n"
        "}\n";
    
    fp = fopen("test_d.c", "w");
    if (fp) {
        fputs(test_d, fp);
        fclose(fp);
        execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_d.c -o test_d");
    }
}

/* Run test programs to generate .gcda files */
static void generate_coverage_data(void) {
    /* Run test_a */
    execute_command("./test_a");
    
    /* Run test_b multiple times with different inputs */
    execute_command("./test_b 3");
    execute_command("./test_b 5");
    execute_command("./test_b 2");
    
    /* Run test_c */
    execute_command("./test_c");
    
    /* Compile but DON'T run test_d to get zero counts */
    /* Actually run it but it won't hit instrumented code */
    execute_command("./test_d");
}

/* Execute gcov-tool overlap with various flag combinations */
static void test_gcov_tool_overlap(void) {
    char cmd[MAX_CMD];
    int status;
    
    printf("\n=== Testing gcov-tool overlap with various flags ===\n");
    
    /* Test 1: -v flag (verbose) - triggers case 'v' */
    printf("\n--- Test 1: -v flag ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -v test_a.gcda test_a.gcno 2>&1 | head -20");
    status = execute_command(cmd);
    
    /* Test 2: -f flag (function level) - triggers case 'f' */
    printf("\n--- Test 2: -f flag ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -f test_a.gcda test_b.gcda 2>&1 | head -20");
    status = execute_command(cmd);
    
    /* Test 3: -F flag (fullname) - triggers case 'F' */
    printf("\n--- Test 3: -F flag ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -F test_a.gcda test_b.gcda 2>&1 | head -20");
    status = execute_command(cmd);
    
    /* Test 4: -o flag (object level) - triggers case 'o' */
    printf("\n--- Test 4: -o flag ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -o test_a.gcda test_b.gcda 2>&1 | head -20");
    status = execute_command(cmd);
    
    /* Test 5: -h flag (hot only) - triggers case 'h' */
    printf("\n--- Test 5: -h flag ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -h test_a.gcda test_b.gcda 2>&1 | head -20");
    status = execute_command(cmd);
    
    /* Test 6: -t flag with threshold - triggers case 't' */
    printf("\n--- Test 6: -t flag with threshold 0.5 ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -t 0.5 test_a.gcda test_b.gcda 2>&1 | head -20");
    status = execute_command(cmd);
    
    /* Test 7: -t flag with different threshold - triggers case 't' */
    printf("\n--- Test 7: -t flag with threshold 0.75 ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -t 0.75 test_a.gcda test_b.gcda 2>&1 | head -20");
    status = execute_command(cmd);
    
    /* Test 8: -t flag with threshold 0.0 - triggers case 't' */
    printf("\n--- Test 8: -t flag with threshold 0.0 ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -t 0.0 test_a.gcda test_b.gcda 2>&1 | head -20");
    status = execute_command(cmd);
    
    /* Test 9: Combination of flags - triggers multiple cases */
    printf("\n--- Test 9: Combination -v -f -o ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -v -f -o test_a.gcda test_b.gcda 2>&1 | head -20");
    status = execute_command(cmd);
    
    /* Test 10: Combination -F -h -t */
    printf("\n--- Test 10: Combination -F -h -t 0.3 ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -F -h -t 0.3 test_a.gcda test_b.gcda 2>&1 | head -20");
    status = execute_command(cmd);
    
    /* Test 11: All flags together */
    printf("\n--- Test 11: All flags combined ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -v -f -F -o -h -t 0.25 test_a.gcda test_b.gcda 2>&1 | head -20");
    status = execute_command(cmd);
    
    /* Test 12: Invalid option -z to trigger default case and overlap_usage() */
    printf("\n--- Test 12: Invalid option -z (triggers default case) ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -z 2>&1 | head -10");
    status = execute_command(cmd);
    
    /* Test 13: Test with multiple .gcda files from scenario C */
    printf("\n--- Test 13: Multiple source files with -v -f ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -v -f test_c1.gcda test_c2.gcda 2>&1 | head -20");
    status = execute_command(cmd);
    
    /* Test 14: Test with zero-count file (scenario D) */
    printf("\n--- Test 14: Zero-count file with -t 0.1 ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -t 0.1 test_a.gcda test_d.gcda 2>&1 | head -20");
    status = execute_command(cmd);
    
    /* Test 15: Test with .gcno files as input */
    printf("\n--- Test 15: Using .gcno files with -v ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -v test_a.gcno test_b.gcno 2>&1 | head -20");
    status = execute_command(cmd);
    
    /* Test 16: Mixed .gcda and .gcno files */
    printf("\n--- Test 16: Mixed file types with -o -F ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -o -F test_a.gcda test_b.gcno 2>&1 | head -20");
    status = execute_command(cmd);
}

/* Clean up generated files */
static void cleanup(void) {
    /* Remove test source files */
    remove("test_a.c");
    remove("test_b.c");
    remove("test_c1.c");
    remove("test_c2.c");
    remove("test_c.h");
    remove("test_d.c");
    
    /* Remove executables */
    remove("test_a");
    remove("test_b");
    remove("test_c");
    remove("test_d");
    
    /* Remove coverage files */
    system("rm -f *.gcda *.gcno *.gcov");
}

int main(int argc, char **argv) {
    printf("=== gcov-tool overlap test harness ===\n");
    printf("Target: Trigger lines 534-554 in gcov-tool.cc\n");
    
    /* Check if gcov-tool exists */
    if (system("which gcov-tool > /dev/null 2>&1") != 0) {
        fprintf(stderr, "Error: gcov-tool not found in PATH\n");
        fprintf(stderr, "Please ensure gcov-tool is built and in your PATH\n");
        fprintf(stderr, "You may need to: cd gcc-build-dir && make gcov-tool\n");
        return 1;
    }
    
    /* Create and compile test programs */
    printf("\n--- Creating test programs with coverage instrumentation ---\n");
    create_and_compile_test_programs();
    
    /* Generate coverage data by running test programs */
    printf("\n--- Generating coverage data (.gcda files) ---\n");
    generate_coverage_data();
    
    /* Test gcov-tool overlap with various flag combinations */
    test_gcov_tool_overlap();
    
    /* Cleanup */
    printf("\n--- Cleaning up generated files ---\n");
    cleanup();
    
    printf("\n=== Test completed ===\n");
    printf("The following gcov-tool overlap flags were tested:\n");
    printf("  -v (verbose)           - triggers case 'v'\n");
    printf("  -f (function level)    - triggers case 'f'\n");
    printf("  -F (fullname)          - triggers case 'F'\n");
    printf("  -o (object level)      - triggers case 'o'\n");
    printf("  -h (hot only)          - triggers case 'h'\n");
    printf("  -t (threshold)         - triggers case 't' with atof(optarg)\n");
    printf("  -z (invalid)           - triggers default case and overlap_usage()\n");
    
    return 0;
}
