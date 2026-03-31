/**
 * test_gcov_tool_overlap.c
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
 * Simple utility function to execute a shell command and check return status
 */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        printf("Warning: Command returned non-zero: %d\n", status);
    }
    return status;
}

/**
 * Create a simple C program for coverage testing
 */
void create_test_program_a(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create test program A");
        exit(1);
    }
    
    fprintf(f, "/* Test program A - Simple functions with conditionals */\n");
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int func1(int x) {\n");
    fprintf(f, "    if (x > 0) {\n");
    fprintf(f, "        return x * 2;\n");
    fprintf(f, "    } else {\n");
    fprintf(f, "        return x - 5;\n");
    fprintf(f, "    }\n");
    fprintf(f, "}\n\n");
    fprintf(f, "int func2(int a, int b) {\n");
    fprintf(f, "    int sum = a + b;\n");
    fprintf(f, "    if (sum > 100) {\n");
    fprintf(f, "        return sum / 2;\n");
    fprintf(f, "    }\n");
    fprintf(f, "    return sum;\n");
    fprintf(f, "}\n\n");
    fprintf(f, "int main() {\n");
    fprintf(f, "    printf(\"Test A running\\n\");\n");
    fprintf(f, "    int r1 = func1(10);\n");
    fprintf(f, "    int r2 = func1(-3);\n");
    fprintf(f, "    int r3 = func2(50, 60);\n");
    fprintf(f, "    int r4 = func2(10, 20);\n");
    fprintf(f, "    printf(\"Results: %%d %%d %%d %%d\\n\", r1, r2, r3, r4);\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
}

/**
 * Create a loop-heavy C program
 */
void create_test_program_b(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create test program B");
        exit(1);
    }
    
    fprintf(f, "/* Test program B - Loop heavy with nested loops */\n");
    fprintf(f, "#include <stdio.h>\n");
    fprintf(f, "#include <stdlib.h>\n\n");
    fprintf(f, "int process_array(int *arr, int size) {\n");
    fprintf(f, "    int total = 0;\n");
    fprintf(f, "    for (int i = 0; i < size; i++) {\n");
    fprintf(f, "        for (int j = 0; j < i; j++) {\n");
    fprintf(f, "            total += arr[j];\n");
    fprintf(f, "        }\n");
    fprintf(f, "    }\n");
    fprintf(f, "    return total;\n");
    fprintf(f, "}\n\n");
    fprintf(f, "int main(int argc, char **argv) {\n");
    fprintf(f, "    printf(\"Test B running\\n\");\n");
    fprintf(f, "    int iterations = 1;\n");
    fprintf(f, "    if (argc > 1) {\n");
    fprintf(f, "        iterations = atoi(argv[1]);\n");
    fprintf(f, "        if (iterations < 1) iterations = 1;\n");
    fprintf(f, "        if (iterations > 10) iterations = 10;\n");
    fprintf(f, "    }\n");
    fprintf(f, "    \n");
    fprintf(f, "    int data[20];\n");
    fprintf(f, "    for (int i = 0; i < 20; i++) {\n");
    fprintf(f, "        data[i] = i * 2;\n");
    fprintf(f, "    }\n");
    fprintf(f, "    \n");
    fprintf(f, "    int result = 0;\n");
    fprintf(f, "    for (int iter = 0; iter < iterations; iter++) {\n");
    fprintf(f, "        result += process_array(data, 15 + iter %% 5);\n");
    fprintf(f, "    }\n");
    fprintf(f, "    printf(\"Result: %%d\\n\", result);\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
}

/**
 * Create a multi-file test program (main file)
 */
void create_test_program_c1(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create test program C1");
        exit(1);
    }
    
    fprintf(f, "/* Test program C - Main file for multi-file test */\n");
    fprintf(f, "#include <stdio.h>\n");
    fprintf(f, "#include \"test_c_utils.h\"\n\n");
    fprintf(f, "int main() {\n");
    fprintf(f, "    printf(\"Test C running\\n\");\n");
    fprintf(f, "    \n");
    fprintf(f, "    // Call functions from both files\n");
    fprintf(f, "    int a = helper_multiply(7, 8);\n");
    fprintf(f, "    int b = helper_add(10, 20);\n");
    fprintf(f, "    \n");
    fprintf(f, "    if (a > b) {\n");
    fprintf(f, "        printf(\"Multiply wins: %%d\\n\", a);\n");
    fprintf(f, "    } else {\n");
    fprintf(f, "        printf(\"Add wins: %%d\\n\", b);\n");
    fprintf(f, "    }\n");
    fprintf(f, "    \n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
}

/**
 * Create a multi-file test program (utility file)
 */
void create_test_program_c2(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create test program C2");
        exit(1);
    }
    
    fprintf(f, "/* Test program C - Utility file */\n");
    fprintf(f, "#include \"test_c_utils.h\"\n\n");
    fprintf(f, "int helper_multiply(int x, int y) {\n");
    fprintf(f, "    int result = 0;\n");
    fprintf(f, "    for (int i = 0; i < y; i++) {\n");
    fprintf(f, "        result += x;\n");
    fprintf(f, "    }\n");
    fprintf(f, "    return result;\n");
    fprintf(f, "}\n\n");
    fprintf(f, "int helper_add(int x, int y) {\n");
    fprintf(f, "    return x + y;\n");
    fprintf(f, "}\n");
    
    fclose(f);
}

