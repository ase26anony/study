/**
 * test_gcov_tool_overlap.c
 * 
 * Test driver for gcov-tool overlap command parsing.
 * Exercises the parse_overlap_options function to cover lines 534-554.
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

/* Global verbosity flag for test output */
static int test_verbose = 0;

/**
 * Execute a command and capture its exit status.
 * Returns exit code of the command.
 */
static int execute_command(const char *cmd, int capture_output) {
    if (test_verbose) {
        printf("Executing: %s\n", cmd);
    }
    
    if (capture_output) {
        /* Redirect output to /dev/null to avoid cluttering test output */
        char full_cmd[MAX_CMD_LEN];
        snprintf(full_cmd, sizeof(full_cmd), "%s > /dev/null 2>&1", cmd);
        return system(full_cmd);
    } else {
        return system(cmd);
    }
}

/**
 * Create a minimal C program, compile it with GCOV instrumentation,
 * run it to generate .gcda files, and return the path to the .gcda file.
 */
static char* create_gcda_file(const char *temp_dir, int file_num) {
    char src_path[256];
    char exe_path[256];
    char gcda_path[256];
    
    /* Create source file */
    snprintf(src_path, sizeof(src_path), "%s/test%d.c", temp_dir, file_num);
    FILE *src = fopen(src_path, "w");
    if (!src) {
        perror("Failed to create source file");
        return NULL;
    }
    fprintf(src, "int main() { return 0; }\n");
    fclose(src);
    
    /* Compile with GCOV instrumentation */
    snprintf(exe_path, sizeof(exe_path), "%s/test%d.exe", temp_dir, file_num);
    char compile_cmd[512];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -o %s %s",
             exe_path, src_path);
    
    if (execute_command(compile_cmd, 1) != 0) {
        fprintf(stderr, "Compilation failed for %s\n", src_path);
        return NULL;
    }
    
    /* Run the program to generate .gcda file */
    char run_cmd[256];
    snprintf(run_cmd, sizeof(run_cmd), "%s", exe_path);
    execute_command(run_cmd, 1);
    
    /* Return the path to the .gcda file */
    snprintf(gcda_path, sizeof(gcda_path), "%s/test%d.gcda", temp_dir, file_num);
    return strdup(gcda_path);
}

/**
 * Test valid flag combinations to trigger all case statements.
 */
static void test_valid_combinations(const char *gcov_tool_path, 
                                   const char **gcda_files, 
                                   int num_files) {
    printf("\n=== Testing Valid Flag Combinations ===\n");
    
    /* Base command with files */
    char base_cmd[MAX_CMD_LEN];
    snprintf(base_cmd, sizeof(base_cmd), "%s overlap", gcov_tool_path);
    for (int i = 0; i < num_files && i < 2; i++) {
        strncat(base_cmd, " ", MAX_CMD_LEN - strlen(base_cmd) - 1);
        strncat(base_cmd, gcda_files[i], MAX_CMD_LEN - strlen(base_cmd) - 1);
    }
    
    /* Test 1: All flags in one command (covers all cases) */
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "%s -v -f -F -o -h -t 0.75", base_cmd);
    printf("Test 1: All flags combined\n");
    int result = execute_command(cmd, 1);
    printf("  Exit code: %d\n", WEXITSTATUS(result));
    
    /* Test 2: Different order of flags */
    snprintf(cmd, sizeof(cmd), "%s -t 1.0 -h -o -F -f -v", base_cmd);
    printf("Test 2: Reverse order flags\n");
    result = execute_command(cmd, 1);
    printf("  Exit code: %d\n", WEXITSTATUS(result));
    
    /* Test 3: Only -v flag (verbose) */
    snprintf(cmd, sizeof(cmd), "%s -v", base_cmd);
    printf("Test 3: Only -v flag\n");
    result = execute_command(cmd, 1);
    printf("  Exit code: %d\n", WEXITSTATUS(result));
    
    /* Test 4: Only -f flag (function level) */
    snprintf(cmd, sizeof(cmd), "%s -f", base_cmd);
    printf("Test 4: Only -f flag\n");
    result = execute_command(cmd, 1);
    printf("  Exit code: %d\n", WEXITSTATUS(result));
    
    /* Test 5: Only -F flag (full filename) */
    snprintf(cmd, sizeof(cmd), "%s -F", base_cmd);
    printf("Test 5: Only -F flag\n");
    result = execute_command(cmd, 1);
    printf("  Exit code: %d\n", WEXITSTATUS(result));
    
    /* Test 6: Only -o flag (object level) */
    snprintf(cmd, sizeof(cmd), "%s -o", base_cmd);
    printf("Test 6: Only -o flag\n");
    result = execute_command(cmd, 1);
    printf("  Exit code: %d\n", WEXITSTATUS(result));
    
    /* Test 7: Only -h flag (hot only) */
    snprintf(cmd, sizeof(cmd), "%s -h", base_cmd);
    printf("Test 7: Only -h flag\n");
    result = execute_command(cmd, 1);
    printf("  Exit code: %d\n", WEXITSTATUS(result));
    
    /* Test 8: Only -t flag with different values */
    snprintf(cmd, sizeof(cmd), "%s -t 0.5", base_cmd);
    printf("Test 8: Only -t 0.5 flag\n");
    result = execute_command(cmd, 1);
    printf("  Exit code: %d\n", WEXITSTATUS(result));
    
    snprintf(cmd, sizeof(cmd), "%s -t 0.0", base_cmd);
    printf("Test 9: Only -t 0.0 flag\n");
    result = execute_command(cmd, 1);
    printf("  Exit code: %d\n", WEXITSTATUS(result));
    
    snprintf(cmd, sizeof(cmd), "%s -t 99.9", base_cmd);
    printf("Test 10: Only -t 99.9 flag\n");
    result = execute_command(cmd, 1);
    printf("  Exit code: %d\n", WEXITSTATUS(result));
    
    /* Test 11: Combination without -t */
    snprintf(cmd, sizeof(cmd), "%s -v -f -F -o -h", base_cmd);
    printf("Test 11: All flags except -t\n");
    result = execute_command(cmd, 1);
    printf("  Exit code: %d\n", WEXITSTATUS(result));
    
    /* Test 12: Repeated flags */
    snprintf(cmd, sizeof(cmd), "%s -v -v -v", base_cmd);
    printf("Test 12: Repeated -v flags\n");
    result = execute_command(cmd, 1);
    printf("  Exit code: %d\n", WEXITSTATUS(result));
}

