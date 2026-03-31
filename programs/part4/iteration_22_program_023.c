/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by passing invalid
 * command-line flags and verifying the error message.
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
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "./gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "gcov-dump",  // Try PATH
        NULL
    };
    
    // Check environment variable first
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, path_len - 1);
        path[path_len - 1] = '\0';
        return 0;
    }
    
    // Check common build locations
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path, common_paths[i], path_len - 1);
            path[path_len - 1] = '\0';
            return 0;
        }
    }
    
    // Last resort: try which/where command
    FILE *fp = popen("which gcov-dump 2>/dev/null", "r");
    if (fp != NULL) {
        if (fgets(path, path_len, fp) != NULL) {
            // Remove trailing newline
            path[strcspn(path, "\n")] = '\0';
            if (strlen(path) > 0 && access(path, X_OK) == 0) {
                pclose(fp);
                return 0;
            }
        }
        pclose(fp);
    }
    
    return -1; // Not found
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 0 on success (found target error), -1 on execution failure.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
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
    
    if (pid == 0) {  // Child process
        // Close read end of pipe
        close(pipefd[0]);
        
        // Redirect stderr to pipe
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        // Prepare arguments
        char *args[] = {
            (char *)gcov_dump_path,
            (char *)flag,
            NULL
        };
        
        // Execute gcov-dump
        execvp(gcov_dump_path, args);
        
        // If we get here, exec failed
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else {  // Parent process
        // Close write end of pipe
        close(pipefd[1]);
        
        // Read stderr output from pipe
        ssize_t bytes_read = read(pipefd[0], output, sizeof(output) - 1);
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
            
            // Check for target error message
            if (strstr(output, TARGET_ERROR_MSG) != NULL) {
                found_target = 1;
                printf("✓ Found target error for flag '%s':\n", flag);
                printf("  %s", output);
            } else {
                printf("✗ No target error for flag '%s':\n", flag);
                printf("  %s", output);
            }
        } else {
            printf("✗ No output for flag '%s'\n", flag);
        }
        
        close(pipefd[0]);
        
        // Wait for child to finish
        int status;
        waitpid(pid, &status, 0);
        
        return found_target ? 0 : -1;
    }
}

/**
 * Test multiple invalid flags in different positions.
 */
static void run_comprehensive_tests(const char *gcov_dump_path) {
    printf("\n=== Testing Invalid Flags ===\n");
    
    // Test single invalid flags
    const char *invalid_flags[] = {
        "-x",        // Simple invalid flag
        "-z",        // Another invalid flag
        "-?",        // Question mark (not in switch)
        "-X",        // Uppercase invalid flag
        "-1",        // Number flag
        "-@",        // Symbol flag
        NULL
    };
    
    int success_count = 0;
    int total_tests = 0;
    
    for (int i = 0; invalid_flags[i] != NULL; i++) {
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, invalid_flags[i]) == 0) {
            success_count++;
        }
    }
    
    // Test combinations with valid flags
    printf("\n=== Testing Flag Combinations ===\n");
    
    const char *flag_combinations[] = {
        "-l -x -p",           // Invalid flag between valid ones
        "-x -l -p",           // Invalid flag first
        "-l -p -x",           // Invalid flag last
        "-x -y -z",           // Multiple invalid flags
        "-l -x -p -r -s -z",  // Mix of valid and invalid
        "--x",                // Double dash with single char (edge case)
        "-x -?",              // Multiple different invalid flags
        NULL
    };
    
    for (int i = 0; flag_combinations[i] != NULL; i++) {
        total_tests++;
        // Build command with multiple flags
        char cmd[MAX_PATH_LEN + 50];
        snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, flag_combinations[i]);
        
        FILE *fp = popen(cmd, "r");
        if (fp != NULL) {
            char output[MAX_OUTPUT_LEN] = {0};
            if (fgets(output, sizeof(output) - 1, fp) != NULL) {
                if (strstr(output, TARGET_ERROR_MSG) != NULL) {
                    success_count++;
                    printf("✓ Found target error for combination '%s':\n", flag_combinations[i]);
                    printf("  %s", output);
                } else {
                    printf("✗ No target error for combination '%s':\n", flag_combinations[i]);
                    printf("  %s", output);
                }
            }
            pclose(fp);
        }
    }
    
    // Test with filename argument (non-option argument)
    printf("\n=== Testing with Filename Argument ===\n");
    
    // Create a dummy file to pass as argument
    FILE *tmp = fopen("dummy.gcno", "w");
    if (tmp != NULL) {
        fprintf(tmp, "dummy coverage data\n");
        fclose(tmp);
        
        const char *filename_tests[] = {
            "-x dummy.gcno",      // Invalid flag before filename
            "dummy.gcno -x",      // Invalid flag after filename
            "-l dummy.gcno -x",   // Invalid flag after filename with valid flag
            NULL
        };
        
        for (int i = 0; filename_tests[i] != NULL; i++) {
            total_tests++;
            char cmd[MAX_PATH_LEN + 50];
            snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, filename_tests[i]);
            
            FILE *fp = popen(cmd, "r");
            if (fp != NULL) {
                char output[MAX_OUTPUT_LEN] = {0};
                if (fgets(output, sizeof(output) - 1, fp) != NULL) {
                    if (strstr(output, TARGET_ERROR_MSG) != NULL) {
                        success_count++;
                        printf("✓ Found target error for '%s':\n", filename_tests[i]);
                        printf("  %s", output);
                    } else {
                        printf("✗ No target error for '%s':\n", filename_tests[i]);
                        printf("  %s", output);
                    }
                }
                pclose(fp);
            }
        }
        
        // Clean up dummy file
        remove("dummy.gcno");
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Successful (triggered default case): %d\n", success_count);
    printf("Failed: %d\n", total_tests - success_count);
    
    if (success_count > 0) {
        printf("\n✅ SUCCESS: Triggered the uncovered default case!\n");
    } else {
        printf("\n❌ FAILURE: Could not trigger the uncovered default case\n");
    }
}

int main(int argc, char *argv[]) {
    char gcov_dump_path[MAX_PATH_LEN];
    
    printf("=== gcov-dump Invalid Flag Tester ===\n");
    
    // Find gcov-dump executable
    if (find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path)) != 0) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Found gcov-dump at: %s\n", gcov_dump_path);
    
    // Run comprehensive tests
    run_comprehensive_tests(gcov_dump_path);
    
    return EXIT_SUCCESS;
}
