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

/* Structure to hold test case information */
typedef struct {
    char *description;
    char *args;
    int expected_exit_code;
} test_case_t;

/* Global variables for cleanup */
char temp_dir[256] = "";
char *gcda_files[MAX_FILES] = {NULL};
int gcda_count = 0;

/* Function prototypes */
void cleanup(void);
int create_temp_dir(void);
int compile_instrumented_program(const char *source_path, const char *executable);
int generate_gcda_file(const char *executable, const char *gcda_name);
int run_gcov_tool(const char *args, int *exit_code);
void run_test_suite(void);

/* Clean up temporary files and directories */
void cleanup(void) {
    int i;
    
    /* Remove generated .gcda files */
    for (i = 0; i < gcda_count; i++) {
        if (gcda_files[i]) {
            unlink(gcda_files[i]);
            free(gcda_files[i]);
            gcda_files[i] = NULL;
        }
    }
    gcda_count = 0;
    
    /* Remove temporary directory if created */
    if (temp_dir[0] != '\0') {
        rmdir(temp_dir);
        temp_dir[0] = '\0';
    }
}

/* Create a temporary directory for test files */
int create_temp_dir(void) {
    char template[] = "/tmp/gcov_test_XXXXXX";
    char *dir_name = mkdtemp(template);
    
    if (dir_name == NULL) {
        perror("Failed to create temporary directory");
        return -1;
    }
    
    strncpy(temp_dir, dir_name, sizeof(temp_dir) - 1);
    printf("Created temporary directory: %s\n", temp_dir);
    return 0;
}

/* Compile a simple C program with GCOV instrumentation */
int compile_instrumented_program(const char *source_path, const char *executable) {
    char cmd[MAX_CMD_LEN];
    int status;
    
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s 2>&1",
             executable, source_path);
    
    printf("Compiling: %s\n", cmd);
    status = system(cmd);
    
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return 0;
    } else {
        fprintf(stderr, "Failed to compile instrumented program\n");
        return -1;
    }
}

/* Run the instrumented program to generate .gcda file */
int generate_gcda_file(const char *executable, const char *gcda_name) {
    char cmd[MAX_CMD_LEN];
    int status;
    
    /* Run the program */
    snprintf(cmd, sizeof(cmd), "./%s", executable);
    status = system(cmd);
    
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Failed to run instrumented program\n");
        return -1;
    }
    
    /* Rename the default .gcda file to our desired name */
    char old_gcda[256];
    char new_gcda[256];
    
    snprintf(old_gcda, sizeof(old_gcda), "%s.gcda", executable);
    snprintf(new_gcda, sizeof(new_gcda), "%s/%s", temp_dir, gcda_name);
    
    if (rename(old_gcda, new_gcda) != 0) {
        perror("Failed to rename .gcda file");
        return -1;
    }
    
    /* Store the gcda file path for cleanup */
    if (gcda_count < MAX_FILES) {
        gcda_files[gcda_count] = strdup(new_gcda);
        if (gcda_files[gcda_count]) {
            gcda_count++;
        }
    }
    
    printf("Generated: %s\n", new_gcda);
    return 0;
}

/* Run gcov-tool with given arguments and capture exit code */
int run_gcov_tool(const char *args, int *exit_code) {
    char cmd[MAX_CMD_LEN];
    int status;
    
    /* Construct the full command */
    snprintf(cmd, sizeof(cmd), "gcov-tool %s", args);
    
    printf("\nRunning: %s\n", cmd);
    
    /* Execute the command */
    status = system(cmd);
    
    if (WIFEXITED(status)) {
        *exit_code = WEXITSTATUS(status);
        printf("Exit code: %d\n", *exit_code);
        return 0;
    } else if (WIFSIGNALED(status)) {
        *exit_code = -WTERMSIG(status);
        fprintf(stderr, "Command terminated by signal %d\n", WTERMSIG(status));
        return -1;
    } else {
        *exit_code = -1;
        fprintf(stderr, "Command did not exit normally\n");
        return -1;
    }
}

