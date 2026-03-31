/**
 * test_gcov_tool_overlap.c
 * 
 * Test driver to exercise the uncovered lines in gcov-tool.cc's
 * parse_overlap_options function (lines 534-554).
 * 
 * Compile with: gcc -O0 -Wall -Wextra test_gcov_tool_overlap.c -o test_gcov_tool_overlap
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#define MAX_PATH 1024
#define MAX_CMD 4096
#define TEMP_DIR_TEMPLATE "/tmp/gcov_test_XXXXXX"

typedef struct {
    char *command;
    int expected_exit_code;
    char *description;
} test_case_t;

/* Global variables for cleanup */
static char temp_dir[MAX_PATH] = "";
static char test_prog_path[MAX_PATH] = "";
static char gcda1_path[MAX_PATH] = "";
static char gcda2_path[MAX_PATH] = "";

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

/* Clean up temporary files and directory */
void cleanup() {
    if (temp_dir[0] != '\0') {
        char cmd[MAX_CMD];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
        system(cmd);
    }
}

/* Create a temporary directory */
int create_temp_dir() {
    char *dir = mkdtemp(TEMP_DIR_TEMPLATE);
    if (!dir) {
        perror("mkdtemp failed");
        return 0;
    }
    strncpy(temp_dir, dir, sizeof(temp_dir) - 1);
    return 1;
}

/* Write the test program to a file */
int write_test_program() {
    snprintf(test_prog_path, sizeof(test_prog_path), "%s/test_prog.c", temp_dir);
    
    FILE *fp = fopen(test_prog_path, "w");
    if (!fp) {
        perror("Failed to write test program");
        return 0;
    }
    
    fputs(test_program, fp);
    fclose(fp);
    return 1;
}

/* Compile the test program with GCOV instrumentation */
int compile_test_program() {
    char cmd[MAX_CMD];
    int status;
    
    /* Compile with coverage flags */
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s/test_prog %s/test_prog.c",
             temp_dir, temp_dir);
    
    status = system(cmd);
    if (WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Compilation failed: %s\n", cmd);
        return 0;
    }
    
    /* Create paths for gcda files */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_prog.gcda", temp_dir);
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_prog2.gcda", temp_dir);
    
    return 1;
}

/* Run the test program to generate gcda files */
int generate_gcda_files() {
    char cmd[MAX_CMD];
    int status;
    
    /* First run - generate first gcda file */
    snprintf(cmd, sizeof(cmd), "%s/test_prog > /dev/null 2>&1", temp_dir);
    status = system(cmd);
    if (WEXITSTATUS(status) != 0) {
        fprintf(stderr, "First program run failed\n");
        return 0;
    }
    
    /* Copy the gcda file to create a second one with different data */
    char copy_cmd[MAX_CMD];
    snprintf(copy_cmd, sizeof(copy_cmd), "cp %s/test_prog.gcda %s/test_prog2.gcda", 
             temp_dir, temp_dir);
    system(copy_cmd);
    
    /* Run again to modify the second gcda file */
    snprintf(cmd, sizeof(cmd), "cd %s && ./test_prog > /dev/null 2>&1", temp_dir);
    status = system(cmd);
    
    return 1;
}

/* Run a test case and check the result */
int run_test_case(const char *command, int expected_exit_code, const char *description) {
    printf("Test: %s\n", description);
    printf("Command: %s\n", command);
    
    int status = system(command);
    int exit_code = WEXITSTATUS(status);
    
    printf("Exit code: %d (expected: %d) - %s\n\n", 
           exit_code, expected_exit_code,
           (exit_code == expected_exit_code) ? "PASS" : "FAIL");
    
    return (exit_code == expected_exit_code);
}

/* Execute gcov-tool with given arguments */
int run_gcov_tool(const char *args, int expected_exit_code, const char *description) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "gcov-tool %s", args);
    return run_test_case(cmd, expected_exit_code, description);
}

