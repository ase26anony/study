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

typedef struct {
    char *cmd;
    int expected_exit;
    char *description;
} test_case_t;

/* Create a simple C program for GCOV instrumentation */
const char *test_program = 
"#include <stdio.h>\n"
"int main() {\n"
"    int i;\n"
"    for (i = 0; i < 10; i++) {\n"
"        if (i % 2 == 0) {\n"
"            printf(\"Even: %d\\n\", i);\n"
"        } else {\n"
"            printf(\"Odd: %d\\n\", i);\n"
"        }\n"
"    }\n"
"    return 0;\n"
"}\n";

/* Create a second program with different execution pattern */
const char *test_program2 = 
"#include <stdio.h>\n"
"int main() {\n"
"    int i;\n"
"    for (i = 0; i < 5; i++) {\n"
"        printf(\"Count: %d\\n\", i);\n"
"    }\n"
"    return 0;\n"
"}\n";

/* Execute a command and return exit status */
int run_command(const char *cmd) {
    printf("Running: %s\n", cmd);
    fflush(stdout);
    
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Create a temporary directory */
char *create_temp_dir() {
    char *template = strdup("/tmp/gcov_test_XXXXXX");
    char *dir = mkdtemp(template);
    if (!dir) {
        perror("mkdtemp failed");
        exit(1);
    }
    return dir;
}

/* Generate GCOV data files */
void generate_gcov_data(const char *dir, int num_files) {
    char cmd[MAX_CMD_LEN];
    char src_path[MAX_CMD_LEN];
    
    /* Create and compile first test program */
    snprintf(src_path, sizeof(src_path), "%s/test1.c", dir);
    FILE *fp = fopen(src_path, "w");
    if (!fp) {
        perror("Failed to create test1.c");
        exit(1);
    }
    fputs(test_program, fp);
    fclose(fp);
    
    /* Compile with GCOV instrumentation */
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -o %s/test1 %s/test1.c 2>/dev/null", 
             dir, dir);
    run_command(cmd);
    
    /* Run the program to generate .gcda files */
    snprintf(cmd, sizeof(cmd), "cd %s && ./test1 >/dev/null 2>&1", dir);
    run_command(cmd);
    
    /* Create and compile second test program */
    snprintf(src_path, sizeof(src_path), "%s/test2.c", dir);
    fp = fopen(src_path, "w");
    if (!fp) {
        perror("Failed to create test2.c");
        exit(1);
    }
    fputs(test_program2, fp);
    fclose(fp);
    
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -o %s/test2 %s/test2.c 2>/dev/null", 
             dir, dir);
    run_command(cmd);
    
    snprintf(cmd, sizeof(cmd), "cd %s && ./test2 >/dev/null 2>&1", dir);
    run_command(cmd);
    
    /* Create multiple .gcda files by running multiple times */
    for (int i = 0; i < num_files && i < 3; i++) {
        snprintf(cmd, sizeof(cmd), "cd %s && GCOV_PREFIX=%s GCOV_PREFIX_STRIP=0 ./test1 >/dev/null 2>&1", 
                 dir, dir);
        run_command(cmd);
    }
}