/**
 * Test edge cases and error conditions.
 */
static void test_edge_cases(const char *gcov_tool_path, 
                           const char **gcda_files, 
                           int num_files) {
    printf("\n=== Testing Edge Cases ===\n");
    
    /* Base command with files */
    char base_cmd[MAX_CMD_LEN];
    snprintf(base_cmd, sizeof(base_cmd), "%s overlap", gcov_tool_path);
    for (int i = 0; i < num_files && i < 2; i++) {
        strncat(base_cmd, " ", MAX_CMD_LEN - strlen(base_cmd) - 1);
        strncat(base_cmd, gcda_files[i], MAX_CMD_LEN - strlen(base_cmd) - 1);
    }
    
    /* Test 1: Invalid argument for -t (should trigger atof) */
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "%s -t not_a_number", base_cmd);
    printf("Test 1: Invalid -t argument (not_a_number)\n");
    int result = execute_command(cmd, 0);  // Don't capture to see error message
    printf("  Exit code: %d\n", WEXITSTATUS(result));
    
    /* Test 2: Missing argument for -t (should trigger error in option parsing) */
    snprintf(cmd, sizeof(cmd), "%s -t", base_cmd);
    printf("Test 2: Missing -t argument\n");
    result = execute_command(cmd, 0);
    printf("  Exit code: %d\n", WEXITSTATUS(result));
    
    /* Test 3: Unknown flag (should trigger default case) */
    snprintf(cmd, sizeof(cmd), "%s -x", base_cmd);
    printf("Test 3: Unknown flag -x\n");
    result = execute_command(cmd, 0);
    printf("  Exit code: %d\n", WEXITSTATUS(result));
    
    /* Test 4: Combination with unknown flag */
    snprintf(cmd, sizeof(cmd), "%s -v -x -f", base_cmd);
    printf("Test 4: Valid flags with unknown -x\n");
    result = execute_command(cmd, 0);
    printf("  Exit code: %d\n", WEXITSTATUS(result));
    
    /* Test 5: Empty argument list (just overlap command) */
    snprintf(cmd, sizeof(cmd), "%s overlap", gcov_tool_path);
    printf("Test 5: No flags, no files\n");
    result = execute_command(cmd, 0);
    printf("  Exit code: %d\n", WEXITSTATUS(result));
    
    /* Test 6: -t with negative value */
    snprintf(cmd, sizeof(cmd), "%s -t -1.0", base_cmd);
    printf("Test 6: Negative threshold -t -1.0\n");
    result = execute_command(cmd, 1);
    printf("  Exit code: %d\n", WEXITSTATUS(result));
    
    /* Test 7: -t with very large value */
    snprintf(cmd, sizeof(cmd), "%s -t 1e100", base_cmd);
    printf("Test 7: Very large threshold -t 1e100\n");
    result = execute_command(cmd, 1);
    printf("  Exit code: %d\n", WEXITSTATUS(result));
}

