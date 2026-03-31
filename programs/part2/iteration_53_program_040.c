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
#define NUM_TEST_CASES 4

/* Simple test programs to generate coverage data */
const char *test_program_a = 
"#include <stdio.h>\n"
"void func1(int x) {\n"
"    if (x > 0) printf(\"Positive\\n\");\n"
"    else printf(\"Non-positive\\n\");\n"
"}\n"
"int main() {\n"
"    func1(5);\n"
"    func1(-3);\n"
"    return 0;\n"
"}\n";

const char *test_program_b = 
"#include <stdio.h>\n"
"void nested_loops(int n) {\n"
"    int i, j;\n"
"    for (i = 0; i < n; i++) {\n"
"        for (j = 0; j < i; j++) {\n"
"            printf(\".\");\n"
"        }\n"
"        printf(\"\\n\");\n"
"    }\n"
"}\n"
"int main(int argc, char **argv) {\n"
"    int iterations = 3;\n"
"    if (argc > 1) iterations = atoi(argv[1]);\n"
"    nested_loops(iterations);\n"
"    return 0;\n"
"}\n";

const char *test_program_c1 = 
"#include \"test_c.h\"\n"
"int helper1(int x) {\n"
"    return x * 2;\n"
"}\n"
"int main() {\n"
"    int result = helper1(5) + helper2(3);\n"
"    printf(\"Result: %d\\n\", result);\n"
"    return 0;\n"
"}\n";

const char *test_program_c2 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"int helper2(int y) {\n"
"    if (y > 0) return y + 10;\n"
"    return y - 5;\n"
"}\n";

const char *test_header_c = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"#include <stdio.h>\n"
"int helper1(int x);\n"
"int helper2(int y);\n"
"#endif\n";

const char *test_program_d = 
"#include <stdio.h>\n"
"int never_called() {\n"
"    return 42;\n"
"}\n"
"int main() {\n"
"    // This function is instrumented but never called\n"
"    // Generates zero-count coverage data\n"
"    printf(\"Main executed but instrumented function not called\\n\");\n"
"    return 0;\n"
"}\n";

/* Structure to hold test case information */
typedef struct {
    const char *name;
    const char *source;
    const char *executable;
    const char *gcda;
    const char *gcno;
    int needs_header;
} test_case_t;

/* Function prototypes */
int compile_with_coverage(const char *source, const char *output, 
                         const char *additional_flags);
int run_program(const char *program, const char *args);
int run_gcov_tool_overlap(const char *args, const char *gcda, const char *gcno);
void cleanup_files(void);

/* Global test files for cleanup */
char test_files[10][MAX_PATH];
int num_test_files = 0;

void add_test_file(const char *filename) {
    if (num_test_files < 10) {
        strncpy(test_files[num_test_files], filename, MAX_PATH-1);
        num_test_files++;
    }
}

