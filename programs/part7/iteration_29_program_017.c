/**
 * test_gcov_dump_flags.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by invoking it with
 * invalid command-line flags and verifying the error message.
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
    "../gcc/gcov-dump",
    "../prev-gcc/build/gcc/gcov-dump",
    "/usr/bin/gcov-dump",
    "/usr/local/bin/gcov-dump",
    NULL  /* Sentinel */
};

/* Test cases with invalid flags */
static const char *TEST_CASES[][10] = {
    /* Single invalid flag tests */
    {"-x", NULL},
    {"-z", NULL},
    {"-?", NULL},
    {"-@", NULL},
    
    /* Invalid flag in different positions */
    {"-l", "-x", "-p", NULL},           /* Invalid in middle */
    {"-x", "-l", "-p", NULL},           /* Invalid at start */
    {"-l", "-p", "-x", NULL},           /* Invalid at end */
    
    /* Multiple invalid flags */
    {"-x", "-y", "-z", NULL},
    
    /* With filename argument */
    {"-x", "test.gcda", NULL},
    {"-l", "-x", "test.gcda", NULL},
    {"test.gcda", "-x", NULL},          /* Invalid after filename */
    
    /* Double dash cases */
    {"--", "-x", NULL},                 /* Should still trigger error */
    {"--", "--x", NULL},                /* getopt might handle differently */
    
    /* Edge cases */
    {"-", NULL},                        /* Just a dash */
    {"-lxpr", NULL},                    /* Combined flags with invalid 'x' */
    
    /* Sentinel */
    {NULL}
};

/**
 * Find the gcov-dump executable.
 * Returns dynamically allocated string or NULL if not found.
 */
static char *find_gcov_dump(void)
{
    char *path;
    int i;
    
    /* Check environment variable first */
    path = getenv("GCOV_DUMP");
    if (path && access(path, X_OK) == 0) {
        return strdup(path);
    }
    
    /* Try default paths */
    for (i = 0; DEFAULT_PATHS[i] != NULL; i++) {
        if (access(DEFAULT_PATHS[i], X_OK) == 0) {
            return strdup(DEFAULT_PATHS[i]);
        }
    }
    
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns dynamically allocated string with stderr output.
 * Returns NULL on execution failure.
 */
static char *run_gcov_dump(const char *gcov_dump_path, const char **args)
{
    int pipefd[2];
    pid_t pid;
    char *output = NULL;
    size_t output_size = 0;
    size_t output_len = 0;
    char buffer[4096];
    ssize_t n;
    int i, arg_count = 0;
    
    /* Count arguments */
    while (args[arg_count] != NULL) {
        arg_count++;
    }
    
    /* Create pipe for stderr */
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
        
        /* Build argument vector */
        char **argv = malloc((arg_count + 2) * sizeof(char *));
        if (!argv) {
            exit(EXIT_FAILURE);
        }
        
        argv[0] = (char *)gcov_dump_path;
        for (i = 0; i < arg_count; i++) {
            argv[i + 1] = (char *)args[i];
        }
        argv[arg_count + 1] = NULL;
        
        /* Execute gcov-dump */
        execvp(gcov_dump_path, argv);
        
        /* If we get here, exec failed */
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        free(argv);
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        close(pipefd[1]);  /* Close write end */
        
        /* Read stderr output */
        while ((n = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            /* Reallocate output buffer if needed */
            if (output_len + n + 1 > output_size) {
                size_t new_size = output_size == 0 ? 1024 : output_size * 2;
                char *new_output = realloc(output, new_size);
                if (!new_output) {
                    free(output);
                    close(pipefd[0]);
                    return NULL;
                }
                output = new_output;
                output_size = new_size;
            }
            
            /* Append to output */
            memcpy(output + output_len, buffer, n);
            output_len += n;
        }
        
        close(pipefd[0]);
        
        /* Wait for child */
        int status;
        waitpid(pid, &status, 0);
        
        /* Null-terminate the output */
        if (output) {
            output[output_len] = '\0';
        }
        
        return output;
    }
}

/**
 * Check if output contains the expected error message.
 */
static int contains_unknown_flag_error(const char *output)
{
    if (!output) {
        return 0;
    }
    
    /* Look for error message - case insensitive */
    const char *patterns[] = {
        "unknown flag",
        "unknown option",
        "invalid option",
        "unrecognized option",
        NULL
    };
    
    int i;
    for (i = 0; patterns[i] != NULL; i++) {
        if (strstr(output, patterns[i]) != NULL) {
            return 1;
        }
    }
    
    return 0;
}

/**
 * Run a single test case.
 * Returns 1 on success (error message found), 0 on failure.
 */
static int run_test_case(const char *gcov_dump_path, const char **args, int test_num)
{
    char *output;
    int success;
    
    printf("Test %d: Running gcov-dump with args: ", test_num);
    
    /* Print arguments */
    int i = 0;
    while (args[i] != NULL) {
        printf("%s ", args[i]);
        i++;
    }
    printf("\n");
    
    /* Run gcov-dump */
    output = run_gcov_dump(gcov_dump_path, args);
    
    if (!output) {
        printf("  FAILED: Could not execute gcov-dump or capture output\n");
        return 0;
    }
    
    /* Check for error message */
    success = contains_unknown_flag_error(output);
    
    if (success) {
        printf("  SUCCESS: Found 'unknown flag' error in output\n");
        /* Print first line of error for verification */
        char *newline = strchr(output, '\n');
        if (newline) {
            *newline = '\0';
        }
        printf("  Error: %s\n", output);
    } else {
        printf("  FAILED: No 'unknown flag' error found\n");
        if (strlen(output) > 0) {
            printf("  Output: %s\n", output);
        } else {
            printf("  (No output)\n");
        }
    }
    
    free(output);
    return success;
}

int main(void)
{
    char *gcov_dump_path;
    int test_num = 1;
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("=== Testing gcov-dump invalid flag handling ===\n\n");
    
    /* Find gcov-dump executable */
    gcov_dump_path = find_gcov_dump();
    if (!gcov_dump_path) {
        fprintf(stderr, "ERROR: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in a default location\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    /* Run all test cases */
    for (total_tests = 0; TEST_CASES[total_tests][0] != NULL; total_tests++) {
        if (run_test_case(gcov_dump_path, TEST_CASES[total_tests], test_num)) {
            passed_tests++;
        }
        test_num++;
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    
    free(gcov_dump_path);
    
    if (passed_tests > 0) {
        printf("\nSUCCESS: Triggered the uncovered default case in gcov-dump.cc\n");
        return EXIT_SUCCESS;
    } else {
        printf("\nFAILURE: Could not trigger the uncovered default case\n");
        return EXIT_FAILURE;
    }
}
