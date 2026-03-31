/**
 * test_gcov_dump_default_case.c
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

#define MAX_PATH_LEN 1024
#define MAX_OUTPUT_LEN 4096
#define ERROR_MSG_PREFIX "unknown flag"

/**
 * Find the gcov-dump executable path.
 * Priority: 1. GCov_DUMP environment variable
 *           2. Common build locations
 *           3. PATH search
 */
static char *find_gcov_dump(void) {
    static char path[MAX_PATH_LEN];
    
    // 1. Check environment variable
    char *env_path = getenv("GCov_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, MAX_PATH_LEN - 1);
        path[MAX_PATH_LEN - 1] = '\0';
        return path;
    }
    
    // 2. Check common build locations
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "../../prev-gcc/build/gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path, common_paths[i], MAX_PATH_LEN - 1);
            path[MAX_PATH_LEN - 1] = '\0';
            return path;
        }
    }
    
    // 3. Search in PATH
    char *path_env = getenv("PATH");
    if (path_env) {
        char *path_copy = strdup(path_env);
        char *dir = strtok(path_copy, ":");
        
        while (dir != NULL) {
            snprintf(path, MAX_PATH_LEN, "%s/gcov-dump", dir);
            if (access(path, X_OK) == 0) {
                free(path_copy);
                return path;
            }
            dir = strtok(NULL, ":");
        }
        free(path_copy);
    }
    
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 0 on success (found error message), non-zero on failure.
 */
static int test_gcov_dump(const char *gcov_dump_path, const char **args, int arg_count) {
    int pipefd[2];
    pid_t pid;
    char output[MAX_OUTPUT_LEN] = {0};
    int status;
    
    // Create pipe for capturing stderr
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
        char **argv = malloc((arg_count + 2) * sizeof(char *));
        if (!argv) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        
        argv[0] = (char *)gcov_dump_path;
        for (int i = 0; i < arg_count; i++) {
            argv[i + 1] = (char *)args[i];
        }
        argv[arg_count + 1] = NULL;
        
        // Execute gcov-dump
        execv(gcov_dump_path, argv);
        
        // If we get here, exec failed
        perror("execv");
        free(argv);
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        close(pipefd[1]);  // Close write end
        
        // Read stderr output
        ssize_t bytes_read = read(pipefd[0], output, MAX_OUTPUT_LEN - 1);
        close(pipefd[0]);
        
        // Wait for child
        waitpid(pid, &status, 0);
        
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
            
            // Check for error message
            if (strstr(output, ERROR_MSG_PREFIX) != NULL) {
                printf("Found expected error message:\n%s\n", output);
                return 0;  // Success
            } else {
                printf("Unexpected output:\n%s\n", output);
                return 1;  // Failure
            }
        } else {
            printf("No output captured\n");
            return 1;
        }
    }
}

/**
 * Test various invalid flag scenarios.
 */
int main(void) {
    char *gcov_dump_path = find_gcov_dump();
    
    if (!gcov_dump_path) {
        fprintf(stderr, "Error: gcov-dump executable not found\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test cases covering different scenarios
    struct test_case {
        const char *name;
        const char *args[10];
        int arg_count;
    } test_cases[] = {
        // Single invalid flag
        {"Single invalid flag -x", {"-x"}, 1},
        {"Single invalid flag -z", {"-z"}, 1},
        {"Single invalid flag -?", {"-?"}, 1},
        
        // Invalid flag in different positions
        {"Invalid flag first: -x -l", {"-x", "-l"}, 2},
        {"Valid then invalid: -l -x", {"-l", "-x"}, 2},
        {"Invalid between valid: -l -x -p", {"-l", "-x", "-p"}, 3},
        {"Multiple invalid: -x -y -z", {"-x", "-y", "-z"}, 3},
        
        // With filename argument
        {"Invalid flag before filename: -x test.gcda", {"-x", "test.gcda"}, 2},
        {"Invalid flag after filename: test.gcda -x", {"test.gcda", "-x"}, 2},
        
        // Double dash handling
        {"Double dash invalid: --x", {"--x"}, 1},
        {"Double dash with valid: --l", {"--l"}, 1},
        
        // Combined valid and invalid
        {"Mixed: -l -p -x -s -r", {"-l", "-p", "-x", "-s", "-r"}, 5},
        
        // Edge cases
        {"Unknown flag with value: -Xvalue", {"-Xvalue"}, 1},
        {"Just dash: -", {"-"}, 1},
    };
    
    int total_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int passed_tests = 0;
    
    for (int i = 0; i < total_tests; i++) {
        printf("Test %d/%d: %s\n", i + 1, total_tests, test_cases[i].name);
        printf("Arguments: ");
        for (int j = 0; j < test_cases[i].arg_count; j++) {
            printf("%s ", test_cases[i].args[j]);
        }
        printf("\n");
        
        int result = test_gcov_dump(gcov_dump_path, 
                                   test_cases[i].args, 
                                   test_cases[i].arg_count);
        
        if (result == 0) {
            printf("✓ PASSED\n\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n\n");
        }
    }
    
    printf("\n========================================\n");
    printf("Test Summary: %d/%d tests passed\n", passed_tests, total_tests);
    
    if (passed_tests == total_tests) {
        printf("All tests passed successfully!\n");
        return EXIT_SUCCESS;
    } else {
        printf("Some tests failed.\n");
        return EXIT_FAILURE;
    }
}
