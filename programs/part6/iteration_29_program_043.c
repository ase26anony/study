/**
 * test_gcov_tool_overlap.c
 * 
 * Test driver for gcov-tool overlap command parsing.
 * Exercises the specific switch cases in parse_overlap_options function.
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

/* Global variables to track test results */
static int tests_passed = 0;
static int tests_failed = 0;

/**
 * Execute a command and return its exit status.
 * Captures stderr to avoid cluttering test output.
 */
static int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    
    int status = system(cmd);
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/**
 * Create a simple C program, compile it with GCOV instrumentation,
 * run it to generate .gcda files.
 */
static int create_gcov_test_files(const char *temp_dir, char *gcda_files[], int num_files) {
    char src_path[256];
    char exe_path[256];
    char cmd[512];
    
    /* Create simple C source file */
    snprintf(src_path, sizeof(src_path), "%s/test_prog.c", temp_dir);
    FILE *src = fopen(src_path, "w");
    if (!src) {
        perror("Failed to create source file");
        return -1;
    }
    
    fprintf(src, 
        "#include <stdio.h>\n"
        "int func1() { return 1; }\n"
        "int func2() { return 2; }\n"
        "int main() {\n"
        "    printf(\"Test program\\n\");\n"
        "    func1();\n"
        "    func2();\n"
        "    return 0;\n"
        "}\n");
    fclose(src);
    
    /* Compile with GCOV instrumentation */
    snprintf(exe_path, sizeof(exe_path), "%s/test_prog", temp_dir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s 2>/dev/null",
             exe_path, src_path);
    
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return -1;
    }
    
    /* Run program multiple times to generate different .gcda files */
    for (int i = 0; i < num_files; i++) {
        /* Run the program */
        snprintf(cmd, sizeof(cmd), "cd %s && ./test_prog >/dev/null 2>&1", temp_dir);
        system(cmd);
        
        /* Set unique name for each .gcda file */
        snprintf(gcda_files[i], 256, "%s/test_prog.gcda", temp_dir);
        
        /* For multiple files, we need to move/rename them */
        if (i < num_files - 1) {
            char new_name[256];
            snprintf(new_name, sizeof(new_name), "%s/test_prog_%d.gcda", temp_dir, i);
            rename(gcda_files[i], new_name);
            strcpy(gcda_files[i], new_name);
        }
    }
    
    return 0;
}

/**
 * Test basic flag combinations to trigger all case statements
 */
static void test_basic_flag_combinations(const char *gcov_tool_path, char *gcda_files[], int num_gcda_files) {
    const char *flag_combinations[] = {
        "-v -f -F -o -h -t 0.5",
        "-f -F -o -h -t 0.75 -v",
        "-t 1.0 -h -o -F -f -v",
        "-v -t 0.25",
        "-f -F",
        "-o -h",
        "-v",
        "-f",
        "-F",
        "-o",
        "-h",
        "-t 0.9",
    };
    
    int num_combinations = sizeof(flag_combinations) / sizeof(flag_combinations[0]);
    
    for (int i = 0; i < num_combinations; i++) {
        char cmd[MAX_CMD_LEN];
        
        /* Build file list */
        char file_list[512] = "";
        for (int j = 0; j < num_gcda_files && j < 2; j++) {
            strcat(file_list, " ");
            strcat(file_list, gcda_files[j]);
        }
        
        /* Construct command */
        snprintf(cmd, sizeof(cmd), "%s overlap %s %s >/dev/null 2>&1",
                gcov_tool_path, flag_combinations[i], file_list);
        
        int status = execute_command(cmd);
        
        /* For valid flag combinations, we expect success (0) or maybe 1 if no data */
        if (status == 0 || status == 1) {
            printf("  PASS: Combination %d\n", i);
            tests_passed++;
        } else {
            printf("  FAIL: Combination %d (exit code: %d)\n", i, status);
            tests_failed++;
        }
    }
}

/**
 * Test edge cases and error conditions
 */
static void test_edge_cases(const char *gcov_tool_path, char *gcda_files[]) {
    char cmd[MAX_CMD_LEN];
    
    /* Test 1: Invalid argument for -t */
    snprintf(cmd, sizeof(cmd), "%s overlap -t not_a_number %s %s 2>/dev/null",
            gcov_tool_path, gcda_files[0], gcda_files[1]);
    int status = execute_command(cmd);
    /* Should fail with non-zero exit code */
    if (status != 0) {
        printf("  PASS: Invalid -t argument correctly rejected\n");
        tests_passed++;
    } else {
        printf("  FAIL: Invalid -t argument should have failed\n");
        tests_failed++;
    }
    
    /* Test 2: Missing argument for -t (at end) */
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f -t 2>/dev/null",
            gcov_tool_path);
    status = execute_command(cmd);
    if (status != 0) {
        printf("  PASS: Missing -t argument correctly rejected\n");
        tests_passed++;
    } else {
        printf("  FAIL: Missing -t argument should have failed\n");
        tests_failed++;
    }
    
    /* Test 3: Unknown flag to trigger default case */
    snprintf(cmd, sizeof(cmd), "%s overlap -x -v %s 2>/dev/null",
            gcov_tool_path, gcda_files[0]);
    status = execute_command(cmd);
    /* Unknown flag should trigger usage and exit with error */
    if (status != 0) {
        printf("  PASS: Unknown flag correctly rejected\n");
        tests_passed++;
    } else {
        printf("  FAIL: Unknown flag should have failed\n");
        tests_failed++;
    }
    
    /* Test 4: Repeated flags */
    snprintf(cmd, sizeof(cmd), "%s overlap -v -v -v %s 2>/dev/null",
            gcov_tool_path, gcda_files[0]);
    status = execute_command(cmd);
    if (status == 0 || status == 1) {
        printf("  PASS: Repeated flags handled\n");
        tests_passed++;
    } else {
        printf("  FAIL: Repeated flags caused error\n");
        tests_failed++;
    }
    
    /* Test 5: Very high threshold value */
    snprintf(cmd, sizeof(cmd), "%s overlap -t 1000.5 %s %s 2>/dev/null",
            gcov_tool_path, gcda_files[0], gcda_files[1]);
    status = execute_command(cmd);
    if (status == 0 || status == 1) {
        printf("  PASS: High threshold value accepted\n");
        tests_passed++;
    } else {
        printf("  FAIL: High threshold value rejected\n");
        tests_failed++;
    }
    
    /* Test 6: Negative threshold (edge case for atof) */
    snprintf(cmd, sizeof(cmd), "%s overlap -t -0.5 %s %s 2>/dev/null",
            gcov_tool_path, gcda_files[0], gcda_files[1]);
    status = execute_command(cmd);
    if (status == 0 || status == 1) {
        printf("  PASS: Negative threshold accepted\n");
        tests_passed++;
    } else {
        printf("  FAIL: Negative threshold rejected\n");
        tests_failed++;
    }
}

