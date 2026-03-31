/**
 * gcov-dump_parser_test.c
 * 
 * Comprehensive test program for gcov-dump command-line flag parsing.
 * Specifically targets the switch statement in gcov-dump.cc lines 111-130.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/**
 * Creates a minimal, valid .gcda file for testing.
 * Compiles a trivial C program with coverage flags and runs it.
 */
static int create_dummy_gcda(void) {
    const char *source_code = 
        "#include <stdio.h>\n"
        "int main() {\n"
        "    printf(\"Dummy program for coverage\\n\");\n"
        "    return 0;\n"
        "}\n";
    
    FILE *fp = fopen("dummy_test.c", "w");
    if (!fp) {
        perror("Failed to create dummy_test.c");
        return -1;
    }
    fputs(source_code, fp);
    fclose(fp);
    
    // Compile with coverage flags
    int compile_status = system("gcc -fprofile-arcs -ftest-coverage -O0 dummy_test.c -o dummy_test");
    if (compile_status != 0) {
        fprintf(stderr, "Failed to compile dummy test program\n");
        return -1;
    }
    
    // Run to generate .gcda file
    int run_status = system("./dummy_test > /dev/null 2>&1");
    if (run_status != 0) {
        fprintf(stderr, "Failed to run dummy test program\n");
        return -1;
    }
    
    return 0;
}

/**
 * Execute gcov-dump using execvp for precise argument control.
 * Returns child process exit status.
 */
static int exec_gcov_dump(const char *args[]) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        // Child process
        execvp("gcov-dump", (char *const *)args);
        // If execvp returns, it failed
        fprintf(stderr, "execvp failed for gcov-dump: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return -1;
        }
    }
}

/**
 * Execute gcov-dump using system() call.
 */
static void system_gcov_dump(const char *cmd) {
    printf("Testing with system(): %s\n", cmd);
    int status = system(cmd);
    printf("Exit status: %d\n\n", status);
}

int main(void) {
    printf("=== Starting gcov-dump flag parser tests ===\n\n");
    
    // Create dummy .gcda file for file-based tests
    printf("Creating dummy .gcda file...\n");
    if (create_dummy_gcda() != 0) {
        fprintf(stderr, "Warning: Could not create dummy .gcda file\n");
        printf("File-based tests will be skipped\n\n");
    } else {
        printf("Dummy .gcda file created successfully\n\n");
    }
    
    // Test individual flags (using execvp for precise control)
    printf("--- Testing individual flags with execvp ---\n");
    
    // Array of test cases: each NULL-terminated array of arguments
    const char *test_cases[][5] = {
        // Basic individual flags
        {"gcov-dump", "-h", NULL},
        {"gcov-dump", "-v", NULL},
        {"gcov-dump", "-l", NULL},
        {"gcov-dump", "-p", NULL},
        {"gcov-dump", "-r", NULL},
        {"gcov-dump", "-s", NULL},
        
        // Invalid flag to trigger default case
        {"gcov-dump", "-x", NULL},
        
        // Combination of valid flags
        {"gcov-dump", "-l", "-p", NULL},
        {"gcov-dump", "-r", "-s", "-v", NULL},
        {"gcov-dump", "-h", "-l", NULL},  // -h may cause early exit
        
        // Repeated flags
        {"gcov-dump", "-p", "-p", NULL},
        {"gcov-dump", "-l", "-l", "-p", NULL},
        
        // Combined short options (if supported by getopt)
        {"gcov-dump", "-lp", NULL},
        {"gcov-dump", "-rs", NULL},
        {"gcov-dump", "-lprs", NULL},
        
        // With positional arguments (gcov files)
        {"gcov-dump", "-l", "dummy_test.gcda", NULL},
        {"gcov-dump", "-p", "-r", "dummy_test.gcda", NULL},
        
        // With -- delimiter
        {"gcov-dump", "-l", "--", "dummy_test.gcda", NULL},
        {"gcov-dump", "--", "-l", "dummy_test.gcda", NULL},  // -l after -- should be treated as file
        
        // No arguments (may trigger default behavior)
        {"gcov-dump", NULL},
        
        // End marker
        {NULL}
    };
    
    for (int i = 0; test_cases[i][0] != NULL; i++) {
        printf("Test %d: ", i + 1);
        for (int j = 0; test_cases[i][j] != NULL; j++) {
            printf("%s ", test_cases[i][j]);
        }
        printf("\n");
        
        int status = exec_gcov_dump(test_cases[i]);
        printf("Exit status: %d\n\n", status);
    }
    
    // Test with system() calls for different syntactic styles
    printf("--- Testing with system() calls ---\n");
    
    // Set environment variable that might affect parsing
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    
    system_gcov_dump("gcov-dump -h");
    system_gcov_dump("gcov-dump -v");
    system_gcov_dump("gcov-dump -l -p");
    system_gcov_dump("gcov-dump -r -s");
    
    // Test with output redirection to capture stderr
    printf("Testing invalid flag with stderr capture:\n");
    system("gcov-dump -x 2>&1 | grep 'unknown flag'");
    printf("\n");
    
    // Test combined flags with system()
    system_gcov_dump("gcov-dump -lp dummy_test.gcda");
    system_gcov_dump("gcov-dump -l -p -- dummy_test.gcda");
    
    // Test with different flag orders
    system_gcov_dump("gcov-dump dummy_test.gcda -l");
    system_gcov_dump("gcov-dump -- -l dummy_test.gcda");
    
    // Clean up
    printf("--- Cleaning up ---\n");
    system("rm -f dummy_test.c dummy_test dummy_test.gcda dummy_test.gcno 2>/dev/null");
    
    printf("\n=== All tests completed ===\n");
    
    return 0;
}
