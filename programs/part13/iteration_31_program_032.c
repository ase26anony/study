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

void create_test_source(void) {
    FILE *f = fopen(TEST_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source file");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

void compile_test_with_coverage(void) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: compile with coverage
        char *args[] = {"gcc", "-O0", "--coverage", "-fprofile-arcs", 
                       "-ftest-coverage", "-o", TEST_BINARY, TEST_SOURCE, NULL};
        execvp("gcc", args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent: wait for compilation
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Failed to compile test program with coverage\n");
            exit(EXIT_FAILURE);
        }
        printf("Generated %s successfully\n", GCNO_FILE);
    } else {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
}

int run_gcov_dump(const char *gcov_dump_path, char *const args[], 
                  int expect_success, const char *description) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: run gcov-dump
        execvp(gcov_dump_path, (char *const*)args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent: wait and check result
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            int success = (exit_code == 0);
            
            if (expect_success) {
                if (success) {
                    printf("✓ %s: PASSED (exit code: %d)\n", description, exit_code);
                    return 1;
                } else {
                    printf("✗ %s: FAILED - expected success but got exit code %d\n", 
                           description, exit_code);
                    return 0;
                }
            } else {
                if (!success) {
                    printf("✓ %s: PASSED (exit code: %d)\n", description, exit_code);
                    return 1;
                } else {
                    printf("✗ %s: FAILED - expected failure but got exit code %d\n", 
                           description, exit_code);
                    return 0;
                }
            }
        } else {
            printf("✗ %s: FAILED - process didn't exit normally\n", description);
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

int main(int argc, char *argv[]) {
    const char *gcov_dump_path = NULL;
    
    // Try to get gcov-dump path from environment or use default
    gcov_dump_path = getenv("GCOV_DUMP");
    if (!gcov_dump_path) {
        gcov_dump_path = "gcov-dump";  // Assume it's in PATH
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Create test source file and compile with coverage
    printf("Creating test files...\n");
    create_test_source();
    compile_test_with_coverage();
    printf("\n");
    
    int passed = 0;
    int total = 0;
    
    // Test cases
    printf("Running test cases:\n");
    printf("===================\n");
    
    // 1. Test -h (help)
    {
        char *args[] = { "gcov-dump", "-h", NULL };
        passed += run_gcov_dump(gcov_dump_path, args, 1, "Test -h (help)");
        total++;
    }
    
    // 2. Test -v (version)
    {
        char *args[] = { "gcov-dump", "-v", NULL };
        passed += run_gcov_dump(gcov_dump_path, args, 1, "Test -v (version)");
        total++;
    }
    
    // 3. Test -l (dump contents)
    {
        char *args[] = { "gcov-dump", "-l", GCNO_FILE, NULL };
        passed += run_gcov_dump(gcov_dump_path, args, 1, "Test -l (dump contents)");
        total++;
    }
    
    // 4. Test -p (dump positions)
    {
        char *args[] = { "gcov-dump", "-p", GCNO_FILE, NULL };
        passed += run_gcov_dump(gcov_dump_path, args, 1, "Test -p (dump positions)");
        total++;
    }
    
    // 5. Test -r (dump raw)
    {
        char *args[] = { "gcov-dump", "-r", GCNO_FILE, NULL };
        passed += run_gcov_dump(gcov_dump_path, args, 1, "Test -r (dump raw)");
        total++;
    }
    
    // 6. Test -s (dump stable)
    {
        char *args[] = { "gcov-dump", "-s", GCNO_FILE, NULL };
        passed += run_gcov_dump(gcov_dump_path, args, 1, "Test -s (dump stable)");
        total++;
    }
    
    // 7. Test invalid flag (default case)
    {
        char *args[] = { "gcov-dump", "-x", NULL };
        passed += run_gcov_dump(gcov_dump_path, args, 0, "Test invalid flag -x");
        total++;
    }
    
    // 8. Test another invalid flag
    {
        char *args[] = { "gcov-dump", "-Z", NULL };
        passed += run_gcov_dump(gcov_dump_path, args, 0, "Test invalid flag -Z");
        total++;
    }
    
    // 9. Test invalid flag with question mark
    {
        char *args[] = { "gcov-dump", "-?", NULL };
        passed += run_gcov_dump(gcov_dump_path, args, 0, "Test invalid flag -?");
        total++;
    }
    
    printf("\n===================\n");
    printf("Summary: %d/%d tests passed\n", passed, total);
    
    // Cleanup
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    // Note: test.gcno is needed for gcov-dump tests, but we can clean it up too
    unlink(GCNO_FILE);
    
    return (passed == total) ? EXIT_SUCCESS : EXIT_FAILURE;
}
