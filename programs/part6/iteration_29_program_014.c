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

typedef struct {
    const char *name;
    int passed;
    int exit_code;
} test_result_t;

/**
 * Create a simple C program, compile it with GCOV instrumentation,
 * run it to generate .gcda files, and return the executable name.
 */
static int create_gcov_test_files(char *temp_dir, char *gcda_files[][2], int *num_pairs) {
    char src_path[256];
    char exec_path[256];
    char gcda_path[256];
    char gcno_path[256];
    FILE *src_fp;
    int i;
    
    /* Create source file */
    snprintf(src_path, sizeof(src_path), "%s/test_prog.c", temp_dir);
    src_fp = fopen(src_path, "w");
    if (!src_fp) {
        perror("Failed to create source file");
        return -1;
    }
    
    fprintf(src_fp, 
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "\n"
        "void func1() {\n"
        "    printf(\"func1\\n\");\n"
        "}\n"
        "\n"
        "void func2(int x) {\n"
        "    if (x > 0) {\n"
        "        printf(\"positive\\n\");\n"
        "    } else {\n"
        "        printf(\"non-positive\\n\");\n"
        "    }\n"
        "}\n"
        "\n"
        "int main(int argc, char **argv) {\n"
        "    int i;\n"
        "    func1();\n"
        "    for (i = 0; i < argc; i++) {\n"
        "        func2(i);\n"
        "    }\n"
        "    return 0;\n"
        "}\n");
    fclose(src_fp);
    
    /* Compile with GCOV instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", temp_dir);
    char compile_cmd[512];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s",
             exec_path, src_path);
    
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return -1;
    }
    
    /* Generate multiple .gcda files by running the program with different arguments */
    *num_pairs = 3;
    for (i = 0; i < *num_pairs; i++) {
        /* Run program to generate .gcda */
        char run_cmd[256];
        snprintf(run_cmd, sizeof(run_cmd), "cd %s && ./test_prog", temp_dir);
        for (int j = 0; j < i; j++) {
            strcat(run_cmd, " arg");
        }
        
        if (system(run_cmd) != 0) {
            fprintf(stderr, "Failed to run test program iteration %d\n", i);
        }
        
        /* Store paths for the generated .gcda and .gcno files */
        gcda_files[i][0] = strdup(temp_dir);
        gcda_files[i][1] = strdup("test_prog.gcda");
    }
    
    /* Also get the .gcno file path */
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_prog.gcno", temp_dir);
    
    return 0;
}

/**
 * Execute gcov-tool with given arguments and capture exit code
 */
static int run_gcov_tool(const char *args, const char *gcda_files[][2], int num_files, 
                         char *output, size_t output_size) {
    char cmd[MAX_CMD_LEN];
    int result;
    FILE *fp;
    
    /* Build the command */
    snprintf(cmd, sizeof(cmd), "gcov-tool overlap %s", args);
    
    /* Add gcda files */
    for (int i = 0; i < num_files && i < 2; i++) {
        if (gcda_files[i][0] && gcda_files[i][1]) {
            char file_path[512];
            snprintf(file_path, sizeof(file_path), "%s/%s", 
                     gcda_files[i][0], gcda_files[i][1]);
            strcat(cmd, " ");
            strcat(cmd, file_path);
        }
    }
    
    /* Execute and capture output */
    fp = popen(cmd, "r");
    if (!fp) {
        snprintf(output, output_size, "Failed to execute: %s", cmd);
        return -1;
    }
    
    /* Read output (optional, for debugging) */
    if (output && output_size > 0) {
        size_t bytes_read = fread(output, 1, output_size - 1, fp);
        output[bytes_read] = '\0';
    }
    
    result = pclose(fp);
    return WEXITSTATUS(result);
}

/**
 * Test specific flag combinations to hit the uncovered switch cases
 */
