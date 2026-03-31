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
#define TEMP_DIR "/tmp/gcov_test_XXXXXX"

typedef struct {
    char *cmd;
    int expected_exit;
    char *description;
} test_case_t;

/* Simple C program to generate GCOV data */
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

/* Create a temporary directory for test files */
char *create_temp_dir() {
    char *template = strdup(TEMP_DIR);
    if (mkdtemp(template) == NULL) {
        perror("mkdtemp failed");
        free(template);
        return NULL;
    }
    return template;
}

/* Compile and run a test program with GCOV instrumentation */
int generate_gcda_files(const char *dir, int count) {
    char src_path[256];
    char exe_path[256];
    char gcda_path[256];
    int i;
    
    /* Write test program source */
    snprintf(src_path, sizeof(src_path), "%s/test.c", dir);
    FILE *src = fopen(src_path, "w");
    if (!src) {
        perror("Failed to create test.c");
        return -1;
    }
    fprintf(src, "%s", test_program);
    fclose(src);
    
    /* Compile with GCOV instrumentation */
    snprintf(exe_path, sizeof(exe_path), "%s/test", dir);
    char compile_cmd[512];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -o %s %s 2>/dev/null",
             exe_path, src_path);
    
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return -1;
    }
    
    /* Run multiple times to generate different .gcda files */
    for (i = 0; i < count; i++) {
        char run_cmd[256];
        snprintf(run_cmd, sizeof(run_cmd), "cd %s && ./test > /dev/null 2>&1", dir);
        
        if (system(run_cmd) != 0) {
            fprintf(stderr, "Failed to run test program\n");
            return -1;
        }
        
        /* Rename .gcda file for each run */
        if (i > 0) {
            snprintf(gcda_path, sizeof(gcda_path), "%s/test.gcda.%d", dir, i);
            char rename_cmd[512];
            snprintf(rename_cmd, sizeof(rename_cmd),
                     "mv %s/test.gcda %s 2>/dev/null", dir, gcda_path);
            system(rename_cmd);
        }
    }
    
    return 0;
}

