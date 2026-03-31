/**
 * test_gcov_tool_overlap.c
 * 
 * Test driver for gcov-tool overlap command-line parsing.
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
#define MAX_FILENAME 256
#define TEMP_DIR "/tmp/gcov_test_XXXXXX"

/* Global variables to track test results */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

/**
 * Execute a command and return its exit status.
 * Also captures stderr output for analysis.
 */
int execute_command(const char *cmd, char *output, size_t output_size) {
    FILE *fp;
    int status;
    
    if (output && output_size > 0) {
        output[0] = '\0';
        
        /* Use popen to capture stderr */
        char cmd_with_stderr[MAX_CMD_LEN];
        snprintf(cmd_with_stderr, sizeof(cmd_with_stderr), "%s 2>&1", cmd);
        
        fp = popen(cmd_with_stderr, "r");
        if (fp == NULL) {
            return -1;
        }
        
        /* Read output */
        size_t total_read = 0;
        while (fgets(output + total_read, output_size - total_read, fp) != NULL) {
            total_read = strlen(output);
            if (total_read >= output_size - 1) {
                break;
            }
        }
        
        status = pclose(fp);
    } else {
        /* Simple execution without output capture */
        status = system(cmd);
    }
    
    return WEXITSTATUS(status);
}

/**
 * Create a temporary directory for test files.
 */
char *create_temp_dir() {
    static char temp_dir[256];
    strcpy(temp_dir, TEMP_DIR);
    
    if (mkdtemp(temp_dir) == NULL) {
        perror("Failed to create temp directory");
        return NULL;
    }
    
    return temp_dir;
}

/**
 * Create a simple C program for GCOV instrumentation.
 */
