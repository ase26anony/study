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
"            if (j % 2 == 0) {\n"
"                sum += 1;\n"
"            }\n"
"        }\n"
"    }\n"
"    printf(\"Sum: %d\\n\", sum);\n"
"    return 0;\n"
"}\n";

/* Scenario C: Multiple source files - part 1 */
const char *test_prog_c1 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"void helper1(int x) {\n"
"    printf(\"Helper1: %d\\n\", x);\n"
"    if (x > 100) {\n"
"        printf(\"Large value\\n\");\n"
"    }\n"
"}\n"
"int main() {\n"
"    helper1(50);\n"
"    helper1(150);\n"
"    helper2();\n"
"    return 0;\n"
"}\n";

const char *test_prog_c2 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"void helper2(void) {\n"
"    printf(\"Helper2 called\\n\");\n"
"    for (int i = 0; i < 5; i++) {\n"
"        printf(\"Count: %d\\n\", i);\n"
"    }\n"
"}\n";

const char *test_prog_c_header = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"void helper1(int x);\n"
"void helper2(void);\n"
"#endif\n";

/* Scenario D: Program that may produce zero counts */
const char *test_prog_d = 
"#include <stdio.h>\n"
"int main() {\n"
"    int flag = 0;  /* Change to 1 to get some coverage */\n"
"    if (flag) {\n"
"        printf(\"This won't execute\\n\");\n"
"        for (int i = 0; i < 10; i++) {\n"
"            printf(\"Loop %d\\n\", i);\n"
"        }\n"
"    }\n"
"    return 0;\n"
"}\n";

