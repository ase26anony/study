/**
 * test_gcov_dump_default_case.c
 * 
 * Tests the uncovered default case in gcov-dump.cc that handles
 * unknown command-line flags. Executes gcov-dump with various
 * invalid flags and verifies the error message is printed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

/* Default paths to try if GCov_DUMP environment variable is not set */
static const char *DEFAULT_PATHS[] = {
    "./gcc/gcov-dump",
    "./gcov-dump",
    "../prev-gcc/build/gcc/gcov-dump",
    "../../gcc/gcov-dump",
    "/usr/bin/gcov-dump",
    "/usr/local/bin/gcov-dump",
    NULL  /* Sentinel */
};

/**
 * Find the gcov-dump executable.
 * Returns a dynamically allocated string with the path, or NULL if not found.
 */
static char *find_gcov_dump(void)
{
    char *path = NULL;
    
    /* First check environment variable */
    path = getenv("GCOV_DUMP");
    if (path != NULL && access(path, X_OK) == 0) {
        return strdup(path);
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
 * Returns 1 if "unknown flag" message is found in stderr, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag)
{
    int found = 0;
    int pipefd[2];
    pid_t pid;
    
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
        
        /* Execute gcov-dump with the invalid flag */
        execl(gcov_dump_path, "gcov-dump", flag, NULL);
        
        /* If we get here, exec failed */
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        char buffer[1024];
        ssize_t nbytes;
        
        close(pipefd[1]);  /* Close write end */
        
        /* Read from pipe (child's stderr) */
        while ((nbytes = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[nbytes] = '\0';
            
            /* Check for the error message */
            if (strstr(buffer, "unknown flag") != NULL) {
                found = 1;
                /* Print for debugging */
                printf("Found error message for flag %s:\n  %s", flag, buffer);
            }
        }
        
        close(pipefd[0]);
        
        /* Wait for child to avoid zombie */
        waitpid(pid, NULL, 0);
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
        {"Single invalid flag", "-x"},
        {"Another invalid flag", "-z"},
        {"Question mark flag", "-?"},
        {"Invalid flag after valid flag", "-l -x"},  /* Combined in one string */
        {"Invalid flag between valid flags", "-p -z -r"},
        {"Multiple invalid flags", "-a -b -c"},
        {"Invalid flag with double dash", "--x"},    /* getopt may treat differently */
        {"Uppercase invalid flag", "-X"},
        {"Number flag", "-1"},
        {"Special character flag", "-@"},
        {NULL, NULL}
    };
    
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    int total_tests = 0;
    int passed_tests = 0;
    
    for (int i = 0; tests[i].description != NULL; i++) {
        total_tests++;
        
        printf("Test %d: %s (flag: %s)\n", 
               total_tests, tests[i].description, tests[i].flag);
        
        if (test_invalid_flag(gcov_dump_path, tests[i].flag)) {
            printf("  ✓ PASSED\n\n");
            passed_tests++;
        } else {
            printf("  ✗ FAILED - 'unknown flag' message not found\n\n");
        }
    }
    
    /* Additional test: Use system() for simpler combined flag test */
    printf("Additional test with system() call:\n");
    total_tests++;
    
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "%s -l -x -p 2>&1 | grep -q 'unknown flag'", 
             gcov_dump_path);
    
    int result = system(cmd);
    if (WIFEXITED(result) && WEXITSTATUS(result) == 0) {
        printf("  ✓ PASSED (combined flags -l -x -p)\n");
        passed_tests++;
    } else {
        printf("  ✗ FAILED (combined flags test)\n");
    }
    
    printf("\n========================================\n");
    printf("Test Results: %d/%d tests passed\n", passed_tests, total_tests);
    
    if (passed_tests > 0) {
        printf("\nSUCCESS: Uncovered default case was triggered!\n");
    } else {
        printf("\nFAILURE: Could not trigger the uncovered default case\n");
    }
}

int main(void)
{
    char *gcov_dump_path = find_gcov_dump();
    
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in a default path.\n");
        fprintf(stderr, "Tried paths:\n");
        for (int i = 0; DEFAULT_PATHS[i] != NULL; i++) {
            fprintf(stderr, "  %s\n", DEFAULT_PATHS[i]);
        }
        return EXIT_FAILURE;
    }
    
    printf("Found gcov-dump at: %s\n", gcov_dump_path);
    
    run_tests(gcov_dump_path);
    
    free(gcov_dump_path);
    return EXIT_SUCCESS;
}
