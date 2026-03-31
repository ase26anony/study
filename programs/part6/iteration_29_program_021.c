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

// Create a simple C program for GCOV instrumentation
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

// Function to run a command and return exit status
int run_command(const char *cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

// Function to create and compile test program
int setup_gcov_test_files(const char *temp_dir, char *gcda_files[], int num_files) {
    char cmd[MAX_CMD_LEN];
    char src_path[MAX_CMD_LEN];
    
    // Create source file
    snprintf(src_path, sizeof(src_path), "%s/test_prog.c", temp_dir);
    FILE *fp = fopen(src_path, "w");
    if (!fp) {
        perror("Failed to create source file");
        return -1;
    }
    fputs(test_program, fp);
    fclose(fp);
    
    // Compile with GCOV instrumentation
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -o %s/test_prog %s/test_prog.c 2>/dev/null", 
             temp_dir, temp_dir);
    if (run_command(cmd) != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return -1;
    }
    
    // Generate multiple .gcda files by running the program multiple times
    for (int i = 0; i < num_files; i++) {
        snprintf(cmd, sizeof(cmd), "cd %s && ./test_prog > /dev/null 2>&1", temp_dir);
        run_command(cmd);
        
        // Copy .gcda file with different name for each run
        char src_gcda[MAX_CMD_LEN];
        char dst_gcda[MAX_CMD_LEN];
        snprintf(src_gcda, sizeof(src_gcda), "%s/test_prog.gcda", temp_dir);
        snprintf(dst_gcda, sizeof(dst_gcda), "%s/test_prog_run%d.gcda", temp_dir, i);
        
        // Use system cp command for simplicity
        snprintf(cmd, sizeof(cmd), "cp %s %s", src_gcda, dst_gcda);
        run_command(cmd);
        
        gcda_files[i] = strdup(dst_gcda);
    }
    
    return 0;
}

// Function to test gcov-tool with various argument combinations
void test_overlap_options(const char *gcov_tool_path, char *gcda_files[], int num_files) {
    test_case_t tests[] = {
        // Basic tests covering all uncovered flags
        {"%s overlap -v %s %s", 0, "Verbose flag"},
        {"%s overlap -f %s %s", 0, "Function level flag"},
        {"%s overlap -F %s %s", 0, "Full filename flag"},
        {"%s overlap -o %s %s", 0, "Object level flag"},
        {"%s overlap -h %s %s", 0, "Hot only flag"},
        {"%s overlap -t 0.5 %s %s", 0, "Threshold 0.5"},
        {"%s overlap -t 1.0 %s %s", 0, "Threshold 1.0"},
        {"%s overlap -t 0.75 %s %s", 0, "Threshold 0.75"},
        
        // Combined flags - testing all uncovered cases in one command
        {"%s overlap -v -f -F -o -h -t 0.8 %s %s", 0, "All flags combined"},
        {"%s overlap -f -F -o -h -t 0.6 -v %s %s", 0, "All flags permuted 1"},
        {"%s overlap -t 0.9 -h -o -F -f -v %s %s", 0, "All flags permuted 2"},
        
        // Different flag orders and combinations
        {"%s overlap -v -f %s %s", 0, "Verbose + function level"},
        {"%s overlap -F -o %s %s", 0, "Fullname + object level"},
        {"%s overlap -h -t 0.3 %s %s", 0, "Hot only + threshold"},
        {"%s overlap -v -F -t 0.4 %s %s", 0, "Verbose + fullname + threshold"},
        
        // Edge cases for threshold
        {"%s overlap -t 0.0 %s %s", 0, "Zero threshold"},
        {"%s overlap -t 100.0 %s %s", 0, "Large threshold"},
        {"%s overlap -t 0.001 %s %s", 0, "Small threshold"},
        
        // Multiple files
        {"%s overlap -v -f %s %s %s", 0, "Three input files"},
        
        // Error cases (should trigger different code paths)
        {"%s overlap -t not_a_number %s %s", 1, "Invalid threshold (non-numeric)"},
        {"%s overlap -x %s %s", 1, "Unknown flag (should trigger default case)"},
        {"%s overlap -v -v %s %s", 0, "Repeated verbose flag"},
        {"%s overlap -f -f -F -F %s %s", 0, "Repeated flags"},
        
        // Missing argument for -t (last argument)
        {"%s overlap -t", 1, "Missing threshold argument"},
        
        // Empty command
        {"%s overlap", 1, "No arguments"},
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    
    printf("\n=== Testing gcov-tool overlap options ===\n");
    
    for (int i = 0; i < num_tests; i++) {
        char cmd[MAX_CMD_LEN];
        int expected = tests[i].expected_exit;
        
        // Format the command string with actual file paths
        if (strstr(tests[i].cmd, "%s %s %s") || strstr(tests[i].cmd, "%s %s")) {
            // Commands that need file arguments
            if (strstr(tests[i].cmd, "%s %s %s %s")) {
                snprintf(cmd, sizeof(cmd), tests[i].cmd, 
                        gcov_tool_path, gcda_files[0], gcda_files[1], gcda_files[2]);
            } else if (strstr(tests[i].cmd, "%s %s %s")) {
                snprintf(cmd, sizeof(cmd), tests[i].cmd, 
                        gcov_tool_path, gcda_files[0], gcda_files[1]);
            }
        } else {
            // Commands without file arguments (error cases)
            snprintf(cmd, sizeof(cmd), tests[i].cmd, gcov_tool_path);
        }
        
        printf("\nTest %d/%d: %s\n", i + 1, num_tests, tests[i].description);
        printf("Command: %s\n", cmd);
        
        int result = run_command(cmd);
        
        // For error cases, we don't always know the exact exit code,
        // but we expect non-zero for clearly invalid commands
        if (expected == 0) {
            if (result == 0) {
                printf("✓ PASSED\n");
                passed++;
            } else {
                printf("✗ FAILED (expected 0, got %d)\n", result);
            }
        } else {
            if (result != 0) {
                printf("✓ PASSED (expected non-zero, got %d)\n", result);
                passed++;
            } else {
                printf("✗ FAILED (expected non-zero, got 0)\n");
            }
        }
    }
    
    printf("\n=== Summary ===\n");
    printf("Passed: %d/%d tests\n", passed, num_tests);
}

int main(int argc, char *argv[]) {
    char temp_dir[] = "/tmp/gcov_test_XXXXXX";
    char *gcov_tool_path = "gcov-tool";  // Default path
    char *gcda_files[MAX_FILES];
    
    // Allow specifying gcov-tool path
    if (argc > 1) {
        gcov_tool_path = argv[1];
    }
    
    // Create temporary directory
    if (!mkdtemp(temp_dir)) {
        perror("Failed to create temporary directory");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", temp_dir);
    
    // Setup GCOV test files
    if (setup_gcov_test_files(temp_dir, gcda_files, 3) != 0) {
        fprintf(stderr, "Failed to setup GCOV test files\n");
        return 1;
    }
    
    printf("Generated GCOV data files:\n");
    for (int i = 0; i < 3; i++) {
        printf("  %s\n", gcda_files[i]);
    }
    
    // Test gcov-tool with various overlap options
    test_overlap_options(gcov_tool_path, gcda_files, 3);
    
    // Cleanup
    printf("\nCleaning up temporary files...\n");
    char cleanup_cmd[MAX_CMD_LEN];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
    run_command(cleanup_cmd);
    
    for (int i = 0; i < 3; i++) {
        free(gcda_files[i]);
    }
    
    return 0;
}
