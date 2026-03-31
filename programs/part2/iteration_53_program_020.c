#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH 1024

/* Simple test programs to generate coverage data */

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
"void func2() {\n"
"    for (int i = 0; i < 3; i++) {\n"
"        printf(\"Loop %d\\n\", i);\n"
"    }\n"
"}\n"
"int main() {\n"
"    func1(5);\n"
"    func1(-2);\n"
"    func2();\n"
"    return 0;\n"
"}\n";

/* Scenario B: Loop heavy program */
const char *test_b = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"int main(int argc, char **argv) {\n"
"    int limit = 5;\n"
"    if (argc > 1) limit = atoi(argv[1]);\n"
"    \n"
"    int sum = 0;\n"
"    for (int i = 0; i < limit; i++) {\n"
"        for (int j = 0; j < i; j++) {\n"
"            sum += i * j;\n"
"        }\n"
"    }\n"
"    printf(\"Sum: %d\\n\", sum);\n"
"    \n"
"    // Hot path vs cold path\n"
"    if (sum > 100) {\n"
"        printf(\"Hot path\\n\");\n"
"    } else {\n"
"        printf(\"Cold path\\n\");\n"
"    }\n"
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
"    for (int i = 0; i < 2; i++) {\n"
"        printf(\"Iteration %d\\n\", i);\n"
"    }\n"
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
"int main() {\n"
"    // This program runs but doesn't hit instrumented code\n"
"    // if we compile with optimization that removes the printf\n"
"    // For zero counts, we just won't run it at all\n"
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
char* find_gcov_tool() {
    static char path[MAX_PATH];
    
    /* Check current directory first */
    if (access("./gcov-tool", X_OK) == 0) {
        strcpy(path, "./gcov-tool");
        return path;
    }
    
    /* Check in PATH */
    char *path_env = getenv("PATH");
    if (path_env) {
        char *path_copy = strdup(path_env);
        char *dir = strtok(path_copy, ":");
        while (dir) {
            snprintf(path, MAX_PATH, "%s/gcov-tool", dir);
            if (access(path, X_OK) == 0) {
                free(path_copy);
                return path;
            }
            dir = strtok(NULL, ":");
        }
        free(path_copy);
    }
    
    /* Not found */
    return NULL;
}

