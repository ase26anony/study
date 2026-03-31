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

// Function to execute a command and return exit status
int execute_command(const char *cmd, int capture_output) {
    printf("Executing: %s\n", cmd);
    
    if (capture_output) {
        // Redirect output to /dev/null to avoid cluttering test output
        char full_cmd[MAX_CMD_LEN];
        snprintf(full_cmd, sizeof(full_cmd), "%s > /dev/null 2>&1", cmd);
        return system(full_cmd);
    } else {
        return system(cmd);
    }
}

// Create GCOV instrumented program and generate .gcda files
int setup_gcov_test_files(const char *temp_dir, char *gcda_files[], int num_files) {
    char path[MAX_CMD_LEN];
    int i;
    
    // Write test program
    snprintf(path, sizeof(path), "%s/test_prog.c", temp_dir);
    FILE *fp = fopen(path, "w");
    if (!fp) {
        perror("Failed to create test program");
        return -1;
    }
    fprintf(fp, "%s", test_program);
    fclose(fp);
    
    // Compile with GCOV instrumentation
    snprintf(path, sizeof(path), "cd %s && gcc -fprofile-arcs -ftest-coverage test_prog.c -o test_prog", temp_dir);
    if (execute_command(path, 1) != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return -1;
    }
    
    // Generate multiple .gcda files with different execution patterns
    for (i = 0; i < num_files; i++) {
        // Run the program to generate .gcda
        snprintf(path, sizeof(path), "cd %s && ./test_prog", temp_dir);
        execute_command(path, 1);
        
        // Copy .gcda file with different name for each run
        char src_path[MAX_CMD_LEN];
        char dst_path[MAX_CMD_LEN];
        snprintf(src_path, sizeof(src_path), "%s/test_prog.gcda", temp_dir);
        snprintf(dst_path, sizeof(dst_path), "%s/test_prog_run%d.gcda", temp_dir, i);
        
        // Copy file
        FILE *src = fopen(src_path, "rb");
        FILE *dst = fopen(dst_path, "wb");
        if (src && dst) {
            char buffer[1024];
            size_t bytes;
            while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
                fwrite(buffer, 1, bytes, dst);
            }
            fclose(src);
            fclose(dst);
            
            // Store the path for later use
            gcda_files[i] = strdup(dst_path);
        } else {
            if (src) fclose(src);
            if (dst) fclose(dst);
            return -1;
        }
        
        // Also need the .gcno file
        if (i == 0) {
            snprintf(src_path, sizeof(src_path), "%s/test_prog.gcno", temp_dir);
            gcda_files[num_files] = strdup(src_path); // Store .gcno at the end
        }
    }
    
    return 0;
}

// Clean up temporary files
void cleanup_files(char *files[], int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) {
            free(files[i]);
            files[i] = NULL;
        }
    }
}