static void test_flag_combinations(const char *gcda_files[][2], int num_files, 
                                   test_result_t *results, int *num_results) {
    int idx = 0;
    char output[4096];
    
    /* Test 1: All flags together in one command */
    results[idx].name = "All flags combined";
    results[idx].exit_code = run_gcov_tool("-v -f -F -o -h -t 0.75", 
                                           gcda_files, num_files, 
                                           output, sizeof(output));
    results[idx].passed = (results[idx].exit_code == 0);
    idx++;
    
    /* Test 2: Different order of flags */
    results[idx].name = "Flags in different order";
    results[idx].exit_code = run_gcov_tool("-t 1.0 -h -o -F -f -v", 
                                           gcda_files, num_files, 
                                           output, sizeof(output));
    results[idx].passed = (results[idx].exit_code == 0);
    idx++;
    
    /* Test 3: Only verbose flag */
    results[idx].name = "Verbose flag only (-v)";
    results[idx].exit_code = run_gcov_tool("-v", 
                                           gcda_files, num_files, 
                                           output, sizeof(output));
    results[idx].passed = (results[idx].exit_code == 0);
    idx++;
    
    /* Test 4: Function level and fullname flags */
    results[idx].name = "Function and fullname flags (-f -F)";
    results[idx].exit_code = run_gcov_tool("-f -F", 
                                           gcda_files, num_files, 
                                           output, sizeof(output));
    results[idx].passed = (results[idx].exit_code == 0);
    idx++;
    
    /* Test 5: Object level and hot only flags */
    results[idx].name = "Object and hot flags (-o -h)";
    results[idx].exit_code = run_gcov_tool("-o -h", 
                                           gcda_files, num_files, 
                                           output, sizeof(output));
    results[idx].passed = (results[idx].exit_code == 0);
    idx++;
    
    /* Test 6: Threshold with different values */
    results[idx].name = "Various threshold values (-t)";
    results[idx].exit_code = run_gcov_tool("-t 0.5", 
                                           gcda_files, num_files, 
                                           output, sizeof(output));
    results[idx].passed = (results[idx].exit_code == 0);
    idx++;
    
    /* Test 7: Combined without threshold */
    results[idx].name = "All except threshold (-v -f -F -o -h)";
    results[idx].exit_code = run_gcov_tool("-v -f -F -o -h", 
                                           gcda_files, num_files, 
                                           output, sizeof(output));
    results[idx].passed = (results[idx].exit_code == 0);
    idx++;
    
    /* Test 8: Repeated flags */
    results[idx].name = "Repeated verbose flags (-v -v -v)";
    results[idx].exit_code = run_gcov_tool("-v -v -v", 
                                           gcda_files, num_files, 
                                           output, sizeof(output));
    results[idx].passed = (results[idx].exit_code == 0);
    idx++;
    
    /* Test 9: Flags separated (not grouped) */
    results[idx].name = "Flags separated";
    results[idx].exit_code = run_gcov_tool("-v -f -F -o -h -t 0.25", 
                                           gcda_files, num_files, 
                                           output, sizeof(output));
    results[idx].passed = (results[idx].exit_code == 0);
    idx++;
    
    /* Test 10: With absolute paths */
    results[idx].name = "With absolute path argument";
    char abs_path_cmd[512];
    if (gcda_files[0][0] && gcda_files[0][1]) {
        char abs_path[512];
        snprintf(abs_path, sizeof(abs_path), "%s/%s", 
                 gcda_files[0][0], gcda_files[0][1]);
        char cwd[256];
        if (getcwd(cwd, sizeof(cwd))) {
            char full_path[768];
            snprintf(full_path, sizeof(full_path), "%s/%s", cwd, abs_path);
            snprintf(abs_path_cmd, sizeof(abs_path_cmd), "-v -f %s", full_path);
            results[idx].exit_code = run_gcov_tool(abs_path_cmd, 
                                                   gcda_files, 1, 
                                                   output, sizeof(output));
            results[idx].passed = (results[idx].exit_code == 0);
        } else {
            results[idx].passed = 0;
            results[idx].exit_code = -1;
        }
    }
    idx++;
    
    *num_results = idx;
}

/**
 * Test error conditions and edge cases
 */
