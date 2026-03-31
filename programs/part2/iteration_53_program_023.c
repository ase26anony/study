/**
 * Test harness for gcov-tool overlap command-line parsing coverage.
 * This program generates various coverage data files and invokes
 * gcov-tool overlap with different flag combinations to cover
 * the switch statement cases in gcov-tool.cc lines 534-554.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH 1024

/**
 * Execute a shell command and check return status.
 * Returns 0 on success, non-zero on failure.
 */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Command failed with status %d: %s\n", status, cmd);
    }
    return status;
}

/**
 * Create a simple C program with basic conditional branches.
 */
void create_scenario_a() {
    FILE *f = fopen("scenario_a.c", "w");
    if (!f) {
        perror("Failed to create scenario_a.c");
        exit(1);
    }
    
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int func1(int x) {\n");
    fprintf(f, "    if (x > 0) {\n");
    fprintf(f, "        return x * 2;\n");
    fprintf(f, "    } else {\n");
    fprintf(f, "        return x / 2;\n");
    fprintf(f, "    }\n");
    fprintf(f, "}\n\n");
    fprintf(f, "int func2(int a, int b) {\n");
    fprintf(f, "    return a + b;\n");
    fprintf(f, "}\n\n");
    fprintf(f, "int main() {\n");
    fprintf(f, "    int result1 = func1(10);\n");
    fprintf(f, "    int result2 = func1(-5);\n");
    fprintf(f, "    int sum = func2(result1, result2);\n");
    fprintf(f, "    printf(\"Result: %%d\\n\", sum);\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
}

/**
 * Create a C program with nested loops.
 */
void create_scenario_b() {
    FILE *f = fopen("scenario_b.c", "w");
    if (!f) {
        perror("Failed to create scenario_b.c");
        exit(1);
    }
    
    fprintf(f, "#include <stdio.h>\n");
    fprintf(f, "#include <stdlib.h>\n\n");
    fprintf(f, "int process_data(int iterations) {\n");
    fprintf(f, "    int total = 0;\n");
    fprintf(f, "    for (int i = 0; i < iterations; i++) {\n");
    fprintf(f, "        for (int j = 0; j < i; j++) {\n");
    fprintf(f, "            total += j;\n");
    fprintf(f, "        }\n");
    fprintf(f, "        if (i %% 2 == 0) {\n");
    fprintf(f, "            total += i * 2;\n");
    fprintf(f, "        } else {\n");
    fprintf(f, "            total += i;\n");
    fprintf(f, "        }\n");
    fprintf(f, "    }\n");
    fprintf(f, "    return total;\n");
    fprintf(f, "}\n\n");
    fprintf(f, "int main(int argc, char *argv[]) {\n");
    fprintf(f, "    int iterations = 5;\n");
    fprintf(f, "    if (argc > 1) {\n");
    fprintf(f, "        iterations = atoi(argv[1]);\n");
    fprintf(f, "    }\n");
    fprintf(f, "    int result = process_data(iterations);\n");
    fprintf(f, "    printf(\"Processed %%d iterations, result: %%d\\n\", iterations, result);\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
}

/**
 * Create multiple source files for scenario C.
 */
void create_scenario_c() {
    // Create header file
    FILE *h = fopen("scenario_c.h", "w");
    if (!h) {
        perror("Failed to create scenario_c.h");
        exit(1);
    }
    fprintf(h, "#ifndef SCENARIO_C_H\n");
    fprintf(h, "#define SCENARIO_C_H\n\n");
    fprintf(h, "int helper_function(int x);\n");
    fprintf(h, "void process_helper(int *data, int size);\n\n");
    fprintf(h, "#endif\n");
    fclose(h);
    
    // Create first source file
    FILE *f1 = fopen("scenario_c1.c", "w");
    if (!f1) {
        perror("Failed to create scenario_c1.c");
        exit(1);
    }
    fprintf(f1, "#include \"scenario_c.h\"\n");
    fprintf(f1, "#include <stdio.h>\n\n");
    fprintf(f1, "int helper_function(int x) {\n");
    fprintf(f1, "    if (x > 100) {\n");
    fprintf(f1, "        return x - 50;\n");
    fprintf(f1, "    }\n");
    fprintf(f1, "    return x + 50;\n");
    fprintf(f1, "}\n\n");
    fprintf(f1, "void process_helper(int *data, int size) {\n");
    fprintf(f1, "    for (int i = 0; i < size; i++) {\n");
    fprintf(f1, "        data[i] = helper_function(data[i]);\n");
    fprintf(f1, "    }\n");
    fprintf(f1, "}\n");
    fclose(f1);
    
    // Create second source file
    FILE *f2 = fopen("scenario_c2.c", "w");
    if (!f2) {
        perror("Failed to create scenario_c2.c");
        exit(1);
    }
    fprintf(f2, "#include \"scenario_c.h\"\n");
    fprintf(f2, "#include <stdio.h>\n\n");
    fprintf(f2, "int main() {\n");
    fprintf(f2, "    int data[5] = {10, 20, 150, 200, 30};\n");
    fprintf(f2, "    process_helper(data, 5);\n");
    fprintf(f2, "    printf(\"Processed data:\\n\");\n");
    fprintf(f2, "    for (int i = 0; i < 5; i++) {\n");
    fprintf(f2, "        printf(\"  [%%d] = %%d\\n\", i, data[i]);\n");
    fprintf(f2, "    }\n");
    fprintf(f2, "    return 0;\n");
    fprintf(f2, "}\n");
    fclose(f2);
}

/**
 * Create a C program that may produce zero coverage counts.
 */
void create_scenario_d() {
    FILE *f = fopen("scenario_d.c", "w");
    if (!f) {
        perror("Failed to create scenario_d.c");
        exit(1);
    }
    
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int rarely_called(int x) {\n");
    fprintf(f, "    // This function might not be called\n");
    fprintf(f, "    return x * 3;\n");
    fprintf(f, "}\n\n");
    fprintf(f, "int main() {\n");
    fprintf(f, "    // Minimal execution path\n");
    fprintf(f, "    printf(\"Minimal execution\\n\");\n");
    fprintf(f, "    // Note: rarely_called() is not called\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
}

/**
 * Check if gcov-tool is available in PATH or current directory.
 */
int check_gcov_tool() {
    // First check current directory
    if (access("./gcov-tool", X_OK) == 0) {
        return 1;  // Found in current directory
    }
    
    // Check PATH
    char *path = getenv("PATH");
    if (!path) {
        return 0;
    }
    
    char path_copy[MAX_PATH];
    strncpy(path_copy, path, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';
    
    char *dir = strtok(path_copy, ":");
    while (dir) {
        char tool_path[MAX_PATH];
        snprintf(tool_path, sizeof(tool_path), "%s/gcov-tool", dir);
        if (access(tool_path, X_OK) == 0) {
            return 1;  // Found in PATH
        }
        dir = strtok(NULL, ":");
    }
    
    return 0;
}

/**
 * Main test orchestrator.
 */
int main(int argc, char *argv[]) {
    printf("=== Starting gcov-tool overlap coverage test ===\n");
    
    // Check if gcov-tool is available
    if (!check_gcov_tool()) {
        fprintf(stderr, "Error: gcov-tool not found in PATH or current directory.\n");
        fprintf(stderr, "Please ensure gcov-tool is built and available.\n");
        return 1;
    }
    
    printf("Creating test scenarios...\n");
    
    // Create all test scenarios
    create_scenario_a();
    create_scenario_b();
    create_scenario_c();
    create_scenario_d();
    
    // Compile scenarios with coverage flags
    printf("\nCompiling test scenarios with coverage...\n");
    
    // Scenario A
    if (execute_command("gcc -O0 -fprofile-arcs -ftest-coverage scenario_a.c -o test_a") != 0) {
        fprintf(stderr, "Failed to compile scenario A\n");
    }
    
    // Scenario B
    if (execute_command("gcc -O0 -fprofile-arcs -ftest-coverage scenario_b.c -o test_b") != 0) {
        fprintf(stderr, "Failed to compile scenario B\n");
    }
    
    // Scenario C (multiple files)
    if (execute_command("gcc -O0 -fprofile-arcs -ftest-coverage scenario_c1.c scenario_c2.c -o test_c") != 0) {
        fprintf(stderr, "Failed to compile scenario C\n");
    }
    
    // Scenario D
    if (execute_command("gcc -O0 -fprofile-arcs -ftest-coverage scenario_d.c -o test_d") != 0) {
        fprintf(stderr, "Failed to compile scenario D\n");
    }
    
    // Run the executables to generate .gcda files
    printf("\nRunning test executables to generate coverage data...\n");
    
    // Run scenario A
    if (execute_command("./test_a") != 0) {
        fprintf(stderr, "Failed to run scenario A\n");
    }
    
    // Run scenario B multiple times with different inputs
    if (execute_command("./test_b 3") != 0) {
        fprintf(stderr, "Failed to run scenario B (iteration 1)\n");
    }
    if (execute_command("./test_b 7") != 0) {
        fprintf(stderr, "Failed to run scenario B (iteration 2)\n");
    }
    
    // Run scenario C
    if (execute_command("./test_c") != 0) {
        fprintf(stderr, "Failed to run scenario C\n");
    }
    
    // Run scenario D (minimal execution)
    if (execute_command("./test_d") != 0) {
        fprintf(stderr, "Failed to run scenario D\n");
    }
    
    // Now invoke gcov-tool overlap with various flag combinations
    printf("\n=== Invoking gcov-tool overlap with different flags ===\n");
    
    // Test case 1: -v flag (verbose mode)
    printf("\n1. Testing -v flag (case 'v'):\n");
    execute_command("gcov-tool overlap -v test_a.gcda test_a.gcno 2>&1 | head -20");
    
    // Test case 2: -f flag (function level overlap)
    printf("\n2. Testing -f flag (case 'f'):\n");
    execute_command("gcov-tool overlap -f test_a.gcda test_b.gcda 2>&1 | head -20");
    
    // Test case 3: -F flag (use fullname)
    printf("\n3. Testing -F flag (case 'F'):\n");
    execute_command("gcov-tool overlap -F test_a.gcda test_b.gcda 2>&1 | head -20");
    
    // Test case 4: -o flag (object level)
    printf("\n4. Testing -o flag (case 'o'):\n");
    execute_command("gcov-tool overlap -o test_a.gcda test_b.gcda 2>&1 | head -20");
    
    // Test case 5: -h flag (hot only)
    printf("\n5. Testing -h flag (case 'h'):\n");
    execute_command("gcov-tool overlap -h test_a.gcda test_b.gcda 2>&1 | head -20");
    
    // Test case 6: -t flag with threshold (case 't')
    printf("\n6. Testing -t flag with threshold 0.5 (case 't'):\n");
    execute_command("gcov-tool overlap -t 0.5 test_a.gcda test_b.gcda 2>&1 | head -20");
    
    // Test case 7: -t flag with different threshold
    printf("\n7. Testing -t flag with threshold 0.75:\n");
    execute_command("gcov-tool overlap -t 0.75 test_a.gcda test_b.gcda 2>&1 | head -20");
    
    // Test case 8: -t flag with threshold 0.0
    printf("\n8. Testing -t flag with threshold 0.0:\n");
    execute_command("gcov-tool overlap -t 0.0 test_a.gcda test_b.gcda 2>&1 | head -20");
    
    // Test case 9: Combination of flags
    printf("\n9. Testing combination -v -f -o:\n");
    execute_command("gcov-tool overlap -v -f -o test_a.gcda test_b.gcda 2>&1 | head -20");
    
    // Test case 10: Combination with -t
    printf("\n10. Testing combination -v -h -t 0.3:\n");
    execute_command("gcov-tool overlap -v -h -t 0.3 test_a.gcda test_b.gcda 2>&1 | head -20");
    
    // Test case 11: Using multiple .gcda files
    printf("\n11. Testing with multiple input files:\n");
    execute_command("gcov-tool overlap -v test_a.gcda test_b.gcda test_d.gcda 2>&1 | head -20");
    
    // Test case 12: Using .gcno files (different input pattern)
    printf("\n12. Testing with .gcno files:\n");
    execute_command("gcov-tool overlap -v test_a.gcda test_a.gcno 2>&1 | head -20");
    
    // Test case 13: Using files from scenario C (multiple source files)
    printf("\n13. Testing with multi-file scenario:\n");
    execute_command("gcov-tool overlap -v scenario_c1.gcda scenario_c2.gcda 2>&1 | head -20");
    
    // Test case 14: Trigger default case with invalid option -z
    printf("\n14. Testing invalid option -z (should trigger default case):\n");
    execute_command("gcov-tool overlap -z 2>&1 | head -10");
    
    // Test case 15: Another invalid option
    printf("\n15. Testing invalid option --invalid (should trigger default case):\n");
    execute_command("gcov-tool overlap --invalid 2>&1 | head -10");
    
    // Test case 16: No arguments (should also trigger usage)
    printf("\n16. Testing no arguments (should trigger usage):\n");
    execute_command("gcov-tool overlap 2>&1 | head -10");
    
    // Test case 17: Valid option with missing required argument for -t
    printf("\n17. Testing -t without argument (should trigger error/usage):\n");
    execute_command("gcov-tool overlap -t 2>&1 | head -10");
    
    // Cleanup
    printf("\n=== Cleaning up generated files ===\n");
    
    // Remove source files
    remove("scenario_a.c");
    remove("scenario_b.c");
    remove("scenario_c.h");
    remove("scenario_c1.c");
    remove("scenario_c2.c");
    remove("scenario_d.c");
    
    // Remove executables
    remove("test_a");
    remove("test_b");
    remove("test_c");
    remove("test_d");
    
    // Remove coverage files
    system("rm -f *.gcda *.gcno *.gcov");
    
    printf("\n=== Test completed ===\n");
    printf("The gcov-tool overlap command has been invoked with various flag combinations.\n");
    printf("Check the coverage report for gcov-tool.cc to verify lines 534-554 are covered.\n");
    
    return 0;
}
