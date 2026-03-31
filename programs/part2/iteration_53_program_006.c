/**
 * Test harness for gcov-tool overlap command-line parsing coverage.
 * This program creates multiple test scenarios, generates coverage data,
 * and invokes gcov-tool with various flag combinations to cover lines 534-554
 * in gcov-tool.cc.
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
void cleanup_files(const char *base_name);

/* Test scenario A: Simple function with conditionals */
const char *test_a_source = 
"#include <stdio.h>\n"
"void func1(int x) {\n"
"    if (x > 0) {\n"
"        printf(\"Positive\\n\");\n"
"    } else {\n"
"        printf(\"Non-positive\\n\");\n"
"    }\n"
"}\n"
"void func2(int y) {\n"
"    for (int i = 0; i < y; i++) {\n"
"        printf(\"Iteration %d\\n\", i);\n"
"    }\n"
"}\n"
"int main() {\n"
"    func1(5);\n"
"    func1(-3);\n"
"    func2(3);\n"
"    return 0;\n"
"}\n";

/* Test scenario B: Loop-heavy program */
const char *test_b_source = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"int process_matrix(int size) {\n"
"    int sum = 0;\n"
"    for (int i = 0; i < size; i++) {\n"
"        for (int j = 0; j < size; j++) {\n"
"            sum += i * j;\n"
"            if (sum > 1000) {\n"
"                sum -= 500;\n"
"            }\n"
"        }\n"
"    }\n"
"    return sum;\n"
"}\n"
"int main(int argc, char **argv) {\n"
"    int iterations = 2;\n"
"    if (argc > 1) {\n"
"        iterations = atoi(argv[1]);\n"
"    }\n"
"    int total = 0;\n"
"    for (int k = 0; k < iterations; k++) {\n"
"        total += process_matrix(5 + k);\n"
"    }\n"
"    printf(\"Total: %d\\n\", total);\n"
"    return 0;\n"
"}\n";

/* Test scenario C: Multiple source files */
const char *test_c1_source = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"int helper1(int x) {\n"
"    return x * 2;\n"
"}\n"
"int main() {\n"
"    int result = helper1(10) + helper2(5);\n"
"    printf(\"Result: %d\\n\", result);\n"
"    return 0;\n"
"}\n";

const char *test_c2_source = 
"#include \"test_c.h\"\n"
"int helper2(int y) {\n"
"    if (y > 0) {\n"
"        return y + 5;\n"
"    }\n"
"    return y - 5;\n"
"}\n";

const char *test_c_header = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"int helper1(int x);\n"
"int helper2(int y);\n"
"#endif\n";

/* Test scenario D: Program with zero coverage */
const char *test_d_source = 
"#include <stdio.h>\n"
"int never_called() {\n"
"    return 42;\n"
"}\n"
"int main() {\n"
"    // Don't call any instrumented functions\n"
"    printf(\"No coverage generated\\n\");\n"
"    return 0;\n"
"}\n";

/* Execute a shell command and return exit status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status == -1) {
        perror("system() failed");
        return -1;
    }
    return WEXITSTATUS(status);
}

/* Compile a C source file with coverage flags */
int compile_with_coverage(const char *source, const char *output) {
    char cmd[MAX_PATH * 2];
    snprintf(cmd, sizeof(cmd), "gcc -O0 -fprofile-arcs -ftest-coverage -o %s -xc -", output);
    
    FILE *fp = popen(cmd, "w");
    if (!fp) {
        perror("popen failed");
        return -1;
    }
    
    fwrite(source, 1, strlen(source), fp);
    return pclose(fp);
}

/* Compile multiple source files */
int compile_multiple_with_coverage(const char **sources, const char **filenames, 
                                   int count, const char *output) {
    char cmd[MAX_PATH * 4] = "gcc -O0 -fprofile-arcs -ftest-coverage -o ";
    strcat(cmd, output);
    
    for (int i = 0; i < count; i++) {
        // Write source to temporary file
        FILE *fp = fopen(filenames[i], "w");
        if (!fp) {
            perror("fopen failed");
            return -1;
        }
        fwrite(sources[i], 1, strlen(sources[i]), fp);
        fclose(fp);
        
        strcat(cmd, " ");
        strcat(cmd, filenames[i]);
    }
    
    return execute_command(cmd);
}

