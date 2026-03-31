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
    const char *description;
    const char *args;
    int expected_exit_code;  /* 0 for success, non-zero for expected failure */
} test_case_t;

/* Global variables for test configuration */
static char temp_dir[256];
static char test_prog_path[256];
static char gcda_files[MAX_FILES][256];
static int num_gcda_files = 0;

/* Function prototypes */
static int create_temp_directory(void);
static int compile_test_program(void);
static int generate_gcda_files(void);
static void cleanup(void);
static int run_gcov_tool(const char *args, int expected_exit);
static void run_test_suite(void);

/* Create a temporary directory for test files */
static int create_temp_directory(void) {
    const char *tmp_base = "/tmp/gcov_test_XXXXXX";
    char *tmp = mkdtemp(strcpy(temp_dir, tmp_base));
    if (!tmp) {
        perror("Failed to create temporary directory");
        return -1;
    }
    printf("Created temporary directory: %s\n", temp_dir);
    return 0;
}

/* Compile a simple test program with GCOV instrumentation */
static int compile_test_program(void) {
    const char *test_prog = 
        "#include <stdio.h>\n"
        "int main() {\n"
        "    printf(\"Test program for gcov-tool\\n\");\n"
        "    return 0;\n"
        "}\n";
    
    char source_path[256];
    snprintf(source_path, sizeof(source_path), "%s/test.c", temp_dir);
    
    /* Write test program source */
    FILE *fp = fopen(source_path, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return -1;
    }
    fputs(test_prog, fp);
    fclose(fp);
    
    /* Compile with GCOV instrumentation */
    char compile_cmd[512];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -o %s/test_prog %s/test.c 2>&1",
             temp_dir, temp_dir);
    
    printf("Compiling test program...\n");
    int ret = system(compile_cmd);
    if (ret != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return -1;
    }
    
    snprintf(test_prog_path, sizeof(test_prog_path), "%s/test_prog", temp_dir);
    return 0;
}

/* Generate multiple .gcda files by running the test program */
static int generate_gcda_files(void) {
    /* Run the test program multiple times to generate different .gcda files */
    for (int i = 0; i < 3; i++) {
        char run_cmd[256];
        snprintf(run_cmd, sizeof(run_cmd), "%s > /dev/null 2>&1", test_prog_path);
        
        /* Set GCOV_PREFIX to separate .gcda files */
        char env_var[256];
        snprintf(env_var, sizeof(env_var), "GCOV_PREFIX=%s/run%d", temp_dir, i);
        putenv(env_var);
        
        /* Run the program */
        system(run_cmd);
        
        /* Copy .gcda file to our test directory with unique name */
        char src_gcda[256], dst_gcda[256];
        snprintf(src_gcda, sizeof(src_gcda), "%s/run%d/test.gcda", temp_dir, i);
        snprintf(dst_gcda, sizeof(dst_gcda), "%s/test%d.gcda", temp_dir, i);
        
        /* Use cp command to copy the file */
        char cp_cmd[512];
        snprintf(cp_cmd, sizeof(cp_cmd), "cp %s %s 2>/dev/null", src_gcda, dst_gcda);
        system(cp_cmd);
        
        /* Store the path for later use */
        strncpy(gcda_files[num_gcda_files], dst_gcda, sizeof(gcda_files[0]) - 1);
        num_gcda_files++;
    }
    
    /* Also create a .gcno file for testing */
    char gcno_src[256], gcno_dst[256];
    snprintf(gcno_src, sizeof(gcno_src), "%s/test.gcno", temp_dir);
    snprintf(gcno_dst, sizeof(gcno_dst), "%s/test.gcno", temp_dir);
    
    /* The .gcno file should have been created during compilation */
    strncpy(gcda_files[num_gcda_files], gcno_dst, sizeof(gcda_files[0]) - 1);
    num_gcda_files++;
    
    printf("Generated %d GCOV data files\n", num_gcda_files);
    return 0;
}

/* Clean up temporary files */
static void cleanup(void) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
    system(cmd);
    printf("Cleaned up temporary directory\n");
}

/* Run gcov-tool with given arguments and check exit code */
static int run_gcov_tool(const char *args, int expected_exit) {
    char cmd[MAX_CMD_LEN];
    int exit_status;
    
    /* Build the command string */
    if (num_gcda_files > 0) {
        /* Include GCOV data files in the command */
        char file_list[512] = "";
        for (int i = 0; i < num_gcda_files && i < 2; i++) {
            strncat(file_list, " ", sizeof(file_list) - strlen(file_list) - 1);
            strncat(file_list, gcda_files[i], sizeof(file_list) - strlen(file_list) - 1);
        }
        
        snprintf(cmd, sizeof(cmd), "gcov-tool overlap %s %s 2>&1", args, file_list);
    } else {
        snprintf(cmd, sizeof(cmd), "gcov-tool overlap %s 2>&1", args);
    }
    
    printf("Running: %s\n", cmd);
    
    /* Execute the command */
    int ret = system(cmd);
    
    if (WIFEXITED(ret)) {
        exit_status = WEXITSTATUS(ret);
    } else {
        exit_status = -1;
    }
    
    /* Check if exit status matches expectation */
    if (expected_exit == 0) {
        if (exit_status == 0) {
            printf("  ✓ Success (exit code: %d)\n", exit_status);
            return 1;
        } else {
            printf("  ✗ Failed (exit code: %d, expected 0)\n", exit_status);
            return 0;
        }
    } else {
        if (exit_status != 0) {
            printf("  ✓ Expected failure (exit code: %d)\n", exit_status);
            return 1;
        } else {
            printf("  ✗ Unexpected success (exit code: %d, expected non-zero)\n", exit_status);
            return 0;
        }
    }
}

