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
#define TEMP_DIR_PREFIX "/tmp/gcov_test_XXXXXX"

// Structure to hold test case information
typedef struct {
    const char *description;
    const char *args;
    int expected_exit_code;
} test_case_t;

// Function to create a temporary directory
char *create_temp_dir() {
    char *template = strdup(TEMP_DIR_PREFIX);
    if (!template) {
        perror("strdup");
        return NULL;
    }
    
    char *dir = mkdtemp(template);
    if (!dir) {
        perror("mkdtemp");
        free(template);
        return NULL;
    }
    
    return dir;
}

// Function to remove a directory recursively
void remove_temp_dir(const char *dir) {
    if (dir) {
        char cmd[MAX_CMD_LEN];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", dir);
        system(cmd);
    }
}

// Function to compile a simple C program with GCOV instrumentation
int compile_gcov_program(const char *dir, const char *prog_name) {
    char src_path[MAX_CMD_LEN];
    char exe_path[MAX_CMD_LEN];
    
    snprintf(src_path, sizeof(src_path), "%s/%s.c", dir, prog_name);
    snprintf(exe_path, sizeof(exe_path), "%s/%s", dir, prog_name);
    
    // Create a simple C program
    FILE *fp = fopen(src_path, "w");
    if (!fp) {
        perror("fopen");
        return -1;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        printf(\"Hello %%d\\n\", i);\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    fclose(fp);
    
    // Compile with GCOV instrumentation
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s 2>/dev/null",
             exe_path, src_path);
    
    return system(cmd);
}

// Function to run the compiled program to generate .gcda files
int run_gcov_program(const char *dir, const char *prog_name, int run_num) {
    char exe_path[MAX_CMD_LEN];
    char gcda_pattern[MAX_CMD_LEN];
    
    snprintf(exe_path, sizeof(exe_path), "%s/%s", dir, prog_name);
    
    // Run the program
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "cd %s && %s > /dev/null 2>&1", dir, exe_path);
    int ret = system(cmd);
    
    if (ret == 0 && run_num > 1) {
        // Rename .gcda files for multiple runs
        snprintf(gcda_pattern, sizeof(gcda_pattern), 
                 "cd %s && for f in *.gcda; do mv \"$f\" \"${f%.gcda}_run%d.gcda\"; done",
                 dir, run_num);
        system(gcda_pattern);
    }
    
    return ret;
}

