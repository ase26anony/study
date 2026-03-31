#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

#define TEST_SOURCE "test_coverage.c"
#define TEST_BINARY "test_coverage"
#define GCNO_FILE "test_coverage.gcno"

int run_gcov_dump(const char *gcov_dump_path, char *const args[], int *exit_status) {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        execvp(gcov_dump_path, args);
        // If execvp returns, it failed
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            *exit_status = WEXITSTATUS(status);
            return 0;
        } else {
            fprintf(stderr, "Process terminated abnormally\n");
            return -1;
        }
    } else {
        // Fork failed
        fprintf(stderr, "Fork failed: %s\n", strerror(errno));
        return -1;
    }
}

void create_test_source() {
    FILE *fp = fopen(TEST_SOURCE, "w");
    if (!fp) {
        perror("Failed to create test source file");
        exit(EXIT_FAILURE);
    }
    
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int x = 0;\n");
    fprintf(fp, "    if (x == 0) {\n");
    fprintf(fp, "        x = 1;\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return x;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
}

void compile_with_coverage() {
    printf("Compiling test program with coverage flags...\n");
    
    // Remove existing files
    unlink(GCNO_FILE);
    unlink(TEST_BINARY);
    
    // Compile with coverage flags
    char compile_cmd[256];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -O0 --coverage -fprofile-arcs -ftest-coverage -o %s %s",
             TEST_BINARY, TEST_SOURCE);
    
    int result = system(compile_cmd);
    if (result != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        exit(EXIT_FAILURE);
    }
    
    // Run the program to potentially generate .gcda file
    result = system("./" TEST_BINARY " > /dev/null 2>&1");
    
    printf("Generated %s for testing\n", GCNO_FILE);
}

