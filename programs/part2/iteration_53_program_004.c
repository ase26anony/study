/* test_gcov_tool_overlap.c - Test harness for gcov-tool overlap command parsing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Simple test programs to generate varied coverage data */

/* Scenario A: Simple function with conditionals */
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
"    func1(5);\n"
"    func1(-3);\n"
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
"        sum += i % 7;\n"
"    }\n"
"    printf(\"Sum: %d\\n\", sum);\n"
"    return 0;\n"
"}\n";

/* Scenario C: Multiple source files - part 1 */
const char *test_c1 = 
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

const char *test_c2 = 
"#include \"test_c.h\"\n"
"int helper2(int y) {\n"
"    if (y > 0) {\n"
"        return y + 1;\n"
"    }\n"
"    return y - 1;\n"
"}\n";

const char *test_c_header = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"int helper1(int x);\n"
"int helper2(int y);\n"
"#endif\n";

/* Scenario D: Program with zero coverage */
const char *test_d = 
"#include <stdio.h>\n"
"int never_called() {\n"
"    return 42;\n"
"}\n"
"int main() {\n"
"    /* Don't call any instrumented functions */\n"
"    printf(\"No coverage generated\\n\");\n"
"    return 0;\n"
"}\n";

/* Utility functions */
int compile_with_coverage(const char *source, const char *output, 
                         const char *extra_flags) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s",
             extra_flags ? extra_flags : "", output);
    
    /* Write source to temp file */
    char src_file[256];
    snprintf(src_file, sizeof(src_file), "/tmp/%s.c", output);
    FILE *f = fopen(src_file, "w");
    if (!f) {
        perror("Failed to create source file");
        return 0;
    }
    fputs(source, f);
    fclose(f);
    
    /* Compile */
    strcat(cmd, " ");
    strcat(cmd, src_file);
    
    printf("Compiling: %s\n", cmd);
    int ret = system(cmd);
    return (ret == 0);
}

int run_program(const char *program, const char *args) {
    char cmd[512];
    if (args) {
        snprintf(cmd, sizeof(cmd), "./%s %s", program, args);
    } else {
        snprintf(cmd, sizeof(cmd), "./%s", program);
    }
    
    printf("Running: %s\n", cmd);
    return system(cmd) == 0;
}

int run_gcov_tool(const char *args) {
    char cmd[1024];
    /* Try to find gcov-tool in PATH or current directory */
    const char *gcov_tool = "gcov-tool";
    
    snprintf(cmd, sizeof(cmd), "%s %s", gcov_tool, args);
    printf("\nExecuting: %s\n", cmd);
    
    int ret = system(cmd);
    if (ret != 0) {
        printf("Note: gcov-tool returned %d (this may be expected for invalid args)\n", ret);
    }
    return 1; /* We consider execution successful regardless of exit code */
}

void cleanup_files(const char *base_name) {
    char pattern[256];
    snprintf(pattern, sizeof(pattern), 
             "rm -f %s %s.gcda %s.gcno /tmp/%s.c 2>/dev/null", 
             base_name, base_name, base_name, base_name);
    system(pattern);
}

