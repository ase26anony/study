/**
 * test_gcov_dump_default_case.c
 * 
 * Tests the uncovered default case in gcov-dump's option parsing.
 * Executes gcov-dump with various invalid flags and verifies the
 * "unknown flag" error message is printed to stderr.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

/* Default path to gcov-dump if GCov_DUMP env var is not set */
#define DEFAULT_GCOV_DUMP_PATH "./gcc/gcov-dump"

/* Target error message to look for */
#define TARGET_ERROR "unknown flag"

/**
 * Find the gcov-dump executable path.
 * Returns: dynamically allocated string with the path (caller must free),
 *          or NULL if not found.
 */
static char *find_gcov_dump(void)
{
    char *path = getenv("GCov_DUMP");
    
    if (path != NULL && access(path, X_OK) == 0) {
        return strdup(path);
    }
    
    /* Try default path */
    if (access(DEFAULT_GCOV_DUMP_PATH, X_OK) == 0) {
        return strdup(DEFAULT_GCOV_DUMP_PATH);
    }
    
    /* Try some other common locations in a GCC build tree */
    const char *common_paths[] = {
        "../prev-gcc/build/gcc/gcov-dump",
        "../gcc-build/gcc/gcov-dump",
        "gcov-dump",  /* Maybe it's in PATH */
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
 * Returns: dynamically allocated string with stderr output (caller must free),
 *          or NULL on execution failure.
 */
static char *run_gcov_dump(const char *gcov_dump_path, const char *arg)
{
    int pipefd[2];
    pid_t pid;
    char *output = NULL;
    size_t output_size = 0;
    size_t output_len = 0;
    
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return NULL;
    }
    
    pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return NULL;
    }
    
    if (pid == 0) {
        /* Child process */
        close(pipefd[0]);  /* Close read end */
        
        /* Redirect stderr to pipe */
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        /* Prepare arguments */
        const char *argv[4];
        int argc = 0;
        argv[argc++] = gcov_dump_path;
        
        if (arg != NULL) {
            argv[argc++] = arg;
        }
        
        argv[argc] = NULL;
        
        execvp(gcov_dump_path, (char *const *)argv);
        
        /* execvp only returns on error */
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    /* Parent process */
    close(pipefd[1]);  /* Close write end */
    
    /* Read stderr output from pipe */
    char buffer[4096];
    ssize_t bytes_read;
    
    while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
        /* Resize output buffer if needed */
        if (output_len + bytes_read + 1 > output_size) {
            size_t new_size = output_size == 0 ? 1024 : output_size * 2;
            char *new_output = realloc(output, new_size);
            if (new_output == NULL) {
                free(output);
                close(pipefd[0]);
                return NULL;
            }
            output = new_output;
            output_size = new_size;
        }
        
        /* Append to output */
        memcpy(output + output_len, buffer, bytes_read);
        output_len += bytes_read;
    }
    
    close(pipefd[0]);
    
    /* Wait for child */
    int status;
    waitpid(pid, &status, 0);
    
    if (output != NULL) {
        output[output_len] = '\0';
    }
    
    return output;
}

/**
 * Test a specific invalid flag.
 * Returns: 1 if test passed (error message found), 0 if failed.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag, const char *description)
{
    printf("Testing %s: %s\n", description, flag);
    
    char *output = run_gcov_dump(gcov_dump_path, flag);
    if (output == NULL) {
        printf("  FAILED: Could not execute gcov-dump\n");
        return 0;
    }
    
    int found = (strstr(output, TARGET_ERROR) != NULL);
    
    if (found) {
        printf("  PASSED: Found target error message\n");
        if (strlen(output) > 0) {
            printf("  Output: %s", output);
        }
    } else {
        printf("  FAILED: Target error message not found\n");
        if (strlen(output) > 0) {
            printf("  Output: %s", output);
        } else {
            printf("  (No output)\n");
        }
    }
    
    free(output);
    return found;
}

int main(void)
{
    printf("=== Testing gcov-dump default case coverage ===\n");
    
    /* Find gcov-dump executable */
    char *gcov_dump_path = find_gcov_dump();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "ERROR: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or place executable in:\n");
        fprintf(stderr, "  - ./gcc/gcov-dump\n");
        fprintf(stderr, "  - ../prev-gcc/build/gcc/gcov-dump\n");
        fprintf(stderr, "  - Or ensure 'gcov-dump' is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    /* Test cases designed to trigger the default case */
    struct test_case {
        const char *flag;
        const char *description;
    } test_cases[] = {
        /* Single invalid flags */
        {"-x", "Single invalid flag 'x'"},
        {"-z", "Single invalid flag 'z'"},
        {"-?", "Invalid flag '?'"},
        {"-X", "Invalid uppercase flag 'X'"},
        
        /* Invalid flag in combination with valid flags */
        {"-l", "Valid flag 'l' (should not trigger default case)"},
        {"-l -x", "Valid flag followed by invalid flag"},
        {"-x -l", "Invalid flag followed by valid flag"},
        {"-l -x -p", "Multiple flags with invalid in middle"},
        {"-x -z -?", "Multiple invalid flags"},
        
        /* Edge cases for getopt parsing */
        {"-x -y -z", "Multiple consecutive invalid flags"},
        {"-lp", "Combined valid flags (should work)"},
        {"-lx", "Combined flags with invalid at end"},
        {"-xl", "Combined flags with invalid at start"},
        
        /* Double dash handling */
        {"--x", "Double dash with single char (may be treated as filename)"},
        {"--", "Double dash alone"},
        {"-- -x", "Double dash followed by invalid flag"},
        
        /* With filename argument */
        {"-x test.gcda", "Invalid flag with filename"},
        {"test.gcda -x", "Filename before invalid flag"},
        {"-l -x test.gcda", "Mixed flags with filename"},
        
        /* NULL terminator for array */
        {NULL, NULL}
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    
    /* Run test cases */
    for (int i = 0; test_cases[i].flag != NULL; i++) {
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, test_cases[i].flag, test_cases[i].description)) {
            passed_tests++;
        }
        printf("\n");
    }
    
    /* Special test: no arguments (should not trigger default case) */
    printf("Testing: No arguments (should not trigger default case)\n");
    char *output = run_gcov_dump(gcov_dump_path, NULL);
    if (output != NULL) {
        int found = (strstr(output, TARGET_ERROR) != NULL);
        if (!found) {
            printf("  PASSED: No unexpected error (as expected)\n");
            passed_tests++;
        } else {
            printf("  FAILED: Unexpected error for no arguments\n");
        }
        free(output);
    }
    total_tests++;
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    
    free(gcov_dump_path);
    
    return (passed_tests == total_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}
