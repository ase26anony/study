#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

#define TEST_SOURCE "test_gcov.c"
#define TEST_BINARY "test_gcov"
#define TEST_GCNO "test_gcov.gcno"
#define GCOV_DUMP_PATH "./gcov-dump"

typedef struct {
    const char *name;
    char **argv;
    int argc;
    int expected_exit;
    const char *expected_stderr_prefix;
} test_case_t;

void create_test_source() {
    FILE *f = fopen(TEST_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(1);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

int compile_with_coverage() {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        char *argv[] = {
            "gcc",
            "-O0",
            "--coverage",
            "-fprofile-arcs",
            "-ftest-coverage",
            "-o", TEST_BINARY,
            TEST_SOURCE,
            NULL
        };
        execvp("gcc", argv);
        perror("execvp gcc failed");
        exit(1);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("✓ Compiled test program with coverage\n");
            return 1;
        } else {
            printf("✗ Failed to compile test program\n");
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

int run_test(const char *name, char *const argv[], int expected_exit, 
             const char *expected_stderr_prefix) {
    printf("\n=== Testing: %s ===\n", name);
    printf("Command:");
    for (int i = 0; argv[i] != NULL; i++) {
        printf(" %s", argv[i]);
    }
    printf("\n");
    
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
        
        execvp(argv[0], argv);
        perror("execvp failed");
        exit(1);
    } else if (pid > 0) {
        // Parent process
        close(stderr_pipe[1]);  // Close write end
        
        // Read stderr output
        char stderr_buf[1024] = {0};
        ssize_t bytes_read = read(stderr_pipe[0], stderr_buf, sizeof(stderr_buf) - 1);
        close(stderr_pipe[0]);
        
        int status;
        waitpid(pid, &status, 0);
        
        int exit_code = -1;
        if (WIFEXITED(status)) {
            exit_code = WEXITSTATUS(status);
        }
        
        // Check exit code
        if (exit_code == expected_exit) {
            printf("✓ Exit code: %d (expected %d)\n", exit_code, expected_exit);
        } else {
            printf("✗ Exit code: %d (expected %d)\n", exit_code, expected_exit);
            return 0;
        }
        
        // Check stderr if expected
        if (expected_stderr_prefix) {
            if (bytes_read > 0 && strstr(stderr_buf, expected_stderr_prefix) != NULL) {
                printf("✓ Stderr contains: %s\n", expected_stderr_prefix);
            } else {
                printf("✗ Stderr missing expected text: %s\n", expected_stderr_prefix);
                if (bytes_read > 0) {
                    printf("  Actual stderr: %s\n", stderr_buf);
                }
                return 0;
            }
        } else if (bytes_read > 0) {
            // Print any unexpected stderr
            printf("Note: Unexpected stderr output: %s\n", stderr_buf);
        }
        
        return 1;
    } else {
        perror("fork failed");
        return 0;
    }
}

int main() {
    printf("=== GCOV-Dump Test Harness ===\n");
    
    // Create test source file
    create_test_source();
    
    // Compile with coverage to generate .gcno file
    if (!compile_with_coverage()) {
        printf("Failed to compile test program. Exiting.\n");
        return 1;
    }
    
    // Check if .gcno file exists
    if (access(TEST_GCNO, F_OK) != 0) {
        printf("Warning: %s not found. Trying to run test program...\n", TEST_GCNO);
        // Run the test program to ensure .gcno exists
        pid_t pid = fork();
        if (pid == 0) {
            execl(TEST_BINARY, TEST_BINARY, NULL);
            exit(1);
        } else if (pid > 0) {
            waitpid(pid, NULL, 0);
        }
    }
    
    if (access(TEST_GCNO, F_OK) != 0) {
        printf("Error: %s still not found after running test program\n", TEST_GCNO);
        return 1;
    }
    
    printf("✓ Found %s for testing\n", TEST_GCNO);
    
    // Define test cases
    char *help_args[] = {GCOV_DUMP_PATH, "-h", NULL};
    char *version_args[] = {GCOV_DUMP_PATH, "-v", NULL};
    char *dump_contents_args[] = {GCOV_DUMP_PATH, "-l", TEST_GCNO, NULL};
    char *dump_positions_args[] = {GCOV_DUMP_PATH, "-p", TEST_GCNO, NULL};
    char *dump_raw_args[] = {GCOV_DUMP_PATH, "-r", TEST_GCNO, NULL};
    char *dump_stable_args[] = {GCOV_DUMP_PATH, "-s", TEST_GCNO, NULL};
    char *unknown_flag_args[] = {GCOV_DUMP_PATH, "-x", NULL};
    char *another_unknown_args[] = {GCOV_DUMP_PATH, "-Z", NULL};
    
    test_case_t tests[] = {
        {"Help (-h)", help_args, 2, 0, NULL},
        {"Version (-v)", version_args, 2, 0, NULL},
        {"Dump contents (-l)", dump_contents_args, 3, 0, NULL},
        {"Dump positions (-p)", dump_positions_args, 3, 0, NULL},
        {"Dump raw (-r)", dump_raw_args, 3, 0, NULL},
        {"Dump stable (-s)", dump_stable_args, 3, 0, NULL},
        {"Unknown flag (-x)", unknown_flag_args, 2, 1, "unknown flag `x'"},
        {"Unknown flag (-Z)", another_unknown_args, 2, 1, "unknown flag `Z'"},
    };
    
    int total_tests = sizeof(tests) / sizeof(tests[0]);
    int passed_tests = 0;
    
    // Run all tests
    for (int i = 0; i < total_tests; i++) {
        if (run_test(tests[i].name, tests[i].argv, 
                    tests[i].expected_exit, 
                    tests[i].expected_stderr_prefix)) {
            passed_tests++;
        }
    }
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d tests\n", passed_tests, total_tests);
    
    // Cleanup
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    // Keep .gcno file for inspection if needed
    
    return (passed_tests == total_tests) ? 0 : 1;
}