int main(int argc, char *argv[]) {
    const char *gcov_dump_path = NULL;
    
    // Try to get gcov-dump path from environment or use default
    if (getenv("GCOV_DUMP")) {
        gcov_dump_path = getenv("GCOV_DUMP");
    } else {
        gcov_dump_path = "gcov-dump";
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Create test source and compile with coverage
    create_test_source();
    compile_with_coverage();
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Test 1: -h flag (help)
    {
        printf("Test 1: Testing -h flag (help)...\n");
        char *args[] = { "gcov-dump", "-h", NULL };
        int exit_status;
        
        if (run_gcov_dump(gcov_dump_path, args, &exit_status) == 0) {
            if (exit_status == 0) {
                printf("✓ -h flag test PASSED (exit code: %d)\n", exit_status);
                passed_tests++;
            } else {
                printf("✗ -h flag test FAILED (exit code: %d)\n", exit_status);
            }
        } else {
            printf("✗ -h flag test FAILED (execution error)\n");
        }
        total_tests++;
        printf("\n");
    }
    
    // Test 2: -v flag (version)
    {
        printf("Test 2: Testing -v flag (version)...\n");
        char *args[] = { "gcov-dump", "-v", NULL };
        int exit_status;
        
        if (run_gcov_dump(gcov_dump_path, args, &exit_status) == 0) {
            if (exit_status == 0) {
                printf("✓ -v flag test PASSED (exit code: %d)\n", exit_status);
                passed_tests++;
            } else {
                printf("✗ -v flag test FAILED (exit code: %d)\n", exit_status);
            }
        } else {
            printf("✗ -v flag test FAILED (execution error)\n");
        }
        total_tests++;
        printf("\n");
    }
    
    // Test 3: -l flag (dump contents)
    {
        printf("Test 3: Testing -l flag (dump contents)...\n");
        char *args[] = { "gcov-dump", "-l", GCNO_FILE, NULL };
        int exit_status;
        
        if (run_gcov_dump(gcov_dump_path, args, &exit_status) == 0) {
            if (exit_status == 0) {
                printf("✓ -l flag test PASSED (exit code: %d)\n", exit_status);
                passed_tests++;
            } else {
                printf("✗ -l flag test FAILED (exit code: %d)\n", exit_status);
            }
        } else {
            printf("✗ -l flag test FAILED (execution error)\n");
        }
        total_tests++;
        printf("\n");
    }
    
    // Test 4: -p flag (dump positions)
    {
        printf("Test 4: Testing -p flag (dump positions)...\n");
        char *args[] = { "gcov-dump", "-p", GCNO_FILE, NULL };
        int exit_status;
        
        if (run_gcov_dump(gcov_dump_path, args, &exit_status) == 0) {
            if (exit_status == 0) {
                printf("✓ -p flag test PASSED (exit code: %d)\n", exit_status);
                passed_tests++;
            } else {
                printf("✗ -p flag test FAILED (exit code: %d)\n", exit_status);
            }
        } else {
            printf("✗ -p flag test FAILED (execution error)\n");
        }
        total_tests++;
        printf("\n");
    }
    
    // Test 5: -r flag (dump raw)
    {
        printf("Test 5: Testing -r flag (dump raw)...\n");
        char *args[] = { "gcov-dump", "-r", GCNO_FILE, NULL };
        int exit_status;
        
        if (run_gcov_dump(gcov_dump_path, args, &exit_status) == 0) {
            if (exit_status == 0) {
                printf("✓ -r flag test PASSED (exit code: %d)\n", exit_status);
                passed_tests++;
            } else {
                printf("✗ -r flag test FAILED (exit code: %d)\n", exit_status);
            }
        } else {
            printf("✗ -r flag test FAILED (execution error)\n");
        }
        total_tests++;
        printf("\n");
    }
    
    // Test 6: -s flag (dump stable)
    {
        printf("Test 6: Testing -s flag (dump stable)...\n");
        char *args[] = { "gcov-dump", "-s", GCNO_FILE, NULL };
        int exit_status;
        
        if (run_gcov_dump(gcov_dump_path, args, &exit_status) == 0) {
            if (exit_status == 0) {
                printf("✓ -s flag test PASSED (exit code: %d)\n", exit_status);
                passed_tests++;
            } else {
                printf("✗ -s flag test FAILED (exit code: %d)\n", exit_status);
            }
        } else {
            printf("✗ -s flag test FAILED (execution error)\n");
        }
        total_tests++;
        printf("\n");
    }
    
    // Test 7: Invalid flag (should trigger default case)
    {
        printf("Test 7: Testing invalid flag -x (should trigger default case)...\n");
        char *args[] = { "gcov-dump", "-x", NULL };
        int exit_status;
        
        if (run_gcov_dump(gcov_dump_path, args, &exit_status) == 0) {
            // Invalid flag should return non-zero exit code
            if (exit_status != 0) {
                printf("✓ Invalid flag test PASSED (exit code: %d)\n", exit_status);
                passed_tests++;
            } else {
                printf("✗ Invalid flag test FAILED (exit code: %d, expected non-zero)\n", exit_status);
            }
        } else {
            printf("✗ Invalid flag test FAILED (execution error)\n");
        }
        total_tests++;
        printf("\n");
    }
    
    // Additional test: Test with -? flag (another invalid flag)
    {
        printf("Test 8: Testing invalid flag -? (another invalid flag)...\n");
        char *args[] = { "gcov-dump", "-?", NULL };
        int exit_status;
        
        if (run_gcov_dump(gcov_dump_path, args, &exit_status) == 0) {
            if (exit_status != 0) {
                printf("✓ Invalid flag -? test PASSED (exit code: %d)\n", exit_status);
                passed_tests++;
            } else {
                printf("✗ Invalid flag -? test FAILED (exit code: %d, expected non-zero)\n", exit_status);
            }
        } else {
            printf("✗ Invalid flag -? test FAILED (execution error)\n");
        }
        total_tests++;
        printf("\n");
    }
    
    // Cleanup
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    
    // Summary
    printf("\n=== TEST SUMMARY ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", total_tests - passed_tests);
    
    if (passed_tests == total_tests) {
        printf("\n✓ All tests PASSED!\n");
        return EXIT_SUCCESS;
    } else {
        printf("\n✗ Some tests FAILED\n");
        return EXIT_FAILURE;
    }
}
