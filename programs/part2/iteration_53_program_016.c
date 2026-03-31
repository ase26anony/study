/**
 * Test program to trigger uncovered lines in gcov-tool.cc (lines 534-554)
 * This program creates various coverage data files and invokes gcov-tool overlap
 * with different command-line arguments to exercise the switch statement.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH 1024
#define NUM_TEST_CASES 4

/* Function prototypes */
int compile_with_coverage(const char *source, const char *output);
int run_program(const char *program, const char *args);
int invoke_gcov_tool(const char *args, const char *gcda_file, const char *gcno_file);
void create_test_files(void);
void cleanup_files(void);

/* Test C source files content */
const char *test_sources[NUM_TEST_CASES] = {
    /* Scenario A: Simple Function */
    "test_simple.c",
    /* Scenario B: Loop Heavy */
    "test_loops.c", 
    /* Scenario C: Multiple Files (main) */
    "test_multi_main.c",
    /* Scenario D: Zero Counts */
    "test_zero.c"
};

const char *test_source_content[NUM_TEST_CASES] = {
    /* Scenario A: Simple Function */
    "#include <stdio.h>\n"
    "int add(int a, int b) { return a + b; }\n"
    "int subtract(int a, int b) { return a - b; }\n"
    "int main() {\n"
    "    int x = 10, y = 5;\n"
    "    if (x > y) {\n"
    "        printf(\"Sum: %d\\n\", add(x, y));\n"
    "    } else {\n"
    "        printf(\"Diff: %d\\n\", subtract(x, y));\n"
    "    }\n"
    "    return 0;\n"
    "}\n",
    
    /* Scenario B: Loop Heavy */
    "#include <stdio.h>\n"
    "#include <stdlib.h>\n"
    "int main(int argc, char *argv[]) {\n"
    "    int iterations = (argc > 1) ? atoi(argv[1]) : 10;\n"
    "    int sum = 0;\n"
    "    for (int i = 0; i < iterations; i++) {\n"
    "        for (int j = 0; j < i; j++) {\n"
    "            sum += i * j;\n"
    "        }\n"
    "    }\n"
    "    printf(\"Sum: %d\\n\", sum);\n"
    "    return 0;\n"
    "}\n",
    
    /* Scenario C: Multiple Files (main) */
    "#include <stdio.h>\n"
    "#include \"test_multi_lib.h\"\n"
    "int main() {\n"
    "    printf(\"Factorial of 5: %d\\n\", factorial(5));\n"
    "    printf(\"Fibonacci of 7: %d\\n\", fibonacci(7));\n"
    "    return 0;\n"
    "}\n",
    
    /* Scenario D: Zero Counts */
    "#include <stdio.h>\n"
    "int main() {\n"
    "    /* This program runs but doesn't execute instrumented paths */\n"
    "    /* The .gcda file will be created but with zero counts */\n"
    "    return 0;\n"
    "}\n"
};

/* Additional file for Scenario C */
const char *test_lib_content = 
    "#ifndef TEST_MULTI_LIB_H\n"
    "#define TEST_MULTI_LIB_H\n"
    "int factorial(int n) {\n"
    "    if (n <= 1) return 1;\n"
    "    return n * factorial(n - 1);\n"
    "}\n"
    "int fibonacci(int n) {\n"
    "    if (n <= 1) return n;\n"
    "    return fibonacci(n - 1) + fibonacci(n - 2);\n"
    "}\n"
    "#endif\n";

/* List of files to clean up */
char *files_to_cleanup[100];
int cleanup_count = 0;

void add_to_cleanup(const char *filename) {
    if (cleanup_count < 100) {
        files_to_cleanup[cleanup_count] = strdup(filename);
        cleanup_count++;
    }
}

int compile_with_coverage(const char *source, const char *output) {
    char command[MAX_PATH * 2];
    int result;
    
    /* Compile with coverage flags */
    snprintf(command, sizeof(command), 
             "gcc -O0 -fprofile-arcs -ftest-coverage -o %s %s 2>/dev/null", 
             output, source);
    
    printf("Compiling: %s\n", command);
    result = system(command);
    
    if (result == 0) {
        add_to_cleanup(output);
        /* Also add the .gcno file that will be generated */
        char gcno_file[MAX_PATH];
        snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", source);
        add_to_cleanup(gcno_file);
    }
    
    return result;
}

