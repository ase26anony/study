#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEST_SOURCE "test_gcov.c"
#define TEST_EXECUTABLE "test_gcov"
#define GCNO_FILE "test_gcov.gcno"
#define GCOV_DUMP_EXECUTABLE "./gcov-dump"

typedef struct {
    const char *name;
    const char *args[4];
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

int run_gcov_dump(const char *name, const char *args[], int *exit_code, 
                  char *stderr_buf, size_t stderr_size) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: run gcov-dump
        int dev_null = open("/dev/null", O_WRONLY);
        if (dev_null >= 0) {
            dup2(dev_null, STDOUT_FILENO);
            close(dev_null);
        }
        
        // Create pipe for stderr
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("pipe");
            exit(1);
        }
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        execvp(GCOV_DUMP_EXECUTABLE, (char *const *)args);
        perror("exec gcov-dump failed");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            *exit_code = WEXITSTATUS(status);
        } else {
            *exit_code = -1;
        }
        return 1;
    }
    return 0;
}

void run_test(const test_case *test) {
    printf("Running test: %s\n", test->name);
    printf("Command: %s", GCOV_DUMP_EXECUTABLE);
    for (int i = 0; test->args[i]; i++) {
        printf(" %s", test->args[i]);
    }
    printf("\n");
    
    int exit_code;
    char stderr_buf[1024] = {0};
    
    if (run_gcov_dump(test->name, test->args, &exit_code, stderr_buf, sizeof(stderr_buf))) {
        int passed = (exit_code == test->expected_exit);
        
        if (test->expected_stderr) {
            if (strstr(stderr_buf, test->expected_stderr) == NULL) {
                passed = 0;
            }
        }
        
        if (passed) {
            printf("  ✓ PASSED (exit code: %d)\n", exit_code);
        } else {
            printf("  ✗ FAILED\n");
            printf("    Expected exit code: %d, got: %d\n", test->expected_exit, exit_code);
            if (test->expected_stderr && stderr_buf[0]) {
                printf("    Stderr output: %s\n", stderr_buf);
            }
        }
    } else {
        printf("  ✗ FAILED to run test\n");
    }
    printf("\n");
}

int main() {
    printf("=== GCOV-DUMP Test Harness ===\n\n");
    
    // Step 1: Create test source and compile with coverage
    printf("1. Creating test source file...\n");
    create_test_source();
    
    printf("2. Compiling with coverage flags...\n");
    if (!compile_with_coverage()) {
        fprintf(stderr, "Failed to compile test program with coverage\n");
        return 1;
    }
    
    // Check if gcno file was created
    struct stat st;
    if (stat(GCNO_FILE, &st) != 0) {
        fprintf(stderr, "Failed to create %s file\n", GCNO_FILE);
        return 1;
    }
    printf("Generated %s successfully\n\n", GCNO_FILE);
    
    // Step 2: Define test cases
    test_case tests[] = {
        {
            .name = "Help flag (-h)",
            .args = {GCOV_DUMP_EXECUTABLE, "-h", NULL},
            .expected_exit = 0,
            .expected_stderr = NULL
        },
        {
            .name = "Version flag (-v)",
            .args = {GCOV_DUMP_EXECUTABLE, "-v", NULL},
            .expected_exit = 0,
            .expected_stderr = NULL
        },
        {
            .name = "Dump contents flag (-l)",
            .args = {GCOV_DUMP_EXECUTABLE, "-l", GCNO_FILE, NULL},
            .expected_exit = 0,
            .expected_stderr = NULL
        },
        {
            .name = "Dump positions flag (-p)",
            .args = {GCOV_DUMP_EXECUTABLE, "-p", GCNO_FILE, NULL},
            .expected_exit = 0,
            .expected_stderr = NULL
        },
        {
            .name = "Dump raw flag (-r)",
            .args = {GCOV_DUMP_EXECUTABLE, "-r", GCNO_FILE, NULL},
            .expected_exit = 0,
            .expected_stderr = NULL
        },
        {
            .name = "Dump stable flag (-s)",
            .args = {GCOV_DUMP_EXECUTABLE, "-s", GCNO_FILE, NULL},
            .expected_exit = 0,
            .expected_stderr = NULL
        },
        {
            .name = "Unknown flag (-x)",
            .args = {GCOV_DUMP_EXECUTABLE, "-x", NULL},
            .expected_exit = 1,
            .expected_stderr = "unknown flag"
        },
        {
            .name = "Unknown flag (-Z)",
            .args = {GCOV_DUMP_EXECUTABLE, "-Z", NULL},
            .expected_exit = 1,
            .expected_stderr = "unknown flag"
        },
        {
            .name = "Unknown flag (-?)",
            .args = {GCOV_DUMP_EXECUTABLE, "-?", NULL},
            .expected_exit = 1,
            .expected_stderr = "unknown flag"
        }
    };
    
    // Step 3: Run all tests
    printf("3. Running gcov-dump tests...\n\n");
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    
    for (int i = 0; i < num_tests; i++) {
        run_test(&tests[i]);
        passed++;
    }
    
    // Step 4: Cleanup
    printf("4. Cleaning up...\n");
    unlink(TEST_SOURCE);
    unlink(TEST_EXECUTABLE);
    unlink(GCNO_FILE);
    
    printf("\n=== Summary ===\n");
    printf("Total tests run: %d\n", num_tests);
    printf("All tests executed successfully\n");
    
    return 0;
}
