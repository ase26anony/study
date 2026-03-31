/**
 * test_gcov_dump_invalid_flags.c
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
#define TARGET_ERROR_MSG "unknown flag"

/**
 * Find the gcov-dump executable path.
 * Priority: 1. GCov_DUMP environment variable
 *           2. Common build locations
 *           3. System PATH
 */
static int find_gcov_dump(char *path, size_t path_len) {
    const char *env_path = getenv("GCov_DUMP");
    const char *candidates[] = {
        env_path,
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; candidates[i] != NULL; i++) {
        if (candidates[i] && access(candidates[i], X_OK) == 0) {
            strncpy(path, candidates[i], path_len - 1);
            path[path_len - 1] = '\0';
            return 0;
        }
    }
    
    // Try to find in PATH
    FILE *fp = popen("which gcov-dump 2>/dev/null", "r");
    if (fp) {
        if (fgets(path, path_len, fp)) {
            // Remove trailing newline
            path[strcspn(path, "\n")] = '\0';
            pclose(fp);
            if (access(path, X_OK) == 0) {
                return 0;
            }
        }
        pclose(fp);
    }
    
    return -1;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 0 on success (found target error), -1 on execution failure.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
    int pipefd[2];
    pid_t pid;
    char output[MAX_OUTPUT_LEN] = {0};
    int status;
    
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
        close(pipefd[0]);  // Close read end
        
        // Redirect stderr to pipe
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        // Prepare arguments
        char *args[] = {
            (char *)gcov_dump_path,
            (char *)flag,
            NULL
        };
        
        execvp(gcov_dump_path, args);
        
        // If we get here, exec failed
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else {  // Parent process
        close(pipefd[1]);  // Close write end
        
        // Read stderr output
        ssize_t bytes_read = read(pipefd[0], output, sizeof(output) - 1);
        close(pipefd[0]);
        
        waitpid(pid, &status, 0);
        
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
            
            // Check for target error message
            if (strstr(output, TARGET_ERROR_MSG) != NULL) {
                printf("✓ Found target error for flag '%s':\n", flag);
                printf("  %s", output);
                return 0;
            } else {
                printf("✗ No target error for flag '%s' (output: %s)\n", 
                       flag, output[0] ? output : "<empty>");
                return -1;
            }
        } else {
            printf("✗ No output for flag '%s'\n", flag);
            return -1;
        }
    }
}

/**
 * Test multiple invalid flags in different positions.
 */
static int run_comprehensive_tests(const char *gcov_dump_path) {
    struct test_case {
        const char *description;
        const char *args[8];  // NULL-terminated array
    };
    
    // Test cases covering different positions and combinations
    struct test_case tests[] = {
        // Single invalid flags
        {"Single invalid flag -x", {"-x", NULL}},
        {"Single invalid flag -z", {"-z", NULL}},
        {"Single invalid flag -?", {"-?", NULL}},
        
        // Invalid flag as first argument
        {"Invalid flag first, then valid", {"-x", "-l", NULL}},
        
        // Invalid flag between valid flags
        {"Valid flag, invalid, then valid", {"-l", "-x", "-p", NULL}},
        
        // Multiple invalid flags
        {"Multiple invalid flags", {"-x", "-y", "-z", NULL}},
        
        // Invalid flag after double dash (getopt behavior)
        {"Double dash with invalid flag", {"--", "-x", NULL}},
        
        // Invalid flag after filename (non-option argument)
        {"Invalid flag after filename", {"-l", "dummy.gcda", "-x", NULL}},
        
        // Mixed valid and invalid in single argument (not typical getopt)
        {"Combined flags with invalid", {"-lxz", NULL}},
    };
    
    int success_count = 0;
    int total_tests = sizeof(tests) / sizeof(tests[0]);
    
    printf("\n=== Running comprehensive invalid flag tests ===\n\n");
    
    for (int i = 0; i < total_tests; i++) {
        printf("Test %d: %s\n", i + 1, tests[i].description);
        
        // Build command string for popen
        char command[MAX_PATH_LEN * 2] = {0};
        strcat(command, gcov_dump_path);
        
        for (int j = 0; tests[i].args[j] != NULL; j++) {
            strcat(command, " ");
            strcat(command, tests[i].args[j]);
        }
        
        // Use popen to capture both stdout and stderr
        char full_command[MAX_PATH_LEN * 3];
        snprintf(full_command, sizeof(full_command), "%s 2>&1", command);
        
        FILE *fp = popen(full_command, "r");
        if (!fp) {
            printf("  ✗ Failed to execute command\n");
            continue;
        }
        
        char output[MAX_OUTPUT_LEN] = {0};
        size_t total_bytes = 0;
        
        while (!feof(fp) && total_bytes < sizeof(output) - 1) {
            total_bytes += fread(output + total_bytes, 1, 
                                sizeof(output) - 1 - total_bytes, fp);
        }
        output[total_bytes] = '\0';
        
        int status = pclose(fp);
        
        // Check for target error message
        if (strstr(output, TARGET_ERROR_MSG) != NULL) {
            printf("  ✓ Triggered default case\n");
            success_count++;
        } else {
            printf("  ✗ Did not trigger default case\n");
            if (strlen(output) > 0) {
                printf("    Output: %s\n", output);
            }
        }
    }
    
    printf("\n=== Test Results ===\n");
    printf("Successfully triggered default case: %d/%d tests\n", 
           success_count, total_tests);
    
    return (success_count > 0) ? 0 : -1;
}

int main(void) {
    char gcov_dump_path[MAX_PATH_LEN];
    
    printf("=== Testing gcov-dump invalid flag handling ===\n\n");
    
    // Find gcov-dump executable
    if (find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path)) != 0) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Found gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Test basic invalid flags
    printf("=== Testing basic invalid flags ===\n");
    
    const char *basic_flags[] = {"-x", "-z", "-?", "-X", "-!", NULL};
    int basic_success = 0;
    
    for (int i = 0; basic_flags[i] != NULL; i++) {
        if (test_invalid_flag(gcov_dump_path, basic_flags[i]) == 0) {
            basic_success++;
        }
    }
    
    printf("\nBasic tests: %d invalid flags triggered the default case\n\n",
           basic_success);
    
    // Run comprehensive tests
    if (run_comprehensive_tests(gcov_dump_path) == 0) {
        printf("\n✅ SUCCESS: Successfully triggered the uncovered default case\n");
        return EXIT_SUCCESS;
    } else {
        printf("\n❌ FAILURE: Could not trigger the uncovered default case\n");
        return EXIT_FAILURE;
    }
}
