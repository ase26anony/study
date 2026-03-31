/**
 * test_gcov_dump_flags.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by invoking
 * gcov-dump with invalid command-line flags.
 * 
 * Compile with: gcc -std=c99 -O0 -g -o test_gcov_dump test_gcov_dump_flags.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define MAX_OUTPUT_LEN 4096
#define DEFAULT_GCOV_DUMP_PATH "./gcc/gcov-dump"

/**
 * Find the gcov-dump executable path.
 * Checks environment variable GCov_DUMP first, then falls back
 * to common build locations.
 */
static const char *find_gcov_dump_path(void)
{
    const char *env_path = getenv("GCOV_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        return env_path;
    }
    
    /* Try common build locations */
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            return common_paths[i];
        }
    }
    
    return DEFAULT_GCOV_DUMP_PATH;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if "unknown flag" error is found in stderr, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char **argv, int argc)
{
    int pipefd[2];
    pid_t pid;
    char output[MAX_OUTPUT_LEN] = {0};
    int found_error = 0;
    
    /* Create pipe for capturing stderr */
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 0;
    }
    
    pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return 0;
    }
    
    if (pid == 0) {
        /* Child process */
        close(pipefd[0]);  /* Close read end */
        
        /* Redirect stderr to pipe */
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        /* Build argument array */
        char **exec_argv = malloc((argc + 2) * sizeof(char *));
        if (!exec_argv) {
            exit(EXIT_FAILURE);
        }
        
        exec_argv[0] = (char *)gcov_dump_path;
        for (int i = 0; i < argc; i++) {
            exec_argv[i + 1] = (char *)argv[i];
        }
        exec_argv[argc + 1] = NULL;
        
        execvp(gcov_dump_path, exec_argv);
        
        /* If we get here, exec failed */
        perror("execvp");
        free(exec_argv);
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        close(pipefd[1]);  /* Close write end */
        
        /* Read stderr output */
        ssize_t bytes_read = read(pipefd[0], output, MAX_OUTPUT_LEN - 1);
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
            
            /* Check for "unknown flag" error message */
            if (strstr(output, "unknown flag") != NULL) {
                found_error = 1;
                printf("Found target error message:\n%s\n", output);
            } else {
                printf("Output (no error found):\n%s\n", output);
            }
        }
        
        close(pipefd[0]);
        
        /* Wait for child */
        int status;
        waitpid(pid, &status, 0);
        
        /* Also check exit status (should be non-zero for invalid flag) */
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            printf("Process exited with non-zero status: %d\n", WEXITSTATUS(status));
        }
    }
    
    return found_error;
}

/**
 * Test various invalid flag scenarios.
 */
static void run_tests(const char *gcov_dump_path)
{
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    int total_tests = 0;
    int passed_tests = 0;
    
    /* Test 1: Single invalid flag as first argument */
    {
        printf("Test 1: Single invalid flag '-x'\n");
        const char *argv[] = {"-x"};
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, argv, 1)) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n");
        }
        printf("\n");
    }
    
    /* Test 2: Invalid flag between valid flags */
    {
        printf("Test 2: Invalid flag '-z' between valid flags '-l' and '-p'\n");
        const char *argv[] = {"-l", "-z", "-p"};
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, argv, 3)) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n");
        }
        printf("\n");
    }
    
    /* Test 3: Multiple invalid flags */
    {
        printf("Test 3: Multiple invalid flags '-? -y'\n");
        const char *argv[] = {"-?", "-y"};
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, argv, 2)) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n");
        }
        printf("\n");
    }
    
    /* Test 4: Invalid flag after non-option argument (filename) */
    {
        printf("Test 4: Invalid flag '-a' after filename 'test.gcda'\n");
        const char *argv[] = {"test.gcda", "-a"};
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, argv, 2)) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n");
        }
        printf("\n");
    }
    
    /* Test 5: Double dash with invalid single character flag */
    {
        printf("Test 5: Double dash with invalid flag '--x'\n");
        const char *argv[] = {"--x"};
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, argv, 1)) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n");
        }
        printf("\n");
    }
    
    /* Test 6: Combination of valid and invalid flags */
    {
        printf("Test 6: Combination '-l -x -p -r -s -w'\n");
        const char *argv[] = {"-l", "-x", "-p", "-r", "-s", "-w"};
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, argv, 6)) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n");
        }
        printf("\n");
    }
    
    /* Test 7: Boundary case - invalid flag at end */
    {
        printf("Test 7: Valid flags followed by invalid '-l -p -m'\n");
        const char *argv[] = {"-l", "-p", "-m"};
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, argv, 3)) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n");
        }
        printf("\n");
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", total_tests - passed_tests);
    
    if (passed_tests == total_tests) {
        printf("\n✓ All tests passed!\n");
    } else {
        printf("\n✗ Some tests failed\n");
    }
}

int main(void)
{
    const char *gcov_dump_path = find_gcov_dump_path();
    
    printf("Using gcov-dump path: %s\n", gcov_dump_path);
    
    /* Check if executable exists and is accessible */
    if (access(gcov_dump_path, X_OK) != 0) {
        fprintf(stderr, "Error: Cannot execute '%s': %s\n", 
                gcov_dump_path, strerror(errno));
        fprintf(stderr, "Try setting GCOV_DUMP environment variable.\n");
        return EXIT_FAILURE;
    }
    
    run_tests(gcov_dump_path);
    
    return EXIT_SUCCESS;
}
