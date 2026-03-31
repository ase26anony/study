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
    const char *description;
    const char *args;
    int expected_exit_code;
} test_case_t;

// Global variables to prevent optimization
volatile int use_verbose = 1;
volatile int use_func = 1;
volatile int use_fullname = 1;
volatile int use_obj = 1;
volatile int use_hot = 1;
volatile float threshold = 0.75;

// Function to create a simple instrumented C program
int create_instrumented_program(const char *dir) {
    char src_path[256];
    char compile_cmd[512];
    
    snprintf(src_path, sizeof(src_path), "%s/test_prog.c", dir);
    
    // Create a simple C program
    FILE *fp = fopen(src_path, "w");
    if (!fp) {
        perror("Failed to create test program");
        return 0;
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
    
    // Compile with coverage instrumentation
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -O0 %s/test_prog.c -o %s/test_prog",
             dir, dir);
    
    return system(compile_cmd) == 0;
}

// Function to run the instrumented program and generate .gcda files
int generate_gcda_files(const char *dir, int count) {
    char cmd[256];
    int i;
    
    for (i = 0; i < count; i++) {
        snprintf(cmd, sizeof(cmd), "cd %s && ./test_prog > /dev/null 2>&1", dir);
        if (system(cmd) != 0) {
            fprintf(stderr, "Failed to run test program\n");
            return 0;
        }
        
        // Rename the .gcda file to create multiple versions
        if (i < count - 1) {
            char old_name[256], new_name[256];
            snprintf(old_name, sizeof(old_name), "%s/test_prog.gcda", dir);
            snprintf(new_name, sizeof(new_name), "%s/test_prog%d.gcda", dir, i);
            rename(old_name, new_name);
        }
    }
    
    return 1;
}

// Function to execute a command and capture exit code
int execute_command(const char *cmd, const char *description) {
    printf("Testing: %s\n", description);
    printf("Command: %s\n", cmd);
    
    int status = system(cmd);
    int exit_code = WEXITSTATUS(status);
    
    printf("Exit code: %d\n\n", exit_code);
    return exit_code;
}

// Function to test all flag combinations
void test_flag_combinations(const char *dir) {
    char cmd[MAX_CMD_LEN];
    int test_num = 1;
    
    // Base command with gcda files
    const char *base_cmd = "gcov-tool overlap %s %s/test_prog.gcda %s/test_prog0.gcda";
    
    // Test 1: All flags together (main test for uncovered lines)
    snprintf(cmd, sizeof(cmd), base_cmd, 
             "-v -f -F -o -h -t 0.5", dir, dir);
    execute_command(cmd, "All flags combined");
    
    // Test 2: Different order of flags
    snprintf(cmd, sizeof(cmd), base_cmd,
             "-t 1.0 -h -o -F -f -v", dir, dir);
    execute_command(cmd, "Flags in reverse order");
    
    // Test 3: Flags with different threshold values
    const float thresholds[] = {0.1, 0.25, 0.5, 0.75, 0.99, 1.0};
    for (size_t i = 0; i < sizeof(thresholds)/sizeof(thresholds[0]); i++) {
        snprintf(cmd, sizeof(cmd), base_cmd,
                 "-v -t", dir, dir);
        // Need to handle float formatting separately
        char full_cmd[MAX_CMD_LEN];
        snprintf(full_cmd, sizeof(full_cmd), "gcov-tool overlap -v -t %f %s/test_prog.gcda %s/test_prog0.gcda",
                 thresholds[i], dir, dir);
        execute_command(full_cmd, "Different threshold values");
    }
    
    // Test 4: Individual flags (each case statement separately)
    const char *individual_flags[] = {"-v", "-f", "-F", "-o", "-h"};
    for (size_t i = 0; i < sizeof(individual_flags)/sizeof(individual_flags[0]); i++) {
        snprintf(cmd, sizeof(cmd), base_cmd,
                 individual_flags[i], dir, dir);
        execute_command(cmd, individual_flags[i]);
    }
    
    // Test 5: -t flag alone
    snprintf(cmd, sizeof(cmd), base_cmd,
             "-t 0.3", dir, dir);
    execute_command(cmd, "-t flag alone");
    
    // Test 6: Combinations of 2-3 flags
    const char *combinations[] = {
        "-v -f", "-v -F", "-v -o", "-v -h", "-v -t 0.5",
        "-f -F", "-f -o", "-f -h", "-f -t 0.5",
        "-F -o", "-F -h", "-F -t 0.5",
        "-o -h", "-o -t 0.5",
        "-h -t 0.5",
        "-v -f -F", "-f -F -o", "-F -o -h", "-o -h -t 0.5"
    };
    
    for (size_t i = 0; i < sizeof(combinations)/sizeof(combinations[0]); i++) {
        snprintf(cmd, sizeof(cmd), base_cmd,
                 combinations[i], dir, dir);
        char desc[100];
        snprintf(desc, sizeof(desc), "Combination: %s", combinations[i]);
        execute_command(cmd, desc);
    }
}

