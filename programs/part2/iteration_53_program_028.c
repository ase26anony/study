/**
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

#define MAX_PATH 1024
#define MAX_CMD 4096

/**
 * Simple utility function to execute a shell command and check status
 */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        printf("Warning: Command returned non-zero: %s\n", cmd);
    }
    return status;
}

/**
 * Create and compile a simple C program for coverage testing
 */
void create_and_compile_test(const char *filename, const char *content) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create test file");
        exit(1);
    }
    fprintf(fp, "%s", content);
    fclose(fp);
    
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "gcc -O0 -fprofile-arcs -ftest-coverage -o %s.exe %s", 
             filename, filename);
    execute_command(cmd);
}

/**
 * Run the compiled test to generate .gcda files
 */
void run_test(const char *basename, const char *args) {
    char cmd[MAX_CMD];
    if (args && args[0]) {
        snprintf(cmd, sizeof(cmd), "./%s.exe %s > /dev/null 2>&1", basename, args);
    } else {
        snprintf(cmd, sizeof(cmd), "./%s.exe > /dev/null 2>&1", basename);
    }
    execute_command(cmd);
}

/**
 * Check if gcov-tool exists in PATH or current directory
 */
int check_gcov_tool() {
    // First check current directory
    if (access("./gcov-tool", X_OK) == 0) {
        return 1;
    }
    
    // Check if it's in PATH
    int status = system("which gcov-tool > /dev/null 2>&1");
    return (WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

/**
 * Main test function
 */
int main(int argc, char *argv[]) {
    printf("=== Starting gcov-tool overlap argument parsing test ===\n\n");
    
    // Check if gcov-tool is available
    if (!check_gcov_tool()) {
        printf("ERROR: gcov-tool not found!\n");
        printf("Please ensure gcov-tool is in current directory or PATH.\n");
        printf("You may need to build it with: gcc -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
        return 1;
    }
    
    printf("Creating test programs with coverage instrumentation...\n\n");
    
    // ============================================
    // Scenario A: Simple function with conditionals
    // ============================================
    const char *testA = 
        "#include <stdio.h>\n"
        "void func1(int x) {\n"
        "    if (x > 0) {\n"
        "        printf(\"Positive\\n\");\n"
        "    } else {\n"
        "        printf(\"Non-positive\\n\");\n"
        "    }\n"
        "}\n"
        "void func2() {\n"
        "    for (int i = 0; i < 3; i++) {\n"
        "        printf(\"Loop %d\\n\", i);\n"
        "    }\n"
        "}\n"
        "int main() {\n"
        "    func1(5);\n"
        "    func1(-2);\n"
        "    func2();\n"
        "    return 0;\n"
        "}\n";
    
    create_and_compile_test("test_simple.c", testA);
    run_test("test_simple", "");
    
    // ============================================
    // Scenario B: Loop-heavy program
    // ============================================
    const char *testB = 
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "int process(int iterations) {\n"
        "    int sum = 0;\n"
        "    for (int i = 0; i < iterations; i++) {\n"
        "        for (int j = 0; j < i; j++) {\n"
        "            sum += j;\n"
        "        }\n"
        "    }\n"
        "    return sum;\n"
        "}\n"
        "int main(int argc, char *argv[]) {\n"
        "    int iter = 10;\n"
        "    if (argc > 1) {\n"
        "        iter = atoi(argv[1]);\n"
        "    }\n"
        "    int result = process(iter);\n"
        "    printf(\"Result: %d\\n\", result);\n"
        "    return 0;\n"
        "}\n";
    
    create_and_compile_test("test_loops.c", testB);
    // Run multiple times with different inputs
    run_test("test_loops", "5");
    run_test("test_loops", "8");
    run_test("test_loops", "3");
    
    // ============================================
    // Scenario C: Multiple source files
    // ============================================
    const char *header = 
        "#ifndef TEST_MULTI_H\n"
        "#define TEST_MULTI_H\n"
        "int helper1(int x);\n"
        "int helper2(int x, int y);\n"
        "#endif\n";
    
    const char *source1 = 
        "#include \"test_multi.h\"\n"
        "#include <stdio.h>\n"
        "int helper1(int x) {\n"
        "    return x * 2;\n"
        "}\n"
        "int main() {\n"
        "    int a = helper1(5);\n"
        "    int b = helper2(a, 3);\n"
        "    printf(\"Result: %d\\n\", b);\n"
        "    return 0;\n"
        "}\n";
    
    const char *source2 = 
        "#include \"test_multi.h\"\n"
        "int helper2(int x, int y) {\n"
        "    if (x > y) {\n"
        "        return x - y;\n"
        "    } else {\n"
        "        return y - x;\n"
        "    }\n"
        "}\n";
    
    FILE *fp = fopen("test_multi.h", "w");
    fputs(header, fp);
    fclose(fp);
    
    fp = fopen("test_multi1.c", "w");
    fputs(source1, fp);
    fclose(fp);
    
    fp = fopen("test_multi2.c", "w");
    fputs(source2, fp);
    fclose(fp);
    
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage -o test_multi.exe test_multi1.c test_multi2.c");
    execute_command("./test_multi.exe > /dev/null 2>&1");
    
    // ============================================
    // Scenario D: Program with zero coverage
    // ============================================
    const char *testD = 
        "#include <stdio.h>\n"
        "int main() {\n"
        "    // This program does nothing when run normally\n"
        "    // but has instrumented code paths\n"
        "    #ifdef NEVER_DEFINED\n"
        "    printf(\"This never executes\\n\");\n"
        "    for (int i = 0; i < 10; i++) {\n"
        "        printf(\"Loop %d\\n\", i);\n"
        "    }\n"
        "    #endif\n"
        "    return 0;\n"
        "}\n";
    
    create_and_compile_test("test_zero.c", testD);
    run_test("test_zero", "");
    
    printf("\n=== Testing gcov-tool overlap with various flags ===\n\n");
    
    // Base gcov-tool command
    const char *gcov_tool = "gcov-tool";
    
    // Test 1: -v flag (verbose)
    printf("Test 1: Testing -v flag (verbose)\n");
    execute_command("gcov-tool overlap -v test_simple.gcda test_simple.gcno");
    
    // Test 2: -f flag (function level)
    printf("\nTest 2: Testing -f flag (function level)\n");
    execute_command("gcov-tool overlap -f test_simple.gcda test_simple.gcno");
    
    // Test 3: -F flag (fullname)
    printf("\nTest 3: Testing -F flag (fullname)\n");
    execute_command("gcov-tool overlap -F test_simple.gcda test_simple.gcno");
    
    // Test 4: -o flag (object level)
    printf("\nTest 4: Testing -o flag (object level)\n");
    execute_command("gcov-tool overlap -o test_simple.gcda test_simple.gcno");
    
    // Test 5: -h flag (hot only)
    printf("\nTest 5: Testing -h flag (hot only)\n");
    execute_command("gcov-tool overlap -h test_simple.gcda test_simple.gcno");
    
    // Test 6: -t flag with threshold
    printf("\nTest 6: Testing -t flag with threshold 0.5\n");
    execute_command("gcov-tool overlap -t 0.5 test_simple.gcda test_simple.gcno");
    
    // Test 7: -t flag with different threshold
    printf("\nTest 7: Testing -t flag with threshold 0.75\n");
    execute_command("gcov-tool overlap -t 0.75 test_simple.gcda test_simple.gcno");
    
    // Test 8: -t flag with threshold 0.0
    printf("\nTest 8: Testing -t flag with threshold 0.0\n");
    execute_command("gcov-tool overlap -t 0.0 test_simple.gcda test_simple.gcno");
    
    // Test 9: Combination of flags
    printf("\nTest 9: Testing combination -v -f -o\n");
    execute_command("gcov-tool overlap -v -f -o test_simple.gcda test_simple.gcno");
    
    // Test 10: Another combination
    printf("\nTest 10: Testing combination -F -h -t 0.3\n");
    execute_command("gcov-tool overlap -F -h -t 0.3 test_simple.gcda test_simple.gcno");
    
    // Test 11: With loop-heavy data
    printf("\nTest 11: Testing with loop-heavy data (-v -f)\n");
    execute_command("gcov-tool overlap -v -f test_loops.gcda test_loops.gcno");
    
    // Test 12: With multiple source files
    printf("\nTest 12: Testing with multiple source files (-o -F)\n");
    execute_command("gcov-tool overlap -o -F test_multi1.gcda test_multi1.gcno test_multi2.gcda test_multi2.gcno");
    
    // Test 13: With zero-coverage data
    printf("\nTest 13: Testing with zero-coverage data (-h -t 0.1)\n");
    execute_command("gcov-tool overlap -h -t 0.1 test_zero.gcda test_zero.gcno");
    
    // Test 14: Invalid option to trigger default case
    printf("\nTest 14: Testing invalid option -z (should trigger default case and usage)\n");
    execute_command("gcov-tool overlap -z 2>&1 | head -20");
    
    // Test 15: Another invalid combination
    printf("\nTest 15: Testing invalid option --invalid (should trigger default case)\n");
    execute_command("gcov-tool overlap --invalid 2>&1 | head -20");
    
    // Test 16: No arguments (should show usage)
    printf("\nTest 16: Testing no arguments (should show usage)\n");
    execute_command("gcov-tool overlap 2>&1 | head -20");
    
    // Test 17: All flags together
    printf("\nTest 17: Testing all valid flags together\n");
    execute_command("gcov-tool overlap -v -f -F -o -h -t 0.25 test_simple.gcda test_simple.gcno");
    
    printf("\n=== Cleanup ===\n");
    
    // Clean up generated files
    execute_command("rm -f test_simple.c test_simple.exe test_simple.gcda test_simple.gcno test_simple.gcov");
    execute_command("rm -f test_loops.c test_loops.exe test_loops.gcda test_loops.gcno test_loops.gcov");
    execute_command("rm -f test_multi.h test_multi1.c test_multi2.c test_multi.exe");
    execute_command("rm -f test_multi1.gcda test_multi1.gcno test_multi2.gcda test_multi2.gcno");
    execute_command("rm -f test_zero.c test_zero.exe test_zero.gcda test_zero.gcno test_zero.gcov");
    execute_command("rm -f *.gcov");
    
    printf("\n=== Test completed ===\n");
    printf("The gcov-tool overlap command has been invoked with all flag combinations.\n");
    printf("Check coverage of gcov-tool.cc lines 534-554 to verify execution.\n");
    
    return 0;
}
