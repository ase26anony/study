#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH 1024

/* Simple test program A - Basic functions and conditionals */
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

/* Test program B - Loop heavy with command line arguments */
const char *test_prog_b = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"int main(int argc, char *argv[]) {\n"
"    int outer = 2;\n"
"    int inner = 3;\n"
"    \n"
"    if (argc > 1) outer = atoi(argv[1]);\n"
"    if (argc > 2) inner = atoi(argv[2]);\n"
"    \n"
"    int sum = 0;\n"
"    for (int i = 0; i < outer; i++) {\n"
"        for (int j = 0; j < inner; j++) {\n"
"            sum += i * j;\n"
"            if (j % 2 == 0) {\n"
"                printf(\"Even inner loop\\n\");\n"
"            }\n"
"        }\n"
"    }\n"
"    printf(\"Sum: %d\\n\", sum);\n"
"    return 0;\n"
"}\n";

/* Test program C - Multiple files (part 1) */
const char *test_prog_c1 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"void helper1(int x) {\n"
"    printf(\"Helper1: %d\\n\", x);\n"
"    if (x > 10) {\n"
"        printf(\"Large value\\n\");\n"
"    }\n"
"}\n"
"int main() {\n"
"    helper1(5);\n"
"    helper1(15);\n"
"    helper2();\n"
"    return 0;\n"
"}\n";

const char *test_prog_c2 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"void helper2(void) {\n"
"    printf(\"Helper2 called\\n\");\n"
"    for (int i = 0; i < 2; i++) {\n"
"        printf(\"Helper2 loop %d\\n\", i);\n"
"    }\n"
"}\n";

const char *test_header_c = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"void helper1(int x);\n"
"void helper2(void);\n"
"#endif\n";

/* Test program D - Minimal coverage (potentially zero counts) */
const char *test_prog_d = 
"#include <stdio.h>\n"
"int main() {\n"
"    int flag = 0;\n"
"    /* This branch is never taken */\n"
"    if (flag) {\n"
"        printf(\"This never executes\\n\");\n"
"    }\n"
"    return 0;\n"
"}\n";

/* Function to execute a command and check return status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Function to write a string to a file */
void write_to_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (f) {
        fputs(content, f);
        fclose(f);
    }
}

/* Function to check if gcov-tool exists */
int check_gcov_tool() {
    char cmd[MAX_PATH];
    snprintf(cmd, sizeof(cmd), "which gcov-tool > /dev/null 2>&1");
    int result = system(cmd);
    if (WIFEXITED(result) && WEXITSTATUS(result) == 0) {
        return 1;
    }
    
    /* Check in current directory */
    snprintf(cmd, sizeof(cmd), "test -x ./gcov-tool");
    result = system(cmd);
    if (WIFEXITED(result) && WEXITSTATUS(result) == 0) {
        return 1;
    }
    
    return 0;
}

