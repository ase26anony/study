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
#define TEMP_DIR_TEMPLATE "/tmp/gcov_test_XXXXXX"

typedef struct {
    char *cmd;
    int expected_exit;
    char *description;
} test_case_t;

/* Global variables to track test results */
static int tests_passed = 0;
static int tests_failed = 0;
static int tests_skipped = 0;

/**
 * Execute a command and return its exit status
 */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    fflush(stdout);
    
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    } else {
        return -1; /* Command didn't exit normally */
    }
}

/**
 * Create a minimal C program, compile it with GCOV instrumentation,
 * run it to generate .gcda files, and return the path to the .gcda file
 */
char* create_gcov_data(const char *temp_dir, int index) {
    char source_path[256];
    char exec_path[256];
    char gcda_path[256];
    char cmd[512];
    
    /* Create source file */
    snprintf(source_path, sizeof(source_path), "%s/test%d.c", temp_dir, index);
    FILE *src = fopen(source_path, "w");
    if (!src) {
        perror("Failed to create source file");
        return NULL;
    }
    
    fprintf(src, "#include <stdio.h>\n");
    fprintf(src, "int func%d(int x) { return x * %d; }\n", index, index);
    fprintf(src, "int main() { \n");
    fprintf(src, "    printf(\"Test %d\\n\");\n", index);
    fprintf(src, "    return func%d(%d);\n", index, index);
    fprintf(src, "}\n");
    fclose(src);
    
    /* Compile with GCOV instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test%d", temp_dir, index);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s 2>/dev/null",
             exec_path, source_path);
    
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile test program %d\n", index);
        return NULL;
    }
    
    /* Run the program to generate .gcda file */
    snprintf(cmd, sizeof(cmd), "cd %s && ./test%d >/dev/null 2>&1", temp_dir, index);
    system(cmd);
    
    /* Return the path to the .gcda file */
    snprintf(gcda_path, sizeof(gcda_path), "%s/test%d.gcda", temp_dir, index);
    
    /* Also generate .gcno file path for completeness */
    snprintf(cmd, sizeof(cmd), "cd %s && gcc -fprofile-arcs -ftest-coverage -c test%d.c 2>/dev/null", 
             temp_dir, index);
    system(cmd);
    
    return strdup(gcda_path);
}

/**
 * Run a single test case
 */
void run_test_case(test_case_t *test, const char *temp_dir) {
    printf("\n=== Test: %s ===\n", test->description);
    
    int result = execute_command(test->cmd);
    
    if (result == test->expected_exit) {
        printf("✓ PASSED (exit code: %d)\n", result);
        tests_passed++;
    } else if (result == -2) {
        printf("- SKIPPED (gcov-tool not found)\n");
        tests_skipped++;
    } else {
        printf("✗ FAILED - Expected exit code %d, got %d\n", 
               test->expected_exit, result);
        tests_failed++;
    }
}

/**
 * Check if gcov-tool exists in PATH
 */
int check_gcov_tool_exists() {
    return system("which gcov-tool >/dev/null 2>&1") == 0;
}

