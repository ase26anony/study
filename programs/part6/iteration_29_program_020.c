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
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

// Create and compile test program with GCOV instrumentation
int setup_gcov_test_files(const char *temp_dir) {
    char cmd[MAX_CMD_LEN];
    
    // Create test source file
    char src_path[MAX_CMD_LEN];
    snprintf(src_path, sizeof(src_path), "%s/test_prog.c", temp_dir);
    
    FILE *fp = fopen(src_path, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return -1;
    }
    fwrite(test_program, 1, strlen(test_program), fp);
    fclose(fp);
    
    // Compile with GCOV instrumentation
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -o %s/test_prog %s/test_prog.c 2>/dev/null",
             temp_dir, temp_dir);
    
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return -1;
    }
    
    // Run the program multiple times to generate different .gcda files
    for (int i = 0; i < 3; i++) {
        snprintf(cmd, sizeof(cmd), 
                 "cd %s && ./test_prog > /dev/null 2>&1 && "
                 "cp test_prog.gcda test_prog%d.gcda",
                 temp_dir, i + 1);
        execute_command(cmd);
    }
    
    return 0;
}

// Generate test cases for the overlap subcommand
void generate_test_cases(test_case_t *tests, const char *temp_dir, int *test_count) {
    int idx = 0;
    
    // Base command with multiple .gcda files
    char base_cmd[MAX_CMD_LEN];
    snprintf(base_cmd, sizeof(base_cmd), 
             "gcov-tool overlap %s/test_prog1.gcda %s/test_prog2.gcda",
             temp_dir, temp_dir);
    
    // Test 1: All flags combined in one command (triggers all case statements)
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN, "%s -v -f -F -o -h -t 0.75", base_cmd);
    tests[idx].expected_exit = 0;
    tests[idx].description = "All flags combined (-v -f -F -o -h -t 0.75)";
    idx++;
    
    // Test 2: Different order of flags
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN, "%s -t 1.0 -h -o -F -f -v", base_cmd);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Flags in reverse order";
    idx++;
    
    // Test 3: Only verbose flag
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN, "%s -v", base_cmd);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Only verbose flag (-v)";
    idx++;
    
    // Test 4: Function level and fullname flags
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN, "%s -f -F", base_cmd);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Function level and fullname flags (-f -F)";
    idx++;
    
    // Test 5: Object level and hot only
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN, "%s -o -h", base_cmd);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Object level and hot only (-o -h)";
    idx++;
    
    // Test 6: Different threshold values
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN, "%s -t 0.5", base_cmd);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Threshold 0.5 (-t 0.5)";
    idx++;
    
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN, "%s -t 0.0", base_cmd);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Threshold 0.0 (-t 0.0)";
    idx++;
    
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN, "%s -t 99.9", base_cmd);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Threshold 99.9 (-t 99.9)";
    idx++;
    
    // Test 7: Repeated flags
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN, "%s -v -v -v", base_cmd);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Repeated verbose flag (-v -v -v)";
    idx++;
    
    // Test 8: Edge case - invalid threshold (should still parse but may fail later)
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN, "%s -t not_a_number", base_cmd);
    tests[idx].expected_exit = 1;  // Expected to fail
    tests[idx].description = "Invalid threshold (triggers atof)";
    idx++;
    
    // Test 9: Missing argument for -t (should trigger error handling)
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN, "%s -t", base_cmd);
    tests[idx].expected_exit = 1;  // Expected to fail
    tests[idx].description = "Missing threshold argument";
    idx++;
    
    // Test 10: Unknown flag (should trigger default case)
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN, "%s -x", base_cmd);
    tests[idx].expected_exit = 1;  // Expected to fail
    tests[idx].description = "Unknown flag (triggers default case)";
    idx++;
    
    // Test 11: Combination with unknown flag
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN, "%s -v -f -x -F", base_cmd);
    tests[idx].expected_exit = 1;  // Expected to fail
    tests[idx].description = "Valid flags with unknown flag";
    idx++;
    
    // Test 12: Absolute paths for files
    char abs_path1[MAX_CMD_LEN], abs_path2[MAX_CMD_LEN];
    realpath(temp_dir, abs_path1);
    snprintf(abs_path2, sizeof(abs_path2), "%s/test_prog1.gcda", abs_path1);
    char abs_path3[MAX_CMD_LEN];
    snprintf(abs_path3, sizeof(abs_path3), "%s/test_prog2.gcda", abs_path1);
    
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN, "gcov-tool overlap -v -f %s %s", 
             abs_path2, abs_path3);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Absolute file paths with -v -f";
    idx++;
    
    // Test 13: Single file (edge case)
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN, "gcov-tool overlap -v %s/test_prog1.gcda", temp_dir);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Single file with verbose flag";
    idx++;
    
    // Test 14: Three files with all options
    tests[idx].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[idx].cmd, MAX_CMD_LEN, 
             "gcov-tool overlap -v -f -F -o -h -t 0.8 %s/test_prog1.gcda %s/test_prog2.gcda %s/test_prog3.gcda",
             temp_dir, temp_dir, temp_dir);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Three files with all flags";
    idx++;
    
    *test_count = idx;
}

