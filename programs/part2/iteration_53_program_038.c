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

/* Simple test programs to generate coverage data */

/* Scenario A: Simple function with basic branches */
const char *test_prog_a = 
"#include <stdio.h>\n"
"void func1(int x) {\n"
"    if (x > 0) {\n"
"        printf(\"Positive\\n\");\n"
"    } else {\n"
"        printf(\"Non-positive\\n\");\n"
"    }\n"
"}\n"
"int main() {\n"
"    func1(5);\n"
"    func1(-3);\n"
"    return 0;\n"
"}\n";

/* Scenario B: Loop-heavy program */
const char *test_prog_b = 
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

/* Scenario C: Multiple source files (part 1) */
const char *test_prog_c1 = 
"#include \"test_c.h\"\n"
"#include <stdio.h>\n"
"int helper1(int x) {\n"
"    return x * 2;\n"
"}\n"
"void public_func(int val) {\n"
"    if (val % 2 == 0) {\n"
"        printf(\"Even: %d\\n\", helper1(val));\n"
"    } else {\n"
"        printf(\"Odd: %d\\n\", val);\n"
"    }\n"
"}\n";

const char *test_prog_c2 = 
"#include \"test_c.h\"\n"
"#include <stdio.h>\n"
"int main() {\n"
"    public_func(4);\n"
"    public_func(7);\n"
"    return 0;\n"
"}\n";

const char *test_header_c = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"void public_func(int val);\n"
"#endif\n";

/* Scenario D: Program that may produce zero counts */
const char *test_prog_d = 
"#include <stdio.h>\n"
"int main(int argc, char **argv) {\n"
"    /* This code is instrumented but may not execute */\n"
"    if (argc > 100) {  /* Will never be true */\n"
"        printf(\"This never executes\\n\");\n"
"        int i;\n"
"        for (i = 0; i < 10; i++) {\n"
"            printf(\"Loop %d\\n\", i);\n"
"        }\n"
"    }\n"
"    return 0;\n"
"}\n";

/* Utility functions */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Command failed: %s\n", cmd);
    }
    return status;
}

int compile_with_coverage(const char *src_file, const char *exe_name, const char *extra_flags) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s %s 2>/dev/null",
             extra_flags ? extra_flags : "", exe_name, src_file);
    return execute_command(cmd);
}

int run_program(const char *exe_name, const char *args) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "./%s %s > /dev/null 2>&1", exe_name, args ? args : "");
    return execute_command(cmd);
}

int invoke_gcov_tool_overlap(const char *gcda_file, const char *gcno_file, const char *flags) {
    char cmd[MAX_CMD];
    const char *gcov_tool = "gcov-tool";
    
    /* Try to find gcov-tool in common locations */
    if (access("./gcov-tool", X_OK) == 0) {
        gcov_tool = "./gcov-tool";
    } else if (access("../gcov-tool", X_OK) == 0) {
        gcov_tool = "../gcov-tool";
    } else if (system("which gcov-tool > /dev/null 2>&1") != 0) {
        fprintf(stderr, "Error: gcov-tool not found in PATH or current directory\n");
        fprintf(stderr, "Please build gcov-tool with: gcc -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
        return -1;
    }
    
    snprintf(cmd, sizeof(cmd), "%s overlap %s %s %s 2>&1", 
             gcov_tool, flags, gcda_file, gcno_file);
    printf("\n=== Testing gcov-tool overlap with flags: '%s' ===\n", flags);
    return execute_command(cmd);
}

void write_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to write file");
        return;
    }
    fputs(content, f);
    fclose(f);
}

void cleanup_files(const char **files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) {
            remove(files[i]);
        }
    }
}