int main(int argc, char **argv) {
    printf("=== Test Harness for gcov-tool overlap argument parsing ===\n");
    printf("Target: Lines 534-554 in gcov-tool.cc\n\n");
    
    /* Check if gcov-tool exists */
    if (system("which gcov-tool > /dev/null 2>&1") != 0) {
        /* Try in current directory */
        if (access("./gcov-tool", X_OK) != 0) {
            fprintf(stderr, "Error: gcov-tool not found in PATH or current directory\n");
            fprintf(stderr, "Please build gcov-tool with coverage flags first:\n");
            fprintf(stderr, "  g++ -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
            return 1;
        }
    }
    
    /* Create test programs */
    printf("1. Creating test programs with coverage instrumentation...\n");
    
    /* Test Case A: Simple function with branches */
    FILE *fp = fopen("test_a.c", "w");
    if (!fp) { perror("test_a.c"); return 1; }
    fputs(test_program_a, fp);
    fclose(fp);
    add_test_file("test_a.c");
    
    if (compile_with_coverage("test_a.c", "test_a", "") != 0) {
        fprintf(stderr, "Failed to compile test_a.c\n");
        return 1;
    }
    add_test_file("test_a");
    add_test_file("test_a.gcno");
    
    /* Run to generate gcda */
    if (run_program("./test_a", "") != 0) {
        fprintf(stderr, "Failed to run test_a\n");
    }
    add_test_file("test_a.gcda");
    
    /* Test Case B: Loop heavy program */
    fp = fopen("test_b.c", "w");
    if (!fp) { perror("test_b.c"); return 1; }
    fputs(test_program_b, fp);
    fclose(fp);
    add_test_file("test_b.c");
    
    if (compile_with_coverage("test_b.c", "test_b", "") != 0) {
        fprintf(stderr, "Failed to compile test_b.c\n");
        return 1;
    }
    add_test_file("test_b");
    add_test_file("test_b.gcno");
    
    /* Run multiple times with different arguments */
    run_program("./test_b", "");
    run_program("./test_b", "5");
    add_test_file("test_b.gcda");
    
    /* Test Case C: Multiple source files */
    fp = fopen("test_c.h", "w");
    if (!fp) { perror("test_c.h"); return 1; }
    fputs(test_header_c, fp);
    fclose(fp);
    add_test_file("test_c.h");
    
    fp = fopen("test_c1.c", "w");
    if (!fp) { perror("test_c1.c"); return 1; }
    fputs(test_program_c1, fp);
    fclose(fp);
    add_test_file("test_c1.c");
    
    fp = fopen("test_c2.c", "w");
    if (!fp) { perror("test_c2.c"); return 1; }
    fputs(test_program_c2, fp);
    fclose(fp);
    add_test_file("test_c2.c");
    
    /* Compile with multiple source files */
    char compile_cmd[MAX_PATH * 2];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -O0 -fprofile-arcs -ftest-coverage test_c1.c test_c2.c -o test_c");
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile test_c\n");
    } else {
        add_test_file("test_c");
        add_test_file("test_c1.gcno");
        add_test_file("test_c2.gcno");
        
        run_program("./test_c", "");
        add_test_file("test_c1.gcda");
        add_test_file("test_c2.gcda");
    }
    
    /* Test Case D: Zero counts */
    fp = fopen("test_d.c", "w");
    if (!fp) { perror("test_d.c"); return 1; }
    fputs(test_program_d, fp);
    fclose(fp);
    add_test_file("test_d.c");
    
    if (compile_with_coverage("test_d.c", "test_d", "") != 0) {
        fprintf(stderr, "Failed to compile test_d.c\n");
    } else {
        add_test_file("test_d");
        add_test_file("test_d.gcno");
        
        /* Run but don't call the instrumented function */
        run_program("./test_d", "");
        add_test_file("test_d.gcda");
    }
    
    printf("2. Generated coverage data files:\n");
    system("ls -la *.gcda *.gcno 2>/dev/null | head -20");
    printf("\n");
    
    printf("3. Testing gcov-tool overlap with various flag combinations...\n\n");
    
    /* Test Case 1: -v flag (verbose) - triggers case 'v' */
    printf("Test 1: -v flag (verbose mode)\n");
    printf("--------------------------------\n");
    run_gcov_tool_overlap("-v", "test_a.gcda", "test_a.gcno");
    printf("\n");
    
    /* Test Case 2: -f flag (function level) - triggers case 'f' */
    printf("Test 2: -f flag (function level overlap)\n");
    printf("-----------------------------------------\n");
    run_gcov_tool_overlap("-f", "test_a.gcda", "test_a.gcno");
    printf("\n");
    
    /* Test Case 3: -F flag (fullname) - triggers case 'F' */
    printf("Test 3: -F flag (use fullnames)\n");
    printf("--------------------------------\n");
    run_gcov_tool_overlap("-F", "test_a.gcda", "test_a.gcno");
    printf("\n");
    
    /* Test Case 4: -o flag (object level) - triggers case 'o' */
    printf("Test 4: -o flag (object level)\n");
    printf("-------------------------------\n");
    run_gcov_tool_overlap("-o", "test_a.gcda", "test_a.gcno");
    printf("\n");
    
    /* Test Case 5: -h flag (hot only) - triggers case 'h' */
    printf("Test 5: -h flag (hot only)\n");
    printf("---------------------------\n");
    run_gcov_tool_overlap("-h", "test_b.gcda", "test_b.gcno");
    printf("\n");
    
    /* Test Case 6: -t flag with threshold - triggers case 't' */
    printf("Test 6: -t flag with threshold 0.5\n");
    printf("-----------------------------------\n");
    run_gcov_tool_overlap("-t 0.5", "test_b.gcda", "test_b.gcno");
    printf("\n");
    
    /* Test Case 7: -t flag with different threshold */
    printf("Test 7: -t flag with threshold 0.75\n");
    printf("------------------------------------\n");
    run_gcov_tool_overlap("-t 0.75", "test_b.gcda", "test_b.gcno");
    printf("\n");
    
    /* Test Case 8: Multiple flags combined */
    printf("Test 8: Combined flags -v -f -o\n");
    printf("--------------------------------\n");
    run_gcov_tool_overlap("-v -f -o", "test_a.gcda", "test_a.gcno");
    printf("\n");
    
    /* Test Case 9: Multiple files input */
    printf("Test 9: Multiple .gcda files with -v flag\n");
    printf("-----------------------------------------\n");
    if (access("test_c1.gcda", F_OK) == 0 && access("test_c2.gcda", F_OK) == 0) {
        char cmd[MAX_PATH * 4];
        snprintf(cmd, sizeof(cmd), 
                 "gcov-tool overlap -v test_c1.gcda test_c2.gcda 2>&1 | head -20");
        system(cmd);
    } else {
        printf("Skipping - test_c gcda files not available\n");
    }
    printf("\n");
    
    /* Test Case 10: Invalid option to trigger default case */
    printf("Test 10: Invalid option -z (triggers default case -> overlap_usage())\n");
    printf("---------------------------------------------------------------------\n");
    /* Redirect stderr to capture usage message */
    system("gcov-tool overlap -z 2>&1 | head -10");
    printf("\n");
    
    /* Test Case 11: Zero count data with hot threshold */
    printf("Test 11: Zero count data with -t 0.1\n");
    printf("-------------------------------------\n");
    if (access("test_d.gcda", F_OK) == 0) {
        run_gcov_tool_overlap("-t 0.1", "test_d.gcda", "test_d.gcno");
    } else {
        printf("Skipping - test_d.gcda not available\n");
    }
    printf("\n");
    
    /* Test Case 12: All overlap flags together */
    printf("Test 12: All flags combined\n");
    printf("----------------------------\n");
    run_gcov_tool_overlap("-v -f -F -o -h -t 0.3", "test_a.gcda", "test_a.gcno");
    printf("\n");
    
    printf("4. Cleaning up generated files...\n");
    cleanup_files();
    
    printf("\n=== Test completed ===\n");
    printf("The gcov-tool overlap command has been invoked with various flag combinations.\n");
    printf("Each flag triggers a different case in the switch statement (lines 534-554):\n");
    printf("  -v : case 'v' (verbose)\n");
    printf("  -f : case 'f' (overlap_func_level)\n");
    printf("  -F : case 'F' (overlap_use_fullname)\n");
    printf("  -o : case 'o' (overlap_obj_level)\n");
    printf("  -h : case 'h' (overlap_hot_only)\n");
    printf("  -t : case 't' (overlap_hot_threshold)\n");
    printf("  -z : default case (overlap_usage)\n");
    
    return 0;
}

