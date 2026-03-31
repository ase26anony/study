#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#define TEST_SOURCE "test_gcov.c"
#define TEST_EXECUTABLE "test_gcov"
#define GCNO_FILE "test_gcov.gcno"
#define GCOV_DUMP_EXEC "./gcov-dump"

typedef struct {
    const char *name;
    const char *args[4];
    int expected_exit;
    const char *expected_stderr;
} test_case;

void create_test_source(void) {
    FILE *f = fopen(TEST_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(1);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

int compile_test_program(void) {
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

int run_gcov_dump(const char *args[], int *exit_code, char *stderr_buf, size_t buf_size) {
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
            perror("pipe failed");
            exit(1);
        }
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        execvp(GCOV_DUMP_EXEC, (char *const *)args);
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
        
        // Read stderr from pipe (simplified - in real test you'd want to capture it)
        return 1;
    } else {
        perror("fork failed");
        return 0;
    }
}

int main(void) {
    printf("=== gcov-dump Test Harness ===\n");
    
    // Check if gcov-dump exists
    struct stat st;
    if (stat(GCOV_DUMP_EXEC, &st) != 0) {
        fprintf(stderr, "Error: %s not found\n", GCOV_DUMP_EXEC);
        fprintf(stderr, "Set GCOV_DUMP environment variable or place executable in current directory\n");
        return 1;
    }
    
    // Create test source file
    create_test_source();
    
    // Compile test program to generate .gcno file
    printf("Compiling test program to generate .gcno file...\n");
    if (!compile_test_program()) {
        fprintf(stderr, "Failed to compile test program\n");
        return 1;
    }
    
    // Check if .gcno file was created
    if (stat(GCNO_FILE, &st) != 0) {
        fprintf(stderr, "Error: %s not generated\n", GCNO_FILE);
        return 1;
    }
    
    printf(".gcno file created successfully\n\n");
    
    // Define test cases
    test_case tests[] = {
        // Test case 1: -h (help)
        {"help", {GCOV_DUMP_EXEC, "-h", NULL}, 0, NULL},
        
        // Test case 2: -v (version)
        {"version", {GCOV_DUMP_EXEC, "-v", NULL}, 0, NULL},
        
        // Test case 3: -l (dump contents)
        {"dump contents", {GCOV_DUMP_EXEC, "-l", GCNO_FILE, NULL}, 0, NULL},
        
        // Test case 4: -p (dump positions)
        {"dump positions", {GCOV_DUMP_EXEC, "-p", GCNO_FILE, NULL}, 0, NULL},
        
        // Test case 5: -r (dump raw)
        {"dump raw", {GCOV_DUMP_EXEC, "-r", GCNO_FILE, NULL}, 0, NULL},
        
        // Test case 6: -s (dump stable)
        {"dump stable", {GCOV_DUMP_EXEC, "-s", GCNO_FILE, NULL}, 0, NULL},
        
        // Test case 7: -x (unknown flag)
        {"unknown flag", {GCOV_DUMP_EXEC, "-x", NULL}, 1, "unknown flag `x'"}
    };
    
    int total_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    
    // Run all test cases
    for (int i = 0; i < total_tests; i++) {
        printf("Test %d/%d: %s... ", i + 1, total_tests, tests[i].name);
        
        int exit_code;
        char stderr_buf[256] = {0};
        
        if (run_gcov_dump(tests[i].args, &exit_code, stderr_buf, sizeof(stderr_buf))) {
            if (exit_code == tests[i].expected_exit) {
                printf("PASSED\n");
                passed++;
            } else {
                printf("FAILED (expected exit code %d, got %d)\n", 
                       tests[i].expected_exit, exit_code);
            }
        } else {
            printf("FAILED (execution error)\n");
        }
        
        fflush(stdout);
    }
    
    // Cleanup
    unlink(TEST_SOURCE);
    unlink(TEST_EXECUTABLE);
    unlink(GCNO_FILE);
    
    // Summary
    printf("\n=== Summary ===\n");
    printf("Passed: %d/%d\n", passed, total_tests);
    
    return (passed == total_tests) ? 0 : 1;
}