static void test_error_conditions(const char *gcda_files[][2], int num_files,
                                  test_result_t *results, int *num_results) {
    int idx = *num_results;
    char output[4096];
    
    /* Test 11: Invalid argument for -t (should trigger atof) */
    results[idx].name = "Invalid threshold (non-numeric)";
    results[idx].exit_code = run_gcov_tool("-t not_a_number", 
                                           gcda_files, num_files, 
                                           output, sizeof(output));
    /* Non-zero exit expected for invalid argument */
    results[idx].passed = (results[idx].exit_code != 0);
    idx++;
    
    /* Test 12: Missing argument for -t (edge case) */
    results[idx].name = "Missing threshold argument";
    results[idx].exit_code = run_gcov_tool("-t", 
                                           gcda_files, num_files, 
                                           output, sizeof(output));
    results[idx].passed = (results[idx].exit_code != 0);
    idx++;
    
    /* Test 13: Unknown flag (should trigger default case) */
    results[idx].name = "Unknown flag (-x) triggers default case";
    results[idx].exit_code = run_gcov_tool("-x", 
                                           gcda_files, num_files, 
                                           output, sizeof(output));
    results[idx].passed = (results[idx].exit_code != 0);
    idx++;
    
    /* Test 14: Valid threshold with scientific notation */
    results[idx].name = "Scientific notation threshold (-t 1e-3)";
    results[idx].exit_code = run_gcov_tool("-t 1e-3", 
                                           gcda_files, num_files, 
                                           output, sizeof(output));
    results[idx].passed = (results[idx].exit_code == 0);
    idx++;
    
    /* Test 15: Threshold at boundary values */
    results[idx].name = "Boundary threshold (-t 0.0)";
    results[idx].exit_code = run_gcov_tool("-t 0.0", 
                                           gcda_files, num_files, 
                                           output, sizeof(output));
    results[idx].passed = (results[idx].exit_code == 0);
    idx++;
    
    results[idx].name = "Large threshold (-t 100.0)";
    results[idx].exit_code = run_gcov_tool("-t 100.0", 
                                           gcda_files, num_files, 
                                           output, sizeof(output));
    results[idx].passed = (results[idx].exit_code == 0);
    idx++;
    
    *num_results = idx;
}

/**
 * Clean up temporary files
 */
static void cleanup_temp_files(char *temp_dir, char *gcda_files[][2], int num_pairs) {
    char cmd[256];
    
    /* Remove temporary directory */
    if (temp_dir) {
        snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
        system(cmd);
    }
    
    /* Free duplicated strings */
    for (int i = 0; i < num_pairs; i++) {
        if (gcda_files[i][0]) free((void*)gcda_files[i][0]);
        if (gcda_files[i][1]) free((void*)gcda_files[i][1]);
    }
}

int main(int argc, char **argv) {
    char temp_dir_template[] = TEMP_DIR_PATTERN;
    char *temp_dir;
    const char *gcda_files[MAX_FILES][2] = {{0}};
    int num_file_pairs = 0;
    test_result_t results[50];
    int num_results = 0;
    int passed_count = 0;
    int i;
    
    printf("=== GCOV-TOOL Overlap Parser Test ===\n");
    printf("Targeting parse_overlap_options() lines 534-554 in gcov-tool.cc\n\n");
    
    /* Create temporary directory */
    temp_dir = mkdtemp(temp_dir_template);
    if (!temp_dir) {
        perror("Failed to create temporary directory");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", temp_dir);
    
    /* Create test GCOV data files */
    if (create_gcov_test_files(temp_dir, (char* (*)[2])gcda_files, &num_file_pairs) != 0) {
        fprintf(stderr, "Failed to create GCOV test files\n");
        cleanup_temp_files(temp_dir, (char* (*)[2])gcda_files, num_file_pairs);
        return 1;
    }
    
    printf("Generated %d GCOV data file pairs\n\n", num_file_pairs);
    
    /* Test normal flag combinations */
    printf("Testing flag combinations...\n");
    test_flag_combinations(gcda_files, num_file_pairs, results, &num_results);
    
    /* Test error conditions */
    printf("Testing error conditions...\n");
    test_error_conditions(gcda_files, num_file_pairs, results, &num_results);
    
    /* Print results */
    printf("\n=== Test Results ===\n");
    for (i = 0; i < num_results; i++) {
        printf("%-40s: %s (exit code: %d)\n", 
               results[i].name,
               results[i].passed ? "PASS" : "FAIL",
               results[i].exit_code);
        if (results[i].passed) passed_count++;
    }
    
    printf("\nSummary: %d/%d tests passed (%.1f%%)\n", 
           passed_count, num_results, 
           (float)passed_count * 100 / num_results);
    
    /* Cleanup */
    cleanup_temp_files(temp_dir, (char* (*)[2])gcda_files, num_file_pairs);
    
    /* Final check to ensure we've triggered the coverage */
    printf("\nTo collect coverage data for gcov-tool.cc:\n");
    printf("1. Ensure gcov-tool was built with --enable-coverage\n");
    printf("2. Run: gcov gcov-tool.cc\n");
    printf("3. Check that lines 534-554 are marked as executed\n");
    
    return (passed_count == num_results) ? 0 : 1;
}