// Function to test edge cases and error conditions
void test_edge_cases(const char *dir) {
    char cmd[MAX_CMD_LEN];
    
    printf("\n=== Testing Edge Cases ===\n\n");
    
    // Test 1: Invalid argument for -t (should trigger atof conversion)
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -t not_a_number %s/test_prog.gcda %s/test_prog0.gcda",
             dir, dir);
    execute_command(cmd, "Invalid argument for -t (non-numeric)");
    
    // Test 2: -t with negative number
    snprintf(cmd, sizeof(cmd),
             "gcov-tool overlap -t -1.5 %s/test_prog.gcda %s/test_prog0.gcda",
             dir, dir);
    execute_command(cmd, "Negative threshold value");
    
    // Test 3: -t with very large number
    snprintf(cmd, sizeof(cmd),
             "gcov-tool overlap -t 999999.99 %s/test_prog.gcda %s/test_prog0.gcda",
             dir, dir);
    execute_command(cmd, "Very large threshold value");
    
    // Test 4: Missing argument for -t (edge case - may cause different behavior)
    snprintf(cmd, sizeof(cmd),
             "gcov-tool overlap -t %s/test_prog.gcda",
             dir);
    execute_command(cmd, "Missing argument for -t");
    
    // Test 5: Unknown flag (should trigger default case)
    snprintf(cmd, sizeof(cmd),
             "gcov-tool overlap -x %s/test_prog.gcda %s/test_prog0.gcda",
             dir, dir);
    execute_command(cmd, "Unknown flag -x");
    
    // Test 6: Repeated flags
    snprintf(cmd, sizeof(cmd),
             "gcov-tool overlap -v -v -v %s/test_prog.gcda %s/test_prog0.gcda",
             dir, dir);
    execute_command(cmd, "Repeated -v flags");
    
    // Test 7: Empty flags (just overlap command)
    snprintf(cmd, sizeof(cmd),
             "gcov-tool overlap %s/test_prog.gcda %s/test_prog0.gcda",
             dir, dir);
    execute_command(cmd, "No flags at all");
    
    // Test 8: Flags with single gcda file
    snprintf(cmd, sizeof(cmd),
             "gcov-tool overlap -v -f %s/test_prog.gcda",
             dir);
    execute_command(cmd, "Single gcda file with flags");
    
    // Test 9: Flags with multiple gcda files (more than 2)
    snprintf(cmd, sizeof(cmd),
             "gcov-tool overlap -v -f %s/test_prog.gcda %s/test_prog0.gcda %s/test_prog.gcno",
             dir, dir, dir);
    execute_command(cmd, "Multiple input files");
    
    // Test 10: Using absolute paths
    char abs_path[256];
    realpath(dir, abs_path);
    snprintf(cmd, sizeof(cmd),
             "gcov-tool overlap -v -F %s/test_prog.gcda %s/test_prog0.gcda",
             abs_path, abs_path);
    execute_command(cmd, "Absolute paths with -F flag");
}

// Function to run gcov-tool using fork/exec for better control
int run_gcov_tool_direct(const char *args) {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        char *argv[64];
        int argc = 0;
        
        // Parse arguments
        char *token;
        char *args_copy = strdup(args);
        token = strtok(args_copy, " ");
        
        while (token != NULL && argc < 63) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
        argv[argc] = NULL;
        
        free(args_copy);
        
        execvp("gcov-tool", argv);
        
        // If execvp fails
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    } else {
        perror("fork failed");
        return -1;
    }
}

int main(int argc, char *argv[]) {
    char temp_dir[] = "/tmp/gcov_test_XXXXXX";
    char *dir_name;
    
    printf("=== GCOV Tool Overlap Parser Test ===\n\n");
    
    // Create temporary directory
    dir_name = mkdtemp(temp_dir);
    if (!dir_name) {
        perror("Failed to create temporary directory");
        return 1;
    }
    
    printf("Created temporary directory: %s\n\n", dir_name);
    
    // Step 1: Create and compile instrumented program
    printf("Creating instrumented test program...\n");
    if (!create_instrumented_program(dir_name)) {
        fprintf(stderr, "Failed to create instrumented program\n");
        return 1;
    }
    
    // Step 2: Generate multiple .gcda files
    printf("Generating .gcda files...\n");
    if (!generate_gcda_files(dir_name, 3)) {
        fprintf(stderr, "Failed to generate .gcda files\n");
        return 1;
    }
    
    // Step 3: Test various flag combinations
    printf("\n=== Testing Flag Combinations ===\n\n");
    test_flag_combinations(dir_name);
    
    // Step 4: Test edge cases
    test_edge_cases(dir_name);
    
    // Step 5: Additional tests using fork/exec
    printf("\n=== Additional Tests with fork/exec ===\n\n");
    
    // Test with environment variable to prevent optimization
    char *test_type = getenv("GCOV_TEST_TYPE");
    if (!test_type) test_type = "comprehensive";
    
    char test_cmd[512];
    snprintf(test_cmd, sizeof(test_cmd),
             "gcov-tool overlap -%c -%c -%c -%c -%c -t %f %s/test_prog.gcda %s/test_prog0.gcda",
             use_verbose ? 'v' : ' ',
             use_func ? 'f' : ' ',
             use_fullname ? 'F' : ' ',
             use_obj ? 'o' : ' ',
             use_hot ? 'h' : ' ',
             threshold,
             dir_name, dir_name);
    
    // Clean up the command string (remove spaces for unused flags)
    char cleaned_cmd[512];
    char *src = test_cmd;
    char *dst = cleaned_cmd;
    int in_flag = 0;
    
    while (*src) {
        if (*src == '-') {
            in_flag = 1;
            *dst++ = *src++;
        } else if (in_flag && *src == ' ') {
            // Skip space after flag
            src++;
            in_flag = 0;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
    
    printf("Dynamic command: %s\n", cleaned_cmd);
    int exit_code = run_gcov_tool_direct(cleaned_cmd);
    printf("Exit code from fork/exec: %d\n\n", exit_code);
    
    // Step 6: Cleanup
    printf("Cleaning up temporary directory...\n");
    char cleanup_cmd[256];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", dir_name);
    system(cleanup_cmd);
    
    printf("\n=== Test Summary ===\n");
    printf("All tests completed. The uncovered lines in parse_overlap_options\n");
    printf("should have been executed for flags: -v, -f, -F, -o, -h, -t\n");
    printf("Check coverage with: gcov gcov-tool.cc\n");
    
    return 0;
}