/* Test the specific uncovered switch cases */
void test_overlap_flags(const char *dir, const char *gcov_tool_path) {
    char cmd[MAX_CMD_LEN];
    char gcda1[MAX_CMD_LEN], gcda2[MAX_CMD_LEN];
    int passed = 0, total = 0;
    
    /* Get .gcda file paths */
    snprintf(gcda1, sizeof(gcda1), "%s/test1.gcda", dir);
    snprintf(gcda2, sizeof(gcda2), "%s/test2.gcda", dir);
    
    /* Test cases covering the uncovered lines */
    test_case_t tests[] = {
        /* Basic flag combinations - all uncovered flags in one command */
        {NULL, 0, "All flags combined (-v -f -F -o -h -t 0.5)"},
        {NULL, 0, "Verbose and function level (-v -f)"},
        {NULL, 0, "Full filename and object level (-F -o)"},
        {NULL, 0, "Hot only with threshold (-h -t 0.75)"},
        
        /* Permutations of flag order */
        {NULL, 0, "Flags in reverse order (-t 1.0 -h -o -F -f -v)"},
        {NULL, 0, "Mixed order 1 (-f -v -F -o -t 0.25 -h)"},
        {NULL, 0, "Mixed order 2 (-o -F -v -f -h -t 0.9)"},
        
        /* Individual flags (each case statement) */
        {NULL, 0, "Verbose only (-v)"},
        {NULL, 0, "Function level only (-f)"},
        {NULL, 0, "Full filename only (-F)"},
        {NULL, 0, "Object level only (-o)"},
        {NULL, 0, "Hot only (-h)"},
        {NULL, 0, "Threshold only (-t 0.5)"},
        
        /* Edge cases for threshold */
        {NULL, 0, "Threshold with 0.0 (-t 0.0)"},
        {NULL, 0, "Threshold with 1.0 (-t 1.0)"},
        {NULL, 0, "Threshold with high value (-t 100.0)"},
        {NULL, 0, "Threshold with decimal (-t 0.333)"},
        
        /* Repeated flags */
        {NULL, 0, "Repeated verbose (-v -v -v)"},
        {NULL, 0, "Multiple thresholds (should use last) (-t 0.1 -t 0.2 -t 0.3)"},
        
        /* Invalid cases that should trigger error handling */
        {NULL, 1, "Missing threshold argument (-t)"},
        {NULL, 1, "Invalid threshold (-t not_a_number)"},
        {NULL, 1, "Unknown flag (-x)"},
        
        /* Combined with file arguments */
        {NULL, 0, "All flags with two files (-v -f -F -o -h -t 0.5 with files)"},
        
        {NULL, 0, NULL}  /* Sentinel */
    };
    
    /* Fill in the command templates */
    const char *base_fmt = "%s overlap %s %s %s";
    const char *base_fmt_files = "%s overlap %s %s %s";
    
    int i = 0;
    while (tests[i].description) {
        char flags[256] = "";
        
        /* Set flags based on test description pattern */
        if (strstr(tests[i].description, "All flags combined")) {
            snprintf(flags, sizeof(flags), "-v -f -F -o -h -t 0.5");
            snprintf(cmd, sizeof(cmd), base_fmt_files, gcov_tool_path, flags, gcda1, gcda2);
        }
        else if (strstr(tests[i].description, "Verbose and function level")) {
            snprintf(flags, sizeof(flags), "-v -f");
            snprintf(cmd, sizeof(cmd), base_fmt_files, gcov_tool_path, flags, gcda1, gcda2);
        }
        else if (strstr(tests[i].description, "Full filename and object level")) {
            snprintf(flags, sizeof(flags), "-F -o");
            snprintf(cmd, sizeof(cmd), base_fmt_files, gcov_tool_path, flags, gcda1, gcda2);
        }
        else if (strstr(tests[i].description, "Hot only with threshold")) {
            snprintf(flags, sizeof(flags), "-h -t 0.75");
            snprintf(cmd, sizeof(cmd), base_fmt_files, gcov_tool_path, flags, gcda1, gcda2);
        }
        else if (strstr(tests[i].description, "Flags in reverse order")) {
            snprintf(flags, sizeof(flags), "-t 1.0 -h -o -F -f -v");
            snprintf(cmd, sizeof(cmd), base_fmt_files, gcov_tool_path, flags, gcda1, gcda2);
        }
        else if (strstr(tests[i].description, "Mixed order 1")) {
            snprintf(flags, sizeof(flags), "-f -v -F -o -t 0.25 -h");
            snprintf(cmd, sizeof(cmd), base_fmt_files, gcov_tool_path, flags, gcda1, gcda2);
        }
        else if (strstr(tests[i].description, "Mixed order 2")) {
            snprintf(flags, sizeof(flags), "-o -F -v -f -h -t 0.9");
            snprintf(cmd, sizeof(cmd), base_fmt_files, gcov_tool_path, flags, gcda1, gcda2);
        }
        else if (strstr(tests[i].description, "Verbose only")) {
            snprintf(flags, sizeof(flags), "-v");
            snprintf(cmd, sizeof(cmd), base_fmt_files, gcov_tool_path, flags, gcda1, gcda2);
        }
        else if (strstr(tests[i].description, "Function level only")) {
            snprintf(flags, sizeof(flags), "-f");
            snprintf(cmd, sizeof(cmd), base_fmt_files, gcov_tool_path, flags, gcda1, gcda2);
        }
        else if (strstr(tests[i].description, "Full filename only")) {
            snprintf(flags, sizeof(flags), "-F");
            snprintf(cmd, sizeof(cmd), base_fmt_files, gcov_tool_path, flags, gcda1, gcda2);
        }
        else if (strstr(tests[i].description, "Object level only")) {
            snprintf(flags, sizeof(flags), "-o");
            snprintf(cmd, sizeof(cmd), base_fmt_files, gcov_tool_path, flags, gcda1, gcda2);
        }
        else if (strstr(tests[i].description, "Hot only")) {
            snprintf(flags, sizeof(flags), "-h");
            snprintf(cmd, sizeof(cmd), base_fmt_files, gcov_tool_path, flags, gcda1, gcda2);
        }
        else if (strstr(tests[i].description, "Threshold only")) {
            snprintf(flags, sizeof(flags), "-t 0.5");
            snprintf(cmd, sizeof(cmd), base_fmt_files, gcov_tool_path, flags, gcda1, gcda2);
        }
        else if (strstr(tests[i].description, "Threshold with 0.0")) {
            snprintf(flags, sizeof(flags), "-t 0.0");
            snprintf(cmd, sizeof(cmd), base_fmt_files, gcov_tool_path, flags, gcda1, gcda2);
        }
        else if (strstr(tests[i].description, "Threshold with 1.0")) {
            snprintf(flags, sizeof(flags), "-t 1.0");
            snprintf(cmd, sizeof(cmd), base_fmt_files, gcov_tool_path, flags, gcda1, gcda2);
        }
        else if (strstr(tests[i].description, "Threshold with high value")) {
            snprintf(flags, sizeof(flags), "-t 100.0");
            snprintf(cmd, sizeof(cmd), base_fmt_files, gcov_tool_path, flags, gcda1, gcda2);
        }
        else if (strstr(tests[i].description, "Threshold with decimal")) {
            snprintf(flags, sizeof(flags), "-t 0.333");
            snprintf(cmd, sizeof(cmd), base_fmt_files, gcov_tool_path, flags, gcda1, gcda2);
        }
        else if (strstr(tests[i].description, "Repeated verbose")) {
            snprintf(flags, sizeof(flags), "-v -v -v");
            snprintf(cmd, sizeof(cmd), base_fmt_files, gcov_tool_path, flags, gcda1, gcda2);
        }
        else if (strstr(tests[i].description, "Multiple thresholds")) {
            snprintf(flags, sizeof(flags), "-t 0.1 -t 0.2 -t 0.3");
            snprintf(cmd, sizeof(cmd), base_fmt_files, gcov_tool_path, flags, gcda1, gcda2);
        }
        else if (strstr(tests[i].description, "Missing threshold argument")) {
            snprintf(flags, sizeof(flags), "-t");
            snprintf(cmd, sizeof(cmd), base_fmt, gcov_tool_path, flags, gcda1, gcda2);
        }
        else if (strstr(tests[i].description, "Invalid threshold")) {
            snprintf(flags, sizeof(flags), "-t not_a_number");
            snprintf(cmd, sizeof(cmd), base_fmt_files, gcov_tool_path, flags, gcda1, gcda2);
        }
        else if (strstr(tests[i].description, "Unknown flag")) {
            snprintf(flags, sizeof(flags), "-x");
            snprintf(cmd, sizeof(cmd), base_fmt_files, gcov_tool_path, flags, gcda1, gcda2);
        }
        else if (strstr(tests[i].description, "All flags with two files")) {
            snprintf(flags, sizeof(flags), "-v -f -F -o -h -t 0.5");
            /* Use both .gcda files */
            snprintf(cmd, sizeof(cmd), "%s overlap %s %s %s", 
                     gcov_tool_path, flags, gcda1, gcda2);
        }
        else {
            /* Default test */
            snprintf(cmd, sizeof(cmd), "%s overlap -v %s", gcov_tool_path, gcda1);
        }
        
        tests[i].cmd = strdup(cmd);
        i++;
    }
    
    /* Run all tests */
    printf("\n=== Running gcov-tool overlap flag tests ===\n");
    for (i = 0; tests[i].description; i++) {
        printf("\nTest %d: %s\n", i+1, tests[i].description);
        printf("Command: %s\n", tests[i].cmd);
        
        int exit_code = run_command(tests[i].cmd);
        
        if (tests[i].expected_exit == 0) {
            if (exit_code == 0) {
                printf("✓ PASSED (exit code: %d)\n", exit_code);
                passed++;
            } else {
                printf("✗ FAILED - Expected 0, got %d\n", exit_code);
            }
        } else {
            if (exit_code != 0) {
                printf("✓ PASSED - Expected non-zero, got %d\n", exit_code);
                passed++;
            } else {
                printf("✗ FAILED - Expected non-zero, got 0\n");
            }
        }
        total++;
        
        free(tests[i].cmd);
    }
    
    printf("\n=== Summary ===\n");
    printf("Passed: %d/%d tests\n", passed, total);
    
    if (passed == total) {
        printf("✓ All tests passed!\n");
    } else {
        printf("✗ Some tests failed\n");
    }
}

