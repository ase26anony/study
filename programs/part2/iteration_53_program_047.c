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

/* Simple test programs to generate varied coverage data */

/* Scenario A: Simple function with basic branches */
const char *test_a = 
"#include <stdio.h>\n"
"void func1(int x) {\n"
"    if (x > 0) {\n"
"        printf(\"Positive\\n\");\n"
"    } else {\n"
"        printf(\"Non-positive\\n\");\n"
"    }\n"
"}\n"
"int main() {\n"
"    func1(1);\n"
"    func1(-1);\n"
"    return 0;\n"
"}\n";

/* Scenario B: Loop-heavy program */
const char *test_b = 
"#include <stdio.h>\n"
"int main(int argc, char **argv) {\n"
"    int i, j, iterations = 3;\n"
"    if (argc > 1) iterations = atoi(argv[1]);\n"
"    \n"
"    for (i = 0; i < iterations; i++) {\n"
"        for (j = 0; j < i + 1; j++) {\n"
"            printf(\"*\");\n"
"        }\n"
"        printf(\"\\n\");\n"
"    }\n"
"    \n"
"    int sum = 0;\n"
"    for (i = 0; i < 100; i++) {\n"
"        sum += i;\n"
"    }\n"
"    printf(\"Sum: %d\\n\", sum);\n"
"    return 0;\n"
"}\n";

/* Scenario C: Multiple source files - file 1 */
const char *test_c1 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"void helper1() {\n"
"    printf(\"Helper1 called\\n\");\n"
"}\n"
"int main() {\n"
"    helper1();\n"
"    helper2();\n"
"    return 0;\n"
"}\n";

/* Scenario C: Multiple source files - file 2 */
const char *test_c2 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"void helper2() {\n"
"    printf(\"Helper2 called\\n\");\n"
"}\n";

/* Scenario C: Header file */
const char *test_c_header = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"void helper1();\n"
"void helper2();\n"
"#endif\n";

/* Scenario D: Program that may produce zero counts */
const char *test_d = 
"#include <stdio.h>\n"
"int main(int argc, char **argv) {\n"
"    /* This path is never taken when argc == 1 */\n"
"    if (argc > 2) {\n"
"        printf(\"This should not print\\n\");\n"
"        int i;\n"
"        for (i = 0; i < 10; i++) {\n"
"            printf(\"Loop %d\\n\", i);\n"
"        }\n"
"    }\n"
"    return 0;\n"
"}\n";

/* Utility functions */
int run_command(const char *cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        printf("Command failed with status %d: %s\n", status, cmd);
    }
    return status;
}

int compile_with_coverage(const char *src, const char *output, const char *extra_flags) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s",
             extra_flags ? extra_flags : "", output);
    
    /* Write source to temp file */
    char src_file[256];
    snprintf(src_file, sizeof(src_file), "%s.c", output);
    FILE *f = fopen(src_file, "w");
    if (!f) {
        perror("Failed to create source file");
        return -1;
    }
    fputs(src, f);
    fclose(f);
    
    /* Compile */
    strcat(cmd, " ");
    strcat(cmd, src_file);
    
    return run_command(cmd);
}

int run_gcov_tool_overlap(const char *args, const char *gcda_files) {
    char cmd[2048];
    /* Try to find gcov-tool in common locations */
    const char *gcov_tool_paths[] = {
        "./gcov-tool",
        "../gcc/build/gcc/gcov-tool",
        "/usr/bin/gcov-tool",
        "/usr/local/bin/gcov-tool",
        NULL
    };
    
    const char *gcov_tool = NULL;
    for (int i = 0; gcov_tool_paths[i]; i++) {
        if (access(gcov_tool_paths[i], X_OK) == 0) {
            gcov_tool = gcov_tool_paths[i];
            break;
        }
    }
    
    if (!gcov_tool) {
        printf("ERROR: gcov-tool not found in any expected location\n");
        printf("Please ensure gcov-tool is built and in PATH\n");
        return -1;
    }
    
    snprintf(cmd, sizeof(cmd), "%s overlap %s %s", gcov_tool, args, gcda_files);
    return run_command(cmd);
}

void cleanup_files(const char *base_name) {
    char pattern[256];
    snprintf(pattern, sizeof(pattern), "%s*", base_name);
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -f %s 2>/dev/null", pattern);
    system(cmd);
}

