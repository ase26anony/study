/**
 * Test harness for gcov-tool overlap command-line parsing coverage.
 * This program creates multiple test scenarios, generates coverage data,
 * and invokes gcov-tool with different flag combinations to cover
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
#define MAX_CMD 4096

/**
 * Simple utility function to execute a shell command and check status.
 * Returns 0 on success, non-zero on failure.
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
 * Create and compile test scenario A: Simple function with conditionals.
 */
int create_scenario_a() {
    const char *source = 
        "#include <stdio.h>\n"
        "int func1(int x) {\n"
        "    if (x > 0) {\n"
        "        return x * 2;\n"
        "    } else {\n"
        "        return x - 1;\n"
        "    }\n"
        "}\n"
        "int func2(int a, int b) {\n"
        "    return a + b;\n"
        "}\n"
        "int main() {\n"
        "    int result = func1(5) + func1(-3);\n"
        "    result += func2(10, 20);\n"
        "    printf(\"Result: %d\\n\", result);\n"
        "    return 0;\n"
        "}\n";
    
    FILE *fp = fopen("test_a.c", "w");
    if (!fp) {
        perror("Failed to create test_a.c");
        return -1;
    }
    fputs(source, fp);
    fclose(fp);
    
    // Compile with coverage
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage test_a.c -o test_a");
    if (execute_command(cmd) != 0) {
        return -1;
    }
    
    // Run to generate .gcda file
    if (execute_command("./test_a") != 0) {
        return -1;
    }
    
    return 0;
}

/**
 * Create and compile test scenario B: Loop-heavy program.
 */
int create_scenario_b() {
    const char *source = 
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "void process_array(int *arr, int size, int multiplier) {\n"
        "    for (int i = 0; i < size; i++) {\n"
        "        for (int j = 0; j < i; j++) {\n"
        "            arr[i] += arr[j] * multiplier;\n"
        "        }\n"
        "    }\n"
        "}\n"
        "int main(int argc, char **argv) {\n"
        "    int iterations = 1;\n"
        "    if (argc > 1) iterations = atoi(argv[1]);\n"
        "    \n"
        "    int data[10] = {1,2,3,4,5,6,7,8,9,10};\n"
        "    \n"
        "    for (int iter = 0; iter < iterations; iter++) {\n"
        "        process_array(data, 10, iter + 1);\n"
        "    }\n"
        "    \n"
        "    int sum = 0;\n"
        "    for (int i = 0; i < 10; i++) {\n"
        "        sum += data[i];\n"
        "    }\n"
        "    printf(\"Sum: %d\\n\", sum);\n"
        "    return 0;\n"
        "}\n";
    
    FILE *fp = fopen("test_b.c", "w");
    if (!fp) {
        perror("Failed to create test_b.c");
        return -1;
    }
    fputs(source, fp);
    fclose(fp);
    
    // Compile with coverage
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage test_b.c -o test_b");
    if (execute_command(cmd) != 0) {
        return -1;
    }
    
    // Run multiple times with different arguments to generate varied counts
    execute_command("./test_b 1");
    execute_command("./test_b 3");
    execute_command("./test_b 5");
    
    return 0;
}

/**
 * Create and compile test scenario C: Multiple source files.
 */
int create_scenario_c() {
    // Header file
    const char *header = 
        "#ifndef TEST_C_H\n"
        "#define TEST_C_H\n"
        "int helper1(int x);\n"
        "int helper2(int x, int y);\n"
        "#endif\n";
    
    FILE *fp = fopen("test_c.h", "w");
    if (!fp) {
        perror("Failed to create test_c.h");
        return -1;
    }
    fputs(header, fp);
    fclose(fp);
    
    // First source file
    const char *source1 = 
        "#include \"test_c.h\"\n"
        "#include <stdio.h>\n"
        "int helper1(int x) {\n"
        "    return x * x;\n"
        "}\n"
        "void file1_func() {\n"
        "    printf(\"File1 function called\\n\");\n"
        "}\n";
    
    fp = fopen("test_c1.c", "w");
    if (!fp) {
        perror("Failed to create test_c1.c");
        return -1;
    }
    fputs(source1, fp);
    fclose(fp);
    
    // Second source file
    const char *source2 = 
        "#include \"test_c.h\"\n"
        "#include <stdio.h>\n"
        "int helper2(int x, int y) {\n"
        "    if (x > y) return x - y;\n"
        "    else return y - x;\n"
        "}\n"
        "int main() {\n"
        "    int a = helper1(5);\n"
        "    int b = helper2(10, 3);\n"
        "    printf(\"Results: %d, %d\\n\", a, b);\n"
        "    return 0;\n"
        "}\n";
    
    fp = fopen("test_c2.c", "w");
    if (!fp) {
        perror("Failed to create test_c2.c");
        return -1;
    }
    fputs(source2, fp);
    fclose(fp);
    
    // Compile both files together with coverage
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage test_c1.c test_c2.c -o test_c");
    if (execute_command(cmd) != 0) {
        return -1;
    }
    
    // Run to generate .gcda files
    execute_command("./test_c");
    
    return 0;
}

