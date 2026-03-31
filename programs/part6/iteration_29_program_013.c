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
    char *cmd;
    int expected_exit;
    char *description;
} test_case_t;

// Global variables to track test results
static int tests_passed = 0;
static int tests_failed = 0;

// Function to create a simple instrumented C program
void create_test_program(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create test program");
        exit(1);
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
}

// Function to compile with GCOV instrumentation
int compile_with_gcov(const char *source, const char *output) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s 2>/dev/null",
             output, source);
    return system(cmd);
}

// Function to run a program and generate .gcda files
int run_program(const char *program, const char *gcda_prefix) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), 
             "GCOV_PREFIX=%s GCOV_PREFIX_STRIP=0 ./%s >/dev/null 2>&1",
             gcda_prefix, program);
    return system(cmd);
}

// Function to execute a test command and check result
void execute_test(const char *cmd, int expected_exit, const char *description) {
    printf("Test: %s\n", description);
    printf("Command: %s\n", cmd);
    
    int status = system(cmd);
    int exit_code = WEXITSTATUS(status);
    
    if (exit_code == expected_exit) {
        printf("✓ PASSED (exit code: %d)\n\n", exit_code);
        tests_passed++;
    } else {
        printf("✗ FAILED (expected: %d, got: %d)\n\n", expected_exit, exit_code);
        tests_failed++;
    }
}

// Function to create multiple .gcda files with different profiles
void create_gcov_data_files(const char *temp_dir, char *gcda_files[], int count) {
    char source_file[256];
    char program_file[256];
    char gcda_dir[256];
    
    // Create and compile test program
    snprintf(source_file, sizeof(source_file), "%s/test.c", temp_dir);
    snprintf(program_file, sizeof(program_file), "%s/test_prog", temp_dir);
    
    create_test_program(source_file);
    compile_with_gcov(source_file, program_file);
    
    // Change to temp directory to run programs
    char original_dir[256];
    getcwd(original_dir, sizeof(original_dir));
    chdir(temp_dir);
    
    // Generate multiple .gcda files with different run counts
    for (int i = 0; i < count; i++) {
        snprintf(gcda_dir, sizeof(gcda_dir), "run%d", i);
        mkdir(gcda_dir, 0755);
        
        // Run program multiple times to create different profiles
        for (int j = 0; j <= i; j++) {
            run_program("test_prog", gcda_dir);
        }
        
        // Find the .gcda file
        char find_cmd[512];
        snprintf(find_cmd, sizeof(find_cmd),
                 "find %s -name '*.gcda' | head -1", gcda_dir);
        
        FILE *fp = popen(find_cmd, "r");
        if (fp) {
            char gcda_path[256];
            if (fgets(gcda_path, sizeof(gcda_path), fp)) {
                // Remove newline
                gcda_path[strcspn(gcda_path, "\n")] = 0;
                gcda_files[i] = strdup(gcda_path);
            }
            pclose(fp);
        }
    }
    
    // Change back to original directory
    chdir(original_dir);
}