/**
 * Test with different file path formats.
 */
static void test_file_paths(const char *gcov_tool_path, const char *temp_dir) {
    printf("\n=== Testing Different File Path Formats ===\n");
    
    /* Create .gcda files in different locations */
    char *gcda1 = create_gcda_file(temp_dir, 1);
    char *gcda2 = create_gcda_file(temp_dir, 2);
    
    if (!gcda1 || !gcda2) {
        fprintf(stderr, "Failed to create .gcda files\n");
        free(gcda1);
        free(gcda2);
        return;
    }
    
    /* Test with relative paths */
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f test1.gcda test2.gcda", 
             gcov_tool_path);
    printf("Test 1: Relative paths (from temp dir)\n");
    /* Change to temp directory for relative path test */
    char old_cwd[256];
    getcwd(old_cwd, sizeof(old_cwd));
    chdir(temp_dir);
    int result = execute_command(cmd, 1);
    chdir(old_cwd);
    printf("  Exit code: %d\n", WEXITSTATUS(result));
    
    /* Test with absolute paths */
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f %s %s", 
             gcov_tool_path, gcda1, gcda2);
    printf("Test 2: Absolute paths\n");
    result = execute_command(cmd, 1);
    printf("  Exit code: %d\n", WEXITSTATUS(result));
    
    /* Test with mixed paths */
    char relative_path[256];
    snprintf(relative_path, sizeof(relative_path), "test1.gcda");
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f %s %s", 
             gcov_tool_path, gcda1, relative_path);
    printf("Test 3: Mixed absolute and relative paths\n");
    chdir(temp_dir);
    result = execute_command(cmd, 1);
    chdir(old_cwd);
    printf("  Exit code: %d\n", WEXITSTATUS(result));
    
    free(gcda1);
    free(gcda2);
}

/**
 * Main test driver.
 */
int main(int argc, char *argv[]) {
    const char *gcov_tool_path = "./gcov-tool";
    
    if (argc > 1) {
        gcov_tool_path = argv[1];
    }
    
    printf("Testing gcov-tool overlap command parsing\n");
    printf("Using gcov-tool at: %s\n", gcov_tool_path);
    
    /* Check if gcov-tool exists and is executable */
    if (access(gcov_tool_path, X_OK) != 0) {
        fprintf(stderr, "Error: gcov-tool not found or not executable at %s\n", 
                gcov_tool_path);
        fprintf(stderr, "Usage: %s [path/to/gcov-tool]\n", argv[0]);
        return 1;
    }
    
    /* Create temporary directory for test files */
    char temp_dir[256];
    strcpy(temp_dir, TEMP_DIR);
    if (mkdtemp(temp_dir) == NULL) {
        perror("Failed to create temporary directory");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", temp_dir);
    
    /* Create .gcda files for testing */
    char *gcda_files[MAX_FILES];
    int num_files = 2;  /* Create 2 files for overlap comparison */
    
    for (int i = 0; i < num_files; i++) {
        gcda_files[i] = create_gcda_file(temp_dir, i + 1);
        if (!gcda_files[i]) {
            fprintf(stderr, "Failed to create gcda file %d\n", i + 1);
            for (int j = 0; j < i; j++) {
                free(gcda_files[j]);
            }
            rmdir(temp_dir);
            return 1;
        }
        printf("Created: %s\n", gcda_files[i]);
    }
    
    /* Run the test suites */
    test_valid_combinations(gcov_tool_path, (const char **)gcda_files, num_files);
    test_edge_cases(gcov_tool_path, (const char **)gcda_files, num_files);
    test_file_paths(gcov_tool_path, temp_dir);
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    for (int i = 0; i < num_files; i++) {
        free(gcda_files[i]);
    }
    
    /* Remove all files in temp directory */
    char cleanup_cmd[512];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
    execute_command(cleanup_cmd, 1);
    printf("Removed temporary directory: %s\n", temp_dir);
    
    printf("\n=== Test Complete ===\n");
    printf("To collect coverage data for gcov-tool.cc:\n");
    printf("1. Ensure gcov-tool was built with --enable-coverage\n");
    printf("2. Run: gcov gcov-tool.cc\n");
    printf("3. Check gcov-tool.cc.gcov for coverage of lines 534-554\n");
    
    return 0;
}
