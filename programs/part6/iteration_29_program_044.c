/**
 * test_gcov_tool_overlap.c
 * 
 * Test driver for gcov-tool overlap command-line parsing.
 * Specifically targets the parse_overlap_options function in gcov-tool.cc
 * to cover lines 534-554.
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
#define TEMP_DIR "/tmp/gcov_test_XXXXXX"

/* Structure to hold test case information */
typedef struct {
    const char *description;
    const char *args;
    int expected_exit_code;  /* 0 for success, non-zero for failure */
    int should_trigger_default; /* Whether this should trigger default case */
} test_case_t;

/* Global variables for test configuration */
static char temp_dir[256];
static char test_prog_path[512];
static char gcda_files[2][512];

/**
 * Create a temporary directory and set up test environment
 */
static int setup_test_environment(void) {
    char *tmp;
    
    /* Create temporary directory */
    strcpy(temp_dir, TEMP_DIR);
    tmp = mkdtemp(temp_dir);
    if (!tmp) {
        perror("Failed to create temporary directory");
        return -1;
    }
    
    printf("Created temporary directory: %s\n", temp_dir);
    
    /* Create a simple C program for GCOV instrumentation */
    const char *test_program = 
        "#include <stdio.h>\n"
        "int main() {\n"
        "    int i;\n"
        "    for (i = 0; i < 10; i++) {\n"
        "        printf(\"Test %d\\n\", i);\n"
        "    }\n"
        "    return 0;\n"
        "}\n";
    
    snprintf(test_prog_path, sizeof(test_prog_path), "%s/test_prog.c", temp_dir);
    FILE *fp = fopen(test_prog_path, "w");
    if (!fp) {
        perror("Failed to create test program");
        return -1;
    }
    fputs(test_program, fp);
    fclose(fp);
    
    return 0;
}

/**
 * Compile and run the test program with GCOV instrumentation
 * to generate .gcda files
 */
static int generate_gcda_files(void) {
    char compile_cmd[1024];
    char run_cmd[1024];
    int status;
    
    /* Compile with GCOV instrumentation */
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -o %s/test_prog %s/test_prog.c",
             temp_dir, temp_dir);
    
    printf("Compiling test program: %s\n", compile_cmd);
    status = system(compile_cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return -1;
    }
    
    /* Run the program to generate first .gcda file */
    snprintf(run_cmd, sizeof(run_cmd), "%s/test_prog > /dev/null 2>&1", temp_dir);
    printf("Running test program to generate first .gcda file\n");
    system(run_cmd);
    
    /* Copy the .gcda file to create multiple versions */
    snprintf(gcda_files[0], sizeof(gcda_files[0]), 
             "%s/test_prog.gcda", temp_dir);
    
    /* Create a second .gcda file by running again with different output */
    snprintf(run_cmd, sizeof(run_cmd), 
             "cd %s && ./test_prog > /dev/null 2>&1 && cp test_prog.gcda test_prog2.gcda", 
             temp_dir);
    system(run_cmd);
    snprintf(gcda_files[1], sizeof(gcda_files[1]), 
             "%s/test_prog2.gcda", temp_dir);
    
    printf("Generated .gcda files:\n");
    printf("  %s\n", gcda_files[0]);
    printf("  %s\n", gcda_files[1]);
    
    return 0;
}

/**
 * Execute a gcov-tool command and check its exit status
 */