int main(int argc, char *argv[]) {
    char temp_dir[] = "/tmp/gcov_test_XXXXXX";
    char *gcda_files[MAX_FILES + 1] = {NULL}; // +1 for .gcno file
    int num_test_files = 3; // Generate 3 .gcda files
    int passed = 0, failed = 0;
    
    // Create temporary directory
    if (!mkdtemp(temp_dir)) {
        perror("Failed to create temp directory");
        return 1;
    }
    
    printf("Created temp directory: %s\n", temp_dir);
    
    // Setup GCOV test files
    if (setup_gcov_test_files(temp_dir, gcda_files, num_test_files) != 0) {
        fprintf(stderr, "Failed to setup GCOV test files\n");
        cleanup_files(gcda_files, MAX_FILES + 1);
        rmdir(temp_dir);
        return 1;
    }
    
    // Define test cases covering all uncovered switch cases
    test_case_t test_cases[] = {
        // Basic tests for each individual flag
        {"gcov-tool overlap -v %s %s", 0, "Verbose flag (-v)"},
        {"gcov-tool overlap -f %s %s", 0, "Function level flag (-f)"},
        {"gcov-tool overlap -F %s %s", 0, "Full filename flag (-F)"},
        {"gcov-tool overlap -o %s %s", 0, "Object level flag (-o)"},
        {"gcov-tool overlap -h %s %s", 0, "Hot only flag (-h)"},
        {"gcov-tool overlap -t 0.5 %s %s", 0, "Threshold flag with value (-t 0.5)"},
        
        // Combined flags - testing all uncovered cases in one command
        {"gcov-tool overlap -v -f -F -o -h -t 0.75 %s %s", 0, "All flags combined"},
        
        // Different orders of flags
        {"gcov-tool overlap -t 1.0 -h -o -F -f -v %s %s", 0, "All flags reverse order"},
        {"gcov-tool overlap -f -v -t 0.25 -h -F -o %s %s", 0, "All flags mixed order"},
        
        // Multiple instances of same flag
        {"gcov-tool overlap -v -v -v %s %s", 0, "Multiple verbose flags"},
        
        // Edge cases for threshold
        {"gcov-tool overlap -t 0.0 %s %s", 0, "Zero threshold"},
        {"gcov-tool overlap -t 1.0 %s %s", 0, "One threshold"},
        {"gcov-tool overlap -t 100.5 %s %s", 0, "Large threshold"},
        {"gcov-tool overlap -t 0.001 %s %s", 0, "Small threshold"},
        
        // Invalid cases (should trigger errors but still parse the flags)
        {"gcov-tool overlap -t not_a_number %s %s", 256, "Invalid threshold (non-numeric)"},
        
        // Missing argument for -t (last argument)
        {"gcov-tool overlap -t", 256, "Missing threshold argument"},
        
        // Unknown flag (should trigger default case)
        {"gcov-tool overlap -x %s %s", 256, "Unknown flag (-x)"},
        
        // Combination with unknown flag
        {"gcov-tool overlap -v -f -x -F %s %s", 256, "Valid flags with unknown flag"},
        
        // Empty command (just overlap)
        {"gcov-tool overlap", 256, "No input files"},
        
        // Single file
        {"gcov-tool overlap -v %s", 0, "Single file with verbose"},
        
        // Three files
        {"gcov-tool overlap -v -f %s %s %s", 0, "Three files with verbose and function level"},
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    // Run test cases
    for (int i = 0; i < num_cases; i++) {
        char cmd[MAX_CMD_LEN];
        int exit_status;
        
        // Format command with appropriate number of files
        if (strstr(test_cases[i].cmd, "%s %s %s")) {
            snprintf(cmd, sizeof(cmd), test_cases[i].cmd, 
                    gcda_files[0], gcda_files[1], gcda_files[2]);
        } else if (strstr(test_cases[i].cmd, "%s %s")) {
            snprintf(cmd, sizeof(cmd), test_cases[i].cmd, 
                    gcda_files[0], gcda_files[1]);
        } else if (strstr(test_cases[i].cmd, "%s")) {
            snprintf(cmd, sizeof(cmd), test_cases[i].cmd, gcda_files[0]);
        } else {
            snprintf(cmd, sizeof(cmd), "%s", test_cases[i].cmd);
        }
        
        // Execute command
        exit_status = execute_command(cmd, 1);
        
        // Check result (WEXITSTATUS for system() return)
        int actual_exit = WEXITSTATUS(exit_status);
        
        // For invalid commands, we expect non-zero exit
        // For valid commands, we expect 0 exit (success)
        int expected = test_cases[i].expected_exit;
        
        // Check if test passed
        // Note: We're checking if command was executed (parsed), not necessarily if it succeeded
        // The important part is that the parsing code was reached
        if ((expected == 0 && actual_exit == 0) || 
            (expected != 0 && actual_exit != 0)) {
            printf("✓ PASS: %s\n", test_cases[i].description);
            passed++;
        } else {
            printf("✗ FAIL: %s (expected exit: %d, got: %d)\n", 
                   test_cases[i].description, expected, actual_exit);
            failed++;
        }
        
        // Small delay to avoid overwhelming the system
        usleep(10000);
    }
    
    // Additional permutation tests
    printf("\n=== Running flag permutation tests ===\n");
    
    // Generate permutations of the 6 main flags
    char *flags[] = {"-v", "-f", "-F", "-o", "-h", "-t 0.5"};
    int num_flags = 6;
    
    // Test different subsets and orders
    for (int subset_size = 1; subset_size <= num_flags; subset_size++) {
        // Simple approach: test first N flags in different orders
        char perm_cmd[MAX_CMD_LEN];
        strcpy(perm_cmd, "gcov-tool overlap");
        
        // Add flags
        for (int j = 0; j < subset_size; j++) {
            strcat(perm_cmd, " ");
            strcat(perm_cmd, flags[j]);
        }
        
        // Add files
        char full_cmd[MAX_CMD_LEN];
        snprintf(full_cmd, sizeof(full_cmd), "%s %s %s", 
                perm_cmd, gcda_files[0], gcda_files[1]);
        
        printf("Testing: %s\n", perm_cmd);
        execute_command(full_cmd, 1);
        
        // Also test in reverse order
        strcpy(perm_cmd, "gcov-tool overlap");
        for (int j = subset_size - 1; j >= 0; j--) {
            strcat(perm_cmd, " ");
            strcat(perm_cmd, flags[j]);
        }
        
        snprintf(full_cmd, sizeof(full_cmd), "%s %s %s", 
                perm_cmd, gcda_files[0], gcda_files[1]);
        
        printf("Testing (reverse): %s\n", perm_cmd);
        execute_command(full_cmd, 1);
    }
    
    // Test with absolute paths
    printf("\n=== Testing with absolute paths ===\n");
    char abs_cmd[MAX_CMD_LEN];
    snprintf(abs_cmd, sizeof(abs_cmd), 
             "gcov-tool overlap -v -f -F %s %s %s",
             gcda_files[0], gcda_files[1], gcda_files[num_files]); // Include .gcno file
    execute_command(abs_cmd, 0);
    
    // Test with relative paths from different directories
    printf("\n=== Testing from different directory ===\n");
    char rel_cmd[MAX_CMD_LEN];
    snprintf(rel_cmd, sizeof(rel_cmd),
             "cd / && gcov-tool overlap -v -o %s/test_prog_run0.gcda %s/test_prog_run1.gcda",
             temp_dir, temp_dir);
    execute_command(rel_cmd, 1);
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", num_cases);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    
    // Cleanup
    cleanup_files(gcda_files, MAX_FILES + 1);
    
    // Remove temp directory
    char cleanup_cmd[MAX_CMD_LEN];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
    execute_command(cleanup_cmd, 1);
    
    printf("\nTemporary directory cleaned up: %s\n", temp_dir);
    
    return (failed > 0) ? 1 : 0;
}
