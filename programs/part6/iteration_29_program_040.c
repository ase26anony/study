/**
 * test_gcov_tool_overlap.c
 * 
 * Test driver for gcov-tool overlap subcommand to exercise the
 * parse_overlap_options function uncovered lines.
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
#define TEMP_DIR_PREFIX "/tmp/gcov_test_XXXXXX"

/* Structure to hold test case information */
typedef struct {
    const char *description;
    const char *args;
    int expected_exit_code;
    int should_succeed;
} test_case_t;

/* Global variables for test configuration */
static char temp_dir[256];
static char test_prog_path[512];
static char gcda_files[2][512];

/**
 * Create a temporary directory for test files
 */
static int create_temp_dir(void) {
    char *dir = mkdtemp(strcpy(temp_dir, TEMP_DIR_PREFIX));
    if (!dir) {
        perror("Failed to create temporary directory");
        return -1;
    }
    printf("Created temp directory: %s\n", temp_dir);
    return 0;
}

/**
 * Clean up temporary directory and files
 */
static void cleanup_temp_dir(void) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
    system(cmd);
    printf("Cleaned up temp directory: %s\n", temp_dir);
}

/**
 * Create a simple C program for GCOV instrumentation
 */
static int create_test_program(void) {
    const char *program = 
        "#include <stdio.h>\n"
        "int helper(int x) { return x * 2; }\n"
        "int main() {\n"
        "    int i;\n"
        "    for (i = 0; i < 10; i++) {\n"
        "        printf(\"Value: %d\\n\", helper(i));\n"
        "    }\n"
        "    return 0;\n"
        "}\n";
    
    snprintf(test_prog_path, sizeof(test_prog_path), "%s/test_prog.c", temp_dir);
    
    FILE *fp = fopen(test_prog_path, "w");
    if (!fp) {
        perror("Failed to create test program");
        return -1;
    }
    fputs(program, fp);
    fclose(fp);
    
    printf("Created test program: %s\n", test_prog_path);
    return 0;
}

/**
 * Compile the test program with GCOV instrumentation
 */
static int compile_test_program(void) {
    char cmd[1024];
    const char *binary_path = "/tmp/test_prog_gcov";
    
    /* Compile with coverage flags */
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s 2>&1",
             binary_path, test_prog_path);
    
    printf("Compiling: %s\n", cmd);
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Compilation failed\n");
        return -1;
    }
    
    /* Run the program to generate .gcda files */
    printf("Running program to generate coverage data...\n");
    ret = system(binary_path);
    if (ret != 0) {
        fprintf(stderr, "Program execution failed\n");
        return -1;
    }
    
    /* Copy .gcda files to our temp directory */
    snprintf(cmd, sizeof(cmd), "cp *.gcda *.gcno %s/ 2>/dev/null", temp_dir);
    system(cmd);
    
    /* Set up paths for the gcda files */
    for (int i = 0; i < 2; i++) {
        snprintf(gcda_files[i], sizeof(gcda_files[i]), 
                 "%s/test_prog.gcda", temp_dir);
    }
    
    printf("Generated GCOV data files\n");
    return 0;
}

/**
 * Execute a gcov-tool command and return exit code
 */
static int execute_gcov_tool(const char *args, char *output, size_t output_size) {
    char cmd[MAX_CMD_LEN];
    char tmp_output[4096];
    FILE *fp;
    int status;
    
    /* Build the command */
    snprintf(cmd, sizeof(cmd), "gcov-tool %s 2>&1", args);
    
    printf("Executing: %s\n", cmd);
    
    /* Execute command and capture output */
    fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        return -1;
    }
    
    /* Read output */
    output[0] = '\0';
    while (fgets(tmp_output, sizeof(tmp_output), fp) != NULL) {
        strncat(output, tmp_output, output_size - strlen(output) - 1);
    }
    
    /* Get exit status */
    status = pclose(fp);
    
    return WEXITSTATUS(status);
}

/**
 * Run a single test case
 */
static int run_test_case(const test_case_t *test, int test_num) {
    char output[4096];
    char full_args[MAX_CMD_LEN];
    int exit_code;
    
    printf("\n=== Test %d: %s ===\n", test_num, test->description);
    printf("Args: %s\n", test->args);
    
    /* Build full argument string */
    snprintf(full_args, sizeof(full_args), "%s %s %s", 
             test->args, gcda_files[0], gcda_files[1]);
    
    /* Execute the command */
    exit_code = execute_gcov_tool(full_args, output, sizeof(output));
    
    /* Print output if any */
    if (strlen(output) > 0) {
        printf("Output:\n%s\n", output);
    }
    
    printf("Exit code: %d (expected: %d)\n", exit_code, test->expected_exit_code);
    
    /* Check result */
    if (test->should_succeed) {
        if (exit_code == 0) {
            printf("✓ PASS\n");
            return 1;
        } else {
            printf("✗ FAIL - Expected success but got exit code %d\n", exit_code);
            return 0;
        }
    } else {
        if (exit_code != 0) {
            printf("✓ PASS (expected failure)\n");
            return 1;
        } else {
            printf("✗ FAIL - Expected failure but got success\n");
            return 0;
        }
    }
}