/* Test with different file path types */
void test_file_paths(const char *dir, const char *gcov_tool_path) {
    char cmd[MAX_CMD_LEN];
    char gcda1[MAX_CMD_LEN], gcda2[MAX_CMD_LEN];
    
    snprintf(gcda1, sizeof(gcda1), "%s/test1.gcda", dir);
    snprintf(gcda2, sizeof(gcda2), "%s/test2.gcda", dir);
    
    printf("\n=== Testing different file path types ===\n");
    
    /* Test with relative paths */
    printf("\n1. Relative paths:\n");
    snprintf(cmd, sizeof(cmd), "cd %s && %s overlap -v -f test1.gcda test2.gcda", 
             dir, gcov_tool_path);
    run_command(cmd);
    
    /* Test with absolute paths */
    printf("\n2. Absolute paths:\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f %s %s", 
             gcov_tool_path, gcda1, gcda2);
    run_command(cmd);
    
    /* Test with mixed paths */
    printf("\n3. Mixed relative/absolute (using pwd):\n");
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd))) {
        snprintf(cmd, sizeof(cmd), "cd %s && %s overlap -F -o test1.gcda %s/test2.gcda", 
                 dir, gcov_tool_path, dir);
        run_command(cmd);
    }
    
    /* Test with .gcno files as well */
    printf("\n4. With .gcno files:\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -h -t 0.5 %s/test1.gcno %s/test1.gcda", 
             gcov_tool_path, dir, dir);
    run_command(cmd);
}