/**
 * Create header for multi-file test
 */
void create_test_program_c_header(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create test program C header");
        exit(1);
    }
    
    fprintf(f, "/* Test program C - Header file */\n");
    fprintf(f, "#ifndef TEST_C_UTILS_H\n");
    fprintf(f, "#define TEST_C_UTILS_H\n\n");
    fprintf(f, "int helper_multiply(int x, int y);\n");
    fprintf(f, "int helper_add(int x, int y);\n\n");
    fprintf(f, "#endif /* TEST_C_UTILS_H */\n");
    
    fclose(f);
}

/**
 * Create a program that generates zero coverage
 */
void create_test_program_d(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create test program D");
        exit(1);
    }
    
    fprintf(f, "/* Test program D - Zero coverage (unexecuted paths) */\n");
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int never_called() {\n");
    fprintf(f, "    printf(\"This function is never called\\n\");\n");
    fprintf(f, "    return 42;\n");
    fprintf(f, "}\n\n");
    fprintf(f, "int main() {\n");
    fprintf(f, "    printf(\"Test D running - but only taking one path\\n\");\n");
    fprintf(f, "    // The conditional is always false, so branch not taken\n");
    fprintf(f, "    if (0) {\n");
    fprintf(f, "        never_called();\n");
    fprintf(f, "    }\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
}

/**
 * Find gcov-tool in PATH or current directory
 */
char* find_gcov_tool() {
    static char path[MAX_PATH];
    
    // Check current directory first
    if (access("./gcov-tool", X_OK) == 0) {
        strcpy(path, "./gcov-tool");
        return path;
    }
    
    // Check in PATH
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
    
    // Not found
    return NULL;
}

int main(int argc, char **argv) {
    printf("=== Test Harness for gcov-tool overlap argument parsing ===\n");
    printf("Target: Lines 534-554 in gcov-tool.cc\n\n");
    
    // Find gcov-tool
    char *gcov_tool_path = find_gcov_tool();
    if (!gcov_tool_path) {
        fprintf(stderr, "ERROR: gcov-tool not found in PATH or current directory\n");
        fprintf(stderr, "Please build gcov-tool with coverage flags first:\n");
        fprintf(stderr, "  g++ -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
        return 1;
    }
    printf("Found gcov-tool at: %s\n\n", gcov_tool_path);
    
    // Create test directories
    execute_command("mkdir -p test_coverage_data");
    execute_command("cd test_coverage_data");
    
    // Change to test directory
    if (chdir("test_coverage_data") != 0) {
        perror("Failed to change to test directory");
        return 1;
    }
    
    // ============================================
    // Create and compile test programs
    // ============================================
    
    printf("\n--- Creating and compiling test programs ---\n");
    
    // Test A: Simple function with conditionals
    create_test_program_a("test_a.c");
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_a.c -o test_a");
    execute_command("./test_a");
    
    // Test B: Loop heavy program
    create_test_program_b("test_b.c");
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_b.c -o test_b");
    // Run multiple times with different arguments
    execute_command("./test_b");
    execute_command("./test_b 3");
    execute_command("./test_b 5");
    
    // Test C: Multi-file program
    create_test_program_c_header("test_c_utils.h");
    create_test_program_c1("test_c_main.c");
    create_test_program_c2("test_c_utils.c");
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_c_main.c test_c_utils.c -o test_c");
    execute_command("./test_c");
    
    // Test D: Zero coverage program
    create_test_program_d("test_d.c");
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_d.c -o test_d");
    execute_command("./test_d");
    
    printf("\n--- Generated coverage data files ---\n");
    execute_command("ls -la *.gcda *.gcno");
    
    // ============================================
    // Test gcov-tool overlap with various flags
    // Targeting lines 534-554 in gcov-tool.cc
    // ============================================
    
    printf("\n--- Testing gcov-tool overlap argument parsing ---\n");
    
    char cmd[MAX_CMD];
    
    // Test 1: -v flag (verbose) - case 'v'
    printf("\n1. Testing -v flag (verbose mode)...\n");
    snprintf(cmd, MAX_CMD, "%s overlap -v test_a.gcda test_a.gcno", gcov_tool_path);
    execute_command(cmd);
    
    // Test 2: -f flag (function level) - case 'f'
    printf("\n2. Testing -f flag (function level)...\n");
    snprintf(cmd, MAX_CMD, "%s overlap -f test_a.gcda test_b.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 3: -F flag (fullname) - case 'F'
    printf("\n3. Testing -F flag (fullname)...\n");
    snprintf(cmd, MAX_CMD, "%s overlap -F test_a.gcda test_b.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 4: -o flag (object level) - case 'o'
    printf("\n4. Testing -o flag (object level)...\n");
    snprintf(cmd, MAX_CMD, "%s overlap -o test_a.gcda test_b.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 5: -h flag (hot only) - case 'h'
    printf("\n5. Testing -h flag (hot only)...\n");
    snprintf(cmd, MAX_CMD, "%s overlap -h test_a.gcda test_b.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 6: -t flag with threshold - case 't'
    printf("\n6. Testing -t flag with threshold 0.5...\n");
    snprintf(cmd, MAX_CMD, "%s overlap -t 0.5 test_a.gcda test_b.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 7: -t flag with different threshold
    printf("\n7. Testing -t flag with threshold 0.75...\n");
    snprintf(cmd, MAX_CMD, "%s overlap -t 0.75 test_a.gcda test_b.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 8: -t flag with threshold 0.0
    printf("\n8. Testing -t flag with threshold 0.0...\n");
    snprintf(cmd, MAX_CMD, "%s overlap -t 0.0 test_a.gcda test_b.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 9: -t flag with threshold 1.0
    printf("\n9. Testing -t flag with threshold 1.0...\n");
    snprintf(cmd, MAX_CMD, "%s overlap -t 1.0 test_a.gcda test_b.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 10: Combination of flags
    printf("\n10. Testing combination of flags (-v -f -o)...\n");
    snprintf(cmd, MAX_CMD, "%s overlap -v -f -o test_a.gcda test_b.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 11: Another combination (-F -h)
    printf("\n11. Testing combination of flags (-F -h)...\n");
    snprintf(cmd, MAX_CMD, "%s overlap -F -h test_a.gcda test_b.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 12: All flags together
    printf("\n12. Testing all flags together...\n");
    snprintf(cmd, MAX_CMD, "%s overlap -v -f -F -o -h -t 0.3 test_a.gcda test_b.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 13: Test with multi-file coverage data
    printf("\n13. Testing with multi-file coverage data...\n");
    snprintf(cmd, MAX_CMD, "%s overlap -v test_c_main.gcda test_c_utils.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 14: Test with zero-coverage data
    printf("\n14. Testing with zero-coverage data...\n");
    snprintf(cmd, MAX_CMD, "%s overlap -t 0.5 test_d.gcda test_a.gcda", gcov_tool_path);
    execute_command(cmd);
    
    // Test 15: Invalid option to trigger default case and overlap_usage()
    printf("\n15. Testing invalid option to trigger default case...\n");
    snprintf(cmd, MAX_CMD, "%s overlap -z 2>&1 | head -20", gcov_tool_path);
    execute_command(cmd);
    
    // Test 16: Another invalid option
    printf("\n16. Testing another invalid option...\n");
    snprintf(cmd, MAX_CMD, "%s overlap --invalid-option 2>&1 | head -20", gcov_tool_path);
    execute_command(cmd);
    
    // Test 17: No arguments to trigger usage
    printf("\n17. Testing no arguments (should show usage)...\n");
    snprintf(cmd, MAX_CMD, "%s overlap 2>&1 | head -20", gcov_tool_path);
    execute_command(cmd);
    
    // Test 18: Help flag (should also show usage)
    printf("\n18. Testing help flag...\n");
    snprintf(cmd, MAX_CMD, "%s overlap --help 2>&1 | head -30", gcov_tool_path);
    execute_command(cmd);
    
    // ============================================
    // Cleanup
    // ============================================
    
    printf("\n--- Cleaning up test files ---\n");
    
    // Change back to parent directory
    chdir("..");
    
    // Remove test directory
    execute_command("rm -rf test_coverage_data");
    
    printf("\n=== Test completed ===\n");
    printf("The following cases in gcov-tool.cc lines 534-554 should have been triggered:\n");
    printf("  - case 'v': verbose mode\n");
    printf("  - case 'f': function level overlap\n");
    printf("  - case 'F': fullname mode\n");
    printf("  - case 'o': object level\n");
    printf("  - case 'h': hot only\n");
    printf("  - case 't': hot threshold (with various values)\n");
    printf("  - default: invalid option (triggers overlap_usage())\n");
    
    return 0;
}