int main(int argc, char *argv[]) {
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("=== Testing gcov-tool overlap argument parsing ===\n\n");
    
    /* Set up cleanup handler */
    atexit(cleanup);
    
    /* Create temporary workspace */
    if (!create_temp_dir()) {
        return 1;
    }
    
    printf("Created temporary directory: %s\n", temp_dir);
    
    /* Write, compile, and run test program */
    if (!write_test_program()) {
        return 1;
    }
    
    if (!compile_test_program()) {
        return 1;
    }
    
    if (!generate_gcda_files()) {
        return 1;
    }
    
    printf("Generated GCOV data files:\n");
    printf("  %s\n", gcda1_path);
    printf("  %s\n", gcda2_path);
    printf("\n");
    
    /* Define test cases for the uncovered switch cases */
    test_case_t test_cases[] = {
        /* Basic tests for each individual flag */
        {"overlap -v", 0, "Test -v flag (verbose mode)"},
        {"overlap -f", 0, "Test -f flag (function level)"},
        {"overlap -F", 0, "Test -F flag (use fullname)"},
        {"overlap -o", 0, "Test -o flag (object level)"},
        {"overlap -h", 0, "Test -h flag (hot only)"},
        {"overlap -t 0.5", 0, "Test -t flag with threshold 0.5"},
        
        /* Combined flags - testing all uncovered cases in one command */
        {"overlap -v -f -F -o -h -t 0.75", 0, 
         "Test all flags combined: -v -f -F -o -h -t 0.75"},
        
        /* Different orders of flags */
        {"overlap -t 1.0 -h -o -F -f -v", 0, 
         "Test flags in reverse order"},
        {"overlap -f -v -t 0.25 -h -F -o", 0, 
         "Test flags in mixed order"},
        
        /* With actual gcda files */
        {"overlap -v -f -F -o -h -t 0.5 test_prog.gcda test_prog2.gcda", 0,
         "Test with gcda files (relative path)"},
        
        /* Edge cases for -t flag */
        {"overlap -t 0", 0, "Test -t with threshold 0"},
        {"overlap -t 1.0", 0, "Test -t with threshold 1.0"},
        {"overlap -t 100.5", 0, "Test -t with large threshold"},
        {"overlap -t 0.001", 0, "Test -t with small threshold"},
        
        /* Invalid cases that should trigger error handling */
        {"overlap -t", 1, "Test -t without argument (should fail)"},
        {"overlap -t not_a_number", 1, 
         "Test -t with non-numeric argument (should fail)"},
        {"overlap -x", 1, "Test unknown flag -x (should trigger default case)"},
        
        /* Repeated flags */
        {"overlap -v -v -v", 0, "Test repeated -v flags"},
        {"overlap -f -f -t 0.5 -t 0.7", 0, "Test repeated -f and -t flags"},
        
        /* Flags with gcda files in different positions */
        {"overlap test_prog.gcda -v -f test_prog2.gcda", 0,
         "Test flags interspersed with filenames"},
        
        /* Using absolute paths */
        {"", 0, "Placeholder for absolute path test"},
        
        /* Empty overlap command */
        {"overlap", 0, "Test overlap with no flags"},
        
        /* NULL terminator */
        {NULL, 0, ""}
    };
    
    /* Calculate number of tests */
    for (int i = 0; test_cases[i].command != NULL; i++) {
        total_tests++;
    }
    
    /* Change to temp directory for relative path tests */
    char original_dir[MAX_PATH];
    if (getcwd(original_dir, sizeof(original_dir)) == NULL) {
        perror("getcwd failed");
        return 1;
    }
    
    if (chdir(temp_dir) != 0) {
        perror("chdir failed");
        return 1;
    }
    
    /* Run all test cases */
    for (int i = 0; test_cases[i].command != NULL; i++) {
        char full_cmd[MAX_CMD];
        
        /* Special handling for absolute path test */
        if (strcmp(test_cases[i].description, "Placeholder for absolute path test") == 0) {
            char abs_path1[MAX_PATH], abs_path2[MAX_PATH];
            snprintf(abs_path1, sizeof(abs_path1), "%s/test_prog.gcda", temp_dir);
            snprintf(abs_path2, sizeof(abs_path2), "%s/test_prog2.gcda", temp_dir);
            snprintf(full_cmd, sizeof(full_cmd), 
                     "overlap -v -f %s %s", abs_path1, abs_path2);
            
            if (run_gcov_tool(full_cmd, test_cases[i].expected_exit_code,
                            "Test with absolute paths")) {
                passed_tests++;
            }
        } else {
            if (run_gcov_tool(test_cases[i].command, test_cases[i].expected_exit_code,
                            test_cases[i].description)) {
                passed_tests++;
            }
        }
        
        /* Small delay to avoid overwhelming the system */
        usleep(10000);
    }
    
    /* Change back to original directory */
    chdir(original_dir);
    
    /* Additional permutation tests */
    printf("\n=== Testing flag permutations ===\n\n");
    
    /* Generate permutations of the 6 main flags */
    char *flags[] = {"-v", "-f", "-F", "-o", "-h", "-t 0.5"};
    int num_flags = 6;
    
    /* Test several permutations */
    int permutations[][6] = {
        {0, 1, 2, 3, 4, 5},  /* Original order */
        {5, 4, 3, 2, 1, 0},  /* Reverse order */
        {2, 0, 4, 1, 5, 3},  /* Mixed order 1 */
        {4, 2, 0, 5, 3, 1},  /* Mixed order 2 */
        {1, 3, 5, 0, 2, 4},  /* Mixed order 3 */
    };
    
    int num_permutations = sizeof(permutations) / sizeof(permutations[0]);
    
    for (int p = 0; p < num_permutations; p++) {
        char perm_cmd[MAX_CMD] = "overlap ";
        char temp[MAX_CMD];
        
        for (int f = 0; f < num_flags; f++) {
            strcpy(temp, perm_cmd);
            snprintf(perm_cmd, sizeof(perm_cmd), "%s %s", temp, flags[permutations[p][f]]);
        }
        
        /* Add gcda files */
        strcpy(temp, perm_cmd);
        snprintf(perm_cmd, sizeof(perm_cmd), "%s %s/test_prog.gcda %s/test_prog2.gcda",
                 temp, temp_dir, temp_dir);
        
        char desc[100];
        snprintf(desc, sizeof(desc), "Flag permutation %d", p + 1);
        
        if (run_gcov_tool(perm_cmd, 0, desc)) {
            passed_tests++;
        }
        total_tests++;
        
        usleep(5000);
    }
    
    /* Test with environment variable to prevent optimization */
    printf("\n=== Testing with environment variables ===\n\n");
    
    /* Use environment variable to construct command */
    char *gcov_tool_flags = getenv("GCOV_TOOL_FLAGS");
    if (gcov_tool_flags) {
        char env_cmd[MAX_CMD];
        snprintf(env_cmd, sizeof(env_cmd), "overlap %s %s/test_prog.gcda",
                 gcov_tool_flags, temp_dir);
        
        if (run_gcov_tool(env_cmd, 0, "Test with flags from environment variable")) {
            passed_tests++;
        }
        total_tests++;
    } else {
        /* Set and test with environment variable */
        setenv("GCOV_TOOL_FLAGS", "-v -f -t 0.8", 1);
        char env_cmd[MAX_CMD];
        snprintf(env_cmd, sizeof(env_cmd), "overlap $GCOV_TOOL_FLAGS %s/test_prog.gcda",
                 temp_dir);
        
        /* Expand environment variable */
        char expanded_cmd[MAX_CMD];
        snprintf(expanded_cmd, sizeof(expanded_cmd), "overlap -v -f -t 0.8 %s/test_prog.gcda",
                 temp_dir);
        
        if (run_gcov_tool(expanded_cmd, 0, "Test with hardcoded env-like flags")) {
            passed_tests++;
        }
        total_tests++;
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Total tests run: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", total_tests - passed_tests);
    printf("Success rate: %.1f%%\n", (passed_tests * 100.0) / total_tests);
    
    printf("\nTemporary directory (for inspection): %s\n", temp_dir);
    printf("To clean up manually: rm -rf %s\n", temp_dir);
    
    return (passed_tests == total_tests) ? 0 : 1;
}