/* Run a program and capture its output */
int run_program(const char *program, const char *args) {
    char cmd[MAX_PATH * 2];
    if (args && args[0]) {
        snprintf(cmd, sizeof(cmd), "./%s %s > /dev/null 2>&1", program, args);
    } else {
        snprintf(cmd, sizeof(cmd), "./%s > /dev/null 2>&1", program);
    }
    return execute_command(cmd);
}

/* Invoke gcov-tool with specific arguments */
int invoke_gcov_tool(const char *args, const char *gcda_file, const char *gcno_file) {
    char cmd[MAX_PATH * 4];
    
    // First try to find gcov-tool in common locations
    const char *gcov_tool_path = "gcov-tool";
    
    // Test if gcov-tool exists by running with --help
    if (system("gcov-tool --help > /dev/null 2>&1") != 0) {
        // Try in current directory
        if (system("./gcov-tool --help > /dev/null 2>&1") == 0) {
            gcov_tool_path = "./gcov-tool";
        } else {
            fprintf(stderr, "Error: gcov-tool not found in PATH or current directory\n");
            fprintf(stderr, "Please ensure gcov-tool is built and available\n");
            return -1;
        }
    }
    
    if (gcda_file && gcno_file) {
        snprintf(cmd, sizeof(cmd), "%s overlap %s %s %s 2>&1", 
                gcov_tool_path, args, gcda_file, gcno_file);
    } else {
        snprintf(cmd, sizeof(cmd), "%s overlap %s 2>&1", gcov_tool_path, args);
    }
    
    printf("\n=== Testing gcov-tool with: %s ===\n", args);
    int result = execute_command(cmd);
    printf("Exit code: %d\n", result);
    return result;
}

/* Clean up generated files */
void cleanup_files(const char *base_name) {
    char pattern[MAX_PATH];
    snprintf(pattern, sizeof(pattern), "rm -f %s %s.gcda %s.gcno %s-*.gcda %s-*.gcno",
             base_name, base_name, base_name, base_name, base_name);
    system(pattern);
}

