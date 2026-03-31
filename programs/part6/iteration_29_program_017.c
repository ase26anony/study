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

// Run a command and return exit status
int run_command(const char *cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

// Create test GCOV data files
int create_gcov_test_files(const char *temp_dir) {
    char cmd[MAX_CMD_LEN];
    
    // Create test source file
    char src_path[MAX_CMD_LEN];
    snprintf(src_path, sizeof(src_path), "%s/test.c", temp_dir);
    
    FILE *fp = fopen(src_path, "w");
    if (!fp) {
        perror("Failed to create test.c");
        return -1;
    }
    fputs(test_program, fp);
    fclose(fp);
    
    // Compile with GCOV instrumentation
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -o %s/test %s/test.c 2>/dev/null", 
             temp_dir, temp_dir);
    if (run_command(cmd) != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return -1;
    }
    
    // Run the program multiple times to generate different .gcda files
    for (int i = 0; i < 3; i++) {
        snprintf(cmd, sizeof(cmd), "cd %s && ./test > /dev/null 2>&1", temp_dir);
        run_command(cmd);
        
        // Rename gcda file to create multiple versions
        if (i < 2) {
            snprintf(cmd, sizeof(cmd), "mv %s/test.gcda %s/test%d.gcda 2>/dev/null", 
                     temp_dir, temp_dir, i+1);
            run_command(cmd);
        }
    }
    
    return 0;
}

// Test gcov-tool with various argument combinations
void test_overlap_options(const char *temp_dir, const char *gcov_tool_path) {
    test_case_t tests[] = {
        // Basic flag combinations covering all uncovered cases
        {"%s overlap -v -f -F -o -h -t 0.5 %s/test1.gcda %s/test2.gcda", 0, "All flags combined"},
        {"%s overlap -t 1.0 -v -f -F -o -h %s/test1.gcda %s/test2.gcda", 0, "All flags, different order"},
        {"%s overlap -f -F -o -h -t 0.75 -v %s/test1.gcda %s/test2.gcda", 0, "All flags, another order"},
        
        // Individual flag tests
        {"%s overlap -v %s/test1.gcda %s/test2.gcda", 0, "Verbose flag only"},
        {"%s overlap -f %s/test1.gcda %s/test2.gcda", 0, "Function level flag only"},
        {"%s overlap -F %s/test1.gcda %s/test2.gcda", 0, "Full filename flag only"},
        {"%s overlap -o %s/test1.gcda %s/test2.gcda", 0, "Object level flag only"},
        {"%s overlap -h %s/test1.gcda %s/test2.gcda", 0, "Hot only flag only"},
        {"%s overlap -t 0.3 %s/test1.gcda %s/test2.gcda", 0, "Threshold flag only"},
        
        // Flag combinations
        {"%s overlap -v -f %s/test1.gcda %s/test2.gcda", 0, "Verbose + function level"},
        {"%s overlap -F -o %s/test1.gcda %s/test2.gcda", 0, "Fullname + object level"},
        {"%s overlap -h -t 0.8 %s/test1.gcda %s/test2.gcda", 0, "Hot only + threshold"},
        {"%s overlap -v -F -o %s/test1.gcda %s/test2.gcda", 0, "Verbose + fullname + object"},
        
        // Multiple input files
        {"%s overlap -v -f %s/test.gcda %s/test1.gcda %s/test2.gcda", 0, "Three input files"},
        
        // Edge cases for threshold
        {"%s overlap -t 0.0 %s/test1.gcda %s/test2.gcda", 0, "Zero threshold"},
        {"%s overlap -t 1.0 %s/test1.gcda %s/test2.gcda", 0, "One threshold"},
        {"%s overlap -t 0.999 %s/test1.gcda %s/test2.gcda", 0, "High threshold"},
        {"%s overlap -t 0.001 %s/test1.gcda %s/test2.gcda", 0, "Low threshold"},
        
        // Repeated flags
        {"%s overlap -v -v -v %s/test1.gcda %s/test2.gcda", 0, "Repeated verbose flag"},
        {"%s overlap -f -f -F -F %s/test1.gcda %s/test2.gcda", 0, "Repeated function/fullname flags"},
        
        // Error cases (should trigger different code paths)
        {"%s overlap -t not_a_number %s/test1.gcda %s/test2.gcda", 1, "Invalid threshold (non-numeric)"},
        {"%s overlap -t %s/test1.gcda", 1, "Missing threshold value"},
        {"%s overlap -x %s/test1.gcda %s/test2.gcda", 1, "Unknown flag (should trigger default case)"},
        
        // Empty command line after overlap
        {"%s overlap", 1, "No arguments after subcommand"},
        
        {NULL, 0, NULL} // Sentinel
    };
    
    printf("\n=== Testing gcov-tool overlap options ===\n\n");
    
    int passed = 0;
    int total = 0;
    
    for (int i = 0; tests[i].cmd != NULL; i++) {
        total++;
        char cmd[MAX_CMD_LEN];
        snprintf(cmd, sizeof(cmd), tests[i].cmd, 
                 gcov_tool_path, temp_dir, temp_dir, temp_dir);
        
        int exit_status = run_command(cmd);
        
        // Check if result matches expectation
        // Note: For error cases, we accept any non-zero exit status
        int success;
        if (tests[i].expected_exit == 0) {
            success = (exit_status == 0);
        } else {
            success = (exit_status != 0);
        }
        
        if (success) {
            printf("✓ PASS: %s (exit: %d)\n", tests[i].description, exit_status);
            passed++;
        } else {
            printf("✗ FAIL: %s (expected: %d, got: %d)\n", 
                   tests[i].description, tests[i].expected_exit, exit_status);
        }
    }
    
    printf("\n=== Summary: %d/%d tests passed ===\n", passed, total);
}

