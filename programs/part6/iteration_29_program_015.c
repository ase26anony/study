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

// Structure to hold test case information
typedef struct {
    char *description;
    char *command;
    int expected_exit_code;
} test_case_t;

// Simple C program to generate GCOV data
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

// Function to execute a command and return exit code
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    fflush(stdout);
    
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

// Function to create a temporary directory
char *create_temp_dir() {
    char *template = strdup("/tmp/gcov_test_XXXXXX");
    if (mkdtemp(template) == NULL) {
        perror("Failed to create temp directory");
        free(template);
        return NULL;
    }
    return template;
}

// Function to compile and run test program to generate GCOV data
int generate_gcov_data(const char *temp_dir, int num_files) {
    char cmd[MAX_CMD_LEN];
    char src_path[MAX_CMD_LEN];
    char exe_path[MAX_CMD_LEN];
    
    // Create source file
    snprintf(src_path, sizeof(src_path), "%s/test.c", temp_dir);
    FILE *src = fopen(src_path, "w");
    if (!src) {
        perror("Failed to create source file");
        return 0;
    }
    fprintf(src, "%s", test_program);
    fclose(src);
    
    // Compile with GCOV instrumentation
    snprintf(exe_path, sizeof(exe_path), "%s/test", temp_dir);
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -o %s %s", 
             exe_path, src_path);
    
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return 0;
    }
    
    // Generate multiple .gcda files by running the program multiple times
    for (int i = 0; i < num_files; i++) {
        // Run the program
        snprintf(cmd, sizeof(cmd), "cd %s && ./test > /dev/null 2>&1", temp_dir);
        if (execute_command(cmd) != 0) {
            fprintf(stderr, "Failed to run test program iteration %d\n", i);
            return 0;
        }
        
        // Rename gcda file to create multiple versions
        if (i < num_files - 1) {
            char old_gcda[MAX_CMD_LEN];
            char new_gcda[MAX_CMD_LEN];
            snprintf(old_gcda, sizeof(old_gcda), "%s/test.gcda", temp_dir);
            snprintf(new_gcda, sizeof(new_gcda), "%s/test%d.gcda", temp_dir, i);
            
            if (rename(old_gcda, new_gcda) != 0) {
                perror("Failed to rename gcda file");
            }
        }
    }
    
    return 1;
}

// Function to run a single test case
int run_test_case(const char *temp_dir, const test_case_t *test) {
    char full_cmd[MAX_CMD_LEN];
    
    // Replace placeholders with actual file paths
    snprintf(full_cmd, sizeof(full_cmd), test->command, temp_dir);
    
    int exit_code = execute_command(full_cmd);
    
    printf("Test: %s\n", test->description);
    printf("Exit code: %d (expected: %d)\n", exit_code, test->expected_exit_code);
    
    if (exit_code == test->expected_exit_code) {
        printf("Result: PASS\n\n");
        return 1;
    } else {
        printf("Result: FAIL\n\n");
        return 0;
    }
}

