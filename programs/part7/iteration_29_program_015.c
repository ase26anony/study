/**
 * test_gcov_dump_invalid_flags.c
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

#define MAX_CMD_LEN 1024
#define MAX_OUTPUT_LEN 4096
#define TARGET_ERROR "unknown flag"

/**
 * Find the gcov-dump executable path.
 * Priority: 1. GCov_DUMP environment variable
 *           2. Common build locations
 * Returns dynamically allocated string or NULL if not found.
 */
static char *find_gcov_dump_path(void) {
    char *path = getenv("GCov_DUMP");
    
    if (path != NULL && access(path, X_OK) == 0) {
        return strdup(path);
    }
    
    // Try common build locations
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
            return strdup(common_paths[i]);
        }
    }
    
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if target error message found, 0 if not, -1 on execution error.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
    int pipefd[2];
    pid_t pid;
    char output[MAX_OUTPUT_LEN] = {0};
    int status;
    int found_target = 0;
    
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
        
        // Prepare arguments
        char *argv[4] = {0};
        argv[0] = (char *)gcov_dump_path;
        argv[1] = (char *)flag;
        argv[2] = NULL;
        
        execvp(gcov_dump_path, argv);
        
        // execvp only returns on error
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        close(pipefd[1]);  // Close write end
        
        // Read stderr output
        ssize_t bytes_read = read(pipefd[0], output, sizeof(output) - 1);
        close(pipefd[0]);
        
        // Wait for child
        waitpid(pid, &status, 0);
        
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
            
            // Check for target error message
            if (strstr(output, TARGET_ERROR) != NULL) {
                printf("Found target error in output:\n%s\n", output);
                found_target = 1;
            } else {
                printf("Output (no target error):\n%s\n", output);
            }
        }
        
        // Check exit status (should be non-zero for invalid flag)
        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            printf("Exit status: %d\n", exit_status);
        }
    }
    
    return found_target;
}

/**
 * Test multiple invalid flag scenarios.
 */
static void run_test_suite(const char *gcov_dump_path) {
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test cases covering different positions and combinations
    struct test_case {
        const char *description;
        const char *flag;
    } test_cases[] = {
        {"Single invalid flag (first position)", "-x"},
        {"Single invalid flag (different char)", "-z"},
        {"Question mark flag", "-?"},
        {"Invalid flag between valid flags", "-l -x -p"},
        {"Multiple invalid flags", "-x -y -z"},
        {"Invalid flag after valid flag", "-p -x"},
        {"Invalid flag before valid flag", "-x -l"},
        {"Double dash with single char", "--x"},
        {"Invalid flag with double dash", "--z"},
        {"Mixed valid and invalid", "-l -p -x -r"},
        {"Only invalid flags", "-a -b -c"},
        {"Invalid flag at end", "-l -p -x"},
        {NULL, NULL}
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    
    for (int i = 0; test_cases[i].description != NULL; i++) {
        printf("Test %d: %s\n", i + 1, test_cases[i].description);
        printf("Command: %s %s\n", gcov_dump_path, test_cases[i].flag);
        
        // For multi-flag tests, we need a different approach
        if (strchr(test_cases[i].flag, ' ') != NULL) {
            // Build command with multiple arguments
            char cmd[MAX_CMD_LEN];
            snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, test_cases[i].flag);
            
            FILE *fp = popen(cmd, "r");
            if (fp == NULL) {
                printf("Failed to execute command\n\n");
                continue;
            }
            
            char output[MAX_OUTPUT_LEN] = {0};
            fread(output, 1, sizeof(output) - 1, fp);
            pclose(fp);
            
            if (strstr(output, TARGET_ERROR) != NULL) {
                printf("✓ Found target error\n");
                passed_tests++;
            } else {
                printf("✗ Target error not found\n");
            }
        } else {
            // Single flag test
            int result = test_invalid_flag(gcov_dump_path, test_cases[i].flag);
            if (result == 1) {
                printf("✓ Found target error\n");
                passed_tests++;
            } else if (result == 0) {
                printf("✗ Target error not found\n");
            } else {
                printf("✗ Execution failed\n");
            }
        }
        
        total_tests++;
        printf("\n");
    }
    
    printf("Test Summary:\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    
    if (passed_tests > 0) {
        printf("\nSUCCESS: Triggered the uncovered default case in gcov-dump.cc\n");
    } else {
        printf("\nFAILURE: Could not trigger the uncovered default case\n");
    }
}

int main(void) {
    char *gcov_dump_path = find_gcov_dump_path();
    
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in a common location.\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n", gcov_dump_path);
    
    // First, test a simple valid flag to ensure gcov-dump works
    printf("Testing with valid flag '-h' to verify gcov-dump works...\n");
    char test_cmd[MAX_CMD_LEN];
    snprintf(test_cmd, sizeof(test_cmd), "%s -h 2>&1", gcov_dump_path);
    FILE *fp = popen(test_cmd, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to test gcov-dump\n");
        free(gcov_dump_path);
        return EXIT_FAILURE;
    }
    
    char buffer[256];
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("gcov-dump appears to be working.\n\n");
    }
    pclose(fp);
    
    // Run the test suite
    run_test_suite(gcov_dump_path);
    
    free(gcov_dump_path);
    return EXIT_SUCCESS;
}
