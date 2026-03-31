#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define TEST_SOURCE "test.c"
#define TEST_EXECUTABLE "test"
#define TEST_GCNO "test.gcno"
#define GCOV_DUMP_EXECUTABLE "./gcov-dump"

typedef struct {
    const char *description;
    char **argv;
    int argc;
    int expected_exit_code;
    const char *expected_stderr_substr;
} test_case_t;

void create_test_source(void) {
    FILE *f = fopen(TEST_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

void compile_test_with_coverage(void) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: compile with coverage
        execlp("gcc", "gcc", "-O0", "--coverage", "-fprofile-arcs", 
               "-ftest-coverage", "-o", TEST_EXECUTABLE, TEST_SOURCE, NULL);
        perror("Failed to execute gcc");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
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

int run_test_case(const test_case_t *test) {
    printf("Running: %s\n", test->description);
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: execute gcov-dump
        execvp(GCOV_DUMP_EXECUTABLE, test->argv);
        perror("Failed to execute gcov-dump");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("  Exit code: %d (expected: %d)\n", exit_code, test->expected_exit_code);
            
            if (exit_code == test->expected_exit_code) {
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
    
    // Create test source file
    printf("Creating test source file...\n");
    create_test_source();
    
    // Compile with coverage to generate .gcno file
    printf("Compiling test program with coverage...\n");
    compile_test_with_coverage();
    
    // Verify .gcno file exists
    if (access(TEST_GCNO, F_OK) != 0) {
        fprintf(stderr, "Error: %s not generated\n", TEST_GCNO);
        exit(EXIT_FAILURE);
    }
    printf("Generated %s successfully\n\n", TEST_GCNO);
    
    // Define test cases
    char *help_argv[] = {GCOV_DUMP_EXECUTABLE, "-h", NULL};
    char *version_argv[] = {GCOV_DUMP_EXECUTABLE, "-v", NULL};
    char *dump_contents_argv[] = {GCOV_DUMP_EXECUTABLE, "-l", TEST_GCNO, NULL};
    char *dump_positions_argv[] = {GCOV_DUMP_EXECUTABLE, "-p", TEST_GCNO, NULL};
    char *dump_raw_argv[] = {GCOV_DUMP_EXECUTABLE, "-r", TEST_GCNO, NULL};
    char *dump_stable_argv[] = {GCOV_DUMP_EXECUTABLE, "-s", TEST_GCNO, NULL};
    char *unknown_flag_argv[] = {GCOV_DUMP_EXECUTABLE, "-x", NULL};
    
    test_case_t test_cases[] = {
        {
            "Test -h flag (help)",
            help_argv,
            2,
            0,
            NULL
        },
        {
            "Test -v flag (version)",
            version_argv,
            2,
            0,
            NULL
        },
        {
            "Test -l flag (dump contents)",
            dump_contents_argv,
            3,
            0,
            NULL
        },
        {
            "Test -p flag (dump positions)",
            dump_positions_argv,
            3,
            0,
            NULL
        },
        {
            "Test -r flag (dump raw)",
            dump_raw_argv,
            3,
            0,
            NULL
        },
        {
            "Test -s flag (dump stable)",
            dump_stable_argv,
            3,
            0,
            NULL
        },
        {
            "Test unknown flag -x",
            unknown_flag_argv,
            2,
            1,  // Should exit with error
            "unknown flag"
        }
    };
    
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int passed = 0;
    
    // Run all test cases
    for (int i = 0; i < num_tests; i++) {
        printf("\nTest %d/%d: ", i + 1, num_tests);
        if (run_test_case(&test_cases[i])) {
            passed++;
        }
    }
    
    // Summary
    printf("\n=== Summary ===\n");
    printf("Passed: %d/%d tests\n", passed, num_tests);
    
    // Cleanup
    unlink(TEST_SOURCE);
    unlink(TEST_EXECUTABLE);
    unlink(TEST_GCNO);
    
    return (passed == num_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}