int create_test_program(const char *dir, const char *name) {
    char filename[MAX_FILENAME];
    FILE *fp;
    
    snprintf(filename, sizeof(filename), "%s/%s.c", dir, name);
    
    fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("Failed to create test program");
        return -1;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        printf(\"Hello, World!\\n\");\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/**
 * Compile a program with GCOV instrumentation.
 */
int compile_with_gcov(const char *dir, const char *name) {
    char cmd[MAX_CMD_LEN];
    char src_file[MAX_FILENAME];
    char exe_file[MAX_FILENAME];
    
    snprintf(src_file, sizeof(src_file), "%s/%s.c", dir, name);
    snprintf(exe_file, sizeof(exe_file), "%s/%s", dir, name);
    
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s 2>&1",
             exe_file, src_file);
    
    char output[1024];
    int status = execute_command(cmd, output, sizeof(output));
    
    if (status != 0) {
        fprintf(stderr, "Compilation failed: %s\n", output);
        return -1;
    }
    
    return 0;
}

/**
 * Run a program to generate .gcda files.
 */
int run_program(const char *dir, const char *name) {
    char cmd[MAX_CMD_LEN];
    char exe_file[MAX_FILENAME];
    
    snprintf(exe_file, sizeof(exe_file), "%s/%s", dir, name);
    snprintf(cmd, sizeof(cmd), "%s > /dev/null 2>&1", exe_file);
    
    return system(cmd);
}

/**
 * Test a specific gcov-tool overlap command.
 */
void test_overlap_command(const char *test_name, const char *cmd, int expected_status) {
    tests_run++;
    
    printf("Test %d: %s\n", tests_run, test_name);
    printf("  Command: %s\n", cmd);
    
    char output[4096];
    int status = execute_command(cmd, output, sizeof(output));
    
    if (status == expected_status) {
        printf("  ✓ PASSED (expected status %d, got %d)\n", expected_status, status);
        tests_passed++;
    } else {
        printf("  ✗ FAILED (expected status %d, got %d)\n", expected_status, status);
        printf("  Output: %s\n", output);
        tests_failed++;
    }
    
    printf("\n");
}

/**
 * Generate permutations of flags to test different orderings.
 */
void test_flag_permutations(const char *temp_dir, const char *gcda1, const char *gcda2) {
    /* Base command without flags */
    char base_cmd[MAX_CMD_LEN];
    snprintf(base_cmd, sizeof(base_cmd), 
             "gcov-tool overlap %s %s", gcda1, gcda2);
    
    /* All flags we want to test (except -t which needs argument) */
    const char *flags[] = {"-v", "-f", "-F", "-o", "-h"};
    const int num_flags = 5;
    
    /* Test each flag individually */
    for (int i = 0; i < num_flags; i++) {
        char test_name[100];
        char cmd[MAX_CMD_LEN];
        
        snprintf(test_name, sizeof(test_name), "Single flag: %s", flags[i]);
        snprintf(cmd, sizeof(cmd), "%s %s", base_cmd, flags[i]);
        
        test_overlap_command(test_name, cmd, 0);
    }
    
    /* Test all flags together in different orders */
    const char *flag_orders[][6] = {
        {"-v", "-f", "-F", "-o", "-h", "-t 0.5"},
        {"-h", "-o", "-F", "-f", "-v", "-t 0.75"},
        {"-f", "-v", "-h", "-F", "-o", "-t 1.0"},
        {"-F", "-o", "-h", "-v", "-f", "-t 0.25"},
        {"-o", "-h", "-f", "-v", "-F", "-t 0.9"}
    };
    
    for (int i = 0; i < 5; i++) {
        char test_name[100];
        char cmd[MAX_CMD_LEN];
        
        snprintf(test_name, sizeof(test_name), "All flags order %d", i + 1);
        snprintf(cmd, sizeof(cmd), "%s", base_cmd);
        
        /* Add all flags in this order */
        for (int j = 0; j < 6; j++) {
            strcat(cmd, " ");
            strcat(cmd, flag_orders[i][j]);
        }
        
        test_overlap_command(test_name, cmd, 0);
    }
    
    /* Test with -t flag variations */
    const char *thresholds[] = {"0.0", "0.5", "1.0", "50.0", "100.0", "0.001"};
    for (int i = 0; i < 6; i++) {
        char test_name[100];
        char cmd[MAX_CMD_LEN];
        
        snprintf(test_name, sizeof(test_name), "Threshold: %s", thresholds[i]);
        snprintf(cmd, sizeof(cmd), "%s -t %s -v", base_cmd, thresholds[i]);
        
        test_overlap_command(test_name, cmd, 0);
    }
}

/**
 * Test edge cases and error conditions.
 */
void test_edge_cases(const char *temp_dir, const char *gcda1, const char *gcda2) {
    char base_cmd[MAX_CMD_LEN];
    snprintf(base_cmd, sizeof(base_cmd), 
             "gcov-tool overlap %s %s", gcda1, gcda2);
    
    /* Test 1: Invalid argument for -t (should trigger atof parsing) */
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "%s -t not_a_number", base_cmd);
    test_overlap_command("Invalid threshold (non-numeric)", cmd, 1);
    
    /* Test 2: Missing argument for -t (should trigger error in option parsing) */
    snprintf(cmd, sizeof(cmd), "%s -t", base_cmd);
    test_overlap_command("Missing threshold argument", cmd, 1);
    
    /* Test 3: Unknown flag (should trigger default case and overlap_usage) */
    snprintf(cmd, sizeof(cmd), "%s -x", base_cmd);
    test_overlap_command("Unknown flag -x", cmd, 1);
    
    /* Test 4: Repeated flags */
    snprintf(cmd, sizeof(cmd), "%s -v -v -v", base_cmd);
    test_overlap_command("Repeated -v flag", cmd, 0);
    
    /* Test 5: Combination with invalid flag */
    snprintf(cmd, sizeof(cmd), "%s -v -f -x -F", base_cmd);
    test_overlap_command("Valid flags with invalid flag", cmd, 1);
    
    /* Test 6: Empty threshold (edge case) */
    snprintf(cmd, sizeof(cmd), "%s -t ''", base_cmd);
    test_overlap_command("Empty threshold", cmd, 1);
    
    /* Test 7: Negative threshold */
    snprintf(cmd, sizeof(cmd), "%s -t -1.5", base_cmd);
    test_overlap_command("Negative threshold", cmd, 0);
    
    /* Test 8: Very large threshold */
    snprintf(cmd, sizeof(cmd), "%s -t 999999.999", base_cmd);
    test_overlap_command("Very large threshold", cmd, 0);
    
    /* Test 9: Threshold with scientific notation */
    snprintf(cmd, sizeof(cmd), "%s -t 1e-3", base_cmd);
    test_overlap_command("Scientific notation threshold", cmd, 0);
    
    /* Test 10: All flags with edge threshold */
    snprintf(cmd, sizeof(cmd), "%s -v -f -F -o -h -t 0.0", base_cmd);
    test_overlap_command("All flags with zero threshold", cmd, 0);
}

/**
 * Test with different file path types.
 */
