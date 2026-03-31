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

void create_test_source(void) {
    FILE *f = fopen(TEST_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

int compile_with_coverage(void) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: compile with coverage
        execlp("gcc", "gcc", "-O0", "--coverage", "-fprofile-arcs", 
               "-ftest-coverage", "-o", TEST_BINARY, TEST_SOURCE, NULL);
        perror("exec gcc failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            // Verify gcno file was created
            struct stat st;
            if (stat(TEST_GCNO, &st) == 0 && S_ISREG(st.st_mode)) {
                return 1;
            }
        }
    }
    return 0;
}

void run_test(const test_case_t *test) {
    printf("Running test: %s\n", test->name);
    
    // Capture stderr
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return;
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        close(pipefd[0]);  // Close read end
        dup2(pipefd[1], STDERR_FILENO);  // Redirect stderr to pipe
        close(pipefd[1]);
        
        execvp(GCOV_DUMP_PATH, test->argv);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        close(pipefd[1]);  // Close write end
        
        // Read stderr output
        char stderr_buf[1024] = {0};
        ssize_t bytes_read = read(pipefd[0], stderr_buf, sizeof(stderr_buf) - 1);
        close(pipefd[0]);
        
        int status;
        waitpid(pid, &status, 0);
        
        int exit_code = -1;
        if (WIFEXITED(status)) {
            exit_code = WEXITSTATUS(status);
        }
        
        // Check results
        int passed = 1;
        if (exit_code != test->expected_exit) {
            printf("  FAIL: Expected exit code %d, got %d\n", 
                   test->expected_exit, exit_code);
            passed = 0;
        }
        
        if (test->expected_stderr && bytes_read > 0) {
            if (strstr(stderr_buf, test->expected_stderr) == NULL) {
                printf("  FAIL: Expected stderr to contain '%s', got: %s\n",
                       test->expected_stderr, stderr_buf);
                passed = 0;
            }
        }
        
        if (passed) {
            printf("  PASS\n");
        }
    } else {
        perror("fork failed");
    }
}

int main(void) {
    printf("=== GCOV-DUMP Test Harness ===\n");
    
    // Create test source file
    create_test_source();
    printf("Created test source: %s\n", TEST_SOURCE);
    
    // Compile with coverage to generate .gcno file
    printf("Compiling with coverage...\n");
    if (!compile_with_coverage()) {
        fprintf(stderr, "Failed to compile with coverage or no .gcno file generated\n");
        return EXIT_FAILURE;
    }
    printf("Generated %s\n", TEST_GCNO);
    
    // Define test cases
    char *help_args[] = {GCOV_DUMP_PATH, "-h", NULL};
    char *version_args[] = {GCOV_DUMP_PATH, "-v", NULL};
    char *dump_contents_args[] = {GCOV_DUMP_PATH, "-l", TEST_GCNO, NULL};
    char *dump_positions_args[] = {GCOV_DUMP_PATH, "-p", TEST_GCNO, NULL};
    char *dump_raw_args[] = {GCOV_DUMP_PATH, "-r", TEST_GCNO, NULL};
    char *dump_stable_args[] = {GCOV_DUMP_PATH, "-s", TEST_GCNO, NULL};
    char *unknown_flag_args[] = {GCOV_DUMP_PATH, "-x", NULL};
    
    test_case_t tests[] = {
        {
            "Help flag (-h)",
            help_args,
            2,
            0,
            NULL
        },
        {
            "Version flag (-v)",
            version_args,
            2,
            0,
            NULL
        },
        {
            "Dump contents flag (-l)",
            dump_contents_args,
            3,
            0,
            NULL
        },
        {
            "Dump positions flag (-p)",
            dump_positions_args,
            3,
            0,
            NULL
        },
        {
            "Dump raw flag (-r)",
            dump_raw_args,
            3,
            0,
            NULL
        },
        {
            "Dump stable flag (-s)",
            dump_stable_args,
            3,
            0,
            NULL
        },
        {
            "Unknown flag (-x)",
            unknown_flag_args,
            2,
            1,  // Should exit with error
            "unknown flag `x`"
        }
    };
    
    // Run all tests
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < num_tests; i++) {
        run_test(&tests[i]);
        printf("\n");
    }
    
    // Cleanup
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    unlink(TEST_GCNO);
    unlink("test_gcov.gcda");  // In case it was created
    
    printf("=== Test Complete ===\n");
    return EXIT_SUCCESS;
}