int main(int argc, char *argv[]) {
    char temp_dir[256];
    char *gcda_files[MAX_FILES];
    int num_gcda_files = 0;
    
    /* Create temporary directory */
    strcpy(temp_dir, TEMP_DIR_TEMPLATE);
    if (mkdtemp(temp_dir) == NULL) {
        perror("Failed to create temporary directory");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", temp_dir);
    
    /* Check if gcov-tool exists */
    if (!check_gcov_tool_exists()) {
        printf("Warning: gcov-tool not found in PATH.\n");
        printf("Trying to use ./gcov-tool...\n");
        
        /* Try to use local gcov-tool */
        if (access("./gcov-tool", X_OK) != 0) {
            fprintf(stderr, "Error: gcov-tool not found.\n");
            fprintf(stderr, "Please build gcov-tool with coverage instrumentation first:\n");
            fprintf(stderr, "  g++ --coverage -o gcov-tool gcov-tool.cc -lgcov\n");
            rmdir(temp_dir);
            return 1;
        }
    }
    
    /* Create GCOV data files */
    printf("\nCreating test GCOV data files...\n");
    for (int i = 0; i < 3; i++) {
        gcda_files[num_gcda_files] = create_gcov_data(temp_dir, i + 1);
        if (gcda_files[num_gcda_files]) {
            printf("  Created: %s\n", gcda_files[num_gcda_files]);
            num_gcda_files++;
        }
    }
    
    if (num_gcda_files < 2) {
        fprintf(stderr, "Failed to create enough GCOV data files\n");
        for (int i = 0; i < num_gcda_files; i++) free(gcda_files[i]);
        rmdir(temp_dir);
        return 1;
    }
    
    /* Define test cases */
    test_case_t test_cases[] = {
        /* Basic flag tests - each flag individually */
        {"gcov-tool overlap -v %s %s", 0, "Verbose flag (-v)"},
        {"gcov-tool overlap -f %s %s", 0, "Function level flag (-f)"},
        {"gcov-tool overlap -F %s %s", 0, "Full filename flag (-F)"},
        {"gcov-tool overlap -o %s %s", 0, "Object level flag (-o)"},
        {"gcov-tool overlap -h %s %s", 0, "Hot only flag (-h)"},
        {"gcov-tool overlap -t 0.5 %s %s", 0, "Threshold flag (-t 0.5)"},
        {"gcov-tool overlap -t 1.0 %s %s", 0, "Threshold flag (-t 1.0)"},
        {"gcov-tool overlap -t 0.75 %s %s", 0, "Threshold flag (-t 0.75)"},
        
        /* Combined flags - testing all uncovered flags together */
        {"gcov-tool overlap -v -f -F -o -h -t 0.8 %s %s", 0, 
         "All flags combined: -v -f -F -o -h -t 0.8"},
        
        /* Flag permutations - different orders */
        {"gcov-tool overlap -f -F -v -o %s %s", 0, 
         "Flag permutation: -f -F -v -o"},
        {"gcov-tool overlap -t 0.3 -h -v %s %s", 0, 
         "Flag permutation: -t 0.3 -h -v"},
        {"gcov-tool overlap -o -F -f -v -h -t 0.9 %s %s", 0, 
         "Flag permutation: -o -F -f -v -h -t 0.9"},
        
        /* Edge cases for -t flag */
        {"gcov-tool overlap -t 0 %s %s", 0, "Threshold edge case (-t 0)"},
        {"gcov-tool overlap -t 100.5 %s %s", 0, "Large threshold (-t 100.5)"},
        {"gcov-tool overlap -t 1e-3 %s %s", 0, "Scientific notation (-t 1e-3)"},
        
        /* Error cases */
        {"gcov-tool overlap -t", 1, "Missing argument for -t (should fail)"},
        {"gcov-tool overlap -t not_a_number %s %s", 1, 
         "Invalid argument for -t (should fail)"},
        {"gcov-tool overlap -x %s %s", 1, "Unknown flag -x (should fail)"},
        
        /* Repeated flags */
        {"gcov-tool overlap -v -v -v %s %s", 0, "Repeated -v flag"},
        {"gcov-tool overlap -f -f -h -h %s %s", 0, "Repeated -f and -h flags"},
        
        /* Mixed valid files and flags */
        {"gcov-tool overlap -v -f %s -F %s -o -h -t 0.6", 0, 
         "Flags interspersed with filenames"},
        
        /* With .gcno files instead of .gcda */
        {"gcov-tool overlap -v %s/test1.gcno %s/test2.gcno", 0, 
         "Using .gcno files instead"},
        
        /* Multiple input files */
        {"gcov-tool overlap -v %s %s %s", 0, "Three input files"},
        
        /* Absolute paths */
        {"gcov-tool overlap -v `pwd`/%s/test1.gcda `pwd`/%s/test2.gcda", 0, 
         "Using absolute paths"},
    };
    
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    
    printf("\nRunning %d test cases...\n", num_tests);
    
    /* Run all test cases */
    for (int i = 0; i < num_tests; i++) {
        char cmd[MAX_CMD_LEN];
        
        /* Format the command with actual file paths */
        if (strstr(test_cases[i].cmd, "%s") != NULL) {
            if (strstr(test_cases[i].cmd, "test1.gcno") != NULL) {
                /* Special case for .gcno test */
                snprintf(cmd, sizeof(cmd), test_cases[i].cmd, 
                        temp_dir, temp_dir);
            } else if (strstr(test_cases[i].cmd, "pwd") != NULL) {
                /* Special case for absolute paths */
                char cwd[256];
                getcwd(cwd, sizeof(cwd));
                snprintf(cmd, sizeof(cmd), test_cases[i].cmd, 
                        cwd, temp_dir, cwd, temp_dir);
            } else if (strstr(test_cases[i].cmd, "%s %s %s") != NULL) {
                /* Three files */
                snprintf(cmd, sizeof(cmd), test_cases[i].cmd,
                        gcda_files[0], gcda_files[1], gcda_files[2]);
            } else {
                /* Two files (most common case) */
                snprintf(cmd, sizeof(cmd), test_cases[i].cmd,
                        gcda_files[0], gcda_files[1]);
            }
        } else {
            /* No file placeholders */
            strncpy(cmd, test_cases[i].cmd, sizeof(cmd));
        }
        
        test_cases[i].cmd = strdup(cmd);
        run_test_case(&test_cases[i], temp_dir);
        free(test_cases[i].cmd);
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Total tests:  %d\n", num_tests);
    printf("Passed:       %d\n", tests_passed);
    printf("Failed:       %d\n", tests_failed);
    printf("Skipped:      %d\n", tests_skipped);
    
    /* Cleanup */
    printf("\nCleaning up temporary files...\n");
    for (int i = 0; i < num_gcda_files; i++) {
        free(gcda_files[i]);
    }
    
    /* Remove all files in temp directory */
    char cleanup_cmd[512];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
    system(cleanup_cmd);
    
    return tests_failed > 0 ? 1 : 0;
}
