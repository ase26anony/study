#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEST_SOURCE "test.c"
#define TEST_BINARY "test"
#define TEST_GCNO "test.gcno"
#define GCOV_DUMP_PATH "./gcov-dump"

typedef struct {
    const char *name;
    const char *args[5];
    int expected_exit;
    const char *expected_stderr;
} test_case;

// Create minimal test source file
void create_test_source(void) {
    FILE *f = fopen(TEST_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

// Compile test source with coverage flags
void compile_with_coverage(void) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: compile with coverage
        execlp("gcc", "gcc", "-O0", "--coverage", "-fprofile-arcs", 
               "-ftest-coverage", "-o", TEST_BINARY, TEST_SOURCE, NULL);
        perror("Failed to execute gcc");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent: wait for compilation
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Failed to compile test program\n");
            exit(EXIT_FAILURE);
        }
    } else {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
}

// Run a single test case
int run_test(const char *test_name, const char *args[], 
             int expected_exit, const char *expected_stderr) {
    printf("Running test: %s\n", test_name);
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: execute gcov-dump
        execvp(GCOV_DUMP_PATH, (char * const *)args);
        perror("Failed to execute gcov-dump");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent: wait for gcov-dump
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("  Exit code: %d (expected: %d)\n", exit_code, expected_exit);
            
            if (exit_code == expected_exit) {
                printf("  ✓ PASS\n");
                return 1;
            } else {
                printf("  ✗ FAIL - Wrong exit code\n");
                return 0;
            }
        } else {
            printf("  ✗ FAIL - Process didn't exit normally\n");
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

int main(void) {
    printf("=== GCOV-DUMP Test Harness ===\n\n");
    
    // Check if gcov-dump exists
    struct stat st;
    if (stat(GCOV_DUMP_PATH, &st) != 0) {
        fprintf(stderr, "Error: %s not found\n", GCOV_DUMP_PATH);
        fprintf(stderr, "Please ensure gcov-dump is in the current directory\n");
        return EXIT_FAILURE;
    }
    
    // Create test files
    printf("Creating test files...\n");
    create_test_source();
    compile_with_coverage();
    
    // Verify .gcno file was created
    if (stat(TEST_GCNO, &st) != 0) {
        fprintf(stderr, "Error: %s not created\n", TEST_GCNO);
        return EXIT_FAILURE;
    }
    printf("Created %s successfully\n\n", TEST_GCNO);
    
    // Define test cases
    test_case tests[] = {
        // Test -h (help)
        {
            "Help flag (-h)",
            {GCOV_DUMP_PATH, "-h", NULL},
            0,
            NULL
        },
        // Test -v (version)
        {
            "Version flag (-v)",
            {GCOV_DUMP_PATH, "-v", NULL},
            0,
            NULL
        },
        // Test -l (dump contents)
        {
            "Dump contents flag (-l)",
            {GCOV_DUMP_PATH, "-l", TEST_GCNO, NULL},
            0,
            NULL
        },
        // Test -p (dump positions)
        {
            "Dump positions flag (-p)",
            {GCOV_DUMP_PATH, "-p", TEST_GCNO, NULL},
            0,
            NULL
        },
        // Test -r (dump raw)
        {
            "Dump raw flag (-r)",
            {GCOV_DUMP_PATH, "-r", TEST_GCNO, NULL},
            0,
            NULL
        },
        // Test -s (dump stable)
        {
            "Dump stable flag (-s)",
            {GCOV_DUMP_PATH, "-s", TEST_GCNO, NULL},
            0,
            NULL
        },
        // Test invalid flag
        {
            "Invalid flag (-x)",
            {GCOV_DUMP_PATH, "-x", NULL},
            1,  // Should exit with error
            "unknown flag"
        },
        // Test another invalid flag
        {
            "Invalid flag (-Z)",
            {GCOV_DUMP_PATH, "-Z", NULL},
            1,
            "unknown flag"
        },
        // Test invalid flag with question mark
        {
            "Invalid flag (-?)",
            {GCOV_DUMP_PATH, "-?", NULL},
            1,
            "unknown flag"
        }
    };
    
    // Run all tests
    int total_tests = sizeof(tests) / sizeof(tests[0]);
    int passed_tests = 0;
    
    for (int i = 0; i < total_tests; i++) {
        printf("\nTest %d/%d: ", i + 1, total_tests);
        passed_tests += run_test(tests[i].name, tests[i].args, 
                                tests[i].expected_exit, tests[i].expected_stderr);
    }
    
    // Cleanup
    printf("\n=== Cleaning up ===\n");
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    unlink(TEST_GCNO);
    unlink("test.gcda");  // Just in case
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", total_tests - passed_tests);
    
    return (passed_tests == total_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}