int main(int argc, char **argv) {
    printf("=== Starting gcov-tool overlap command-line parsing test ===\n\n");
    
    int overall_result = 0;
    
    /* Test Scenario A: Simple function */
    printf("--- Test Scenario A: Simple function with conditionals ---\n");
    if (compile_with_coverage(test_a_source, "test_a") == 0) {
        run_program("test_a", NULL);
        
        // Test various flag combinations to cover the switch cases
        invoke_gcov_tool("-v", "test_a.gcda", "test_a.gcno");           // case 'v'
        invoke_gcov_tool("-f", "test_a.gcda", "test_a.gcno");           // case 'f'
        invoke_gcov_tool("-F", "test_a.gcda", "test_a.gcno");           // case 'F'
        invoke_gcov_tool("-o", "test_a.gcda", "test_a.gcno");           // case 'o'
        invoke_gcov_tool("-h", "test_a.gcda", "test_a.gcno");           // case 'h'
        invoke_gcov_tool("-t 0.5", "test_a.gcda", "test_a.gcno");       // case 't' with argument
        invoke_gcov_tool("-t 0.75", "test_a.gcda", "test_a.gcno");      // case 't' with different value
        invoke_gcov_tool("-t 1.0", "test_a.gcda", "test_a.gcno");       // case 't' with 1.0
        
        // Test combinations
        invoke_gcov_tool("-v -f -o", "test_a.gcda", "test_a.gcno");     // Multiple flags
        invoke_gcov_tool("-f -F -o -h", "test_a.gcda", "test_a.gcno");  // More combinations
        invoke_gcov_tool("-v -t 0.3", "test_a.gcda", "test_a.gcno");    // Verbose with threshold
        
        // Test invalid option to trigger default case
        invoke_gcov_tool("-z", "test_a.gcda", "test_a.gcno");           // default case
        
        // Test with no arguments (should show usage)
        invoke_gcov_tool("", NULL, NULL);                               // No files provided
    }
    
    /* Test Scenario B: Loop-heavy program with multiple runs */
    printf("\n--- Test Scenario B: Loop-heavy program ---\n");
    if (compile_with_coverage(test_b_source, "test_b") == 0) {
        // Run multiple times with different arguments to generate varied counts
        run_program("test_b", "1");
        run_program("test_b", "2");
        run_program("test_b", "3");
        
        // Test hot threshold with different values
        invoke_gcov_tool("-t 0.1", "test_b.gcda", "test_b.gcno");       // Low threshold
        invoke_gcov_tool("-t 0.9", "test_b.gcda", "test_b.gcno");       // High threshold
        invoke_gcov_tool("-h -t 0.5", "test_b.gcda", "test_b.gcno");    // Hot only with threshold
        invoke_gcov_tool("-f -F -o -t 0.3", "test_b.gcda", "test_b.gcno"); // All flags
    }
    
    /* Test Scenario C: Multiple source files */
    printf("\n--- Test Scenario C: Multiple source files ---\n");
    const char *c_sources[] = {test_c1_source, test_c2_source};
    const char *c_filenames[] = {"test_c1.c", "test_c2.c"};
    
    // Write header file
    FILE *header_fp = fopen("test_c.h", "w");
    if (header_fp) {
        fwrite(test_c_header, 1, strlen(test_c_header), header_fp);
        fclose(header_fp);
    }
    
    if (compile_multiple_with_coverage(c_sources, c_filenames, 2, "test_c") == 0) {
        run_program("test_c", NULL);
        
        // Test with fullname flag on multi-file scenario
        invoke_gcov_tool("-F", "test_c1.gcda", "test_c1.gcno");
        invoke_gcov_tool("-F", "test_c2.gcda", "test_c2.gcno");
        invoke_gcov_tool("-f -F -o", "test_c1.gcda", "test_c1.gcno");
        
        // Test with multiple input files
        char multi_cmd[MAX_PATH * 4];
        snprintf(multi_cmd, sizeof(multi_cmd), 
                "gcov-tool overlap -v test_c1.gcda test_c1.gcno test_c2.gcda test_c2.gcno 2>&1");
        printf("\n=== Testing with multiple .gcda files ===\n");
        execute_command(multi_cmd);
    }
    
    /* Test Scenario D: Zero coverage */
    printf("\n--- Test Scenario D: Zero coverage program ---\n");
    if (compile_with_coverage(test_d_source, "test_d") == 0) {
        run_program("test_d", NULL);
        
        // Test hot threshold with zero counts
        invoke_gcov_tool("-t 0.5", "test_d.gcda", "test_d.gcno");
        invoke_gcov_tool("-h -t 0.0", "test_d.gcda", "test_d.gcno");  // Zero threshold
        
        // Test invalid threshold value
        invoke_gcov_tool("-t invalid", "test_d.gcda", "test_d.gcno"); // Should trigger error
    }
    
    /* Additional edge case tests */
    printf("\n--- Additional edge cases ---\n");
    
    // Test with non-existent files
    invoke_gcov_tool("-v", "nonexistent.gcda", "nonexistent.gcno");
    
    // Test with malformed arguments
    invoke_gcov_tool("-t", "test_a.gcda", "test_a.gcno");  // Missing threshold value
    invoke_gcov_tool("--invalid-option", NULL, NULL);      // Long invalid option
    
    // Test all flags together
    invoke_gcov_tool("-v -f -F -o -h -t 0.6", "test_a.gcda", "test_a.gcno");
    
    /* Cleanup */
    printf("\n=== Cleaning up test files ===\n");
    cleanup_files("test_a");
    cleanup_files("test_b");
    cleanup_files("test_c");
    cleanup_files("test_d");
    system("rm -f test_c1.c test_c2.c test_c.h");
    
    printf("\n=== Test completed ===\n");
    printf("Note: Check gcov-tool's own .gcda file for coverage of the target lines.\n");
    printf("The target lines (534-554 in gcov-tool.cc) should now be covered if\n");
    printf("gcov-tool was built with profiling enabled.\n");
    
    return overall_result;
}
