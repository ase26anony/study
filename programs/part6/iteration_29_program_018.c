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
    const char *description;
} test_case_t;

// Create a simple C program for GCOV instrumentation
const char *test_program = 
    "#include <stdio.h>\n"
    "int main() {\n"
    "    int i;\n"
    "    for (i = 0; i < 10; i++) {\n"
    "        printf(\"Test %d\\n\", i);\n"
    "    }\n"
    "    return 0;\n"
    "}\n";

// Function to execute a command and return exit status
int execute_command(const char *cmd, int capture_output) {
    if (capture_output) {
        char full_cmd[MAX_CMD_LEN];
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
        return system(full_cmd);
    }
    return system(cmd);
}

// Function to compile and run instrumented program
int generate_gcda_files(const char *temp_dir, char *gcda_files[], int num_files) {
    char cmd[MAX_CMD_LEN];
    int i;
    
    // Create test source file
    char src_path[MAX_CMD_LEN];
    snprintf(src_path, sizeof(src_path), "%s/test.c", temp_dir);
    
    FILE *fp = fopen(src_path, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return -1;
    }
    fprintf(fp, "%s", test_program);
    fclose(fp);
    
    // Compile with GCOV instrumentation
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -o %s/test_prog %s/test.c 2>&1", 
             temp_dir, temp_dir);
    
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return -1;
    }
    
    // Generate multiple .gcda files by running the program multiple times
    for (i = 0; i < num_files; i++) {
        // Run the program
        snprintf(cmd, sizeof(cmd), "cd %s && ./test_prog > /dev/null 2>&1", temp_dir);
        system(cmd);
        
        // Copy .gcda file with different name for each run
        char gcda_path[MAX_CMD_LEN];
        snprintf(gcda_path, sizeof(gcda_path), "%s/test%d.gcda", temp_dir, i);
        
        // Rename the default .gcda file
        snprintf(cmd, sizeof(cmd), "cp %s/test.gcda %s 2>/dev/null", temp_dir, gcda_path);
        system(cmd);
        
        // Also copy .gcno file
        char gcno_path[MAX_CMD_LEN];
        snprintf(gcno_path, sizeof(gcno_path), "%s/test%d.gcno", temp_dir, i);
        snprintf(cmd, sizeof(cmd), "cp %s/test.gcno %s 2>/dev/null", temp_dir, gcno_path);
        system(cmd);
        
        gcda_files[i] = strdup(gcda_path);
    }
    
    return 0;
}

// Clean up temporary files
void cleanup_files(char *gcda_files[], int num_files, const char *temp_dir) {
    int i;
    for (i = 0; i < num_files; i++) {
        if (gcda_files[i]) {
            free(gcda_files[i]);
        }
    }
    
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
    system(cmd);
}

