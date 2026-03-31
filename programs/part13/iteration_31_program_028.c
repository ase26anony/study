#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEST_SOURCE "test.c"
#define TEST_BINARY "test"
#define TEST_GCNO "test.gcno"

// Create a minimal test C source file
void create_test_source(void) {
    FILE *f = fopen(TEST_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source file");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

// Compile the test source with coverage flags to generate .gcno file
int compile_with_coverage(void) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        // Child process: compile with coverage
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
        perror("execvp gcc failed");
        exit(EXIT_FAILURE);
    }
    
    // Parent process: wait for compilation
    int status;
    waitpid(pid, &status, 0);
    
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("Successfully compiled test program with coverage\n");
        return 0;
    } else {
        printf("Failed to compile test program\n");
        return -1;
    }
}

// Run gcov-dump with specified arguments and check exit code
int run_gcov_dump(const char *description, char *const args[], int expected_exit) {
    printf("\n=== Testing: %s ===\n", description);
    printf("Command: gcov-dump");
    for (int i = 0; args[i] != NULL; i++) {
        printf(" %s", args[i]);
    }
    printf("\n");
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        // Child process: execute gcov-dump
        execvp("gcov-dump", args);
        
        // Try alternative paths if execvp fails
        char *paths[] = {"./gcov-dump", "/usr/bin/gcov-dump", NULL};
        for (int i = 0; paths[i] != NULL; i++) {
            char *new_args[32];
            int j;
            for (j = 0; args[j] != NULL && j < 30; j++) {
                new_args[j] = args[j];
            }
            new_args[j] = NULL;
            
            // Replace first argument with path
            new_args[0] = paths[i];
            execv(paths[i], new_args);
        }
        
        perror("execvp gcov-dump failed");
        exit(EXIT_FAILURE);
    }
    
    // Parent process: wait for gcov-dump
    int status;
    waitpid(pid, &status, 0);
    
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        printf("Exit code: %d (expected: %d)\n", exit_code, expected_exit);
        
        if (exit_code == expected_exit) {
            printf("✓ PASS\n");
            return 0;
        } else {
            printf("✗ FAIL - Wrong exit code\n");
            return -1;
        }
    } else if (WIFSIGNALED(status)) {
        printf("✗ FAIL - Process terminated by signal %d\n", WTERMSIG(status));
        return -1;
    } else {
        printf("✗ FAIL - Process didn't exit normally\n");
        return -1;
    }
}

int main(void) {
    printf("=== GCOV-DUMP Test Harness ===\n");
    
    // Check if gcov-dump exists
    if (access("gcov-dump", X_OK) != 0 && 
        access("/usr/bin/gcov-dump", X_OK) != 0) {
        printf("Error: gcov-dump not found in current directory or /usr/bin/\n");
        printf("Please ensure gcov-dump is in PATH or current directory\n");
        return EXIT_FAILURE;
    }
    
    // Create test source and compile with coverage
    printf("\nCreating test source file...\n");
    create_test_source();
    
    printf("Compiling with coverage flags...\n");
    if (compile_with_coverage() != 0) {
        printf("Failed to compile test program. Cleaning up...\n");
        unlink(TEST_SOURCE);
        return EXIT_FAILURE;
    }
    
    // Check if .gcno file was created
    if (access(TEST_GCNO, R_OK) != 0) {
        printf("Error: %s not found after compilation\n", TEST_GCNO);
        printf("Compilation may not have generated coverage data\n");
        unlink(TEST_SOURCE);
        unlink(TEST_BINARY);
        return EXIT_FAILURE;
    }
    
    int passed = 0;
    int total = 0;
    
    // Test cases
    char *args_h[] = {"gcov-dump", "-h", NULL};
    if (run_gcov_dump("Help flag (-h)", args_h, 0) == 0) passed++;
    total++;
    
    char *args_v[] = {"gcov-dump", "-v", NULL};
    if (run_gcov_dump("Version flag (-v)", args_v, 0) == 0) passed++;
    total++;
    
    char *args_l[] = {"gcov-dump", "-l", TEST_GCNO, NULL};
    if (run_gcov_dump("Dump contents flag (-l)", args_l, 0) == 0) passed++;
    total++;
    
    char *args_p[] = {"gcov-dump", "-p", TEST_GCNO, NULL};
    if (run_gcov_dump("Dump positions flag (-p)", args_p, 0) == 0) passed++;
    total++;
    
    char *args_r[] = {"gcov-dump", "-r", TEST_GCNO, NULL};
    if (run_gcov_dump("Dump raw flag (-r)", args_r, 0) == 0) passed++;
    total++;
    
    char *args_s[] = {"gcov-dump", "-s", TEST_GCNO, NULL};
    if (run_gcov_dump("Dump stable flag (-s)", args_s, 0) == 0) passed++;
    total++;
    
    char *args_x[] = {"gcov-dump", "-x", NULL};
    if (run_gcov_dump("Unknown flag (-x) - should fail", args_x, 1) == 0) passed++;
    total++;
    
    // Test with another unknown flag
    char *args_z[] = {"gcov-dump", "-Z", NULL};
    if (run_gcov_dump("Unknown flag (-Z) - should fail", args_z, 1) == 0) passed++;
    total++;
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d tests\n", passed, total);
    
    // Cleanup
    printf("\nCleaning up temporary files...\n");
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    unlink(TEST_GCNO);
    
    return (passed == total) ? EXIT_SUCCESS : EXIT_FAILURE;
}
