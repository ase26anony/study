#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define MAX_FILES 10

/* Global variables to track test results */
int tests_passed = 0;
int tests_failed = 0;

/* Function to create a simple C program for GCOV instrumentation */
void create_test_program(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create test program");
        exit(1);
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        printf(\"Hello, World!\\n\");\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
}

/* Function to compile and run a test program with GCOV instrumentation */
int generate_gcda_file(const char *source_file, const char *output_name) {
    char compile_cmd[512];
    char run_cmd[512];
    
    /* Compile with GCOV instrumentation */
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s",
             output_name, source_file);
    
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return 0;
    }
    
    /* Run the program to generate .gcda file */
    snprintf(run_cmd, sizeof(run_cmd), "./%s", output_name);
    if (system(run_cmd) != 0) {
        fprintf(stderr, "Failed to run test program\n");
        return 0;
    }
    
    return 1;
}

/* Function to execute gcov-tool and check results */
void test_gcov_tool(const char *description, const char *command, int expect_success) {
    printf("Test: %s\n", description);
    printf("Command: %s\n", command);
    
    int result = system(command);
    int exit_status = WEXITSTATUS(result);
    
    if ((expect_success && exit_status == 0) || (!expect_success && exit_status != 0)) {
        printf("✓ PASSED\n\n");
        tests_passed++;
    } else {
        printf("✗ FAILED (exit status: %d)\n\n", exit_status);
        tests_failed++;
    }
}

/* Function to create permutations of flags */
void test_flag_permutations(const char *gcda1, const char *gcda2) {
    /* All individual flags */
    const char *flag_combinations[] = {
        "-v",
        "-f", 
        "-F",
        "-o",
        "-h",
        "-t 0.5",
        "-v -f",
        "-v -F",
        "-v -o",
        "-v -h",
        "-v -t 0.75",
        "-f -F -o",
        "-f -F -o -h",
        "-v -f -F -o -h -t 0.9",
        "-F -o -h -t 1.0 -v -f",
        "-h -t 0.25 -o -F -f -v",
        "-t 0.33 -v -f -F -o -h",
        NULL
    };
    
    for (int i = 0; flag_combinations[i] != NULL; i++) {
        char cmd[MAX_CMD_LEN];
        snprintf(cmd, sizeof(cmd), "gcov-tool overlap %s %s %s",
                 flag_combinations[i], gcda1, gcda2);
        
        char desc[256];
        snprintf(desc, sizeof(desc), "Flag combination: %s", flag_combinations[i]);
        test_gcov_tool(desc, cmd, 1);
    }
}

/* Function to test edge cases */
void test_edge_cases(const char *gcda1, const char *gcda2) {
    /* Test invalid argument for -t */
    test_gcov_tool("Invalid argument for -t flag", 
                   "gcov-tool overlap -t not_a_number test.gcda", 0);
    
    /* Test missing argument for -t (at end) */
    test_gcov_tool("Missing argument for -t flag (at end)",
                   "gcov-tool overlap -v -t", 0);
    
    /* Test missing argument for -t (in middle) */
    test_gcov_tool("Missing argument for -t flag (in middle)",
                   "gcov-tool overlap -v -t -f test.gcda", 0);
    
    /* Test unknown flag to trigger default case */
    test_gcov_tool("Unknown flag to trigger default case",
                   "gcov-tool overlap -x test.gcda", 0);
    
    /* Test repeated flags */
    test_gcov_tool("Repeated -v flags",
                   "gcov-tool overlap -v -v -v test.gcda", 1);
    
    /* Test with no gcda files */
    test_gcov_tool("No input files",
                   "gcov-tool overlap -v", 0);
    
    /* Test with non-existent file */
    test_gcov_tool("Non-existent input file",
                   "gcov-tool overlap -v nonexistent.gcda", 0);
    
    /* Test with absolute paths */
    char abs_path1[512], abs_path2[512];
    realpath(gcda1, abs_path1);
    realpath(gcda2, abs_path2);
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -v -f -F %s %s", 
             abs_path1, abs_path2);
    test_gcov_tool("Absolute file paths with -F flag", cmd, 1);
    
    /* Test threshold edge values */
    test_gcov_tool("Zero threshold",
                   "gcov-tool overlap -t 0.0 test.gcda", 1);
    
    test_gcov_tool("High threshold",
                   "gcov-tool overlap -t 100.0 test.gcda", 1);
    
    test_gcov_tool("Negative threshold",
                   "gcov-tool overlap -t -1.0 test.gcda", 1);
    
    /* Test with .gcno files */
    test_gcov_tool("With .gcno files",
                   "gcov-tool overlap -v test.gcno test.gcda", 1);
}

