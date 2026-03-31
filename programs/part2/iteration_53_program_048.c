/* test_gcov_tool_overlap.c
 * 
 * This program tests the gcov-tool overlap subcommand by:
 * 1. Creating multiple C test programs with different coverage patterns
 * 2. Compiling them with coverage instrumentation
 * 3. Running them to generate .gcda files
 * 4. Invoking gcov-tool overlap with various flag combinations
 * 5. Cleaning up generated files
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

/* Create and compile test program A: Simple function with conditionals */
static int create_test_program_a(void) {
    const char *source = 
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "\n"
        "int func1(int x) {\n"
        "    if (x > 0) {\n"
        "        return x * 2;\n"
        "    } else {\n"
        "        return x - 5;\n"
        "    }\n"
        "}\n"
        "\n"
        "int func2(int a, int b) {\n"
        "    return a + b;\n"
        "}\n"
        "\n"
        "int main(int argc, char **argv) {\n"
        "    int val = (argc > 1) ? atoi(argv[1]) : 10;\n"
        "    int result1 = func1(val);\n"
        "    int result2 = func2(val, 5);\n"
        "    printf(\"Results: %d, %d\\n\", result1, result2);\n"
        "    return 0;\n"
        "}\n";
    
    FILE *fp = fopen("test_a.c", "w");
    if (!fp) {
        perror("Failed to create test_a.c");
        return -1;
    }
    fputs(source, fp);
    fclose(fp);
    
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage test_a.c -o test_a");
    return execute_command(cmd);
}