/* Helper function to execute a command and check status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Command failed: %s (status: %d)\n", cmd, status);
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

/* Main test orchestrator */
int main(int argc, char **argv) {
    printf("=== Starting gcov-tool overlap command parsing test ===\n");
    
    /* Check if gcov-tool exists */
    if (system("which gcov-tool > /dev/null 2>&1") != 0) {
        /* Try in current directory */
        if (access("./gcov-tool", X_OK) != 0) {
            fprintf(stderr, "Error: gcov-tool not found in PATH or current directory\n");
            fprintf(stderr, "Please build gcov-tool with coverage flags first:\n");
            fprintf(stderr, "  g++ -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
            return 1;
        }
    }
    
    /* Create test directory */
    execute_command("mkdir -p test_coverage_data");
    
    /* =========================================== */
    /* Scenario A: Simple function with conditionals */
    /* =========================================== */
    printf("\n--- Scenario A: Simple function ---\n");
    write_to_file("test_coverage_data/test_a.c", test_prog_a);
    
    /* Compile with coverage */
    execute_command("cd test_coverage_data && "
                    "gcc -O0 -fprofile-arcs -ftest-coverage test_a.c -o test_a");
    
    /* Run to generate .gcda file */
    execute_command("cd test_coverage_data && ./test_a");
    
    /* =========================================== */
    /* Scenario B: Loop-heavy program */
    /* =========================================== */
    printf("\n--- Scenario B: Loop-heavy program ---\n");
    write_to_file("test_coverage_data/test_b.c", test_prog_b);
    
    execute_command("cd test_coverage_data && "
                    "gcc -O0 -fprofile-arcs -ftest-coverage test_b.c -o test_b");
    
    /* Run multiple times with different inputs for richer coverage */
    execute_command("cd test_coverage_data && ./test_b 1");
    execute_command("cd test_coverage_data && ./test_b 5");
    execute_command("cd test_coverage_data && ./test_b 10");
    
    /* =========================================== */
    /* Scenario C: Multiple source files */
    /* =========================================== */
    printf("\n--- Scenario C: Multiple source files ---\n");
    write_to_file("test_coverage_data/test_c1.c", test_prog_c1);
    write_to_file("test_coverage_data/test_c2.c", test_prog_c2);
    write_to_file("test_coverage_data/test_c.h", test_prog_c_header);
    
    execute_command("cd test_coverage_data && "
                    "gcc -O0 -fprofile-arcs -ftest-coverage "
                    "test_c1.c test_c2.c -o test_c");
    
    execute_command("cd test_coverage_data && ./test_c");
    
    /* =========================================== */
    /* Scenario D: Zero-count program */
    /* =========================================== */
    printf("\n--- Scenario D: Zero-count program ---\n");
    write_to_file("test_coverage_data/test_d.c", test_prog_d);
    
    execute_command("cd test_coverage_data && "
                    "gcc -O0 -fprofile-arcs -ftest-coverage test_d.c -o test_d");
    
    /* Run but most code won't execute */
    execute_command("cd test_coverage_data && ./test_d");
    
    /* =========================================== */
    /* Now invoke gcov-tool overlap with various flags */
    /* =========================================== */
    printf("\n=== Invoking gcov-tool overlap with various flags ===\n");
    
    /* Base command for gcov-tool */
    const char *base_cmd = "gcov-tool overlap";
    
    /* Test 1: -v flag (verbose) - triggers case 'v' */
    printf("\n--- Test 1: -v flag (verbose) ---\n");
    execute_command("cd test_coverage_data && "
                    "gcov-tool overlap -v test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 2: -f flag (function level) - triggers case 'f' */
    printf("\n--- Test 2: -f flag (function level) ---\n");
    execute_command("cd test_coverage_data && "
                    "gcov-tool overlap -f test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 3: -F flag (fullname) - triggers case 'F' */
    printf("\n--- Test 3: -F flag (fullname) ---\n");
    execute_command("cd test_coverage_data && "
                    "gcov-tool overlap -F test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 4: -o flag (object level) - triggers case 'o' */
    printf("\n--- Test 4: -o flag (object level) ---\n");
    execute_command("cd test_coverage_data && "
                    "gcov-tool overlap -o test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 5: -h flag (hot only) - triggers case 'h' */
    printf("\n--- Test 5: -h flag (hot only) ---\n");
    execute_command("cd test_coverage_data && "
                    "gcov-tool overlap -h test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 6: -t flag with threshold - triggers case 't' */
    printf("\n--- Test 6: -t flag with threshold (0.5) ---\n");
    execute_command("cd test_coverage_data && "
                    "gcov-tool overlap -t 0.5 test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 7: -t flag with different threshold - triggers case 't' again */
    printf("\n--- Test 7: -t flag with threshold (0.75) ---\n");
    execute_command("cd test_coverage_data && "
                    "gcov-tool overlap -t 0.75 test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 8: -t flag with extreme threshold - triggers case 't' */
    printf("\n--- Test 8: -t flag with threshold (0.01) ---\n");
    execute_command("cd test_coverage_data && "
                    "gcov-tool overlap -t 0.01 test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 9: Combination of flags - triggers multiple cases */
    printf("\n--- Test 9: Combination of flags (-v -f -o) ---\n");
    execute_command("cd test_coverage_data && "
                    "gcov-tool overlap -v -f -o test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 10: Another combination (-F -h -t) */
    printf("\n--- Test 10: Combination of flags (-F -h -t 0.3) ---\n");
    execute_command("cd test_coverage_data && "
                    "gcov-tool overlap -F -h -t 0.3 test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 11: All flags together */
    printf("\n--- Test 11: All flags together ---\n");
    execute_command("cd test_coverage_data && "
                    "gcov-tool overlap -v -f -F -o -h -t 0.25 test_a.gcda test_a.gcno 2>&1 | head -20");
    
    /* Test 12: With different input files (test_b) */
    printf("\n--- Test 12: With loop-heavy program data ---\n");
    execute_command("cd test_coverage_data && "
                    "gcov-tool overlap -v -f test_b.gcda test_b.gcno 2>&1 | head -20");
    
    /* Test 13: With multiple source files */
    printf("\n--- Test 13: With multiple source files ---\n");
    execute_command("cd test_coverage_data && "
                    "gcov-tool overlap -v test_c1.gcda test_c1.gcno test_c2.gcda test_c2.gcno 2>&1 | head -20");
    
    /* Test 14: With zero-count data */
    printf("\n--- Test 14: With zero-count data ---\n");
    execute_command("cd test_coverage_data && "
                    "gcov-tool overlap -t 0.5 test_d.gcda test_d.gcno 2>&1 | head -20");
    
    /* Test 15: Invalid option - triggers default case and overlap_usage() */
    printf("\n--- Test 15: Invalid option (-z) to trigger default case ---\n");
    execute_command("cd test_coverage_data && "
                    "gcov-tool overlap -z test_a.gcda test_a.gcno 2>&1 | head -10");
    
    /* Test 16: Another invalid option */
    printf("\n--- Test 16: Another invalid option (-X) ---\n");
    execute_command("cd test_coverage_data && "
                    "gcov-tool overlap -X test_a.gcda test_a.gcno 2>&1 | head -10");
    
    /* Test 17: No arguments after -t (should also trigger issues) */
    printf("\n--- Test 17: Missing argument for -t ---\n");
    execute_command("cd test_coverage_data && "
                    "gcov-tool overlap -t 2>&1 | head -10");
    
    /* Test 18: Help flag (should show usage but not trigger our target lines) */
    printf("\n--- Test 18: Help flag for reference ---\n");
    execute_command("gcov-tool overlap --help 2>&1 | head -20");
    
    /* =========================================== */
    /* Cleanup */
    /* =========================================== */
    printf("\n=== Cleaning up test files ===\n");
    
    /* Remove generated files */
    execute_command("rm -rf test_coverage_data");
    
    printf("\n=== Test completed ===\n");
    printf("The gcov-tool overlap command has been invoked with various flag combinations.\n");
    printf("Check coverage of gcov-tool.cc lines 534-554 to verify all cases were executed.\n");
    
    return 0;
}
