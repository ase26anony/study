/**
 * test_gcov_dump_flags.c
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

/**
 * Find the gcov-dump executable path.
 * Priority: 1. GCov_DUMP environment variable
 *           2. Common build locations
 *           3. System PATH
 */
static char *find_gcov_dump_path(void) {
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
    
    // 3. Try to find in PATH
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
 * Returns 1 if target error message found, 0 if not, -1 on error.
 */
static int test_gcov_dump_args(const char *gcov_dump_path, char *const argv[]) {
    int pipefd[2];
    pid_t pid;
    char output[MAX_OUTPUT_LEN] = {0};
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
        
        // Read stderr output
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
        
        // Wait for child
        int status;
        waitpid(pid, &status, 0);
        
        // Check if target error message is in output
        if (strstr(output, TARGET_ERROR_MSG) != NULL) {
            found_target = 1;
            printf("Found target error message in output:\n%s\n", output);
        } else {
            printf("Target error message not found. Output was:\n%s\n", output);
        }
        
        return found_target;
    }
}

/**
 * Test various invalid flag combinations.
 */
static int run_tests(const char *gcov_dump_path) {
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test 1: Single invalid flag as first argument
    {
        printf("Test 1: Single invalid flag '-x'\n");
        char *args[] = { "gcov-dump", "-x", NULL };
        int result = test_gcov_dump_args(gcov_dump_path, args);
        total_tests++;
        if (result == 1) passed_tests++;
        printf("\n");
    }
    
    // Test 2: Invalid flag between valid flags
    {
        printf("Test 2: Invalid flag '-z' between valid flags '-l' and '-p'\n");
        char *args[] = { "gcov-dump", "-l", "-z", "-p", NULL };
        int result = test_gcov_dump_args(gcov_dump_path, args);
        total_tests++;
        if (result == 1) passed_tests++;
        printf("\n");
    }
    
    // Test 3: Multiple invalid flags
    {
        printf("Test 3: Multiple invalid flags '-? -y'\n");
        char *args[] = { "gcov-dump", "-?", "-y", NULL };
        int result = test_gcov_dump_args(gcov_dump_path, args);
        total_tests++;
        if (result == 1) passed_tests++;
        printf("\n");
    }
    
    // Test 4: Invalid flag after a filename (non-option argument)
    {
        printf("Test 4: Invalid flag '-a' after filename 'test.gcda'\n");
        char *args[] = { "gcov-dump", "test.gcda", "-a", NULL };
        int result = test_gcov_dump_args(gcov_dump_path, args);
        total_tests++;
        if (result == 1) passed_tests++;
        printf("\n");
    }
    
    // Test 5: Double dash with invalid single-character flag
    {
        printf("Test 5: Double dash with invalid flag '--x'\n");
        char *args[] = { "gcov-dump", "--x", NULL };
        int result = test_gcov_dump_args(gcov_dump_path, args);
        total_tests++;
        if (result == 1) passed_tests++;
        printf("\n");
    }
    
    // Test 6: Combination of valid and invalid flags
    {
        printf("Test 6: Combination '-l -x -p -r -s -w'\n");
        char *args[] = { "gcov-dump", "-l", "-x", "-p", "-r", "-s", "-w", NULL };
        int result = test_gcov_dump_args(gcov_dump_path, args);
        total_tests++;
        if (result == 1) passed_tests++;
        printf("\n");
    }
    
    // Test 7: Invalid flag at end
    {
        printf("Test 7: Valid flags followed by invalid '-l -p -q'\n");
        char *args[] = { "gcov-dump", "-l", "-p", "-q", NULL };
        int result = test_gcov_dump_args(gcov_dump_path, args);
        total_tests++;
        if (result == 1) passed_tests++;
        printf("\n");
    }
    
    printf("========================================\n");
    printf("Test Results: %d/%d tests passed\n", passed_tests, total_tests);
    
    return (passed_tests > 0) ? 0 : 1;
}

int main(void) {
    char *gcov_dump_path = find_gcov_dump_path();
    
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Found gcov-dump: %s\n", gcov_dump_path);
    
    int result = run_tests(gcov_dump_path);
    
    return result;
}
