#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define TEST_SOURCE "test_coverage.c"
#define TEST_BINARY "test_coverage"
#define GCOV_DUMP_PATH "./gcov-dump"

void run_gcov_dump(const char **args, int expected_exit, const char *test_name) {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        execvp(GCOV_DUMP_PATH, (char *const *)args);
        // If execvp returns, it failed
        fprintf(stderr, "Failed to execute %s: %s\n", GCOV_DUMP_PATH, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == expected_exit) {
                printf("✓ %s: PASSED (exit code %d)\n", test_name, exit_code);
            } else {
                printf("✗ %s: FAILED - expected %d, got %d\n", 
                       test_name, expected_exit, exit_code);
            }
        } else {
            printf("✗ %s: FAILED - child did not exit normally\n", test_name);
        }
    } else {
        fprintf(stderr, "Fork failed: %s\n", strerror(errno));
    }
}

int create_test_gcno() {
    // Create minimal C source file
    FILE *fp = fopen(TEST_SOURCE, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return 0;
    }
    
    fprintf(fp, "int main() { return 0; }\n");
    fclose(fp);
    
    // Compile with coverage flags
    char compile_cmd[256];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -O0 --coverage -fprofile-arcs -ftest-coverage -o %s %s",
             TEST_BINARY, TEST_SOURCE);
    
    int result = system(compile_cmd);
    if (result != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return 0;
    }
    
    // Check if .gcno file was created
    if (access("test_coverage.gcno", F_OK) != 0) {
        fprintf(stderr, "No .gcno file generated\n");
        return 0;
    }
    
    return 1;
}

void cleanup() {
    // Remove generated files
    remove(TEST_SOURCE);
    remove(TEST_BINARY);
    remove("test_coverage.gcno");
    remove("test_coverage.gcda");
}

int main() {
    printf("=== Testing gcov-dump uncovered lines ===\n\n");
    
    // Check if gcov-dump exists
    if (access(GCOV_DUMP_PATH, X_OK) != 0) {
        fprintf(stderr, "Error: %s not found or not executable\n", GCOV_DUMP_PATH);
        fprintf(stderr, "Please ensure gcov-dump is in the current directory\n");
        return EXIT_FAILURE;
    }
    
    // Create test .gcno file
    printf("Creating test coverage data...\n");
    if (!create_test_gcno()) {
        fprintf(stderr, "Failed to create test .gcno file\n");
        return EXIT_FAILURE;
    }
    printf("Test .gcno file created successfully\n\n");
    
    // Test cases
    const char *help_args[] = {GCOV_DUMP_PATH, "-h", NULL};
    run_gcov_dump(help_args, 0, "Help flag (-h)");
    
    const char *version_args[] = {GCOV_DUMP_PATH, "-v", NULL};
    run_gcov_dump(version_args, 0, "Version flag (-v)");
    
    const char *dump_contents_args[] = {GCOV_DUMP_PATH, "-l", "test_coverage.gcno", NULL};
    run_gcov_dump(dump_contents_args, 0, "Dump contents flag (-l)");
    
    const char *dump_positions_args[] = {GCOV_DUMP_PATH, "-p", "test_coverage.gcno", NULL};
    run_gcov_dump(dump_positions_args, 0, "Dump positions flag (-p)");
    
    const char *dump_raw_args[] = {GCOV_DUMP_PATH, "-r", "test_coverage.gcno", NULL};
    run_gcov_dump(dump_raw_args, 0, "Dump raw flag (-r)");
    
    const char *dump_stable_args[] = {GCOV_DUMP_PATH, "-s", "test_coverage.gcno", NULL};
    run_gcov_dump(dump_stable_args, 0, "Dump stable flag (-s)");
    
    // Test unknown flag - should exit with non-zero
    const char *unknown_args[] = {GCOV_DUMP_PATH, "-x", NULL};
    run_gcov_dump(unknown_args, 1, "Unknown flag (-x)");
    
    // Test another unknown flag
    const char *unknown_args2[] = {GCOV_DUMP_PATH, "-Z", NULL};
    run_gcov_dump(unknown_args2, 1, "Unknown flag (-Z)");
    
    // Test with question mark (special case)
    const char *unknown_args3[] = {GCOV_DUMP_PATH, "-?", NULL};
    run_gcov_dump(unknown_args3, 1, "Unknown flag (-?)");
    
    printf("\n=== All tests completed ===\n");
    
    // Cleanup
    cleanup();
    
    return EXIT_SUCCESS;
}
