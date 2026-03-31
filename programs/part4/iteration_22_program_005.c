/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Tests the uncovered default case in gcov-dump's option parsing.
 * Executes gcov-dump with invalid command-line flags and verifies
 * the "unknown flag" error message is printed to stderr.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

/* Maximum path length for executable */
#define MAX_PATH 1024
/* Maximum command line length */
#define MAX_CMD 4096
/* Buffer size for reading output */
#define BUF_SIZE 1024

/**
 * Get the path to gcov-dump executable.
 * Checks GCov_DUMP environment variable first, then common locations.
 * Returns 1 on success, 0 on failure.
 */
static int get_gcov_dump_path(char *path, size_t path_size)
{
    const char *env_path = getenv("GCOV_DUMP");
    const char *default_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    /* Try environment variable first */
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, path_size - 1);
        path[path_size - 1] = '\0';
        return 1;
    }
    
    /* Try default paths */
    for (int i = 0; default_paths[i] != NULL; i++) {
        if (access(default_paths[i], X_OK) == 0) {
            strncpy(path, default_paths[i], path_size - 1);
            path[path_size - 1] = '\0';
            return 1;
        }
    }
    
    return 0;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if "unknown flag" message found in stderr, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag)
{
    int pipefd[2];
    pid_t pid;
    char buffer[BUF_SIZE];
    int found = 0;
    int status;
    
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
        
        /* Execute gcov-dump with invalid flag */
        execl(gcov_dump_path, "gcov-dump", flag, NULL);
        
        /* If we get here, exec failed */
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        close(pipefd[1]);  /* Close write end */
        
        /* Read stderr output from pipe */
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], buffer, BUF_SIZE - 1)) > 0) {
            buffer[bytes_read] = '\0';
            
            /* Check for "unknown flag" message */
            if (strstr(buffer, "unknown flag") != NULL) {
                found = 1;
                /* Print the captured error for verification */
                printf("Found expected error: %s", buffer);
            }
        }
        
        close(pipefd[0]);
        
        /* Wait for child to finish */
        waitpid(pid, &status, 0);
        
        /* Check if child exited normally */
        if (!WIFEXITED(status)) {
            printf("Child process did not exit normally\n");
            return 0;
        }
    }
    
    return found;
}

/**
 * Test various invalid flag scenarios.
 */
static void run_tests(const char *gcov_dump_path)
{
    struct test_case {
        const char *description;
        const char *flag;
    };
    
    /* Test cases covering different scenarios */
    struct test_case tests[] = {
        {"Single invalid flag as first argument", "-x"},
        {"Another single invalid flag", "-z"},
        {"Invalid flag with question mark", "-?"},
        {"Invalid flag between valid flags", "-l -x -p"},
        {"Invalid flag after valid flag", "-v -x"},
        {"Multiple invalid flags", "-x -y -z"},
        {"Invalid flag after double dash", "-- -x"},
        {"Mixed case invalid flag", "-X"},
        {"Invalid numeric flag", "-9"},
        {"Invalid flag with special char", "-@"},
        {NULL, NULL}
    };
    
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    int total_tests = 0;
    int passed_tests = 0;
    
    for (int i = 0; tests[i].description != NULL; i++) {
        printf("Test %d: %s\n", total_tests + 1, tests[i].description);
        printf("  Command: gcov-dump %s\n", tests[i].flag);
        
        if (test_invalid_flag(gcov_dump_path, tests[i].flag)) {
            printf("  Result: PASSED (unknown flag error detected)\n");
            passed_tests++;
        } else {
            printf("  Result: FAILED (unknown flag error not found)\n");
        }
        
        printf("\n");
        total_tests++;
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", total_tests - passed_tests);
    
    if (passed_tests == total_tests) {
        printf("\nAll tests passed! The uncovered default case was triggered.\n");
    } else {
        printf("\nSome tests failed. The uncovered default case may not have been triggered.\n");
    }
}

int main(void)
{
    char gcov_dump_path[MAX_PATH];
    
    printf("=== Testing gcov-dump Invalid Flag Handling ===\n\n");
    
    /* Find gcov-dump executable */
    if (!get_gcov_dump_path(gcov_dump_path, sizeof(gcov_dump_path))) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in a standard location\n");
        return EXIT_FAILURE;
    }
    
    printf("Found gcov-dump at: %s\n", gcov_dump_path);
    
    /* Verify executable exists and is executable */
    if (access(gcov_dump_path, X_OK) != 0) {
        fprintf(stderr, "Error: %s is not executable\n", gcov_dump_path);
        return EXIT_FAILURE;
    }
    
    /* Run the tests */
    run_tests(gcov_dump_path);
    
    return EXIT_SUCCESS;
}
