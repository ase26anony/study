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
    char **argv;
    int argc;
    int expected_exit;
    const char *expected_stderr;
} test_case;

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
        execlp("gcc", "gcc", "-O0", "--coverage", "-fprofile-arcs", 
               "-ftest-coverage", "-o", TEST_BINARY, TEST_SOURCE, NULL);
        perror("exec gcc failed");
        exit(1);
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
        perror("exec gcov-dump failed");
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
        int exit_ok = (exit_code == expected_exit);
        
        // Check stderr if expected
        int stderr_ok = 1;
        if (expected_stderr) {
            stderr_ok = (bytes_read > 0 && strstr(stderr_buf, expected_stderr) != NULL);
            if (!stderr_ok) {
                printf("  Expected stderr containing: '%s'\n", expected_stderr);
                printf("  Got: '%.*s'\n", (int)bytes_read, stderr_buf);
            }
        }
        
        int passed = exit_ok && stderr_ok;
        printf("  %s - exit: %d (expected %d), stderr: %s\n",
               passed ? "PASS" : "FAIL",
               exit_code, expected_exit,
               stderr_ok ? "OK" : "MISMATCH");
        
        return passed;
    } else {
        perror("fork failed");
        return 0;
    }
}

int main() {
    printf("=== GCOV-Dump Test Harness ===\n");
    
    // Check if gcov-dump exists
    if (access(GCOV_DUMP_PATH, X_OK) != 0) {
        fprintf(stderr, "gcov-dump not found at %s\n", GCOV_DUMP_PATH);
        fprintf(stderr, "Set GCOV_DUMP environment variable or copy gcov-dump to current directory\n");
        return 1;
    }
    
    // Create test source and compile with coverage
    printf("\nCreating test program with coverage...\n");
    create_test_source();
    if (!compile_with_coverage()) {
        fprintf(stderr, "Failed to create test.gcno file\n");
        return 1;
    }
    
    // Check if .gcno file was created
    if (access(TEST_GCNO, R_OK) != 0) {
        fprintf(stderr, "No %s file generated\n", TEST_GCNO);
        return 1;
    }
    
    printf("Test files created successfully\n\n");
    
    // Define test cases
    int total_tests = 0;
    int passed_tests = 0;
    
    // Test 1: -h (help)
    {
        char *args[] = {GCOV_DUMP_PATH, "-h", NULL};
        passed_tests += run_test("Help (-h)", args, 0, NULL);
        total_tests++;
    }
    
    // Test 2: -v (version)
    {
        char *args[] = {GCOV_DUMP_PATH, "-v", NULL};
        passed_tests += run_test("Version (-v)", args, 0, NULL);
        total_tests++;
    }
    
    // Test 3: -l (dump contents)
    {
        char *args[] = {GCOV_DUMP_PATH, "-l", TEST_GCNO, NULL};
        passed_tests += run_test("Dump contents (-l)", args, 0, NULL);
        total_tests++;
    }
    
    // Test 4: -p (dump positions)
    {
        char *args[] = {GCOV_DUMP_PATH, "-p", TEST_GCNO, NULL};
        passed_tests += run_test("Dump positions (-p)", args, 0, NULL);
        total_tests++;
    }
    
    // Test 5: -r (dump raw)
    {
        char *args[] = {GCOV_DUMP_PATH, "-r", TEST_GCNO, NULL};
        passed_tests += run_test("Dump raw (-r)", args, 0, NULL);
        total_tests++;
    }
    
    // Test 6: -s (dump stable)
    {
        char *args[] = {GCOV_DUMP_PATH, "-s", TEST_GCNO, NULL};
        passed_tests += run_test("Dump stable (-s)", args, 0, NULL);
        total_tests++;
    }
    
    // Test 7: -x (unknown flag)
    {
        char *args[] = {GCOV_DUMP_PATH, "-x", NULL};
        passed_tests += run_test("Unknown flag (-x)", args, 1, "unknown flag `x`");
        total_tests++;
    }
    
    // Test 8: -? (another unknown flag)
    {
        char *args[] = {GCOV_DUMP_PATH, "-?", NULL};
        passed_tests += run_test("Unknown flag (-?)", args, 1, "unknown flag `?`");
        total_tests++;
    }
    
    // Test 9: -Z (another unknown flag)
    {
        char *args[] = {GCOV_DUMP_PATH, "-Z", NULL};
        passed_tests += run_test("Unknown flag (-Z)", args, 1, "unknown flag `Z`");
        total_tests++;
    }
    
    // Cleanup
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    unlink(TEST_GCNO);
    
    printf("\n=== Summary ===\n");
    printf("Passed: %d/%d tests\n", passed_tests, total_tests);
    
    return (passed_tests == total_tests) ? 0 : 1;
}