int main(int argc, char *argv[]) {
    char temp_dir[] = "/tmp/gcov_test_XXXXXX";
    char *gcda_files[MAX_FILES] = {NULL};
    int num_test_files = 2;  // Need at least 2 files for overlap analysis
    int i, status;
    int passed = 0, failed = 0;
    
    // Create temporary directory
    if (!mkdtemp(temp_dir)) {
        perror("Failed to create temporary directory");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", temp_dir);
    
    // Generate GCOV data files
    if (generate_gcda_files(temp_dir, gcda_files, num_test_files) != 0) {
        cleanup_files(gcda_files, num_test_files, temp_dir);
        return 1;
    }
    
    printf("Generated %d GCOV data files\n", num_test_files);
    
    // Define test cases for the uncovered switch statement
    test_case_t test_cases[] = {
        // Basic tests for each flag
        {"gcov-tool overlap -v %s %s", 0, "Verbose flag (-v)"},
        {"gcov-tool overlap -f %s %s", 0, "Function level flag (-f)"},
        {"gcov-tool overlap -F %s %s", 0, "Full filename flag (-F)"},
        {"gcov-tool overlap -o %s %s", 0, "Object level flag (-o)"},
        {"gcov-tool overlap -h %s %s", 0, "Hot only flag (-h)"},
        {"gcov-tool overlap -t 0.5 %s %s", 0, "Hot threshold flag (-t 0.5)"},
        {"gcov-tool overlap -t 1.0 %s %s", 0, "Hot threshold flag (-t 1.0)"},
        {"gcov-tool overlap -t 0.75 %s %s", 0, "Hot threshold flag (-t 0.75)"},
        
        // Combined flags - testing all uncovered cases in one command
        {"gcov-tool overlap -v -f -F -o -h -t 0.8 %s %s", 0, "All flags combined"},
        
        // Different permutations of flags
        {"gcov-tool overlap -f -F -v -o -h -t 0.6 %s %s", 0, "Flags permutation 1"},
        {"gcov-tool overlap -h -t 0.9 -v -f -F -o %s %s", 0, "Flags permutation 2"},
        {"gcov-tool overlap -t 0.3 -h -o -F -f -v %s %s", 0, "Flags permutation 3"},
        
        // Edge cases for -t flag
        {"gcov-tool overlap -t 0.0 %s %s", 0, "Zero threshold"},
        {"gcov-tool overlap -t 100.0 %s %s", 0, "Large threshold"},
        {"gcov-tool overlap -t 0.001 %s %s", 0, "Small threshold"},
        
        // Repeated flags
        {"gcov-tool overlap -v -v -v %s %s", 0, "Repeated verbose flag"},
        {"gcov-tool overlap -f -f -h -h %s %s", 0, "Multiple repeated flags"},
        
        // Invalid cases (should trigger errors but still exercise parsing)
        {"gcov-tool overlap -t not_a_number %s %s", 1, "Invalid number for -t (should fail)"},
        
        // Missing argument for -t (last argument)
        {"gcov-tool overlap -t", 1, "Missing argument for -t"},
        
        // Unknown flag (should trigger default case)
        {"gcov-tool overlap -x %s %s", 1, "Unknown flag -x"},
        
        // Mixed valid and invalid
        {"gcov-tool overlap -v -x -f %s %s", 1, "Mixed valid and invalid flags"},
        
        // Empty flag (just test files)
        {"gcov-tool overlap %s %s", 0, "No flags, just files"},
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    printf("\n=== Running gcov-tool overlap tests ===\n\n");
    
    for (i = 0; i < num_cases; i++) {
        char cmd[MAX_CMD_LEN];
        
        // Format the command with actual file paths
        if (strstr(test_cases[i].cmd, "%s")) {
            // Commands that expect file arguments
            snprintf(cmd, sizeof(cmd), test_cases[i].cmd, 
                    gcda_files[0], gcda_files[1]);
        } else {
            // Commands without file arguments (error cases)
            strncpy(cmd, test_cases[i].cmd, sizeof(cmd));
        }
        
        printf("Test %2d: %s\n", i + 1, test_cases[i].description);
        printf("  Command: %s\n", cmd);
        
        // Execute the command
        status = execute_command(cmd, 1);
        int exit_status = WEXITSTATUS(status);
        
        // Check if result matches expectation
        if ((test_cases[i].expected_exit == 0 && exit_status == 0) ||
            (test_cases[i].expected_exit != 0 && exit_status != 0)) {
            printf("  Result: PASS (exit code: %d)\n", exit_status);
            passed++;
        } else {
            printf("  Result: FAIL (expected %d, got %d)\n", 
                   test_cases[i].expected_exit, exit_status);
            failed++;
        }
        printf("\n");
    }
    
    // Additional test: Use absolute paths
    printf("=== Testing with absolute paths ===\n");
    char abs_cmd[MAX_CMD_LEN];
    char abs_path1[MAX_CMD_LEN], abs_path2[MAX_CMD_LEN];
    
    // Get absolute paths
    realpath(gcda_files[0], abs_path1);
    realpath(gcda_files[1], abs_path2);
    
    snprintf(abs_cmd, sizeof(abs_cmd), 
             "gcov-tool overlap -v -f -F -o -h -t 0.5 %s %s",
             abs_path1, abs_path2);
    
    printf("Command: %s\n", abs_cmd);
    status = execute_command(abs_cmd, 1);
    printf("Exit code: %d\n\n", WEXITSTATUS(status));
    
    // Test with .gcno files as well
    printf("=== Testing with .gcno files ===\n");
    char gcno_cmd[MAX_CMD_LEN];
    snprintf(gcno_cmd, sizeof(gcno_cmd),
             "gcov-tool overlap -v -f %s/test0.gcno %s/test1.gcno",
             temp_dir, temp_dir);
    
    printf("Command: %s\n", gcno_cmd);
    status = execute_command(gcno_cmd, 1);
    printf("Exit code: %d\n\n", WEXITSTATUS(status));
    
    // Summary
    printf("=== Test Summary ===\n");
    printf("Total tests: %d\n", num_cases + 2);  // +2 for additional tests
    printf("Passed: %d\n", passed + (WEXITSTATUS(status) == 0 ? 1 : 0));
    printf("Failed: %d\n", failed + (WEXITSTATUS(status) != 0 ? 1 : 0));
    
    // Cleanup
    cleanup_files(gcda_files, num_test_files, temp_dir);
    
    return (failed > 0) ? 1 : 0;
}