int main(int argc, char **argv) {
    printf("=== Test harness for gcov-tool overlap argument parsing ===\n");
    printf("Target: lines 534-554 in gcov-tool.cc\n\n");
    
    /* Track files for cleanup */
    const char *files_to_clean[50];
    int file_count = 0;
    
    /* Create and compile test programs */
    printf("--- Creating test programs with coverage instrumentation ---\n");
    
    /* Scenario A */
    write_file("test_a.c", test_prog_a);
    files_to_clean[file_count++] = "test_a.c";
    
    if (compile_with_coverage("test_a.c", "test_a_exe", NULL) != 0) {
        fprintf(stderr, "Failed to compile test_a.c\n");
        return 1;
    }
    files_to_clean[file_count++] = "test_a_exe";
    
    run_program("test_a_exe", NULL);
    
    /* Scenario B */
    write_file("test_b.c", test_prog_b);
    files_to_clean[file_count++] = "test_b.c";
    
    if (compile_with_coverage("test_b.c", "test_b_exe", NULL) != 0) {
        fprintf(stderr, "Failed to compile test_b.c\n");
        return 1;
    }
    files_to_clean[file_count++] = "test_b_exe";
    
    run_program("test_b_exe", "2");  // Run with argument
    run_program("test_b_exe", "4");  // Run again with different argument
    
    /* Scenario C (multiple files) */
    write_file("test_c1.c", test_prog_c1);
    write_file("test_c2.c", test_prog_c2);
    write_file("test_c.h", test_header_c);
    files_to_clean[file_count++] = "test_c1.c";
    files_to_clean[file_count++] = "test_c2.c";
    files_to_clean[file_count++] = "test_c.h";
    
    if (compile_with_coverage("test_c1.c test_c2.c", "test_c_exe", "-I.") != 0) {
        fprintf(stderr, "Failed to compile test_c files\n");
        return 1;
    }
    files_to_clean[file_count++] = "test_c_exe";
    
    run_program("test_c_exe", NULL);
    
    /* Scenario D (zero counts) */
    write_file("test_d.c", test_prog_d);
    files_to_clean[file_count++] = "test_d.c";
    
    if (compile_with_coverage("test_d.c", "test_d_exe", NULL) != 0) {
        fprintf(stderr, "Failed to compile test_d.c\n");
        return 1;
    }
    files_to_clean[file_count++] = "test_d_exe";
    
    /* Don't run this one - keep zero counts */
    
    printf("\n--- Testing gcov-tool overlap with various flags ---\n");
    
    /* Test case 1: -v flag (verbose) - triggers case 'v' */
    printf("\n>>> Testing -v flag <<<\n");
    invoke_gcov_tool_overlap("test_a_exe-test_a.gcda", "test_a.gcno", "-v");
    
    /* Test case 2: -f flag (function level) - triggers case 'f' */
    printf("\n>>> Testing -f flag <<<\n");
    invoke_gcov_tool_overlap("test_b_exe-test_b.gcda", "test_b.gcno", "-f");
    
    /* Test case 3: -F flag (fullname) - triggers case 'F' */
    printf("\n>>> Testing -F flag <<<\n");
    invoke_gcov_tool_overlap("test_a_exe-test_a.gcda test_b_exe-test_b.gcda", 
                            "test_a.gcno test_b.gcno", "-F");
    
    /* Test case 4: -o flag (object level) - triggers case 'o' */
    printf("\n>>> Testing -o flag <<<\n");
    invoke_gcov_tool_overlap("test_c_exe-test_c1.gcda", "test_c1.gcno", "-o");
    
    /* Test case 5: -h flag (hot only) - triggers case 'h' */
    printf("\n>>> Testing -h flag <<<\n");
    invoke_gcov_tool_overlap("test_b_exe-test_b.gcda", "test_b.gcno", "-h");
    
    /* Test case 6: -t flag with threshold - triggers case 't' */
    printf("\n>>> Testing -t 0.5 flag <<<\n");
    invoke_gcov_tool_overlap("test_a_exe-test_a.gcda", "test_a.gcno", "-t 0.5");
    
    /* Test case 7: -t flag with different threshold */
    printf("\n>>> Testing -t 0.75 flag <<<\n");
    invoke_gcov_tool_overlap("test_b_exe-test_b.gcda", "test_b.gcno", "-t 0.75");
    
    /* Test case 8: -t flag with very low threshold */
    printf("\n>>> Testing -t 0.1 flag <<<\n");
    invoke_gcov_tool_overlap("test_d_exe-test_d.gcda", "test_d.gcno", "-t 0.1");
    
    /* Test case 9: Combination of flags */
    printf("\n>>> Testing -v -f -o combination <<<\n");
    invoke_gcov_tool_overlap("test_a_exe-test_a.gcda test_b_exe-test_b.gcda", 
                            "test_a.gcno test_b.gcno", "-v -f -o");
    
    /* Test case 10: Another combination */
    printf("\n>>> Testing -F -h -t 0.3 combination <<<\n");
    invoke_gcov_tool_overlap("test_c_exe-test_c1.gcda test_c_exe-test_c2.gcda", 
                            "test_c1.gcno test_c2.gcno", "-F -h -t 0.3");
    
    /* Test case 11: Invalid option to trigger default case and overlap_usage() */
    printf("\n>>> Testing invalid option -z (should trigger default case) <<<\n");
    char cmd[MAX_CMD];
    const char *gcov_tool = access("./gcov-tool", X_OK) == 0 ? "./gcov-tool" : "gcov-tool";
    snprintf(cmd, sizeof(cmd), "%s overlap -z 2>&1 | head -20", gcov_tool);
    execute_command(cmd);
    
    /* Test case 12: Another invalid option */
    printf("\n>>> Testing invalid option --invalid (should trigger default case) <<<\n");
    snprintf(cmd, sizeof(cmd), "%s overlap --invalid 2>&1 | head -20", gcov_tool);
    execute_command(cmd);
    
    /* Test case 13: Valid flags but with invalid file (should still parse flags) */
    printf("\n>>> Testing -v -f with non-existent file (parsing should still happen) <<<\n");
    invoke_gcov_tool_overlap("nonexistent.gcda", "nonexistent.gcno", "-v -f");
    
    /* Test case 14: Multiple -t flags (last one wins) */
    printf("\n>>> Testing multiple -t flags <<<\n");
    invoke_gcov_tool_overlap("test_a_exe-test_a.gcda", "test_a.gcno", "-t 0.2 -t 0.8");
    
    /* Test case 15: All flags together */
    printf("\n>>> Testing all flags together <<<\n");
    invoke_gcov_tool_overlap("test_a_exe-test_a.gcda test_b_exe-test_b.gcda test_c_exe-test_c1.gcda",
                            "test_a.gcno test_b.gcno test_c1.gcno",
                            "-v -f -F -o -h -t 0.6");
    
    printf("\n--- Cleaning up generated files ---\n");
    
    /* Clean up executables */
    cleanup_files(files_to_clean, file_count);
    
    /* Clean up coverage files */
    const char *coverage_files[] = {
        "test_a.gcda", "test_a.gcno",
        "test_b.gcda", "test_b.gcno",
        "test_c1.gcda", "test_c1.gcno",
        "test_c2.gcda", "test_c2.gcno",
        "test_d.gcda", "test_d.gcno",
        "test_a_exe-test_a.gcda", "test_b_exe-test_b.gcda",
        "test_c_exe-test_c1.gcda", "test_c_exe-test_c2.gcda",
        "test_d_exe-test_d.gcda"
    };
    
    for (int i = 0; i < sizeof(coverage_files)/sizeof(coverage_files[0]); i++) {
        remove(coverage_files[i]);
    }
    
    printf("\n=== Test completed ===\n");
    printf("The following gcov-tool overlap flags were tested:\n");
    printf("  -v (verbose)           - triggers case 'v' and gcov_set_verbose()\n");
    printf("  -f (function level)    - triggers case 'f'\n");
    printf("  -F (fullname)          - triggers case 'F'\n");
    printf("  -o (object level)      - triggers case 'o'\n");
    printf("  -h (hot only)          - triggers case 'h'\n");
    printf("  -t N (threshold)       - triggers case 't' with atof(optarg)\n");
    printf("  -z (invalid)           - triggers default case and overlap_usage()\n");
    
    return 0;
}
