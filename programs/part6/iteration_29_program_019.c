/**
 * test_gcov_tool_overlap.c
 * 
 * Test driver to exercise the parse_overlap_options function in gcov-tool.cc
 * Specifically targets lines 534-554 handling flags: -v, -f, -F, -o, -h, -t
 */

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
#define TEMP_DIR_PATTERN "/tmp/gcov_test_XXXXXX"

typedef struct {
    char *cmd;
    int expected_exit;
    char *description;
} test_case_t;

/* Global test counters */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

/* Temporary directory for test files */
static char temp_dir[256];

/**
 * Execute a command and return its exit status
 */
static int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    fflush(stdout);
    
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/**
 * Create a simple C program for GCOV instrumentation
 */
static void create_test_program(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create test program");
        exit(1);
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    printf(\"Hello from test program\\n\");\n");
    fprintf(fp, "    for (int i = 0; i < 10; i++) {\n");
    fprintf(fp, "        if (i %% 2 == 0) {\n");
    fprintf(fp, "            printf(\"Even: %%d\\n\", i);\n");
    fprintf(fp, "        } else {\n");
    fprintf(fp, "            printf(\"Odd: %%d\\n\", i);\n");
    fprintf(fp, "        }\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
}

/**
 * Compile a program with GCOV instrumentation
 */
static void compile_with_gcov(const char *source, const char *output) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s 2>/dev/null",
             output, source);
    
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Failed to compile %s with GCOV\n", source);
        exit(1);
    }
}

/**
 * Run a program to generate .gcda files
 */
static void run_program_to_generate_gcda(const char *program, const char *gcda_prefix) {
    char cmd[MAX_CMD_LEN];
    
    /* First run */
    snprintf(cmd, sizeof(cmd), "./%s > /dev/null 2>&1", program);
    execute_command(cmd);
    
    /* Rename .gcda file to create multiple versions */
    char old_gcda[MAX_CMD_LEN];
    char new_gcda[MAX_CMD_LEN];
    
    snprintf(old_gcda, sizeof(old_gcda), "%s.gcda", program);
    snprintf(new_gcda, sizeof(new_gcda), "%s_1.gcda", gcda_prefix);
    
    rename(old_gcda, new_gcda);
    
    /* Second run with different behavior by setting environment variable */
    setenv("TEST_VAR", "1", 1);
    execute_command(cmd);
    
    snprintf(new_gcda, sizeof(new_gcda), "%s_2.gcda", gcda_prefix);
    rename(old_gcda, new_gcda);
    
    /* Third run */
    setenv("TEST_VAR", "2", 1);
    execute_command(cmd);
    
    snprintf(new_gcda, sizeof(new_gcda), "%s_3.gcda", gcda_prefix);
    rename(old_gcda, new_gcda);
}

/**
 * Run a single test case
 */
static void run_test_case(const test_case_t *test) {
    tests_run++;
    
    printf("\n=== Test %d: %s ===\n", tests_run, test->description);
    
    int exit_code = execute_command(test->cmd);
    
    if ((test->expected_exit == 0 && exit_code == 0) ||
        (test->expected_exit != 0 && exit_code != 0)) {
        printf("✓ PASSED (exit code: %d)\n", exit_code);
        tests_passed++;
    } else {
        printf("✗ FAILED - Expected exit code %d, got %d\n", 
               test->expected_exit, exit_code);
        tests_failed++;
    }
}

/**
 * Clean up temporary files
 */
static void cleanup() {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
    execute_command(cmd);
}