/**
 * Create and compile test scenario D: Program with zero counts.
 */
int create_scenario_d() {
    const char *source = 
        "#include <stdio.h>\n"
        "int never_called() {\n"
        "    return 42;\n"
        "}\n"
        "int main() {\n"
        "    // This main function doesn't call any instrumented functions\n"
        "    // except itself, but it will have basic block counts\n"
        "    printf(\"Minimal execution\\n\");\n"
        "    return 0;\n"
        "}\n";
    
    FILE *fp = fopen("test_d.c", "w");
    if (!fp) {
        perror("Failed to create test_d.c");
        return -1;
    }
    fputs(source, fp);
    fclose(fp);
    
    // Compile with coverage
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage test_d.c -o test_d");
    if (execute_command(cmd) != 0) {
        return -1;
    }
    
    // Run once - will generate .gcda but with minimal counts
    execute_command("./test_d");
    
    return 0;
}

/**
 * Check if gcov-tool exists in PATH or current directory.
 */
int find_gcov_tool(char *tool_path, size_t path_size) {
    // First check current directory
    if (access("./gcov-tool", X_OK) == 0) {
        snprintf(tool_path, path_size, "./gcov-tool");
        return 0;
    }
    
    // Check in PATH
    const char *path = getenv("PATH");
    if (!path) {
        return -1;
    }
    
    char *path_copy = strdup(path);
    if (!path_copy) {
        return -1;
    }
    
    char *dir = strtok(path_copy, ":");
    while (dir) {
        snprintf(tool_path, path_size, "%s/gcov-tool", dir);
        if (access(tool_path, X_OK) == 0) {
            free(path_copy);
            return 0;
        }
        dir = strtok(NULL, ":");
    }
    
    free(path_copy);
    return -1;
}

/**
 * Run gcov-tool overlap with various flag combinations to cover the switch cases.
 */
