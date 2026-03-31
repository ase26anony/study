/**
 * test_gcov_dump_invalid_flags.c
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

#define MAX_PATH_LEN 1024
#define MAX_OUTPUT_LEN 4096
#define TARGET_ERROR_MSG "unknown flag"

/* Test cases for invalid flag injection */
typedef struct {
    const char *description;
    const char *args[8];  /* NULL-terminated array of arguments */
} test_case_t;

static const test_case_t test_cases[] = {
    /* Single invalid flags */
    {"Single invalid flag -x", {"-x", NULL}},
    {"Single invalid flag -z", {"-z", NULL}},
    {"Single invalid flag -?", {"-?", NULL}},
    {"Single invalid flag -@", {"-@", NULL}},
    
    /* Invalid flag combinations */
    {"Valid flag followed by invalid -l -x", {"-l", "-x", NULL}},
    {"Invalid flag between valid ones -l -x -p", {"-l", "-x", "-p", NULL}},
    {"Multiple invalid flags -x -y -z", {"-x", "-y", "-z", NULL}},
    
    /* Invalid flag after filename argument */
    {"Invalid flag after filename test.gcda -x", {"test.gcda", "-x", NULL}},
    
    /* Double dash with invalid flag (getopt behavior test) */
    {"Double dash with invalid flag -- -x", {"--", "-x", NULL}},
    
    /* Mixed case: valid, filename, invalid */
    {"Mixed: -l test.gcda -x", {"-l", "test.gcda", "-x", NULL}},
    
    /* Edge: just a dash (might be interpreted as stdin) */
    {"Single dash argument -", {"-", NULL}},
    
    /* End marker */
    {NULL, {NULL}}
};

/**
 * Find gcov-dump executable path.
 * Checks GCov_DUMP environment variable first, then common locations.
 */
static int find_gcov_dump(char *path, size_t path_len) {
    const char *env_path = getenv("GCOV_DUMP");
    const char *candidate_paths[] = {
        "gcov-dump",
        "./gcov-dump",
        "./gcc/gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    /* Try environment variable first */
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, path_len - 1);
        path[path_len - 1] = '\0';
        return 0;
    }
    
    /* Try candidate paths */
    for (int i = 0; candidate_paths[i] != NULL; i++) {
        if (access(candidate_paths[i], X_OK) == 0) {
            strncpy(path, candidate_paths[i], path_len - 1);
            path[path_len - 1] = '\0';
            return 0;
        }
    }
    
    return -1; /* Not found */
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 0 if target error message found, -1 on execution error,
 * 1 if executed but target message not found.
 */
static int run_test(const char *gcov_dump_path, const char **args, 
                    char *output, size_t output_len) {
    pid_t pid;
    int pipefd[2];
    int status;
    ssize_t bytes_read;
    
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
    
    if (pid == 0) { /* Child process */
        /* Close read end of pipe */
        close(pipefd[0]);
        
        /* Redirect stderr to pipe */
        if (dup2(pipefd[1], STDERR_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(pipefd[1]);
        
        /* Build argument vector */
        char *argv[32];
        int argc = 0;
        
        argv[argc++] = (char *)gcov_dump_path;
        
        for (int i = 0; args[i] != NULL && argc < 30; i++) {
            argv[argc++] = (char *)args[i];
        }
        argv[argc] = NULL;
        
        /* Execute gcov-dump */
        execvp(gcov_dump_path, argv);
        
        /* If we get here, exec failed */
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else { /* Parent process */
        /* Close write end of pipe */
        close(pipefd[1]);
        
        /* Read stderr output */
        bytes_read = read(pipefd[0], output, output_len - 1);
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
        } else {
            output[0] = '\0';
        }
        close(pipefd[0]);
        
        /* Wait for child */
        waitpid(pid, &status, 0);
        
        /* Check if target error message is in output */
        if (strstr(output, TARGET_ERROR_MSG) != NULL) {
            return 0; /* Success - found target message */
        }
        
        /* Check if child exited normally with non-zero status
           (gcov-dump typically exits with error for invalid flags) */
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            return 1; /* Executed but no target message */
        }
        
        return -1; /* Execution failed */
    }
}

/**
 * Print colored output for test results.
 */
static void print_result(const char *description, int result, 
                         const char *output) {
    const char *color_green = "\033[32m";
    const char *color_red = "\033[31m";
    const char *color_yellow = "\033[33m";
    const char *color_reset = "\033[0m";
    
    printf("%-40s: ", description);
    
    switch (result) {
        case 0:
            printf("%sPASS%s (found '%s')", color_green, color_reset, 
                   TARGET_ERROR_MSG);
            break;
        case 1:
            printf("%sFAIL%s (no '%s' in output)", color_red, color_reset, 
                   TARGET_ERROR_MSG);
            break;
        default:
            printf("%sERROR%s (execution failed)", color_yellow, color_reset);
            break;
    }
    
    if (output[0] != '\0' && result != 0) {
        printf("\n  Output: %.200s%s", output, 
               strlen(output) > 200 ? "..." : "");
    }
    printf("\n");
}

int main(void) {
    char gcov_dump_path[MAX_PATH_LEN];
    char output[MAX_OUTPUT_LEN];
    int total_tests = 0;
    int passed_tests = 0;
    int failed_tests = 0;
    int error_tests = 0;
    
    printf("=== Testing gcov-dump invalid flag handling ===\n\n");
    
    /* Find gcov-dump executable */
    if (find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path)) != 0) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure ");
        fprintf(stderr, "gcov-dump is in PATH.\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    /* Run all test cases */
    for (int i = 0; test_cases[i].description != NULL; i++) {
        int result;
        
        total_tests++;
        
        /* Run the test */
        result = run_test(gcov_dump_path, test_cases[i].args, 
                         output, sizeof(output));
        
        /* Print result */
        print_result(test_cases[i].description, result, output);
        
        /* Update counters */
        switch (result) {
            case 0: passed_tests++; break;
            case 1: failed_tests++; break;
            default: error_tests++; break;
        }
        
        /* Small delay to avoid overwhelming the system */
        usleep(10000);
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Total tests : %d\n", total_tests);
    printf("Passed      : %d\n", passed_tests);
    printf("Failed      : %d\n", failed_tests);
    printf("Errors      : %d\n", error_tests);
    
    /* Also test that valid flags don't trigger the error */
    printf("\n=== Sanity Check: Valid flag should not trigger error ===\n");
    {
        const char *valid_args[] = {"-h", NULL};  /* Help flag should work */
        int result = run_test(gcov_dump_path, valid_args, output, sizeof(output));
        
        if (result == 0) {
            printf("FAIL: Valid flag -h triggered 'unknown flag' error!\n");
            printf("Output: %s\n", output);
        } else if (strstr(output, "Usage:") != NULL || 
                   strstr(output, "usage:") != NULL) {
            printf("PASS: Valid flag -h shows usage (as expected)\n");
        } else {
            printf("UNEXPECTED: Valid flag -h produced unexpected output\n");
            printf("Output: %s\n", output);
        }
    }
    
    return (passed_tests > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
