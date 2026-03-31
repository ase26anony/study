/* test_gcov_tool_overlap.c
 * Test harness to trigger uncovered lines in gcov-tool.cc (lines 534-554)
 * Compile and run: gcc -o test_gcov_tool test_gcov_tool_overlap.c && ./test_gcov_tool
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

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
"    int i, j, sum = 0;\n"
"    int limit = (argc > 1) ? atoi(argv[1]) : 10;\n"
"    \n"
"    for (i = 0; i < limit; i++) {\n"
"        for (j = 0; j < i; j++) {\n"
"            sum += i * j;\n"
"        }\n"
"    }\n"
"    \n"
"    printf(\"Sum: %d\\n\", sum);\n"
"    return 0;\n"
"}\n";

/* Scenario C: Multiple source files - part 1 */
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

/* Scenario C: Multiple source files - part 2 */
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

/* Scenario D: Program with zero coverage */
const char *test_d = 
"#include <stdio.h>\n"
"int main() {\n"
"    /* This program is compiled but never executed */\n"
"    return 0;\n"
"}\n";

/* Helper function to write a string to a file */
void write_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    fputs(content, f);
    fclose(f);
}

/* Execute a command and check return status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        printf("Command failed with status %d: %s\n", status, cmd);
    }
    return status;
}

/* Check if gcov-tool exists */
int check_gcov_tool() {
    struct stat st;
    if (stat("gcov-tool", &st) == 0) {
        return 1;  /* Found in current directory */
    }
    if (system("which gcov-tool > /dev/null 2>&1") == 0) {
        return 2;  /* Found in PATH */
    }
    return 0;  /* Not found */
}

/* Clean up generated files */
void cleanup_files(const char **files, int count) {
    for (int i = 0; i < count; i++) {
        remove(files[i]);
    }
}

