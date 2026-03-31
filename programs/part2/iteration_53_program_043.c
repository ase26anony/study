/* test_gcov_tool_overlap.c - Test harness for gcov-tool overlap command parsing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Simple C test programs to generate coverage data */

/* Scenario A: Simple function with conditionals */
const char *test_prog_a = 
"#include <stdio.h>\n"
"void func1(int x) {\n"
"    if (x > 0) {\n"
"        printf(\"Positive\\n\");\n"
"    } else {\n"
"        printf(\"Non-positive\\n\");\n"
"    }\n"
"}\n"
"void func2(int y) {\n"
"    switch(y) {\n"
"        case 1: printf(\"One\\n\"); break;\n"
"        case 2: printf(\"Two\\n\"); break;\n"
"        default: printf(\"Other\\n\");\n"
"    }\n"
"}\n"
"int main() {\n"
"    func1(5);\n"
"    func1(-3);\n"
"    func2(1);\n"
"    func2(3);\n"
"    return 0;\n"
"}\n";

/* Scenario B: Loop-heavy program */
const char *test_prog_b = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"int main(int argc, char **argv) {\n"
"    int iterations = argc > 1 ? atoi(argv[1]) : 10;\n"
"    int sum = 0;\n"
"    \n"
"    // Outer loop\n"
"    for (int i = 0; i < iterations; i++) {\n"
"        // Inner loop\n"
"        for (int j = 0; j < i; j++) {\n"
"            sum += j;\n"
"        }\n"
"        \n"
"        // Conditional in loop\n"
"        if (i % 2 == 0) {\n"
"            sum += i * 2;\n"
"        } else {\n"
"            sum += i;\n"
"        }\n"
"    }\n"
"    \n"
"    printf(\"Sum: %d\\n\", sum);\n"
"    return 0;\n"
"}\n";

/* Scenario C: Multiple source files - part 1 */
const char *test_prog_c1 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"void helper1(int x) {\n"
"    printf(\"Helper1: %d\\n\", x);\n"
"}\n"
"int main() {\n"
"    helper1(10);\n"
"    helper2(20);\n"
"    return 0;\n"
"}\n";

/* Scenario C: Multiple source files - part 2 */
const char *test_prog_c2 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"void helper2(int y) {\n"
"    if (y > 15) {\n"
"        printf(\"Helper2 large: %d\\n\", y);\n"
"    } else {\n"
"        printf(\"Helper2 small: %d\\n\", y);\n"
"    }\n"
"}\n";

/* Scenario C: Header file */
const char *test_header_c = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"void helper1(int x);\n"
"void helper2(int y);\n"
"#endif\n";

/* Scenario D: Program that may produce zero counts */
const char *test_prog_d = 
"#include <stdio.h>\n"
"int main(int argc, char **argv) {\n"
"    // This path is only taken if specific argument is provided\n"
"    if (argc > 1 && argv[1][0] == 'X') {\n"
"        printf(\"Executed instrumented path\\n\");\n"
"        for (int i = 0; i < 5; i++) {\n"
"            printf(\"Loop iteration %d\\n\", i);\n"
"        }\n"
"    }\n"
"    // Otherwise, no instrumented code runs\n"
"    return 0;\n"
"}\n";

/* Utility functions */
int compile_program(const char *source, const char *output, const char *extra_flags) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s 2>/dev/null",
             extra_flags ? extra_flags : "", output);
    
    /* Write source to temp file */
    char src_file[256];
    snprintf(src_file, sizeof(src_file), "/tmp/%s.c", output);
    FILE *f = fopen(src_file, "w");
    if (!f) return 0;
    fputs(source, f);
    fclose(f);
    
    /* Compile */
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage %s /tmp/%s.c -o %s 2>/dev/null",
             extra_flags ? extra_flags : "", output, output);
    
    int ret = system(cmd);
    return (ret == 0);
}

int run_program(const char *program, const char *args) {
    char cmd[1024];
    if (args) {
        snprintf(cmd, sizeof(cmd), "./%s %s > /dev/null 2>&1", program, args);
    } else {
        snprintf(cmd, sizeof(cmd), "./%s > /dev/null 2>&1", program);
    }
    return system(cmd) == 0;
}

int run_gcov_tool(const char *args) {
    char cmd[1024];
    /* Try to find gcov-tool in common locations */
    const char *gcov_tool_paths[] = {
        "./gcov-tool",
        "../gcov-tool",
        "gcov-tool",
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
        fprintf(stderr, "ERROR: gcov-tool not found in PATH or common locations\n");
        return 0;
    }
    
    snprintf(cmd, sizeof(cmd), "%s %s", gcov_tool, args);
    printf("Executing: %s\n", cmd);
    int ret = system(cmd);
    return 1; /* Return success even if gcov-tool fails (we want to trigger parsing) */
}

void cleanup_files(const char *base_name) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "rm -f %s %s.gcda %s.gcno %s.c.gcov 2>/dev/null", 
             base_name, base_name, base_name, base_name);
    system(cmd);
}

