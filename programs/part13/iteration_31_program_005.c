#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

#define TEST_SOURCE "test.c"
#define TEST_BINARY "test"
#define GCNO_FILE "test.gcno"
#define GCOV_DUMP_PATH "./gcov-dump"

void run_gcov_dump(const char **args, const char *test_name, int expect_success) {
    pid_t pid;
    int status;
    
    printf("Running: %s", test_name);
    for (int i = 0; args[i] != NULL; i++) {
        printf(" %s", args[i]);
    }
    printf("\n");
    
    pid = fork();
    if (pid == 0) {
        // Child process
        execvp(args[0], (char * const *)args);
        // If execvp returns, it failed
        fprintf(stderr, "Failed to execute %s: %s\n", args[0], strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (expect_success) {
                if (exit_code == 0) {
                    printf("  ✓ PASS: Exited successfully\n");
                } else {
                    printf("  ✗ FAIL: Expected success but got exit code %d\n", exit_code);
                }
            } else {
                if (exit_code != 0) {
                    printf("  ✓ PASS: Exited with error code %d (as expected)\n", exit_code);
                } else {
                    printf("  ✗ FAIL: Expected failure but got success\n");
                }
            }
        } else {
            printf("  ✗ FAIL: Process terminated abnormally\n");
        }
    } else {
        fprintf(stderr, "Fork failed: %s\n", strerror(errno));
    }
    printf("\n");
}

int create_test_gcno() {
    // Create minimal test source file
    FILE *fp = fopen(TEST_SOURCE, "w");
    if (!fp) {
        fprintf(stderr, "Failed to create %s\n", TEST_SOURCE);
        return 0;
    }
    fprintf(fp, "int main() { return 0; }\n");
    fclose(fp);
    
    // Compile with coverage flags to generate .gcno file
    printf("Creating test .gcno file...\n");
    pid_t pid = fork();
    if (pid == 0) {
        execlp("gcc", "gcc", "-O0", "--coverage", "-fprofile-arcs", 
               "-ftest-coverage", "-o", TEST_BINARY, TEST_SOURCE, NULL);
        fprintf(stderr, "Failed to compile test file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Compilation failed\n");
            return 0;
        }
    } else {
        fprintf(stderr, "Fork failed: %s\n", strerror(errno));
        return 0;
    }
    
    // Verify .gcno file was created
    if (access(GCNO_FILE, F_OK) != 0) {
        fprintf(stderr, "Failed to create %s\n", GCNO_FILE);
        return 0;
    }
    
    printf("Successfully created %s\n\n", GCNO_FILE);
    return 1;
}

void cleanup() {
    // Remove generated files
    remove(TEST_SOURCE);
    remove(TEST_BINARY);
    remove(GCNO_FILE);
    // Also remove any .gcda file that might have been created
    remove("test.gcda");
}

int main() {
    printf("=== GCOV-DUMP Test Harness ===\n\n");
    
    // Check if gcov-dump exists
    if (access(GCOV_DUMP_PATH, X_OK) != 0) {
        fprintf(stderr, "Error: %s not found or not executable\n", GCOV_DUMP_PATH);
        fprintf(stderr, "Please ensure gcov-dump is in the current directory\n");
        return EXIT_FAILURE;
    }
    
    // Create test .gcno file
    if (!create_test_gcno()) {
        cleanup();
        return EXIT_FAILURE;
    }
    
    // Test cases
    const char *help_args[] = {GCOV_DUMP_PATH, "-h", NULL};
    run_gcov_dump(help_args, "Help flag (-h)", 1);
    
    const char *version_args[] = {GCOV_DUMP_PATH, "-v", NULL};
    run_gcov_dump(version_args, "Version flag (-v)", 1);
    
    const char *dump_contents_args[] = {GCOV_DUMP_PATH, "-l", GCNO_FILE, NULL};
    run_gcov_dump(dump_contents_args, "Dump contents flag (-l)", 1);
    
    const char *dump_positions_args[] = {GCOV_DUMP_PATH, "-p", GCNO_FILE, NULL};
    run_gcov_dump(dump_positions_args, "Dump positions flag (-p)", 1);
    
    const char *dump_raw_args[] = {GCOV_DUMP_PATH, "-r", GCNO_FILE, NULL};
    run_gcov_dump(dump_raw_args, "Dump raw flag (-r)", 1);
    
    const char *dump_stable_args[] = {GCOV_DUMP_PATH, "-s", GCNO_FILE, NULL};
    run_gcov_dump(dump_stable_args, "Dump stable flag (-s)", 1);
    
    const char *unknown_flag_args[] = {GCOV_DUMP_PATH, "-x", NULL};
    run_gcov_dump(unknown_flag_args, "Unknown flag (-x)", 0);
    
    // Test another unknown flag
    const char *unknown_flag2_args[] = {GCOV_DUMP_PATH, "-Z", NULL};
    run_gcov_dump(unknown_flag2_args, "Unknown flag (-Z)", 0);
    
    // Test with question mark (special case)
    const char *unknown_flag3_args[] = {GCOV_DUMP_PATH, "-?", NULL};
    run_gcov_dump(unknown_flag3_args, "Unknown flag (-?)", 0);
    
    // Cleanup
    cleanup();
    
    printf("=== Test Complete ===\n");
    return EXIT_SUCCESS;
}