/* Main test suite */
void run_test_suite(void) {
    test_case_t tests[] = {
        /* Basic tests for each flag individually */
        {"Test -v flag (verbose)", "overlap -v test1.gcda test2.gcda", 0},
        {"Test -f flag (function level)", "overlap -f test1.gcda test2.gcda", 0},
        {"Test -F flag (full filename)", "overlap -F test1.gcda test2.gcda", 0},
        {"Test -o flag (object level)", "overlap -o test1.gcda test2.gcda", 0},
        {"Test -h flag (hot only)", "overlap -h test1.gcda test2.gcda", 0},
        {"Test -t flag with value 0.5", "overlap -t 0.5 test1.gcda test2.gcda", 0},
        
        /* Combined flags - testing all uncovered cases in one command */
        {"Test all flags combined", "overlap -v -f -F -o -h -t 0.75 test1.gcda test2.gcda", 0},
        
        /* Permutations of flag order */
        {"Test flag permutation 1", "overlap -t 1.0 -h -o -F -f -v test1.gcda test2.gcda", 0},
        {"Test flag permutation 2", "overlap -F -v -t 0.25 -f -o -h test1.gcda test2.gcda", 0},
        {"Test flag permutation 3", "overlap -h -t 0.9 -o -F -v -f test1.gcda test2.gcda", 0},
        
        /* Edge cases for -t flag */
        {"Test -t with very small value", "overlap -t 0.001 test1.gcda test2.gcda", 0},
        {"Test -t with value 1.0", "overlap -t 1.0 test1.gcda test2.gcda", 0},
        {"Test -t with value 100.0", "overlap -t 100.0 test1.gcda test2.gcda", 0},
        
        /* Error cases - should trigger error handling */
        {"Test -t without argument (error case)", "overlap -t", 1},  /* Should fail */
        {"Test invalid flag -x (should trigger default case)", "overlap -x test1.gcda test2.gcda", 1},
        {"Test -t with non-numeric argument", "overlap -t not_a_number test1.gcda test2.gcda", 1},
        
        /* Repeated flags */
        {"Test repeated -v flags", "overlap -v -v -v test1.gcda test2.gcda", 0},
        {"Test repeated -t flags (last one wins)", "overlap -t 0.1 -t 0.9 test1.gcda test2.gcda", 0},
        
        /* Mixed valid and invalid files */
        {"Test with non-existent file", "overlap -v nonexistent.gcda test1.gcda", 1},
        
        /* Single file (should still parse options) */
        {"Test with single file", "overlap -v -f test1.gcda", 0},
        
        /* No flags at all */
        {"Test with no flags", "overlap test1.gcda test2.gcda", 0},
        
        /* Flags at different positions */
        {"Test flags after files", "overlap test1.gcda -v test2.gcda", 0},
        {"Test flags between files", "overlap test1.gcda -f -F test2.gcda -o -h", 0},
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    int failed = 0;
    
    printf("=== Running GCOV-Tool Overlap Test Suite ===\n");
    printf("Testing parse_overlap_options() function\n");
    printf("Target lines: case 'v', 'f', 'F', 'o', 'h', 't', and default\n\n");
    
    for (int i = 0; i < num_tests; i++) {
        printf("\n--- Test %d/%d: %s ---\n", i + 1, num_tests, tests[i].description);
        
        /* Construct the actual command with file paths */
        char full_cmd[MAX_CMD_LEN];
        char *args_copy = strdup(tests[i].args);
        char *token;
        char *rest = args_copy;
        int first_token = 1;
        
        /* Build the command, replacing placeholder filenames with actual paths */
        snprintf(full_cmd, sizeof(full_cmd), "");
        
        while ((token = strtok_r(rest, " ", &rest))) {
            if (first_token) {
                first_token = 0;
                strncpy(full_cmd, token, sizeof(full_cmd) - 1);
            } else {
                /* Replace placeholder filenames with actual paths */
                if (strcmp(token, "test1.gcda") == 0) {
                    strncat(full_cmd, " ", sizeof(full_cmd) - strlen(full_cmd) - 1);
                    strncat(full_cmd, gcda_files[0], sizeof(full_cmd) - strlen(full_cmd) - 1);
                } else if (strcmp(token, "test2.gcda") == 0) {
                    strncat(full_cmd, " ", sizeof(full_cmd) - strlen(full_cmd) - 1);
                    strncat(full_cmd, gcda_files[1], sizeof(full_cmd) - strlen(full_cmd) - 1);
                } else if (strcmp(token, "nonexistent.gcda") == 0) {
                    strncat(full_cmd, " ", sizeof(full_cmd) - strlen(full_cmd) - 1);
                    strncat(full_cmd, "/tmp/nonexistent_12345.gcda", sizeof(full_cmd) - strlen(full_cmd) - 1);
                } else {
                    strncat(full_cmd, " ", sizeof(full_cmd) - strlen(full_cmd) - 1);
                    strncat(full_cmd, token, sizeof(full_cmd) - strlen(full_cmd) - 1);
                }
            }
        }
        
        free(args_copy);
        
        /* Run the command */
        int exit_code;
        int run_result = run_gcov_tool(full_cmd, &exit_code);
        
        /* Check result */
        if (run_result == 0) {
            if ((tests[i].expected_exit_code == 0 && exit_code == 0) ||
                (tests[i].expected_exit_code != 0 && exit_code != 0)) {
                printf("✓ PASS\n");
                passed++;
            } else {
                printf("✗ FAIL - Expected exit code %d, got %d\n", 
                       tests[i].expected_exit_code, exit_code);
                failed++;
            }
        } else {
            printf("✗ FAIL - Command execution failed\n");
            failed++;
        }
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", num_tests);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    printf("Success rate: %.1f%%\n", (float)passed / num_tests * 100);
}

int main(int argc, char *argv[]) {
    /* Register cleanup handler */
    atexit(cleanup);
    
    /* Create temporary directory */
    if (create_temp_dir() != 0) {
        return 1;
    }
    
    /* Change to temp directory */
    if (chdir(temp_dir) != 0) {
        perror("Failed to change to temp directory");
        return 1;
    }
    
    /* Create a simple C source file for instrumentation */
    const char *source_code = 
        "#include <stdio.h>\n"
        "int main() {\n"
        "    int i;\n"
        "    for (i = 0; i < 10; i++) {\n"
        "        printf(\"Hello, GCOV!\\n\");\n"
        "    }\n"
        "    return 0;\n"
        "}\n";
    
    FILE *source_file = fopen("test_prog.c", "w");
    if (!source_file) {
        perror("Failed to create source file");
        return 1;
    }
    fputs(source_code, source_file);
    fclose(source_file);
    
    /* Compile the instrumented program */
    if (compile_instrumented_program("test_prog.c", "test_prog") != 0) {
        return 1;
    }
    
    /* Generate multiple .gcda files with different execution counts */
    printf("\n=== Generating GCOV Data Files ===\n");
    
    /* First run - normal execution */
    if (generate_gcda_file("test_prog", "test1.gcda") != 0) {
        return 1;
    }
    
    /* Create a modified version of the program for second .gcda */
    const char *source_code2 = 
        "#include <stdio.h>\n"
        "int main() {\n"
        "    int i;\n"
        "    for (i = 0; i < 5; i++) {  /* Different loop count */\n"
        "        printf(\"Hello again!\\n\");\n"
        "    }\n"
        "    return 0;\n"
        "}\n";
    
    FILE *source_file2 = fopen("test_prog2.c", "w");
    if (!source_file2) {
        perror("Failed to create second source file");
        return 1;
    }
    fputs(source_code2, source_file2);
    fclose(source_file2);
    
    /* Compile second instrumented program */
    if (compile_instrumented_program("test_prog2.c", "test_prog2") != 0) {
        return 1;
    }
    
    /* Generate second .gcda file */
    if (generate_gcda_file("test_prog2", "test2.gcda") != 0) {
        return 1;
    }
    
    /* Run the test suite */
    run_test_suite();
    
    /* Provide instructions for coverage collection */
    printf("\n=== Coverage Collection Instructions ===\n");
    printf("To collect coverage data for gcov-tool.cc:\n");
    printf("1. Ensure gcov-tool is built with coverage: --enable-coverage flag\n");
    printf("2. Run this test program\n");
    printf("3. After tests complete, run: gcov gcov-tool.cc\n");
    printf("4. Check gcov-tool.cc.gcov for coverage of lines 534-554\n");
    
    return 0;
}
