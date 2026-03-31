/**
 * test_gcov_tool_overlap.c
 * 
 * Test driver for gcov-tool overlap command line parsing.
 * Specifically targets the uncovered switch cases in parse_overlap_options()
 * in gcov-tool.cc lines 534-554.
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
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

/**
 * Execute a command and return its exit status.
 * Captures stderr to avoid cluttering test output.
 */
static int execute_command(const char *cmd) {
    int status;
    pid_t pid;
    
    printf("Test %d: Executing: %s\n", tests_run + 1, cmd);
    
    pid = fork();
    if (pid == 0) {
        /* Child process */
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0) {
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }
        execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
        perror("execl failed");
        exit(127);
    } else if (pid > 0) {
        /* Parent process */
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    } else {
        perror("fork failed");
        return -1;
    }
}

/**
 * Create a simple instrumented C program and compile it with GCOV.
 * Returns 0 on success, -1 on failure.
 */
static int create_gcov_test_program(const char *temp_dir, char *gcda_files[], int num_files) {
    char src_path[256];
    char exe_path[256];
    char gcda_path[256];
    FILE *src_file;
    int i;
    
    /* Create source file */
    snprintf(src_path, sizeof(src_path), "%s/test_prog.c", temp_dir);
    src_file = fopen(src_path, "w");
    if (!src_file) {
        perror("Failed to create source file");
        return -1;
    }
    
    fprintf(src_file, 
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "\n"
        "int main(int argc, char **argv) {\n"
        "    int i, sum = 0;\n"
        "    \n"
        "    /* Some branching code for coverage */\n"
        "    for (i = 1; i < argc; i++) {\n"
        "        int val = atoi(argv[i]);\n"
        "        if (val > 0) {\n"
        "            sum += val;\n"
        "        } else if (val < 0) {\n"
        "            sum -= val;\n"
        "        } else {\n"
        "            sum += 1;\n"
        "        }\n"
        "    }\n"
        "    \n"
        "    printf(\"Sum: %%d\\n\", sum);\n"
        "    return 0;\n"
        "}\n");
    fclose(src_file);
    
    /* Compile with GCOV instrumentation */
    snprintf(exe_path, sizeof(exe_path), "%s/test_prog", temp_dir);
    char compile_cmd[512];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -o %s %s",
             exe_path, src_path);
    
    if (execute_command(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return -1;
    }
    
    /* Run the program multiple times with different arguments
       to generate multiple .gcda files */
    for (i = 0; i < num_files; i++) {
        char run_cmd[512];
        snprintf(gcda_path, sizeof(gcda_path), "%s/test_prog_%d.gcda", temp_dir, i);
        
        /* Remove any existing .gcda file */
        unlink(gcda_path);
        
        /* Copy the .gcno file to expected location */
        char gcno_src[256], gcno_dst[256];
        snprintf(gcno_src, sizeof(gcno_src), "%s/test_prog.gcno", temp_dir);
        snprintf(gcno_dst, sizeof(gcno_dst), "%s/test_prog_%d.gcno", temp_dir, i);
        if (link(gcno_src, gcno_dst) != 0 && errno != EEXIST) {
            /* Try copy if link fails */
            char cp_cmd[512];
            snprintf(cp_cmd, sizeof(cp_cmd), "cp %s %s", gcno_src, gcno_dst);
            execute_command(cp_cmd);
        }
        
        /* Run with different arguments each time */
        snprintf(run_cmd, sizeof(run_cmd),
                 "cd %s && GCOV_PREFIX=. GCOV_PREFIX_STRIP=0 ./test_prog %d %d %d",
                 temp_dir, i, i*2, i*3);
        
        if (execute_command(run_cmd) != 0) {
            fprintf(stderr, "Failed to run test program iteration %d\n", i);
        }
        
        /* Rename the generated .gcda file */
        char generated_gcda[256];
        snprintf(generated_gcda, sizeof(generated_gcda), "%s/test_prog.gcda", temp_dir);
        if (rename(generated_gcda, gcda_path) != 0 && errno != ENOENT) {
            fprintf(stderr, "Warning: Could not rename gcda file for iteration %d\n", i);
        }
        
        gcda_files[i] = strdup(gcda_path);
        if (!gcda_files[i]) {
            fprintf(stderr, "Memory allocation failed\n");
            return -1;
        }
    }
    
    return 0;
}

/**
 * Test a specific gcov-tool overlap command.
 * Returns 0 if test passed, 1 if failed.
 */
