/* test_gcov_tool_overlap.c - Test harness for gcov-tool overlap command parsing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Simple C program to generate coverage data - Scenario A */
const char *test_prog_a = 
"#include <stdio.h>\n"
"int func1(int x) {\n"
"    if (x > 0) return x * 2;\n"
"    else return x * 3;\n"
"}\n"
"void func2() {\n"
"    for (int i = 0; i < 5; i++) {\n"
"        printf(\"Loop iteration %d\\n\", i);\n"
"    }\n"
"}\n"
"int main() {\n"
"    func1(10);\n"
"    func1(-5);\n"
"    func2();\n"
"    return 0;\n"
"}\n";

/* Loop-heavy program - Scenario B */
const char *test_prog_b = 
"#include <stdio.h>\n"
"int process_data(int n) {\n"
"    int sum = 0;\n"
"    for (int i = 0; i < n; i++) {\n"
"        for (int j = 0; j < i; j++) {\n"
"            sum += i * j;\n"
"        }\n"
"    }\n"
"    return sum;\n"
"}\n"
"int main(int argc, char **argv) {\n"
"    int iterations = 10;\n"
"    if (argc > 1) iterations = atoi(argv[1]);\n"
"    \n"
"    int result = process_data(iterations);\n"
"    printf(\"Result: %d\\n\", result);\n"
"    \n"
"    // Run again with different input\n"
"    if (iterations > 5) {\n"
"        result = process_data(iterations / 2);\n"
"        printf(\"Half result: %d\\n\", result);\n"
"    }\n"
"    return 0;\n"
"}\n";

/* Multi-file scenario - Scenario C (file1) */
const char *test_prog_c1 = 
"#include <stdio.h>\n"
"#include \"test_header.h\"\n"
"void helper_function() {\n"
"    printf(\"Helper called\\n\");\n"
"}\n"
"int main() {\n"
"    int x = compute_value(5);\n"
"    printf(\"Value: %d\\n\", x);\n"
"    helper_function();\n"
"    return 0;\n"
"}\n";

/* Multi-file scenario - Scenario C (file2) */
const char *test_prog_c2 = 
"#include \"test_header.h\"\n"
"int compute_value(int n) {\n"
"    return n * n + 1;\n"
"}\n";

/* Header for multi-file scenario */
const char *test_header = 
"#ifndef TEST_HEADER_H\n"
"#define TEST_HEADER_H\n"
"int compute_value(int n);\n"
"#endif\n";

/* Empty/zero counts program - Scenario D */
const char *test_prog_d = 
"#include <stdio.h>\n"
"int unused_function() {\n"
"    return 42;\n"
"}\n"
"int main() {\n"
"    // Don't call any instrumented functions\n"
"    printf(\"No coverage generated\\n\");\n"
"    return 0;\n"
"}\n";

/* Execute a shell command and check return status */
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

/* Compile a test program with coverage flags */
int compile_with_coverage(const char *src_file, const char *output_name, 
                          const char **additional_files, int num_additional) {
    char cmd[1024];
    
    if (num_additional == 0) {
        snprintf(cmd, sizeof(cmd), 
                 "gcc -O0 -fprofile-arcs -ftest-coverage -o %s %s",
                 output_name, src_file);
    } else {
        // Build command with multiple source files
        snprintf(cmd, sizeof(cmd), 
                 "gcc -O0 -fprofile-arcs -ftest-coverage -o %s %s",
                 output_name, src_file);
        
        for (int i = 0; i < num_additional; i++) {
            strncat(cmd, " ", sizeof(cmd) - strlen(cmd) - 1);
            strncat(cmd, additional_files[i], sizeof(cmd) - strlen(cmd) - 1);
        }
    }
    
    return execute_command(cmd);
}

/* Run the executable to generate .gcda files */
int run_executable(const char *exec_name, const char *args) {
    char cmd[1024];
    if (args && args[0]) {
        snprintf(cmd, sizeof(cmd), "./%s %s", exec_name, args);
    } else {
        snprintf(cmd, sizeof(cmd), "./%s", exec_name);
    }
    return execute_command(cmd);
}

/* Test gcov-tool overlap with various flag combinations */
void test_gcov_tool_overlap(const char *gcda_file, const char *gcno_file) {
    char cmd[1024];
    
    printf("\n=== Testing gcov-tool overlap with %s ===\n", gcda_file);
    
    /* Test case 'v' - verbose flag */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -v %s %s", gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 'f' - function level overlap */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -f %s %s", gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 'F' - use fullname */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -F %s %s", gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 'o' - object level overlap */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -o %s %s", gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 'h' - hot only */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -h %s %s", gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 't' with threshold 0.5 */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -t 0.5 %s %s", gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 't' with threshold 0.75 */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -t 0.75 %s %s", gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 't' with threshold 0.0 */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -t 0.0 %s %s", gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test case 't' with threshold 1.0 */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -t 1.0 %s %s", gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test combination of flags */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -v -f -o %s %s", gcda_file, gcno_file);
    execute_command(cmd);
    
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -F -h -t 0.3 %s %s", gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test default case with invalid option '-z' */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -z %s %s 2>/dev/null", gcda_file, gcno_file);
    execute_command(cmd);
    
    /* Test with no arguments to trigger usage */
    execute_command("gcov-tool overlap 2>/dev/null");
}

