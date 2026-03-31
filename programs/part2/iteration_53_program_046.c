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
const char *test_b = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"int process_matrix(int size) {\n"
"    int sum = 0;\n"
"    for (int i = 0; i < size; i++) {\n"
"        for (int j = 0; j < size; j++) {\n"
"            sum += i * j;\n"
"        }\n"
"    }\n"
"    return sum;\n"
"}\n"
"int main(int argc, char **argv) {\n"
"    int iterations = 1;\n"
"    if (argc > 1) iterations = atoi(argv[1]);\n"
"    \n"
"    int total = 0;\n"
"    for (int k = 0; k < iterations; k++) {\n"
"        total += process_matrix(10 + k % 5);\n"
"    }\n"
"    printf(\"Total: %d\\n\", total);\n"
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

const char *test_c2 = 
"#include <stdio.h>\n"
"#include \"test_c.h\"\n"
"void helper2() {\n"
"    printf(\"Helper2 called\\n\");\n"
"    for (int i = 0; i < 5; i++) {\n"
"        printf(\"  Count: %d\\n\", i);\n"
"    }\n"
"}\n";

const char *test_c_h = 
"#ifndef TEST_C_H\n"
"#define TEST_C_H\n"
"void helper1();\n"
"void helper2();\n"
"#endif\n";

/* Scenario D: Program that may produce zero counts */
const char *test_d = 
"#include <stdio.h>\n"
"int main(int argc, char **argv) {\n"
"    /* This path may not be executed */\n"
"    if (argc > 100) {  /* Never true */\n"
"        printf(\"This never runs\\n\");\n"
"        for (int i = 0; i < 10; i++) {\n"
"            printf(\"Loop %d\\n\", i);\n"
"        }\n"
"    }\n"
"    return 0;\n"
"}\n";

/* Helper function to write a string to a file */
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

/* Execute a command and check return status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        printf("Command failed with status %d: %s\n", status, cmd);
    }
    return status == 0;
}

/* Find gcov-tool in PATH or current directory */
char *find_gcov_tool() {
    static char path[1024];
    
    /* Check current directory first */
    if (access("./gcov-tool", X_OK) == 0) {
        return "./gcov-tool";
    }
    
    /* Check in PATH */
    char *path_env = getenv("PATH");
    if (path_env) {
        char *path_copy = strdup(path_env);
        char *dir = strtok(path_copy, ":");
        while (dir) {
            snprintf(path, sizeof(path), "%s/gcov-tool", dir);
            if (access(path, X_OK) == 0) {
                free(path_copy);
                return path;
            }
            dir = strtok(NULL, ":");
        }
        free(path_copy);
    }
    
    /* Try with .exe extension for Windows */
    if (access("./gcov-tool.exe", X_OK) == 0) {
        return "./gcov-tool.exe";
    }
    
    return NULL;
}

