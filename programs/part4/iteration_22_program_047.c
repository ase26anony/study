/**
 * test_gcov_dump_default_case.c
 * 
 * Tests the uncovered default case in gcov-dump.cc that handles
 * unknown command-line flags.
 * 
 * Compile with: gcc -std=c99 -O0 -g -o test_gcov_dump test_gcov_dump_default_case.c
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
 * Priority:
 * 1. GCov_DUMP environment variable
 * 2. Common build locations
 * 3. System PATH
 */
static int find_gcov_dump(char *path, size_t path_len) {
    const char *env_path = getenv("GCov_DUMP");
    
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, path_len - 1);
        path[path_len - 1] = '\0';
        return 1;
    }
    
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path, common_paths[i], path_len - 1);
            path[path_len - 1] = '\0';
            return 1;
        }
    }
    
    return 0;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if target error message found, 0 if not, -1 on error.
 */
static int test_gcov_dump_with_args(const char *gcov_dump_path, 
                                   const char *test_name,
                                   const char *args[]) {
    printf("Test: %s\n", test_name);
    printf("Command: %s", gcov_dump_path);
    
    for (int i = 0; args[i] != NULL; i++) {
        printf(" %s", args[i]);
    }
    printf("\n");
    
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return -1;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }
    
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        execv(gcov_dump_path, (char * const *)args);
        perror("execv failed");
        exit(EXIT_FAILURE);
    }
    
    close(pipefd[1]);
    
    char buffer[MAX_OUTPUT_LEN];
    ssize_t bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);
    
    int status;
    waitpid(pid, &status, 0);
    
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("Stderr output:\n%s\n", buffer);
        
        if (strstr(buffer, TARGET_ERROR_MSG) != NULL) {
            printf("✓ Found target error message: '%s'\n\n", TARGET_ERROR_MSG);
            return 1;
        }
    } else if (bytes_read == 0) {
        printf("No stderr output\n\n");
    }
    
    printf("✗ Target error message not found\n\n");
    return 0;
}

int main(void) {
    char gcov_dump_path[MAX_PATH_LEN];
    
    if (!find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path))) {
        fprintf(stderr, "Error: gcov-dump executable not found\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    int total_tests = 0;
    int passed_tests = 0;
    
    /* Test 1: Single invalid flag at beginning */
    {
        const char *args[] = {gcov_dump_path, "-x", NULL};
        total_tests++;
        if (test_gcov_dump_with_args(gcov_dump_path, 
                                    "Single invalid flag '-x'", 
                                    args) > 0) {
            passed_tests++;
        }
    }
    
    /* Test 2: Single invalid flag 'z' */
    {
        const char *args[] = {gcov_dump_path, "-z", NULL};
        total_tests++;
        if (test_gcov_dump_with_args(gcov_dump_path, 
                                    "Single invalid flag '-z'", 
                                    args) > 0) {
            passed_tests++;
        }
    }
    
    /* Test 3: Invalid flag between valid flags */
    {
        const char *args[] = {gcov_dump_path, "-l", "-x", "-p", NULL};
        total_tests++;
        if (test_gcov_dump_with_args(gcov_dump_path, 
                                    "Invalid flag '-x' between valid flags '-l -p'", 
                                    args) > 0) {
            passed_tests++;
        }
    }
    
    /* Test 4: Multiple invalid flags */
    {
        const char *args[] = {gcov_dump_path, "-a", "-b", "-c", NULL};
        total_tests++;
        if (test_gcov_dump_with_args(gcov_dump_path, 
                                    "Multiple invalid flags '-a -b -c'", 
                                    args) > 0) {
            passed_tests++;
        }
    }
    
    /* Test 5: Invalid flag after filename argument */
    {
        const char *args[] = {gcov_dump_path, "test.gcda", "-x", NULL};
        total_tests++;
        if (test_gcov_dump_with_args(gcov_dump_path, 
                                    "Invalid flag '-x' after filename", 
                                    args) > 0) {
            passed_tests++;
        }
    }
    
    /* Test 6: Double dash with invalid flag (edge case) */
    {
        const char *args[] = {gcov_dump_path, "--x", NULL};
        total_tests++;
        if (test_gcov_dump_with_args(gcov_dump_path, 
                                    "Double dash with invalid flag '--x'", 
                                    args) > 0) {
            passed_tests++;
        }
    }
    
    /* Test 7: Question mark flag (special getopt handling) */
    {
        const char *args[] = {gcov_dump_path, "-?", NULL};
        total_tests++;
        if (test_gcov_dump_with_args(gcov_dump_path, 
                                    "Question mark flag '-?'", 
                                    args) > 0) {
            passed_tests++;
        }
    }
    
    /* Test 8: Combined valid and invalid flags */
    {
        const char *args[] = {gcov_dump_path, "-v", "-x", "-h", NULL};
        total_tests++;
        if (test_gcov_dump_with_args(gcov_dump_path, 
                                    "Mixed valid/invalid flags '-v -x -h'", 
                                    args) > 0) {
            passed_tests++;
        }
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    
    if (passed_tests > 0) {
        printf("\n✓ Successfully triggered the uncovered default case!\n");
        return EXIT_SUCCESS;
    } else {
        printf("\n✗ Failed to trigger the uncovered default case\n");
        return EXIT_FAILURE;
    }
}