int main() {
    printf("=== Testing gcov-tool overlap command parsing ===\n\n");
    
    /* Step 1: Compile and run test programs to generate coverage data */
    printf("--- Generating coverage data ---\n");
    
    /* Scenario A */
    if (!compile_with_coverage(test_a, "test_a", NULL)) {
        fprintf(stderr, "Failed to compile test_a\n");
        return 1;
    }
    run_program("test_a", NULL);
    
    /* Scenario B - run multiple times with different inputs */
    if (!compile_with_coverage(test_b, "test_b", NULL)) {
        fprintf(stderr, "Failed to compile test_b\n");
        return 1;
    }
    run_program("test_b", "2");
    run_program("test_b", "5");
    
    /* Scenario C - multiple source files */
    /* Write header file */
    FILE *hdr = fopen("test_c.h", "w");
    if (hdr) {
        fputs(test_c_header, hdr);
        fclose(hdr);
    }
    
    /* Write source files */
    FILE *c1 = fopen("test_c1.c", "w");
    if (c1) {
        fputs(test_c1, c1);
        fclose(c1);
    }
    
    FILE *c2 = fopen("test_c2.c", "w");
    if (c2) {
        fputs(test_c2, c2);
        fclose(c2);
    }
    
    /* Compile with multiple source files */
    system("gcc -O0 -fprofile-arcs -ftest-coverage test_c1.c test_c2.c -o test_c");
    run_program("test_c", NULL);
    
    /* Scenario D - zero coverage */
    if (!compile_with_coverage(test_d, "test_d", NULL)) {
        fprintf(stderr, "Failed to compile test_d\n");
        return 1;
    }
    run_program("test_d", NULL);
    
    printf("\n--- Testing gcov-tool overlap with various flags ---\n");
    
    /* Test case 1: -v flag (verbose) - triggers case 'v' */
    printf("\n1. Testing -v flag (case 'v'):\n");
    run_gcov_tool("overlap -v test_a.gcda test_a.gcno");
    
    /* Test case 2: -f flag (function level) - triggers case 'f' */
    printf("\n2. Testing -f flag (case 'f'):\n");
    run_gcov_tool("overlap -f test_a.gcda test_b.gcda");
    
    /* Test case 3: -F flag (fullname) - triggers case 'F' */
    printf("\n3. Testing -F flag (case 'F'):\n");
    run_gcov_tool("overlap -F test_a.gcda test_b.gcda");
    
    /* Test case 4: -o flag (object level) - triggers case 'o' */
    printf("\n4. Testing -o flag (case 'o'):\n");
    run_gcov_tool("overlap -o test_a.gcda test_b.gcda");
    
    /* Test case 5: -h flag (hot only) - triggers case 'h' */
    printf("\n5. Testing -h flag (case 'h'):\n");
    run_gcov_tool("overlap -h test_a.gcda test_b.gcda");
    
    /* Test case 6: -t flag with threshold (case 't') */
    printf("\n6. Testing -t flag with threshold 0.5 (case 't'):\n");
    run_gcov_tool("overlap -t 0.5 test_a.gcda test_b.gcda");
    
    /* Test case 7: -t flag with different threshold */
    printf("\n7. Testing -t flag with threshold 0.75:\n");
    run_gcov_tool("overlap -t 0.75 test_a.gcda test_b.gcda");
    
    /* Test case 8: -t flag with threshold 0.0 */
    printf("\n8. Testing -t flag with threshold 0.0:\n");
    run_gcov_tool("overlap -t 0.0 test_a.gcda test_d.gcda");
    
    /* Test case 9: Combination of flags */
    printf("\n9. Testing combination -v -f -o:\n");
    run_gcov_tool("overlap -v -f -o test_a.gcda test_b.gcda");
    
    printf("\n10. Testing combination -F -h -t 0.3:\n");
    run_gcov_tool("overlap -F -h -t 0.3 test_a.gcda test_b.gcda");
    
    /* Test case 11: Invalid option to trigger default case */
    printf("\n11. Testing invalid option -z (triggers default case):\n");
    run_gcov_tool("overlap -z 2>&1 | head -20");
    
    /* Test case 12: Multiple input files with various flags */
    printf("\n12. Testing with multiple .gcda files:\n");
    run_gcov_tool("overlap -v -f test_a.gcda test_b.gcda test_c1.gcda test_c2.gcda");
    
    /* Test case 13: Using .gcno files as input */
    printf("\n13. Testing with .gcno files:\n");
    run_gcov_tool("overlap -v test_a.gcno test_b.gcno");
    
    /* Test case 14: Mixed .gcda and .gcno */
    printf("\n14. Testing with mixed .gcda and .gcno:\n");
    run_gcov_tool("overlap -o test_a.gcda test_b.gcno");
    
    printf("\n--- Cleanup ---\n");
    
    /* Cleanup generated files */
    cleanup_files("test_a");
    cleanup_files("test_b");
    cleanup_files("test_d");
    
    system("rm -f test_c test_c1.c test_c2.c test_c.h "
           "test_c1.gcda test_c1.gcno test_c2.gcda test_c2.gcno 2>/dev/null");
    
    printf("\n=== Test completed ===\n");
    printf("The following gcov-tool overlap flags were tested:\n");
    printf("  -v (verbose)          - triggers case 'v'\n");
    printf("  -f (func level)       - triggers case 'f'\n");
    printf("  -F (fullname)         - triggers case 'F'\n");
    printf("  -o (object level)     - triggers case 'o'\n");
    printf("  -h (hot only)         - triggers case 'h'\n");
    printf("  -t <threshold>        - triggers case 't'\n");
    printf("  -z (invalid)          - triggers default case\n");
    
    return 0;
}