int main(int argc, char **argv) {
    printf("=== Generating test coverage data for gcov-tool overlap testing ===\n");
    
    /* Check if gcov-tool exists */
    if (system("which gcov-tool >/dev/null 2>&1") != 0) {
        fprintf(stderr, "Error: gcov-tool not found in PATH\n");
        fprintf(stderr, "Trying local build directory...\n");
        if (system("./gcov-tool --help >/dev/null 2>&1") != 0) {
            fprintf(stderr, "Error: gcov-tool not found locally either\n");
            fprintf(stderr, "Please build gcov-tool with coverage first:\n");
            fprintf(stderr, "  gcc -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
            return 1;
        }
    }
    
    /* Create test directory */
    execute_command("mkdir -p test_coverage_data");
    execute_command("cd test_coverage_data");
    
    /* Scenario A: Simple function coverage */
    printf("\n--- Scenario A: Simple function ---\n");
    write_to_file("test_a.c", test_prog_a);
    compile_with_coverage("test_a.c", "test_a", NULL, 0);
    run_executable("test_a", NULL);
    
    /* Scenario B: Loop-heavy program with multiple runs */
    printf("\n--- Scenario B: Loop-heavy program ---\n");
    write_to_file("test_b.c", test_prog_b);
    compile_with_coverage("test_b.c", "test_b", NULL, 0);
    run_executable("test_b", "5");
    run_executable("test_b", "8");
    run_executable("test_b", "3");
    
    /* Scenario C: Multiple source files */
    printf("\n--- Scenario C: Multiple source files ---\n");
    write_to_file("test_c1.c", test_prog_c1);
    write_to_file("test_c2.c", test_prog_c2);
    write_to_file("test_header.h", test_header);
    const char *additional_files[] = {"test_c2.c"};
    compile_with_coverage("test_c1.c", "test_c", additional_files, 1);
    run_executable("test_c", NULL);
    
    /* Scenario D: Empty/zero counts */
    printf("\n--- Scenario D: Zero coverage ---\n");
    write_to_file("test_d.c", test_prog_d);
    compile_with_coverage("test_d.c", "test_d", NULL, 0);
    run_executable("test_d", NULL);
    
    /* Now test gcov-tool overlap with each scenario */
    printf("\n=== Testing gcov-tool overlap with various flags ===\n");
    
    /* Test with Scenario A files */
    test_gcov_tool_overlap("test_a.gcda", "test_a.gcno");
    
    /* Test with Scenario B files */
    test_gcov_tool_overlap("test_b.gcda", "test_b.gcno");
    
    /* Test with Scenario C files (multiple .gcda files) */
    execute_command("gcov-tool overlap -v test_c1.gcda test_c1.gcno test_c2.gcda test_c2.gcno");
    execute_command("gcov-tool overlap -f -o test_c1.gcda test_c1.gcno test_c2.gcda test_c2.gcno");
    execute_command("gcov-tool overlap -F -h -t 0.5 test_c1.gcda test_c1.gcno test_c2.gcda test_c2.gcno");
    
    /* Test with Scenario D files (zero counts) */
    test_gcov_tool_overlap("test_d.gcda", "test_d.gcno");
    
    /* Test with multiple input files together */
    execute_command("gcov-tool overlap -v test_a.gcda test_a.gcno test_b.gcda test_b.gcno");
    execute_command("gcov-tool overlap -f -F test_a.gcda test_a.gcno test_d.gcda test_d.gcno");
    
    /* Additional edge cases */
    printf("\n=== Testing edge cases ===\n");
    
    /* Test with non-existent file (should still parse flags) */
    execute_command("gcov-tool overlap -v nonexistent.gcda nonexistent.gcno 2>/dev/null");
    
    /* Test with just flags and no files */
    execute_command("gcov-tool overlap -v -f 2>/dev/null");
    
    /* Test invalid threshold value */
    execute_command("gcov-tool overlap -t invalid test_a.gcda test_a.gcno 2>/dev/null");
    
    /* Test very large threshold */
    execute_command("gcov-tool overlap -t 999.9 test_a.gcda test_a.gcno");
    
    /* Test negative threshold */
    execute_command("gcov-tool overlap -t -0.5 test_a.gcda test_a.gcno");
    
    /* Cleanup */
    printf("\n=== Cleaning up test files ===\n");
    execute_command("cd ..");
    execute_command("rm -rf test_coverage_data");
    
    printf("\n=== Test completed ===\n");
    printf("The following gcov-tool overlap flags were tested:\n");
    printf("  -v (verbose)           - triggers case 'v' and gcov_set_verbose()\n");
    printf("  -f (function level)    - triggers case 'f', sets overlap_func_level\n");
    printf("  -F (fullname)          - triggers case 'F', sets overlap_use_fullname\n");
    printf("  -o (object level)      - triggers case 'o', sets overlap_obj_level\n");
    printf("  -h (hot only)          - triggers case 'h', sets overlap_hot_only\n");
    printf("  -t (threshold)         - triggers case 't', sets overlap_hot_threshold\n");
    printf("  -z (invalid)           - triggers default case and overlap_usage()\n");
    
    return 0;
}
