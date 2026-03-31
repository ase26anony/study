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

#define MAX_PATH 1024

/* Simple test programs to generate coverage data */

/* Scenario A: Simple function with conditionals */
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

/* Scenario B: Loop heavy program */
const char *test_prog_b = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"int process_value(int val) {\n"
"    int sum = 0;\n"
"    for (int i = 0; i < val; i++) {\n"
"        sum += i;\n"
"        for (int j = 0; j < i; j++) {\n"
"            sum -= j;\n"
"        }\n"
"    }\n"
"    return sum;\n"
"}\n"
"int main(int argc, char *argv[]) {\n"
"    int iterations = 10;\n"
"    if (argc > 1) {\n"
"        iterations = atoi(argv[1]);\n"
"    }\n"
"    int total = 0;\n"
"    for (int k = 0; k < iterations; k++) {\n"
"        total += process_value(k);\n"
"    }\n"
"    printf(\"Total: %d\\n\", total);\n"
"    return 0;\n"
"}\n";

/* Scenario C: Multiple source files - part 1 */
const char *test_prog_c1 = 
"#include \"test_c.h\"\n"
"#include <stdio.h>\n"
"int helper1(int x) {\n"
"    return x * 2;\n"
"}\n"
"void process_data(int *arr, int size) {\n"
"    for (int i = 0; i < size; i++) {\n"
"        arr[i] = helper1(arr[i]);\n"
"    }\n"
"}\n";

/* Scenario C: Multiple source files - part 2 */
const char *test_prog_c2 = 
"#include \"test_c.h\"\n"
"#include <stdio.h>\n"
"int helper2(int y) {\n"
"    return y + 10;\n"
"}\n"
"int main() {\n"
"    int data[5] = {1, 2, 3, 4, 5};\n"
"    process_data(data, 5);\n"
"    for (int i = 0; i < 5; i++) {\n"
"        data[i] = helper2(data[i]);\n"
"    }\n"
"    printf(\"Result: %d\\n\", data[0]);\n"
"    return 0;\n"
"}\n";

/* Scenario C: Header file */
const char *test_header_c = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"void process_data(int *arr, int size);\n"
"int helper1(int x);\n"
"int helper2(int y);\n"
"#endif\n";

/* Scenario D: Program with zero coverage */
const char *test_prog_d = 
"#include <stdio.h>\n"
"int never_called() {\n"
"    return 42;\n"
"}\n"
"int main() {\n"
"    /* Don't call any instrumented functions */\n"
"    printf(\"No coverage generated\\n\");\n"
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

