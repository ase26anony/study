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
static char *find_gcov_dump_path() {
    static char path[MAX_PATH_LEN];
    
    // 1. Check environment variable
    char *env_path = getenv("GCov_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
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
    
    // 3. Search in PATH
    char *path_env = getenv("PATH");
    if (path_env != NULL) {
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
 * Returns 0 on success (found target error), 1 on failure.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
    printf("Testing invalid flag: %s\n", flag);
    
    // Create pipe for capturing stderr
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return 1;
    }
    
    if (pid == 0) {
        // Child process
        close(pipefd[0]);  // Close read end
        
        // Redirect stderr to pipe
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        // Execute gcov-dump with invalid flag
        execl(gcov_dump_path, "gcov-dump", flag, NULL);
        
        // If we get here, exec failed
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        close(pipefd[1]);  // Close write end
        
        // Read stderr output from pipe
        char buffer[MAX_OUTPUT_LEN];
        ssize_t bytes_read = read(pipefd[0], buffer, MAX_OUTPUT_LEN - 1);
        close(pipefd[0]);
        
        // Wait for child
        int status;
        waitpid(pid, &status, 0);
        
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            
            // Check if output contains target error message
            if (strstr(buffer, TARGET_ERROR_MSG) != NULL) {
                printf("  ✓ Success: Found '%s' in output\n", TARGET_ERROR_MSG);
                printf("  Output: %s", buffer);
                return 0;
            } else {
                printf("  ✗ Failure: Target message not found\n");
                printf("  Output: %s", buffer);
                return 1;
            }
        } else {
            printf("  ✗ Failure: No output captured\n");
            return 1;
        }
    }
}

/**
 * Test multiple invalid flags in different positions.
 */
static int test_flag_combinations(const char *gcov_dump_path) {
    int success_count = 0;
    int total_tests = 0;
    
    // Single invalid flags
    const char *invalid_flags[] = {
        "-x",    // Simple invalid flag
        "-z",    // Another invalid flag
        "-?",    // Question mark (not in switch)
        "-X",    // Uppercase invalid flag
        "-1",    // Number flag
        "-@",    // Special character
        NULL
    };
    
    printf("\n=== Testing single invalid flags ===\n");
    for (int i = 0; invalid_flags[i] != NULL; i++) {
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, invalid_flags[i]) == 0) {
            success_count++;
        }
    }
    
    // Flag combinations
    printf("\n=== Testing flag combinations ===\n");
    
    // Invalid flag as first argument
    total_tests++;
    printf("Testing: -x -l -p (invalid first)\n");
    if (system_test(gcov_dump_path, "-x -l -p") == 0) success_count++;
    
    // Invalid flag in middle
    total_tests++;
    printf("Testing: -l -x -p (invalid middle)\n");
    if (system_test(gcov_dump_path, "-l -x -p") == 0) success_count++;
    
    // Invalid flag at end
    total_tests++;
    printf("Testing: -l -p -x (invalid end)\n");
    if (system_test(gcov_dump_path, "-l -p -x") == 0) success_count++;
    
    // Multiple invalid flags
    total_tests++;
    printf("Testing: -x -z -? (multiple invalid)\n");
    if (system_test(gcov_dump_path, "-x -z -?") == 0) success_count++;
    
    // Double dash with invalid flag (edge case)
    total_tests++;
    printf("Testing: --x (double dash)\n");
    if (system_test(gcov_dump_path, "--x") == 0) success_count++;
    
    // Invalid flag after filename (non-option argument)
    total_tests++;
    printf("Testing: -l dummy.gcda -x (invalid after filename)\n");
    if (system_test(gcov_dump_path, "-l dummy.gcda -x") == 0) success_count++;
    
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d tests\n", success_count, total_tests);
    
    return (success_count == total_tests) ? 0 : 1;
}

/**
 * Helper function using system() for complex argument combinations.
 */
static int system_test(const char *gcov_dump_path, const char *args) {
    char command[MAX_PATH_LEN + 100];
    char output_file[] = "/tmp/gcov_dump_test_XXXXXX";
    int fd;
    
    // Create temp file for output
    fd = mkstemp(output_file);
    if (fd == -1) {
        perror("mkstemp");
        return 1;
    }
    close(fd);
    
    // Build command: redirect stderr to temp file
    snprintf(command, sizeof(command), "%s %s 2> %s", 
             gcov_dump_path, args, output_file);
    
    int ret = system(command);
    
    // Read captured stderr
    FILE *fp = fopen(output_file, "r");
    if (fp != NULL) {
        char buffer[MAX_OUTPUT_LEN];
        size_t bytes_read = fread(buffer, 1, MAX_OUTPUT_LEN - 1, fp);
        buffer[bytes_read] = '\0';
        fclose(fp);
        
        if (strstr(buffer, TARGET_ERROR_MSG) != NULL) {
            printf("  ✓ Success: Found '%s' in output\n", TARGET_ERROR_MSG);
            unlink(output_file);
            return 0;
        }
    }
    
    unlink(output_file);
    printf("  ✗ Failure: Target message not found\n");
    return 1;
}

int main(int argc, char *argv[]) {
    printf("=== Testing gcov-dump invalid flag handling ===\n");
    
    // Find gcov-dump executable
    char *gcov_dump_path = find_gcov_dump_path();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Run comprehensive tests
    int result = test_flag_combinations(gcov_dump_path);
    
    if (result == 0) {
        printf("\n✅ All tests passed! Uncovered default case was triggered.\n");
    } else {
        printf("\n❌ Some tests failed.\n");
    }
    
    return result;
}