int main() {
    printf("=== Test Harness for gcov-tool overlap command parsing ===\n\n");
    
    /* Step 1: Compile test programs with coverage */
    printf("1. Compiling test programs with coverage flags...\n");
    
    if (!compile_program(test_prog_a, "test_a", NULL)) {
        fprintf(stderr, "Failed to compile test_a\n");
        return 1;
    }
    
    if (!compile_program(test_prog_b, "test_b", NULL)) {
        fprintf(stderr, "Failed to compile test_b\n");
        cleanup_files("test_a");
        return 1;
    }
    
    /* For multi-file scenario */
    FILE *f1 = fopen("/tmp/test_c1.c", "w");
    FILE *f2 = fopen("/tmp/test_c2.c", "w");
    FILE *fh = fopen("/tmp/test_c.h", "w");
    if (f1 && f2 && fh) {
        fputs(test_prog_c1, f1);
        fputs(test_prog_c2, f2);
        fputs(test_header_c, fh);
        fclose(f1);
        fclose(f2);
        fclose(fh);
        
        if (system("gcc -O0 -fprofile-arcs -ftest-coverage /tmp/test_c1.c /tmp/test_c2.c -I/tmp -o test_c 2>/dev/null") != 0) {
            fprintf(stderr, "Failed to compile test_c\n");
        }
    }
    
    if (!compile_program(test_prog_d, "test_d", NULL)) {
        fprintf(stderr, "Failed to compile test_d\n");
    }
    
    printf("   Done.\n\n");
    
    /* Step 2: Run programs to generate .gcda files */
    printf("2. Running test programs to generate coverage data...\n");
    
    run_program("test_a", NULL);
    run_program("test_b", "5");      /* Run with 5 iterations */
    run_program("test_b", "20");     /* Run again with 20 iterations */
    
    if (access("test_c", X_OK) == 0) {
        run_program("test_c", NULL);
    }
    
    run_program("test_d", NULL);     /* Run without 'X' arg - should produce zero counts */
    
    printf("   Done.\n\n");
    
    /* Step 3: Invoke gcov-tool overlap with various flag combinations */
    printf("3. Testing gcov-tool overlap with different flag combinations...\n\n");
    
    /* Test case 1: -v flag (verbose) */
    printf("--- Test 1: -v flag (verbose) ---\n");
    run_gcov_tool("overlap -v test_a.gcda test_a.gcno");
    printf("\n");
    
    /* Test case 2: -f flag (function level) */
    printf("--- Test 2: -f flag (function level) ---\n");
    run_gcov_tool("overlap -f test_a.gcda test_b.gcda");
    printf("\n");
    
    /* Test case 3: -F flag (use fullname) */
    printf("--- Test 3: -F flag (use fullname) ---\n");
    run_gcov_tool("overlap -F test_a.gcda test_a.gcno");
    printf("\n");
    
    /* Test case 4: -o flag (object level) */
    printf("--- Test 4: -o flag (object level) ---\n");
    run_gcov_tool("overlap -o test_a.gcda test_b.gcda");
    printf("\n");
    
    /* Test case 5: -h flag (hot only) */
    printf("--- Test 5: -h flag (hot only) ---\n");
    run_gcov_tool("overlap -h test_a.gcda test_b.gcda");
    printf("\n");
    
    /* Test case 6: -t flag with threshold value */
    printf("--- Test 6: -t flag with threshold value ---\n");
    run_gcov_tool("overlap -t 0.5 test_a.gcda test_b.gcda");
    printf("\n");
    
    /* Test case 7: -t flag with different threshold */
    printf("--- Test 7: -t flag with different threshold ---\n");
    run_gcov_tool("overlap -t 0.75 test_b.gcda test_a.gcda");
    printf("\n");
    
    /* Test case 8: -t flag with extreme threshold */
    printf("--- Test 8: -t flag with extreme threshold ---\n");
    run_gcov_tool("overlap -t 1.5 test_a.gcda test_a.gcno");
    printf("\n");
    
    /* Test case 9: Combination of flags */
    printf("--- Test 9: Combination of flags ---\n");
    run_gcov_tool("overlap -v -f -o test_a.gcda test_b.gcda");
    printf("\n");
    
    /* Test case 10: Another combination */
    printf("--- Test 10: Another combination ---\n");
    run_gcov_tool("overlap -F -h -t 0.3 test_b.gcda test_a.gcda");
    printf("\n");
    
    /* Test case 11: With zero-count .gcda file */
    printf("--- Test 11: With zero-count .gcda file ---\n");
    run_gcov_tool("overlap -t 0.1 test_d.gcda test_a.gcda");
    printf("\n");
    
    /* Test case 12: Multiple input files */
    printf("--- Test 12: Multiple input files ---\n");
    if (access("test_c.gcda", F_OK) == 0) {
        run_gcov_tool("overlap -v test_a.gcda test_b.gcda test_c.gcda");
    }
    printf("\n");
    
    /* Test case 13: Invalid option to trigger default case and overlap_usage() */
    printf("--- Test 13: Invalid option (trigger default case) ---\n");
    run_gcov_tool("overlap -z 2>&1 | head -20");  /* Capture first 20 lines of usage */
    printf("\n");
    
    /* Test case 14: Another invalid combination */
    printf("--- Test 14: Another invalid option ---\n");
    run_gcov_tool("overlap -x -y 2>&1 | head -10");
    printf("\n");
    
    /* Test case 15: Valid flag with invalid argument */
    printf("--- Test 15: -t flag with non-numeric argument ---\n");
    run_gcov_tool("overlap -t invalid test_a.gcda test_b.gcda 2>&1 | head -5");
    printf("\n");
    
    printf("All gcov-tool invocations completed.\n\n");
    
    /* Step 4: Cleanup */
    printf("4. Cleaning up temporary files...\n");
    cleanup_files("test_a");
    cleanup_files("test_b");
    cleanup_files("test_c");
    cleanup_files("test_d");
    
    /* Clean multi-file temp sources */
    system("rm -f /tmp/test_c1.c /tmp/test_c2.c /tmp/test_c.h 2>/dev/null");
    
    printf("   Done.\n");
    printf("\n=== Test harness finished ===\n");
    
    return 0;
}