int main(int argc, char *argv[]) {
    char cmd[MAX_PATH];
    int ret;
    
    printf("=== Starting gcov-tool overlap coverage test ===\n");
    
    /* Check if gcov-tool is available */
    if (!check_gcov_tool()) {
        fprintf(stderr, "Error: gcov-tool not found in PATH or current directory\n");
        fprintf(stderr, "Please build gcov-tool with coverage flags first:\n");
        fprintf(stderr, "  g++ -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
        return 1;
    }
    
    /* Create test directory */
    execute_command("rm -rf test_gcov_coverage");
    execute_command("mkdir -p test_gcov_coverage");
    if (chdir("test_gcov_coverage") != 0) {
        perror("chdir failed");
        return 1;
    }
    
    /* =========================================== */
    /* Test Scenario A: Simple function with branches */
    /* =========================================== */
    printf("\n--- Test Scenario A: Simple function ---\n");
    write_to_file("test_a.c", test_prog_a);
    
    /* Compile with coverage */
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage test_a.c -o test_a");
    execute_command(cmd);
    
    /* Run to generate .gcda file */
    execute_command("./test_a > /dev/null");
    
    /* Test various gcov-tool overlap flags for test A */
    printf("\nTesting gcov-tool overlap with different flags (Scenario A):\n");
    
    /* Case 'v' - verbose flag */
    printf("\n1. Testing -v flag (case 'v'):\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -v test_a.gcda test_a.gcno 2>&1 | head -5");
    execute_command(cmd);
    
    /* Case 'f' - function level overlap */
    printf("\n2. Testing -f flag (case 'f'):\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -f test_a.gcda test_a.gcno 2>&1");
    execute_command(cmd);
    
    /* Case 'F' - use fullname */
    printf("\n3. Testing -F flag (case 'F'):\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -F test_a.gcda test_a.gcno 2>&1");
    execute_command(cmd);
    
    /* Case 'o' - object level */
    printf("\n4. Testing -o flag (case 'o'):\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -o test_a.gcda test_a.gcno 2>&1");
    execute_command(cmd);
    
    /* Case 'h' - hot only */
    printf("\n5. Testing -h flag (case 'h'):\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -h test_a.gcda test_a.gcno 2>&1");
    execute_command(cmd);
    
    /* Case 't' - hot threshold with argument */
    printf("\n6. Testing -t flag with argument (case 't'):\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -t 0.5 test_a.gcda test_a.gcno 2>&1");
    execute_command(cmd);
    
    /* Test with different threshold values */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -t 0.75 test_a.gcda test_a.gcno 2>&1");
    execute_command(cmd);
    
    /* Test combination of flags */
    printf("\n7. Testing combination of flags:\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -v -f -o test_a.gcda test_a.gcno 2>&1 | head -5");
    execute_command(cmd);
    
    /* =========================================== */
    /* Test Scenario B: Loop heavy program */
    /* =========================================== */
    printf("\n--- Test Scenario B: Loop heavy program ---\n");
    write_to_file("test_b.c", test_prog_b);
    
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage test_b.c -o test_b");
    execute_command(cmd);
    
    /* Run multiple times with different arguments */
    execute_command("./test_b 2 3 > /dev/null");
    execute_command("./test_b 3 4 > /dev/null");
    execute_command("./test_b 1 5 > /dev/null");
    
    /* Test with hot threshold on loop-heavy program */
    printf("\nTesting -t flag with loop-heavy program:\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -t 0.1 test_b.gcda test_b.gcno 2>&1");
    execute_command(cmd);
    
    /* Test with -h (hot only) on loop program */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -h -t 0.01 test_b.gcda test_b.gcno 2>&1");
    execute_command(cmd);
    
    /* =========================================== */
    /* Test Scenario C: Multiple source files */
    /* =========================================== */
    printf("\n--- Test Scenario C: Multiple source files ---\n");
    write_to_file("test_c.h", test_header_c);
    write_to_file("test_c1.c", test_prog_c1);
    write_to_file("test_c2.c", test_prog_c2);
    
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage test_c1.c test_c2.c -o test_c");
    execute_command(cmd);
    
    execute_command("./test_c > /dev/null");
    
    /* Test with multiple .gcda files */
    printf("\nTesting with multiple coverage files:\n");
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -v test_c1.gcda test_c2.gcda test_c1.gcno test_c2.gcno 2>&1 | head -5");
    execute_command(cmd);
    
    /* Test with -F (fullname) on multiple files */
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -F test_c1.gcda test_c2.gcda test_c1.gcno test_c2.gcno 2>&1");
    execute_command(cmd);
    
    /* =========================================== */
    /* Test Scenario D: Minimal/zero coverage */
    /* =========================================== */
    printf("\n--- Test Scenario D: Minimal coverage ---\n");
    write_to_file("test_d.c", test_prog_d);
    
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage test_d.c -o test_d");
    execute_command(cmd);
    
    /* Run but don't trigger any branches */
    execute_command("./test_d > /dev/null");
    
    /* Test with zero coverage - threshold should show nothing hot */
    printf("\nTesting with zero coverage data:\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -h -t 0.01 test_d.gcda test_d.gcno 2>&1");
    execute_command(cmd);
    
    /* =========================================== */
    /* Test default case (invalid option) */
    /* =========================================== */
    printf("\n--- Testing default case (invalid option) ---\n");
    printf("This should trigger overlap_usage():\n");
    
    /* Invalid option -z */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -z test_a.gcda test_a.gcno 2>&1");
    execute_command(cmd);
    
    /* Another invalid option combination */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -x -y test_a.gcda test_a.gcno 2>&1");
    execute_command(cmd);
    
    /* =========================================== */
    /* Additional flag combinations */
    /* =========================================== */
    printf("\n--- Testing additional flag combinations ---\n");
    
    /* All flags together */
    printf("\nTesting all valid flags together:\n");
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -v -f -F -o -h -t 0.3 test_a.gcda test_a.gcno 2>&1 | head -10");
    execute_command(cmd);
    
    /* Just -t with extreme values */
    printf("\nTesting -t with extreme values:\n");
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -t 0.0 test_a.gcda test_a.gcno 2>&1");
    execute_command(cmd);
    
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -t 1.0 test_a.gcda test_a.gcno 2>&1");
    execute_command(cmd);
    
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -t 0.999 test_a.gcda test_a.gcno 2>&1");
    execute_command(cmd);
    
    /* Test with multiple input files and various flags */
    printf("\nTesting with all generated .gcda files:\n");
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -v -f *.gcda *.gcno 2>&1 | head -10");
    execute_command(cmd);
    
    /* =========================================== */
    /* Cleanup and exit */
    /* =========================================== */
    printf("\n=== Cleaning up test directory ===\n");
    chdir("..");
    execute_command("rm -rf test_gcov_coverage");
    
    printf("\n=== gcov-tool overlap coverage test completed ===\n");
    printf("All flag combinations have been tested:\n");
    printf("  -v (verbose)           - Triggered\n");
    printf("  -f (function level)    - Triggered\n");
    printf("  -F (fullname)          - Triggered\n");
    printf("  -o (object level)      - Triggered\n");
    printf("  -h (hot only)          - Triggered\n");
    printf("  -t (threshold)         - Triggered with various values\n");
    printf("  default case           - Triggered with invalid option -z\n");
    
    return 0;
}
