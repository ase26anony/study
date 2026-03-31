/**
 * test_gcov_tool_overlap.c
 * 
 * Test driver to exercise the parse_overlap_options function in gcov-tool.cc
 * Specifically targets lines 534-554 containing the switch statement for
 * flags: -v, -f, -F, -o, -h, -t
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
    char *description;
    char *args;
    int expected_exit_code;
    int should_trigger_usage;
} test_case_t;

/* Global variables for cleanup */
static char temp_dir[256];
static char test_prog_path[512];
static char gcda_files[MAX_FILES][512];
static int num_gcda_files = 0;

/* Function prototypes */
void cleanup(void);
int create_temp_directory(void);
int compile_test_program(void);
int generate_gcda_files(void);
int run_gcov_tool(const char *args, int *exit_code);
void run_test_suite(void);
void test_flag_combinations(void);
void test_edge_cases(void);
void test_permutations(void);

/**
 * Clean up temporary files and directories
 */
void cleanup(void) {
    /* Remove generated files */
    for (int i = 0; i < num_gcda_files; i++) {
        if (gcda_files[i][0] != '\0') {
            unlink(gcda_files[i]);
        }
    }
    
    /* Remove test program */
    if (test_prog_path[0] != '\0') {
        unlink(test_prog_path);
        char gcno_path[512];
        snprintf(gcno_path, sizeof(gcno_path), "%s.gcno", test_prog_path);
        unlink(gcno_path);
    }
    
    /* Remove temp directory if it exists */
    if (temp_dir[0] != '\0' && strstr(temp_dir, "/tmp/gcov_test_") != NULL) {
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
        system(cmd);
    }
}

/**
 * Create a temporary directory for test files
 */
int create_temp_directory(void) {
    strcpy(temp_dir, TEMP_DIR);
    if (mkdtemp(temp_dir) == NULL) {
        perror("Failed to create temporary directory");
        return -1;
    }
    printf("Created temp directory: %s\n", temp_dir);
    return 0;
}

/**
 * Compile a simple test program with GCOV instrumentation
 */
int compile_test_program(void) {
    char source_path[512];
    char compile_cmd[1024];
    
    /* Create source file path */
    snprintf(source_path, sizeof(source_path), "%s/test_prog.c", temp_dir);
    
    /* Create a simple C program */
    FILE *fp = fopen(source_path, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return -1;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    printf(\"Test program for gcov-tool\\n\");\n");
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
    
    /* Compile with GCOV instrumentation */
    snprintf(test_prog_path, sizeof(test_prog_path), "%s/test_prog", temp_dir);
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s",
             test_prog_path, source_path);
    
    printf("Compiling test program: %s\n", compile_cmd);
    int ret = system(compile_cmd);
    if (ret != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return -1;
    }
    
    return 0;
}

/**
 * Generate .gcda files by running the test program multiple times
 */
int generate_gcda_files(void) {
    char run_cmd[512];
    char gcda_path[512];
    
    /* Run the program 3 times to generate different profiles */
    for (int i = 0; i < 3; i++) {
        snprintf(run_cmd, sizeof(run_cmd), "%s > /dev/null 2>&1", test_prog_path);
        system(run_cmd);
        
        /* Get the gcda file path */
        snprintf(gcda_path, sizeof(gcda_path), "%s.gcda", test_prog_path);
        
        /* Copy to a unique name */
        snprintf(gcda_files[num_gcda_files], sizeof(gcda_files[0]),
                 "%s/test_prog_run%d.gcda", temp_dir, i + 1);
        
        char copy_cmd[1024];
        snprintf(copy_cmd, sizeof(copy_cmd), "cp %s %s",
                 gcda_path, gcda_files[num_gcda_files]);
        
        if (system(copy_cmd) != 0) {
            fprintf(stderr, "Failed to copy gcda file\n");
            return -1;
        }
        
        num_gcda_files++;
        
        /* Remove the original so next run creates a fresh one */
        unlink(gcda_path);
    }
    
    printf("Generated %d gcda files\n", num_gcda_files);
    return 0;
}

/**
 * Run gcov-tool with the given arguments
 * Returns 0 on success, -1 on failure
 */