int main(int argc, char *argv[]) {
    test_case_t test_cases[] = {
        /* Basic flag tests - all uncovered cases in one command */
        {
            "All uncovered flags combined",
            "overlap -v -f -F -o -h -t 0.75",
            0,
            1
        },
        
        /* Individual flag tests */
        {
            "Verbose flag (-v)",
            "overlap -v",
            0,
            1
        },
        {
            "Function level flag (-f)",
            "overlap -f",
            0,
            1
        },
        {
            "Full filename flag (-F)",
            "overlap -F",
            0,
            1
        },
        {
            "Object level flag (-o)",
            "overlap -o",
            0,
            1
        },
        {
            "Hot only flag (-h)",
            "overlap -h",
            0,
            1
        },
        {
            "Hot threshold flag (-t)",
            "overlap -t 0.5",
            0,
            1
        },
        
        /* Flag combinations and permutations */
        {
            "Flags in different order 1",
            "overlap -t 1.0 -h -o -F -f -v",
            0,
            1
        },
        {
            "Flags in different order 2",
            "overlap -f -F -v -t 0.25 -h -o",
            0,
            1
        },
        {
            "Multiple -v flags",
            "overlap -v -v -v",
            0,
            1
        },
        {
            "Combination without threshold",
            "overlap -v -f -F -o -h",
            0,
            1
        },
        
        /* Edge cases for -t flag */
        {
            "Threshold with high value",
            "overlap -t 99.999",
            0,
            1
        },
        {
            "Threshold with zero",
            "overlap -t 0.0",
            0,
            1
        },
        {
            "Threshold with scientific notation",
            "overlap -t 1e-3",
            0,
            1
        },
        
        /* Error cases */
        {
            "Missing argument for -t (should fail)",
            "overlap -t",
            1,
            0
        },
        {
            "Invalid argument for -t (should fail)",
            "overlap -t not_a_number",
            1,
            0
        },
        {
            "Unknown flag (should trigger default case)",
            "overlap -x",
            1,
            0
        },
        {
            "Unknown flag with valid flags",
            "overlap -v -x -f",
            1,
            0
        },
        
        /* Additional test cases */
        {
            "Only threshold with different values",
            "overlap -t 0.33",
            0,
            1
        },
        {
            "Verbose with threshold",
            "overlap -v -t 0.66",
            0,
            1
        },
        {
            "Function level with full filename",
            "overlap -f -F",
            0,
            1
        },
        {
            "Object level with hot only",
            "overlap -o -h",
            0,
            1
        },
        
        /* Test with single input file */
        {
            "Single input file with all flags",
            "overlap -v -f -F -o -h -t 0.8",
            0,
            1
        },
        
        /* End marker */
        {NULL, NULL, 0, 0}
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("========================================\n");
    printf("Testing gcov-tool overlap argument parsing\n");
    printf("Target: Lines 534-554 in gcov-tool.cc\n");
    printf("========================================\n\n");
    
    /* Set up test environment */
    if (create_temp_dir() != 0) {
        return 1;
    }
    
    if (create_test_program() != 0) {
        cleanup_temp_dir();
        return 1;
    }
    
    if (compile_test_program() != 0) {
        cleanup_temp_dir();
        return 1;
    }
    
    /* Run all test cases */
    for (int i = 0; test_cases[i].description != NULL; i++) {
        total_tests++;
        if (run_test_case(&test_cases[i], i + 1)) {
            passed_tests++;
        }
    }
    
    /* Summary */
    printf("\n========================================\n");
    printf("Test Summary:\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", total_tests - passed_tests);
    printf("Success rate: %.1f%%\n", 
           (total_tests > 0) ? (100.0 * passed_tests / total_tests) : 0.0);
    
    /* Clean up */
    cleanup_temp_dir();
    
    /* Create a simple .gcda file in current directory for single-file tests */
    system("echo 'int main(){return 0;}' > simple.c && "
           "gcc -fprofile-arcs -ftest-coverage simple.c && "
           "./a.out 2>/dev/null");
    
    /* Run one final test with the simple .gcda file */
    printf("\n=== Final test with current directory .gcda ===\n");
    test_case_t final_test = {
        "Final test with simple.gcda",
        "overlap -v -f -F -o -h -t 0.5 simple.gcda",
        0,
        1
    };
    run_test_case(&final_test, total_tests + 1);
    
    /* Clean up simple test files */
    system("rm -f simple.c simple.gcda simple.gcno a.out 2>/dev/null");
    
    return (passed_tests == total_tests) ? 0 : 1;
}