int main(int argc, char *argv[]) {
    char *temp_dir = NULL;
    const char *gcov_tool_path = "gcov-tool";
    
    /* Allow overriding gcov-tool path via command line */
    if (argc > 1) {
        gcov_tool_path = argv[1];
    }
    
    printf("Using gcov-tool at: %s\n", gcov_tool_path);
    
    /* Verify gcov-tool exists and has overlap subcommand */
    printf("Checking gcov-tool availability...\n");
    if (run_command("which gcov-tool >/dev/null 2>&1") != 0) {
        if (run_command("command -v gcov-tool >/dev/null 2>&1") != 0) {
            fprintf(stderr, "Error: gcov-tool not found in PATH\n");
            fprintf(stderr, "Please build gcov-tool with coverage instrumentation:\n");
            fprintf(stderr, "  gcc --enable-coverage -o gcov-tool gcov-tool.cc ...\n");
            return 1;
        }
    }
    
    /* Create temporary directory for test files */
    temp_dir = create_temp_dir();
    printf("Created temporary directory: %s\n", temp_dir);
    
    /* Generate GCOV data files */
    printf("\nGenerating GCOV test data...\n");
    generate_gcov_data(temp_dir, 2);
    
    /* Test the specific uncovered switch cases */
    test_overlap_flags(temp_dir, gcov_tool_path);
    
    /* Test with different file path types */
    test_file_paths(temp_dir, gcov_tool_path);
    
    /* Cleanup */
    printf("\nCleaning up temporary directory: %s\n", temp_dir);
    char cleanup_cmd[MAX_CMD_LEN];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
    run_command(cleanup_cmd);
    free(temp_dir);
    
    printf("\n=== Test Complete ===\n");
    printf("To collect coverage data for gcov-tool.cc:\n");
    printf("1. Ensure gcov-tool was built with --enable-coverage\n");
    printf("2. Run: gcov gcov-tool.cc\n");
    printf("3. Check gcov-tool.cc.gcov for coverage of lines 534-554\n");
    
    return 0;
}
