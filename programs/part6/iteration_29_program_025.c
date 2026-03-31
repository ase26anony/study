/**
 * test_gcov_tool_overlap.c
 * 
 * Test driver for gcov-tool overlap command line parsing.
 * Specifically targets the uncovered switch cases in parse_overlap_options.
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

/**
 * Structure to hold test case information
 */
typedef struct {
    const char *description;
    const char *args;
    int expected_exit_code;  // 0 for success, non-zero for expected failure
} test_case_t;

/**
 * Create a temporary directory for test files
 */
char* create_temp_dir() {
    char *template = strdup(TEMP_DIR_PREFIX);
    if (mkdtemp(template) == NULL) {
        perror("Failed to create temporary directory");
        free(template);
        return NULL;
    }
    return template;
}

/**
 * Remove a directory and all its contents
 */
void remove_temp_dir(const char *dir_path) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dir_path);
    system(cmd);
}

/**
 * Create a simple C program for GCOV instrumentation
 */
int create_test_program(const char *dir_path, const char *prog_name) {
    char path[MAX_CMD_LEN];
    FILE *fp;
    
    // Create source file
    snprintf(path, sizeof(path), "%s/%s.c", dir_path, prog_name);
    fp = fopen(path, "w");
    if (!fp) {
        perror("Failed to create test program source");
        return -1;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int helper(int x) {\n");
    fprintf(fp, "    if (x > 0) {\n");
    fprintf(fp, "        return x * 2;\n");
    fprintf(fp, "    } else {\n");
    fprintf(fp, "        return x + 1;\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "}\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int result = 0;\n");
    fprintf(fp, "    for (int i = 0; i < 10; i++) {\n");
    fprintf(fp, "        result += helper(i);\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    printf(\"Result: %%d\\n\", result);\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/**
 * Compile a program with GCOV instrumentation
 */
int compile_with_gcov(const char *dir_path, const char *prog_name) {
    char cmd[MAX_CMD_LEN];
    char src_path[MAX_CMD_LEN];
    char exe_path[MAX_CMD_LEN];
    
    snprintf(src_path, sizeof(src_path), "%s/%s.c", dir_path, prog_name);
    snprintf(exe_path, sizeof(exe_path), "%s/%s", dir_path, prog_name);
    
    // Compile with coverage flags
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s 2>&1",
             src_path, exe_path);
    
    printf("Compiling: %s\n", cmd);
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Compilation failed for %s\n", prog_name);
        return -1;
    }
    
    return 0;
}

/**
 * Run a program to generate .gcda files
 */
int run_program(const char *dir_path, const char *prog_name, int run_num) {
    char cmd[MAX_CMD_LEN];
    char exe_path[MAX_CMD_LEN];
    
    snprintf(exe_path, sizeof(exe_path), "%s/%s", dir_path, prog_name);
    
    // Set GCOV_PREFIX if we want separate .gcda files for different runs
    if (run_num > 0) {
        char prefix[MAX_CMD_LEN];
        snprintf(prefix, sizeof(prefix), "%s/run%d", dir_path, run_num);
        setenv("GCOV_PREFIX", prefix, 1);
    }
    
    snprintf(cmd, sizeof(cmd), "%s", exe_path);
    printf("Running: %s\n", cmd);
    int result = system(cmd);
    
    // Clear GCOV_PREFIX for next runs
    unsetenv("GCOV_PREFIX");
    
    return result;
}

/**
 * Execute a gcov-tool command and return exit code
 */
int run_gcov_tool(const char *command) {
    printf("\nExecuting: %s\n", command);
    
    int status = system(command);
    
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        printf("Exit code: %d\n", exit_code);
        return exit_code;
    } else {
        printf("Command terminated abnormally\n");
        return -1;
    }
}

/**
 * Copy .gcda files to create multiple test files
 */
int copy_gcda_files(const char *dir_path, const char *prog_name, int num_copies) {
    char src[MAX_CMD_LEN];
    char dst[MAX_CMD_LEN];
    
    for (int i = 1; i <= num_copies; i++) {
        snprintf(src, sizeof(src), "%s/%s.gcda", dir_path, prog_name);
        snprintf(dst, sizeof(dst), "%s/%s_run%d.gcda", dir_path, prog_name, i);
        
        char cmd[MAX_CMD_LEN];
        snprintf(cmd, sizeof(cmd), "cp %s %s 2>/dev/null", src, dst);
        
        if (system(cmd) != 0) {
            fprintf(stderr, "Failed to copy .gcda file %d\n", i);
            return -1;
        }
    }
    
    return 0;
}

/**
 * Main test driver
 */
int main(int argc, char *argv[]) {
    char *temp_dir = NULL;
    char gcov_tool_path[MAX_CMD_LEN] = "./gcov-tool";
    int overall_result = 0;
    
    // Allow overriding gcov-tool path
    if (argc > 1) {
        strncpy(gcov_tool_path, argv[1], sizeof(gcov_tool_path) - 1);
    }
    
    printf("=== GCOV-TOOL Overlap Parser Test ===\n");
    printf("Using gcov-tool: %s\n", gcov_tool_path);
    
    // Create temporary directory
    temp_dir = create_temp_dir();
    if (!temp_dir) {
        fprintf(stderr, "Failed to create temporary directory\n");
        return 1;
    }
    printf("Created temp directory: %s\n", temp_dir);
    
    // Create and compile test program
    if (create_test_program(temp_dir, "testprog") != 0) {
        remove_temp_dir(temp_dir);
        free(temp_dir);
        return 1;
    }
    
    if (compile_with_gcov(temp_dir, "testprog") != 0) {
        remove_temp_dir(temp_dir);
        free(temp_dir);
        return 1;
    }
    
    // Run program multiple times to generate different .gcda files
    printf("\n--- Generating GCOV data files ---\n");
    for (int i = 0; i < 3; i++) {
        if (run_program(temp_dir, "testprog", i) != 0) {
            fprintf(stderr, "Failed to run program iteration %d\n", i);
        }
        // Small delay to ensure different timestamps
        sleep(1);
    }
    
    // Create copies of .gcda files for overlap analysis
    copy_gcda_files(temp_dir, "testprog", 3);
    
    // Define test cases targeting the uncovered switch cases
    test_case_t test_cases[] = {
        // Basic tests for each individual flag
        {"Test -v flag (verbose)", 
         "%s overlap -v %s/testprog.gcda %s/testprog_run1.gcda", 0},
        
        {"Test -f flag (function level)", 
         "%s overlap -f %s/testprog.gcda %s/testprog_run1.gcda", 0},
        
        {"Test -F flag (full filename)", 
         "%s overlap -F %s/testprog.gcda %s/testprog_run1.gcda", 0},
        
        {"Test -o flag (object level)", 
         "%s overlap -o %s/testprog.gcda %s/testprog_run1.gcda", 0},
        
        {"Test -h flag (hot only)", 
         "%s overlap -h %s/testprog.gcda %s/testprog_run1.gcda", 0},
        
        {"Test -t flag with value 0.5", 
         "%s overlap -t 0.5 %s/testprog.gcda %s/testprog_run1.gcda", 0},
        
        {"Test -t flag with value 1.0", 
         "%s overlap -t 1.0 %s/testprog.gcda %s/testprog_run1.gcda", 0},
        
        {"Test -t flag with value 0.75", 
         "%s overlap -t 0.75 %s/testprog.gcda %s/testprog_run1.gcda", 0},
        
        // Combination tests - all flags together (triggers all case statements)
        {"Test all flags combined", 
         "%s overlap -v -f -F -o -h -t 0.8 %s/testprog.gcda %s/testprog_run1.gcda %s/testprog_run2.gcda", 0},
        
        // Different permutations of flags
        {"Test flags in different order 1", 
         "%s overlap -t 0.3 -h -o -F -f -v %s/testprog.gcda %s/testprog_run1.gcda", 0},
        
        {"Test flags in different order 2", 
         "%s overlap -F -o -t 0.6 -v -f -h %s/testprog.gcda %s/testprog_run1.gcda", 0},
        
        // Edge cases
        {"Test invalid argument for -t (should fail)", 
         "%s overlap -t not_a_number %s/testprog.gcda", 1},
        
        {"Test missing argument for -t (should fail)", 
         "%s overlap -t %s/testprog.gcda", 1},
        
        {"Test unknown flag -x (should trigger default case)", 
         "%s overlap -x %s/testprog.gcda", 1},
        
        {"Test repeated flags", 
         "%s overlap -v -v -f -f -t 0.5 -t 0.7 %s/testprog.gcda %s/testprog_run1.gcda", 0},
        
        {"Test with absolute paths", 
         "%s overlap -v -f %s/testprog.gcda %s/testprog_run1.gcda", 0},
        
        {"Test with multiple input files (3 files)", 
         "%s overlap -v -f -t 0.5 %s/testprog.gcda %s/testprog_run1.gcda %s/testprog_run2.gcda %s/testprog_run3.gcda", 0},
        
        {"Test with mixed flags and files", 
         "%s overlap -v %s/testprog.gcda -f %s/testprog_run1.gcda -t 0.5 %s/testprog_run2.gcda", 0},
        
        // Test with .gcno files as well
        {"Test with .gcno files", 
         "%s overlap -v -f %s/testprog.gcno %s/testprog.gcda", 0},
        
        {NULL, NULL, 0}  // Sentinel
    };
    
    printf("\n--- Running gcov-tool overlap tests ---\n");
    
    // Execute all test cases
    int num_tests = sizeof(test_cases) / sizeof(test_case_t) - 1;
    int passed = 0;
    int failed = 0;
    
    for (int i = 0; i < num_tests; i++) {
        char command[MAX_CMD_LEN];
        snprintf(command, sizeof(command), 
                test_cases[i].args, 
                gcov_tool_path, temp_dir, temp_dir, temp_dir, temp_dir);
        
        printf("\nTest %d: %s\n", i + 1, test_cases[i].description);
        printf("Command: %s\n", command);
        
        int exit_code = run_gcov_tool(command);
        
        if ((test_cases[i].expected_exit_code == 0 && exit_code == 0) ||
            (test_cases[i].expected_exit_code != 0 && exit_code != 0)) {
            printf("✓ PASS\n");
            passed++;
        } else {
            printf("✗ FAIL - Expected exit code %d, got %d\n", 
                   test_cases[i].expected_exit_code, exit_code);
            failed++;
            overall_result = 1;
        }
    }
    
    // Additional test: Create a script to test many permutations
    printf("\n--- Testing flag permutations ---\n");
    
    // Define all flags to test
    const char *flags[] = {"-v", "-f", "-F", "-o", "-h", "-t 0.5"};
    const char *flag_names[] = {"verbose", "func_level", "fullname", "obj_level", "hot_only", "threshold"};
    int num_flags = 6;
    
    // Test each flag individually and in combinations
    for (int mask = 1; mask < (1 << num_flags); mask++) {
        char command[MAX_CMD_LEN];
        char flag_desc[256] = "";
        
        // Start building command
        snprintf(command, sizeof(command), "%s overlap", gcov_tool_path);
        
        // Add flags based on bitmask
        for (int j = 0; j < num_flags; j++) {
            if (mask & (1 << j)) {
                strcat(command, " ");
                strcat(command, flags[j]);
                
                if (strlen(flag_desc) > 0) strcat(flag_desc, ", ");
                strcat(flag_desc, flag_names[j]);
            }
        }
        
        // Add input files
        char files_part[MAX_CMD_LEN];
        snprintf(files_part, sizeof(files_part), " %s/testprog.gcda %s/testprog_run1.gcda",
                temp_dir, temp_dir);
        strcat(command, files_part);
        
        printf("\nTesting flags: %s\n", flag_desc);
        
        int exit_code = run_gcov_tool(command);
        if (exit_code == 0) {
            printf("✓ Success\n");
        } else {
            printf("✗ Failed with exit code %d\n", exit_code);
        }
    }
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", num_tests);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    printf("Flag permutations tested: %d\n", (1 << num_flags) - 1);
    
    // Cleanup
    printf("\nCleaning up temporary directory: %s\n", temp_dir);
    remove_temp_dir(temp_dir);
    free(temp_dir);
    
    if (overall_result == 0) {
        printf("\n✓ All tests completed successfully\n");
    } else {
        printf("\n✗ Some tests failed\n");
    }
    
    return overall_result;
}