int compile_with_coverage(const char *source, const char *output, 
                         const char *additional_flags) {
    char cmd[MAX_PATH * 3];
    snprintf(cmd, sizeof(cmd),
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s %s",
             source, output, additional_flags);
    printf("  Compiling: %s\n", cmd);
    return system(cmd);
}

int run_program(const char *program, const char *args) {
    char cmd[MAX_PATH * 2];
    snprintf(cmd, sizeof(cmd), "%s %s > /dev/null 2>&1", program, args);
    return system(cmd);
}

int run_gcov_tool_overlap(const char *args, const char *gcda, const char *gcno) {
    char cmd[MAX_PATH * 4];
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap %s %s %s 2>&1 | head -5", args, gcda, gcno);
    return system(cmd);
}

void cleanup_files(void) {
    int i;
    for (i = 0; i < num_test_files; i++) {
        if (access(test_files[i], F_OK) == 0) {
            if (remove(test_files[i]) == 0) {
                printf("  Removed: %s\n", test_files[i]);
            }
        }
    }
    
    /* Clean up any other generated files */
    char *extra_files[] = {
        "a.out", "*.gcda", "*.gcno", "*.o",
        "test_*.c", "test_*.h"
    };
    
    for (i = 0; i < sizeof(extra_files)/sizeof(extra_files[0]); i++) {
        char cmd[MAX_PATH];
        snprintf(cmd, sizeof(cmd), "rm -f %s 2>/dev/null", extra_files[i]);
        system(cmd);
    }
}