int main(int argc, char **argv) {
    printf("=== Test harness for gcov-tool overlap command parsing ===\n\n");
    
    /* Find gcov-tool */
    char *gcov_tool = find_gcov_tool();
    if (!gcov_tool) {
        fprintf(stderr, "Error: gcov-tool not found in PATH or current directory\n");
        fprintf(stderr, "Please build gcov-tool with coverage flags first:\n");
        fprintf(stderr, "  g++ -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
        return 1;
    }
    printf("Using gcov-tool: %s\n\n", gcov_tool);
    
    /* Create test directory */
    if (system("mkdir -p test_coverage_data") != 0) {
        fprintf(stderr, "Failed to create test directory\n");
        return 1;
    }
    
    /* Change to test directory */
    if (chdir("test_coverage_data") != 0) {
        perror("chdir");
        return 1;
    }
    
    int success = 1;
    
    /* ===== SCENARIO A: Simple function ===== */
    printf("\n--- Scenario A: Simple function with conditionals ---\n");
    if (!write_file("test_a.c", test_a)) {
        success = 0;
        goto cleanup;
    }
    
    /* Compile with coverage */
    if (!execute_command("gcc -fprofile-arcs -ftest-coverage -O0 test_a.c -o test_a")) {
        success = 0;
        goto cleanup;
    }
    
    /* Run to generate .gcda */
    if (!execute_command("./test_a")) {
        success = 0;
        goto cleanup;
    }
    
    /* ===== SCENARIO B: Loop heavy ===== */
    printf("\n--- Scenario B: Loop heavy program ---\n");
    if (!write_file("test_b.c", test_b)) {
        success = 0;
        goto cleanup;
    }
    
    if (!execute_command("gcc -fprofile-arcs -ftest-coverage -O0 test_b.c -o test_b")) {
        success = 0;
        goto cleanup;
    }
    
    /* Run multiple times with different arguments */
    execute_command("./test_b 1");
    execute_command("./test_b 3");
    execute_command("./test_b 5");
    
    /* ===== SCENARIO C: Multiple source files ===== */
    printf("\n--- Scenario C: Multiple source files ---\n");
    if (!write_file("test_c1.c", test_c1)) {
        success = 0;
        goto cleanup;
    }
    if (!write_file("test_c2.c", test_c2)) {
        success = 0;
        goto cleanup;
    }
    if (!write_file("test_c.h", test_c_h)) {
        success = 0;
        goto cleanup;
    }
    
    if (!execute_command("gcc -fprofile-arcs -ftest-coverage -O0 test_c1.c test_c2.c -o test_c")) {
        success = 0;
        goto cleanup;
    }
    
    execute_command("./test_c");
    
    /* ===== SCENARIO D: Zero counts ===== */
    printf("\n--- Scenario D: Program with potential zero counts ---\n");
    if (!write_file("test_d.c", test_d)) {
        success = 0;
        goto cleanup;
    }
    
    if (!execute_command("gcc -fprofile-arcs -ftest-coverage -O0 test_d.c -o test_d")) {
        success = 0;
        goto cleanup;
    }
    
    /* Run but don't execute instrumented paths */
    execute_command("./test_d");
    
    /* ===== INVOKE GCOV-TOOL OVERLAP WITH VARIOUS FLAGS ===== */
    printf("\n=== Invoking gcov-tool overlap with various flag combinations ===\n");
    
    /* Base command with required arguments */
    char base_cmd[4096];
    snprintf(base_cmd, sizeof(base_cmd), "%s overlap", gcov_tool);
    
    /* Test case 1: -v flag (verbose) */
    printf("\n1. Testing -v flag (case 'v'):\n");
    char cmd[4096];
    snprintf(cmd, sizeof(cmd), "%s -v test_a.gcda test_a.gcno", base_cmd);
    execute_command(cmd);
    
    /* Test case 2: -f flag (function level) */
    printf("\n2. Testing -f flag (case 'f'):\n");
    snprintf(cmd, sizeof(cmd), "%s -f test_a.gcda test_b.gcda", base_cmd);
    execute_command(cmd);
    
    /* Test case 3: -F flag (fullname) */
    printf("\n3. Testing -F flag (case 'F'):\n");
    snprintf(cmd, sizeof(cmd), "%s -F test_a.gcda test_b.gcda", base_cmd);
    execute_command(cmd);
    
    /* Test case 4: -o flag (object level) */
    printf("\n4. Testing -o flag (case 'o'):\n");
    snprintf(cmd, sizeof(cmd), "%s -o test_a.gcda test_b.gcda", base_cmd);
    execute_command(cmd);
    
    /* Test case 5: -h flag (hot only) */
    printf("\n5. Testing -h flag (case 'h'):\n");
    snprintf(cmd, sizeof(cmd), "%s -h test_a.gcda test_b.gcda", base_cmd);
    execute_command(cmd);
    
    /* Test case 6: -t flag with threshold (case 't') */
    printf("\n6. Testing -t flag with threshold 0.5 (case 't'):\n");
    snprintf(cmd, sizeof(cmd), "%s -t 0.5 test_a.gcda test_b.gcda", base_cmd);
    execute_command(cmd);
    
    /* Test case 7: -t flag with different threshold */
    printf("\n7. Testing -t flag with threshold 0.75:\n");
    snprintf(cmd, sizeof(cmd), "%s -t 0.75 test_a.gcda test_b.gcda", base_cmd);
    execute_command(cmd);
    
    /* Test case 8: -t flag with threshold 0.0 */
    printf("\n8. Testing -t flag with threshold 0.0:\n");
    snprintf(cmd, sizeof(cmd), "%s -t 0.0 test_a.gcda test_b.gcda", base_cmd);
    execute_command(cmd);
    
    /* Test case 9: -t flag with threshold 1.0 */
    printf("\n9. Testing -t flag with threshold 1.0:\n");
    snprintf(cmd, sizeof(cmd), "%s -t 1.0 test_a.gcda test_b.gcda", base_cmd);
    execute_command(cmd);
    
    /* Test case 10: Combination of flags */
    printf("\n10. Testing combination -v -f -o:\n");
    snprintf(cmd, sizeof(cmd), "%s -v -f -o test_a.gcda test_b.gcda", base_cmd);
    execute_command(cmd);
    
    /* Test case 11: Combination -F -h */
    printf("\n11. Testing combination -F -h:\n");
    snprintf(cmd, sizeof(cmd), "%s -F -h test_a.gcda test_b.gcda", base_cmd);
    execute_command(cmd);
    
    /* Test case 12: Combination -v -f -F -o -h -t */
    printf("\n12. Testing all flags together:\n");
    snprintf(cmd, sizeof(cmd), "%s -v -f -F -o -h -t 0.3 test_a.gcda test_b.gcda", base_cmd);
    execute_command(cmd);
    
    /* Test case 13: With multiple input files */
    printf("\n13. Testing with multiple .gcda files:\n");
    snprintf(cmd, sizeof(cmd), "%s -v test_a.gcda test_b.gcda test_c1.gcda test_c2.gcda", base_cmd);
    execute_command(cmd);
    
    /* Test case 14: With .gcno files */
    printf("\n14. Testing with .gcno files:\n");
    snprintf(cmd, sizeof(cmd), "%s -v test_a.gcda test_a.gcno test_b.gcda test_b.gcno", base_cmd);
    execute_command(cmd);
    
    /* Test case 15: Default case (invalid option -z) */
    printf("\n15. Testing invalid option -z (should trigger default case):\n");
    snprintf(cmd, sizeof(cmd), "%s -z 2>&1 | head -20", base_cmd);
    execute_command(cmd);
    
    /* Test case 16: Another invalid option */
    printf("\n16. Testing invalid option --invalid (should trigger default case):\n");
    snprintf(cmd, sizeof(cmd), "%s --invalid 2>&1 | head -20", base_cmd);
    execute_command(cmd);
    
    /* Test case 17: -t without argument (should trigger error/usage) */
    printf("\n17. Testing -t without argument:\n");
    snprintf(cmd, sizeof(cmd), "%s -t 2>&1 | head -20", base_cmd);
    execute_command(cmd);
    
    /* Test case 18: Empty arguments */
    printf("\n18. Testing with no arguments after overlap:\n");
    snprintf(cmd, sizeof(cmd), "%s 2>&1 | head -20", base_cmd);
    execute_command(cmd);
    
    /* Test case 19: With zero-count .gcda files */
    printf("\n19. Testing with zero-count .gcda files:\n");
    snprintf(cmd, sizeof(cmd), "%s -t 0.5 test_d.gcda test_a.gcda", base_cmd);
    execute_command(cmd);
    
    /* Test case 20: Different threshold with zero counts */
    printf("\n20. Testing -t 0.1 with zero counts:\n");
    snprintf(cmd, sizeof(cmd), "%s -t 0.1 test_d.gcda test_a.gcda", base_cmd);
    execute_command(cmd);

cleanup:
    printf("\n=== Cleaning up test files ===\n");
    
    /* Clean up generated files */
    system("rm -f test_a.c test_a test_a.gcda test_a.gcno test_a.gcov");
    system("rm -f test_b.c test_b test_b.gcda test_b.gcno test_b.gcov");
    system("rm -f test_c1.c test_c2.c test_c.h test_c test_c1.gcda test_c1.gcno test_c2.gcda test_c2.gcno");
    system("rm -f test_d.c test_d test_d.gcda test_d.gcno test_d.gcov");
    system("rm -f *.gcov");
    
    /* Change back to parent directory */
    chdir("..");
    system("rmdir test_coverage_data 2>/dev/null");
    
    if (success) {
        printf("\n=== Test completed successfully ===\n");
        printf("The gcov-tool overlap command was invoked with various flag combinations\n");
        printf("covering all cases in the target switch statement:\n");
        printf("  -v (verbose), -f (func level), -F (fullname), -o (object level)\n");
        printf("  -h (hot only), -t (threshold), and default case\n");
        return 0;
    } else {
        printf("\n=== Test failed ===\n");
        return 1;
    }
}
