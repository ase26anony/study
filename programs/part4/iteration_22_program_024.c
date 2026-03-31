/**
 * test_gcov_dump_default_case.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by invoking
 * gcov-dump with invalid command-line flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_CMD_LEN 1024
#define MAX_OUTPUT_LEN 4096

/**
 * Get the path to gcov-dump executable.
 * Checks GCov_DUMP environment variable first, then tries common paths.
 */
static const char *get_gcov_dump_path(void) {
    const char *env_path = getenv("GCov_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        return env_path;
    }
    
    // Try common paths where gcov-dump might be found
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            return common_paths[i];
        }
    }
    
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if "unknown flag" message is found in stderr, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *invalid_flag) {
    char cmd[MAX_CMD_LEN];
    char output[MAX_OUTPUT_LEN] = {0};
    FILE *fp;
    int found = 0;
    
    // Construct command to capture stderr (2>&1)
    snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, invalid_flag);
    
    // Execute command and capture output
    fp = popen(cmd, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to execute command: %s\n", cmd);
        return 0;
    }
    
    // Read output
    while (fgets(output, sizeof(output) - 1, fp) != NULL) {
        // Check for the target error message
        if (strstr(output, "unknown flag") != NULL) {
            found = 1;
            printf("Found target message for flag '%s': %s", invalid_flag, output);
        }
    }
    
    pclose(fp);
    return found;
}

/**
 * Test various invalid flag scenarios.
 */
static void run_tests(const char *gcov_dump_path) {
    struct test_case {
        const char *description;
        const char *flag;
    };
    
    // Test cases covering different scenarios
    struct test_case tests[] = {
        {"Single invalid flag as first argument", "-x"},
        {"Another single invalid flag", "-z"},
        {"Invalid flag with question mark", "-?"},
        {"Invalid flag between valid flags", "-l -x -p"},
        {"Invalid flag after valid flag", "-v -q"},
        {"Invalid flag before filename", "-x dummy.gcda"},
        {"Multiple invalid flags", "-a -b -c"},
        {"Invalid flag with double dash", "--x"},
        {"Mixed case invalid flag", "-X"},
        {"Invalid flag with numeric", "-1"},
        {"Invalid flag at end of valid sequence", "-l -p -r -s -k"},
        {"Just invalid flag with no other args", "-@"},
        {NULL, NULL}
    };
    
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    int total_tests = 0;
    int passed_tests = 0;
    
    for (int i = 0; tests[i].description != NULL; i++) {
        printf("Test %d: %s\n", i + 1, tests[i].description);
        printf("  Command: %s %s\n", gcov_dump_path, tests[i].flag);
        
        if (test_invalid_flag(gcov_dump_path, tests[i].flag)) {
            printf("  ✓ PASSED - Unknown flag detected\n\n");
            passed_tests++;
        } else {
            printf("  ✗ FAILED - Unknown flag not detected\n\n");
        }
        
        total_tests++;
    }
    
    // Additional test: Execute with execvp to test exit status
    printf("Testing exit status with invalid flag...\n");
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        char *args[] = {(char *)gcov_dump_path, "-x", NULL};
        execvp(gcov_dump_path, args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            printf("✓ PASSED - Program exited with non-zero status (%d)\n", 
                   WEXITSTATUS(status));
            passed_tests++;
        } else {
            printf("✗ FAILED - Program did not exit with non-zero status\n");
        }
        total_tests++;
    }
    
    printf("\n========================================\n");
    printf("Test Results: %d/%d tests passed\n", passed_tests, total_tests);
    printf("========================================\n");
}

int main(void) {
    const char *gcov_dump_path = get_gcov_dump_path();
    
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCov_DUMP environment variable to the path of gcov-dump,\n");
        fprintf(stderr, "or ensure it's in one of the common locations.\n");
        
        // Try to compile gcov-dump if we have the source
        printf("\nAttempting to compile gcov-dump from source...\n");
        system("cd .. && make gcov-dump 2>/dev/null");
        
        gcov_dump_path = get_gcov_dump_path();
        if (gcov_dump_path == NULL) {
            return EXIT_FAILURE;
        }
        printf("Found gcov-dump after compilation: %s\n", gcov_dump_path);
    }
    
    run_tests(gcov_dump_path);
    
    return EXIT_SUCCESS;
}