int main() {
    printf("=== Test Harness for gcov-tool overlap flag parsing ===\n\n");
    
    /* Clean up any previous test files */
    cleanup_files("test_a");
    cleanup_files("test_b");
    cleanup_files("test_c");
    cleanup_files("test_d");
    cleanup_files("test_c1");
    cleanup_files("test_c2");
    cleanup_files("test_c.h");
    
    /* Step 1: Compile test programs with coverage */
    printf("1. Compiling test programs with coverage flags...\n");
    
    if (compile_with_coverage(test_a, "test_a", "") != 0) {
        printf("Failed to compile test_a\n");
        return 1;
    }
    
    if (compile_with_coverage(test_b, "test_b", "") != 0) {
        printf("Failed to compile test_b\n");
        return 1;
    }
    
    /* Create multi-file test C */
    FILE *f;
    f = fopen("test_c1.c", "w");
    fputs(test_c1, f);
    fclose(f);
    
    f = fopen("test_c2.c", "w");
    fputs(test_c2, f);
    fclose(f);
    
    f = fopen("test_c.h", "w");
    fputs(test_c_header, f);
    fclose(f);
    
    if (run_command("gcc -O0 -fprofile-arcs -ftest-coverage test_c1.c test_c2.c -o test_c") != 0) {
        printf("Failed to compile test_c\n");
        return 1;
    }
    
    if (compile_with_coverage(test_d, "test_d", "") != 0) {
        printf("Failed to compile test_d\n");
        return 1;
    }
    
    /* Step 2: Run test programs to generate .gcda files */
    printf("\n2. Running test programs to generate coverage data...\n");
    
    run_command("./test_a");
    run_command("./test_b 2");  /* Run with argument for varied counts */
    run_command("./test_b 5");  /* Run again to accumulate counts */
    run_command("./test_c");
    run_command("./test_d");    /* This will produce zero counts in .gcda */
    
    /* Step 3: Invoke gcov-tool overlap with various flags */
    printf("\n3. Testing gcov-tool overlap with different flag combinations...\n");
    
    /* Test case 1: -v flag (verbose) - triggers case 'v' */
    printf("\n--- Test 1: -v flag (verbose) ---\n");
    run_gcov_tool_overlap("-v", "test_a.gcda test_a.gcno");
    
    /* Test case 2: -f flag (function level) - triggers case 'f' */
    printf("\n--- Test 2: -f flag (function level) ---\n");
    run_gcov_tool_overlap("-f", "test_b.gcda test_b.gcno");
    
    /* Test case 3: -F flag (fullname) - triggers case 'F' */
    printf("\n--- Test 3: -F flag (fullname) ---\n");
    run_gcov_tool_overlap("-F", "test_c.gcda test_c.gcno");
    
    /* Test case 4: -o flag (object level) - triggers case 'o' */
    printf("\n--- Test 4: -o flag (object level) ---\n");
    run_gcov_tool_overlap("-o", "test_a.gcda test_b.gcda test_a.gcno test_b.gcno");
    
    /* Test case 5: -h flag (hot only) - triggers case 'h' */
    printf("\n--- Test 5: -h flag (hot only) ---\n");
    run_gcov_tool_overlap("-h", "test_b.gcda test_b.gcno");
    
    /* Test case 6: -t flag with threshold - triggers case 't' */
    printf("\n--- Test 6: -t flag with threshold 0.5 ---\n");
    run_gcov_tool_overlap("-t 0.5", "test_b.gcda test_b.gcno");
    
    /* Test case 7: -t flag with different threshold - triggers case 't' */
    printf("\n--- Test 7: -t flag with threshold 0.75 ---\n");
    run_gcov_tool_overlap("-t 0.75", "test_b.gcda test_b.gcno");
    
    /* Test case 8: Combination of flags */
    printf("\n--- Test 8: Combination -v -f -o ---\n");
    run_gcov_tool_overlap("-v -f -o", "test_a.gcda test_b.gcda test_a.gcno test_b.gcno");
    
    /* Test case 9: Another combination with -F and -h */
    printf("\n--- Test 9: Combination -F -h ---\n");
    run_gcov_tool_overlap("-F -h", "test_c.gcda test_c.gcno");
    
    /* Test case 10: Test with zero-count .gcda files */
    printf("\n--- Test 10: Testing with zero-count data (-t 0.1) ---\n");
    run_gcov_tool_overlap("-t 0.1", "test_d.gcda test_d.gcno");
    
    /* Test case 11: Invalid option to trigger default case and overlap_usage() */
    printf("\n--- Test 11: Invalid option to trigger default case ---\n");
    run_gcov_tool_overlap("-z", "test_a.gcda test_a.gcno");
    
    /* Test case 12: Multiple files with various flags */
    printf("\n--- Test 12: All test files with verbose and function level ---\n");
    run_gcov_tool_overlap("-v -f", 
                         "test_a.gcda test_b.gcda test_c.gcda test_d.gcda "
                         "test_a.gcno test_b.gcno test_c.gcno test_d.gcno");
    
    /* Step 4: Cleanup */
    printf("\n4. Cleaning up test files...\n");
    cleanup_files("test_a");
    cleanup_files("test_b");
    cleanup_files("test_c");
    cleanup_files("test_d");
    cleanup_files("test_c1");
    cleanup_files("test_c2");
    cleanup_files("test_c.h");
    cleanup_files("*.gcda");
    cleanup_files("*.gcno");
    
    printf("\n=== Test completed ===\n");
    printf("The following gcov-tool overlap flags were tested:\n");
    printf("  -v (verbose)           - case 'v'\n");
    printf("  -f (function level)    - case 'f'\n");
    printf("  -F (fullname)          - case 'F'\n");
    printf("  -o (object level)      - case 'o'\n");
    printf("  -h (hot only)          - case 'h'\n");
    printf("  -t <threshold>         - case 't' (with values 0.5, 0.75, 0.1)\n");
    printf("  -z (invalid)           - default case (triggers overlap_usage())\n");
    printf("\nTo measure coverage on gcov-tool itself:\n");
    printf("1. Build gcov-tool with coverage: gcc -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
    printf("2. Run this test program\n");
    printf("3. Check gcov-tool.gcda for coverage of the target switch block\n");
    
    return 0;
}