int run_program(const char *program, const char *args) {
    char command[MAX_PATH * 2];
    
    if (args && args[0]) {
        snprintf(command, sizeof(command), "./%s %s > /dev/null 2>&1", program, args);
    } else {
        snprintf(command, sizeof(command), "./%s > /dev/null 2>&1", program);
    }
    
    printf("Running: %s\n", command);
    return system(command);
}

int invoke_gcov_tool(const char *args, const char *gcda_file, const char *gcno_file) {
    char command[MAX_PATH * 4];
    int result;
    
    /* Try to find gcov-tool in PATH or use local path */
    const char *gcov_tool = "gcov-tool";
    
    if (gcda_file && gcno_file) {
        snprintf(command, sizeof(command), "%s overlap %s %s %s 2>&1", 
                 gcov_tool, args, gcda_file, gcno_file);
    } else {
        snprintf(command, sizeof(command), "%s overlap %s 2>&1", gcov_tool, args);
    }
    
    printf("Invoking gcov-tool: %s\n", command);
    result = system(command);
    
    /* We don't care about the exit status for coverage purposes */
    /* The important part is that gcov-tool executed and parsed the arguments */
    return result;
}

void create_test_files(void) {
    FILE *fp;
    
    /* Create test source files */
    for (int i = 0; i < NUM_TEST_CASES; i++) {
        fp = fopen(test_sources[i], "w");
        if (fp) {
            fputs(test_source_content[i], fp);
            fclose(fp);
            add_to_cleanup(test_sources[i]);
            printf("Created: %s\n", test_sources[i]);
        }
    }
    
    /* Create library file for Scenario C */
    fp = fopen("test_multi_lib.h", "w");
    if (fp) {
        fputs(test_lib_content, fp);
        fclose(fp);
        add_to_cleanup("test_multi_lib.h");
        printf("Created: test_multi_lib.h\n");
    }
    
    /* Create separate lib file for compilation */
    fp = fopen("test_multi_lib.c", "w");
    if (fp) {
        fputs("#include \"test_multi_lib.h\"\n", fp);
        fclose(fp);
        add_to_cleanup("test_multi_lib.c");
        printf("Created: test_multi_lib.c\n");
    }
}

void cleanup_files(void) {
    printf("\nCleaning up temporary files...\n");
    for (int i = 0; i < cleanup_count; i++) {
        if (files_to_cleanup[i]) {
            remove(files_to_cleanup[i]);
            free(files_to_cleanup[i]);
        }
    }
    
    /* Also clean up any .gcda files */
    system("rm -f *.gcda *.gcno 2>/dev/null");
    system("rm -f test_simple test_loops test_multi test_zero 2>/dev/null");
}