// Additional test with fork/exec for better control
void test_with_fork_exec(const char *temp_dir, const char *gcov_tool_path) {
    printf("\n=== Testing with fork/exec ===\n\n");
    
    // Test cases with specific argument arrays
    char *test_args[][10] = {
        { "gcov-tool", "overlap", "-v", "-f", "-F", "-o", "-h", "-t", "0.5", 
          "test1.gcda", "test2.gcda", NULL },
        { "gcov-tool", "overlap", "-t", "0.75", "-v", "-f", NULL },
        { "gcov-tool", "overlap", "-x", NULL },  // Unknown flag
        { "gcov-tool", "overlap", "-t", NULL },  // Missing argument
        { NULL }
    };
    
    // Prepend directory to gcda files
    char gcda1[MAX_CMD_LEN], gcda2[MAX_CMD_LEN];
    snprintf(gcda1, sizeof(gcda1), "%s/test1.gcda", temp_dir);
    snprintf(gcda2, sizeof(gcda2), "%s/test2.gcda", temp_dir);
    
    for (int i = 0; test_args[i][0] != NULL; i++) {
        // Build argument list
        char *argv[20];
        int argc = 0;
        
        // Replace "gcov-tool" with actual path
        argv[argc++] = (char *)gcov_tool_path;
        
        // Copy remaining arguments
        for (int j = 1; test_args[i][j] != NULL; j++) {
            if (strcmp(test_args[i][j], "test1.gcda") == 0) {
                argv[argc++] = gcda1;
            } else if (strcmp(test_args[i][j], "test2.gcda") == 0) {
                argv[argc++] = gcda2;
            } else {
                argv[argc++] = test_args[i][j];
            }
        }
        argv[argc] = NULL;
        
        printf("Test %d: ", i+1);
        for (int j = 0; j < argc; j++) {
            printf("%s ", argv[j]);
        }
        printf("\n");
        
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            execvp(argv[0], argv);
            perror("execvp failed");
            exit(1);
        } else if (pid > 0) {
            // Parent process
            int status;
            waitpid(pid, &status, 0);
            
            if (WIFEXITED(status)) {
                printf("  Exit status: %d\n", WEXITSTATUS(status));
            } else {
                printf("  Process terminated abnormally\n");
            }
        } else {
            perror("fork failed");
        }
    }
}

int main(int argc, char *argv[]) {
    // Get gcov-tool path from command line or use default
    const char *gcov_tool_path = "./gcov-tool";
    if (argc > 1) {
        gcov_tool_path = argv[1];
    }
    
    // Check if gcov-tool exists
    if (access(gcov_tool_path, X_OK) != 0) {
        fprintf(stderr, "Error: gcov-tool not found at %s\n", gcov_tool_path);
        fprintf(stderr, "Please build gcov-tool with coverage first:\n");
        fprintf(stderr, "  gcc --enable-coverage -o gcov-tool gcov-tool.cc\n");
        return 1;
    }
    
    // Create temporary directory for test files
    char temp_dir[] = "/tmp/gcov_test_XXXXXX";
    if (!mkdtemp(temp_dir)) {
        perror("Failed to create temp directory");
        return 1;
    }
    
    printf("Created temp directory: %s\n", temp_dir);
    
    // Create GCOV test files
    if (create_gcov_test_files(temp_dir) != 0) {
        fprintf(stderr, "Failed to create GCOV test files\n");
        // Clean up
        char cmd[MAX_CMD_LEN];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
        system(cmd);
        return 1;
    }
    
    // Test with system() calls
    test_overlap_options(temp_dir, gcov_tool_path);
    
    // Test with fork/exec
    test_with_fork_exec(temp_dir, gcov_tool_path);
    
    // Additional permutation tests
    printf("\n=== Testing flag permutations ===\n\n");
    
    // Generate permutations of flags programmatically
    char *flags[] = {"-v", "-f", "-F", "-o", "-h", "-t 0.5", NULL};
    int flag_count = 6;
    
    // Test different permutations (simplified - not all permutations)
    for (int mask = 1; mask < (1 << flag_count); mask++) {
        char cmd[MAX_CMD_LEN] = "";
        strcat(cmd, gcov_tool_path);
        strcat(cmd, " overlap ");
        
        int has_t_flag = 0;
        for (int j = 0; j < flag_count; j++) {
            if (mask & (1 << j)) {
                strcat(cmd, flags[j]);
                strcat(cmd, " ");
                if (j == 5) has_t_flag = 1;  // -t flag is included
            }
        }
        
        // Add gcda files
        char gcda_files[MAX_CMD_LEN];
        snprintf(gcda_files, sizeof(gcda_files), "%s/test1.gcda %s/test2.gcda", 
                 temp_dir, temp_dir);
        strcat(cmd, gcda_files);
        
        // Skip if -t flag is included without value (handled by error case above)
        if (has_t_flag) {
            printf("Testing combination: %s\n", cmd);
            int exit_status = run_command(cmd);
            printf("  Exit: %d\n\n", exit_status);
        }
    }
    
    // Clean up
    printf("\nCleaning up temp directory: %s\n", temp_dir);
    char cleanup_cmd[MAX_CMD_LEN];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
    system(cleanup_cmd);
    
    printf("\n=== Test complete ===\n");
    printf("To collect coverage data for gcov-tool.cc:\n");
    printf("1. Ensure gcov-tool was built with --enable-coverage\n");
    printf("2. Run: gcov gcov-tool.cc\n");
    printf("3. Check gcov-tool.c.gcov for coverage of lines 534-554\n");
    
    return 0;
}
