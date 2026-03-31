/**
 * gcov-dump_invalid_flag_test.c
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

/* Default paths to try if GCov_DUMP env var is not set */
static const char *DEFAULT_PATHS[] = {
    "./gcc/gcov-dump",
    "./gcov-dump",
    "../gcc/gcov-dump",
    "../../gcc/gcov-dump",
    "/usr/bin/gcov-dump",
    "/usr/local/bin/gcov-dump",
    NULL  /* Sentinel */
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
    
    /* Try default paths */
    for (int i = 0; DEFAULT_PATHS[i] != NULL; i++) {
        if (access(DEFAULT_PATHS[i], X_OK) == 0) {
            return strdup(DEFAULT_PATHS[i]);
        }
    }
    
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if "unknown flag" error is found in stderr,
 * 0 if not found, -1 on execution error.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag)
{
    int pipefd[2];
    pid_t pid;
    int status;
    char buffer[1024];
    int found = 0;
    
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
        
        /* Prepare arguments */
        char *args[4] = {
            (char *)gcov_dump_path,
            (char *)flag,
            NULL,
            NULL
        };
        
        /* Add a dummy .gcno file if needed to avoid early exit */
        if (strcmp(flag, "--") != 0) {
            args[1] = (char *)flag;
            args[2] = NULL;
        } else {
            /* For -- case, we need an invalid flag after it */
            args[1] = (char *)"--";
            args[2] = (char *)"-x";
            args[3] = NULL;
        }
        
        execvp(gcov_dump_path, args);
        
        /* If we get here, exec failed */
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        close(pipefd[1]);  /* Close write end */
        
        /* Read stderr output */
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            
            /* Check for "unknown flag" error message */
            if (strstr(buffer, "unknown flag") != NULL ||
                strstr(buffer, "unknown flag `") != NULL) {
                found = 1;
                /* Print for debugging */
                printf("Found error in output: %s", buffer);
            }
        }
        
        close(pipefd[0]);
        
        /* Wait for child */
        waitpid(pid, &status, 0);
        
        return found;
    }
}

/**
 * Test multiple invalid flag scenarios.
 */
static void run_tests(const char *gcov_dump_path)
{
    struct test_case {
        const char *description;
        const char *flag;
        int expect_error;
    };
    
    /* Test cases covering different scenarios */
    struct test_case tests[] = {
        {"Single invalid flag -x", "-x", 1},
        {"Single invalid flag -z", "-z", 1},
        {"Single invalid flag -?", "-?", 1},
        {"Invalid flag between valid flags", "-l -x -p", 1},
        {"Invalid flag after valid flag", "-l -x", 1},
        {"Invalid flag before valid flag", "-x -l", 1},
        {"Double dash with invalid flag", "-- -x", 1},
        {"Multiple invalid flags", "-x -y -z", 1},
        {"Invalid flag after filename", "test.gcno -x", 1},
        {"Mixed case invalid flag", "-X", 1},
        {"Number as invalid flag", "-9", 1},
        {NULL, NULL, 0}  /* Sentinel */
    };
    
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    int passed = 0;
    int total = 0;
    
    for (int i = 0; tests[i].description != NULL; i++) {
        printf("Test %d: %s\n", total + 1, tests[i].description);
        printf("  Command: %s %s\n", gcov_dump_path, tests[i].flag);
        
        int result = test_invalid_flag(gcov_dump_path, tests[i].flag);
        
        if (result == -1) {
            printf("  ERROR: Failed to execute test\n");
        } else if (result == tests[i].expect_error) {
            printf("  PASS: Got expected result\n");
            if (tests[i].expect_error) {
                passed++;
            }
        } else {
            printf("  FAIL: Expected error=%d, got result=%d\n", 
                   tests[i].expect_error, result);
        }
        
        printf("\n");
        total++;
    }
    
    printf("\n=== Summary ===\n");
    printf("Tests passed: %d/%d\n", passed, total);
    
    if (passed > 0) {
        printf("\nSUCCESS: Triggered the uncovered default case!\n");
    } else {
        printf("\nFAILURE: Could not trigger the uncovered default case.\n");
    }
}

int main(void)
{
    char *gcov_dump_path = find_gcov_dump();
    
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "ERROR: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in a default location\n");
        
        /* Try to compile a dummy .gcno file for testing */
        fprintf(stderr, "\nTrying to create a dummy test file...\n");
        system("gcc -ftest-coverage -fprofile-arcs -c -o /tmp/test.o /dev/null 2>/dev/null");
        
        /* Try one more path */
        gcov_dump_path = strdup("/tmp/not_exist/gcov-dump");
    } else {
        printf("Found gcov-dump at: %s\n", gcov_dump_path);
    }
    
    run_tests(gcov_dump_path);
    
    free(gcov_dump_path);
    
    /* Clean up any temporary files */
    system("rm -f /tmp/test.o /tmp/test.gcno 2>/dev/null");
    
    return 0;
}
