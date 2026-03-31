#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEST_SRC "test.c"
#define TEST_EXE "test"
#define TEST_GCNO "test.gcno"
#define GCOV_DUMP_PATH "./gcov-dump"

typedef struct {
    const char *name;
    char **argv;
    int argc;
    int expected_exit;
    const char *expected_stderr;
} test_case_t;

void create_test_source(void) {
    FILE *f = fopen(TEST_SRC, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

int compile_test_program(void) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: compile with coverage
        execlp("gcc", "gcc", "-O0", "--coverage", "-fprofile-arcs", 
               "-ftest-coverage", "-o", TEST_EXE, TEST_SRC, NULL);
        perror("exec gcc failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Failed to compile test program\n");
            return 0;
        }
        return 1;
    } else {
        perror("fork failed");
        return 0;
    }
}

int run_test(const test_case_t *test) {
    printf("Running: ");
    for (int i = 0; i < test->argc; i++) {
        printf("%s ", test->argv[i]);
    }
    printf("\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: run gcov-dump
        if (test->expected_stderr) {
            // Redirect stderr to a pipe to capture it
            int fd[2];
            pipe(fd);
            dup2(fd[1], STDERR_FILENO);
            close(fd[0]);
            close(fd[1]);
        }
        execvp(GCOV_DUMP_PATH, test->argv);
        perror("exec gcov-dump failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == test->expected_exit) {
                printf("  ✓ Exit code matches (expected %d, got %d)\n", 
                       test->expected_exit, exit_code);
                return 1;
            } else {
                printf("  ✗ Exit code mismatch (expected %d, got %d)\n", 
                       test->expected_exit, exit_code);
                return 0;
            }
        } else {
            printf("  ✗ Process did not exit normally\n");
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

int main(void) {
    printf("=== GCOV-DUMP Test Harness ===\n\n");
    
    // Step 1: Create test source file
    printf("1. Creating test source file '%s'...\n", TEST_SRC);
    create_test_source();
    
    // Step 2: Compile with coverage to generate .gcno file
    printf("2. Compiling with coverage to generate '%s'...\n", TEST_GCNO);
    if (!compile_test_program()) {
        fprintf(stderr, "Failed to generate .gcno file\n");
        return EXIT_FAILURE;
    }
    
    // Verify .gcno file exists
    struct stat st;
    if (stat(TEST_GCNO, &st) != 0) {
        fprintf(stderr, "File '%s' not found after compilation\n", TEST_GCNO);
        return EXIT_FAILURE;
    }
    printf("   Generated '%s' (%ld bytes)\n\n", TEST_GCNO, st.st_size);
    
    // Define test cases
    char *help_argv[] = {GCOV_DUMP_PATH, "-h", NULL};
    char *version_argv[] = {GCOV_DUMP_PATH, "-v", NULL};
    char *dump_contents_argv[] = {GCOV_DUMP_PATH, "-l", TEST_GCNO, NULL};
    char *dump_positions_argv[] = {GCOV_DUMP_PATH, "-p", TEST_GCNO, NULL};
    char *dump_raw_argv[] = {GCOV_DUMP_PATH, "-r", TEST_GCNO, NULL};
    char *dump_stable_argv[] = {GCOV_DUMP_PATH, "-s", TEST_GCNO, NULL};
    char *unknown_flag_argv[] = {GCOV_DUMP_PATH, "-x", NULL};
    
    test_case_t tests[] = {
        {"Help (-h)", help_argv, 2, 0, NULL},
        {"Version (-v)", version_argv, 2, 0, NULL},
        {"Dump contents (-l)", dump_contents_argv, 3, 0, NULL},
        {"Dump positions (-p)", dump_positions_argv, 3, 0, NULL},
        {"Dump raw (-r)", dump_raw_argv, 3, 0, NULL},
        {"Dump stable (-s)", dump_stable_argv, 3, 0, NULL},
        {"Unknown flag (-x)", unknown_flag_argv, 2, 1, "unknown flag `x'"}
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    
    // Step 3: Run all test cases
    printf("3. Running test cases:\n");
    for (int i = 0; i < num_tests; i++) {
        printf("\nTest %d/%d: %s\n", i + 1, num_tests, tests[i].name);
        if (run_test(&tests[i])) {
            passed++;
        }
    }
    
    // Step 4: Cleanup
    printf("\n4. Cleaning up...\n");
    unlink(TEST_SRC);
    unlink(TEST_EXE);
    unlink(TEST_GCNO);
    
    // Summary
    printf("\n=== Summary ===\n");
    printf("Passed: %d/%d tests\n", passed, num_tests);
    
    return (passed == num_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}
