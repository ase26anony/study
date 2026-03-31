#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

#define TEST_SRC "test.c"
#define TEST_EXEC "test"
#define TEST_GCNO "test.gcno"
#define GCOV_DUMP_PATH "./gcov-dump"

typedef struct {
    const char *name;
    char **argv;
    int expected_exit;
    const char *expected_stderr;
} test_case_t;

void create_test_source(void) {
    FILE *f = fopen(TEST_SRC, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(1);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

void compile_test_gcno(void) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: compile with coverage
        char *argv[] = {
            "gcc",
            "-O0",
            "--coverage",
            "-fprofile-arcs",
            "-ftest-coverage",
            "-o", TEST_EXEC,
            TEST_SRC,
            NULL
        };
        execvp("gcc", argv);
        perror("execvp gcc failed");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Failed to compile test.gcno\n");
            exit(1);
        }
        printf("Generated %s successfully\n", TEST_GCNO);
    } else {
        perror("fork failed");
        exit(1);
    }
}

int run_test(const char *name, char **argv, int expected_exit, const char *expected_stderr) {
    printf("\n=== Testing: %s ===\n", name);
    printf("Command: ");
    for (int i = 0; argv[i]; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: run gcov-dump
        execvp(GCOV_DUMP_PATH, argv);
        perror("execvp failed");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Exit code: %d (expected: %d)\n", exit_code, expected_exit);
            
            if (exit_code == expected_exit) {
                printf("✓ PASS: %s\n", name);
                return 1;
            } else {
                printf("✗ FAIL: %s - wrong exit code\n", name);
                return 0;
            }
        } else {
            printf("✗ FAIL: %s - process didn't exit normally\n", name);
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

int main(void) {
    printf("=== GCOV-DUMP Test Harness ===\n");
    
    // Create test source and compile to generate .gcno file
    printf("\nCreating test files...\n");
    create_test_source();
    compile_test_gcno();
    
    // Define test cases
    char *help_argv[] = {GCOV_DUMP_PATH, "-h", NULL};
    char *version_argv[] = {GCOV_DUMP_PATH, "-v", NULL};
    char *dump_contents_argv[] = {GCOV_DUMP_PATH, "-l", TEST_GCNO, NULL};
    char *dump_positions_argv[] = {GCOV_DUMP_PATH, "-p", TEST_GCNO, NULL};
    char *dump_raw_argv[] = {GCOV_DUMP_PATH, "-r", TEST_GCNO, NULL};
    char *dump_stable_argv[] = {GCOV_DUMP_PATH, "-s", TEST_GCNO, NULL};
    char *unknown_flag_argv[] = {GCOV_DUMP_PATH, "-x", NULL};
    
    test_case_t tests[] = {
        {"Help (-h)", help_argv, 0, NULL},
        {"Version (-v)", version_argv, 0, NULL},
        {"Dump contents (-l)", dump_contents_argv, 0, NULL},
        {"Dump positions (-p)", dump_positions_argv, 0, NULL},
        {"Dump raw (-r)", dump_raw_argv, 0, NULL},
        {"Dump stable (-s)", dump_stable_argv, 0, NULL},
        {"Unknown flag (-x)", unknown_flag_argv, 1, "unknown flag `x'"},
    };
    
    int total_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    
    // Run all tests
    for (int i = 0; i < total_tests; i++) {
        passed += run_test(tests[i].name, tests[i].argv, 
                          tests[i].expected_exit, tests[i].expected_stderr);
    }
    
    // Cleanup
    unlink(TEST_SRC);
    unlink(TEST_EXEC);
    unlink(TEST_GCNO);
    
    // Summary
    printf("\n=== Summary ===\n");
    printf("Passed: %d/%d\n", passed, total_tests);
    
    return (passed == total_tests) ? 0 : 1;
}