int main(int argc, char *argv[]) {
    char temp_dir[] = "/tmp/gcov_test_XXXXXX";
    test_case_t tests[50];
    int test_count = 0;
    int passed = 0, failed = 0;
    
    // Create temporary directory
    if (!mkdtemp(temp_dir)) {
        perror("Failed to create temporary directory");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", temp_dir);
    
    // Setup GCOV test files
    if (setup_gcov_test_files(temp_dir) != 0) {
        fprintf(stderr, "Failed to setup GCOV test files\n");
        // Clean up
        char cmd[MAX_CMD_LEN];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
        system(cmd);
        return 1;
    }
    
    // Generate test cases
    generate_test_cases(tests, temp_dir, &test_count);
    
    printf("\n=== Running %d test cases ===\n\n", test_count);
    
    // Run all test cases
    for (int i = 0; i < test_count; i++) {
        printf("Test %d: %s\n", i + 1, tests[i].description);
        
        int exit_code = execute_command(tests[i].cmd);
        
        if (tests[i].expected_exit == 0) {
            // For valid commands, check if they succeeded
            if (exit_code == 0) {
                printf("  ✓ PASSED (exit code: %d)\n", exit_code);
                passed++;
            } else {
                printf("  ✗ FAILED - Expected success, got exit code: %d\n", exit_code);
                failed++;
            }
        } else {
            // For invalid commands, check if they failed as expected
            if (exit_code != 0) {
                printf("  ✓ PASSED - Failed as expected (exit code: %d)\n", exit_code);
                passed++;
            } else {
                printf("  ✗ FAILED - Expected failure, got success\n");
                failed++;
            }
        }
        printf("\n");
        
        free(tests[i].cmd);
    }
    
    // Clean up
    printf("Cleaning up temporary directory: %s\n", temp_dir);
    char cleanup_cmd[MAX_CMD_LEN];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
    system(cleanup_cmd);
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", test_count);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    
    // Create a simple report file for verification
    FILE *report = fopen("gcov_tool_test_report.txt", "w");
    if (report) {
        fprintf(report, "GCov-tool overlap subcommand test report\n");
        fprintf(report, "=========================================\n");
        fprintf(report, "Total tests executed: %d\n", test_count);
        fprintf(report, "Tests passed: %d\n", passed);
        fprintf(report, "Tests failed: %d\n", failed);
        fprintf(report, "\nCoverage targets exercised:\n");
        fprintf(report, "- -v flag (verbose mode)\n");
        fprintf(report, "- -f flag (function level)\n");
        fprintf(report, "- -F flag (full filename)\n");
        fprintf(report, "- -o flag (object level)\n");
        fprintf(report, "- -h flag (hot only)\n");
        fprintf(report, "- -t flag with various thresholds\n");
        fprintf(report, "- Invalid arguments (triggers atof)\n");
        fprintf(report, "- Unknown flags (triggers default case)\n");
        fclose(report);
    }
    
    return failed > 0 ? 1 : 0;
}
