/* test_gcov_tool_overlap.c - Test harness for gcov-tool overlap command parsing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* Simple test programs to generate varied coverage data */
const char *test_prog_a = 
"#include <stdio.h>\n"
"void func1(int x) {\n"
"    if (x > 0) {\n"
"        printf(\"Positive\\n\");\n"
"    } else {\n"
"        printf(\"Non-positive\\n\");\n"
"    }\n"
"}\n"
"void func2() {\n"
"    for (int i = 0; i < 3; i++) {\n"
"        printf(\"Loop %d\\n\", i);\n"
"    }\n"
"}\n"
"int main() {\n"
"    func1(1);\n"
"    func1(-1);\n"
"    func2();\n"
"    return 0;\n"
"}\n";

const char *test_prog_b = 
"#include <stdio.h>\n"
"int main(int argc, char **argv) {\n"
"    int iterations = (argc > 1) ? atoi(argv[1]) : 5;\n"
"    int sum = 0;\n"
"    /* Nested loops for rich execution counts */\n"
"    for (int i = 0; i < iterations; i++) {\n"
"        for (int j = 0; j < i; j++) {\n"
"            sum += i * j;\n"
"        }\n"
"    }\n"
"    printf(\"Sum: %d\\n\", sum);\n"
"    /* Conditional with multiple branches */\n"
"    if (sum > 100) {\n"
"        printf(\"Large sum\\n\");\n"
"    } else if (sum > 50) {\n"
"        printf(\"Medium sum\\n\");\n"
"    } else {\n"
"        printf(\"Small sum\\n\");\n"
"    }\n"
"    return 0;\n"
"}\n";

const char *test_prog_c1 = 
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

const char *test_prog_c2 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"void helper2() {\n"
"    printf(\"Helper2 called\\n\");\n"
"}\n";

const char *test_header = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"void helper2();\n"
"#endif\n";

const char *test_prog_d = 
"#include <stdio.h>\n"
"int main() {\n"
"    /* This program runs but takes the path with no instrumented code */\n"
"    /* Or we can simply not run it to get zero-count .gcda files */\n"
"    return 0;\n"
"}\n";

/* Execute a shell command and check return status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Command failed: %s\n", cmd);
    }
    return status;
}

/* Write a string to a file */
void write_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    fputs(content, f);
    fclose(f);
}

/* Check if gcov-tool exists in PATH or current directory */
int gcov_tool_exists() {
    /* First check current directory */
    if (access("./gcov-tool", X_OK) == 0) {
        return 1;
    }
    /* Check in PATH */
    if (system("which gcov-tool > /dev/null 2>&1") == 0) {
        return 1;
    }
    return 0;
}

/* Get gcov-tool command path */
const char *get_gcov_tool_cmd() {
    static char cmd[256];
    if (access("./gcov-tool", X_OK) == 0) {
        return "./gcov-tool";
    }
    return "gcov-tool";
}