int main() {
    printf("=== Testing gcov-tool overlap command-line parsing ===\n\n");
    
    /* Check if gcov-tool is available */
    int tool_status = check_gcov_tool();
    const char *gcov_tool_cmd;
    
    if (tool_status == 1) {
        gcov_tool_cmd = "./gcov-tool";
    } else if (tool_status == 2) {
        gcov_tool_cmd = "gcov-tool";
    } else {
        printf("ERROR: gcov-tool not found in current directory or PATH\n");
        printf("Please build gcov-tool with coverage flags first:\n");
        printf("  g++ -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
        return 1;
    }
    
    /* Create test programs */
    printf("Creating test programs...\n");
    write_file("test_a.c", test_a);
    write_file("test_b.c", test_b);
    write_file("test_c1.c", test_c1);
    write_file("test_c2.c", test_c2);
    write_file("test_c.h", test_c_header);
    write_file("test_d.c", test_d);
    
    /* Compile test programs with coverage */
    printf("\nCompiling test programs with coverage...\n");
    execute_command("gcc -fprofile-arcs -ftest-coverage -O0 test_a.c -o test_a");
    execute_command("gcc -fprofile-arcs -ftest-coverage -O0 test_b.c -o test_b");
    execute_command("gcc -fprofile-arcs -ftest-coverage -O0 test_c1.c test_c2.c -o test_c");
    execute_command("gcc -fprofile-arcs -ftest-coverage -O0 test_d.c -o test_d");
    
    /* Run test programs to generate .gcda files */
    printf("\nRunning test programs to generate coverage data...\n");
    execute_command("./test_a");
    execute_command("./test_b 5");
    execute_command("./test_b 3");  /* Run twice with different inputs */
    execute_command("./test_c");
    /* Note: test_d is NOT run to generate zero-coverage data */
    
    /* List of files to clean up later */
    const char *cleanup_list[] = {
        "test_a.c", "test_a", "test_a.gcda", "test_a.gcno",
        "test_b.c", "test_b", "test_b.gcda", "test_b.gcno",
        "test_c1.c", "test_c2.c", "test_c.h", "test_c",
        "test_c1.gcda", "test_c1.gcno", "test_c2.gcda", "test_c2.gcno",
        "test_d.c", "test_d", "test_d.gcda", "test_d.gcno"
    };
    int cleanup_count = sizeof(cleanup_list) / sizeof(cleanup_list[0]);
    
    /* Now invoke gcov-tool overlap with various flag combinations */
    printf("\n=== Invoking gcov-tool overlap with different flags ===\n\n");
    
    /* Test 1: -v flag (verbose) - triggers case 'v' */
    printf("Test 1: Testing -v flag (verbose)\n");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s overlap -v test_a.gcda test_a.gcno", gcov_tool_cmd);
    execute_command(cmd);
    
    /* Test 2: -f flag (function level) - triggers case 'f' */
    printf("\nTest 2: Testing -f flag (function level)\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -f test_a.gcda test_b.gcda", gcov_tool_cmd);
    execute_command(cmd);
    
    /* Test 3: -F flag (fullname) - triggers case 'F' */
    printf("\nTest 3: Testing -F flag (fullname)\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -F test_a.gcda test_a.gcno", gcov_tool_cmd);
    execute_command(cmd);
    
    /* Test 4: -o flag (object level) - triggers case 'o' */
    printf("\nTest 4: Testing -o flag (object level)\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -o test_a.gcda test_b.gcda", gcov_tool_cmd);
    execute_command(cmd);
    
    /* Test 5: -h flag (hot only) - triggers case 'h' */
    printf("\nTest 5: Testing -h flag (hot only)\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -h test_a.gcda test_a.gcno", gcov_tool_cmd);
    execute_command(cmd);
    
    /* Test 6: -t flag with threshold - triggers case 't' */
    printf("\nTest 6: Testing -t flag with threshold 0.5\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.5 test_a.gcda test_b.gcda", gcov_tool_cmd);
    execute_command(cmd);
    
    /* Test 7: -t flag with different threshold */
    printf("\nTest 7: Testing -t flag with threshold 0.75\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.75 test_b.gcda test_a.gcda", gcov_tool_cmd);
    execute_command(cmd);
    
    /* Test 8: -t flag with threshold 0.0 (edge case) */
    printf("\nTest 8: Testing -t flag with threshold 0.0\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.0 test_a.gcda test_a.gcno", gcov_tool_cmd);
    execute_command(cmd);
    
    /* Test 9: Combination of multiple flags */
    printf("\nTest 9: Testing combination -v -f -o\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f -o test_a.gcda test_b.gcda", gcov_tool_cmd);
    execute_command(cmd);
    
    /* Test 10: Another combination with -F and -h */
    printf("\nTest 10: Testing combination -F -h -t 0.3\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -F -h -t 0.3 test_b.gcda test_a.gcda", gcov_tool_cmd);
    execute_command(cmd);
    
    /* Test 11: Test with multiple .gcda files */
    printf("\nTest 11: Testing with multiple input files\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v test_a.gcda test_b.gcda test_c1.gcda", gcov_tool_cmd);
    execute_command(cmd);
    
    /* Test 12: Test with zero-coverage file (test_d.gcda exists but program wasn't run) */
    printf("\nTest 12: Testing with zero-coverage file\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.5 test_a.gcda test_d.gcda", gcov_tool_cmd);
    execute_command(cmd);
    
    /* Test 13: Invalid option to trigger default case and overlap_usage() */
    printf("\nTest 13: Testing invalid option -z (to trigger default case)\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -z 2>&1 | head -5", gcov_tool_cmd);
    execute_command(cmd);
    
    /* Test 14: Another invalid option combination */
    printf("\nTest 14: Testing invalid option -x (to trigger default case again)\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -x test_a.gcda 2>&1 | head -3", gcov_tool_cmd);
    execute_command(cmd);
    
    /* Test 15: Valid flags but with .gcno files (testing different input types) */
    printf("\nTest 15: Testing with .gcno files\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f test_a.gcno test_b.gcno", gcov_tool_cmd);
    execute_command(cmd);
    
    /* Test 16: Mix of .gcda and .gcno files with hot threshold */
    printf("\nTest 16: Testing mixed .gcda/.gcno files with -t flag\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.6 test_a.gcda test_b.gcno test_c1.gcda", gcov_tool_cmd);
    execute_command(cmd);
    
    /* Clean up */
    printf("\n=== Cleaning up generated files ===\n");
    cleanup_files(cleanup_list, cleanup_count);
    
    printf("\n=== Test completed ===\n");
    printf("The following gcov-tool overlap flags were tested:\n");
    printf("  -v (verbose)           - case 'v'\n");
    printf("  -f (function level)    - case 'f'\n");
    printf("  -F (fullname)          - case 'F'\n");
    printf("  -o (object level)      - case 'o'\n");
    printf("  -h (hot only)          - case 'h'\n");
    printf("  -t (threshold)         - case 't' with various values\n");
    printf("  invalid options        - default case (triggers overlap_usage())\n");
    
    return 0;
}
