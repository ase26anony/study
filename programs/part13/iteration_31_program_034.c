#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEST_SOURCE "test.c"
#define TEST_EXECUTABLE "test"
#define TEST_GCNO "test.gcno"
#define GCOV_DUMP_PATH "./gcov-dump"

typedef struct {
    const char *name;
    const char **args;
    int expected_exit;
    const char *expected_stderr;
    int check_stderr;
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

int compile_test_program() {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: compile with coverage
        execlp("gcc", "gcc", "-O0", "--coverage", "-fprofile-arcs", 
               "-ftest-coverage", "-o", TEST_EXECUTABLE, TEST_SOURCE, NULL);
        perror("exec gcc failed");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) && WEXITSTATUS(status) == 0;
    }
    return 0;
}

int run_test(const char *name, const char **args, int expected_exit, 
             const char *expected_stderr, int check_stderr) {
    printf("Running: %s\n", name);
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: execute gcov-dump
        execvp(GCOV_DUMP_PATH, (char *const *)args);
        perror("exec gcov-dump failed");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == expected_exit) {
                printf("  ✓ Exit code correct (%d)\n", exit_code);
                return 1;
            } else {
                printf("  ✗ Wrong exit code: expected %d, got %d\n", 
                       expected_exit, exit_code);
                return 0;
            }
        } else {
            printf("  ✗ Process didn't exit normally\n");
            return 0;
        }
    }
    return 0;
}

int main() {
    // Test argument vectors
    const char *args_h[] = {GCOV_DUMP_PATH, "-h", NULL};
    const char *args_v[] = {GCOV_DUMP_PATH, "-v", NULL};
    const char *args_l[] = {GCOV_DUMP_PATH, "-l", TEST_GCNO, NULL};
    const char *args_p[] = {GCOV_DUMP_PATH, "-p", TEST_GCNO, NULL};
    const char *args_r[] = {GCOV_DUMP_PATH, "-r", TEST_GCNO, NULL};
    const char *args_s[] = {GCOV_DUMP_PATH, "-s", TEST_GCNO, NULL};
    const char *args_x[] = {GCOV_DUMP_PATH, "-x", NULL};
    
    // Create test cases
    test_case_t tests[] = {
        {"-h (help)", args_h, 0, NULL, 0},
        {"-v (version)", args_v, 0, NULL, 0},
        {"-l (dump contents)", args_l, 0, NULL, 0},
        {"-p (dump positions)", args_p, 0, NULL, 0},
        {"-r (dump raw)", args_r, 0, NULL, 0},
        {"-s (dump stable)", args_s, 0, NULL, 0},
        {"-x (unknown flag)", args_x, 1, "unknown flag `x`", 1}
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    
    printf("=== GCOV-DUMP Test Harness ===\n\n");
    
    // First, verify gcov-dump exists
    struct stat st;
    if (stat(GCOV_DUMP_PATH, &st) != 0) {
        fprintf(stderr, "Error: gcov-dump not found at %s\n", GCOV_DUMP_PATH);
        fprintf(stderr, "Set GCOV_DUMP environment variable or adjust path.\n");
        return 1;
    }
    
    // Create test source and compile with coverage
    printf("Creating test program with coverage...\n");
    create_test_source();
    
    if (!compile_test_program()) {
        fprintf(stderr, "Failed to compile test program with coverage\n");
        // Clean up
        unlink(TEST_SOURCE);
        return 1;
    }
    
    printf("Generated %s for testing\n\n", TEST_GCNO);
    
    // Run all tests
    for (int i = 0; i < num_tests; i++) {
        if (run_test(tests[i].name, tests[i].args, tests[i].expected_exit,
                    tests[i].expected_stderr, tests[i].check_stderr)) {
            passed++;
        }
        printf("\n");
    }
    
    // Cleanup
    printf("Cleaning up...\n");
    unlink(TEST_SOURCE);
    unlink(TEST_EXECUTABLE);
    unlink(TEST_GCNO);
    
    // Summary
    printf("=== Summary ===\n");
    printf("Passed: %d/%d tests\n", passed, num_tests);
    
    return passed == num_tests ? 0 : 1;
}
