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
static const char *test_cases[] = {
    "-x",                    /* Single invalid flag */
    "-l -x -p",              /* Invalid flag between valid flags */
    "-x -l -p",              /* Invalid flag first */
    "-l -p -x",              /* Invalid flag last */
    "test.gcda -x",          /* Invalid flag after filename */
    "--x",                   /* Double dash with invalid flag */
    "-?",                    /* Question mark flag */
    "-z",                    /* Another invalid flag */
    "-l -x -p -z",           /* Multiple invalid flags */
    NULL
};

/* Find gcov-dump executable path */
static int find_gcov_dump(char *path, size_t path_len) {
    const char *env_path = getenv("GCOV_DUMP");
    
    if (env_path != NULL && strlen(env_path) > 0) {
        if (strlen(env_path) < path_len) {
            strcpy(path, env_path);
            return 1;
        }
    }
    
    /* Try common build locations */
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            if (strlen(common_paths[i]) < path_len) {
                strcpy(path, common_paths[i]);
                return 1;
            }
        }
    }
    
    return 0;
}

/* Execute gcov-dump with given arguments and capture stderr */
static int test_gcov_dump(const char *gcov_dump_path, const char *args, 
                         char *output, size_t output_len) {
    int pipefd[2];
    pid_t pid;
    int status;
    
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
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        /* Parse arguments */
        char *argv[32];
        int argc = 0;
        
        /* Copy executable path */
        argv[argc++] = strdup(gcov_dump_path);
        
        /* Tokenize arguments */
        char *args_copy = strdup(args);
        char *token = strtok(args_copy, " ");
        while (token != NULL && argc < 30) {
            argv[argc++] = strdup(token);
            token = strtok(NULL, " ");
        }
        argv[argc] = NULL;
        
        free(args_copy);
        
        /* Execute gcov-dump */
        execvp(gcov_dump_path, argv);
        
        /* If we get here, exec failed */
        perror("execvp");
        
        /* Clean up */
        for (int i = 0; i < argc; i++) {
            free(argv[i]);
        }
        
        exit(EXIT_FAILURE);
    } else { /* Parent process */
        /* Close write end of pipe */
        close(pipefd[1]);
        
        /* Read from pipe */
        ssize_t bytes_read = read(pipefd[0], output, output_len - 1);
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

int main(void) {
    char gcov_dump_path[MAX_PATH_LEN];
    char output[MAX_OUTPUT_LEN];
    int tests_passed = 0;
    int tests_run = 0;
    
    printf("=== Testing gcov-dump uncovered lines ===\n\n");
    
    /* Find gcov-dump executable */
    if (!find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path))) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in a common location\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    /* Run test cases */
    for (int i = 0; test_cases[i] != NULL; i++) {
        printf("Test %d: gcov-dump %s\n", i + 1, test_cases[i]);
        
        int exit_status = test_gcov_dump(gcov_dump_path, test_cases[i], 
                                        output, sizeof(output));
        
        printf("  Exit status: %d\n", exit_status);
        printf("  Output: %s", output[0] ? output : "(none)\n");
        
        /* Check if we got the target error message */
        if (strstr(output, TARGET_ERROR_MSG) != NULL) {
            printf("  ✓ Found target error message: \"%s\"\n", TARGET_ERROR_MSG);
            tests_passed++;
        } else {
            printf("  ✗ Did not find target error message\n");
        }
        
        tests_run++;
        printf("\n");
    }
    
    /* Summary */
    printf("=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    
    if (tests_passed > 0) {
        printf("\nSuccessfully triggered the uncovered default case!\n");
        return EXIT_SUCCESS;
    } else {
        printf("\nFailed to trigger the uncovered default case.\n");
        return EXIT_FAILURE;
    }
}
