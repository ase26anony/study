#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
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
    const char *expected_stderr;
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

void compile_with_coverage(void) {
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
    
    pid_t pid = fork();
    if (pid == 0) {
        execvp("gcc", argv);
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

int run_test(const char *name, char *const argv[], int expected_exit, 
             const char *expected_stderr) {
    printf("Running: %s\n", name);
    
    int pipefd[2];
    if (expected_stderr && pipe(pipefd) == -1) {
        perror("pipe failed");
        return 0;
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        if (expected_stderr) {
            close(pipefd[0]);
            dup2(pipefd[1], STDERR_FILENO);
            close(pipefd[1]);
        }
        execvp(GCOV_DUMP_PATH, argv);
        perror("Failed to execute gcov-dump");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        
        if (expected_stderr) {
            close(pipefd[1]);
            char buffer[1024];
            ssize_t n = read(pipefd[0], buffer, sizeof(buffer) - 1);
            close(pipefd[0]);
            if (n > 0) {
                buffer[n] = '\0';
                if (strstr(buffer, expected_stderr) == NULL) {
                    printf("  FAIL: Expected stderr containing '%s', got: %s\n", 
                           expected_stderr, buffer);
                    return 0;
                }
            }
        }
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == expected_exit) {
                printf("  PASS: Exit code %d\n", exit_code);
                return 1;
            } else {
                printf("  FAIL: Expected exit code %d, got %d\n", 
                       expected_exit, exit_code);
                return 0;
            }
        } else {
            printf("  FAIL: Process did not exit normally\n");
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

int main(void) {
    printf("=== GCOV-Dump Test Harness ===\n\n");
    
    // Create test source and compile with coverage
    printf("Creating test source file...\n");
    create_test_source();
    
    printf("Compiling with coverage flags...\n");
    compile_with_coverage();
    
    // Verify gcno file exists
    if (access(TEST_GCNO, F_OK) != 0) {
        fprintf(stderr, "Coverage file %s not found\n", TEST_GCNO);
        exit(EXIT_FAILURE);
    }
    
    // Test cases
    int passed = 0;
    int total = 0;
    
    // Test 1: -h flag (help)
    {
        char *argv[] = {GCOV_DUMP_PATH, "-h", NULL};
        passed += run_test("Test -h (help)", argv, 0, NULL);
        total++;
    }
    
    // Test 2: -v flag (version)
    {
        char *argv[] = {GCOV_DUMP_PATH, "-v", NULL};
        passed += run_test("Test -v (version)", argv, 0, NULL);
        total++;
    }
    
    // Test 3: -l flag (dump contents)
    {
        char *argv[] = {GCOV_DUMP_PATH, "-l", TEST_GCNO, NULL};
        passed += run_test("Test -l (dump contents)", argv, 0, NULL);
        total++;
    }
    
    // Test 4: -p flag (dump positions)
    {
        char *argv[] = {GCOV_DUMP_PATH, "-p", TEST_GCNO, NULL};
        passed += run_test("Test -p (dump positions)", argv, 0, NULL);
        total++;
    }
    
    // Test 5: -r flag (dump raw)
    {
        char *argv[] = {GCOV_DUMP_PATH, "-r", TEST_GCNO, NULL};
        passed += run_test("Test -r (dump raw)", argv, 0, NULL);
        total++;
    }
    
    // Test 6: -s flag (dump stable)
    {
        char *argv[] = {GCOV_DUMP_PATH, "-s", TEST_GCNO, NULL};
        passed += run_test("Test -s (dump stable)", argv, 0, NULL);
        total++;
    }
    
    // Test 7: Invalid flag (should trigger default case)
    {
        char *argv[] = {GCOV_DUMP_PATH, "-x", NULL};
        passed += run_test("Test invalid flag -x", argv, 1, "unknown flag `x`");
        total++;
    }
    
    // Test 8: Another invalid flag
    {
        char *argv[] = {GCOV_DUMP_PATH, "-Z", NULL};
        passed += run_test("Test invalid flag -Z", argv, 1, "unknown flag `Z`");
        total++;
    }
    
    // Test 9: Invalid flag with question mark
    {
        char *argv[] = {GCOV_DUMP_PATH, "-?", NULL};
        passed += run_test("Test invalid flag -?", argv, 1, "unknown flag `?`");
        total++;
    }
    
    // Cleanup
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    unlink(TEST_GCNO);
    
    printf("\n=== Summary ===\n");
    printf("Passed: %d/%d\n", passed, total);
    
    return (passed == total) ? EXIT_SUCCESS : EXIT_FAILURE;
}
