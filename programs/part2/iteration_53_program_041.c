/**
 * Test program to trigger uncovered lines in gcov-tool.cc (lines 534-554)
 * This program creates multiple test scenarios, generates coverage data,
 * and invokes gcov-tool overlap with various flag combinations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH 1024
#define NUM_TEST_SCENARIOS 4

/* Function prototypes */
int compile_with_coverage(const char *source, const char *output);
int run_program(const char *program, const char *args);
int invoke_gcov_tool(const char *gcda, const char *gcno, const char *flags);
void create_test_scenarios(void);
void cleanup_files(void);

/* Test scenario file names */
const char *test_files[] = {
    "test_simple.c",
    "test_loops.c", 
    "test_multi1.c",
    "test_multi2.c",
    "test_empty.c"
};

const char *executables[] = {
    "test_simple",
    "test_loops",
    "test_multi",
    "test_empty"
};

/* Track created files for cleanup */
char *created_files[50];
int num_created_files = 0;

void add_created_file(const char *filename) {
    if (num_created_files < 50) {
        created_files[num_created_files] = strdup(filename);
        num_created_files++;
    }
}

int compile_with_coverage(const char *source, const char *output) {
    char command[MAX_PATH * 3];
    int ret;
    
    snprintf(command, sizeof(command),
             "gcc -O0 -fprofile-arcs -ftest-coverage -g %s -o %s",
             source, output);
    
    printf("Compiling: %s\n", command);
    ret = system(command);
    
    if (ret == 0) {
        add_created_file(output);
        /* Add expected coverage files */
        char gcno_file[MAX_PATH];
        snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", source);
        add_created_file(gcno_file);
    }
    
    return ret;
}

int run_program(const char *program, const char *args) {
    char command[MAX_PATH * 2];
    
    if (args && args[0]) {
        snprintf(command, sizeof(command), "./%s %s", program, args);
    } else {
        snprintf(command, sizeof(command), "./%s", program);
    }
    
    printf("Running: %s\n", command);
    return system(command);
}

int invoke_gcov_tool(const char *gcda, const char *gcno, const char *flags) {
    char command[MAX_PATH * 4];
    int ret;
    
    if (gcda && gcno) {
        snprintf(command, sizeof(command), 
                 "gcov-tool overlap %s %s %s 2>&1", 
                 flags ? flags : "", gcda, gcno);
    } else {
        /* For testing invalid options */
        snprintf(command, sizeof(command),
                 "gcov-tool overlap %s 2>&1", flags ? flags : "");
    }
    
    printf("Executing: %s\n", command);
    ret = system(command);
    
    /* We don't care about the exit status for coverage purposes */
    return ret;
}

