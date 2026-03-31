#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

#define TEST_SOURCE "test_gcov.c"
#define TEST_EXECUTABLE "test_gcov"
#define GCNO_FILE "test_gcov.gcno"
#define GCOV_DUMP_EXECUTABLE "./gcov-dump"

typedef struct {
    const char *name;
    const char *args[4];
    int expected_exit;
    const char *expected_stderr;
} test_case_t;

// Create a minimal C source file for coverage testing
void create_test_source(void) {
    FILE *f = fopen(TEST_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source file");
        exit(EXIT_FAILURE);
    }
    
    fprintf(f, "int main() {\n");
    fprintf(f, "    int x = 0;\n");
    fprintf(f, "    if (x == 0) {\n");
    fprintf(f, "        x = 1;\n");
    fprintf(f, "    }\n");
    fprintf(f, "    return x;\n");
    fprintf(f, "}\n");
    
    fclose(f);
    printf("Created test source file: %s\n", TEST_SOURCE);
}

// Compile the test source with coverage flags
void compile_with_coverage(void) {
    printf("Compiling with coverage flags...\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: compile with coverage
        execlp("gcc", "gcc", "-O0", "--coverage", "-fprofile-arcs", 
               "-ftest-coverage", "-o", TEST_EXECUTABLE, TEST_SOURCE, NULL);
        perror("Failed to execute gcc");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process: wait for compilation
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Successfully compiled %s (generated %s)\n", 
                   TEST_SOURCE, GCNO_FILE);
        } else {
            fprintf(stderr, "Failed to compile test source\n");
            exit(EXIT_FAILURE);
        }
    } else {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
}

// Run a single test case
int run_test_case(const test_case_t *test, int test_num) {
    printf("\n=== Test %d: %s ===\n", test_num, test->name);
    printf("Command: %s", GCOV_DUMP_EXECUTABLE);
    
    for (int i = 0; test->args[i] != NULL; i++) {
        printf(" %s", test->args[i]);
    }
    printf("\n");
    
    // Create pipes for stderr capture
    int stderr_pipe[2];
    if (pipe(stderr_pipe) == -1) {
        perror("pipe failed");
        return 0;
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        close(stderr_pipe[0]);  // Close read end
        dup2(stderr_pipe[1], STDERR_FILENO);  // Redirect stderr to pipe
        close(stderr_pipe[1]);
        
        // Build argument list
        char *args[10];
        int arg_count = 0;
        args[arg_count++] = (char *)GCOV_DUMP_EXECUTABLE;
        
        for (int i = 0; test->args[i] != NULL && arg_count < 9; i++) {
            args[arg_count++] = (char *)test->args[i];
        }
        args[arg_count] = NULL;
        
        execvp(GCOV_DUMP_EXECUTABLE, args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        close(stderr_pipe[1]);  // Close write end
        
        // Read stderr output
        char stderr_buffer[1024] = {0};
        ssize_t bytes_read = read(stderr_pipe[0], stderr_buffer, 
                                  sizeof(stderr_buffer) - 1);
        close(stderr_pipe[0]);
        
        // Wait for child
        int status;
        waitpid(pid, &status, 0);
        
        int exit_code = -1;
        if (WIFEXITED(status)) {
            exit_code = WEXITSTATUS(status);
        }
        
        // Check results
        int passed = 1;
        
        if (exit_code != test->expected_exit) {
            printf("FAIL: Expected exit code %d, got %d\n", 
                   test->expected_exit, exit_code);
            passed = 0;
        }
        
        if (test->expected_stderr != NULL) {
            if (bytes_read > 0 && strstr(stderr_buffer, test->expected_stderr) == NULL) {
                printf("FAIL: Expected stderr to contain '%s'\n", 
                       test->expected_stderr);
                printf("Got: %s\n", stderr_buffer);
                passed = 0;
            } else if (bytes_read > 0) {
                printf("PASS: Found expected stderr message\n");
            }
        }
        
        if (passed) {
            printf("PASS: Test %d completed successfully\n", test_num);
        }
        
        return passed;
    } else {
        perror("fork failed");
        return 0;
    }
}

int main(void) {
    printf("=== GCOV-DUMP Test Harness ===\n");
    
    // Check if gcov-dump exists
    if (access(GCOV_DUMP_EXECUTABLE, X_OK) != 0) {
        fprintf(stderr, "Error: %s not found or not executable\n", 
                GCOV_DUMP_EXECUTABLE);
        fprintf(stderr, "Please ensure gcov-dump is in the current directory\n");
        return EXIT_FAILURE;
    }
    
    // Create test source and compile with coverage
    create_test_source();
    compile_with_coverage();
    
    // Define test cases
    test_case_t tests[] = {
        {
            .name = "Help flag (-h)",
            .args = {"-h", NULL},
            .expected_exit = 0,
            .expected_stderr = NULL
        },
        {
            .name = "Version flag (-v)",
            .args = {"-v", NULL},
            .expected_exit = 0,
            .expected_stderr = NULL
        },
        {
            .name = "Dump contents flag (-l)",
            .args = {"-l", GCNO_FILE, NULL},
            .expected_exit = 0,
            .expected_stderr = NULL
        },
        {
            .name = "Dump positions flag (-p)",
            .args = {"-p", GCNO_FILE, NULL},
            .expected_exit = 0,
            .expected_stderr = NULL
        },
        {
            .name = "Dump raw flag (-r)",
            .args = {"-r", GCNO_FILE, NULL},
            .expected_exit = 0,
            .expected_stderr = NULL
        },
        {
            .name = "Dump stable flag (-s)",
            .args = {"-s", GCNO_FILE, NULL},
            .expected_exit = 0,
            .expected_stderr = NULL
        },
        {
            .name = "Unknown flag (-x)",
            .args = {"-x", NULL},
            .expected_exit = 1,  // Non-zero exit for error
            .expected_stderr = "unknown flag `x'"
        }
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed_count = 0;
    
    // Run all tests
    for (int i = 0; i < num_tests; i++) {
        passed_count += run_test_case(&tests[i], i + 1);
    }
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", num_tests);
    printf("Passed: %d\n", passed_count);
    printf("Failed: %d\n", num_tests - passed_count);
    
    // Cleanup
    printf("\nCleaning up...\n");
    unlink(TEST_SOURCE);
    unlink(TEST_EXECUTABLE);
    // Note: Keeping .gcno file for inspection if needed
    
    return (passed_count == num_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}