int main(int argc, char *argv[]) {
    char cmd[2048];
    char *gcov_tool_path;
    int ret = 0;
    
    printf("=== Test Harness for gcov-tool overlap command ===\n\n");
    
    /* Find gcov-tool */
    gcov_tool_path = find_gcov_tool();
    if (!gcov_tool_path) {
        fprintf(stderr, "Error: gcov-tool not found in PATH or current directory\n");
        fprintf(stderr, "Please build gcov-tool with: gcc -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
        return 1;
    }
    printf("Using gcov-tool at: %s\n\n", gcov_tool_path);
    
    /* Create test directory */
    execute_command("mkdir -p test_coverage_data");
    chdir("test_coverage_data");
    
    /* =========================================== */
    /* Scenario A: Simple function with conditionals */
    /* =========================================== */
    printf("\n--- Scenario A: Simple function ---\n");
    write_to_file("test_a.c", test_prog_a);
    
    /* Compile with coverage */
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_a.c -o test_a");
    
    /* Run to generate .gcda file */
    execute_command("./test_a");
    
    /* Test various flag combinations to cover the switch cases */
    
    /* Case 'v': verbose flag */
    printf("\nTesting case 'v' (verbose):\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v test_a.gcda test_a.gcno", gcov_tool_path);
    execute_command(cmd);
    
    /* Case 'f': function level overlap */
    printf("\nTesting case 'f' (function level):\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -f test_a.gcda test_a.gcno", gcov_tool_path);
    execute_command(cmd);
    
    /* Case 'F': use fullname */
    printf("\nTesting case 'F' (use fullname):\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -F test_a.gcda test_a.gcno", gcov_tool_path);
    execute_command(cmd);
    
    /* Case 'o': object level */
    printf("\nTesting case 'o' (object level):\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -o test_a.gcda test_a.gcno", gcov_tool_path);
    execute_command(cmd);
    
    /* Case 'h': hot only */
    printf("\nTesting case 'h' (hot only):\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -h test_a.gcda test_a.gcno", gcov_tool_path);
    execute_command(cmd);
    
    /* Case 't': hot threshold with argument */
    printf("\nTesting case 't' (hot threshold 0.5):\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.5 test_a.gcda test_a.gcno", gcov_tool_path);
    execute_command(cmd);
    
    /* Test with different threshold value */
    printf("\nTesting case 't' (hot threshold 0.75):\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.75 test_a.gcda test_a.gcno", gcov_tool_path);
    execute_command(cmd);
    
    /* Test combination of flags */
    printf("\nTesting combination -v -f -o:\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f -o test_a.gcda test_a.gcno", gcov_tool_path);
    execute_command(cmd);
    
    printf("\nTesting combination -F -h -t 0.8:\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -F -h -t 0.8 test_a.gcda test_a.gcno", gcov_tool_path);
    execute_command(cmd);
    
    /* =========================================== */
    /* Scenario B: Loop heavy program */
    /* =========================================== */
    printf("\n--- Scenario B: Loop heavy program ---\n");
    write_to_file("test_b.c", test_prog_b);
    
    /* Compile with coverage */
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_b.c -o test_b");
    
    /* Run multiple times with different inputs to generate varied coverage */
    execute_command("./test_b 5");
    execute_command("./test_b 10");
    execute_command("./test_b 3");
    
    /* Test with loop-heavy program data */
    printf("\nTesting with loop program (verbose + function level):\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f test_b.gcda test_b.gcno", gcov_tool_path);
    execute_command(cmd);
    
    printf("\nTesting with loop program (hot threshold 0.3):\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.3 test_b.gcda test_b.gcno", gcov_tool_path);
    execute_command(cmd);
    
    /* =========================================== */
    /* Scenario C: Multiple source files */
    /* =========================================== */
    printf("\n--- Scenario C: Multiple source files ---\n");
    write_to_file("test_c.h", test_header_c);
    write_to_file("test_c1.c", test_prog_c1);
    write_to_file("test_c2.c", test_prog_c2);
    
    /* Compile both files together with coverage */
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_c1.c test_c2.c -o test_c");
    
    /* Run to generate .gcda files */
    execute_command("./test_c");
    
    /* Test with multiple .gcda files */
    printf("\nTesting with multiple files (-v -o):\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v -o test_c1.gcda test_c2.gcda test_c1.gcno test_c2.gcno", gcov_tool_path);
    execute_command(cmd);
    
    printf("\nTesting with multiple files (-F -h -t 0.6):\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -F -h -t 0.6 test_c1.gcda test_c2.gcda test_c1.gcno test_c2.gcno", gcov_tool_path);
    execute_command(cmd);
    
    /* =========================================== */
    /* Scenario D: Zero coverage program */
    /* =========================================== */
    printf("\n--- Scenario D: Zero coverage program ---\n");
    write_to_file("test_d.c", test_prog_d);
    
    /* Compile with coverage */
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_d.c -o test_d");
    
    /* Run - will generate .gcda but with zero counts */
    execute_command("./test_d");
    
    /* Test with zero coverage data */
    printf("\nTesting with zero coverage (-v -t 0.1):\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v -t 0.1 test_d.gcda test_d.gcno", gcov_tool_path);
    execute_command(cmd);
    
    /* =========================================== */
    /* Test default case (invalid option) */
    /* =========================================== */
    printf("\n--- Testing default case (invalid option) ---\n");
    printf("This should trigger overlap_usage():\n");
    
    /* Test with invalid option -z */
    snprintf(cmd, sizeof(cmd), "%s overlap -z test_a.gcda test_a.gcno 2>&1", gcov_tool_path);
    execute_command(cmd);
    
    /* Test with missing argument for -t */
    printf("\nTesting missing argument for -t (should also trigger usage):\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t test_a.gcda test_a.gcno 2>&1", gcov_tool_path);
    execute_command(cmd);
    
    /* =========================================== */
    /* Additional combinations for thorough coverage */
    /* =========================================== */
    printf("\n--- Additional flag combinations ---\n");
    
    /* All flags together */
    printf("\nTesting all flags together:\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f -F -o -h -t 0.9 test_a.gcda test_b.gcda test_a.gcno test_b.gcno", gcov_tool_path);
    execute_command(cmd);
    
    /* Just -t with extreme values */
    printf("\nTesting -t with very low threshold (0.01):\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.01 test_a.gcda test_a.gcno", gcov_tool_path);
    execute_command(cmd);
    
    printf("\nTesting -t with high threshold (0.99):\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.99 test_b.gcda test_b.gcno", gcov_tool_path);
    execute_command(cmd);
    
    /* Test with multiple input files and various flags */
    printf("\nTesting with all generated files:\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f -o test_a.gcda test_b.gcda test_c1.gcda test_c2.gcda test_d.gcda *.gcno", gcov_tool_path);
    execute_command(cmd);
    
    /* =========================================== */
    /* Cleanup */
    /* =========================================== */
    printf("\n--- Cleaning up test files ---\n");
    chdir("..");
    execute_command("rm -rf test_coverage_data");
    
    printf("\n=== Test completed ===\n");
    printf("The following gcov-tool overlap flags were tested:\n");
    printf("  -v (verbose)           - Triggers case 'v' and gcov_set_verbose()\n");
    printf("  -f (function level)    - Triggers case 'f', sets overlap_func_level\n");
    printf("  -F (use fullname)      - Triggers case 'F', sets overlap_use_fullname\n");
    printf("  -o (object level)      - Triggers case 'o', sets overlap_obj_level\n");
    printf("  -h (hot only)          - Triggers case 'h', sets overlap_hot_only\n");
    printf("  -t (hot threshold)     - Triggers case 't', calls atof(optarg)\n");
    printf("  -z (invalid)           - Triggers default case and overlap_usage()\n");
    
    return ret;
}
