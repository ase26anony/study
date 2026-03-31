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

typedef struct {
    char *cmd;
    int expected_exit;
    char *description;
} test_case_t;

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

/* Run a command and return exit status */
int run_command(const char *cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Create a temporary directory */
char *create_temp_dir() {
    char *template = "/tmp/gcov_test_XXXXXX";
    char *dir = strdup(template);
    if (mkdtemp(dir) == NULL) {
        perror("mkdtemp failed");
        free(dir);
        return NULL;
    }
    return dir;
}

/* Generate GCOV data files */
int generate_gcov_data(const char *dir, int num_files) {
    char path[256];
    char cmd[512];
    
    /* Write test program */
    snprintf(path, sizeof(path), "%s/test.c", dir);
    FILE *fp = fopen(path, "w");
    if (!fp) {
        perror("Failed to create test.c");
        return 0;
    }
    fputs(test_program, fp);
    fclose(fp);
    
    /* Compile with GCOV instrumentation */
    snprintf(cmd, sizeof(cmd), "cd %s && gcc -fprofile-arcs -ftest-coverage test.c -o test", dir);
    if (run_command(cmd) != 0) {
        fprintf(stderr, "Compilation failed\n");
        return 0;
    }
    
    /* Run multiple times to generate different .gcda files */
    for (int i = 0; i < num_files; i++) {
        snprintf(cmd, sizeof(cmd), "cd %s && ./test > /dev/null", dir);
        if (run_command(cmd) != 0) {
            fprintf(stderr, "Execution %d failed\n", i);
            return 0;
        }
        
        /* Rename gcda file to preserve it */
        if (i < num_files - 1) {
            snprintf(cmd, sizeof(cmd), "cd %s && cp test.gcda test%d.gcda", dir, i);
            run_command(cmd);
        }
    }
    
    return 1;
}

/* Test the specific uncovered switch cases */
void test_overlap_flags(const char *dir, const char *gcov_tool_path) {
    char cmd[MAX_CMD_LEN];
    char gcda_files[256];
    
    /* Prepare gcda file list */
    snprintf(gcda_files, sizeof(gcda_files), "%s/test.gcda %s/test0.gcda", dir, dir);
    
    /* Test cases covering all uncovered switch cases */
    test_case_t tests[] = {
        /* Basic test with all flags - triggers all case statements */
        {"%s overlap -v -f -F -o -h -t 0.75 %s", 0, "All flags combined"},
        
        /* Test each flag individually */
        {"%s overlap -v %s", 0, "Verbose flag only"},
        {"%s overlap -f %s", 0, "Function level flag only"},
        {"%s overlap -F %s", 0, "Full filename flag only"},
        {"%s overlap -o %s", 0, "Object level flag only"},
        {"%s overlap -h %s", 0, "Hot only flag only"},
        {"%s overlap -t 0.5 %s", 0, "Threshold flag only"},
        
        /* Different threshold values */
        {"%s overlap -t 0.1 %s", 0, "Low threshold"},
        {"%s overlap -t 1.0 %s", 0, "Threshold 1.0"},
        {"%s overlap -t 99.9 %s", 0, "High threshold"},
        
        /* Flag combinations */
        {"%s overlap -v -f -F %s", 0, "Verbose + function + fullname"},
        {"%s overlap -o -h -t 0.8 %s", 0, "Object + hot + threshold"},
        {"%s overlap -f -o -h %s", 0, "Function + object + hot"},
        
        /* Different flag orders (permutations) */
        {"%s overlap -t 0.6 -h -o -F -f -v %s", 0, "Reverse flag order"},
        {"%s overlap -F -v -t 0.3 -f -o -h %s", 0, "Mixed flag order 1"},
        {"%s overlap -h -t 0.9 -o -F -v -f %s", 0, "Mixed flag order 2"},
        
        /* Repeated flags */
        {"%s overlap -v -v -v %s", 0, "Repeated verbose flag"},
        {"%s overlap -f -f -t 0.5 -t 0.7 %s", 0, "Repeated function and threshold flags"},
        
        /* Edge cases and error conditions */
        {"%s overlap -t not_a_number %s", 1, "Invalid threshold (should fail)"},
        {"%s overlap -t %s", 1, "Missing threshold value (should fail)"},
        {"%s overlap -x %s", 1, "Unknown flag (should fail)"},
        
        /* Empty command (just overlap) */
        {"%s overlap %s", 0, "No flags"},
        
        {NULL, 0, NULL}  /* Sentinel */
    };
    
    printf("\n=== Testing gcov-tool overlap options ===\n");
    
    int passed = 0;
    int total = 0;
    
    for (int i = 0; tests[i].cmd != NULL; i++) {
        total++;
        
        /* Build the command */
        snprintf(cmd, sizeof(cmd), tests[i].cmd, gcov_tool_path, gcda_files);
        
        /* Run the command */
        int exit_code = run_command(cmd);
        
        /* Check result */
        if ((tests[i].expected_exit == 0 && exit_code == 0) ||
            (tests[i].expected_exit != 0 && exit_code != 0)) {
            printf("✓ PASS: %s\n", tests[i].description);
            passed++;
        } else {
            printf("✗ FAIL: %s (expected %d, got %d)\n", 
                   tests[i].description, tests[i].expected_exit, exit_code);
        }
        
        /* Small delay to avoid overwhelming the system */
        usleep(10000);
    }
    
    printf("\n=== Summary: %d/%d tests passed ===\n", passed, total);
}

/* Additional test with different file paths */
void test_file_path_variations(const char *dir, const char *gcov_tool_path) {
    char cmd[MAX_CMD_LEN];
    
    printf("\n=== Testing file path variations ===\n");
    
    /* Test with absolute paths */
    char abs_path1[256], abs_path2[256];
    snprintf(abs_path1, sizeof(abs_path1), "%s/test.gcda", dir);
    snprintf(abs_path2, sizeof(abs_path2), "%s/test0.gcda", dir);
    
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f %s %s", 
             gcov_tool_path, abs_path1, abs_path2);
    printf("Testing absolute paths: %s\n", cmd);
    run_command(cmd);
    
    /* Test with relative paths from different directories */
    snprintf(cmd, sizeof(cmd), "cd %s && %s overlap -F -o test.gcda test0.gcda", 
             dir, gcov_tool_path);
    printf("Testing relative paths: cd %s && ...\n", dir);
    run_command(cmd);
    
    /* Test with more than 2 files */
    snprintf(cmd, sizeof(cmd), "%s overlap -v %s/test.gcda %s/test0.gcda %s/test.gcno", 
             gcov_tool_path, dir, dir, dir);
    printf("Testing with 3+ files: %s\n", cmd);
    run_command(cmd);
}