int main(int argc, char *argv[]) {
    char temp_dir[256];
    char *gcda_files[MAX_FILES] = {NULL};
    
    // Create temporary directory
    strcpy(temp_dir, TEMP_DIR_PATTERN);
    if (mkdtemp(temp_dir) == NULL) {
        perror("Failed to create temp directory");
        return 1;
    }
    
    printf("Created temp directory: %s\n", temp_dir);
    
    // Create GCOV data files
    create_gcov_data_files(temp_dir, gcda_files, 3);
    
    // Check if we have enough .gcda files
    int valid_gcda_count = 0;
    for (int i = 0; i < 3; i++) {
        if (gcda_files[i]) {
            valid_gcda_count++;
        }
    }
    
    if (valid_gcda_count < 2) {
        fprintf(stderr, "Failed to create enough .gcda files\n");
        return 1;
    }
    
    printf("Created %d .gcda files for testing\n\n", valid_gcda_count);
    
    // Test cases covering the uncovered switch cases
    test_case_t test_cases[] = {
        // Basic tests for each flag
        {"gcov-tool overlap -v %s %s", 0, "Verbose flag (-v)"},
        {"gcov-tool overlap -f %s %s", 0, "Function level flag (-f)"},
        {"gcov-tool overlap -F %s %s", 0, "Full filename flag (-F)"},
        {"gcov-tool overlap -o %s %s", 0, "Object level flag (-o)"},
        {"gcov-tool overlap -h %s %s", 0, "Hot only flag (-h)"},
        {"gcov-tool overlap -t 0.5 %s %s", 0, "Threshold flag with value (-t 0.5)"},
        
        // Combined flags - testing all uncovered cases in one command
        {"gcov-tool overlap -v -f -F -o -h -t 0.75 %s %s", 0, 
         "All flags combined in alphabetical order"},
        {"gcov-tool overlap -t 1.0 -h -o -F -f -v %s %s", 0,
         "All flags combined in reverse order"},
        
        // Different threshold values
        {"gcov-tool overlap -t 0.1 %s %s", 0, "Low threshold (0.1)"},
        {"gcov-tool overlap -t 0.9 %s %s", 0, "High threshold (0.9)"},
        {"gcov-tool overlap -t 1.0 %s %s", 0, "Threshold at 1.0"},
        {"gcov-tool overlap -t 0.0 %s %s", 0, "Threshold at 0.0"},
        
        // Flag permutations (different orders)
        {"gcov-tool overlap -f -v -t 0.5 %s %s", 0, "Flags -f -v -t 0.5"},
        {"gcov-tool overlap -F -o -h %s %s", 0, "Flags -F -o -h"},
        {"gcov-tool overlap -v -t 0.3 -f -F %s %s", 0, "Flags -v -t 0.3 -f -F"},
        
        // Edge cases for threshold
        {"gcov-tool overlap -t .5 %s %s", 0, "Threshold without leading zero (.5)"},
        {"gcov-tool overlap -t 1.5 %s %s", 0, "Threshold above 1.0 (1.5)"},
        
        // Multiple repetitions of same flag
        {"gcov-tool overlap -v -v -v %s %s", 0, "Multiple -v flags"},
        {"gcov-tool overlap -f -f -t 0.5 -f %s %s", 0, "Multiple -f flags"},
        
        // Mixed with other overlap options
        {"gcov-tool overlap -v --function-summary %s %s", 0, 
         "-v with long option --function-summary"},
        {"gcov-tool overlap -f --object-summary %s %s", 0,
         "-f with long option --object-summary"},
    };
    
    // Error test cases (should fail)
    test_case_t error_cases[] = {
        {"gcov-tool overlap -t not_a_number %s %s", 1, 
         "Invalid threshold (non-numeric) - should fail"},
        {"gcov-tool overlap -t %s %s", 1, 
         "Missing threshold value - should fail"},
        {"gcov-tool overlap -x %s %s", 1, 
         "Unknown flag (-x) - should trigger default case"},
        {"gcov-tool overlap -t", 1, 
         "-t without argument at end - should fail"},
    };
    
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int num_error_tests = sizeof(error_cases) / sizeof(error_cases[0]);
    
    printf("=== Testing Valid Commands ===\n\n");
    
    // Execute valid test cases
    for (int i = 0; i < num_tests; i++) {
        char cmd[MAX_CMD_LEN];
        snprintf(cmd, sizeof(cmd), test_cases[i].cmd, 
                 gcda_files[0], gcda_files[1]);
        execute_test(cmd, test_cases[i].expected_exit, test_cases[i].description);
    }
    
    printf("\n=== Testing Error Conditions ===\n\n");
    
    // Execute error test cases
    for (int i = 0; i < num_error_tests; i++) {
        char cmd[MAX_CMD_LEN];
        
        // Handle different formats for error cases
        if (strstr(error_cases[i].cmd, "%s")) {
            snprintf(cmd, sizeof(cmd), error_cases[i].cmd, 
                     gcda_files[0], gcda_files[1]);
        } else {
            strncpy(cmd, error_cases[i].cmd, sizeof(cmd));
        }
        
        execute_test(cmd, error_cases[i].expected_exit, error_cases[i].description);
    }
    
    // Additional test: Use absolute paths
    printf("\n=== Testing with Absolute Paths ===\n\n");
    char abs_cmd[MAX_CMD_LEN];
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    
    // Create absolute paths for .gcda files
    char abs_gcda1[512], abs_gcda2[512];
    snprintf(abs_gcda1, sizeof(abs_gcda1), "%s/%s", cwd, gcda_files[0]);
    snprintf(abs_gcda2, sizeof(abs_gcda2), "%s/%s", cwd, gcda_files[1]);
    
    snprintf(abs_cmd, sizeof(abs_cmd), 
             "gcov-tool overlap -v -f -F -o -h -t 0.5 %s %s",
             abs_gcda1, abs_gcda2);
    execute_test(abs_cmd, 0, "All flags with absolute paths");
    
    // Test with more than 2 files
    printf("\n=== Testing with Multiple Files ===\n\n");
    char multi_cmd[MAX_CMD_LEN];
    snprintf(multi_cmd, sizeof(multi_cmd),
             "gcov-tool overlap -v -f %s %s %s",
             gcda_files[0], gcda_files[1], gcda_files[2]);
    execute_test(multi_cmd, 0, "Three input files with -v -f flags");
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", tests_passed + tests_failed);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    
    // Cleanup
    printf("\nCleaning up temporary directory: %s\n", temp_dir);
    char cleanup_cmd[512];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
    system(cleanup_cmd);
    
    // Free allocated memory
    for (int i = 0; i < 3; i++) {
        if (gcda_files[i]) {
            free(gcda_files[i]);
        }
    }
    
    return tests_failed > 0 ? 1 : 0;
}