void test_file_paths(const char *temp_dir) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed");
        return;
    }
    
    /* Create paths for testing */
    char rel_path1[MAX_FILENAME];
    char rel_path2[MAX_FILENAME];
    char abs_path1[MAX_FILENAME];
    char abs_path2[MAX_FILENAME];
    
    snprintf(rel_path1, sizeof(rel_path1), "%s/test1.gcda", temp_dir);
    snprintf(rel_path2, sizeof(rel_path2), "%s/test2.gcda", temp_dir);
    snprintf(abs_path1, sizeof(abs_path1), "%s/%s/test1.gcda", cwd, temp_dir);
    snprintf(abs_path2, sizeof(abs_path2), "%s/%s/test2.gcda", cwd, temp_dir);
    
    /* Test with relative paths */
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -v -f %s %s", 
             rel_path1, rel_path2);
    test_overlap_command("Relative paths with -v -f", cmd, 0);
    
    /* Test with absolute paths */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -F -o %s %s", 
             abs_path1, abs_path2);
    test_overlap_command("Absolute paths with -F -o", cmd, 0);
    
    /* Test with mixed paths */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -v -F %s %s", 
             rel_path1, abs_path2);
    test_overlap_command("Mixed paths with -v -F", cmd, 0);
    
    /* Test with -F flag (full filename display) */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap -F -h -t 0.5 %s %s", 
             abs_path1, abs_path2);
    test_overlap_command("Full filenames with hot threshold", cmd, 0);
}

/**
 * Clean up temporary directory.
 */
void cleanup_temp_dir(const char *temp_dir) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
    system(cmd);
}

int main(int argc, char *argv[]) {
    printf("========================================\n");
    printf("Testing gcov-tool overlap argument parsing\n");
    printf("Target: Lines 534-554 in gcov-tool.cc\n");
    printf("========================================\n\n");
    
    /* Create temporary directory for test files */
    char *temp_dir = create_temp_dir();
    if (temp_dir == NULL) {
        return 1;
    }
    
    printf("Created temp directory: %s\n\n", temp_dir);
    
    /* Create and compile test programs */
    printf("Creating test programs...\n");
    if (create_test_program(temp_dir, "test1") != 0 ||
        create_test_program(temp_dir, "test2") != 0) {
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    printf("Compiling with GCOV instrumentation...\n");
    if (compile_with_gcov(temp_dir, "test1") != 0 ||
        compile_with_gcov(temp_dir, "test2") != 0) {
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Run programs to generate .gcda files */
    printf("Running programs to generate profile data...\n");
    if (run_program(temp_dir, "test1") != 0 ||
        run_program(temp_dir, "test2") != 0) {
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Create paths to .gcda files */
    char gcda1[MAX_FILENAME];
    char gcda2[MAX_FILENAME];
    
    snprintf(gcda1, sizeof(gcda1), "%s/test1.gcda", temp_dir);
    snprintf(gcda2, sizeof(gcda2), "%s/test2.gcda", temp_dir);
    
    printf("Generated GCOV data files:\n");
    printf("  %s\n", gcda1);
    printf("  %s\n\n", gcda2);
    
    /* Run the actual tests */
    printf("Starting gcov-tool overlap tests...\n\n");
    
    /* Test 1: Flag permutations (covers all switch cases) */
    printf("=== Testing Flag Permutations ===\n");
    test_flag_permutations(temp_dir, gcda1, gcda2);
    
    /* Test 2: Edge cases and error conditions */
    printf("=== Testing Edge Cases ===\n");
    test_edge_cases(temp_dir, gcda1, gcda2);
    
    /* Test 3: Different file path types */
    printf("=== Testing File Path Variations ===\n");
    test_file_paths(temp_dir);
    
    /* Test 4: Minimal invocation (no flags) */
    printf("=== Testing Minimal Invocations ===\n");
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap %s", gcda1);
    test_overlap_command("Single input file", cmd, 0);
    
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap %s %s", gcda1, gcda2);
    test_overlap_command("Two input files, no flags", cmd, 0);
    
    /* Test 5: Verify gcov-tool exists and is executable */
    printf("=== Verifying gcov-tool ===\n");
    test_overlap_command("Check gcov-tool version", "gcov-tool --version", 0);
    
    /* Print summary */
    printf("========================================\n");
    printf("Test Summary\n");
    printf("========================================\n");
    printf("Tests Run:    %d\n", tests_run);
    printf("Tests Passed: %d\n", tests_passed);
    printf("Tests Failed: %d\n", tests_failed);
    printf("Pass Rate:    %.1f%%\n", 
           tests_run > 0 ? (100.0 * tests_passed / tests_run) : 0.0);
    
    /* Cleanup */
    printf("\nCleaning up temp directory...\n");
    cleanup_temp_dir(temp_dir);
    
    return tests_failed > 0 ? 1 : 0;
}
