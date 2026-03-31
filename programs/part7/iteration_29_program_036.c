/**
 * test_gcov_dump_flags.c
 * 
 * Tests gcov-dump's handling of invalid command-line flags
 * to trigger the uncovered default case in the switch statement.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define MAX_OUTPUT_LEN 4096
#define TARGET_ERROR_MSG "unknown flag"

/**
 * Find the gcov-dump executable path.
 * Priority: 1. GCov_DUMP environment variable
 *           2. Common build locations
 *           3. System PATH
 */
static char *find_gcov_dump_path(void) {
    static char path[MAX_CMD_LEN];
    
    // 1. Check environment variable
    char *env_path = getenv("GCov_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, MAX_CMD_LEN - 1);
        path[MAX_CMD_LEN - 1] = '\0';
        return path;
    }
    
    // 2. Check common build locations
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "./gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path, common_paths[i], MAX_CMD_LEN - 1);
            path[MAX_CMD_LEN - 1] = '\0';
            return path;
        }
    }
    
    // 3. Try to find in PATH
    char *path_env = getenv("PATH");
    if (path_env != NULL) {
        char *path_copy = strdup(path_env);
        char *dir = strtok(path_copy, ":");
        
        while (dir != NULL) {
            snprintf(path, MAX_CMD_LEN, "%s/gcov-dump", dir);
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
 * Returns 1 if target error message found, 0 if not, -1 on error.
 */
static int test_gcov_dump_args(const char *gcov_dump_path, char *const argv[]) {
    int pipefd[2];
    pid_t pid;
    char output[MAX_OUTPUT_LEN] = {0};
    int status;
    int found_target = 0;
    
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
        
        // Execute gcov-dump
        execvp(gcov_dump_path, argv);
        
        // If we get here, exec failed
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        close(pipefd[1]);  // Close write end
        
        // Read stderr output from pipe
        ssize_t bytes_read;
        size_t total_read = 0;
        
        while ((bytes_read = read(pipefd[0], output + total_read, 
                                 MAX_OUTPUT_LEN - total_read - 1)) > 0) {
            total_read += bytes_read;
            if (total_read >= MAX_OUTPUT_LEN - 1) {
                break;
            }
        }
        output[total_read] = '\0';
        
        close(pipefd[0]);
        
        // Wait for child to finish
        waitpid(pid, &status, 0);
        
        // Check if target error message is in output
        if (strstr(output, TARGET_ERROR_MSG) != NULL) {
            found_target = 1;
            printf("Found target error message in output:\n%s\n", output);
        } else {
            printf("Output (no target message):\n%s\n", output);
        }
        
        // Also check exit status (should be non-zero for invalid flag)
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            printf("Exit status: %d (non-zero as expected)\n", WEXITSTATUS(status));
        }
    }
    
    return found_target;
}

/**
 * Build argument vector for testing.
 * Caller must free the returned array.
 */
static char **build_argv(const char *gcov_dump_path, const char **args, int arg_count) {
    char **argv = malloc((arg_count + 2) * sizeof(char *));
    if (!argv) {
        perror("malloc");
        return NULL;
    }
    
    argv[0] = strdup(gcov_dump_path);
    for (int i = 0; i < arg_count; i++) {
        argv[i + 1] = strdup(args[i]);
    }
    argv[arg_count + 1] = NULL;
    
    return argv;
}

/**
 * Free argument vector.
 */
static void free_argv(char **argv) {
    if (!argv) return;
    
    for (int i = 0; argv[i] != NULL; i++) {
        free(argv[i]);
    }
    free(argv);
}

int main(void) {
    printf("=== Testing gcov-dump invalid flag handling ===\n");
    
    // Find gcov-dump executable
    char *gcov_dump_path = find_gcov_dump_path();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test cases designed to trigger the uncovered default case
    struct test_case {
        const char *name;
        const char *args[10];
        int arg_count;
    } test_cases[] = {
        // Single invalid flag in various positions
        {"Single invalid flag -x", {"-x"}, 1},
        {"Single invalid flag -z", {"-z"}, 1},
        {"Single invalid flag -?", {"-?"}, 1},
        
        // Invalid flag between valid flags
        {"Invalid between valid flags", {"-l", "-x", "-p"}, 3},
        {"Multiple invalid flags", {"-x", "-y", "-z"}, 3},
        
        // Invalid flag after valid flag
        {"Valid then invalid", {"-v", "-x"}, 2},
        {"Invalid then valid", {"-x", "-v"}, 2},
        
        // Invalid flag with filename (non-option argument)
        {"Invalid flag before filename", {"-x", "test.gcda"}, 2},
        {"Valid, invalid, filename", {"-l", "-x", "test.gcda"}, 3},
        {"Filename then invalid flag", {"test.gcda", "-x"}, 2},
        
        // Double dash edge cases
        {"Double dash with invalid", {"--", "-x"}, 2},
        {"Double dash separator", {"-l", "--", "-x"}, 3},
        
        // Combined valid and invalid
        {"Mixed flags 1", {"-l", "-p", "-x", "-r", "-s"}, 5},
        {"Mixed flags 2", {"-x", "-h", "-y"}, 3},
        
        // End marker
        {NULL, {NULL}, 0}
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Execute each test case
    for (int i = 0; test_cases[i].name != NULL; i++) {
        printf("\n--- Test %d: %s ---\n", i + 1, test_cases[i].name);
        printf("Args: ");
        for (int j = 0; j < test_cases[i].arg_count; j++) {
            printf("%s ", test_cases[i].args[j]);
        }
        printf("\n");
        
        char **argv = build_argv(gcov_dump_path, test_cases[i].args, 
                                test_cases[i].arg_count);
        if (!argv) {
            fprintf(stderr, "Failed to build argv for test %d\n", i + 1);
            continue;
        }
        
        int result = test_gcov_dump_args(gcov_dump_path, argv);
        
        if (result == 1) {
            printf("✓ PASS: Triggered unknown flag error\n");
            passed_tests++;
        } else if (result == 0) {
            printf("✗ FAIL: Did not trigger unknown flag error\n");
        } else {
            printf("✗ ERROR: Test execution failed\n");
        }
        
        free_argv(argv);
        total_tests++;
        
        // Small delay between tests
        sleep(1);
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    
    if (passed_tests > 0) {
        printf("\n✓ SUCCESS: Successfully triggered the uncovered default case\n");
        return EXIT_SUCCESS;
    } else {
        printf("\n✗ FAILURE: Could not trigger the uncovered default case\n");
        return EXIT_FAILURE;
    }
}
