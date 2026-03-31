#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

#define MAX_PATH_LEN 1024
#define MAX_OUTPUT_LEN 4096
#define TARGET_ERROR_MSG "unknown flag"

/* Test cases for invalid flags */
const char *test_cases[][10] = {
    /* Single invalid flag in different positions */
    {"-x", NULL},
    {"-z", NULL},
    {"-?", NULL},
    /* Invalid flag between valid flags */
    {"-l", "-x", "-p", NULL},
    {"-p", "-z", "-r", NULL},
    /* Invalid flag after non-option argument */
    {"dummy.gcda", "-x", NULL},
    /* Double dash with invalid flag */
    {"--", "-x", NULL},
    {"--", "--x", NULL},
    /* Multiple invalid flags */
    {"-x", "-y", "-z", NULL},
    /* Mixed valid and invalid */
    {"-v", "-x", "-l", "-z", NULL}
};

int execute_gcov_dump(const char *gcov_dump_path, const char **args, char *output, size_t output_size) {
    int pipefd[2];
    pid_t pid;
    
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
        close(pipefd[0]); /* Close read end */
        
        /* Redirect stderr to pipe */
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        /* Build argument list */
        char *argv[64];
        int argc = 0;
        
        argv[argc++] = (char *)gcov_dump_path;
        
        for (int i = 0; args[i] != NULL && argc < 62; i++) {
            argv[argc++] = (char *)args[i];
        }
        argv[argc] = NULL;
        
        execvp(gcov_dump_path, argv);
        
        /* If we get here, exec failed */
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else { /* Parent process */
        int status;
        close(pipefd[1]); /* Close write end */
        
        /* Read stderr output from pipe */
        ssize_t bytes_read = read(pipefd[0], output, output_size - 1);
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
        } else {
            output[0] = '\0';
        }
        
        close(pipefd[0]);
        
        /* Wait for child to finish */
        waitpid(pid, &status, 0);
        
        return WEXITSTATUS(status);
    }
}

const char *find_gcov_dump() {
    const char *env_path = getenv("GCOV_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        return env_path;
    }
    
    /* Try common paths where gcov-dump might be found */
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            return common_paths[i];
        }
    }
    
    return NULL;
}

int main() {
    const char *gcov_dump_path = find_gcov_dump();
    
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in a common location.\n");
        return EXIT_FAILURE;
    }
    
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    int total_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int passed_tests = 0;
    int failed_tests = 0;
    
    for (int i = 0; i < total_tests; i++) {
        printf("Test %d: ", i + 1);
        
        /* Print test case */
        for (int j = 0; test_cases[i][j] != NULL; j++) {
            printf("%s ", test_cases[i][j]);
        }
        printf("\n");
        
        char output[MAX_OUTPUT_LEN];
        int exit_status = execute_gcov_dump(gcov_dump_path, test_cases[i], output, sizeof(output));
        
        /* Check if we got the expected error message */
        if (strstr(output, TARGET_ERROR_MSG) != NULL) {
            printf("  ✓ PASS: Found '%s' in output\n", TARGET_ERROR_MSG);
            printf("  Output: %s", output);
            passed_tests++;
        } else {
            printf("  ✗ FAIL: Did not find '%s' in output\n", TARGET_ERROR_MSG);
            if (strlen(output) > 0) {
                printf("  Output: %s", output);
            } else {
                printf("  Output: (empty)\n");
            }
            failed_tests++;
        }
        
        printf("  Exit status: %d\n\n", exit_status);
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", failed_tests);
    
    if (failed_tests > 0) {
        printf("\nSome tests failed. This might be because:\n");
        printf("1. gcov-dump handles invalid flags differently than expected\n");
        printf("2. The executable is a different version\n");
        printf("3. Some test cases might trigger different code paths\n");
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
