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

/* Structure to hold test case information */
typedef struct {
    const char *description;
    const char *args;
    int expected_exit_code;
    int should_succeed;
} test_case_t;

/* Global variables for cleanup */
static char temp_dir[256] = "";
static char test_prog_path[512] = "";
static char gcda_files[MAX_FILES][512];
static int num_gcda_files = 0;

/* Function prototypes */
static int create_temp_directory(void);
static int compile_test_program(void);
static int generate_gcda_files(void);
static void cleanup(void);
static int run_gcov_tool(const char *args, const char *description);
static int execute_command(const char *cmd);
static void print_test_result(const char *desc, int passed);

/**
 * Create a simple C program for GCOV instrumentation
 */
static const char *test_program = 
"#include <stdio.h>\n"
"int main() {\n"
"    int x = 0;\n"
"    for (int i = 0; i < 10; i++) {\n"
"        if (i % 2 == 0) {\n"
"            x += i;\n"
"        } else {\n"
"            x -= i;\n"
"        }\n"
"    }\n"
"    printf(\"Result: %d\\n\", x);\n"
"    return 0;\n"
"}\n";

/**
 * Create temporary directory for test files
 */
static int create_temp_directory(void) {
    char *dir = mkdtemp(TEMP_DIR_PATTERN);
    if (!dir) {
        perror("Failed to create temporary directory");
        return 0;
    }
    strcpy(temp_dir, dir);
    printf("Created temporary directory: %s\n", temp_dir);
    return 1;
}

/**
 * Compile the test program with GCOV instrumentation
 */
static int compile_test_program(void) {
    char compile_cmd[1024];
    int result;
    
    /* Write test program to file */
    FILE *fp = fopen("test_prog.c", "w");
    if (!fp) {
        perror("Failed to create test_prog.c");
        return 0;
    }
    fprintf(fp, "%s", test_program);
    fclose(fp);
    
    /* Compile with GCOV instrumentation */
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -O0 test_prog.c -o test_prog");
    
    result = system(compile_cmd);
    if (result != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return 0;
    }
    
    /* Move to temp directory */
    snprintf(test_prog_path, sizeof(test_prog_path), "%s/test_prog", temp_dir);
    rename("test_prog", test_prog_path);
    rename("test_prog.c", "test_prog.c.bak");
    
    return 1;
}

/**
 * Generate multiple .gcda files by running the test program
 */
static int generate_gcda_files(void) {
    char cmd[512];
    int i;
    
    /* Create first .gcda file */
    snprintf(cmd, sizeof(cmd), "%s > /dev/null 2>&1", test_prog_path);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to run test program\n");
        return 0;
    }
    
    /* Copy .gcda file to create multiple versions */
    for (i = 0; i < 3; i++) {
        snprintf(gcda_files[i], sizeof(gcda_files[i]), 
                 "%s/test_prog_%d.gcda", temp_dir, i);
        snprintf(cmd, sizeof(cmd), "cp test_prog.gcda %s 2>/dev/null", gcda_files[i]);
        system(cmd);
        
        /* Also create .gcno file */
        snprintf(cmd, sizeof(cmd), "cp test_prog.gcno %s/test_prog_%d.gcno 2>/dev/null", 
                 temp_dir, i);
        system(cmd);
        
        num_gcda_files++;
    }
    
    /* Create a .gcda file with different path for fullname testing */
    snprintf(gcda_files[3], sizeof(gcda_files[3]), 
             "%s/../gcov_test_parent/test_prog_3.gcda", temp_dir);
    snprintf(cmd, sizeof(cmd), "mkdir -p %s/../gcov_test_parent 2>/dev/null", temp_dir);
    system(cmd);
    snprintf(cmd, sizeof(cmd), "cp test_prog.gcda %s 2>/dev/null", gcda_files[3]);
    system(cmd);
    num_gcda_files++;
    
    return 1;
}

/**
 * Clean up temporary files
 */
