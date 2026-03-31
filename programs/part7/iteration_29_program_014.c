/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by invoking
 * gcov-dump with various invalid command-line flags.
 * 
 * Compile with: gcc -std=c99 -Wall -O0 -g -o test_gcov_dump test_gcov_dump_invalid_flags.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH_LEN 1024
#define MAX_OUTPUT_LEN 4096
#define ERROR_MSG_SUBSTRING "unknown flag"

/**
 * Find the gcov-dump executable path.
 * Priority:
 * 1. GCov_DUMP environment variable
 * 2. Common build tree locations
 * 3. System PATH
 */
static char* find_gcov_dump_path() {
    static char path[MAX_PATH_LEN];
    
    // 1. Check environment variable
    char* env_path = getenv("GCov_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, MAX_PATH_LEN - 1);
        path[MAX_PATH_LEN - 1] = '\0';
        return path;
    }
    
    // 2. Check common build tree locations
    const char* common_paths[] = {
        "./gcc/gcov-dump",
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
    
    // 3. Try to find in PATH
    char* path_env = getenv("PATH");
    if (path_env != NULL) {
        char* path_copy = strdup(path_env);
        char* token = strtok(path_copy, ":");
        
        while (token != NULL) {
            snprintf(path, MAX_PATH_LEN, "%s/gcov-dump", token);
            if (access(path, X_OK) == 0) {
                free(path_copy);
                return path;
            }
            token = strtok(NULL, ":");
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
static int test_gcov_dump_with_args(const char* gcov_path, char* const argv[]) {
    int pipefd[2];
    pid_t pid;
    char output[MAX_OUTPUT_LEN] = {0};
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
    
    if (pid == 0) {  // Child process
        // Close read end of pipe
        close(pipefd[0]);
        
        // Redirect stderr to pipe
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        // Execute gcov-dump
        execvp(gcov_path, argv);
        
        // If we get here, exec failed
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else {  // Parent process
        // Close write end of pipe
        close(pipefd[1]);
        
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
        
        // Check if error message is in output
        if (strstr(output, ERROR_MSG_SUBSTRING) != NULL) {
            found_error = 1;
        }
        
        // Debug output
        printf("Command: ");
        for (int i = 0; argv[i] != NULL; i++) {
            printf("%s ", argv[i]);
        }
        printf("\n");
        printf("Output (stderr):\n%s\n", output);
        printf("Found error message: %s\n\n", found_error ? "YES" : "NO");
    }
    
    return found_error;
}

/**
 * Test various invalid flag scenarios
 */
static int run_all_tests(const char* gcov_path) {
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("Testing gcov-dump at: %s\n\n", gcov_path);
    
    // Test 1: Single invalid flag as first argument
    {
        char* args[] = { "gcov-dump", "-x", NULL };
        printf("Test 1: Single invalid flag '-x'\n");
        int result = test_gcov_dump_with_args(gcov_path, args);
        total_tests++;
        if (result == 1) passed_tests++;
        else if (result == -1) return -1;
    }
    
    // Test 2: Invalid flag between valid flags
    {
        char* args[] = { "gcov-dump", "-l", "-x", "-p", NULL };
        printf("Test 2: Invalid flag '-x' between valid flags '-l' and '-p'\n");
        int result = test_gcov_dump_with_args(gcov_path, args);
        total_tests++;
        if (result == 1) passed_tests++;
        else if (result == -1) return -1;
    }
    
    // Test 3: Multiple invalid flags
    {
        char* args[] = { "gcov-dump", "-x", "-z", "-?", NULL };
        printf("Test 3: Multiple invalid flags '-x -z -?'\n");
        int result = test_gcov_dump_with_args(gcov_path, args);
        total_tests++;
        if (result == 1) passed_tests++;
        else if (result == -1) return -1;
    }
    
    // Test 4: Invalid flag after non-option argument (filename)
    {
        char* args[] = { "gcov-dump", "dummy.gcda", "-x", NULL };
        printf("Test 4: Invalid flag '-x' after filename argument\n");
        int result = test_gcov_dump_with_args(gcov_path, args);
        total_tests++;
        if (result == 1) passed_tests++;
        else if (result == -1) return -1;
    }
    
    // Test 5: Double dash followed by invalid single-character flag
    {
        char* args[] = { "gcov-dump", "--", "-x", NULL };
        printf("Test 5: Double dash '--' followed by invalid flag '-x'\n");
        int result = test_gcov_dump_with_args(gcov_path, args);
        total_tests++;
        if (result == 1) passed_tests++;
        else if (result == -1) return -1;
    }
    
    // Test 6: Combined valid and invalid flags
    {
        char* args[] = { "gcov-dump", "-l", "-x", "-p", "-r", "-z", NULL };
        printf("Test 6: Mixed valid and invalid flags '-l -x -p -r -z'\n");
        int result = test_gcov_dump_with_args(gcov_path, args);
        total_tests++;
        if (result == 1) passed_tests++;
        else if (result == -1) return -1;
    }
    
    // Test 7: Invalid flag with argument (testing getopt behavior)
    {
        char* args[] = { "gcov-dump", "-x", "argument", NULL };
        printf("Test 7: Invalid flag '-x' with following argument\n");
        int result = test_gcov_dump_with_args(gcov_path, args);
        total_tests++;
        if (result == 1) passed_tests++;
        else if (result == -1) return -1;
    }
    
    printf("\n========================================\n");
    printf("Test Results: %d/%d tests passed\n", passed_tests, total_tests);
    printf("========================================\n\n");
    
    return (passed_tests == total_tests) ? 1 : 0;
}

int main(int argc, char* argv[]) {
    printf("========================================\n");
    printf("Testing gcov-dump invalid flag handling\n");
    printf("Target: Trigger default case in switch statement\n");
    printf("Expected error: \"unknown flag `%%c'\\n\"\n");
    printf("========================================\n\n");
    
    // Find gcov-dump executable
    char* gcov_path = find_gcov_dump_path();
    if (gcov_path == NULL) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH.\n");
        fprintf(stderr, "Common locations checked:\n");
        fprintf(stderr, "  - ./gcc/gcov-dump\n");
        fprintf(stderr, "  - ../gcc/gcov-dump\n");
        fprintf(stderr, "  - /usr/bin/gcov-dump\n");
        fprintf(stderr, "  - PATH directories\n");
        return EXIT_FAILURE;
    }
    
    // Run all tests
    int result = run_all_tests(gcov_path);
    
    if (result == -1) {
        fprintf(stderr, "Error during test execution.\n");
        return EXIT_FAILURE;
    }
    
    if (result == 1) {
        printf("SUCCESS: All tests passed!\n");
        printf("The uncovered default case in gcov-dump.cc was triggered.\n");
        return EXIT_SUCCESS;
    } else {
        printf("FAILURE: Some tests did not trigger the expected error.\n");
        printf("This could mean:\n");
        printf("  1. gcov-dump handles invalid flags differently\n");
        printf("  2. The executable is not the expected version\n");
        printf("  3. getopt behavior differs\n");
        return EXIT_FAILURE;
    }
}
