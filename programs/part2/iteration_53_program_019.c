/*
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
"int loop_heavy(int n) {\n"
"    int sum = 0;\n"
"    for (int i = 0; i < n; i++) {\n"
"        for (int j = 0; j < i; j++) {\n"
"            sum += j;\n"
"        }\n"
"    }\n"
"    return sum;\n"
"}\n"
"int main(int argc, char **argv) {\n"
"    int iterations = 10;\n"
"    if (argc > 1) iterations = atoi(argv[1]);\n"
"    printf(\"Result: %d\\n\", loop_heavy(iterations));\n"
"    return 0;\n"
"}\n";

const char *test_program_c1 = 
"#include \"test_c.h\"\n"
"int func_c1(int x) {\n"
"    return x * 2;\n"
"}\n"
"int main() {\n"
"    printf(\"C1: %d\\n\", func_c1(5));\n"
"    printf(\"C2: %d\\n\", func_c2(3));\n"
"    return 0;\n"
"}\n";

const char *test_program_c2 = 
"#include \"test_c.h\"\n"
"int func_c2(int y) {\n"
"    return y + 10;\n"
"}\n";

const char *test_header_c = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"#include <stdio.h>\n"
"int func_c1(int x);\n"
"int func_c2(int y);\n"
"#endif\n";

const char *test_program_d = 
"#include <stdio.h>\n"
"int never_called() {\n"
"    return 42;\n"
"}\n"
"int main() {\n"
"    // Don't call never_called to generate zero counts\n"
"    printf(\"Only main runs\\n\");\n"
"    return 0;\n"
"}\n";

/* Helper function to execute a command and check status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Command failed: %s (status: %d)\n", cmd, status);
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

/* Compile a test program with coverage flags */
int compile_with_coverage(const char *source_file, const char *output_name) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage -o %s %s",
             output_name, source_file);
    return execute_command(cmd);
}

/* Run a program to generate .gcda files */
int run_program(const char *program, const char *args) {
    char cmd[512];
    if (args && args[0]) {
        snprintf(cmd, sizeof(cmd), "./%s %s", program, args);
    } else {
        snprintf(cmd, sizeof(cmd), "./%s", program);
    }
    return execute_command(cmd);
}

/* Test gcov-tool overlap with various flag combinations */
void test_overlap_flags(const char *gcda_file, const char *gcno_file) {
    char cmd[1024];
    
    printf("\n=== Testing gcov-tool overlap flag combinations ===\n");
    
    /* Test case 'v' - verbose flag */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -v %s %s 2>&1 | head -20",
             gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 'f' - function level overlap */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -f %s %s 2>&1 | head -20",
             gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 'F' - use fullname */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -F %s %s 2>&1 | head -20",
             gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 'o' - object level */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -o %s %s 2>&1 | head -20",
             gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 'h' - hot only */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -h %s %s 2>&1 | head -20",
             gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 't' - hot threshold with argument */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -t 0.5 %s %s 2>&1 | head -20",
             gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test with multiple flags combined */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -v -f -o %s %s 2>&1 | head -20",
             gcda_file, gcno_file);
    execute_command(cmd);
    
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -F -h -t 0.75 %s %s 2>&1 | head -20",
             gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test default case (invalid option) - this should trigger overlap_usage() */
    printf("\n=== Testing invalid option to trigger default case ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -z %s %s 2>&1",
             gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test with no arguments to also trigger usage */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap 2>&1");
    execute_command(cmd);
}