static int test_overlap_command(const char *cmd, int expect_success) {
    tests_run++;
    
    int result = execute_command(cmd);
    int passed = 0;
    
    if (expect_success) {
        if (result == 0) {
            printf("  ✓ Command succeeded as expected\n");
            passed = 1;
        } else {
            printf("  ✗ Command failed with exit code %d (expected success)\n", result);
        }
    } else {
        if (result != 0) {
            printf("  ✓ Command failed as expected (exit code %d)\n", result);
            passed = 1;
        } else {
            printf("  ✗ Command succeeded (expected failure)\n");
        }
    }
    
    if (passed) {
        tests_passed++;
        return 0;
    } else {
        tests_failed++;
        return 1;
    }
}

/**
 * Main test driver
 */
int main(int argc, char **argv) {
    char temp_dir[256];
    char *gcda_files[MAX_FILES];
    char gcov_tool_path[256] = "./gcov-tool";
    int i, num_gcda_files = 3;
    
    printf("=== GCOV-TOOL Overlap Parser Test ===\n\n");
    
    /* Use provided gcov-tool path if given */
    if (argc > 1) {
        strncpy(gcov_tool_path, argv[1], sizeof(gcov_tool_path) - 1);
        gcov_tool_path[sizeof(gcov_tool_path) - 1] = '\0';
    }
    
    /* Check if gcov-tool exists */
    if (access(gcov_tool_path, X_OK) != 0) {
        fprintf(stderr, "Error: gcov-tool not found at '%s'\n", gcov_tool_path);
        fprintf(stderr, "Usage: %s [path-to-gcov-tool]\n", argv[0]);
        return 1;
    }
    
    /* Create temporary directory */
    strcpy(temp_dir, TEMP_DIR_PATTERN);
    if (mkdtemp(temp_dir) == NULL) {
        perror("Failed to create temporary directory");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", temp_dir);
    
    /* Create test GCOV data files */
    printf("\nGenerating GCOV test data...\n");
    if (create_gcov_test_program(temp_dir, gcda_files, num_gcda_files) != 0) {
        fprintf(stderr, "Failed to create GCOV test data\n");
        /* Clean up and exit */
        char cleanup_cmd[512];
        snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
        execute_command(cleanup_cmd);
        return 1;
    }
    
    printf("\nGenerated %d .gcda files:\n", num_gcda_files);
    for (i = 0; i < num_gcda_files; i++) {
        printf("  %s\n", gcda_files[i]);
    }
    
    printf("\n=== Testing Overlap Command Parsing ===\n\n");
    
    /* Build base command with input files */
    char base_cmd[MAX_CMD_LEN];
    snprintf(base_cmd, sizeof(base_cmd), "%s overlap ", gcov_tool_path);
    for (i = 0; i < num_gcda_files; i++) {
        strncat(base_cmd, gcda_files[i], sizeof(base_cmd) - strlen(base_cmd) - 1);
        strncat(base_cmd, " ", sizeof(base_cmd) - strlen(base_cmd) - 1);
    }
    
    /* Test 1: All flags combined (targets all uncovered cases) */
    printf("\n--- Test 1: All flags combined ---\n");
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "%s -v -f -F -o -h -t 0.75", base_cmd);
    test_overlap_command(cmd, 1);
    
    /* Test 2: Different order of flags */
    printf("\n--- Test 2: Different flag order ---\n");
    snprintf(cmd, sizeof(cmd), "%s -t 1.0 -h -o -F -f -v", base_cmd);
    test_overlap_command(cmd, 1);
    
    /* Test 3: Flags with different threshold values */
    printf("\n--- Test 3: Various threshold values ---\n");
    const char *thresholds[] = {"0.0", "0.5", "1.0", "99.9", "0.001"};
    for (i = 0; i < sizeof(thresholds)/sizeof(thresholds[0]); i++) {
        snprintf(cmd, sizeof(cmd), "%s -t %s -v", base_cmd, thresholds[i]);
        test_overlap_command(cmd, 1);
    }
    
    /* Test 4: Individual flags (each targets specific case) */
    printf("\n--- Test 4: Individual flags ---\n");
    
    /* -v flag (verbose) - case 'v' */
    snprintf(cmd, sizeof(cmd), "%s -v", base_cmd);
    test_overlap_command(cmd, 1);
    
    /* -f flag (function level) - case 'f' */
    snprintf(cmd, sizeof(cmd), "%s -f", base_cmd);
    test_overlap_command(cmd, 1);
    
    /* -F flag (full filename) - case 'F' */
    snprintf(cmd, sizeof(cmd), "%s -F", base_cmd);
    test_overlap_command(cmd, 1);
    
    /* -o flag (object level) - case 'o' */
    snprintf(cmd, sizeof(cmd), "%s -o", base_cmd);
    test_overlap_command(cmd, 1);
    
    /* -h flag (hot only) - case 'h' */
    snprintf(cmd, sizeof(cmd), "%s -h", base_cmd);
    test_overlap_command(cmd, 1);
    
    /* -t flag (threshold) - case 't' */
    snprintf(cmd, sizeof(cmd), "%s -t 0.5", base_cmd);
    test_overlap_command(cmd, 1);
    
    /* Test 5: Flag combinations (2-3 flags at a time) */
    printf("\n--- Test 5: Flag combinations ---\n");
    
    /* -v -f combination */
    snprintf(cmd, sizeof(cmd), "%s -v -f", base_cmd);
    test_overlap_command(cmd, 1);
    
    /* -F -o combination */
    snprintf(cmd, sizeof(cmd), "%s -F -o", base_cmd);
    test_overlap_command(cmd, 1);
    
    /* -h -t combination */
    snprintf(cmd, sizeof(cmd), "%s -h -t 0.8", base_cmd);
    test_overlap_command(cmd, 1);
    
    /* -v -f -F combination */
    snprintf(cmd, sizeof(cmd), "%s -v -f -F", base_cmd);
    test_overlap_command(cmd, 1);
    
    /* Test 6: Edge cases and error conditions */
    printf("\n--- Test 6: Edge cases and errors ---\n");
    
    /* Invalid threshold (non-numeric) - should trigger atof but may fail */
    snprintf(cmd, sizeof(cmd), "%s -t not_a_number", base_cmd);
    test_overlap_command(cmd, 0);  /* Expect failure */
    
    /* Missing threshold argument */
    snprintf(cmd, sizeof(cmd), "%s -t", base_cmd);
    test_overlap_command(cmd, 0);  /* Expect failure */
    
    /* Unknown flag (should trigger default case) */
    snprintf(cmd, sizeof(cmd), "%s -x", base_cmd);
    test_overlap_command(cmd, 0);  /* Expect failure */
    
    /* Repeated flags */
    snprintf(cmd, sizeof(cmd), "%s -v -v -v", base_cmd);
    test_overlap_command(cmd, 1);  /* Should still work */
    
    /* Threshold with scientific notation */
    snprintf(cmd, sizeof(cmd), "%s -t 1.5e-2", base_cmd);
    test_overlap_command(cmd, 1);
    
    /* Test 7: With absolute and relative paths */
    printf("\n--- Test 7: Path variations ---\n");
    
    /* Use absolute path for one file */
    char abs_path_cmd[MAX_CMD_LEN];
    snprintf(abs_path_cmd, sizeof(abs_path_cmd), "%s overlap -v ", gcov_tool_path);
    
    /* Add first file with absolute path */
    char abs_path[512];
    if (realpath(gcda_files[0], abs_path) != NULL) {
        strncat(abs_path_cmd, abs_path, sizeof(abs_path_cmd) - strlen(abs_path_cmd) - 1);
        strncat(abs_path_cmd, " ", sizeof(abs_path_cmd) - strlen(abs_path_cmd) - 1);
        
        /* Add second file with relative path */
        char *rel_path = strrchr(gcda_files[1], '/');
        if (rel_path) {
            strncat(abs_path_cmd, rel_path + 1, sizeof(abs_path_cmd) - strlen(abs_path_cmd) - 1);
        } else {
            strncat(abs_path_cmd, gcda_files[1], sizeof(abs_path_cmd) - strlen(abs_path_cmd) - 1);
        }
        
        test_overlap_command(abs_path_cmd, 1);
    }
    
    /* Test 8: Minimal valid command (no flags) */
    printf("\n--- Test 8: Minimal valid command ---\n");
    test_overlap_command(base_cmd, 1);
    
    /* Clean up */
    printf("\n=== Cleaning up ===\n");
    for (i = 0; i < num_gcda_files; i++) {
        free(gcda_files[i]);
    }
    
    char cleanup_cmd[512];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
    if (execute_command(cleanup_cmd) == 0) {
        printf("Cleaned up temporary directory: %s\n", temp_dir);
    } else {
        fprintf(stderr, "Warning: Failed to clean up %s\n", temp_dir);
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests run:    %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    
    if (tests_failed == 0) {
        printf("\n✓ All tests passed!\n");
        printf("\nTo collect coverage data for gcov-tool.cc:\n");
        printf("1. Ensure gcov-tool was built with --enable-coverage\n");
        printf("2. Run: gcov gcov-tool.cc\n");
        printf("3. Check that lines 534-554 in gcov-tool.cc are now covered\n");
    } else {
        printf("\n✗ Some tests failed\n");
    }
    
    return (tests_failed == 0) ? 0 : 1;
}