int run_gcov_tool(const char *args, int *exit_code) {
    char cmd[MAX_CMD_LEN];
    int status;
    
    /* Build the command */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap %s", args);
    
    printf("Running: %s\n", cmd);
    
    /* Use system() to execute the command */
    status = system(cmd);
    
    if (WIFEXITED(status)) {
        *exit_code = WEXITSTATUS(status);
        return 0;
    } else {
        *exit_code = -1;
        return -1;
    }
}

/**
 * Test various flag combinations to hit all case statements
 */
void test_flag_combinations(void) {
    int exit_code;
    int passed = 0, total = 0;
    char args[512];
    
    printf("\n=== Testing Flag Combinations ===\n");
    
    /* Test 1: All flags together with valid threshold */
    snprintf(args, sizeof(args), "-v -f -F -o -h -t 0.75 %s %s",
             gcda_files[0], gcda_files[1]);
    if (run_gcov_tool(args, &exit_code) == 0) {
        printf("Test 1 (all flags): exit code = %d\n", exit_code);
        total++;
        if (exit_code == 0) passed++;
    }
    
    /* Test 2: Only verbose flag */
    snprintf(args, sizeof(args), "-v %s %s", gcda_files[0], gcda_files[1]);
    if (run_gcov_tool(args, &exit_code) == 0) {
        printf("Test 2 (-v only): exit code = %d\n", exit_code);
        total++;
        if (exit_code == 0) passed++;
    }
    
    /* Test 3: Function level and fullname flags */
    snprintf(args, sizeof(args), "-f -F %s %s", gcda_files[0], gcda_files[1]);
    if (run_gcov_tool(args, &exit_code) == 0) {
        printf("Test 3 (-f -F): exit code = %d\n", exit_code);
        total++;
        if (exit_code == 0) passed++;
    }
    
    /* Test 4: Object level and hot only */
    snprintf(args, sizeof(args), "-o -h %s %s", gcda_files[0], gcda_files[1]);
    if (run_gcov_tool(args, &exit_code) == 0) {
        printf("Test 4 (-o -h): exit code = %d\n", exit_code);
        total++;
        if (exit_code == 0) passed++;
    }
    
    /* Test 5: Threshold with different values */
    snprintf(args, sizeof(args), "-t 0.5 %s %s", gcda_files[0], gcda_files[1]);
    if (run_gcov_tool(args, &exit_code) == 0) {
        printf("Test 5 (-t 0.5): exit code = %d\n", exit_code);
        total++;
        if (exit_code == 0) passed++;
    }
    
    /* Test 6: Threshold with 1.0 */
    snprintf(args, sizeof(args), "-t 1.0 %s %s", gcda_files[0], gcda_files[1]);
    if (run_gcov_tool(args, &exit_code) == 0) {
        printf("Test 6 (-t 1.0): exit code = %d\n", exit_code);
        total++;
        if (exit_code == 0) passed++;
    }
    
    /* Test 7: Threshold with 0.0 */
    snprintf(args, sizeof(args), "-t 0.0 %s %s", gcda_files[0], gcda_files[1]);
    if (run_gcov_tool(args, &exit_code) == 0) {
        printf("Test 7 (-t 0.0): exit code = %d\n", exit_code);
        total++;
        if (exit_code == 0) passed++;
    }
    
    printf("Flag combinations: %d/%d passed\n\n", passed, total);
}

/**
 * Test edge cases and error conditions
 */
