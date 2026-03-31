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
} test_case;

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

// Create a second program for overlap comparison
const char *test_program2 = 
"#include <stdio.h>\n"
"int main() {\n"
"    int i;\n"
"    for (i = 0; i < 5; i++) {\n"
"        printf(\"Count: %d\\n\", i);\n"
"    }\n"
"    return 0;\n"
"}\n";

// Execute a command and return exit status
int run_command(const char *cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

// Create a temporary directory
char *create_temp_dir() {
    char *template = strdup("/tmp/gcov_test_XXXXXX");
    char *dir = mkdtemp(template);
    if (!dir) {
        perror("mkdtemp failed");
        exit(1);
    }
    return dir;
}

// Generate GCOV data files
void generate_gcov_data(const char *dir, const char *program, const char *name) {
    char source_path[256];
    char exec_path[256];
    char gcda_path[256];
    
    snprintf(source_path, sizeof(source_path), "%s/%s.c", dir, name);
    snprintf(exec_path, sizeof(exec_path), "%s/%s", dir, name);
    snprintf(gcda_path, sizeof(gcda_path), "%s/%s.gcda", dir, name);
    
    // Write source file
    FILE *fp = fopen(source_path, "w");
    if (!fp) {
        perror("Failed to create source file");
        exit(1);
    }
    fprintf(fp, "%s", program);
    fclose(fp);
    
    // Compile with GCOV instrumentation
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s", 
             exec_path, source_path);
    if (run_command(cmd) != 0) {
        fprintf(stderr, "Compilation failed for %s\n", name);
        exit(1);
    }
    
    // Run the program to generate .gcda file
    snprintf(cmd, sizeof(cmd), "cd %s && ./%s > /dev/null", dir, name);
    if (run_command(cmd) != 0) {
        fprintf(stderr, "Execution failed for %s\n", name);
        exit(1);
    }
    
    // Verify .gcda file was created
    struct stat st;
    if (stat(gcda_path, &st) != 0) {
        fprintf(stderr, "Failed to create .gcda file for %s\n", name);
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    char *temp_dir = create_temp_dir();
    printf("Created temporary directory: %s\n", temp_dir);
    
    // Generate GCOV data files
    generate_gcov_data(temp_dir, test_program, "test1");
    generate_gcov_data(temp_dir, test_program2, "test2");
    
    // Build paths to .gcda files
    char gcda1[256], gcda2[256];
    snprintf(gcda1, sizeof(gcda1), "%s/test1.gcda", temp_dir);
    snprintf(gcda2, sizeof(gcda2), "%s/test2.gcda", temp_dir);
    
    // Define test cases covering all uncovered lines
    test_case tests[] = {
        // Basic tests for each flag individually
        {"gcov-tool overlap -v %s %s", 0, "Verbose flag"},
        {"gcov-tool overlap -f %s %s", 0, "Function level flag"},
        {"gcov-tool overlap -F %s %s", 0, "Full filename flag"},
        {"gcov-tool overlap -o %s %s", 0, "Object level flag"},
        {"gcov-tool overlap -h %s %s", 0, "Hot only flag"},
        {"gcov-tool overlap -t 0.5 %s %s", 0, "Hot threshold 0.5"},
        {"gcov-tool overlap -t 1.0 %s %s", 0, "Hot threshold 1.0"},
        {"gcov-tool overlap -t 0.75 %s %s", 0, "Hot threshold 0.75"},
        
        // Combinations of all flags (testing the main uncovered block)
        {"gcov-tool overlap -v -f -F -o -h -t 0.8 %s %s", 0, "All flags combined"},
        {"gcov-tool overlap -t 0.6 -h -o -F -f -v %s %s", 0, "All flags reversed order"},
        
        // Permutations of flag order
        {"gcov-tool overlap -f -v -t 0.3 %s %s", 0, "Flags f, v, t"},
        {"gcov-tool overlap -F -o -h %s %s", 0, "Flags F, o, h"},
        {"gcov-tool overlap -t 0.9 -v -f %s %s", 0, "Flags t, v, f"},
        
        // Edge cases for -t flag
        {"gcov-tool overlap -t 0.0 %s %s", 0, "Zero threshold"},
        {"gcov-tool overlap -t 100.0 %s %s", 0, "Large threshold"},
        {"gcov-tool overlap -t 0.0001 %s %s", 0, "Very small threshold"},
        
        // Error cases (should trigger error handling)
        {"gcov-tool overlap -t not_a_number %s %s", 1, "Invalid threshold (should fail)"},
        {"gcov-tool overlap -t %s %s", 1, "Missing threshold value (should fail)"},
        {"gcov-tool overlap -x %s %s", 1, "Unknown flag (should trigger default case)"},
        
        // Repeated flags
        {"gcov-tool overlap -v -v -v %s %s", 0, "Repeated verbose flag"},
        {"gcov-tool overlap -f -f -t 0.5 -t 0.7 %s %s", 0, "Multiple repeated flags"},
        
        // Mixed valid and invalid (testing robustness)
        {"gcov-tool overlap -v -x -f %s %s", 1, "Mixed valid and invalid flags"},
        
        // With different numbers of input files
        {"gcov-tool overlap -v %s", 0, "Single input file"},
        {"gcov-tool overlap -f -F %s %s %s/test1.c.gcno", 0, "Three input files"},
        
        // Empty threshold (edge case)
        {"gcov-tool overlap -t '' %s %s", 1, "Empty threshold string"},
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    int failed = 0;
    
    printf("\n=== Running gcov-tool overlap tests ===\n\n");
    
    for (int i = 0; i < num_tests; i++) {
        char cmd[MAX_CMD_LEN];
        
        // Format the command with actual file paths
        if (strstr(tests[i].cmd, "%s/test1.c.gcno")) {
            // Special case for three files
            snprintf(cmd, sizeof(cmd), tests[i].cmd, gcda1, gcda2, temp_dir);
        } else if (strstr(tests[i].cmd, "%s %s")) {
            // Two files case
            snprintf(cmd, sizeof(cmd), tests[i].cmd, gcda1, gcda2);
        } else if (strstr(tests[i].cmd, "%s")) {
            // Single file case
            snprintf(cmd, sizeof(cmd), tests[i].cmd, gcda1);
        } else {
            strncpy(cmd, tests[i].cmd, sizeof(cmd));
        }
        
        // Run the test
        int exit_code = run_command(cmd);
        
        // Check result
        int success = (exit_code == tests[i].expected_exit);
        
        printf("Test %d: %s\n", i + 1, tests[i].description);
        printf("  Exit code: %d (expected %d) - %s\n\n", 
               exit_code, tests[i].expected_exit,
               success ? "PASS" : "FAIL");
        
        if (success) {
            passed++;
        } else {
            failed++;
        }
        
        // Small delay to avoid overwhelming the system
        usleep(10000);
    }
    
    // Additional test: Use getenv to vary commands (prevent optimization)
    char *custom_flags = getenv("GCOV_EXTRA_FLAGS");
    if (custom_flags) {
        char extra_cmd[MAX_CMD_LEN];
        snprintf(extra_cmd, sizeof(extra_cmd), 
                "gcov-tool overlap %s %s %s", custom_flags, gcda1, gcda2);
        printf("\n=== Running custom test from environment ===\n");
        printf("Command: %s\n", extra_cmd);
        int exit_code = run_command(extra_cmd);
        printf("Custom test exit code: %d\n", exit_code);
    }
    
    // Cleanup
    char cleanup_cmd[256];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
    run_command(cleanup_cmd);
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", num_tests);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    
    // Create a simple script to run permutations
    printf("\n=== Generating permutation test script ===\n");
    FILE *script = fopen("run_gcov_permutations.sh", "w");
    if (script) {
        fprintf(script, "#!/bin/bash\n");
        fprintf(script, "# Auto-generated permutation tests for gcov-tool overlap\n");
        fprintf(script, "echo 'Running permutation tests...'\n\n");
        
        // All flags to permute
        char *flags[] = {"-v", "-f", "-F", "-o", "-h", "-t 0.5"};
        int num_flags = 6;
        
        // Generate some permutations (not all 720, but a representative sample)
        char *permutations[][6] = {
            {"-v", "-f", "-F", "-o", "-h", "-t 0.5"},
            {"-t 0.5", "-h", "-o", "-F", "-f", "-v"},
            {"-f", "-v", "-t 0.5", "-h", "-o", "-F"},
            {"-F", "-o", "-h", "-t 0.5", "-v", "-f"},
            {"-h", "-t 0.5", "-v", "-f", "-F", "-o"},
        };
        
        for (int i = 0; i < 5; i++) {
            fprintf(script, "echo '\\nPermutation %d:'\n", i + 1);
            fprintf(script, "gcov-tool overlap");
            for (int j = 0; j < 6; j++) {
                fprintf(script, " %s", permutations[i][j]);
            }
            fprintf(script, " %s %s\n", gcda1, gcda2);
            fprintf(script, "echo 'Exit code: $?'\n");
        }
        
        fclose(script);
        chmod("run_gcov_permutations.sh", 0755);
        printf("Generated permutation script: run_gcov_permutations.sh\n");
    }
    
    return (failed == 0) ? 0 : 1;
}