/* Create and compile test program B: Loop-heavy program */
static int create_test_program_b(void) {
    const char *source = 
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "\n"
        "void nested_loops(int n) {\n"
        "    int i, j, k;\n"
        "    int sum = 0;\n"
        "    for (i = 0; i < n; i++) {\n"
        "        for (j = 0; j < n; j++) {\n"
        "            for (k = 0; k < n; k++) {\n"
        "                sum += i * j * k;\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "    printf(\"Sum: %d\\n\", sum);\n"
        "}\n"
        "\n"
        "int factorial(int n) {\n"
        "    int result = 1;\n"
        "    while (n > 1) {\n"
        "        result *= n;\n"
        "        n--;\n"
        "    }\n"
        "    return result;\n"
        "}\n"
        "\n"
        "int main(int argc, char **argv) {\n"
        "    int n = (argc > 1) ? atoi(argv[1]) : 3;\n"
        "    nested_loops(n);\n"
        "    printf(\"Factorial(%d) = %d\\n\", n, factorial(n));\n"
        "    return 0;\n"
        "}\n";
    
    FILE *fp = fopen("test_b.c", "w");
    if (!fp) {
        perror("Failed to create test_b.c");
        return -1;
    }
    fputs(source, fp);
    fclose(fp);
    
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage test_b.c -o test_b");
    return execute_command(cmd);
}

/* Create and compile test program C: Multiple source files */
static int create_test_program_c(void) {
    /* Header file */
    const char *header = 
        "#ifndef TEST_C_H\n"
        "#define TEST_C_H\n"
        "\n"
        "int helper1(int x);\n"
        "int helper2(int x, int y);\n"
        "void process_data(int *arr, int size);\n"
        "\n"
        "#endif\n";
    
    /* Source file 1 */
    const char *source1 = 
        "#include \"test_c.h\"\n"
        "#include <stdio.h>\n"
        "\n"
        "int helper1(int x) {\n"
        "    return x * 2;\n"
        "}\n"
        "\n"
        "int helper2(int x, int y) {\n"
        "    if (x > y) {\n"
        "        return x - y;\n"
        "    } else {\n"
        "        return y - x;\n"
        "    }\n"
        "}\n";
    
    /* Source file 2 */
    const char *source2 = 
        "#include \"test_c.h\"\n"
        "#include <stdio.h>\n"
        "\n"
        "void process_data(int *arr, int size) {\n"
        "    int i;\n"
        "    for (i = 0; i < size; i++) {\n"
        "        arr[i] = helper1(arr[i]);\n"
        "    }\n"
        "}\n"
        "\n"
        "int main(int argc, char **argv) {\n"
        "    int data[5] = {1, 2, 3, 4, 5};\n"
        "    process_data(data, 5);\n"
        "    printf(\"Difference: %d\\n\", helper2(10, 3));\n"
        "    return 0;\n"
        "}\n";
    
    FILE *fp;
    
    fp = fopen("test_c.h", "w");
    if (!fp) {
        perror("Failed to create test_c.h");
        return -1;
    }
    fputs(header, fp);
    fclose(fp);
    
    fp = fopen("test_c1.c", "w");
    if (!fp) {
        perror("Failed to create test_c1.c");
        return -1;
    }
    fputs(source1, fp);
    fclose(fp);
    
    fp = fopen("test_c2.c", "w");
    if (!fp) {
        perror("Failed to create test_c2.c");
        return -1;
    }
    fputs(source2, fp);
    fclose(fp);
    
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage test_c1.c test_c2.c -o test_c");
    return execute_command(cmd);
}

/* Create and compile test program D: Program with zero coverage */
static int create_test_program_d(void) {
    const char *source = 
        "#include <stdio.h>\n"
        "\n"
        "int never_called(void) {\n"
        "    return 42;\n"
        "}\n"
        "\n"
        "int main(void) {\n"
        "    /* This main function doesn't call any instrumented functions */\n"
        "    printf(\"Hello\\n\");\n"
        "    return 0;\n"
        "}\n";
    
    FILE *fp = fopen("test_d.c", "w");
    if (!fp) {
        perror("Failed to create test_d.c");
        return -1;
    }
    fputs(source, fp);
    fclose(fp);
    
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage test_d.c -o test_d");
    return execute_command(cmd);
}

/* Run test programs to generate .gcda files */
static int run_test_programs(void) {
    int status = 0;
    
    /* Run test_a multiple times with different inputs */
    status |= execute_command("./test_a");
    status |= execute_command("./test_a 5");
    status |= execute_command("./test_a -3");
    
    /* Run test_b with different loop sizes */
    status |= execute_command("./test_b 2");
    status |= execute_command("./test_b 3");
    status |= execute_command("./test_b 4");
    
    /* Run test_c */
    status |= execute_command("./test_c");
    
    /* Run test_d (generates .gcda but with zero counts) */
    status |= execute_command("./test_d");
    
    return status;
}

/* Test gcov-tool overlap with various flag combinations */
static int test_gcov_tool_overlap(void) {
    int status = 0;
    char cmd[MAX_CMD];
    
    printf("\n=== Testing gcov-tool overlap with various flags ===\n");
    
    /* Test 1: -v flag (verbose) */
    printf("\n--- Test 1: -v flag ---\n");
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -v test_a.gcda test_a.gcno 2>&1 | head -20");
    status |= execute_command(cmd);
    
    /* Test 2: -f flag (function level) */
    printf("\n--- Test 2: -f flag ---\n");
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -f test_a.gcda test_b.gcda 2>&1 | head -20");
    status |= execute_command(cmd);
    
    /* Test 3: -F flag (fullname) */
    printf("\n--- Test 3: -F flag ---\n");
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -F test_a.gcda test_b.gcda 2>&1 | head -20");
    status |= execute_command(cmd);
    
    /* Test 4: -o flag (object level) */
    printf("\n--- Test 4: -o flag ---\n");
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -o test_a.gcda test_b.gcda 2>&1 | head -20");
    status |= execute_command(cmd);
    
    /* Test 5: -h flag (hot only) */
    printf("\n--- Test 5: -h flag ---\n");
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -h test_a.gcda test_b.gcda 2>&1 | head -20");
    status |= execute_command(cmd);
    
    /* Test 6: -t flag with threshold value */
    printf("\n--- Test 6: -t flag with threshold 0.5 ---\n");
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -t 0.5 test_a.gcda test_b.gcda 2>&1 | head -20");
    status |= execute_command(cmd);
    
    /* Test 7: -t flag with different threshold */
    printf("\n--- Test 7: -t flag with threshold 0.75 ---\n");
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -t 0.75 test_a.gcda test_b.gcda 2>&1 | head -20");
    status |= execute_command(cmd);
    
    /* Test 8: -t flag with threshold 0.0 */
    printf("\n--- Test 8: -t flag with threshold 0.0 ---\n");
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -t 0.0 test_a.gcda test_b.gcda 2>&1 | head -20");
    status |= execute_command(cmd);
    
    /* Test 9: Combination of flags */
    printf("\n--- Test 9: Combination -v -f -o ---\n");
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -v -f -o test_a.gcda test_b.gcda 2>&1 | head -20");
    status |= execute_command(cmd);
    
    /* Test 10: Another combination */
    printf("\n--- Test 10: Combination -F -h -t 0.3 ---\n");
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -F -h -t 0.3 test_a.gcda test_b.gcda 2>&1 | head -20");
    status |= execute_command(cmd);
    
    /* Test 11: Test with multiple .gcda files */
    printf("\n--- Test 11: Multiple .gcda files with -v ---\n");
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -v test_a.gcda test_b.gcda test_c1.gcda 2>&1 | head -20");
    status |= execute_command(cmd);
    
    /* Test 12: Test with .gcno files */
    printf("\n--- Test 12: With .gcno files ---\n");
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -v test_a.gcda test_a.gcno test_b.gcda test_b.gcno 2>&1 | head -20");
    status |= execute_command(cmd);
    
    /* Test 13: Test with zero-coverage files */
    printf("\n--- Test 13: Zero-coverage file with -t 0.5 ---\n");
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -t 0.5 test_d.gcda test_a.gcda 2>&1 | head -20");
    status |= execute_command(cmd);
    
    /* Test 14: Trigger default case with invalid option */
    printf("\n--- Test 14: Invalid option to trigger default case ---\n");
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -z test_a.gcda 2>&1 | head -5");
    status |= execute_command(cmd);
    
    /* Test 15: Another invalid option combination */
    printf("\n--- Test 15: Another invalid option ---\n");
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -x -y test_a.gcda 2>&1 | head -5");
    status |= execute_command(cmd);
    
    return status;
}

/* Clean up generated files */
static void cleanup_files(void) {
    printf("\n=== Cleaning up generated files ===\n");
    
    /* Remove source files */
    remove("test_a.c");
    remove("test_b.c");
    remove("test_c.h");
    remove("test_c1.c");
    remove("test_c2.c");
    remove("test_d.c");
    
    /* Remove executables */
    remove("test_a");
    remove("test_b");
    remove("test_c");
    remove("test_d");
    
    /* Remove coverage files */
    execute_command("rm -f *.gcda *.gcno *.gcov");
    
    /* Remove any other temporary files */
    execute_command("rm -f *.o");
}

int main(int argc, char **argv) {
    int status = 0;
    
    printf("=== Starting gcov-tool overlap coverage test ===\n");
    
    /* Check if gcov-tool exists */
    if (system("which gcov-tool > /dev/null 2>&1") != 0) {
        fprintf(stderr, "Error: gcov-tool not found in PATH\n");
        fprintf(stderr, "Please ensure gcov-tool is built and in your PATH\n");
        fprintf(stderr, "You may need to run: make gcov-tool\n");
        return 1;
    }
    
    /* Create and compile test programs */
    printf("\n=== Creating test programs ===\n");
    status |= create_test_program_a();
    status |= create_test_program_b();
    status |= create_test_program_c();
    status |= create_test_program_d();
    
    if (status != 0) {
        fprintf(stderr, "Failed to create test programs\n");
        cleanup_files();
        return 1;
    }
    
    /* Run test programs to generate coverage data */
    printf("\n=== Running test programs to generate .gcda files ===\n");
    status |= run_test_programs();
    
    if (status != 0) {
        fprintf(stderr, "Some test programs failed to run\n");
        /* Continue anyway - we might still have some .gcda files */
    }
    
    /* Test gcov-tool overlap with various flags */
    status |= test_gcov_tool_overlap();
    
    /* Clean up */
    cleanup_files();
    
    printf("\n=== Test completed ===\n");
    if (status == 0) {
        printf("All tests executed successfully\n");
    } else {
        printf("Some tests had non-zero exit status\n");
    }
    
    return status;
}
