/**
 * test_gcov_dump_default_case.c
 * 
 * Tests the uncovered default case in gcov-dump.cc that handles
 * unknown command-line flags. This program executes gcov-dump with
 * various invalid flags and verifies the error message is printed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

/* Default path to gcov-dump if GCov_DUMP environment variable is not set */
#define DEFAULT_GCOV_DUMP_PATH "./gcc/gcov-dump"

/* Target error message to look for in stderr */
#define TARGET_ERROR_SUBSTRING "unknown flag"

/**
 * Find the gcov-dump executable path.
 * Priority: 1. GCov_DUMP environment variable
 *           2. Hard-coded default path
 * Returns: Dynamically allocated string with the path, or NULL if not found.
 */
static char *find_gcov_dump_path(void)
{
    char *path = getenv("GCov_DUMP");
    
    if (path != NULL && access(path, X_OK) == 0) {
        return strdup(path);
    }
    
    /* Try the default path */
    if (access(DEFAULT_GCOV_DUMP_PATH, X_OK) == 0) {
        return strdup(DEFAULT_GCOV_DUMP_PATH);
    }
    
    /* Try some other common locations in a GCC build tree */
    const char *common_paths[] = {
        "../prev-gcc/build/gcc/gcov-dump",
        "../gcc-build/gcc/gcov-dump",
        "gcov-dump",  /* In PATH */
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            return strdup(common_paths[i]);
        }
    }
    
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns: 1 if target error message found in stderr, 0 if not found,
 *          -1 on execution error.
 */
static int test_gcov_dump_with_args(const char *gcov_dump_path, 
                                    const char *const args[])
{
    int pipefd[2];
    pid_t pid;
    int status;
    char buffer[1024];
    int found_target = 0;
    
    /* Create pipe for capturing stderr */
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }
    
    pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        close(pipefd[0]);  /* Close read end */
        
        /* Redirect stderr to pipe */
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        /* Execute gcov-dump */
        execv(gcov_dump_path, (char *const *)args);
        
        /* If we get here, execv failed */
        fprintf(stderr, "Failed to execute %s: %s\n", 
                gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        close(pipefd[1]);  /* Close write end */
        
        /* Read stderr output from pipe */
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            
            /* Check for target error message */
            if (strstr(buffer, TARGET_ERROR_SUBSTRING) != NULL) {
                found_target = 1;
            }
            
            /* Optional: print captured output for debugging */
            /* fwrite(buffer, 1, bytes_read, stderr); */
        }
        
        close(pipefd[0]);
        
        /* Wait for child to finish */
        waitpid(pid, &status, 0);
        
        /* Check if child exited normally with non-zero status 
         * (expected for invalid flag) */
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            /* This is expected for invalid flags */
        }
        
        return found_target;
    }
}

/**
 * Run a series of test cases with different invalid flag combinations.
 */
static int run_test_cases(const char *gcov_dump_path)
{
    int passed_tests = 0;
    int total_tests = 0;
    
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    /* Test case 1: Single invalid flag as first argument */
    {
        const char *args[] = { gcov_dump_path, "-x", NULL };
        printf("Test 1: Single invalid flag '-x'... ");
        fflush(stdout);
        
        int result = test_gcov_dump_with_args(gcov_dump_path, args);
        if (result == 1) {
            printf("PASSED (found error message)\n");
            passed_tests++;
        } else if (result == 0) {
            printf("FAILED (no error message)\n");
        } else {
            printf("ERROR (execution failed)\n");
        }
        total_tests++;
    }
    
    /* Test case 2: Invalid flag between valid flags */
    {
        const char *args[] = { gcov_dump_path, "-l", "-x", "-p", NULL };
        printf("Test 2: Invalid flag '-x' between '-l' and '-p'... ");
        fflush(stdout);
        
        int result = test_gcov_dump_with_args(gcov_dump_path, args);
        if (result == 1) {
            printf("PASSED (found error message)\n");
            passed_tests++;
        } else if (result == 0) {
            printf("FAILED (no error message)\n");
        } else {
            printf("ERROR (execution failed)\n");
        }
        total_tests++;
    }
    
    /* Test case 3: Multiple invalid flags */
    {
        const char *args[] = { gcov_dump_path, "-z", "-?", "-@", NULL };
        printf("Test 3: Multiple invalid flags '-z', '-?', '-@'... ");
        fflush(stdout);
        
        int result = test_gcov_dump_with_args(gcov_dump_path, args);
        if (result == 1) {
            printf("PASSED (found error message)\n");
            passed_tests++;
        } else if (result == 0) {
            printf("FAILED (no error message)\n");
        } else {
            printf("ERROR (execution failed)\n");
        }
        total_tests++;
    }
    
    /* Test case 4: Invalid flag after filename argument */
    {
        const char *args[] = { gcov_dump_path, "test.gcda", "-y", NULL };
        printf("Test 4: Invalid flag '-y' after filename... ");
        fflush(stdout);
        
        int result = test_gcov_dump_with_args(gcov_dump_path, args);
        if (result == 1) {
            printf("PASSED (found error message)\n");
            passed_tests++;
        } else if (result == 0) {
            printf("FAILED (no error message)\n");
        } else {
            printf("ERROR (execution failed)\n");
        }
        total_tests++;
    }
    
    /* Test case 5: Double dash with invalid flag (edge case) */
    {
        const char *args[] = { gcov_dump_path, "--", "-x", NULL };
        printf("Test 5: Double dash '--' followed by '-x'... ");
        fflush(stdout);
        
        int result = test_gcov_dump_with_args(gcov_dump_path, args);
        /* Note: getopt might stop at '--', so this may not trigger the error */
        if (result == 1) {
            printf("PASSED (found error message)\n");
            passed_tests++;
        } else if (result == 0) {
            printf("SKIPPED (no error - expected with '--')\n");
        } else {
            printf("ERROR (execution failed)\n");
        }
        total_tests++;
    }
    
    /* Test case 6: Combined valid and invalid flags */
    {
        const char *args[] = { gcov_dump_path, "-l", "-x", "-p", "-r", "-s", NULL };
        printf("Test 6: Mixed valid and invalid flags... ");
        fflush(stdout);
        
        int result = test_gcov_dump_with_args(gcov_dump_path, args);
        if (result == 1) {
            printf("PASSED (found error message)\n");
            passed_tests++;
        } else if (result == 0) {
            printf("FAILED (no error message)\n");
        } else {
            printf("ERROR (execution failed)\n");
        }
        total_tests++;
    }
    
    printf("\nTest Summary: %d/%d tests passed\n", passed_tests, total_tests);
    
    return (passed_tests > 0) ? 0 : 1;
}

int main(void)
{
    char *gcov_dump_path = find_gcov_dump_path();
    
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure the executable exists.\n");
        fprintf(stderr, "Tried: %s and other common locations.\n", DEFAULT_GCOV_DUMP_PATH);
        return EXIT_FAILURE;
    }
    
    printf("Found gcov-dump: %s\n", gcov_dump_path);
    
    int result = run_test_cases(gcov_dump_path);
    
    free(gcov_dump_path);
    
    return result;
}