int main(int argc, char *argv[]) {
    int i;
    char gcda_file[MAX_PATH];
    char gcno_file[MAX_PATH];
    
    printf("=== Generating coverage data for gcov-tool testing ===\n");
    
    /* Create test source files */
    create_test_files();
    
    /* Compile and run test programs to generate coverage data */
    
    /* Scenario A: Simple Function */
    printf("\n--- Scenario A: Simple Function ---\n");
    if (compile_with_coverage("test_simple.c", "test_simple") == 0) {
        run_program("test_simple", NULL);
    }
    
    /* Scenario B: Loop Heavy (run multiple times with different inputs) */
    printf("\n--- Scenario B: Loop Heavy ---\n");
    if (compile_with_coverage("test_loops.c", "test_loops") == 0) {
        run_program("test_loops", "5");
        run_program("test_loops", "10");
        run_program("test_loops", "3");
    }
    
    /* Scenario C: Multiple Source Files */
    printf("\n--- Scenario C: Multiple Source Files ---\n");
    system("gcc -O0 -fprofile-arcs -ftest-coverage -o test_multi test_multi_main.c test_multi_lib.c 2>/dev/null");
    add_to_cleanup("test_multi");
    add_to_cleanup("test_multi_main.c.gcno");
    add_to_cleanup("test_multi_lib.c.gcno");
    run_program("test_multi", NULL);
    
    /* Scenario D: Zero Counts */
    printf("\n--- Scenario D: Zero Counts ---\n");
    if (compile_with_coverage("test_zero.c", "test_zero") == 0) {
        run_program("test_zero", NULL);
    }
    
    printf("\n=== Testing gcov-tool overlap with various flags ===\n");
    
    /* Test Case 1: -v flag (verbose) - triggers case 'v' */
    printf("\nTest 1: Testing -v flag\n");
    invoke_gcov_tool("-v", "test_simple.gcda", "test_simple.c.gcno");
    
    /* Test Case 2: -f flag (function level) - triggers case 'f' */
    printf("\nTest 2: Testing -f flag\n");
    invoke_gcov_tool("-f", "test_loops.gcda", "test_loops.c.gcno");
    
    /* Test Case 3: -F flag (fullname) - triggers case 'F' */
    printf("\nTest 3: Testing -F flag\n");
    invoke_gcov_tool("-F", "test_simple.gcda", "test_simple.c.gcno");
    
    /* Test Case 4: -o flag (object level) - triggers case 'o' */
    printf("\nTest 4: Testing -o flag\n");
    invoke_gcov_tool("-o", "test_loops.gcda", "test_loops.c.gcno");
    
    /* Test Case 5: -h flag (hot only) - triggers case 'h' */
    printf("\nTest 5: Testing -h flag\n");
    invoke_gcov_tool("-h", "test_simple.gcda", "test_simple.c.gcno");
    
    /* Test Case 6: -t flag with threshold - triggers case 't' */
    printf("\nTest 6: Testing -t flag with threshold 0.5\n");
    invoke_gcov_tool("-t 0.5", "test_loops.gcda", "test_loops.c.gcno");
    
    /* Test Case 7: -t flag with different threshold */
    printf("\nTest 7: Testing -t flag with threshold 0.75\n");
    invoke_gcov_tool("-t 0.75", "test_simple.gcda", "test_simple.c.gcno");
    
    /* Test Case 8: -t flag with threshold 0.0 */
    printf("\nTest 8: Testing -t flag with threshold 0.0\n");
    invoke_gcov_tool("-t 0.0", "test_zero.gcda", "test_zero.c.gcno");
    
    /* Test Case 9: -t flag with threshold 1.0 */
    printf("\nTest 9: Testing -t flag with threshold 1.0\n");
    invoke_gcov_tool("-t 1.0", "test_multi_main.gcda", "test_multi_main.c.gcno");
    
    /* Test Case 10: Combination of flags */
    printf("\nTest 10: Testing combination -v -f -o\n");
    invoke_gcov_tool("-v -f -o", "test_simple.gcda", "test_simple.c.gcno");
    
    /* Test Case 11: Another combination */
    printf("\nTest 11: Testing combination -F -h -t 0.3\n");
    invoke_gcov_tool("-F -h -t 0.3", "test_loops.gcda", "test_loops.c.gcno");
    
    /* Test Case 12: Invalid option to trigger default case and overlap_usage() */
    printf("\nTest 12: Testing invalid option -z (to trigger default case)\n");
    invoke_gcov_tool("-z", NULL, NULL);
    
    /* Test Case 13: Another invalid option */
    printf("\nTest 13: Testing invalid option --invalid (to trigger default case)\n");
    invoke_gcov_tool("--invalid", NULL, NULL);
    
    /* Test Case 14: Valid flag with invalid argument format */
    printf("\nTest 14: Testing -t without argument (may trigger error or default)\n");
    invoke_gcov_tool("-t", NULL, NULL);
    
    /* Test Case 15: Multiple files with flags */
    printf("\nTest 15: Testing with multiple .gcda files\n");
    invoke_gcov_tool("-v -f", "test_simple.gcda test_loops.gcda", "test_simple.c.gcno test_loops.c.gcno");
    
    printf("\n=== All gcov-tool tests completed ===\n");
    
    /* Clean up */
    cleanup_files();
    
    return 0;
}
