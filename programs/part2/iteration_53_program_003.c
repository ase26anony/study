/* test_gcov_tool_overlap.c - Test harness for gcov-tool overlap command-line parsing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

/* Simple test programs to generate coverage data */

/* Scenario A: Simple function with basic conditional branches */
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
"        if (k % 2 == 0) {\n"
"            sum += k;\n"
"        }\n"
"        k++;\n"
"    }\n"
"    \n"
"    printf(\"Sum: %d\\n\", sum);\n"
"    return 0;\n"
"}\n";

/* Scenario C: Multiple source files - File 1 */
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

/* Scenario C: Multiple source files - File 2 */
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
"void helper1(int x);\n"
"void helper2(int y);\n"
"#endif\n";

/* Scenario D: Program that may produce zero counts */
const char *test_prog_d = 
"#include <stdio.h>\n"
"int main() {\n"
"    /* This program runs but doesn't hit instrumented code paths */\n"
"    /* Actually, let's make it hit some code */\n"
"    int flag = 0;\n"
"    if (flag) {\n"
"        printf(\"This won't execute\\n\");\n"
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

/* Function to compile a test program with coverage flags */
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

/* Function to execute gcov-tool with specific arguments */
void execute_gcov_tool(const char *description, const char *args, 
                       const char *gcda_file, const char *gcno_file) {
    printf("Testing: %s\n", description);
    
    char cmd[512];
    if (gcda_file && gcno_file) {
        snprintf(cmd, sizeof(cmd), "gcov-tool overlap %s %s %s 2>&1 | head -20",
                 args, gcda_file, gcno_file);
    } else {
        snprintf(cmd, sizeof(cmd), "gcov-tool overlap %s 2>&1 | head -5", args);
    }
    
    printf("Command: %s\n", cmd);
    system(cmd);
    printf("\n");
}

/* Function to check if gcov-tool exists */
int gcov_tool_exists() {
    return system("which gcov-tool >/dev/null 2>&1") == 0 ||
           system("command -v gcov-tool >/dev/null 2>&1") == 0;
}

/* Cleanup function */
void cleanup_files(const char **files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) {
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "rm -f %s %s.gcda %s.gcno a.out",
                     files[i], files[i], files[i]);
            system(cmd);
        }
    }
    system("rm -f test_a test_b test_c test_d test_c.h test_c1.c test_c2.c 2>/dev/null");
    system("rm -f *.gcda *.gcno 2>/dev/null");
}

