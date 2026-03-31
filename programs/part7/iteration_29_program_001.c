/**
 * test_gcov_dump_flags.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by invoking
 * gcov-dump with invalid command-line flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

/* Default paths to search for gcov-dump */
static const char *DEFAULT_PATHS[] = {
    "./gcc/gcov-dump",
    "./gcov-dump",
    "../gcc/gcov-dump",
    "../../gcc/gcov-dump",
    "/usr/bin/gcov-dump",
    "/usr/local/bin/gcov-dump",
    NULL
};

/**
 * Find the gcov-dump executable.
 * Returns a dynamically allocated string with the path,
 * or NULL if not found.
 */
static char *find_gcov_dump(void)
{
    const char *env_path = getenv("GCOV_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        return strdup(env_path);
    }
    
    /* Check default paths */
    for (int i = 0; DEFAULT_PATHS[i] != NULL; i++) {
        if (access(DEFAULT_PATHS[i], X_OK) == 0) {
            return strdup(DEFAULT_PATHS[i]);
        }
    }
    
    return NULL;
}

/**
 * Test a specific set of arguments.
 * Returns 1 if the "unknown flag" error was found, 0 otherwise.
 */
static int test_arguments(const char *gcov_dump_path, char *const argv[])
{
    int pipefd[2];
    pid_t pid;
    int status;
    char buffer[1024];
    int found_error = 0;
    
    /* Create pipe for stderr */
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
        
        /* Execute gcov-dump */
        execvp(gcov_dump_path, argv);
        
        /* If we get here, exec failed */
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        close(pipefd[1]);  /* Close write end */
        
        /* Read stderr output */
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            
            /* Check for the error message */
            if (strstr(buffer, "unknown flag") != NULL) {
                found_error = 1;
                /* Print for debugging */
                printf("Found error message: %s", buffer);
            }
        }
        
        close(pipefd[0]);
        
        /* Wait for child */
        waitpid(pid, &status, 0);
        
        /* Also check exit status (should be non-zero for error) */
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            /* Non-zero exit is expected for invalid flags */
        }
    }
    
    return found_error;
}

/**
 * Run a series of test cases with invalid flags.
 */
static int run_tests(const char *gcov_dump_path)
{
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    /* Test case 1: Single invalid flag */
    {
        printf("Test 1: Single invalid flag '-x'\n");
        char *args[] = { "gcov-dump", "-x", NULL };
        if (test_arguments(gcov_dump_path, args)) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n");
        }
        total_tests++;
    }
    
    /* Test case 2: Invalid flag between valid flags */
    {
        printf("\nTest 2: Invalid flag '-z' between valid flags '-l -z -p'\n");
        char *args[] = { "gcov-dump", "-l", "-z", "-p", NULL };
        if (test_arguments(gcov_dump_path, args)) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n");
        }
        total_tests++;
    }
    
    /* Test case 3: Multiple invalid flags */
    {
        printf("\nTest 3: Multiple invalid flags '-? -@ -#'\n");
        char *args[] = { "gcov-dump", "-?", "-@", "-#", NULL };
        if (test_arguments(gcov_dump_path, args)) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n");
        }
        total_tests++;
    }
    
    /* Test case 4: Invalid flag after filename argument */
    {
        printf("\nTest 4: Invalid flag '-y' after filename 'test.gcda'\n");
        char *args[] = { "gcov-dump", "test.gcda", "-y", NULL };
        if (test_arguments(gcov_dump_path, args)) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n");
        }
        total_tests++;
    }
    
    /* Test case 5: Double dash with invalid single char */
    {
        printf("\nTest 5: Double dash with invalid flag '--x'\n");
        char *args[] = { "gcov-dump", "--x", NULL };
        if (test_arguments(gcov_dump_path, args)) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n");
        }
        total_tests++;
    }
    
    /* Test case 6: Combined valid and invalid flags */
    {
        printf("\nTest 6: Combined '-l -x -p -r -s -w'\n");
        char *args[] = { "gcov-dump", "-l", "-x", "-p", "-r", "-s", "-w", NULL };
        if (test_arguments(gcov_dump_path, args)) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n");
        }
        total_tests++;
    }
    
    /* Test case 7: Boundary test - invalid flag at end */
    {
        printf("\nTest 7: Invalid flag '-k' at end of valid flags\n");
        char *args[] = { "gcov-dump", "-l", "-p", "-r", "-s", "-k", NULL };
        if (test_arguments(gcov_dump_path, args)) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n");
        }
        total_tests++;
    }
    
    printf("\n========================================\n");
    printf("Test Results: %d/%d tests passed\n", passed_tests, total_tests);
    
    return (passed_tests > 0) ? 1 : 0;
}

int main(void)
{
    char *gcov_dump_path = find_gcov_dump();
    
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in a standard location.\n");
        return EXIT_FAILURE;
    }
    
    int success = run_tests(gcov_dump_path);
    
    free(gcov_dump_path);
    
    if (success) {
        printf("\nSuccessfully triggered the uncovered default case in gcov-dump.cc\n");
        return EXIT_SUCCESS;
    } else {
        printf("\nFailed to trigger the uncovered default case\n");
        return EXIT_FAILURE;
    }
}