int main(int argc, char *argv[]) {
    char *temp_dir = NULL;
    const char *gcov_tool_path = "gcov-tool";
    
    /* Allow custom gcov-tool path via command line */
    if (argc > 1) {
        gcov_tool_path = argv[1];
    }
    
    printf("Using gcov-tool at: %s\n", gcov_tool_path);
    
    /* Verify gcov-tool exists and has overlap subcommand */
    char test_cmd[256];
    snprintf(test_cmd, sizeof(test_cmd), "%s --help > /dev/null 2>&1", gcov_tool_path);
    if (run_command(test_cmd) != 0) {
        fprintf(stderr, "Error: gcov-tool not found or not executable at '%s'\n", gcov_tool_path);
        fprintf(stderr, "Please build gcov-tool with coverage: ./configure --enable-coverage && make\n");
        return 1;
    }
    
    /* Create temporary directory */
    temp_dir = create_temp_dir();
    if (!temp_dir) {
        return 1;
    }
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Generate GCOV data files */
    printf("Generating GCOV data files...\n");
    if (!generate_gcov_data(temp_dir, 2)) {
        fprintf(stderr, "Failed to generate GCOV data\n");
        free(temp_dir);
        return 1;
    }
    
    /* Run the main tests */
    test_overlap_flags(temp_dir, gcov_tool_path);
    
    /* Test file path variations */
    test_file_path_variations(temp_dir, gcov_tool_path);
    
    /* Cleanup */
    printf("\nCleaning up...\n");
    char cleanup_cmd[256];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
    run_command(cleanup_cmd);
    free(temp_dir);
    
    printf("\n=== Test completed ===\n");
    printf("To collect coverage data for gcov-tool.cc:\n");
    printf("1. Ensure gcov-tool was built with --enable-coverage\n");
    printf("2. Run: gcov gcov-tool.cc\n");
    printf("3. Check gcov-tool.cc.gcov for coverage of lines 534-554\n");
    
    return 0;
}
