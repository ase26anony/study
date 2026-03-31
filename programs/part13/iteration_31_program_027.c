#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

#define TEST_SOURCE "test.c"
#define TEST_BINARY "test"
#define TEST_GCNO "test.gcno"
#define GCOV_DUMP_PATH "./gcov-dump"

typedef struct {
    const char *name;
    char **argv;
    int expected_exit;
    const char *expected_stderr_prefix;
} test_case_t;

void create_test_source(void) {
    FILE *f = fopen(TEST_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(1);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

void compile_test_with_coverage(void) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: compile with coverage
        execlp("gcc", "gcc", "-O0", "--coverage", "-fprofile-arcs", 
               "-ftest-coverage", "-o", TEST_BINARY, TEST_SOURCE, NULL);
        perror("Failed to execute gcc");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Failed to compile test program\n");
            exit(1);
        }
    } else {
        perror("fork failed");
        exit(1);
    }
}

int run_test(const char *name, char *const argv[], int expected_exit, 
             const char *expected_stderr_prefix) {
    printf("Running: %s\n", name);
    
    // Create pipes for stderr
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
        
        execvp(GCOV_DUMP_PATH, argv);
        perror("Failed to execute gcov-dump");
        exit(1);
    } else if (pid > 0) {
        // Parent process
        close(stderr_pipe[1]);  // Close write end
        
        int status;
        waitpid(pid, &status, 0);
        
        // Read stderr output
        char stderr_buf[1024] = {0};
        ssize_t bytes_read = read(stderr_pipe[0], stderr_buf, sizeof(stderr_buf) - 1);
        close(stderr_pipe[0]);
        
        int passed = 1;
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("  Exit code: %d (expected: %d)", exit_code, expected_exit);
            
            if (exit_code != expected_exit) {
                printf("  FAIL - Wrong exit code\n");
                passed = 0;
            } else {
                printf("  OK\n");
            }
            
            // Check stderr if expected
            if (expected_stderr_prefix && bytes_read > 0) {
                stderr_buf[bytes_read] = '\0';
                printf("  Stderr: %s", stderr_buf);
                if (strstr(stderr_buf, expected_stderr_prefix) == NULL) {
                    printf("  FAIL - Expected stderr to contain '%s'\n", expected_stderr_prefix);
                    passed = 0;
                } else {
                    printf("  OK - Found expected stderr pattern\n");
                }
            }
        } else {
            printf("  FAIL - Process didn't exit normally\n");
            passed = 0;
        }
        
        printf("\n");
        return passed;
    } else {
        perror("fork failed");
        return 0;
    }
}

int main(void) {
    printf("=== GCOV-Dump Test Harness ===\n\n");
    
    // Step 1: Create test source and compile with coverage
    printf("1. Creating test source file...\n");
    create_test_source();
    
    printf("2. Compiling test program with coverage flags...\n");
    compile_test_with_coverage();
    printf("   Generated %s\n\n", TEST_GCNO);
    
    // Step 2: Define test cases
    char *help_args[] = {GCOV_DUMP_PATH, "-h", NULL};
    char *version_args[] = {GCOV_DUMP_PATH, "-v", NULL};
    char *dump_contents_args[] = {GCOV_DUMP_PATH, "-l", TEST_GCNO, NULL};
    char *dump_positions_args[] = {GCOV_DUMP_PATH, "-p", TEST_GCNO, NULL};
    char *dump_raw_args[] = {GCOV_DUMP_PATH, "-r", TEST_GCNO, NULL};
    char *dump_stable_args[] = {GCOV_DUMP_PATH, "-s", TEST_GCNO, NULL};
    char *unknown_flag_args[] = {GCOV_DUMP_PATH, "-x", NULL};
    char *unknown_flag2_args[] = {GCOV_DUMP_PATH, "-Z", NULL};
    char *unknown_flag3_args[] = {GCOV_DUMP_PATH, "-?", NULL};
    
    test_case_t tests[] = {
        {"Help (-h)", help_args, 0, NULL},
        {"Version (-v)", version_args, 0, NULL},
        {"Dump contents (-l)", dump_contents_args, 0, NULL},
        {"Dump positions (-p)", dump_positions_args, 0, NULL},
        {"Dump raw (-r)", dump_raw_args, 0, NULL},
        {"Dump stable (-s)", dump_stable_args, 0, NULL},
        {"Unknown flag (-x)", unknown_flag_args, 1, "unknown flag `x'"},
        {"Unknown flag (-Z)", unknown_flag2_args, 1, "unknown flag `Z'"},
        {"Unknown flag (-?)", unknown_flag3_args, 1, "unknown flag `?'"},
    };
    
    // Step 3: Run all tests
    int total_tests = sizeof(tests) / sizeof(tests[0]);
    int passed_tests = 0;
    
    for (int i = 0; i < total_tests; i++) {
        passed_tests += run_test(tests[i].name, tests[i].argv, 
                                tests[i].expected_exit, 
                                tests[i].expected_stderr_prefix);
    }
    
    // Step 4: Print summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", total_tests - passed_tests);
    
    // Cleanup
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    unlink(TEST_GCNO);
    
    return (passed_tests == total_tests) ? 0 : 1;
}