/* Main test orchestration */
int main(int argc, char **argv) {
    printf("=== Starting gcov-tool overlap argument parsing coverage test ===\n");
    
    /* Check if gcov-tool exists */
    if (system("which gcov-tool > /dev/null 2>&1") != 0) {
        fprintf(stderr, "Error: gcov-tool not found in PATH\n");
        fprintf(stderr, "Trying ./gcov-tool...\n");
        if (access("./gcov-tool", X_OK) != 0) {
            fprintf(stderr, "Please build gcov-tool with coverage first:\n");
            fprintf(stderr, "  g++ -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
            return 1;
        }
        /* Add current directory to PATH for this test */
        setenv("PATH", "./:$PATH", 1);
    }
    
    /* Create and test Scenario A: Simple function */
    printf("\n=== Scenario A: Simple function ===\n");
    write_to_file("test_a.c", test_program_a);
    compile_with_coverage("test_a.c", "test_a");
    run_program("test_a", "");
    test_overlap_flags("test_a.gcda", "test_a.gcno");
    
    /* Create and test Scenario B: Loop heavy with different runs */
    printf("\n=== Scenario B: Loop heavy program ===\n");
    write_to_file("test_b.c", test_program_b);
    compile_with_coverage("test_b.c", "test_b");
    run_program("test_b", "5");
    run_program("test_b", "15");  /* Second run to accumulate counts */
    test_overlap_flags("test_b.gcda", "test_b.gcno");
    
    /* Create and test Scenario C: Multiple source files */
    printf("\n=== Scenario C: Multiple source files ===\n");
    write_to_file("test_c.h", test_header_c);
    write_to_file("test_c1.c", test_program_c1);
    write_to_file("test_c2.c", test_program_c2);
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage -o test_c test_c1.c test_c2.c");
    run_program("test_c", "");
    /* Test with multiple .gcda files */
    execute_command("gcov-tool overlap -v test_c1.gcda test_c1.gcno 2>&1 | head -20");
    execute_command("gcov-tool overlap -f -o test_c2.gcda test_c2.gcno 2>&1 | head -20");
    execute_command("gcov-tool overlap -F -h test_c1.gcda test_c2.gcda test_c1.gcno test_c2.gcno 2>&1 | head -20");
    
    /* Create and test Scenario D: Zero counts */
    printf("\n=== Scenario D: Zero counts ===\n");
    write_to_file("test_d.c", test_program_d);
    compile_with_coverage("test_d.c", "test_d");
    run_program("test_d", "");
    /* Test hot threshold with zero counts */
    execute_command("gcov-tool overlap -t 0.1 test_d.gcda test_d.gcno 2>&1 | head -20");
    
    /* Additional comprehensive tests with all scenarios */
    printf("\n=== Additional comprehensive tests ===\n");
    
    /* Test with two .gcda files from different scenarios */
    execute_command("gcov-tool overlap -v -f test_a.gcda test_b.gcda test_a.gcno test_b.gcno 2>&1 | head -20");
    
    /* Test with threshold variations */
    execute_command("gcov-tool overlap -t 0.0 test_a.gcda test_a.gcno 2>&1 | head -20");
    execute_command("gcov-tool overlap -t 1.0 test_b.gcda test_b.gcno 2>&1 | head -20");
    execute_command("gcov-tool overlap -t 0.25 -h test_b.gcda test_b.gcno 2>&1 | head -20");
    
    /* Test with just -h flag */
    execute_command("gcov-tool overlap -h test_c1.gcda test_c1.gcno 2>&1 | head -20");
    
    /* Test with -o and -f together */
    execute_command("gcov-tool overlap -o -f test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test with -F and -v */
    execute_command("gcov-tool overlap -F -v test_b.gcda test_b.gcno 2>&1 | head -20");
    
    /* Cleanup */
    printf("\n=== Cleaning up temporary files ===\n");
    execute_command("rm -f test_a test_a.c test_a.gcda test_a.gcno test_a.gcov");
    execute_command("rm -f test_b test_b.c test_b.gcda test_b.gcno test_b.gcov");
    execute_command("rm -f test_c test_c.h test_c1.c test_c2.c test_c1.gcda test_c2.gcda test_c1.gcno test_c2.gcno test_c1.gcov test_c2.gcov");
    execute_command("rm -f test_d test_d.c test_d.gcda test_d.gcno test_d.gcov");
    execute_command("rm -f *.gcov *.gcda *.gcno");
    
    printf("\n=== Test completed ===\n");
    printf("The gcov-tool overlap command has been invoked with various flag combinations.\n");
    printf("Check coverage of gcov-tool.cc lines 534-554 to verify execution.\n");
    
    return 0;
}
