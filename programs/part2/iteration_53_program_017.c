/* test_gcov_tool_overlap.c - Test harness for gcov-tool overlap command-line parsing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

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
"void func2(int y) {\n"
"    switch(y) {\n"
"        case 1: printf(\"One\\n\"); break;\n"
"        case 2: printf(\"Two\\n\"); break;\n"
"        default: printf(\"Other\\n\"); break;\n"
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
"    int iterations = 1;\n"
"    if (argc > 1) iterations = atoi(argv[1]);\n"
"    \n"
"    int sum = 0;\n"
"    for (int i = 0; i < iterations; i++) {\n"
"        for (int j = 0; j < 10; j++) {\n"
"            sum += i * j;\n"
"        }\n"
"    }\n"
"    \n"
"    int k = 0;\n"
"    while (k < 5) {\n"
"        if (sum % 2 == 0) {\n"
"            sum /= 2;\n"
"        }\n"
"        k++;\n"
"    }\n"
"    \n"
"    printf(\"Result: %d\\n\", sum);\n"
"    return 0;\n"
"}\n";

/* Scenario C: Multiple source files - part 1 */
const char *test_prog_c1 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"void helper1(int x) {\n"
"    printf(\"Helper1: %d\\n\", x * 2);\n"
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
"        printf(\"Helper2: Large %d\\n\", y);\n"
"    } else {\n"
"        printf(\"Helper2: Small %d\\n\", y);\n"
"    }\n"
"}\n";

/* Scenario C: Header file */
const char *test_header_c = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"void helper2(int y);\n"
"#endif\n";

/* Scenario D: Program that may produce zero counts */
const char *test_prog_d = 
"#include <stdio.h>\n"
"int main() {\n"
"    /* This program runs but doesn't hit instrumented code paths */\n"
"    /* if we compile with coverage but don't execute the main logic */\n"
"    int flag = 0;\n"
"    if (flag) {\n"
"        printf(\"This won't execute\\n\");\n"
"        int x = 1;\n"
"        while (x < 10) {\n"
"            x++;\n"
"        }\n"
"    }\n"
"    return 0;\n"
"}\n";

/* Function to write a string to a file */
int write_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    fputs(content, f);
    fclose(f);
    return 1;
}

/* Function to compile a test program with coverage */
int compile_with_coverage(const char *source_file, const char *output_name) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage -o %s %s 2>/dev/null",
             output_name, source_file);
    return system(cmd) == 0;
}

/* Function to compile multiple source files with coverage */
int compile_multiple_with_coverage(const char **sources, int count, 
                                   const char *output_name) {
    char cmd[1024] = "gcc -O0 -fprofile-arcs -ftest-coverage -o ";
    strcat(cmd, output_name);
    strcat(cmd, " ");
    
    for (int i = 0; i < count; i++) {
        strcat(cmd, sources[i]);
        strcat(cmd, " ");
    }
    strcat(cmd, " 2>/dev/null");
    
    return system(cmd) == 0;
}

/* Function to run a program and generate .gcda files */
int run_program(const char *program, const char *args) {
    char cmd[256];
    if (args && args[0]) {
        snprintf(cmd, sizeof(cmd), "./%s %s >/dev/null 2>&1", program, args);
    } else {
        snprintf(cmd, sizeof(cmd), "./%s >/dev/null 2>&1", program);
    }
    return system(cmd) == 0;
}

/* Function to execute gcov-tool with given arguments */
int run_gcov_tool_overlap(const char *args, const char *files) {
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
        fprintf(stderr, "Error: gcov-tool not found in PATH or common locations\n");
        return 0;
    }
    
    snprintf(cmd, sizeof(cmd), "%s overlap %s %s 2>&1", gcov_tool, args, files);
    printf("Executing: %s\n", cmd);
    
    int result = system(cmd);
    /* We don't care about the exit status for coverage purposes */
    return 1;
}

/* Clean up generated files */
void cleanup_files(const char **files, int count) {
    for (int i = 0; i < count; i++) {
        unlink(files[i]);
    }
}