/**
 * Test with different file path formats
 */
static void test_file_paths(const char *gcov_tool_path, const char *temp_dir, char *gcda_files[]) {
    char cmd[MAX_CMD_LEN];
    
    /* Test with relative paths */
    char relative_path[256];
    snprintf(relative_path, sizeof(relative_path), "test_prog.gcda");
    
    /* Change to temp directory and use relative path */
    snprintf(cmd, sizeof(cmd), "cd %s && %s overlap -v %s 2>/dev/null",
            temp_dir, gcov_tool_path, relative_path);
    
    int status = execute_command(cmd);
    if (status == 0 || status == 1) {
        printf("  PASS: Relative path works\n");
        tests_passed++;
    } else {
        printf("  FAIL: Relative path failed\n");
        tests_failed++;
    }
    
    /* Test with absolute path */
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f %s 2>/dev/null",
            gcov_tool_path, gcda_files[0]);
    
    status = execute_command(cmd);
    if (status == 0 || status == 1) {
        printf("  PASS: Absolute path works\n");
        tests_passed++;
    } else {
        printf("  FAIL: Absolute path failed\n");
        tests_failed++;
    }
}

/**
 * Test permutations of flag order to ensure robust parsing
 */
static void test_flag_permutations(const char *gcov_tool_path, char *gcda_files[]) {
    /* Generate different permutations of the flags */
    const char *permutations[] = {
        "-v -f -F -o -h -t 0.5",
        "-t 0.5 -h -o -F -f -v",
        "-f -v -t 0.5 -h -o -F",
        "-F -o -h -t 0.5 -v -f",
        "-h -t 0.5 -v -f -F -o",
        "-o -F -f -v -t 0.5 -h",
    };
    
    int num_permutations = sizeof(permutations) / sizeof(permutations[0]);
    
    for (int i = 0; i < num_permutations; i++) {
        char cmd[MAX_CMD_LEN];
        
        snprintf(cmd, sizeof(cmd), "%s overlap %s %s %s >/dev/null 2>&1",
                gcov_tool_path, permutations[i], gcda_files[0], gcda_files[1]);
        
        int status = execute_command(cmd);
        
        if (status == 0 || status == 1) {
            printf("  PASS: Permutation %d\n", i);
            tests_passed++;
        } else {
            printf("  FAIL: Permutation %d (exit code: %d)\n", i, status);
            tests_failed++;
        }
    }
}

int main(int argc, char *argv[]) {
    char temp_dir_template[] = TEMP_DIR_PATTERN;
    char *temp_dir;
    char gcda_files[MAX_FILES][256];
    const char *gcov_tool_path;
    
    /* Determine gcov-tool path */
    if (argc > 1) {
        gcov_tool_path = argv[1];
    } else {
        /* Try to find it in PATH */
        gcov_tool_path = "gcov-tool";
    }
    
    printf("=== Testing gcov-tool overlap command parsing ===\n");
    printf("Using gcov-tool at: %s\n\n", gcov_tool_path);
    
    /* Create temporary directory */
    temp_dir = mkdtemp(temp_dir_template);
    if (!temp_dir) {
        perror("Failed to create temporary directory");
        return 1;
    }
    
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Create test GCOV data files */
    if (create_gcov_test_files(temp_dir, gcda_files, 3) != 0) {
        fprintf(stderr, "Failed to create GCOV test files\n");
        /* Clean up */
        char cleanup_cmd[256];
        snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
        system(cleanup_cmd);
        return 1;
    }
    
    printf("Created %d GCOV data files\n\n", 3);
    
    /* Run test suites */
    printf("1. Testing basic flag combinations:\n");
    test_basic_flag_combinations(gcov_tool_path, gcda_files, 3);
    printf("\n");
    
    printf("2. Testing flag permutations:\n");
    test_flag_permutations(gcov_tool_path, gcda_files);
    printf("\n");
    
    printf("3. Testing edge cases:\n");
    test_edge_cases(gcov_tool_path, gcda_files);
    printf("\n");
    
    printf("4. Testing file path formats:\n");
    test_file_paths(gcov_tool_path, temp_dir, gcda_files);
    printf("\n");
    
    /* Summary */
    printf("=== Test Summary ===\n");
    printf("Total tests passed: %d\n", tests_passed);
    printf("Total tests failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", 
           tests_passed * 100.0 / (tests_passed + tests_failed));
    
    /* Clean up */
    printf("\nCleaning up temporary directory: %s\n", temp_dir);
    char cleanup_cmd[256];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
    system(cleanup_cmd);
    
    return tests_failed > 0 ? 1 : 0;
}