int main(int argc, char **argv) {
    printf("=== Testing gcov-tool overlap command parsing ===\n");
    
    /* Check if gcov-tool is available */
    if (!gcov_tool_exists()) {
        fprintf(stderr, "Error: gcov-tool not found in current directory or PATH\n");
        fprintf(stderr, "Please build gcov-tool with coverage flags first:\n");
        fprintf(stderr, "  g++ -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
        return 1;
    }
    
    const char *gcov_tool = get_gcov_tool_cmd();
    printf("Using gcov-tool: %s\n", gcov_tool);
    
    /* Create test directory */
    execute_command("mkdir -p test_coverage_data");
    chdir("test_coverage_data");
    
    /* =========================================== */
    /* Scenario A: Simple function with branches   */
    /* =========================================== */
    printf("\n--- Scenario A: Simple function ---\n");
    write_file("test_a.c", test_prog_a);
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_a.c -o test_a");
    execute_command("./test_a");
    
    /* =========================================== */
    /* Scenario B: Loop-heavy program              */
    /* =========================================== */
    printf("\n--- Scenario B: Loop-heavy program ---\n");
    write_file("test_b.c", test_prog_b);
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_b.c -o test_b");
    /* Run multiple times with different inputs for varied counts */
    execute_command("./test_b 3");
    execute_command("./test_b 7");
    execute_command("./test_b 10");
    
    /* =========================================== */
    /* Scenario C: Multiple source files           */
    /* =========================================== */
    printf("\n--- Scenario C: Multiple source files ---\n");
    write_file("test_c.h", test_header);
    write_file("test_c1.c", test_prog_c1);
    write_file("test_c2.c", test_prog_c2);
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_c1.c test_c2.c -o test_c");
    execute_command("./test_c");
    
    /* =========================================== */
    /* Scenario D: Zero-count coverage             */
    /* =========================================== */
    printf("\n--- Scenario D: Zero-count coverage ---\n");
    write_file("test_d.c", test_prog_d);
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_d.c -o test_d");
    /* Compile but don't run to get zero-count .gcda, or run minimal path */
    execute_command("./test_d");
    
    /* =========================================== */
    /* Now invoke gcov-tool overlap with various flags */
    /* Targeting the uncovered switch cases        */
    /* =========================================== */
    printf("\n=== Invoking gcov-tool overlap with various flags ===\n");
    
    /* Case 'v': verbose flag */
    printf("\n1. Testing -v flag (verbose mode)...\n");
    execute_command("cp test_a.gcda test_a.gcda.bak 2>/dev/null; cp test_a.gcno test_a.gcno.bak 2>/dev/null");
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "%s overlap -v test_a.gcda test_a.gcda.bak", gcov_tool);
    execute_command(cmd);
    
    /* Case 'f': function-level overlap */
    printf("\n2. Testing -f flag (function-level)...\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -f test_a.gcda test_b.gcda", gcov_tool);
    execute_command(cmd);
    
    /* Case 'F': use full filenames */
    printf("\n3. Testing -F flag (full filenames)...\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -F test_a.gcda test_b.gcda", gcov_tool);
    execute_command(cmd);
    
    /* Case 'o': object-level overlap */
    printf("\n4. Testing -o flag (object-level)...\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -o test_a.gcda test_b.gcda", gcov_tool);
    execute_command(cmd);
    
    /* Case 'h': hot-only */
    printf("\n5. Testing -h flag (hot-only)...\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -h test_a.gcda test_b.gcda", gcov_tool);
    execute_command(cmd);
    
    /* Case 't': hot threshold with argument */
    printf("\n6. Testing -t flag with threshold 0.5...\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.5 test_a.gcda test_b.gcda", gcov_tool);
    execute_command(cmd);
    
    printf("\n7. Testing -t flag with threshold 0.75...\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.75 test_a.gcda test_b.gcda", gcov_tool);
    execute_command(cmd);
    
    printf("\n8. Testing -t flag with threshold 0.0...\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.0 test_a.gcda test_b.gcda", gcov_tool);
    execute_command(cmd);
    
    printf("\n9. Testing -t flag with threshold 1.0...\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t 1.0 test_a.gcda test_b.gcda", gcov_tool);
    execute_command(cmd);
    
    /* Multiple flag combinations */
    printf("\n10. Testing combination -v -f -o...\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f -o test_a.gcda test_b.gcda", gcov_tool);
    execute_command(cmd);
    
    printf("\n11. Testing combination -F -h -t 0.3...\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -F -h -t 0.3 test_a.gcda test_b.gcda", gcov_tool);
    execute_command(cmd);
    
    printf("\n12. Testing with multiple .gcda files...\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v test_a.gcda test_b.gcda test_c1.gcda", gcov_tool);
    execute_command(cmd);
    
    /* Test with .gcno files as well (overlap can use both) */
    printf("\n13. Testing with .gcno files...\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v test_a.gcno test_b.gcno", gcov_tool);
    execute_command(cmd);
    
    printf("\n14. Testing mixed .gcda and .gcno files...\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -f -o test_a.gcda test_b.gcno", gcov_tool);
    execute_command(cmd);
    
    /* Default case: invalid option to trigger overlap_usage() */
    printf("\n15. Testing invalid option -z (should trigger default case and usage)...\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -z 2>&1 | head -20", gcov_tool);
    execute_command(cmd);
    
    /* Another invalid option combination */
    printf("\n16. Testing invalid option -x (should trigger default case)...\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -x test_a.gcda 2>&1 | head -20", gcov_tool);
    execute_command(cmd);
    
    /* Test with no arguments (should also show usage) */
    printf("\n17. Testing with no arguments (should show usage)...\n");
    snprintf(cmd, sizeof(cmd), "%s overlap 2>&1 | head -20", gcov_tool);
    execute_command(cmd);
    
    /* Test edge case: -t without argument (should trigger error/usage) */
    printf("\n18. Testing -t without argument (should trigger error)...\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t 2>&1 | head -20", gcov_tool);
    execute_command(cmd);
    
    /* =========================================== */
    /* Cleanup                                     */
    /* =========================================== */
    printf("\n=== Cleaning up test files ===\n");
    chdir("..");
    execute_command("rm -rf test_coverage_data");
    
    printf("\n=== Test completed ===\n");
    printf("All gcov-tool overlap invocations executed.\n");
    printf("Check gcov-tool's own .gcda file for coverage of the target switch statement.\n");
    
    return 0;
}