int main() {
    printf("=== Testing gcov-tool overlap command-line parsing ===\n\n");
    
    /* Create test source files */
    if (!write_file("test_a.c", test_prog_a)) return 1;
    if (!write_file("test_b.c", test_prog_b)) return 1;
    if (!write_file("test_c1.c", test_prog_c1)) return 1;
    if (!write_file("test_c2.c", test_prog_c2)) return 1;
    if (!write_file("test_c.h", test_header_c)) return 1;
    if (!write_file("test_d.c", test_prog_d)) return 1;
    
    /* Compile test programs with coverage */
    printf("Compiling test programs with coverage...\n");
    if (!compile_with_coverage("test_a.c", "test_a")) {
        fprintf(stderr, "Failed to compile test_a.c\n");
        return 1;
    }
    if (!compile_with_coverage("test_b.c", "test_b")) {
        fprintf(stderr, "Failed to compile test_b.c\n");
        return 1;
    }
    
    const char *multi_sources[] = {"test_c1.c", "test_c2.c"};
    if (!compile_multiple_with_coverage(multi_sources, 2, "test_c")) {
        fprintf(stderr, "Failed to compile multi-file test\n");
        return 1;
    }
    
    if (!compile_with_coverage("test_d.c", "test_d")) {
        fprintf(stderr, "Failed to compile test_d.c\n");
        return 1;
    }
    
    /* Run programs to generate .gcda files */
    printf("Running test programs to generate coverage data...\n");
    run_program("test_a", "");
    run_program("test_b", "5");  /* Run with 5 iterations */
    run_program("test_b", "1");  /* Run again with different input */
    run_program("test_c", "");
    run_program("test_d", "");   /* This produces mostly zero counts */
    
    /* Test gcov-tool overlap with various flag combinations */
    printf("\n=== Testing gcov-tool overlap with different flags ===\n");
    
    /* Test 1: -v flag (verbose) - triggers case 'v' */
    printf("\n1. Testing -v flag (verbose)...\n");
    run_gcov_tool_overlap("-v", "test_a.gcda test_a.gcno");
    
    /* Test 2: -f flag (function level) - triggers case 'f' */
    printf("\n2. Testing -f flag (function level)...\n");
    run_gcov_tool_overlap("-f", "test_b.gcda test_b.gcno");
    
    /* Test 3: -F flag (fullname) - triggers case 'F' */
    printf("\n3. Testing -F flag (fullname)...\n");
    run_gcov_tool_overlap("-F", "test_a.gcda test_a.gcno test_b.gcda test_b.gcno");
    
    /* Test 4: -o flag (object level) - triggers case 'o' */
    printf("\n4. Testing -o flag (object level)...\n");
    run_gcov_tool_overlap("-o", "test_c1.gcda test_c1.gcno test_c2.gcda test_c2.gcno");
    
    /* Test 5: -h flag (hot only) - triggers case 'h' */
    printf("\n5. Testing -h flag (hot only)...\n");
    run_gcov_tool_overlap("-h", "test_a.gcda test_a.gcno test_b.gcda test_b.gcno");
    
    /* Test 6: -t flag with threshold - triggers case 't' */
    printf("\n6. Testing -t flag with threshold 0.5...\n");
    run_gcov_tool_overlap("-t 0.5", "test_a.gcda test_a.gcno");
    
    /* Test 7: -t flag with different threshold - triggers case 't' again */
    printf("\n7. Testing -t flag with threshold 0.75...\n");
    run_gcov_tool_overlap("-t 0.75", "test_b.gcda test_b.gcno");
    
    /* Test 8: -t flag with threshold 0.0 - edge case */
    printf("\n8. Testing -t flag with threshold 0.0...\n");
    run_gcov_tool_overlap("-t 0.0", "test_d.gcda test_d.gcno");
    
    /* Test 9: Combination of flags */
    printf("\n9. Testing combination -v -f -o...\n");
    run_gcov_tool_overlap("-v -f -o", "test_a.gcda test_a.gcno test_b.gcda test_b.gcno");
    
    /* Test 10: Another combination */
    printf("\n10. Testing combination -F -h -t 0.3...\n");
    run_gcov_tool_overlap("-F -h -t 0.3", "test_a.gcda test_a.gcno test_c1.gcda test_c1.gcno");
    
    /* Test 11: Invalid option to trigger default case and overlap_usage() */
    printf("\n11. Testing invalid option -z (should trigger default case)...\n");
    run_gcov_tool_overlap("-z", "test_a.gcda test_a.gcno");
    
    /* Test 12: Another invalid option combination */
    printf("\n12. Testing invalid option -x (should trigger default case)...\n");
    run_gcov_tool_overlap("-x -y", "test_b.gcda test_b.gcno");
    
    /* Test 13: Valid flag with invalid extra option */
    printf("\n13. Testing -v with invalid extra option...\n");
    run_gcov_tool_overlap("-v -invalid", "test_a.gcda test_a.gcno");
    
    /* Test 14: Empty arguments (just overlap command) */
    printf("\n14. Testing with no flags...\n");
    run_gcov_tool_overlap("", "test_a.gcda test_a.gcno");
    
    /* Test 15: Multiple files with various flags */
    printf("\n15. Testing with multiple data files and all flags...\n");
    run_gcov_tool_overlap("-v -f -F -o -h -t 0.25", 
                         "test_a.gcda test_a.gcno test_b.gcda test_b.gcno test_c1.gcda test_c1.gcno test_d.gcda test_d.gcno");
    
    /* Cleanup */
    printf("\n=== Cleaning up generated files ===\n");
    
    /* List of files to clean up */
    const char *files_to_clean[] = {
        "test_a.c", "test_b.c", "test_c1.c", "test_c2.c", "test_c.h", "test_d.c",
        "test_a", "test_b", "test_c", "test_d",
        "test_a.gcda", "test_a.gcno", "test_b.gcda", "test_b.gcno",
        "test_c1.gcda", "test_c1.gcno", "test_c2.gcda", "test_c2.gcno",
        "test_d.gcda", "test_d.gcno",
        NULL
    };
    
    for (int i = 0; files_to_clean[i]; i++) {
        unlink(files_to_clean[i]);
    }
    
    /* Also clean up any gcov intermediate files */
    system("rm -f *.gcov 2>/dev/null");
    
    printf("\n=== Test completed ===\n");
    printf("The gcov-tool overlap command has been invoked with various flag combinations.\n");
    printf("Each flag combination should have triggered the corresponding case in the switch statement.\n");
    
    return 0;
}