int main(int argc, char *argv[]) {
    printf("========================================\n");
    printf("Testing gcov-tool overlap argument parsing\n");
    printf("Targeting lines 534-554 in gcov-tool.cc\n");
    printf("========================================\n\n");
    
    /* Create temporary directory */
    strcpy(temp_dir, TEMP_DIR_PATTERN);
    if (mkdtemp(temp_dir) == NULL) {
        perror("Failed to create temporary directory");
        return 1;
    }
    
    printf("Using temporary directory: %s\n", temp_dir);
    
    /* Change to temp directory */
    if (chdir(temp_dir) != 0) {
        perror("Failed to change to temp directory");
        cleanup();
        return 1;
    }
    
    /* Create and compile test program */
    printf("\nCreating test program with GCOV instrumentation...\n");
    create_test_program("test_prog.c");
    compile_with_gcov("test_prog.c", "test_prog");
    
    /* Generate .gcda files */
    printf("Generating GCOV data files...\n");
    run_program_to_generate_gcda("test_prog", "test_data");
    
    /* Define test cases */
    test_case_t test_cases[] = {
        /* Basic flag combinations - all should succeed */
        {
            .cmd = "gcov-tool overlap -v test_data_1.gcda test_data_2.gcda",
            .expected_exit = 0,
            .description = "Basic overlap with verbose flag (-v)"
        },
        {
            .cmd = "gcov-tool overlap -f test_data_1.gcda test_data_2.gcda",
            .expected_exit = 0,
            .description = "Function level reporting (-f)"
        },
        {
            .cmd = "gcov-tool overlap -F test_data_1.gcda test_data_2.gcda",
            .expected_exit = 0,
            .description = "Use full filenames (-F)"
        },
        {
            .cmd = "gcov-tool overlap -o test_data_1.gcda test_data_2.gcda",
            .expected_exit = 0,
            .description = "Object level reporting (-o)"
        },
        {
            .cmd = "gcov-tool overlap -h test_data_1.gcda test_data_2.gcda",
            .expected_exit = 0,
            .description = "Hot only reporting (-h)"
        },
        {
            .cmd = "gcov-tool overlap -t 0.5 test_data_1.gcda test_data_2.gcda",
            .expected_exit = 0,
            .description = "Hot threshold 0.5 (-t 0.5)"
        },
        {
            .cmd = "gcov-tool overlap -t 1.0 test_data_1.gcda test_data_2.gcda",
            .expected_exit = 0,
            .description = "Hot threshold 1.0 (-t 1.0)"
        },
        {
            .cmd = "gcov-tool overlap -t 0.75 test_data_1.gcda test_data_2.gcda",
            .expected_exit = 0,
            .description = "Hot threshold 0.75 (-t 0.75)"
        },
        
        /* Combined flags - testing permutations */
        {
            .cmd = "gcov-tool overlap -v -f -F test_data_1.gcda test_data_2.gcda",
            .expected_exit = 0,
            .description = "Combined: verbose + function + fullname (-v -f -F)"
        },
        {
            .cmd = "gcov-tool overlap -f -F -o test_data_1.gcda test_data_2.gcda",
            .expected_exit = 0,
            .description = "Combined: function + fullname + object (-f -F -o)"
        },
        {
            .cmd = "gcov-tool overlap -v -h -t 0.8 test_data_1.gcda test_data_2.gcda",
            .expected_exit = 0,
            .description = "Combined: verbose + hot only + threshold (-v -h -t 0.8)"
        },
        
        /* All flags together */
        {
            .cmd = "gcov-tool overlap -v -f -F -o -h -t 0.6 test_data_1.gcda test_data_2.gcda test_data_3.gcda",
            .expected_exit = 0,
            .description = "All flags combined with three input files"
        },
        
        /* Different flag orders */
        {
            .cmd = "gcov-tool overlap -t 0.3 -h -o -F -f -v test_data_1.gcda test_data_2.gcda",
            .expected_exit = 0,
            .description = "All flags in reverse order"
        },
        {
            .cmd = "gcov-tool overlap -F -v -t 0.9 -f -h -o test_data_1.gcda test_data_2.gcda",
            .expected_exit = 0,
            .description = "Flags in mixed order 1"
        },
        {
            .cmd = "gcov-tool overlap -h -t 0.2 -F -v -o -f test_data_1.gcda test_data_2.gcda",
            .expected_exit = 0,
            .description = "Flags in mixed order 2"
        },
        
        /* Edge cases and error conditions */
        {
            .cmd = "gcov-tool overlap -t not_a_number test_data_1.gcda test_data_2.gcda",
            .expected_exit = 1,  /* Should fail due to invalid number */
            .description = "Invalid argument for -t (not_a_number)"
        },
        {
            .cmd = "gcov-tool overlap -t test_data_1.gcda test_data_2.gcda",
            .expected_exit = 1,  /* Should fail - missing argument */
            .description = "Missing argument for -t flag"
        },
        {
            .cmd = "gcov-tool overlap -x test_data_1.gcda test_data_2.gcda",
            .expected_exit = 1,  /* Should fail - unknown option */
            .description = "Unknown flag (-x) to trigger default case"
        },
        
        /* Repeated flags */
        {
            .cmd = "gcov-tool overlap -v -v test_data_1.gcda test_data_2.gcda",
            .expected_exit = 0,
            .description = "Repeated verbose flag (-v -v)"
        },
        {
            .cmd = "gcov-tool overlap -f -f -f test_data_1.gcda test_data_2.gcda",
            .expected_exit = 0,
            .description = "Multiple function flags (-f -f -f)"
        },
        
        /* With absolute paths */
        {
            .cmd = NULL,  /* Will be set dynamically */
            .expected_exit = 0,
            .description = "With absolute file paths"
        },
        
        /* Threshold boundary values */
        {
            .cmd = "gcov-tool overlap -t 0.0 test_data_1.gcda test_data_2.gcda",
            .expected_exit = 0,
            .description = "Minimum threshold (-t 0.0)"
        },
        {
            .cmd = "gcov-tool overlap -t 100.0 test_data_1.gcda test_data_2.gcda",
            .expected_exit = 0,
            .description = "Large threshold (-t 100.0)"
        },
        {
            .cmd = "gcov-tool overlap -t -1.0 test_data_1.gcda test_data_2.gcda",
            .expected_exit = 0,  /* Negative threshold might be accepted */
            .description = "Negative threshold (-t -1.0)"
        },
        
        /* Empty command terminator */
        { .cmd = NULL, .expected_exit = 0, .description = NULL }
    };
    
    /* Create absolute path test case dynamically */
    char abs_path_cmd[MAX_CMD_LEN];
    char abs_path1[MAX_CMD_LEN];
    char abs_path2[MAX_CMD_LEN];
    
    snprintf(abs_path1, sizeof(abs_path1), "%s/test_data_1.gcda", temp_dir);
    snprintf(abs_path2, sizeof(abs_path2), "%s/test_data_2.gcda", temp_dir);
    snprintf(abs_path_cmd, sizeof(abs_path_cmd),
             "gcov-tool overlap -v -f %s %s", abs_path1, abs_path2);
    
    /* Find and update the absolute path test case */
    for (int i = 0; test_cases[i].cmd != NULL; i++) {
        if (test_cases[i].description && 
            strstr(test_cases[i].description, "absolute file paths")) {
            test_cases[i].cmd = strdup(abs_path_cmd);
            break;
        }
    }
    
    /* Run all test cases */
    printf("\n" "Starting test execution...\n" "========================================\n");
    
    for (int i = 0; test_cases[i].cmd != NULL; i++) {
        run_test_case(&test_cases[i]);
    }
    
    /* Free dynamically allocated command string */
    for (int i = 0; test_cases[i].cmd != NULL; i++) {
        if (test_cases[i].description && 
            strstr(test_cases[i].description, "absolute file paths")) {
            free((char*)test_cases[i].cmd);
            break;
        }
    }
    
    /* Print summary */
    printf("\n" "========================================\n");
    printf("Test Summary:\n");
    printf("  Total tests run:    %d\n", tests_run);
    printf("  Tests passed:       %d\n", tests_passed);
    printf("  Tests failed:       %d\n", tests_failed);
    printf("  Success rate:       %.1f%%\n", 
           tests_run > 0 ? (100.0 * tests_passed / tests_run) : 0.0);
    printf("========================================\n");
    
    /* Cleanup */
    cleanup();
    
    return tests_failed > 0 ? 1 : 0;
}
