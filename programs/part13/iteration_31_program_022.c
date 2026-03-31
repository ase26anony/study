/**
 * gcov-dump_test.c - Test harness for gcov-dump executable
 * Compile: gcc -O0 -g -Wall -Wextra -D_GNU_SOURCE -o gcov-dump_test gcov-dump_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

#define TEST_SOURCE "test_gcov.c"
#define TEST_BINARY "test_gcov"
#define TEST_GCNO   "test_gcov.gcno"
#define GCOV_DUMP_EXEC "./gcov-dump"

/**
 * Execute gcov-dump with given arguments and check exit status
 * Returns 1 on success (expected exit code matches), 0 on failure
 */
int run_gcov_dump(const char *description, char *const args[], int expected_exit) {
    pid_t pid;
    int status;
    
    printf("Testing: %s\n", description);
    printf("Command: %s", GCOV_DUMP_EXEC);
    for (int i = 1; args[i] != NULL; i++) {
        printf(" %s", args[i]);
    }
    printf("\n");
    
    pid = fork();
    if (pid == 0) {
        // Child process
        execvp(GCOV_DUMP_EXEC, (char *const *)args);
        // If execvp returns, it failed
        fprintf(stderr, "Failed to execute %s: %s\n", GCOV_DUMP_EXEC, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == expected_exit) {
                printf("  ✓ PASSED (exit code: %d)\n\n", exit_code);
                return 1;
            } else {
                printf("  ✗ FAILED - Expected exit code %d, got %d\n\n", expected_exit, exit_code);
                return 0;
            }
        } else {
            printf("  ✗ FAILED - Process terminated abnormally\n\n");
            return 0;
        }
    } else {
        fprintf(stderr, "Fork failed: %s\n", strerror(errno));
        return 0;
    }
}

/**
 * Create a minimal test source file for coverage data generation
 */
void create_test_source(void) {
    FILE *fp = fopen(TEST_SOURCE, "w");
    if (!fp) {
        perror("Failed to create test source file");
        exit(EXIT_FAILURE);
    }
    
    fprintf(fp, "/* Minimal test program for gcov-dump testing */\n");
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    printf(\"Hello from test program\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    printf("Created test source: %s\n", TEST_SOURCE);
}

/**
 * Compile test program with coverage flags to generate .gcno file
 */
void compile_with_coverage(void) {
    pid_t pid;
    int status;
    
    printf("Compiling test program with coverage flags...\n");
    
    // Remove existing files
    unlink(TEST_GCNO);
    unlink(TEST_BINARY);
    
    pid = fork();
    if (pid == 0) {
        // Child process - compile with coverage
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
        fprintf(stderr, "Failed to compile test program: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Compilation successful, generated %s\n\n", TEST_GCNO);
        } else {
            fprintf(stderr, "Compilation failed\n");
            exit(EXIT_FAILURE);
        }
    } else {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
}

/**
 * Verify that .gcno file exists and is readable
 */
int verify_gcno_file(void) {
    FILE *fp = fopen(TEST_GCNO, "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open %s: %s\n", TEST_GCNO, strerror(errno));
        return 0;
    }
    
    fclose(fp);
    printf("Verified %s exists and is readable\n", TEST_GCNO);
    return 1;
}

int main(void) {
    int passed = 0;
    int total = 0;
    
    printf("========================================\n");
    printf("gcov-dump Test Harness\n");
    printf("Targeting uncovered lines 111-130\n");
    printf("========================================\n\n");
    
    // Test 1: -h flag (help)
    {
        char *args[] = { GCOV_DUMP_EXEC, "-h", NULL };
        passed += run_gcov_dump("Help flag (-h)", args, 0);
        total++;
    }
    
    // Test 2: -v flag (version)
    {
        char *args[] = { GCOV_DUMP_EXEC, "-v", NULL };
        passed += run_gcov_dump("Version flag (-v)", args, 0);
        total++;
    }
    
    // Create test source and compile with coverage for remaining tests
    printf("========================================\n");
    printf("Preparing coverage data file for tests...\n");
    printf("========================================\n");
    create_test_source();
    compile_with_coverage();
    
    if (!verify_gcno_file()) {
        fprintf(stderr, "Cannot proceed without valid .gcno file\n");
        return EXIT_FAILURE;
    }
    
    // Test 3: -l flag (dump contents)
    {
        char *args[] = { GCOV_DUMP_EXEC, "-l", TEST_GCNO, NULL };
        passed += run_gcov_dump("Dump contents flag (-l)", args, 0);
        total++;
    }
    
    // Test 4: -p flag (dump positions)
    {
        char *args[] = { GCOV_DUMP_EXEC, "-p", TEST_GCNO, NULL };
        passed += run_gcov_dump("Dump positions flag (-p)", args, 0);
        total++;
    }
    
    // Test 5: -r flag (dump raw)
    {
        char *args[] = { GCOV_DUMP_EXEC, "-r", TEST_GCNO, NULL };
        passed += run_gcov_dump("Dump raw flag (-r)", args, 0);
        total++;
    }
    
    // Test 6: -s flag (dump stable)
    {
        char *args[] = { GCOV_DUMP_EXEC, "-s", TEST_GCNO, NULL };
        passed += run_gcov_dump("Dump stable flag (-s)", args, 0);
        total++;
    }
    
    // Test 7: Invalid flag (should trigger default case)
    {
        char *args[] = { GCOV_DUMP_EXEC, "-x", NULL };
        passed += run_gcov_dump("Invalid flag (-x) - should trigger 'unknown flag'", args, 1);
        total++;
    }
    
    // Test 8: Another invalid flag
    {
        char *args[] = { GCOV_DUMP_EXEC, "-Z", NULL };
        passed += run_gcov_dump("Invalid flag (-Z) - should trigger 'unknown flag'", args, 1);
        total++;
    }
    
    // Test 9: Yet another invalid flag
    {
        char *args[] = { GCOV_DUMP_EXEC, "-?", NULL };
        passed += run_gcov_dump("Invalid flag (-?) - should trigger 'unknown flag'", args, 1);
        total++;
    }
    
    printf("========================================\n");
    printf("Test Summary\n");
    printf("========================================\n");
    printf("Passed: %d/%d\n", passed, total);
    printf("Coverage: %.1f%%\n", (passed * 100.0) / total);
    
    // Cleanup
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    unlink(TEST_GCNO);
    
    return (passed == total) ? EXIT_SUCCESS : EXIT_FAILURE;
}