int main(int argc, char *argv[]) {
    printf("=== GCOV-Tool Overlap Options Test Driver ===\n\n");
    
    // Create temporary directory
    char *temp_dir = create_temp_dir();
    if (!temp_dir) {
        return 1;
    }
    printf("Created temp directory: %s\n", temp_dir);
    
    // Generate GCOV data files
    printf("\nGenerating GCOV data files...\n");
    if (!generate_gcov_data(temp_dir, 3)) {
        fprintf(stderr, "Failed to generate GCOV data\n");
        free(temp_dir);
        return 1;
    }
    
    // Define test cases
    test_case_t test_cases[] = {
        // Basic tests with all flags
        {
            "All flags combined in standard order",
            "gcov-tool overlap -v -f -F -o -h -t 0.75 %s/test.gcda %s/test0.gcda",
            0
        },
        
        // Permutations of flag order
        {
            "Flags in reverse order",
            "gcov-tool overlap -t 1.0 -h -o -F -f -v %s/test.gcda %s/test1.gcda",
            0
        },
        
        {
            "Flags in random order 1",
            "gcov-tool overlap -F -v -t 0.5 -f -o -h %s/test.gcda %s/test0.gcda",
            0
        },
        
        {
            "Flags in random order 2",
            "gcov-tool overlap -h -t 0.25 -F -v -o -f %s/test.gcda %s/test1.gcda",
            0
        },
        
        // Individual flag tests
        {
            "Verbose flag only",
            "gcov-tool overlap -v %s/test.gcda %s/test0.gcda",
            0
        },
        
        {
            "Function level flag only",
            "gcov-tool overlap -f %s/test.gcda %s/test1.gcda",
            0
        },
        
        {
            "Full filename flag only",
            "gcov-tool overlap -F %s/test.gcda %s/test0.gcda",
            0
        },
        
        {
            "Object level flag only",
            "gcov-tool overlap -o %s/test.gcda %s/test1.gcda",
            0
        },
        
        {
            "Hot only flag only",
            "gcov-tool overlap -h %s/test.gcda %s/test0.gcda",
            0
        },
        
        {
            "Threshold flag only",
            "gcov-tool overlap -t 0.9 %s/test.gcda %s/test1.gcda",
            0
        },
        
        // Flag combinations
        {
            "Verbose + Function level",
            "gcov-tool overlap -v -f %s/test.gcda %s/test0.gcda",
            0
        },
        
        {
            "Full filename + Object level",
            "gcov-tool overlap -F -o %s/test.gcda %s/test1.gcda",
            0
        },
        
        {
            "Hot only + Threshold",
            "gcov-tool overlap -h -t 0.3 %s/test.gcda %s/test0.gcda",
            0
        },
        
        {
            "Verbose + Full filename + Hot only",
            "gcov-tool overlap -v -F -h %s/test.gcda %s/test1.gcda",
            0
        },
        
        // Edge cases for threshold
        {
            "Threshold with value 0.0",
            "gcov-tool overlap -t 0.0 %s/test.gcda %s/test0.gcda",
            0
        },
        
        {
            "Threshold with value 1.0",
            "gcov-tool overlap -t 1.0 %s/test.gcda %s/test1.gcda",
            0
        },
        
        {
            "Threshold with decimal value",
            "gcov-tool overlap -t 0.123456 %s/test.gcda %s/test0.gcda",
            0
        },
        
        {
            "Threshold with scientific notation (should fail parsing)",
            "gcov-tool overlap -t 1e-3 %s/test.gcda %s/test1.gcda",
            0  // atof should handle this
        },
        
        // Error cases
        {
            "Missing argument for -t flag (should trigger error)",
            "gcov-tool overlap -t",
            1  // Non-zero exit code expected
        },
        
        {
            "Invalid argument for -t flag",
            "gcov-tool overlap -t not_a_number %s/test.gcda",
            1  // Non-zero exit code expected
        },
        
        {
            "Unknown flag -x (should trigger default case)",
            "gcov-tool overlap -x %s/test.gcda",
            1  // Non-zero exit code expected
        },
        
        {
            "Repeated verbose flags",
            "gcov-tool overlap -v -v -v %s/test.gcda %s/test0.gcda",
            0
        },
        
        {
            "Mixed valid and invalid flags",
            "gcov-tool overlap -v -x -f %s/test.gcda",
            1  // Non-zero exit code expected
        },
        
        // File path variations
        {
            "With absolute paths",
            "gcov-tool overlap -v -f %s/test.gcda %s/test0.gcda",
            0
        },
        
        {
            "With relative paths from temp dir",
            "cd %s && gcov-tool overlap -v -f test.gcda test1.gcda",
            0
        },
        
        // Multiple input files
        {
            "Three input files with all flags",
            "gcov-tool overlap -v -f -F -o -h -t 0.5 %s/test.gcda %s/test0.gcda %s/test1.gcda",
            0
        }
    };
    
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int passed = 0;
    
    printf("\n=== Running Test Cases ===\n\n");
    
    // Run all test cases
    for (int i = 0; i < num_tests; i++) {
        if (run_test_case(temp_dir, &test_cases[i])) {
            passed++;
        }
    }
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", num_tests);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", num_tests - passed);
    
    // Cleanup
    printf("\nCleaning up temp directory: %s\n", temp_dir);
    char cleanup_cmd[MAX_CMD_LEN];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
    execute_command(cleanup_cmd);
    free(temp_dir);
    
    return (passed == num_tests) ? 0 : 1;
}
