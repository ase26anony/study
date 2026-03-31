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

#define MAX_PATH_LEN 1024
#define MAX_OUTPUT_LEN 4096
#define ERROR_MSG_PREFIX "unknown flag"

/**
 * Find the gcov-dump executable path.
 * Priority:
 * 1. GCov_DUMP environment variable
 * 2. Common build locations
 * 3. System PATH
 */
static char* find_gcov_dump_path() {
    static char path[MAX_PATH_LEN];
    
    // 1. Check environment variable
    char* env_path = getenv("GCov_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, MAX_PATH_LEN - 1);
        path[MAX_PATH_LEN - 1] = '\0';
        return path;
    }
    
    // 2. Check common build locations
    const char* common_paths[] = {
        "./gcc/gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "./gcov-dump",
        "../gcov-dump",
        "../../gcov-dump",
        "gcc/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path, common_paths[i], MAX_PATH_LEN - 1);
            path[MAX_PATH_LEN - 1] = '\0';
            return path;
        }
    }
    
    // 3. Check system PATH
    char* path_env = getenv("PATH");
    if (path_env) {
        char* path_copy = strdup(path_env);
        char* dir = strtok(path_copy, ":");
        
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
    
    // Not found
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if error message found, 0 if not, -1 on execution error.
 */
static int test_gcov_dump_args(const char* gcov_dump_path, char* const argv[]) {
    int pipefd[2];
    pid_t pid;
    int status;
    char output[MAX_OUTPUT_LEN];
    int found_error = 0;
    
    // Create pipe for stderr
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
        
        // Read stderr output
        ssize_t bytes_read = read(pipefd[0], output, MAX_OUTPUT_LEN - 1);
        close(pipefd[0]);
        
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
            
            // Check for error message
            if (strstr(output, ERROR_MSG_PREFIX) != NULL) {
                printf("Found error message in output:\n%s\n", output);
                found_error = 1;
            } else {
                printf("No error message found. Output:\n%s\n", output);
            }
        }
        
        // Wait for child
        waitpid(pid, &status, 0);
        
        if (!WIFEXITED(status)) {
            printf("Child process did not exit normally\n");
            return -1;
        }
    }
    
    return found_error;
}

/**
 * Test case structure
 */
typedef struct {
    char* name;
    char* args[10];  // NULL terminated
} test_case_t;

int main(void) {
    char* gcov_dump_path = find_gcov_dump_path();
    
    if (!gcov_dump_path) {
        fprintf(stderr, "Error: gcov-dump executable not found\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Define test cases targeting different positions and combinations
    test_case_t test_cases[] = {
        // Single invalid flag in different positions
        {"Single invalid flag first", {gcov_dump_path, "-x", NULL}},
        {"Invalid flag between valid flags", {gcov_dump_path, "-l", "-x", "-p", NULL}},
        {"Invalid flag after valid flags", {gcov_dump_path, "-l", "-p", "-x", NULL}},
        {"Multiple invalid flags", {gcov_dump_path, "-x", "-z", "-?", NULL}},
        
        // Edge cases with double dash
        {"Double dash with invalid flag", {gcov_dump_path, "--", "-x", NULL}},
        {"Double dash with invalid flag after valid", {gcov_dump_path, "-l", "--", "-x", NULL}},
        
        // Invalid flag with filename argument
        {"Invalid flag before filename", {gcov_dump_path, "-x", "test.gcda", NULL}},
        {"Valid flag, invalid flag, filename", {gcov_dump_path, "-l", "-x", "test.gcda", NULL}},
        
        // Boundary: invalid flag that's not a letter
        {"Non-letter invalid flag", {gcov_dump_path, "-@", NULL}},
        {"Number as invalid flag", {gcov_dump_path, "-1", NULL}},
        
        // Combination with help/version flags (should still trigger error for invalid ones)
        {"Invalid flag with -h", {gcov_dump_path, "-h", "-x", NULL}},
        {"Invalid flag with -v", {gcov_dump_path, "-v", "-x", NULL}},
        
        // End marker
        {NULL, {NULL}}
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    int failed_tests = 0;
    
    printf("Running gcov-dump invalid flag tests...\n");
    printf("========================================\n\n");
    
    for (int i = 0; test_cases[i].name != NULL; i++) {
        total_tests++;
        
        printf("Test %d: %s\n", total_tests, test_cases[i].name);
        printf("Command: ");
        for (int j = 0; test_cases[i].args[j] != NULL; j++) {
            printf("%s ", test_cases[i].args[j]);
        }
        printf("\n");
        
        int result = test_gcov_dump_args(gcov_dump_path, test_cases[i].args);
        
        if (result == 1) {
            printf("✓ PASS: Triggered 'unknown flag' error\n\n");
            passed_tests++;
        } else if (result == 0) {
            printf("✗ FAIL: Did not trigger 'unknown flag' error\n\n");
            failed_tests++;
        } else {
            printf("✗ ERROR: Failed to execute test\n\n");
            failed_tests++;
        }
    }
    
    printf("\n========================================\n");
    printf("Test Summary:\n");
    printf("  Total tests:  %d\n", total_tests);
    printf("  Passed:       %d\n", passed_tests);
    printf("  Failed:       %d\n", failed_tests);
    
    if (failed_tests == 0 && total_tests > 0) {
        printf("\nAll tests passed! Successfully triggered uncovered code.\n");
        return EXIT_SUCCESS;
    } else {
        printf("\nSome tests failed.\n");
        return EXIT_FAILURE;
    }
}
