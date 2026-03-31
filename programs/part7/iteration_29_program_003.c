/**
 * test_gcov_dump_flags.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by invoking
 * gcov-dump with invalid command-line flags.
 * 
 * Compile with: gcc -std=c99 -O0 -g -o test_gcov_dump test_gcov_dump_flags.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

/* Maximum path length for gcov-dump executable */
#define MAX_PATH 1024
#define MAX_ARGS 20
#define BUFFER_SIZE 4096

/**
 * Find the gcov-dump executable path.
 * Priority:
 * 1. GCov_DUMP environment variable
 * 2. Common build locations
 * 3. System PATH
 */
static int find_gcov_dump(char *path, size_t path_len) {
    const char *env_path = getenv("GCov_DUMP");
    const char *candidates[] = {
        "./gcc/gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "./gcov-dump",
        "gcov-dump",
        NULL
    };
    
    /* Try environment variable first */
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, path_len - 1);
        path[path_len - 1] = '\0';
        return 1;
    }
    
    /* Try hardcoded paths */
    for (int i = 0; candidates[i] != NULL; i++) {
        if (access(candidates[i], X_OK) == 0) {
            strncpy(path, candidates[i], path_len - 1);
            path[path_len - 1] = '\0';
            return 1;
        }
    }
    
    /* Try PATH lookup */
    const char *path_env = getenv("PATH");
    if (path_env != NULL) {
        char *path_copy = strdup(path_env);
        char *dir = strtok(path_copy, ":");
        
        while (dir != NULL) {
            char full_path[MAX_PATH];
            snprintf(full_path, sizeof(full_path), "%s/gcov-dump", dir);
            if (access(full_path, X_OK) == 0) {
                strncpy(path, full_path, path_len - 1);
                path[path_len - 1] = '\0';
                free(path_copy);
                return 1;
            }
            dir = strtok(NULL, ":");
        }
        free(path_copy);
    }
    
    return 0;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if "unknown flag" error is found in stderr, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, char *const args[]) {
    int pipefd[2];
    pid_t pid;
    char buffer[BUFFER_SIZE];
    int found_error = 0;
    
    /* Create pipe for capturing stderr */
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 0;
    }
    
    pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return 0;
    }
    
    if (pid == 0) {
        /* Child process */
        close(pipefd[0]);  /* Close read end */
        
        /* Redirect stderr to pipe */
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        /* Execute gcov-dump */
        execvp(gcov_dump_path, args);
        
        /* If we get here, exec failed */
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        close(pipefd[1]);  /* Close write end */
        
        /* Read stderr output */
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            
            /* Check for "unknown flag" error message */
            if (strstr(buffer, "unknown flag") != NULL) {
                found_error = 1;
                printf("Found expected error: %s", buffer);
            }
        }
        
        close(pipefd[0]);
        
        /* Wait for child */
        int status;
        waitpid(pid, &status, 0);
        
        /* Also check exit status (should be non-zero for invalid flag) */
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            printf("Process exited with status %d (expected for invalid flag)\n", 
                   WEXITSTATUS(status));
        }
    }
    
    return found_error;
}

/**
 * Test various invalid flag scenarios.
 */
static int run_tests(const char *gcov_dump_path) {
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    /* Test 1: Single invalid flag as first argument */
    {
        printf("Test 1: Single invalid flag '-x'\n");
        char *args[] = { "gcov-dump", "-x", NULL };
        if (test_invalid_flag(gcov_dump_path, args)) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n");
        }
        total_tests++;
    }
    
    /* Test 2: Invalid flag between valid flags */
    {
        printf("\nTest 2: Invalid flag '-z' between valid flags '-l -z -p'\n");
        char *args[] = { "gcov-dump", "-l", "-z", "-p", NULL };
        if (test_invalid_flag(gcov_dump_path, args)) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n");
        }
        total_tests++;
    }
    
    /* Test 3: Multiple invalid flags */
    {
        printf("\nTest 3: Multiple invalid flags '-? -y'\n");
        char *args[] = { "gcov-dump", "-?", "-y", NULL };
        if (test_invalid_flag(gcov_dump_path, args)) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n");
        }
        total_tests++;
    }
    
    /* Test 4: Invalid flag after non-option argument (requires a dummy file) */
    {
        printf("\nTest 4: Invalid flag '-q' after dummy argument\n");
        char *args[] = { "gcov-dump", "dummy.gcda", "-q", NULL };
        if (test_invalid_flag(gcov_dump_path, args)) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n");
        }
        total_tests++;
    }
    
    /* Test 5: Double dash with invalid single-character flag */
    {
        printf("\nTest 5: Double dash with invalid flag '--x'\n");
        char *args[] = { "gcov-dump", "--x", NULL };
        if (test_invalid_flag(gcov_dump_path, args)) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n");
        }
        total_tests++;
    }
    
    /* Test 6: Combined valid and invalid in single argument */
    {
        printf("\nTest 6: Combined flags '-lpz' (z is invalid)\n");
        char *args[] = { "gcov-dump", "-lpz", NULL };
        if (test_invalid_flag(gcov_dump_path, args)) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n");
        }
        total_tests++;
    }
    
    /* Test 7: Boundary case - non-alphabetic invalid flag */
    {
        printf("\nTest 7: Non-alphabetic invalid flag '-@'\n");
        char *args[] = { "gcov-dump", "-@", NULL };
        if (test_invalid_flag(gcov_dump_path, args)) {
            printf("✓ PASSED\n");
            passed_tests++;
        } else {
            printf("✗ FAILED\n");
        }
        total_tests++;
    }
    
    printf("\n========================================\n");
    printf("Test Results: %d/%d tests passed\n", passed_tests, total_tests);
    printf("========================================\n");
    
    return (passed_tests == total_tests) ? 0 : 1;
}

int main(void) {
    char gcov_dump_path[MAX_PATH];
    
    /* Find gcov-dump executable */
    if (!find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path))) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        fprintf(stderr, "Common locations: ./gcc/gcov-dump, ../gcc/gcov-dump\n");
        return EXIT_FAILURE;
    }
    
    printf("Found gcov-dump: %s\n", gcov_dump_path);
    
    /* Run all tests */
    return run_tests(gcov_dump_path);
}