static void cleanup(void) {
    char cmd[1024];
    
    /* Remove temporary directory */
    if (strlen(temp_dir) > 0) {
        snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
        system(cmd);
    }
    
    /* Clean up any remaining files */
    remove("test_prog.c");
    remove("test_prog.c.bak");
    remove("test_prog.gcda");
    remove("test_prog.gcno");
    remove("test_prog");
    
    /* Clean up parent directory */
    snprintf(cmd, sizeof(cmd), "rm -rf %s/../gcov_test_parent", temp_dir);
    system(cmd);
}

/**
 * Execute a command and return exit code
 */
static int execute_command(const char *cmd) {
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/**
 * Run gcov-tool with specific arguments
 */
static int run_gcov_tool(const char *args, const char *description) {
    char cmd[MAX_CMD_LEN];
    int exit_code;
    
    /* Build the command */
    if (num_gcda_files >= 2) {
        snprintf(cmd, sizeof(cmd), "gcov-tool overlap %s %s %s", 
                 args, gcda_files[0], gcda_files[1]);
    } else {
        snprintf(cmd, sizeof(cmd), "gcov-tool overlap %s", args);
    }
    
    printf("Running: %s\n", cmd);
    exit_code = execute_command(cmd);
    
    printf("Exit code: %d\n", exit_code);
    return exit_code;
}

/**
 * Print test result
 */
static void print_test_result(const char *desc, int passed) {
    printf("[%s] %s\n", passed ? "PASS" : "FAIL", desc);
}

/**
 * Main test driver
 */
int main(int argc, char *argv[]) {
    int tests_passed = 0;
    int total_tests = 0;
    int exit_code;
    
    printf("=== GCOV-TOOL Overlap Parser Test ===\n\n");
    
    /* Set up test environment */
    if (!create_temp_directory()) {
        fprintf(stderr, "Failed to set up test environment\n");
        return 1;
    }
    
    /* Change to temp directory */
    if (chdir(temp_dir) != 0) {
        perror("Failed to change to temp directory");
        cleanup();
        return 1;
    }
    
    /* Compile test program and generate .gcda files */
    if (!compile_test_program()) {
        fprintf(stderr, "Failed to compile test program\n");
        cleanup();
        return 1;
    }
    
    if (!generate_gcda_files()) {
        fprintf(stderr, "Failed to generate .gcda files\n");
        cleanup();
        return 1;
    }
    
    printf("\n=== Testing Basic Flag Combinations ===\n\n");
    
    /* Test 1: All flags together (covers all case statements) */
    total_tests++;
    exit_code = run_gcov_tool("-v -f -F -o -h -t 0.75", 
                             "All flags combined");
    print_test_result("All flags: -v -f -F -o -h -t 0.75", exit_code == 0);
    if (exit_code == 0) tests_passed++;
    
    /* Test 2: Different order of flags */
    total_tests++;
    exit_code = run_gcov_tool("-t 0.5 -h -o -F -f -v", 
                             "Flags in reverse order");
    print_test_result("Flags in reverse order", exit_code == 0);
    if (exit_code == 0) tests_passed++;
    
    /* Test 3: Flags with different threshold values */
    total_tests++;
    exit_code = run_gcov_tool("-v -t 0.0", "Threshold 0.0");
    print_test_result("Threshold 0.0", exit_code == 0);
    if (exit_code == 0) tests_passed++;
    
    total_tests++;
    exit_code = run_gcov_tool("-v -t 1.0", "Threshold 1.0");
    print_test_result("Threshold 1.0", exit_code == 0);
    if (exit_code == 0) tests_passed++;
    
    total_tests++;
    exit_code = run_gcov_tool("-v -t 0.25", "Threshold 0.25");
    print_test_result("Threshold 0.25", exit_code == 0);
    if (exit_code == 0) tests_passed++;
    
    /* Test 4: Individual flags */
    total_tests++;
    exit_code = run_gcov_tool("-v", "Verbose flag only");
    print_test_result("Verbose flag only", exit_code == 0);
    if (exit_code == 0) tests_passed++;
    
    total_tests++;
    exit_code = run_gcov_tool("-f", "Function level flag only");
    print_test_result("Function level flag only", exit_code == 0);
    if (exit_code == 0) tests_passed++;
    
    total_tests++;
    exit_code = run_gcov_tool("-F", "Fullname flag only");
    print_test_result("Fullname flag only", exit_code == 0);
    if (exit_code == 0) tests_passed++;
    
    total_tests++;
    exit_code = run_gcov_tool("-o", "Object level flag only");
    print_test_result("Object level flag only", exit_code == 0);
    if (exit_code == 0) tests_passed++;
    
    total_tests++;
    exit_code = run_gcov_tool("-h", "Hot only flag only");
    print_test_result("Hot only flag only", exit_code == 0);
    if (exit_code == 0) tests_passed++;
    
    /* Test 5: Combined subsets */
    total_tests++;
    exit_code = run_gcov_tool("-v -f -t 0.8", "Verbose + function + threshold");
    print_test_result("Verbose + function + threshold", exit_code == 0);
    if (exit_code == 0) tests_passed++;
    
    total_tests++;
    exit_code = run_gcov_tool("-F -o -h", "Fullname + object + hot only");
    print_test_result("Fullname + object + hot only", exit_code == 0);
    if (exit_code == 0) tests_passed++;
    
    printf("\n=== Testing Edge Cases ===\n\n");
    
    /* Test 6: Invalid threshold (should trigger atof but may fail) */
    total_tests++;
    exit_code = run_gcov_tool("-t not_a_number", "Invalid threshold value");
    print_test_result("Invalid threshold (non-numeric)", exit_code != 0);
    if (exit_code != 0) tests_passed++;
    
    /* Test 7: Missing threshold argument */
    total_tests++;
    exit_code = run_gcov_tool("-t", "Missing threshold argument");
    print_test_result("Missing threshold argument", exit_code != 0);
    if (exit_code != 0) tests_passed++;
    
    /* Test 8: Unknown flag (should trigger default case) */
    total_tests++;
    exit_code = run_gcov_tool("-x", "Unknown flag");
    print_test_result("Unknown flag -x", exit_code != 0);
    if (exit_code != 0) tests_passed++;
    
    /* Test 9: Repeated flags */
    total_tests++;
    exit_code = run_gcov_tool("-v -v -v", "Repeated verbose flags");
    print_test_result("Repeated verbose flags", exit_code == 0);
    if (exit_code == 0) tests_passed++;
    
    /* Test 10: Flags with multiple files */
    total_tests++;
    char multi_file_cmd[MAX_CMD_LEN];
    snprintf(multi_file_cmd, sizeof(multi_file_cmd),
             "gcov-tool overlap -v -f %s %s %s",
             gcda_files[0], gcda_files[1], gcda_files[2]);
    printf("Running: %s\n", multi_file_cmd);
    exit_code = execute_command(multi_file_cmd);
    print_test_result("Multiple input files (3 .gcda)", exit_code == 0);
    if (exit_code == 0) tests_passed++;
    total_tests++;
    
    /* Test 11: Flags with absolute paths (for -F testing) */
    total_tests++;
    char abs_path_cmd[MAX_CMD_LEN];
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    snprintf(abs_path_cmd, sizeof(abs_path_cmd),
             "gcov-tool overlap -F -v %s/%s %s/%s",
             cwd, gcda_files[0], cwd, gcda_files[1]);
    printf("Running: %s\n", abs_path_cmd);
    exit_code = execute_command(abs_path_cmd);
    print_test_result("Absolute paths with -F flag", exit_code == 0);
    if (exit_code == 0) tests_passed++;
    
    /* Test 12: Mixed valid and invalid flags */
    total_tests++;
    exit_code = run_gcov_tool("-v -x -f", "Mixed valid and invalid flags");
    print_test_result("Mixed valid/invalid flags", exit_code != 0);
    if (exit_code != 0) tests_passed++;
    
    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    printf("Success rate: %.1f%%\n", (float)tests_passed/total_tests * 100);
    
    /* Clean up */
    cleanup();
    
    /* Return non-zero if any required tests failed */
    if (tests_passed < total_tests) {
        printf("\nSome tests failed. Check gcov-tool output above.\n");
        return 1;
    }
    
    printf("\nAll tests completed successfully!\n");
    return 0;
}