void test_edge_cases(void) {
    int exit_code;
    int total = 0;
    
    printf("\n=== Testing Edge Cases ===\n");
    
    /* Test 1: Invalid argument for -t (should trigger atof) */
    char args[512];
    snprintf(args, sizeof(args), "-t not_a_number %s %s", 
             gcda_files[0], gcda_files[1]);
    if (run_gcov_tool(args, &exit_code) == 0) {
        printf("Test 1 (invalid -t arg): exit code = %d\n", exit_code);
        total++;
    }
    
    /* Test 2: -t without argument (should trigger error in option parsing) */
    snprintf(args, sizeof(args), "-t %s %s", gcda_files[0], gcda_files[1]);
    if (run_gcov_tool(args, &exit_code) == 0) {
        printf("Test 2 (-t no arg): exit code = %d\n", exit_code);
        total++;
    }
    
    /* Test 3: Unknown flag (should trigger default case and overlap_usage) */
    snprintf(args, sizeof(args), "-x %s %s", gcda_files[0], gcda_files[1]);
    if (run_gcov_tool(args, &exit_code) == 0) {
        printf("Test 3 (unknown flag -x): exit code = %d\n", exit_code);
        total++;
    }
    
    /* Test 4: Repeated flags */
    snprintf(args, sizeof(args), "-v -v -v %s %s", gcda_files[0], gcda_files[1]);
    if (run_gcov_tool(args, &exit_code) == 0) {
        printf("Test 4 (repeated -v): exit code = %d\n", exit_code);
        total++;
    }
    
    /* Test 5: No input files */
    if (run_gcov_tool("-v -f", &exit_code) == 0) {
        printf("Test 5 (no input files): exit code = %d\n", exit_code);
        total++;
    }
    
    /* Test 6: Single input file (overlap needs at least 2) */
    snprintf(args, sizeof(args), "-v %s", gcda_files[0]);
    if (run_gcov_tool(args, &exit_code) == 0) {
        printf("Test 6 (single file): exit code = %d\n", exit_code);
        total++;
    }
    
    printf("Edge cases: %d tests executed\n\n", total);
}

/**
 * Test permutations of flag order
 */
void test_permutations(void) {
    int exit_code;
    int passed = 0, total = 0;
    
    printf("\n=== Testing Flag Order Permutations ===\n");
    
    /* Different permutations of the same flags */
    const char *permutations[] = {
        "-v -f -F -o -h -t 0.8",
        "-t 0.8 -h -o -F -f -v",
        "-f -v -F -h -o -t 0.8",
        "-o -h -t 0.8 -v -f -F",
        "-h -t 0.8 -o -F -v -f",
        "-F -o -v -h -f -t 0.8",
        "-t 0.8 -v -h -F -o -f"
    };
    
    int num_perms = sizeof(permutations) / sizeof(permutations[0]);
    
    for (int i = 0; i < num_perms; i++) {
        char args[512];
        snprintf(args, sizeof(args), "%s %s %s", 
                 permutations[i], gcda_files[0], gcda_files[1]);
        
        if (run_gcov_tool(args, &exit_code) == 0) {
            printf("Permutation %d: exit code = %d\n", i + 1, exit_code);
            total++;
            if (exit_code == 0) passed++;
        }
    }
    
    printf("Permutations: %d/%d passed\n\n", passed, total);
}

/**
 * Main test suite runner
 */
void run_test_suite(void) {
    printf("Starting gcov-tool overlap argument parsing tests\n");
    printf("================================================\n");
    
    /* Setup */
    if (create_temp_directory() != 0) {
        fprintf(stderr, "Failed to create temp directory\n");
        return;
    }
    
    if (compile_test_program() != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        cleanup();
        return;
    }
    
    if (generate_gcda_files() != 0) {
        fprintf(stderr, "Failed to generate gcda files\n");
        cleanup();
        return;
    }
    
    /* Run tests */
    test_flag_combinations();
    test_edge_cases();
    test_permutations();
    
    /* Additional test: Use absolute paths */
    printf("\n=== Testing with Absolute Paths ===\n");
    char abs_args[512];
    char cwd[256];
    
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        char abs_path1[512], abs_path2[512];
        snprintf(abs_path1, sizeof(abs_path1), "%s/%s", cwd, gcda_files[0]);
        snprintf(abs_path2, sizeof(abs_path2), "%s/%s", cwd, gcda_files[1]);
        
        snprintf(abs_args, sizeof(abs_args), "-v -f -F %s %s", abs_path1, abs_path2);
        
        int exit_code;
        if (run_gcov_tool(abs_args, &exit_code) == 0) {
            printf("Absolute paths test: exit code = %d\n", exit_code);
        }
    }
    
    /* Cleanup */
    cleanup();
    
    printf("\n================================================\n");
    printf("Test suite completed\n");
    printf("Note: To get coverage for gcov-tool.cc lines 534-554,\n");
    printf("      gcov-tool must be built with --enable-coverage\n");
    printf("      Run: gcov gcov-tool.cc after executing these tests\n");
}

/**
 * Main function
 */
int main(int argc, char *argv[]) {
    /* Register cleanup handler */
    atexit(cleanup);
    
    /* Run the test suite */
    run_test_suite();
    
    return 0;
}