int main(int argc, char **argv) {
    char gcov_tool_path[MAX_PATH];
    char *gcov_tool = find_gcov_tool();
    
    if (!gcov_tool) {
        fprintf(stderr, "Error: gcov-tool not found in PATH or current directory\n");
        fprintf(stderr, "Please build gcov-tool with coverage flags first:\n");
        fprintf(stderr, "  g++ -fprofile-arcs -ftest-coverage -o gcov-tool gcov-tool.cc\n");
        return 1;
    }
    
    strncpy(gcov_tool_path, gcov_tool, MAX_PATH);
    printf("Using gcov-tool at: %s\n", gcov_tool_path);
    
    /* Create test directory */
    if (system("mkdir -p test_coverage_data") != 0) {
        perror("mkdir");
        return 1;
    }
    
    /* Change to test directory */
    if (chdir("test_coverage_data") != 0) {
        perror("chdir");
        return 1;
    }
    
    /* Compile and run test programs to generate coverage data */
    
    /* Scenario A: Simple function */
    printf("\n=== Generating Scenario A coverage data ===\n");
    if (!write_file("test_a.c", test_a)) return 1;
    if (!execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_a.c -o test_a")) return 1;
    if (!execute_command("./test_a")) return 1;
    
    /* Scenario B: Loop heavy - run multiple times with different inputs */
    printf("\n=== Generating Scenario B coverage data ===\n");
    if (!write_file("test_b.c", test_b)) return 1;
    if (!execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_b.c -o test_b")) return 1;
    if (!execute_command("./test_b 3")) return 1;
    if (!execute_command("./test_b 10")) return 1;  // Different execution counts
    
    /* Scenario C: Multiple source files */
    printf("\n=== Generating Scenario C coverage data ===\n");
    if (!write_file("test_c.h", test_c_header)) return 1;
    if (!write_file("test_c1.c", test_c1)) return 1;
    if (!write_file("test_c2.c", test_c2)) return 1;
    if (!execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_c1.c test_c2.c -o test_c")) return 1;
    if (!execute_command("./test_c")) return 1;
    
    /* Scenario D: Zero counts - compile but don't run */
    printf("\n=== Generating Scenario D coverage data (zero counts) ===\n");
    if (!write_file("test_d.c", test_d)) return 1;
    if (!execute_command("gcc -O0 -fprofile-arcs -ftest-coverage test_d.c -o test_d")) return 1;
    /* Intentionally not running the executable to get zero counts */
    
    /* Now invoke gcov-tool overlap with various flag combinations */
    printf("\n=== Testing gcov-tool overlap with various flags ===\n");
    
    /* Test case 1: -v flag (verbose) - triggers case 'v' */
    printf("\n--- Testing -v flag ---\n");
    execute_command("%s overlap -v test_a.gcda test_a.gcno", gcov_tool_path);
    
    /* Test case 2: -f flag (function level) - triggers case 'f' */
    printf("\n--- Testing -f flag ---\n");
    execute_command("%s overlap -f test_a.gcda test_b.gcda", gcov_tool_path);
    
    /* Test case 3: -F flag (fullname) - triggers case 'F' */
    printf("\n--- Testing -F flag ---\n");
    execute_command("%s overlap -F test_a.gcda test_b.gcda", gcov_tool_path);
    
    /* Test case 4: -o flag (object level) - triggers case 'o' */
    printf("\n--- Testing -o flag ---\n");
    execute_command("%s overlap -o test_a.gcda test_b.gcda", gcov_tool_path);
    
    /* Test case 5: -h flag (hot only) - triggers case 'h' */
    printf("\n--- Testing -h flag ---\n");
    execute_command("%s overlap -h test_a.gcda test_b.gcda", gcov_tool_path);
    
    /* Test case 6: -t flag with threshold - triggers case 't' */
    printf("\n--- Testing -t flag with threshold 0.5 ---\n");
    execute_command("%s overlap -t 0.5 test_a.gcda test_b.gcda", gcov_tool_path);
    
    /* Test case 7: -t flag with different threshold - triggers case 't' */
    printf("\n--- Testing -t flag with threshold 0.75 ---\n");
    execute_command("%s overlap -t 0.75 test_a.gcda test_b.gcda", gcov_tool_path);
    
    /* Test case 8: -t flag with threshold 0.0 - triggers case 't' */
    printf("\n--- Testing -t flag with threshold 0.0 ---\n");
    execute_command("%s overlap -t 0.0 test_a.gcda test_b.gcda", gcov_tool_path);
    
    /* Test case 9: Combination of flags - triggers multiple cases */
    printf("\n--- Testing combination -v -f -o ---\n");
    execute_command("%s overlap -v -f -o test_a.gcda test_b.gcda", gcov_tool_path);
    
    /* Test case 10: Another combination - triggers multiple cases */
    printf("\n--- Testing combination -F -h -t 0.8 ---\n");
    execute_command("%s overlap -F -h -t 0.8 test_a.gcda test_b.gcda", gcov_tool_path);
    
    /* Test case 11: All flags together */
    printf("\n--- Testing all flags together ---\n");
    execute_command("%s overlap -v -f -F -o -h -t 0.9 test_a.gcda test_b.gcda", gcov_tool_path);
    
    /* Test case 12: Invalid option - triggers default case and overlap_usage() */
    printf("\n--- Testing invalid option -z (triggers default case) ---\n");
    execute_command("%s overlap -z 2>&1 | head -20", gcov_tool_path);
    
    /* Test case 13: Multiple input files with various flags */
    printf("\n--- Testing with multiple .gcda files ---\n");
    execute_command("%s overlap -v -f test_a.gcda test_b.gcda test_c1.gcda test_c2.gcda", gcov_tool_path);
    
    /* Test case 14: With .gcno files as input */
    printf("\n--- Testing with .gcno files ---\n");
    execute_command("%s overlap -v test_a.gcno test_b.gcno", gcov_tool_path);
    
    /* Test case 15: Mixed .gcda and .gcno with threshold */
    printf("\n--- Testing mixed files with threshold ---\n");
    execute_command("%s overlap -t 0.3 test_a.gcda test_a.gcno test_b.gcda", gcov_tool_path);
    
    /* Test case 16: Testing with zero-count files (Scenario D) */
    printf("\n--- Testing with zero-count files ---\n");
    execute_command("%s overlap -t 0.1 test_d.gcda test_a.gcda", gcov_tool_path);
    
    /* Test case 17: Different order of flags */
    printf("\n--- Testing different flag order ---\n");
    execute_command("%s overlap test_a.gcda test_b.gcda -v -f", gcov_tool_path);
    
    /* Test case 18: Edge case - very high threshold */
    printf("\n--- Testing with very high threshold ---\n");
    execute_command("%s overlap -t 99.9 test_a.gcda test_b.gcda", gcov_tool_path);
    
    /* Test case 19: Edge case - very low threshold */
    printf("\n--- Testing with very low threshold ---\n");
    execute_command("%s overlap -t 0.001 test_a.gcda test_b.gcda", gcov_tool_path);
    
    /* Test case 20: Another invalid option to trigger default */
    printf("\n--- Testing another invalid option -x ---\n");
    execute_command("%s overlap -x 2>&1 | head -20", gcov_tool_path);
    
    printf("\n=== All gcov-tool overlap tests completed ===\n");
    
    /* Optional: Clean up */
    printf("\nClean up? (y/n): ");
    int c = getchar();
    if (c == 'y' || c == 'Y') {
        chdir("..");
        system("rm -rf test_coverage_data");
        printf("Cleaned up test directory.\n");
    } else {
        printf("Test files preserved in test_coverage_data/\n");
    }
    
    return 0;
}
