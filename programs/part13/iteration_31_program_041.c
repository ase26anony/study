#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEST_SRC "test.c"
#define TEST_EXE "test"
#define TEST_GCNO "test.gcno"
#define GCOV_DUMP_PATH "./gcov-dump"

typedef struct {
    const char *name;
    const char *args[4];
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

void compile_with_coverage(void) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: compile with coverage
        execlp("gcc", "gcc", "-O0", "--coverage", "-fprofile-arcs", 
               "-ftest-coverage", "-o", TEST_EXE, TEST_SRC, NULL);
        perror("exec gcc failed");
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

int run_test(const char *name, const char *argv[], int expected_exit, 
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
        
        execvp(GCOV_DUMP_PATH, (char *const *)argv);
        perror("exec gcov-dump failed");
        exit(1);
    } else if (pid > 0) {
        // Parent process
        close(stderr_pipe[1]);  // Close write end
        
        int status;
        waitpid(pid, &status, 0);
        
        // Read stderr output
        char stderr_buf[1024] = {0};
        ssize_t bytes = read(stderr_pipe[0], stderr_buf, sizeof(stderr_buf) - 1);
        close(stderr_pipe[0]);
        
        int passed = 1;
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code != expected_exit) {
                printf("  FAIL: Expected exit code %d, got %d\n", 
                       expected_exit, exit_code);
                passed = 0;
            }
        } else {
            printf("  FAIL: Process didn't exit normally\n");
            passed = 0;
        }
        
        if (expected_stderr && bytes > 0) {
            // Check if expected string appears in stderr
            if (strstr(stderr_buf, expected_stderr) == NULL) {
                printf("  FAIL: Expected stderr to contain '%s', got: %s\n",
                       expected_stderr, stderr_buf);
                passed = 0;
            }
        }
        
        if (passed) {
            printf("  PASS\n");
        }
        
        return passed;
    } else {
        perror("fork failed");
        return 0;
    }
}

int main(void) {
    // Check if gcov-dump exists
    struct stat st;
    if (stat(GCOV_DUMP_PATH, &st) != 0) {
        fprintf(stderr, "gcov-dump not found at %s\n", GCOV_DUMP_PATH);
        fprintf(stderr, "Set GCOV_DUMP environment variable or adjust path\n");
        return 1;
    }
    
    // Create test source and compile with coverage
    printf("Creating test program with coverage...\n");
    create_test_source();
    compile_with_coverage();
    
    // Verify gcno file was created
    if (stat(TEST_GCNO, &st) != 0) {
        fprintf(stderr, "Failed to create %s\n", TEST_GCNO);
        return 1;
    }
    
    printf("\nStarting gcov-dump tests...\n\n");
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Test cases
    test_case_t tests[] = {
        // Test -h (help)
        {
            "Help flag (-h)",
            {GCOV_DUMP_PATH, "-h", NULL},
            0,
            NULL
        },
        
        // Test -v (version)
        {
            "Version flag (-v)",
            {GCOV_DUMP_PATH, "-v", NULL},
            0,
            NULL
        },
        
        // Test -l (dump contents)
        {
            "Dump contents flag (-l)",
            {GCOV_DUMP_PATH, "-l", TEST_GCNO, NULL},
            0,
            NULL
        },
        
        // Test -p (dump positions)
        {
            "Dump positions flag (-p)",
            {GCOV_DUMP_PATH, "-p", TEST_GCNO, NULL},
            0,
            NULL
        },
        
        // Test -r (dump raw)
        {
            "Dump raw flag (-r)",
            {GCOV_DUMP_PATH, "-r", TEST_GCNO, NULL},
            0,
            NULL
        },
        
        // Test -s (dump stable)
        {
            "Dump stable flag (-s)",
            {GCOV_DUMP_PATH, "-s", TEST_GCNO, NULL},
            0,
            NULL
        },
        
        // Test unknown flag
        {
            "Unknown flag (-x)",
            {GCOV_DUMP_PATH, "-x", NULL},
            1,  // Should exit with error
            "unknown flag `x'"
        },
        
        // Test another unknown flag
        {
            "Unknown flag (-Z)",
            {GCOV_DUMP_PATH, "-Z", NULL},
            1,
            "unknown flag `Z'"
        },
        
        // Test unknown flag with file argument
        {
            "Unknown flag with file (-? test.gcno)",
            {GCOV_DUMP_PATH, "-?", TEST_GCNO, NULL},
            1,
            "unknown flag `?'"
        }
    };
    
    // Run all tests
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
        total_tests++;
        passed_tests += run_test(tests[i].name, tests[i].args, 
                                tests[i].expected_exit, 
                                tests[i].expected_stderr);
        printf("\n");
    }
    
    // Cleanup
    unlink(TEST_SRC);
    unlink(TEST_EXE);
    unlink(TEST_GCNO);
    unlink("test.gcda");  // In case it was created
    
    printf("\nSummary: %d/%d tests passed\n", passed_tests, total_tests);
    
    return (passed_tests == total_tests) ? 0 : 1;
}