int main(int argc, char *argv[]) {
    printf("=== GCOV-Tool Overlap Parser Test Suite ===\n\n");
    
    /* Create temporary directory for test files */
    char tmpdir[] = "/tmp/gcov_test_XXXXXX";
    if (mkdtemp(tmpdir) == NULL) {
        perror("Failed to create temporary directory");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", tmpdir);
    
    /* Change to temporary directory */
    if (chdir(tmpdir) != 0) {
        perror("Failed to change to temporary directory");
        return 1;
    }
    
    /* Create test source files */
    create_test_program("test1.c");
    create_test_program("test2.c");
    
    /* Generate GCOV data files */
    printf("\nGenerating GCOV data files...\n");
    
    if (!generate_gcda_file("test1.c", "prog1")) {
        fprintf(stderr, "Failed to generate first gcda file\n");
        return 1;
    }
    
    /* Rename gcda file to avoid overwriting */
    rename("test1.gcda", "prog1.gcda");
    rename("test1.gcno", "prog1.gcno");
    
    if (!generate_gcda_file("test2.c", "prog2")) {
        fprintf(stderr, "Failed to generate second gcda file\n");
        return 1;
    }
    
    rename("test2.gcda", "prog2.gcda");
    rename("test2.gcno", "prog2.gcno");
    
    /* Create a third program with different execution pattern */
    FILE *fp = fopen("test3.c", "w");
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i;\n");
    fprintf(fp, "    for (i = 0; i < 5; i++) {\n");  // Different loop count
    fprintf(fp, "        printf(\"Test 3\\n\");\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    fclose(fp);
    
    generate_gcda_file("test3.c", "prog3");
    rename("test3.gcda", "prog3.gcda");
    rename("test3.gcno", "prog3.gcno");
    
    printf("Generated gcda files: prog1.gcda, prog2.gcda, prog3.gcda\n\n");
    
    /* Test 1: Basic flag combinations */
    printf("--- Testing Basic Flag Combinations ---\n");
    test_flag_permutations("prog1.gcda", "prog2.gcda");
    
    /* Test 2: Edge cases */
    printf("--- Testing Edge Cases ---\n");
    test_edge_cases("prog1.gcda", "prog2.gcda");
    
    /* Test 3: Multiple input files */
    printf("--- Testing Multiple Input Files ---\n");
    test_gcov_tool("Three input files with all flags",
                   "gcov-tool overlap -v -f -F -o -h -t 0.5 prog1.gcda prog2.gcda prog3.gcda", 1);
    
    /* Test 4: Different flag orders (explicit permutations) */
    printf("--- Testing Different Flag Orders ---\n");
    
    const char *flag_orders[] = {
        "-v -f -F -o -h -t 0.75",
        "-t 0.75 -h -o -F -f -v",
        "-f -v -o -h -F -t 0.75",
        "-o -h -t 0.75 -F -v -f",
        "-h -t 0.75 -o -F -f -v",
        NULL
    };
    
    for (int i = 0; flag_orders[i] != NULL; i++) {
        char cmd[MAX_CMD_LEN];
        snprintf(cmd, sizeof(cmd), "gcov-tool overlap %s prog1.gcda prog2.gcda",
                 flag_orders[i]);
        
        char desc[256];
        snprintf(desc, sizeof(desc), "Flag order permutation %d", i + 1);
        test_gcov_tool(desc, cmd, 1);
    }
    
    /* Test 5: Combined short options (if supported) */
    printf("--- Testing Combined Short Options ---\n");
    test_gcov_tool("Combined -vfF flags",
                   "gcov-tool overlap -vfF prog1.gcda prog2.gcda", 1);
    
    /* Test 6: Environment variable to prevent optimization */
    printf("--- Testing with Environment Variables ---\n");
    
    /* Use environment variable to construct command (prevents optimization) */
    char *threshold_env = getenv("GCOV_THRESHOLD");
    char threshold_arg[32];
    
    if (threshold_env) {
        snprintf(threshold_arg, sizeof(threshold_arg), "-t %s", threshold_env);
    } else {
        strcpy(threshold_arg, "-t 0.8");  /* Default */
    }
    
    char env_cmd[MAX_CMD_LEN];
    snprintf(env_cmd, sizeof(env_cmd), 
             "gcov-tool overlap -v -f %s prog1.gcda prog2.gcda", threshold_arg);
    test_gcov_tool("Command with env-based threshold", env_cmd, 1);
    
    /* Test 7: Read command from file (prevents optimization) */
    printf("--- Testing Commands from File ---\n");
    
    FILE *cmd_file = fopen("test_commands.txt", "w");
    if (cmd_file) {
        fprintf(cmd_file, "gcov-tool overlap -v -f -t 0.33 prog1.gcda prog2.gcda\n");
        fprintf(cmd_file, "gcov-tool overlap -F -o -h prog1.gcda prog3.gcda\n");
        fclose(cmd_file);
        
        /* Execute commands from file */
        system("while read cmd; do echo \"Executing: $cmd\"; $cmd; echo \"Exit: $?\"; done < test_commands.txt");
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", tests_passed + tests_failed);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    
    /* Cleanup */
    printf("\nCleaning up temporary directory...\n");
    char cleanup_cmd[256];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", tmpdir);
    system(cleanup_cmd);
    
    return (tests_failed == 0) ? 0 : 1;
}
