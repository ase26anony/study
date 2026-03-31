#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEST_SOURCE "test_gcov.c"
#define TEST_BINARY "test_gcov"
#define TEST_GCNO "test_gcov.gcno"
#define GCOV_DUMP_PATH "./gcov-dump"

typedef struct {
    const char *name;
    char **argv;
    int argc;
    int expected_exit;
    const char *expected_stderr;
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
        // Child process: compile with coverage
        char *argv[] = {
            "gcc",
            "-O0",
            "--coverage",
            "-fprofile-arcs",
            "-ftest-coverage",
            TEST_SOURCE,
            "-o",
            TEST_BINARY,
            NULL
        };
        execvp("gcc", argv);
        perror("execvp failed");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) && WEXITSTATUS(status) == 0;
    }
    return 0;
}

int run_test(const char *name, char *const argv[], int expected_exit, 
             const char *expected_stderr) {
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
        
        int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
        
        // Check results
        int passed = 1;
        if (exit_code != expected_exit) {
            printf("  FAIL: Expected exit code %d, got %d\n", expected_exit, exit_code);
            passed = 0;
        }
        
        if (expected_stderr && bytes_read > 0) {
            // Check if expected string appears in stderr
            if (strstr(stderr_buf, expected_stderr) == NULL) {
                printf("  FAIL: Expected stderr to contain '%s', got: %s\n", 
                       expected_stderr, stderr_buf);
                passed = 0;
            }
        } else if (expected_stderr && bytes_read <= 0) {
            printf("  FAIL: Expected stderr output but got none\n");
            passed = 0;
        }
        
        if (passed) {
            printf("  PASS\n");
        }
        
        return passed;
    }
    
    return 0;
}

int main() {
    printf("=== GCOV-DUMP Test Harness ===\n");
    
    // Create test source file
    create_test_source();
    
    // Compile with coverage to generate .gcno file
    printf("Compiling test program with coverage...\n");
    if (!compile_with_coverage()) {
        fprintf(stderr, "Failed to compile test program\n");
        return 1;
    }
    
    // Verify .gcno file exists
    struct stat st;
    if (stat(TEST_GCNO, &st) != 0) {
        fprintf(stderr, "Failed to find %s\n", TEST_GCNO);
        return 1;
    }
    printf("Generated %s successfully\n\n", TEST_GCNO);
    
    // Define test cases
    int total_tests = 0;
    int passed_tests = 0;
    
    // Test 1: -h (help)
    {
        char *argv[] = {GCOV_DUMP_PATH, "-h", NULL};
        passed_tests += run_test("Test -h (help)", argv, 0, NULL);
        total_tests++;
    }
    
    // Test 2: -v (version)
    {
        char *argv[] = {GCOV_DUMP_PATH, "-v", NULL};
        passed_tests += run_test("Test -v (version)", argv, 0, NULL);
        total_tests++;
    }
    
    // Test 3: -l (dump contents)
    {
        char *argv[] = {GCOV_DUMP_PATH, "-l", TEST_GCNO, NULL};
        passed_tests += run_test("Test -l (dump contents)", argv, 0, NULL);
        total_tests++;
    }
    
    // Test 4: -p (dump positions)
    {
        char *argv[] = {GCOV_DUMP_PATH, "-p", TEST_GCNO, NULL};
        passed_tests += run_test("Test -p (dump positions)", argv, 0, NULL);
        total_tests++;
    }
    
    // Test 5: -r (dump raw)
    {
        char *argv[] = {GCOV_DUMP_PATH, "-r", TEST_GCNO, NULL};
        passed_tests += run_test("Test -r (dump raw)", argv, 0, NULL);
        total_tests++;
    }
    
    // Test 6: -s (dump stable)
    {
        char *argv[] = {GCOV_DUMP_PATH, "-s", TEST_GCNO, NULL};
        passed_tests += run_test("Test -s (dump stable)", argv, 0, NULL);
        total_tests++;
    }
    
    // Test 7: -x (unknown flag)
    {
        char *argv[] = {GCOV_DUMP_PATH, "-x", NULL};
        passed_tests += run_test("Test -x (unknown flag)", argv, 1, "unknown flag `x'");
        total_tests++;
    }
    
    // Test 8: -? (another unknown flag)
    {
        char *argv[] = {GCOV_DUMP_PATH, "-?", NULL};
        passed_tests += run_test("Test -? (unknown flag)", argv, 1, "unknown flag `?'");
        total_tests++;
    }
    
    // Test 9: -Z (another unknown flag)
    {
        char *argv[] = {GCOV_DUMP_PATH, "-Z", NULL};
        passed_tests += run_test("Test -Z (unknown flag)", argv, 1, "unknown flag `Z'");
        total_tests++;
    }
    
    // Test 10: Combined flags
    {
        char *argv[] = {GCOV_DUMP_PATH, "-l", "-p", TEST_GCNO, NULL};
        passed_tests += run_test("Test -l -p (combined flags)", argv, 0, NULL);
        total_tests++;
    }
    
    // Test 11: No arguments (should fail)
    {
        char *argv[] = {GCOV_DUMP_PATH, NULL};
        passed_tests += run_test("Test no arguments", argv, 1, NULL);
        total_tests++;
    }
    
    // Cleanup
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    // Keep TEST_GCNO for inspection if needed
    
    printf("\n=== Summary ===\n");
    printf("Passed: %d/%d tests\n", passed_tests, total_tests);
    
    return passed_tests == total_tests ? 0 : 1;
}
