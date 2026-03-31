#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH_LEN 1024
#define MAX_OUTPUT_LEN 4096
#define ERROR_MSG_PREFIX "unknown flag"

/* Test cases for invalid flag injection */
typedef struct {
    const char *description;
    const char *args[10];  /* NULL-terminated array of arguments */
} test_case_t;

/* Array of test cases targeting different flag parsing scenarios */
static test_case_t test_cases[] = {
    /* Single invalid flag as first argument */
    {"Single invalid flag '-x'", {"-x", NULL}},
    {"Single invalid flag '-z'", {"-z", NULL}},
    {"Single invalid flag '-?'", {"-?", NULL}},
    
    /* Invalid flag between valid flags */
    {"Invalid flag '-x' between valid '-l' and '-p'", {"-l", "-x", "-p", NULL}},
    {"Invalid flag '-z' after valid '-r'", {"-r", "-z", NULL}},
    
    /* Multiple invalid flags */
    {"Multiple invalid flags '-x -z'", {"-x", "-z", NULL}},
    
    /* Invalid flag after non-option argument (filename) */
    {"Invalid flag '-x' after filename", {"test.gcda", "-x", NULL}},
    
    /* Double-dash with invalid single-character flag */
    {"Double-dash with invalid flag '--x'", {"--x", NULL}},
    
    /* Combination with help flag (should still trigger error for invalid flag) */
    {"Valid '-h' with invalid '-x'", {"-h", "-x", NULL}},
    
    /* Edge: dash alone */
    {"Single dash '-'", {"-", NULL}},
    
    /* End marker */
    {NULL, {NULL}}
};

/* Find gcov-dump executable path */
static char *find_gcov_dump(void) {
    char *path = getenv("GCOV_DUMP");
    if (path && access(path, X_OK) == 0) {
        return strdup(path);
    }
    
    /* Try common build locations */
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i]; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            return strdup(common_paths[i]);
        }
    }
    
    return NULL;
}

/* Execute gcov-dump with given arguments and capture stderr */
static int run_test(const char *gcov_dump_path, const char **args, 
                    char *output, size_t output_size) {
    int pipefd[2];
    pid_t pid;
    
    /* Create pipe for stderr */
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
        char *argv[20];
        int argc = 0;
        
        argv[argc++] = (char *)gcov_dump_path;
        
        for (int i = 0; args[i] && argc < 18; i++) {
            argv[argc++] = (char *)args[i];
        }
        argv[argc] = NULL;
        
        /* Execute gcov-dump */
        execvp(gcov_dump_path, argv);
        
        /* If we get here, exec failed */
        perror("execvp");
        exit(EXIT_FAILURE);
    } else { /* Parent process */
        int status;
        
        /* Close write end of pipe */
        close(pipefd[1]);
        
        /* Read stderr output */
        ssize_t bytes_read = read(pipefd[0], output, output_size - 1);
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
        } else {
            output[0] = '\0';
        }
        
        close(pipefd[0]);
        
        /* Wait for child */
        waitpid(pid, &status, 0);
        
        return WEXITSTATUS(status);
    }
}

/* Check if output contains the expected error message */
static int contains_unknown_flag_error(const char *output) {
    if (!output) return 0;
    
    /* Look for the error message prefix */
    return (strstr(output, ERROR_MSG_PREFIX) != NULL);
}

int main(void) {
    char *gcov_dump_path = find_gcov_dump();
    if (!gcov_dump_path) {
        fprintf(stderr, "Error: gcov-dump executable not found\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    int total_tests = 0;
    int passed_tests = 0;
    int failed_tests = 0;
    
    /* Run all test cases */
    for (int i = 0; test_cases[i].description; i++) {
        total_tests++;
        
        printf("Test %d: %s\n", total_tests, test_cases[i].description);
        printf("  Arguments: ");
        for (int j = 0; test_cases[i].args[j]; j++) {
            printf("%s ", test_cases[i].args[j]);
        }
        printf("\n");
        
        char output[MAX_OUTPUT_LEN];
        int exit_status = run_test(gcov_dump_path, test_cases[i].args, 
                                  output, sizeof(output));
        
        if (contains_unknown_flag_error(output)) {
            printf("  ✓ PASS: Got 'unknown flag' error\n");
            passed_tests++;
            
            /* Print first line of error for verification */
            char *first_line = strtok(output, "\n");
            if (first_line) {
                printf("    Error: %s\n", first_line);
            }
        } else {
            printf("  ✗ FAIL: No 'unknown flag' error detected\n");
            failed_tests++;
            
            if (output[0]) {
                printf("    Output: %s\n", output);
            } else {
                printf("    (No output captured)\n");
            }
        }
        
        printf("  Exit status: %d\n\n", exit_status);
    }
    
    /* Summary */
    printf("========================================\n");
    printf("Test Summary:\n");
    printf("  Total tests:  %d\n", total_tests);
    printf("  Passed:       %d\n", passed_tests);
    printf("  Failed:       %d\n", failed_tests);
    printf("  Success rate: %.1f%%\n", 
           total_tests > 0 ? (100.0 * passed_tests / total_tests) : 0.0);
    
    free(gcov_dump_path);
    
    return (failed_tests == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