/* Run the complete test suite */
static void run_test_suite(void) {
    int passed = 0;
    int total = 0;
    
    /* Test cases covering the uncovered switch cases */
    test_case_t test_cases[] = {
        /* Basic flag tests - each flag individually */
        {"Verbose flag", "-v", 0},
        {"Function level flag", "-f", 0},
        {"Full filename flag", "-F", 0},
        {"Object level flag", "-o", 0},
        {"Hot only flag", "-h", 0},
        {"Hot threshold with value", "-t 0.5", 0},
        {"Hot threshold with different value", "-t 1.0", 0},
        {"Hot threshold with decimal", "-t 0.75", 0},
        
        /* Combinations of flags - testing all uncovered cases in one run */
        {"All flags combined", "-v -f -F -o -h -t 0.8", 0},
        
        /* Permutations of flag order */
        {"Flags in reverse order", "-t 0.6 -h -o -F -f -v", 0},
        {"Flags in mixed order 1", "-f -v -t 0.9 -F -o -h", 0},
        {"Flags in mixed order 2", "-F -o -h -t 0.3 -v -f", 0},
        
        /* Repeated flags */
        {"Repeated verbose flag", "-v -v -v", 0},
        {"Multiple hot thresholds (last wins)", "-t 0.1 -t 0.2 -t 0.3", 0},
        
        /* Edge cases and error conditions */
        {"Missing argument for -t", "-t", 1},  /* Should fail */
        {"Invalid argument for -t", "-t not_a_number", 1},  /* Should fail */
        {"Unknown flag", "-x", 1},  /* Should trigger default case */
        {"Unknown flag with valid flags", "-v -x -f", 1},  /* Should fail */
        
        /* Flag combinations without threshold */
        {"Verbose and function level", "-v -f", 0},
        {"Fullname and object level", "-F -o", 0},
        {"Hot only with verbose", "-h -v", 0},
        {"All except threshold", "-v -f -F -o -h", 0},
        
        /* Threshold with other flags */
        {"Threshold with verbose", "-t 0.5 -v", 0},
        {"Threshold with function level", "-f -t 0.7", 0},
        {"Threshold with all boolean flags", "-t 0.85 -v -f -F -o -h", 0},
        
        /* Boundary values for threshold */
        {"Zero threshold", "-t 0.0", 0},
        {"Small threshold", "-t 0.001", 0},
        {"Large threshold", "-t 100.0", 0},
        {"Negative threshold", "-t -0.5", 0},  /* May or may not be valid */
        
        /* Empty arguments (just overlap command) */
        {"No flags", "", 0},
    };
    
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    
    printf("\n=== Running gcov-tool overlap tests ===\n\n");
    
    for (int i = 0; i < num_tests; i++) {
        printf("Test %d: %s\n", i + 1, test_cases[i].description);
        if (run_gcov_tool(test_cases[i].args, test_cases[i].expected_exit_code)) {
            passed++;
        }
        total++;
        printf("\n");
    }
    
    /* Additional test: Use absolute paths for files */
    printf("Test with absolute paths:\n");
    char abs_path_cmd[256];
    if (num_gcda_files > 0) {
        char abs_path[256];
        realpath(gcda_files[0], abs_path);
        snprintf(abs_path_cmd, sizeof(abs_path_cmd), "-v -f %s", abs_path);
        if (run_gcov_tool(abs_path_cmd, 0)) {
            passed++;
        }
        total++;
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d\n", passed, total);
    printf("Coverage: %.1f%%\n", (passed * 100.0) / total);
}

int main(int argc, char *argv[]) {
    printf("=== GCOV Tool Overlap Parser Test Driver ===\n\n");
    
    /* Set up signal handler for cleanup */
    atexit(cleanup);
    
    /* Create temporary workspace */
    if (create_temp_directory() != 0) {
        return EXIT_FAILURE;
    }
    
    /* Compile test program */
    if (compile_test_program() != 0) {
        return EXIT_FAILURE;
    }
    
    /* Generate GCOV data files */
    if (generate_gcda_files() != 0) {
        return EXIT_FAILURE;
    }
    
    /* Run the test suite */
    run_test_suite();
    
    /* Note: cleanup happens via atexit */
    
    return EXIT_SUCCESS;
}
