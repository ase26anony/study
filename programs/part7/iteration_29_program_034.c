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
 * Find the gcov-dump executable path
 * Returns: dynamically allocated string with path, or NULL if not found
 */
char* find_gcov_dump_path() {
    char* path = NULL;
    
    // 1. Check environment variable
    char* env_path = getenv("GCOV_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        path = strdup(env_path);
        printf("Using gcov-dump from GCOV_DUMP env: %s\n", path);
        return path;
    }
    
    // 2. Check common build locations
    const char* common_paths[] = {
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
            path = strdup(common_paths[i]);
            printf("Found gcov-dump at: %s\n", path);
            return path;
        }
    }
    
    // 3. Try which command
    FILE* fp = popen("which gcov-dump 2>/dev/null", "r");
    if (fp != NULL) {
        char buffer[MAX_PATH_LEN];
        if (fgets(buffer, sizeof(buffer), fp) != NULL) {
            // Remove trailing newline
            buffer[strcspn(buffer, "\n")] = 0;
            if (access(buffer, X_OK) == 0) {
                path = strdup(buffer);
                printf("Found gcov-dump via which: %s\n", path);
            }
        }
        pclose(fp);
    }
    
    return path;
}

/**
 * Execute gcov-dump with given arguments and capture stderr
 * Returns: 1 if target error message found, 0 if not, -1 on execution error
 */
int test_gcov_dump(const char* gcov_dump_path, const char** args, int arg_count) {
    int found_error = 0;
    int pipefd[2];
    pid_t pid;
    
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
    
    if (pid == 0) {  // Child process
        // Close read end of pipe
        close(pipefd[0]);
        
        // Redirect stderr to pipe
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        // Prepare argument array
        char** argv = malloc((arg_count + 2) * sizeof(char*));
        if (!argv) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        
        argv[0] = (char*)gcov_dump_path;
        for (int i = 0; i < arg_count; i++) {
            argv[i + 1] = (char*)args[i];
        }
        argv[arg_count + 1] = NULL;
        
        // Execute gcov-dump
        execvp(gcov_dump_path, argv);
        
        // If we get here, exec failed
        perror("execvp");
        free(argv);
        exit(EXIT_FAILURE);
    } else {  // Parent process
        // Close write end of pipe
        close(pipefd[1]);
        
        // Read stderr output
        char buffer[MAX_OUTPUT_LEN];
        ssize_t bytes_read;
        char output[MAX_OUTPUT_LEN] = {0};
        
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            strncat(output, buffer, sizeof(output) - strlen(output) - 1);
        }
        
        close(pipefd[0]);
        
        // Wait for child
        int status;
        waitpid(pid, &status, 0);
        
        // Check if target error message is in output
        if (strstr(output, TARGET_ERROR_MSG) != NULL) {
            printf("✓ Found target error message in output:\n%s\n", output);
            found_error = 1;
        } else {
            printf("✗ Target error message not found. Output:\n%s\n", output);
        }
        
        return found_error;
    }
}

/**
 * Test cases for invalid flag parsing
 */
typedef struct {
    const char* description;
    const char* args[10];
    int arg_count;
} TestCase;

int main() {
    printf("=== Testing uncovered lines in gcov-dump.cc ===\n\n");
    
    // Find gcov-dump executable
    char* gcov_dump_path = find_gcov_dump_path();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: gcov-dump executable not found\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    // Define test cases
    TestCase test_cases[] = {
        {
            "Single invalid flag '-x'",
            {"-x"},
            1
        },
        {
            "Single invalid flag '-z'",
            {"-z"},
            1
        },
        {
            "Single invalid flag '-?'",
            {"-?"},
            1
        },
        {
            "Invalid flag between valid flags '-l -x -p'",
            {"-l", "-x", "-p"},
            3
        },
        {
            "Multiple invalid flags '-x -y -z'",
            {"-x", "-y", "-z"},
            3
        },
        {
            "Invalid flag after valid flag '-l -x'",
            {"-l", "-x"},
            2
        },
        {
            "Invalid flag before valid flag '-x -l'",
            {"-x", "-l"},
            2
        },
        {
            "Invalid flag with filename 'test.gcda -x'",
            {"test.gcda", "-x"},
            2
        },
        {
            "Double dash with invalid flag '--x'",
            {"--x"},
            1
        },
        {
            "Mixed valid/invalid with filename '-l test.gcda -x'",
            {"-l", "test.gcda", "-x"},
            3
        },
        {
            "Combination flag with invalid char '-lxpr'",
            {"-lxpr"},
            1
        }
    };
    
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int passed_tests = 0;
    int failed_tests = 0;
    
    // Run all test cases
    for (int i = 0; i < num_tests; i++) {
        printf("\nTest %d/%d: %s\n", i + 1, num_tests, test_cases[i].description);
        printf("Arguments: ");
        for (int j = 0; j < test_cases[i].arg_count; j++) {
            printf("%s ", test_cases[i].args[j]);
        }
        printf("\n");
        
        int result = test_gcov_dump(gcov_dump_path, 
                                   test_cases[i].args, 
                                   test_cases[i].arg_count);
        
        if (result == 1) {
            passed_tests++;
            printf("Result: PASS\n");
        } else if (result == 0) {
            failed_tests++;
            printf("Result: FAIL (no error message)\n");
        } else {
            failed_tests++;
            printf("Result: ERROR (execution failed)\n");
        }
    }
    
    // Cleanup
    free(gcov_dump_path);
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", num_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", failed_tests);
    
    // Create a simple test file if needed (optional)
    printf("\nCreating dummy test file for additional tests...\n");
    FILE* test_file = fopen("dummy_test.gcda", "w");
    if (test_file) {
        fprintf(test_file, "Dummy GCov data file for testing\n");
        fclose(test_file);
        
        // Test with the dummy file
        printf("\nAdditional test with dummy file: '-x dummy_test.gcda'\n");
        const char* extra_args[] = {"-x", "dummy_test.gcda"};
        test_gcov_dump(gcov_dump_path, extra_args, 2);
        
        // Clean up dummy file
        remove("dummy_test.gcda");
    }
    
    return (passed_tests > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