void create_test_scenario_simple(void) {
    FILE *fp = fopen("test_simple.c", "w");
    if (!fp) {
        perror("Failed to create test_simple.c");
        exit(1);
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int func1(int x) {\n");
    fprintf(fp, "    if (x > 0) {\n");
    fprintf(fp, "        return x * 2;\n");
    fprintf(fp, "    } else {\n");
    fprintf(fp, "        return x - 5;\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "}\n\n");
    fprintf(fp, "int func2(int a, int b) {\n");
    fprintf(fp, "    int sum = a + b;\n");
    fprintf(fp, "    for (int i = 0; i < 3; i++) {\n");
    fprintf(fp, "        sum += i;\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return sum;\n");
    fprintf(fp, "}\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int result1 = func1(10);\n");
    fprintf(fp, "    int result2 = func1(-3);\n");
    fprintf(fp, "    int result3 = func2(5, 7);\n");
    fprintf(fp, "    printf(\"Results: %%d, %%d, %%d\\n\", result1, result2, result3);\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    add_created_file("test_simple.c");
}

void create_test_scenario_loops(void) {
    FILE *fp = fopen("test_loops.c", "w");
    if (!fp) {
        perror("Failed to create test_loops.c");
        exit(1);
    }
    
    fprintf(fp, "#include <stdio.h>\n");
    fprintf(fp, "#include <stdlib.h>\n\n");
    fprintf(fp, "void process_array(int *arr, int size) {\n");
    fprintf(fp, "    for (int i = 0; i < size; i++) {\n");
    fprintf(fp, "        for (int j = 0; j < i; j++) {\n");
    fprintf(fp, "            arr[i] += arr[j];\n");
    fprintf(fp, "        }\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "}\n\n");
    fprintf(fp, "int main(int argc, char **argv) {\n");
    fprintf(fp, "    int iterations = 1;\n");
    fprintf(fp, "    if (argc > 1) {\n");
    fprintf(fp, "        iterations = atoi(argv[1]);\n");
    fprintf(fp, "        if (iterations < 1) iterations = 1;\n");
    fprintf(fp, "        if (iterations > 10) iterations = 10;\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    \n");
    fprintf(fp, "    int data[20];\n");
    fprintf(fp, "    for (int iter = 0; iter < iterations; iter++) {\n");
    fprintf(fp, "        for (int i = 0; i < 20; i++) {\n");
    fprintf(fp, "            data[i] = i + iter;\n");
    fprintf(fp, "        }\n");
    fprintf(fp, "        process_array(data, 20);\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    \n");
    fprintf(fp, "    printf(\"Processed %%d iterations\\n\", iterations);\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    add_created_file("test_loops.c");
}

void create_test_scenario_multi(void) {
    /* Create header file */
    FILE *fp = fopen("test_multi.h", "w");
    if (!fp) {
        perror("Failed to create test_multi.h");
        exit(1);
    }
    
    fprintf(fp, "#ifndef TEST_MULTI_H\n");
    fprintf(fp, "#define TEST_MULTI_H\n\n");
    fprintf(fp, "int helper_func(int x);\n");
    fprintf(fp, "void utility_func(void);\n\n");
    fprintf(fp, "#endif\n");
    
    fclose(fp);
    add_created_file("test_multi.h");
    
    /* Create first source file */
    fp = fopen("test_multi1.c", "w");
    if (!fp) {
        perror("Failed to create test_multi1.c");
        exit(1);
    }
    
    fprintf(fp, "#include <stdio.h>\n");
    fprintf(fp, "#include \"test_multi.h\"\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int val = 42;\n");
    fprintf(fp, "    printf(\"Starting program\\n\");\n");
    fprintf(fp, "    \n");
    fprintf(fp, "    int result = helper_func(val);\n");
    fprintf(fp, "    utility_func();\n");
    fprintf(fp, "    \n");
    fprintf(fp, "    printf(\"Result: %%d\\n\", result);\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    add_created_file("test_multi1.c");
    
    /* Create second source file */
    fp = fopen("test_multi2.c", "w");
    if (!fp) {
        perror("Failed to create test_multi2.c");
        exit(1);
    }
    
    fprintf(fp, "#include <stdio.h>\n");
    fprintf(fp, "#include \"test_multi.h\"\n\n");
    fprintf(fp, "int helper_func(int x) {\n");
    fprintf(fp, "    if (x %% 2 == 0) {\n");
    fprintf(fp, "        return x / 2;\n");
    fprintf(fp, "    } else {\n");
    fprintf(fp, "        return x * 3 + 1;\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "}\n\n");
    fprintf(fp, "void utility_func(void) {\n");
    fprintf(fp, "    static int count = 0;\n");
    fprintf(fp, "    count++;\n");
    fprintf(fp, "    printf(\"Utility called %%d times\\n\", count);\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    add_created_file("test_multi2.c");
}

void create_test_scenario_empty(void) {
    FILE *fp = fopen("test_empty.c", "w");
    if (!fp) {
        perror("Failed to create test_empty.c");
        exit(1);
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int unused_function(int x) {\n");
    fprintf(fp, "    return x * 2;\n");
    fprintf(fp, "}\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    /* This main does nothing that triggers coverage */\n");
    fprintf(fp, "    /* The program runs but doesn't execute instrumented code paths */\n");
    fprintf(fp, "    printf(\"Program ran (but no coverage)\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    add_created_file("test_empty.c");
}

void create_test_scenarios(void) {
    printf("Creating test scenarios...\n");
    
    create_test_scenario_simple();
    create_test_scenario_loops();
    create_test_scenario_multi();
    create_test_scenario_empty();
    
    printf("Test scenarios created.\n");
}

void compile_and_run_scenarios(void) {
    printf("\n=== Compiling and running test scenarios ===\n");
    
    /* Compile simple test */
    if (compile_with_coverage("test_simple.c", "test_simple") == 0) {
        run_program("test_simple", "");
    }
    
    /* Compile loops test */
    if (compile_with_coverage("test_loops.c", "test_loops") == 0) {
        /* Run multiple times with different arguments */
        run_program("test_loops", "1");
        run_program("test_loops", "3");
        run_program("test_loops", "5");
    }
    
    /* Compile multi-file test */
    char compile_cmd[MAX_PATH * 3];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -O0 -fprofile-arcs -ftest-coverage -g test_multi1.c test_multi2.c -o test_multi");
    printf("Compiling: %s\n", compile_cmd);
    if (system(compile_cmd) == 0) {
        add_created_file("test_multi");
        add_created_file("test_multi1.c.gcno");
        add_created_file("test_multi2.c.gcno");
        run_program("test_multi", "");
    }
    
    /* Compile empty test */
    if (compile_with_coverage("test_empty.c", "test_empty") == 0) {
        run_program("test_empty", "");
    }
}

void test_gcov_tool_flags(void) {
    printf("\n=== Testing gcov-tool overlap with various flags ===\n");
    
    /* Test 1: -v flag (verbose) - triggers case 'v' */
    printf("\n--- Test 1: -v flag ---\n");
    invoke_gcov_tool("test_simple.gcda", "test_simple.c.gcno", "-v");
    
    /* Test 2: -f flag (function level) - triggers case 'f' */
    printf("\n--- Test 2: -f flag ---\n");
    invoke_gcov_tool("test_simple.gcda", "test_simple.c.gcno", "-f");
    
    /* Test 3: -F flag (fullname) - triggers case 'F' */
    printf("\n--- Test 3: -F flag ---\n");
    invoke_gcov_tool("test_simple.gcda", "test_simple.c.gcno", "-F");
    
    /* Test 4: -o flag (object level) - triggers case 'o' */
    printf("\n--- Test 4: -o flag ---\n");
    invoke_gcov_tool("test_simple.gcda", "test_simple.c.gcno", "-o");
    
    /* Test 5: -h flag (hot only) - triggers case 'h' */
    printf("\n--- Test 5: -h flag ---\n");
    invoke_gcov_tool("test_simple.gcda", "test_simple.c.gcno", "-h");
    
    /* Test 6: -t flag with threshold - triggers case 't' */
    printf("\n--- Test 6: -t flag with threshold 0.5 ---\n");
    invoke_gcov_tool("test_simple.gcda", "test_simple.c.gcno", "-t 0.5");
    
    /* Test 7: -t flag with different threshold */
    printf("\n--- Test 7: -t flag with threshold 0.75 ---\n");
    invoke_gcov_tool("test_simple.gcda", "test_simple.c.gcno", "-t 0.75");
    
    /* Test 8: -t flag with high threshold */
    printf("\n--- Test 8: -t flag with threshold 0.9 ---\n");
    invoke_gcov_tool("test_simple.gcda", "test_simple.c.gcno", "-t 0.9");
    
    /* Test 9: Combination of flags */
    printf("\n--- Test 9: Combination -v -f -o ---\n");
    invoke_gcov_tool("test_simple.gcda", "test_simple.c.gcno", "-v -f -o");
    
    /* Test 10: Another combination */
    printf("\n--- Test 10: Combination -F -h ---\n");
    invoke_gcov_tool("test_simple.gcda", "test_simple.c.gcno", "-F -h");
    
    /* Test 11: All flags together */
    printf("\n--- Test 11: All flags (except invalid) ---\n");
    invoke_gcov_tool("test_simple.gcda", "test_simple.c.gcno", "-v -f -F -o -h -t 0.6");
    
    /* Test 12: With loops test data */
    printf("\n--- Test 12: With loops test data (-v -t 0.3) ---\n");
    invoke_gcov_tool("test_loops.gcda", "test_loops.c.gcno", "-v -t 0.3");
    
    /* Test 13: With multi-file data */
    printf("\n--- Test 13: With multi-file data (-f -o) ---\n");
    invoke_gcov_tool("test_multi1.gcda", "test_multi1.c.gcno", "-f -o");
    invoke_gcov_tool("test_multi2.gcda", "test_multi2.c.gcno", "-f -o");
    
    /* Test 14: With empty coverage data */
    printf("\n--- Test 14: With empty coverage (-h -t 0.1) ---\n");
    invoke_gcov_tool("test_empty.gcda", "test_empty.c.gcno", "-h -t 0.1");
    
    /* Test 15: Multiple input files */
    printf("\n--- Test 15: Multiple .gcda files (-v) ---\n");
    char multi_cmd[MAX_PATH * 4];
    snprintf(multi_cmd, sizeof(multi_cmd),
             "gcov-tool overlap -v test_simple.gcda test_loops.gcda test_simple.c.gcno test_loops.c.gcno 2>&1");
    printf("Executing: %s\n", multi_cmd);
    system(multi_cmd);
    
    /* Test 16: Invalid option - triggers default case and overlap_usage() */
    printf("\n--- Test 16: Invalid option -z (triggers default case) ---\n");
    invoke_gcov_tool(NULL, NULL, "-z");
    
    /* Test 17: Another invalid option */
    printf("\n--- Test 17: Invalid option --invalid (triggers default case) ---\n");
    invoke_gcov_tool(NULL, NULL, "--invalid");
    
    /* Test 18: Missing argument for -t (should trigger error handling) */
    printf("\n--- Test 18: -t without argument ---\n");
    invoke_gcov_tool(NULL, NULL, "-t");
    
    /* Test 19: Edge case - threshold 0.0 */
    printf("\n--- Test 19: -t with threshold 0.0 ---\n");
    invoke_gcov_tool("test_simple.gcda", "test_simple.c.gcno", "-t 0.0");
    
    /* Test 20: Edge case - threshold 1.0 */
    printf("\n--- Test 20: -t with threshold 1.0 ---\n");
    invoke_gcov_tool("test_simple.gcda", "test_simple.c.gcno", "-t 1.0");
}

void cleanup_files(void) {
    printf("\n=== Cleaning up generated files ===\n");
    
    for (int i = 0; i < num_created_files; i++) {
        if (created_files[i]) {
            printf("Removing: %s\n", created_files[i]);
            /* Also remove corresponding .gcda files */
            char *dot = strrchr(created_files[i], '.');
            if (dot && strcmp(dot, ".c") == 0) {
                char gcda_file[MAX_PATH];
                snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", created_files[i]);
                remove(gcda_file);
            }
            remove(created_files[i]);
            free(created_files[i]);
        }
    }
    
    /* Remove any other generated files */
    remove("test_multi.h");
    
    const char *extra_files[] = {
        "test_simple.gcda", "test_loops.gcda", "test_empty.gcda",
        "test_multi1.gcda", "test_multi2.gcda",
        "a.out"
    };
    
    for (size_t i = 0; i < sizeof(extra_files)/sizeof(extra_files[0]); i++) {
        remove(extra_files[i]);
    }
}

int main(int argc, char **argv) {
    printf("=== gcov-tool overlap coverage test ===\n");
    printf("This test targets lines 534-554 in gcov-tool.cc\n");
    printf("Testing case statements for: v, f, F, o, h, t, and default\n\n");
    
    /* Check if gcov-tool exists */
    if (system("which gcov-tool > /dev/null 2>&1") != 0) {
        printf("Warning: gcov-tool not found in PATH\n");
        printf("Trying current directory...\n");
        if (access("./gcov-tool", X_OK) != 0) {
            printf("ERROR: gcov-tool not found. Please build gcov-tool first.\n");
            printf("Build with: gcc -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
            return 1;
        }
    }
    
    /* Create test scenarios */
    create_test_scenarios();
    
    /* Compile and run them to generate coverage data */
    compile_and_run_scenarios();
    
    /* Test gcov-tool with various flag combinations */
    test_gcov_tool_flags();
    
    /* Cleanup */
    cleanup_files();
    
    printf("\n=== Test completed ===\n");
    printf("All flag combinations for gcov-tool overlap have been tested.\n");
    printf("The following cases in the switch statement should now be covered:\n");
    printf("  case 'v': (verbose)\n");
    printf("  case 'f': (function level)\n");
    printf("  case 'F': (fullname)\n");
    printf("  case 'o': (object level)\n");
    printf("  case 'h': (hot only)\n");
    printf("  case 't': (threshold)\n");
    printf("  default:  (invalid options)\n");
    
    return 0;
}