// Function to execute gcov-tool and capture exit code
int execute_gcov_tool(const char *cmd) {
    printf("Executing: %s\n", cmd);
    
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

// Function to test a specific command
void test_command(const char *description, const char *cmd, int expected_exit) {
    printf("\n=== Test: %s ===\n", description);
    
    int exit_code = execute_gcov_tool(cmd);
    
    if (exit_code == expected_exit) {
        printf("✓ PASS: Exit code %d (expected %d)\n", exit_code, expected_exit);
    } else {
        printf("✗ FAIL: Exit code %d (expected %d)\n", exit_code, expected_exit);
    }
}

int main(int argc, char *argv[]) {
    char *temp_dir = NULL;
    char gcda_files[MAX_CMD_LEN] = "";
    int test_count = 0;
    int passed_count = 0;
    
    printf("=== GCOV-Tool Overlap Subcommand Test Suite ===\n");
    
    // Create temporary directory
    temp_dir = create_temp_dir();
    if (!temp_dir) {
        fprintf(stderr, "Failed to create temporary directory\n");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", temp_dir);
    
    // Compile test program with GCOV instrumentation
    if (compile_gcov_program(temp_dir, "test_prog") != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        remove_temp_dir(temp_dir);
        free(temp_dir);
        return 1;
    }
    
    // Run program multiple times to generate different .gcda files
    for (int i = 1; i <= 3; i++) {
        if (run_gcov_program(temp_dir, "test_prog", i) != 0) {
            fprintf(stderr, "Failed to run test program (run %d)\n", i);
        }
    }
    
    // Prepare .gcda file list for overlap command
    char find_cmd[MAX_CMD_LEN];
    snprintf(find_cmd, sizeof(find_cmd),
             "find %s -name \"*.gcda\" | head -2", temp_dir);
    
    FILE *fp = popen(find_cmd, "r");
    if (fp) {
        char path[256];
        int first = 1;
        while (fgets(path, sizeof(path), fp)) {
            path[strcspn(path, "\n")] = 0;
            if (!first) {
                strcat(gcda_files, " ");
            }
            strcat(gcda_files, path);
            first = 0;
        }
        pclose(fp);
    }
    
    if (strlen(gcda_files) == 0) {
        fprintf(stderr, "No .gcda files generated\n");
        remove_temp_dir(temp_dir);
        free(temp_dir);
        return 1;
    }
    
    printf("Using .gcda files: %s\n", gcda_files);
    
    // Define test cases
    test_case_t test_cases[] = {
        // Basic flag tests - all individual flags
        {"Single -v flag", "-v", 0},
        {"Single -f flag", "-f", 0},
        {"Single -F flag", "-F", 0},
        {"Single -o flag", "-o", 0},
        {"Single -h flag", "-h", 0},
        {"Single -t flag with value", "-t 0.5", 0},
        
        // Combinations of all uncovered flags
        {"All flags together", "-v -f -F -o -h -t 0.75", 0},
        {"All flags reverse order", "-t 1.0 -h -o -F -f -v", 0},
        {"Flags with different -t values", "-v -f -F -o -h -t 0.25", 0},
        
        // Permutations of flag order
        {"Flags: -v -f -F", "-v -f -F", 0},
        {"Flags: -F -f -v", "-F -f -v", 0},
        {"Flags: -o -h -t 0.3", "-o -h -t 0.3", 0},
        {"Flags: -h -o -t 0.7", "-h -o -t 0.7", 0},
        
        // Edge cases
        {"Repeated -v flag", "-v -v -v", 0},
        {"Multiple -t flags (last wins)", "-t 0.1 -t 0.2 -t 0.3", 0},
        {"Flags with spaces", "-v  -f  -F  -o  -h  -t  0.9", 0},
        
        // Error cases
        {"Missing argument for -t", "-t", 1},  // Should fail
        {"Invalid argument for -t", "-t not_a_number", 1},  // Should fail
        {"Unknown flag -x", "-x", 1},  // Should trigger default case
        {"Mixed valid and invalid", "-v -x -f", 1},  // Should fail
        
        // Boundary values for -t
        {"-t with zero", "-t 0.0", 0},
        {"-t with one", "-t 1.0", 0},
        {"-t with high value", "-t 100.0", 0},
        {"-t with decimal", "-t 0.333333", 0},
        
        // Flag combinations with file arguments in different positions
        {"Flags before files", "-v -f", 0},
        {"Flags between files", "", 0},  // Will be constructed dynamically
    };
    
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    // Execute test cases
    for (int i = 0; i < num_test_cases; i++) {
        char full_cmd[MAX_CMD_LEN];
        
        // Special handling for test case that needs files in the middle
        if (i == num_test_cases - 1) {  // Last test case
            // Split files and put flags in between
            char *first_space = strchr(gcda_files, ' ');
            if (first_space) {
                *first_space = '\0';
                char *first_file = gcda_files;
                char *second_file = first_space + 1;
                snprintf(full_cmd, sizeof(full_cmd),
                        "gcov-tool overlap %s %s -v -f %s",
                        first_file, second_file, second_file);
                *first_space = ' ';  // Restore original string
            } else {
                snprintf(full_cmd, sizeof(full_cmd),
                        "gcov-tool overlap %s -v -f",
                        gcda_files);
            }
        } else {
            // Normal test case construction
            snprintf(full_cmd, sizeof(full_cmd),
                    "gcov-tool overlap %s %s",
                    test_cases[i].args, gcda_files);
        }
        
        test_count++;
        test_command(test_cases[i].description, full_cmd, 
                    test_cases[i].expected_exit_code);
        
        // Check if test passed
        int exit_code = execute_gcov_tool(full_cmd);
        if (exit_code == test_cases[i].expected_exit_code) {
            passed_count++;
        }
        
        // Add delay to avoid overwhelming the system
        usleep(100000);  // 100ms delay
    }
    
    // Additional permutation tests
    printf("\n=== Testing Flag Permutations ===\n");
    
    // Array of the uncovered flags (excluding -t which needs an argument)
    const char *base_flags[] = {"-v", "-f", "-F", "-o", "-h"};
    int num_base_flags = sizeof(base_flags) / sizeof(base_flags[0]);
    
    // Test different permutations
    const char *permutations[][6] = {
        {"-v", "-f", "-F", "-o", "-h", "-t 0.5"},
        {"-h", "-o", "-F", "-f", "-v", "-t 0.6"},
        {"-f", "-v", "-o", "-h", "-F", "-t 0.7"},
        {"-F", "-o", "-h", "-v", "-f", "-t 0.8"},
        {"-o", "-h", "-v", "-F", "-f", "-t 0.9"},
    };
    
    for (int i = 0; i < sizeof(permutations) / sizeof(permutations[0]); i++) {
        char perm_cmd[MAX_CMD_LEN] = "gcov-tool overlap ";
        
        for (int j = 0; j < 6; j++) {
            if (permutations[i][j]) {
                strcat(perm_cmd, permutations[i][j]);
                strcat(perm_cmd, " ");
            }
        }
        
        strcat(perm_cmd, gcda_files);
        
        char desc[100];
        snprintf(desc, sizeof(desc), "Permutation %d", i + 1);
        
        test_count++;
        test_command(desc, perm_cmd, 0);
        
        int exit_code = execute_gcov_tool(perm_cmd);
        if (exit_code == 0) {
            passed_count++;
        }
        
        usleep(50000);  // 50ms delay
    }
    
    // Test environment variable to prevent optimization
    printf("\n=== Testing with Environment Variables ===\n");
    
    // Use environment variable to construct command (prevents compiler optimization)
    const char *env_flags = getenv("GCOV_TEST_FLAGS");
    if (!env_flags) {
        env_flags = "-v -f -F -o -h -t 0.5";  // Default if not set
    }
    
    char env_cmd[MAX_CMD_LEN];
    snprintf(env_cmd, sizeof(env_cmd), "gcov-tool overlap %s %s", 
             env_flags, gcda_files);
    
    test_count++;
    test_command("Environment-based flags", env_cmd, 0);
    
    int env_exit_code = execute_gcov_tool(env_cmd);
    if (env_exit_code == 0) {
        passed_count++;
    }
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", test_count);
    printf("Passed: %d\n", passed_count);
    printf("Failed: %d\n", test_count - passed_count);
    printf("Pass rate: %.1f%%\n", (passed_count * 100.0) / test_count);
    
    // Cleanup
    printf("\nCleaning up temporary directory: %s\n", temp_dir);
    remove_temp_dir(temp_dir);
    free(temp_dir);
    
    // Return non-zero if any tests failed
    return (passed_count == test_count) ? 0 : 1;
}