void run_gcov_tool_tests(const char *gcov_tool_path) {
    char cmd[MAX_CMD];
    
    printf("\n=== Running gcov-tool overlap tests ===\n");
    
    // Test 1: -v flag (case 'v')
    printf("\nTest 1: Testing -v flag (verbose mode)\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v test_a.gcda test_a.gcno", gcov_tool_path);
    execute_command(cmd);
    
    // Test 2: -f flag (function level overlap)
    printf("\nTest 2: Testing -f flag (function level)\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -f test_a.gcda test_b.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 3: -F flag (use fullname)
    printf("\nTest 3: Testing -F flag (use fullname)\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -F test_a.gcda test_b.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 4: -o flag (object level)
    printf("\nTest 4: Testing -o flag (object level)\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -o test_a.gcda test_b.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 5: -h flag (hot only)
    printf("\nTest 5: Testing -h flag (hot only)\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -h test_a.gcda test_b.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 6: -t flag with threshold (case 't')
    printf("\nTest 6: Testing -t flag with threshold 0.5\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.5 test_a.gcda test_b.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 7: -t flag with different threshold
    printf("\nTest 7: Testing -t flag with threshold 0.75\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.75 test_a.gcda test_b.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 8: -t flag with threshold 0.0
    printf("\nTest 8: Testing -t flag with threshold 0.0\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.0 test_a.gcda test_b.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 9: Combination of flags
    printf("\nTest 9: Testing combination -v -f -o\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f -o test_a.gcda test_b.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 10: Another combination
    printf("\nTest 10: Testing combination -F -h -t 0.3\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -F -h -t 0.3 test_a.gcda test_b.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 11: With .gcno files
    printf("\nTest 11: Testing with .gcno files\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v test_a.gcda test_a.gcno", gcov_tool_path);
    execute_command(cmd);
    
    // Test 12: Multiple input files
    printf("\nTest 12: Testing with multiple input files\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v test_a.gcda test_b.gcda test_c1.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 13: Default case (invalid option -z)
    printf("\nTest 13: Testing invalid option -z (should trigger default case)\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -z 2>&1", gcov_tool_path);
    execute_command(cmd);
    
    // Test 14: Invalid option with valid files
    printf("\nTest 14: Testing invalid option -x with files\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -x test_a.gcda test_b.gcda 2>&1", gcov_tool_path);
    execute_command(cmd);
    
    // Test 15: -t without argument (should trigger error/usage)
    printf("\nTest 15: Testing -t without argument\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t 2>&1", gcov_tool_path);
    execute_command(cmd);
    
    // Test 16: Empty argument list (should show usage)
    printf("\nTest 16: Testing with no arguments\n");
    snprintf(cmd, sizeof(cmd), "%s overlap 2>&1", gcov_tool_path);
    execute_command(cmd);
    
    // Test 17: With zero-count files (scenario D)
    printf("\nTest 17: Testing with zero-count coverage data\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v test_d.gcda test_a.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 18: All flags together
    printf("\nTest 18: Testing all valid flags together\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f -F -o -h -t 0.1 test_a.gcda test_b.gcda", gcov_tool_path);
    execute_command(cmd);
}

/**
 * Clean up generated files.
 */
void cleanup_files() {
    printf("\n=== Cleaning up generated files ===\n");
    
    // Remove test source files
    remove("test_a.c");
    remove("test_b.c");
    remove("test_c.h");
    remove("test_c1.c");
    remove("test_c2.c");
    remove("test_d.c");
    
    // Remove executables
    remove("test_a");
    remove("test_b");
    remove("test_c");
    remove("test_d");
    
    // Remove coverage data files
    system("rm -f *.gcda *.gcno *.gcov");
    
    // Remove any other temporary files
    remove("a.out");
}

int main(int argc, char **argv) {
    printf("=== Starting gcov-tool overlap coverage test ===\n");
    
    // Find gcov-tool
    char gcov_tool_path[MAX_PATH];
    if (find_gcov_tool(gcov_tool_path, sizeof(gcov_tool_path)) != 0) {
        fprintf(stderr, "Error: gcov-tool not found in PATH or current directory.\n");
        fprintf(stderr, "Please build gcov-tool with coverage flags first:\n");
        fprintf(stderr, "  gcc -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
        return 1;
    }
    
    printf("Found gcov-tool at: %s\n", gcov_tool_path);
    
    // Create test scenarios
    printf("\n=== Creating test scenarios ===\n");
    
    printf("Creating scenario A (simple functions)...\n");
    if (create_scenario_a() != 0) {
        fprintf(stderr, "Failed to create scenario A\n");
        cleanup_files();
        return 1;
    }
    
    printf("Creating scenario B (loop heavy)...\n");
    if (create_scenario_b() != 0) {
        fprintf(stderr, "Failed to create scenario B\n");
        cleanup_files();
        return 1;
    }
    
    printf("Creating scenario C (multiple source files)...\n");
    if (create_scenario_c() != 0) {
        fprintf(stderr, "Failed to create scenario C\n");
        cleanup_files();
        return 1;
    }
    
    printf("Creating scenario D (zero counts)...\n");
    if (create_scenario_d() != 0) {
        fprintf(stderr, "Failed to create scenario D\n");
        cleanup_files();
        return 1;
    }
    
    // Run gcov-tool with various flag combinations
    run_gcov_tool_tests(gcov_tool_path);
    
    // Clean up
    if (argc > 1 && strcmp(argv[1], "--no-cleanup") == 0) {
        printf("\nSkipping cleanup (files preserved for inspection)\n");
    } else {
        cleanup_files();
    }
    
    printf("\n=== Test completed ===\n");
    printf("The gcov-tool overlap command-line parser should now have executed\n");
    printf("all switch cases including:\n");
    printf("  - case 'v' (verbose)\n");
    printf("  - case 'f' (function level)\n");
    printf("  - case 'F' (fullname)\n");
    printf("  - case 'o' (object level)\n");
    printf("  - case 'h' (hot only)\n");
    printf("  - case 't' (threshold)\n");
    printf("  - default case (invalid options)\n");
    
    return 0;
}
