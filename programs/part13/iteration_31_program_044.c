#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEST_SOURCE "test.c"
#define TEST_BINARY "test"
#define TEST_GCNO "test.gcno"
#define GCOV_DUMP_PATH "./gcov-dump"

void run_gcov_dump(const char *args[], int expected_exit, const char *description) {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        execvp(args[0], (char * const *)args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == expected_exit) {
                printf("✓ %s: PASSED (exit code %d)\n", description, exit_code);
            } else {
                printf("✗ %s: FAILED (expected %d, got %d)\n", 
                       description, expected_exit, exit_code);
            }
        } else {
            printf("✗ %s: FAILED (child did not exit normally)\n", description);
        }
    } else {
        perror("fork failed");
    }
}

int create_test_source() {
    FILE *f = fopen(TEST_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source");
        return 0;
    }
    
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
    return 1;
}

int compile_with_coverage() {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Compile with coverage flags to generate .gcno file
        char *args[] = {
            "gcc",
            "-O0",
            "--coverage",
            "-fprofile-arcs",
            "-ftest-coverage",
            "-o", TEST_BINARY,
            TEST_SOURCE,
            NULL
        };
        
        execvp("gcc", args);
        perror("gcc exec failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) && WEXITSTATUS(status) == 0;
    }
    
    return 0;
}

int main() {
    printf("=== GCOV-DUMP Test Harness ===\n\n");
    
    // Check if gcov-dump exists
    if (access(GCOV_DUMP_PATH, X_OK) != 0) {
        fprintf(stderr, "Error: %s not found or not executable\n", GCOV_DUMP_PATH);
        fprintf(stderr, "Please ensure gcov-dump is in the current directory\n");
        return EXIT_FAILURE;
    }
    
    // Create test source file
    printf("Creating test source file...\n");
    if (!create_test_source()) {
        return EXIT_FAILURE;
    }
    
    // Compile with coverage to generate .gcno file
    printf("Compiling with coverage flags...\n");
    if (!compile_with_coverage()) {
        fprintf(stderr, "Failed to compile test program with coverage\n");
        return EXIT_FAILURE;
    }
    
    printf("\nRunning test cases...\n");
    
    // Test 1: -h flag (help)
    {
        const char *args[] = {GCOV_DUMP_PATH, "-h", NULL};
        run_gcov_dump(args, 0, "Test -h flag (help)");
    }
    
    // Test 2: -v flag (version)
    {
        const char *args[] = {GCOV_DUMP_PATH, "-v", NULL};
        run_gcov_dump(args, 0, "Test -v flag (version)");
    }
    
    // Test 3: -l flag (dump contents)
    {
        const char *args[] = {GCOV_DUMP_PATH, "-l", TEST_GCNO, NULL};
        run_gcov_dump(args, 0, "Test -l flag (dump contents)");
    }
    
    // Test 4: -p flag (dump positions)
    {
        const char *args[] = {GCOV_DUMP_PATH, "-p", TEST_GCNO, NULL};
        run_gcov_dump(args, 0, "Test -p flag (dump positions)");
    }
    
    // Test 5: -r flag (dump raw)
    {
        const char *args[] = {GCOV_DUMP_PATH, "-r", TEST_GCNO, NULL};
        run_gcov_dump(args, 0, "Test -r flag (dump raw)");
    }
    
    // Test 6: -s flag (dump stable)
    {
        const char *args[] = {GCOV_DUMP_PATH, "-s", TEST_GCNO, NULL};
        run_gcov_dump(args, 0, "Test -s flag (dump stable)");
    }
    
    // Test 7: Invalid flag (should trigger default case)
    {
        const char *args[] = {GCOV_DUMP_PATH, "-x", NULL};
        run_gcov_dump(args, 1, "Test invalid -x flag (should fail)");
    }
    
    // Test 8: Another invalid flag
    {
        const char *args[] = {GCOV_DUMP_PATH, "-Z", NULL};
        run_gcov_dump(args, 1, "Test invalid -Z flag (should fail)");
    }
    
    // Test 9: Yet another invalid flag
    {
        const char *args[] = {GCOV_DUMP_PATH, "-?", NULL};
        run_gcov_dump(args, 1, "Test invalid -? flag (should fail)");
    }
    
    printf("\n=== Test Complete ===\n");
    
    // Cleanup
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    unlink(TEST_GCNO);
    
    return EXIT_SUCCESS;
}
