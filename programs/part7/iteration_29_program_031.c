/**
 * Test program to trigger uncovered lines in gcov-dump.cc
 * Specifically targets the default case in the flag parsing switch
 * that prints "unknown flag `%c'\n"
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
 * Find the gcov-dump executable path
 * Priority: 1. GCov_DUMP environment variable
 *           2. Common build locations
 *           3. System PATH
 */
static char *find_gcov_dump_path(void) {
    static char path[MAX_PATH_LEN];
    
    // Check environment variable first
    char *env_path = getenv("GCov_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, MAX_PATH_LEN - 1);
        path[MAX_PATH_LEN - 1] = '\0';
        return path;
    }
    
    // Try common build locations
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "./gcov-dump",
        "../gcov-dump",
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
    
    // Last resort: check PATH
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
 * Execute gcov-dump with given arguments and capture stderr
 * Returns 1 if target error message found, 0 if not, -1 on error
 */
static int test_gcov_dump_with_args(const char *gcov_dump_path, char *const argv[]) {
    int pipefd[2];
    pid_t pid;
    char output[MAX_OUTPUT_LEN] = {0};
    int status;
    
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
    
    if (pid == 0) {  // Child process
        // Redirect stderr to pipe
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        
        // Execute gcov-dump
        execvp(gcov_dump_path, argv);
        
        // If we get here, exec failed
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else {  // Parent process
        close(pipefd[1]);  // Close write end
        
        // Read stderr output
        ssize_t bytes_read = read(pipefd[0], output, MAX_OUTPUT_LEN - 1);
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
        }
        
        close(pipefd[0]);
        
        // Wait for child
        waitpid(pid, &status, 0);
        
        // Check if target error message is in output
        if (strstr(output, TARGET_ERROR_MSG) != NULL) {
            printf("Found target error message in output:\n%s\n", output);
            return 1;
        }
        
        return 0;
    }
}

/**
 * Run a series of test cases with invalid flags
 */
static int run_test_cases(const char *gcov_dump_path) {
    int success_count = 0;
    int test_count = 0;
    
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test 1: Single invalid flag at beginning
    {
        char *args[] = { "gcov-dump", "-x", NULL };
        printf("Test %d: Single invalid flag '-x'\n", ++test_count);
        if (test_gcov_dump_with_args(gcov_dump_path, args) == 1) {
            success_count++;
            printf("  PASS\n");
        } else {
            printf("  FAIL\n");
        }
    }
    
    // Test 2: Invalid flag between valid flags
    {
        char *args[] = { "gcov-dump", "-l", "-z", "-p", NULL };
        printf("\nTest %d: Invalid flag '-z' between valid flags '-l' and '-p'\n", ++test_count);
        if (test_gcov_dump_with_args(gcov_dump_path, args) == 1) {
            success_count++;
            printf("  PASS\n");
        } else {
            printf("  FAIL\n");
        }
    }
    
    // Test 3: Multiple invalid flags
    {
        char *args[] = { "gcov-dump", "-?", "-@", "-!", NULL };
        printf("\nTest %d: Multiple invalid flags '-?', '-@', '-!'\n", ++test_count);
        if (test_gcov_dump_with_args(gcov_dump_path, args) == 1) {
            success_count++;
            printf("  PASS\n");
        } else {
            printf("  FAIL\n");
        }
    }
    
    // Test 4: Invalid flag after non-option argument (simulated with help)
    {
        char *args[] = { "gcov-dump", "-h", "-y", NULL };
        printf("\nTest %d: Invalid flag '-y' after help flag '-h'\n", ++test_count);
        if (test_gcov_dump_with_args(gcov_dump_path, args) == 1) {
            success_count++;
            printf("  PASS\n");
        } else {
            printf("  FAIL\n");
        }
    }
    
    // Test 5: Double dash with invalid single character
    {
        char *args[] = { "gcov-dump", "--x", NULL };
        printf("\nTest %d: Double dash with invalid flag '--x'\n", ++test_count);
        if (test_gcov_dump_with_args(gcov_dump_path, args) == 1) {
            success_count++;
            printf("  PASS\n");
        } else {
            printf("  FAIL\n");
        }
    }
    
    // Test 6: Combined valid and invalid in single argument
    {
        char *args[] = { "gcov-dump", "-lpzr", NULL };
        printf("\nTest %d: Combined flags '-lpzr' with invalid 'z'\n", ++test_count);
        if (test_gcov_dump_with_args(gcov_dump_path, args) == 1) {
            success_count++;
            printf("  PASS\n");
        } else {
            printf("  FAIL\n");
        }
    }
    
    // Test 7: Edge case - invalid flag at end
    {
        char *args[] = { "gcov-dump", "-l", "-p", "-k", NULL };
        printf("\nTest %d: Invalid flag '-k' at end\n", ++test_count);
        if (test_gcov_dump_with_args(gcov_dump_path, args) == 1) {
            success_count++;
            printf("  PASS\n");
        } else {
            printf("  FAIL\n");
        }
    }
    
    printf("\n========================================\n");
    printf("Test Results: %d/%d tests passed\n", success_count, test_count);
    printf("========================================\n");
    
    return (success_count > 0) ? 0 : 1;
}

int main(void) {
    char *gcov_dump_path = find_gcov_dump_path();
    
    if (!gcov_dump_path) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCov_DUMP environment variable to specify path\n");
        return EXIT_FAILURE;
    }
    
    int result = run_test_cases(gcov_dump_path);
    
    return result;
}