/* Execute a gcov-tool command and return exit code */
int run_gcov_tool(const char *cmd) {
    printf("Running: %s\n", cmd);
    
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Test all flag combinations to trigger the uncovered switch cases */
void test_flag_combinations(const char *dir, const char *gcov_tool_path) {
    char gcda_files[2][256];
    snprintf(gcda_files[0], sizeof(gcda_files[0]), "%s/test.gcda", dir);
    snprintf(gcda_files[1], sizeof(gcda_files[1]), "%s/test.gcda.1", dir);
    
    /* Base test cases covering all uncovered flags */
    test_case_t tests[] = {
        /* Test all flags together - should trigger all case statements */
        {"%s overlap -v -f -F -o -h -t 0.75 %s %s", 0, "All flags combined"},
        
        /* Test each flag individually */
        {"%s overlap -v %s %s", 0, "Verbose flag only"},
        {"%s overlap -f %s %s", 0, "Function level flag only"},
        {"%s overlap -F %s %s", 0, "Full filename flag only"},
        {"%s overlap -o %s %s", 0, "Object level flag only"},
        {"%s overlap -h %s %s", 0, "Hot only flag only"},
        {"%s overlap -t 0.5 %s %s", 0, "Threshold flag only"},
        
        /* Test with different threshold values */
        {"%s overlap -t 0.1 %s %s", 0, "Low threshold (0.1)"},
        {"%s overlap -t 1.0 %s %s", 0, "Threshold 1.0"},
        {"%s overlap -t 99.9 %s %s", 0, "High threshold (99.9)"},
        
        /* Test flag permutations (different orders) */
        {"%s overlap -t 0.5 -v -f -F %s %s", 0, "Flags in different order 1"},
        {"%s overlap -h -o -F -f -v -t 0.25 %s %s", 0, "Flags in different order 2"},
        {"%s overlap -F -o -h -t 0.8 -v -f %s %s", 0, "Flags in different order 3"},
        
        /* Test with repeated flags */
        {"%s overlap -v -v -v %s %s", 0, "Repeated verbose flag"},
        {"%s overlap -f -f -t 0.5 -t 0.6 %s %s", 0, "Repeated function and threshold flags"},
        
        /* Test edge cases for threshold */
        {"%s overlap -t 0 %s %s", 0, "Zero threshold"},
        {"%s overlap -t .5 %s %s", 0, "Threshold without leading zero"},
        {"%s overlap -t 1. %s %s", 0, "Threshold without decimal part"},
        
        /* Test with single input file */
        {"%s overlap -v -f %s", 0, "Single input file"},
        
        /* Test with absolute paths */
        {"%s overlap -v -F %s %s", 0, "With full filename flag and absolute paths"},
        
        /* Test invalid cases (should trigger error handling) */
        {"%s overlap -t not_a_number %s %s", 1, "Invalid threshold (non-numeric)"},
        {"%s overlap -t %s %s", 1, "Missing threshold value"},
        {"%s overlap -x %s %s", 1, "Unknown flag (-x)"},
        {"%s overlap -t", 1, "Threshold with no value or files"},
        
        {NULL, 0, NULL}
    };
    
    int passed = 0, failed = 0;
    
    for (int i = 0; tests[i].cmd != NULL; i++) {
        char cmd[MAX_CMD_LEN];
        
        /* Format the command string with appropriate arguments */
        if (strstr(tests[i].cmd, "%s %s") != NULL) {
            /* Command expects two files */
            snprintf(cmd, sizeof(cmd), tests[i].cmd, 
                    gcov_tool_path, gcda_files[0], gcda_files[1]);
        } else if (strstr(tests[i].cmd, "%s") != NULL && 
                  strstr(strstr(tests[i].cmd, "%s") + 2, "%s") == NULL) {
            /* Command expects one file */
            snprintf(cmd, sizeof(cmd), tests[i].cmd, 
                    gcov_tool_path, gcda_files[0]);
        } else {
            /* Command with no file arguments */
            snprintf(cmd, sizeof(cmd), tests[i].cmd, gcov_tool_path);
        }
        
        /* Add absolute path test for -F flag */
        if (strstr(tests[i].description, "absolute paths")) {
            char abs_path1[512], abs_path2[512];
            realpath(gcda_files[0], abs_path1);
            realpath(gcda_files[1], abs_path2);
            snprintf(cmd, sizeof(cmd), tests[i].cmd, 
                    gcov_tool_path, abs_path1, abs_path2);
        }
        
        int exit_code = run_gcov_tool(cmd);
        
        /* Check if result matches expectation */
        if ((tests[i].expected_exit == 0 && exit_code == 0) ||
            (tests[i].expected_exit != 0 && exit_code != 0)) {
            printf("  ✓ PASS: %s (exit: %d)\n", tests[i].description, exit_code);
            passed++;
        } else {
            printf("  ✗ FAIL: %s (expected: %d, got: %d)\n", 
                   tests[i].description, tests[i].expected_exit, exit_code);
            failed++;
        }
        
        printf("\n");
    }
    
    printf("\n=== Summary ===\n");
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    printf("Total:  %d\n", passed + failed);
}

int main(int argc, char *argv[]) {
    char *temp_dir = NULL;
    const char *gcov_tool_path = "gcov-tool";
    
    /* Allow overriding gcov-tool path via command line */
    if (argc > 1) {
        gcov_tool_path = argv[1];
    }
    
    printf("Testing gcov-tool overlap argument parsing\n");
    printf("Targeting parse_overlap_options() lines 534-554\n");
    printf("Using gcov-tool at: %s\n\n", gcov_tool_path);
    
    /* Create temporary directory for test files */
    temp_dir = create_temp_dir();
    if (!temp_dir) {
        fprintf(stderr, "Failed to create temporary directory\n");
        return 1;
    }
    
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Generate GCOV data files */
    if (generate_gcda_files(temp_dir, 2) != 0) {
        fprintf(stderr, "Failed to generate GCOV data files\n");
        free(temp_dir);
        return 1;
    }
    
    printf("Generated test.gcda and test.gcda.1\n\n");
    
    /* Run comprehensive tests */
    test_flag_combinations(temp_dir, gcov_tool_path);
    
    /* Cleanup */
    printf("\nCleaning up...\n");
    char cleanup_cmd[256];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
    system(cleanup_cmd);
    free(temp_dir);
    
    printf("Done.\n");
    
    return 0;
}
