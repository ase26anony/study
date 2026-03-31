#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

#define TEST_SOURCE "test_coverage.c"
#define TEST_EXECUTABLE "test_coverage"
#define GCOV_DUMP_EXECUTABLE "./gcov-dump"

typedef struct {
    const char *name;
    const char *args[4];
    int expected_exit;
    int check_stderr;
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
            printf("Successfully compiled %s (generated .gcno file)\n", TEST_SOURCE);
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
    printf("Command: gcov-dump");
    for (int i = 0; test->args[i] != NULL; i++) {
        printf(" %s", test->args[i]);
    }
    printf("\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: execute gcov-dump
        if (test->check_stderr) {
            // Redirect stderr to stdout for unknown flag test
            dup2(STDOUT_FILENO, STDERR_FILENO);
        }
        execvp(GCOV_DUMP_EXECUTABLE, (char *const *)test->args);
        perror("Failed to execute gcov-dump");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process: wait for gcov-dump
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Exit code: %d (expected: %d)\n", exit_code, test->expected_exit);
            
            if (exit_code == test->expected_exit) {
                printf("✓ PASS\n");
                return 1;
            } else {
                printf("✗ FAIL - Wrong exit code\n");
                return 0;
            }
        } else if (WIFSIGNALED(status)) {
            printf("✗ FAIL - Process terminated by signal %d\n", WTERMSIG(status));
            return 0;
        } else {
            printf("✗ FAIL - Process didn't exit normally\n");
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

int main(void) {
    printf("=== GCOV-Dump Test Harness ===\n");
    printf("Testing executable: %s\n", GCOV_DUMP_EXECUTABLE);
    
    // Check if gcov-dump exists
    if (access(GCOV_DUMP_EXECUTABLE, X_OK) != 0) {
        fprintf(stderr, "Error: gcov-dump executable not found at %s\n", 
                GCOV_DUMP_EXECUTABLE);
        fprintf(stderr, "Please build gcov-dump first or adjust the path.\n");
        return EXIT_FAILURE;
    }
    
    // Create and compile test source for coverage files
    create_test_source();
    compile_with_coverage();
    
    // Define test cases
    test_case_t test_cases[] = {
        // Test 1: -h flag (help)
        {
            "Help flag (-h)",
            {GCOV_DUMP_EXECUTABLE, "-h", NULL},
            0, 0, NULL
        },
        
        // Test 2: -v flag (version)
        {
            "Version flag (-v)",
            {GCOV_DUMP_EXECUTABLE, "-v", NULL},
            0, 0, NULL
        },
        
        // Test 3: -l flag (dump contents)
        {
            "Dump contents flag (-l)",
            {GCOV_DUMP_EXECUTABLE, "-l", "test_coverage.gcno", NULL},
            0, 0, NULL
        },
        
        // Test 4: -p flag (dump positions)
        {
            "Dump positions flag (-p)",
            {GCOV_DUMP_EXECUTABLE, "-p", "test_coverage.gcno", NULL},
            0, 0, NULL
        },
        
        // Test 5: -r flag (dump raw)
        {
            "Dump raw flag (-r)",
            {GCOV_DUMP_EXECUTABLE, "-r", "test_coverage.gcno", NULL},
            0, 0, NULL
        },
        
        // Test 6: -s flag (dump stable)
        {
            "Dump stable flag (-s)",
            {GCOV_DUMP_EXECUTABLE, "-s", "test_coverage.gcno", NULL},
            0, 0, NULL
        },
        
        // Test 7: Unknown flag (-x)
        {
            "Unknown flag (-x)",
            {GCOV_DUMP_EXECUTABLE, "-x", NULL},
            EXIT_FAILURE, 1, "unknown flag"
        },
        
        // Test 8: Another unknown flag (-Z)
        {
            "Unknown flag (-Z)",
            {GCOV_DUMP_EXECUTABLE, "-Z", NULL},
            EXIT_FAILURE, 1, "unknown flag"
        },
        
        // Test 9: Unknown flag with question mark
        {
            "Unknown flag (-?)",
            {GCOV_DUMP_EXECUTABLE, "-?", NULL},
            EXIT_FAILURE, 1, "unknown flag"
        }
    };
    
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int passed = 0;
    
    // Run all test cases
    for (int i = 0; i < num_tests; i++) {
        passed += run_test_case(&test_cases[i], i + 1);
    }
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", num_tests);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", num_tests - passed);
    
    // Cleanup
    printf("\nCleaning up...\n");
    unlink(TEST_SOURCE);
    unlink(TEST_EXECUTABLE);
    unlink("test_coverage.gcno");
    
    if (passed == num_tests) {
        printf("\n✓ All tests passed!\n");
        return EXIT_SUCCESS;
    } else {
        printf("\n✗ Some tests failed\n");
        return EXIT_FAILURE;
    }
}