static int run_gcov_tool_test(const char *description, const char *args, 
                              int expected_exit_code, int should_trigger_default) {
    char command[MAX_CMD_LEN];
    int status;
    pid_t pid;
    
    /* Construct the full command */
    snprintf(command, sizeof(command), "gcov-tool overlap %s %s %s", 
             args, gcda_files[0], gcda_files[1]);
    
    printf("\n=== Test: %s ===\n", description);
    printf("Command: %s\n", command);
    printf("Expected exit code: %d\n", expected_exit_code);
    
    /* Execute using fork/exec to capture exit status */
    pid = fork();
    if (pid == 0) {
        /* Child process */
        char *argv[20];
        int argc = 0;
        char *token;
        char cmd_copy[MAX_CMD_LEN];
        
        /* Parse command into arguments */
        strcpy(cmd_copy, command);
        token = strtok(cmd_copy, " ");
        
        while (token != NULL && argc < 19) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
        argv[argc] = NULL;
        
        /* Execute gcov-tool */
        execvp("gcov-tool", argv);
        
        /* If we get here, exec failed */
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        /* Parent process */
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Actual exit code: %d\n", exit_code);
            
            if ((expected_exit_code == 0 && exit_code == 0) ||
                (expected_exit_code != 0 && exit_code != 0)) {
                printf("✓ Test PASSED\n");
                return 1;  /* Success */
            } else {
                printf("✗ Test FAILED - Unexpected exit code\n");
                return 0;  /* Failure */
            }
        } else {
            printf("✗ Test FAILED - Child did not exit normally\n");
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

/**
 * Clean up temporary files and directory
 */
static void cleanup_test_environment(void) {
    char cleanup_cmd[512];
    
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
    printf("\nCleaning up: %s\n", cleanup_cmd);
    system(cleanup_cmd);
}

int main(void) {
    test_case_t test_cases[] = {
        /* Basic tests covering all individual flags */
        {"Test -v flag (verbose)", "-v", 0, 0},
        {"Test -f flag (function level)", "-f", 0, 0},
        {"Test -F flag (full filename)", "-F", 0, 0},
        {"Test -o flag (object level)", "-o", 0, 0},
        {"Test -h flag (hot only)", "-h", 0, 0},
        {"Test -t flag with valid value", "-t 0.5", 0, 0},
        {"Test -t flag with different value", "-t 1.0", 0, 0},
        {"Test -t flag with decimal value", "-t 0.75", 0, 0},
        
        /* Combinations of flags to trigger multiple case statements */
        {"Test -v -f combination", "-v -f", 0, 0},
        {"Test -v -F combination", "-v -F", 0, 0},
        {"Test -f -F -o combination", "-f -F -o", 0, 0},
        {"Test -v -f -F -o combination", "-v -f -F -o", 0, 0},
        {"Test -v -h -t combination", "-v -h -t 0.8", 0, 0},
        
        /* All flags together in different orders */
        {"Test all flags in order -v -f -F -o -h -t", 
         "-v -f -F -o -h -t 0.6", 0, 0},
        {"Test all flags in reverse order -t -h -o -F -f -v", 
         "-t 0.7 -h -o -F -f -v", 0, 0},
        {"Test all flags in mixed order", 
         "-F -v -t 0.9 -o -f -h", 0, 0},
        
        /* Edge cases and error conditions */
        {"Test -t without argument (should fail)", "-t", 1, 0},
        {"Test -t with invalid argument", "-t not_a_number", 1, 0},
        {"Test repeated -v flags", "-v -v -v", 0, 0},
        {"Test unknown flag -x (trigger default case)", "-x", 1, 1},
        {"Test combination with unknown flag", "-v -x -f", 1, 1},
        
        /* Flag permutations to test parsing robustness */
        {"Test permutation 1", "-f -v -t 0.3 -F", 0, 0},
        {"Test permutation 2", "-o -h -t 0.4 -v", 0, 0},
        {"Test permutation 3", "-F -o -f -v -h -t 0.55", 0, 0},
        
        /* Test with single .gcda file (overlap needs at least 2) */
        {"Test with single file (different behavior)", "-v", 0, 0},
        
        {NULL, NULL, 0, 0}  /* Sentinel */
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    int i;
    
    printf("========================================\n");
    printf("Testing gcov-tool overlap argument parsing\n");
    printf("Target: parse_overlap_options() lines 534-554\n");
    printf("========================================\n\n");
    
    /* Set up test environment */
    if (setup_test_environment() != 0) {
        fprintf(stderr, "Failed to set up test environment\n");
        return EXIT_FAILURE;
    }
    
    /* Generate GCOV data files */
    if (generate_gcda_files() != 0) {
        fprintf(stderr, "Failed to generate .gcda files\n");
        cleanup_test_environment();
        return EXIT_FAILURE;
    }
    
    /* Run all test cases */
    for (i = 0; test_cases[i].description != NULL; i++) {
        total_tests++;
        passed_tests += run_gcov_tool_test(
            test_cases[i].description,
            test_cases[i].args,
            test_cases[i].expected_exit_code,
            test_cases[i].should_trigger_default
        );
    }
    
    /* Additional test: Use environment variable to prevent optimization */
    printf("\n=== Additional test with environment variable ===\n");
    {
        char env_cmd[MAX_CMD_LEN];
        const char *env_args = getenv("GCOV_TEST_ARGS");
        if (env_args) {
            snprintf(env_cmd, sizeof(env_cmd), "gcov-tool overlap %s %s %s", 
                     env_args, gcda_files[0], gcda_files[1]);
            printf("Using environment variable GCOV_TEST_ARGS=%s\n", env_args);
            printf("Command: %s\n", env_cmd);
            system(env_cmd);
        } else {
            printf("Set GCOV_TEST_ARGS environment variable for custom tests\n");
            printf("Example: export GCOV_TEST_ARGS='-v -f -F -o -h -t 0.5'\n");
        }
    }
    
    /* Summary */
    printf("\n========================================\n");
    printf("Test Summary:\n");
    printf("  Total tests run: %d\n", total_tests);
    printf("  Tests passed: %d\n", passed_tests);
    printf("  Tests failed: %d\n", total_tests - passed_tests);
    printf("========================================\n");
    
    /* Clean up */
    cleanup_test_environment();
    
    return (passed_tests == total_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}
