#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

#define TEST_SOURCE "test.c"
#define TEST_BINARY "test"
#define TEST_GCNO "test.gcno"
#define GCOV_DUMP_PATH "./gcov-dump"

typedef struct {
    const char *name;
    const char *args[4];
    int expected_exit;
    int should_have_output;
} test_case_t;

// Create a minimal C source file for generating .gcno
void create_test_source(void) {
    FILE *f = fopen(TEST_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source file");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

// Compile the test source with coverage flags to generate .gcno
int compile_with_coverage(void) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: compile with coverage
        execlp("gcc", "gcc", "-O0", "--coverage", "-fprofile-arcs", 
               "-ftest-coverage", "-o", TEST_BINARY, TEST_SOURCE, NULL);
        perror("Failed to execute gcc");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Failed to compile test source\n");
            return 0;
        }
        return 1;
    } else {
        perror("fork failed");
        return 0;
    }
}

// Run a single test case
int run_test(const char *test_name, const char **args, int expected_exit, 
             int should_have_output) {
    printf("Running test: %s\n", test_name);
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: execute gcov-dump
        execvp(GCOV_DUMP_PATH, (char *const *)args);
        // If we get here, exec failed
        fprintf(stderr, "Failed to execute %s: %s\n", GCOV_DUMP_PATH, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
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
    if (access(GCOV_DUMP_PATH, X_OK) != 0) {
        fprintf(stderr, "Error: %s not found or not executable\n", GCOV_DUMP_PATH);
        fprintf(stderr, "Please ensure gcov-dump is in the current directory\n");
        return EXIT_FAILURE;
    }
    
    // Create test files
    printf("Creating test source file...\n");
    create_test_source();
    
    printf("Compiling with coverage flags...\n");
    if (!compile_with_coverage()) {
        fprintf(stderr, "Failed to generate .gcno file\n");
        return EXIT_FAILURE;
    }
    
    // Check if .gcno file was created
    if (access(TEST_GCNO, R_OK) != 0) {
        fprintf(stderr, "Error: %s not found after compilation\n", TEST_GCNO);
        return EXIT_FAILURE;
    }
    
    printf("Test files created successfully\n\n");
    
    // Define test cases
    test_case_t tests[] = {
        // Test 1: -h (help)
        {
            "Help flag (-h)",
            {GCOV_DUMP_PATH, "-h", NULL},
            0,  // Expected exit code
            0   // Should have output
        },
        // Test 2: -v (version)
        {
            "Version flag (-v)",
            {GCOV_DUMP_PATH, "-v", NULL},
            0,
            0
        },
        // Test 3: -l (dump contents)
        {
            "Dump contents flag (-l)",
            {GCOV_DUMP_PATH, "-l", TEST_GCNO, NULL},
            0,
            1
        },
        // Test 4: -p (dump positions)
        {
            "Dump positions flag (-p)",
            {GCOV_DUMP_PATH, "-p", TEST_GCNO, NULL},
            0,
            1
        },
        // Test 5: -r (dump raw)
        {
            "Dump raw flag (-r)",
            {GCOV_DUMP_PATH, "-r", TEST_GCNO, NULL},
            0,
            1
        },
        // Test 6: -s (dump stable)
        {
            "Dump stable flag (-s)",
            {GCOV_DUMP_PATH, "-s", TEST_GCNO, NULL},
            0,
            1
        },
        // Test 7: Invalid flag (-x)
        {
            "Invalid flag (-x)",
            {GCOV_DUMP_PATH, "-x", NULL},
            EXIT_FAILURE,  // Should exit with failure
            0
        }
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    
    // Run all tests
    for (int i = 0; i < num_tests; i++) {
        printf("\nTest %d/%d: ", i + 1, num_tests);
        if (run_test(tests[i].name, tests[i].args, 
                     tests[i].expected_exit, tests[i].should_have_output)) {
            passed++;
        }
    }
    
    // Cleanup
    printf("\n=== Cleaning up test files ===\n");
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    unlink(TEST_GCNO);
    unlink("test.gcda");  // In case it was created
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", num_tests);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", num_tests - passed);
    
    return (passed == num_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}
