/**
 * Test program to trigger uncovered lines in gcov-dump.cc
 * Specifically targets the default case in the flag parsing switch statement
 * Compile with: gcc -std=c99 -O0 -g -o test_gcov_dump test_gcov_dump.c
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

/**
 * Find the gcov-dump executable path.
 * Checks GCov_DUMP environment variable first, then common build locations.
 */
static int find_gcov_dump(char *path, size_t path_len) {
    const char *env_path = getenv("GCOV_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, path_len - 1);
        path[path_len - 1] = '\0';
        return 0;
    }

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
            strncpy(path, common_paths[i], path_len - 1);
            path[path_len - 1] = '\0';
            return 0;
        }
    }

    return -1;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 0 if target error message found, -1 on execution error, -2 if message not found.
 */
static int test_gcov_dump(const char *gcov_dump_path, char **argv, int argc) {
    int pipefd[2];
    pid_t pid;
    char output[MAX_OUTPUT_LEN] = {0};
    int status;
    ssize_t bytes_read;

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
        // Child process
        close(pipefd[0]);  // Close read end
        
        // Redirect stderr to pipe
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        // Build argument array
        char **exec_args = malloc((argc + 2) * sizeof(char *));
        if (!exec_args) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        exec_args[0] = (char *)gcov_dump_path;
        for (int i = 0; i < argc; i++) {
            exec_args[i + 1] = argv[i];
        }
        exec_args[argc + 1] = NULL;

        execvp(gcov_dump_path, exec_args);
        
        // If we get here, exec failed
        perror("execvp");
        free(exec_args);
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        close(pipefd[1]);  // Close write end
        
        // Read stderr output
        bytes_read = read(pipefd[0], output, sizeof(output) - 1);
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
        }
        close(pipefd[0]);

        // Wait for child
        waitpid(pid, &status, 0);

        // Check if target error message is in output
        if (strstr(output, TARGET_ERROR_MSG) != NULL) {
            printf("Found target error message in output:\n%s\n", output);
            return 0;
        } else if (bytes_read > 0) {
            printf("Output (no target message):\n%s\n", output);
            return -2;
        } else if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            // Program exited with error but no stderr captured
            printf("Program exited with status %d but no error message captured\n", 
                   WEXITSTATUS(status));
            return -2;
        }
    }

    return -2;
}

/**
 * Test case structure
 */
typedef struct {
    char *description;
    char **argv;
    int argc;
} test_case_t;

int main(void) {
    char gcov_dump_path[MAX_PATH_LEN];
    
    // Find gcov-dump executable
    if (find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path)) != 0) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);

    // Define test cases
    char *test1_args[] = {"-x", NULL};
    char *test2_args[] = {"-z", NULL};
    char *test3_args[] = {"-?", NULL};
    char *test4_args[] = {"-l", "-x", "-p", NULL};
    char *test5_args[] = {"-x", "-l", "-p", NULL};
    char *test6_args[] = {"-l", "-p", "-x", NULL};
    char *test7_args[] = {"testfile.gcda", "-x", NULL};
    char *test8_args[] = {"-l", "testfile.gcda", "-x", NULL};
    char *test9_args[] = {"--", "-x", NULL};
    char *test10_args[] = {"--x", NULL};
    char *test11_args[] = {"-l", "--", "-x", NULL};
    char *test12_args[] = {"-a", "-b", "-c", NULL};  // Multiple invalid flags

    test_case_t test_cases[] = {
        {"Single invalid flag -x", test1_args, 1},
        {"Single invalid flag -z", test2_args, 1},
        {"Single invalid flag -?", test3_args, 1},
        {"Valid flag -l, invalid -x, valid -p", test4_args, 3},
        {"Invalid -x first, then valid flags", test5_args, 3},
        {"Valid flags first, invalid -x last", test6_args, 3},
        {"Filename before invalid flag", test7_args, 2},
        {"Valid flag, filename, invalid flag", test8_args, 3},
        {"Double dash separator with invalid flag", test9_args, 2},
        {"Double dash invalid flag (--x)", test10_args, 1},
        {"Valid flag, double dash, invalid flag", test11_args, 3},
        {"Multiple invalid flags (-a -b -c)", test12_args, 3},
    };

    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int passed = 0;
    int failed = 0;

    // Run all test cases
    for (int i = 0; i < num_tests; i++) {
        printf("Test %d: %s\n", i + 1, test_cases[i].description);
        printf("Arguments:");
        for (int j = 0; j < test_cases[i].argc; j++) {
            printf(" %s", test_cases[i].argv[j]);
        }
        printf("\n");

        int result = test_gcov_dump(gcov_dump_path, 
                                   test_cases[i].argv, 
                                   test_cases[i].argc);

        if (result == 0) {
            printf("✓ PASS: Triggered default case with 'unknown flag' error\n\n");
            passed++;
        } else if (result == -2) {
            printf("✗ FAIL: Did not trigger target error message\n\n");
            failed++;
        } else {
            printf("✗ ERROR: Execution failed\n\n");
            failed++;
        }
    }

    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", num_tests);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);

    if (passed > 0) {
        printf("\nSuccessfully triggered the uncovered default case in gcov-dump.cc!\n");
        return EXIT_SUCCESS;
    } else {
        printf("\nFailed to trigger the target code path.\n");
        return EXIT_FAILURE;
    }
}