int main() {
    printf("=== Testing gcov-tool overlap command-line parsing ===\n\n");
    
    /* Check if gcov-tool exists */
    if (!gcov_tool_exists()) {
        printf("ERROR: gcov-tool not found in PATH.\n");
        printf("Please ensure gcov-tool is built and available.\n");
        printf("You can build it with: gcc -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
        return 1;
    }
    
    /* Create and compile test programs */
    printf("Creating test programs...\n");
    
    /* Scenario A */
    if (!write_file("test_a.c", test_prog_a)) {
        printf("Failed to create test_a.c\n");
        return 1;
    }
    if (!compile_with_coverage("test_a.c", "test_a")) {
        printf("Failed to compile test_a.c\n");
        return 1;
    }
    run_program("test_a", "");
    
    /* Scenario B */
    if (!write_file("test_b.c", test_prog_b)) {
        printf("Failed to create test_b.c\n");
        cleanup_files((const char*[]){"test_a.c"}, 1);
        return 1;
    }
    if (!compile_with_coverage("test_b.c", "test_b")) {
        printf("Failed to compile test_b.c\n");
        cleanup_files((const char*[]){"test_a.c", "test_b.c"}, 2);
        return 1;
    }
    /* Run multiple times with different arguments for richer coverage */
    run_program("test_b", "1");
    run_program("test_b", "5");
    run_program("test_b", "10");
    
    /* Scenario C - Multiple files */
    if (!write_file("test_c.h", test_header_c)) {
        printf("Failed to create test_c.h\n");
        cleanup_files((const char*[]){"test_a.c", "test_b.c"}, 2);
        return 1;
    }
    if (!write_file("test_c1.c", test_prog_c1)) {
        printf("Failed to create test_c1.c\n");
        cleanup_files((const char*[]){"test_a.c", "test_b.c", "test_c.h"}, 3);
        return 1;
    }
    if (!write_file("test_c2.c", test_prog_c2)) {
        printf("Failed to create test_c2.c\n");
        cleanup_files((const char*[]){"test_a.c", "test_b.c", "test_c.h", "test_c1.c"}, 4);
        return 1;
    }
    const char *c_sources[] = {"test_c1.c", "test_c2.c"};
    if (!compile_multiple_with_coverage(c_sources, 2, "test_c")) {
        printf("Failed to compile test_c sources\n");
        cleanup_files((const char*[]){"test_a.c", "test_b.c", "test_c.h", "test_c1.c", "test_c2.c"}, 5);
        return 1;
    }
    run_program("test_c", "");
    
    /* Scenario D */
    if (!write_file("test_d.c", test_prog_d)) {
        printf("Failed to create test_d.c\n");
        cleanup_files((const char*[]){"test_a.c", "test_b.c", "test_c.h", "test_c1.c", "test_c2.c"}, 5);
        return 1;
    }
    if (!compile_with_coverage("test_d.c", "test_d")) {
        printf("Failed to compile test_d.c\n");
        cleanup_files((const char*[]){"test_a.c", "test_b.c", "test_c.h", "test_c1.c", "test_c2.c", "test_d.c"}, 6);
        return 1;
    }
    run_program("test_d", "");
    
    printf("Test programs compiled and executed successfully.\n\n");
    
    /* Now test gcov-tool overlap with various flags */
    printf("=== Testing gcov-tool overlap command-line options ===\n\n");
    
    /* Test case 1: -v flag (verbose) - triggers case 'v' */
    execute_gcov_tool("Verbose mode (-v)", "-v", "test_a.gcda", "test_a.gcno");
    
    /* Test case 2: -f flag (function level) - triggers case 'f' */
    execute_gcov_tool("Function level overlap (-f)", "-f", "test_b.gcda", "test_b.gcno");
    
    /* Test case 3: -F flag (fullname) - triggers case 'F' */
    execute_gcov_tool("Fullname mode (-F)", "-F", "test_c.gcda", "test_c.gcno");
    
    /* Test case 4: -o flag (object level) - triggers case 'o' */
    execute_gcov_tool("Object level overlap (-o)", "-o", "test_d.gcda", "test_d.gcno");
    
    /* Test case 5: -h flag (hot only) - triggers case 'h' */
    execute_gcov_tool("Hot only mode (-h)", "-h", "test_a.gcda", "test_a.gcno");
    
    /* Test case 6: -t flag with threshold - triggers case 't' */
    execute_gcov_tool("Hot threshold 0.5 (-t 0.5)", "-t 0.5", "test_b.gcda", "test_b.gcno");
    execute_gcov_tool("Hot threshold 0.75 (-t 0.75)", "-t 0.75", "test_c.gcda", "test_c.gcno");
    execute_gcov_tool("Hot threshold 0.1 (-t 0.1)", "-t 0.1", "test_d.gcda", "test_d.gcno");
    
    /* Test case 7: Combination of flags */
    execute_gcov_tool("Combination -v -f -o", "-v -f -o", "test_a.gcda", "test_a.gcno");
    execute_gcov_tool("Combination -F -h -t 0.3", "-F -h -t 0.3", "test_b.gcda", "test_b.gcno");
    
    /* Test case 8: Invalid option - triggers default case and overlap_usage() */
    printf("Testing: Invalid option (-z) to trigger default case\n");
    printf("Command: gcov-tool overlap -z 2>&1 | head -10\n");
    system("gcov-tool overlap -z 2>&1 | head -10");
    printf("\n");
    
    /* Test case 9: Multiple input files with flags */
    printf("Testing: Multiple input files with verbose flag\n");
    printf("Command: gcov-tool overlap -v test_a.gcda test_b.gcda test_a.gcno test_b.gcno 2>&1 | head -10\n");
    system("gcov-tool overlap -v test_a.gcda test_b.gcda test_a.gcno test_b.gcno 2>&1 | head -10");
    printf("\n");
    
    /* Test case 10: No arguments (should show usage) */
    printf("Testing: No arguments\n");
    printf("Command: gcov-tool overlap 2>&1 | head -5\n");
    system("gcov-tool overlap 2>&1 | head -5");
    printf("\n");
    
    /* Cleanup */
    printf("Cleaning up temporary files...\n");
    cleanup_files((const char*[]){"test_a.c", "test_b.c", "test_c.h", 
                                  "test_c1.c", "test_c2.c", "test_d.c"}, 6);
    
    printf("\n=== Test completed successfully ===\n");
    printf("All gcov-tool overlap command-line options have been exercised.\n");
    printf("The following switch cases in gcov-tool.cc should now be covered:\n");
    printf("  case 'v': (verbose)\n");
    printf("  case 'f': (overlap_func_level)\n");
    printf("  case 'F': (overlap_use_fullname)\n");
    printf("  case 'o': (overlap_obj_level)\n");
    printf("  case 'h': (overlap_hot_only)\n");
    printf("  case 't': (overlap_hot_threshold)\n");
    printf("  default: (overlap_usage)\n");
    
    return 0;
}
