#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

#define TEST_SOURCE "test_coverage.c"
#define TEST_BINARY "test_coverage"
#define TEST_GCNO "test_coverage.gcno"

/* Create a minimal C source file for coverage testing */
void create_test_source(void) {
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
    printf("Created test source file: %s\n", TEST_SOURCE);
}

/* Compile the test source with coverage flags */
int compile_with_coverage(void) {
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process: compile with coverage */
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
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        /* Parent process: wait for compilation */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Successfully compiled with coverage: %s.gcno generated\n", TEST_BINARY);
            
            /* Verify the .gcno file exists */
            struct stat st;
            if (stat(TEST_GCNO, &st) == 0 && S_ISREG(st.st_mode)) {
                return 1; /* Success */
            } else {
                fprintf(stderr, "Warning: %s not found after compilation\n", TEST_GCNO);
                return 0;
            }
        } else {
            fprintf(stderr, "Compilation failed\n");
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

/* Run gcov-dump with specified arguments and check exit status */
int run_gcov_dump(const char *description, char *const args[], int expect_success) {
    printf("\n=== Testing: %s ===\n", description);
    printf("Command: gcov-dump");
    for (int i = 0; args[i] != NULL; i++) {
        printf(" %s", args[i]);
    }
    printf("\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process: execute gcov-dump */
        execvp("gcov-dump", args);
        
        /* If execvp fails, try with ./ prefix */
        char *local_args[256];
        local_args[0] = "./gcov-dump";
        for (int i = 0; args[i] != NULL; i++) {
            local_args[i + 1] = args[i];
        }
        local_args[1 + (args[0] ? 1 : 0)] = NULL;
        
        execvp("./gcov-dump", local_args);
        
        /* If still fails, try using environment variable */
        char *gcov_dump_path = getenv("GCOV_DUMP");
        if (gcov_dump_path) {
            execvp(gcov_dump_path, args);
        }
        
        fprintf(stderr, "Failed to execute gcov-dump. Ensure it's in PATH or set GCOV_DUMP environment variable.\n");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        /* Parent process: wait and check results */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Exit code: %d\n", exit_code);
            
            if (expect_success) {
                if (exit_code == 0) {
                    printf("✓ PASS: %s\n", description);
                    return 1;
                } else {
                    printf("✗ FAIL: %s (expected success, got exit code %d)\n", description, exit_code);
                    return 0;
                }
            } else {
                if (exit_code != 0) {
                    printf("✓ PASS: %s (expected failure, got exit code %d)\n", description, exit_code);
                    return 1;
                } else {
                    printf("✗ FAIL: %s (expected failure, but succeeded)\n", description);
                    return 0;
                }
            }
        } else {
            printf("✗ FAIL: %s (process terminated abnormally)\n", description);
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

int main(void) {
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("=== GCOV-Dump Test Harness ===\n");
    
    /* Test 1: Help flag (-h) */
    char *help_args[] = {"-h", NULL};
    passed_tests += run_gcov_dump("Help flag (-h)", help_args, 1);
    total_tests++;
    
    /* Test 2: Version flag (-v) */
    char *version_args[] = {"-v", NULL};
    passed_tests += run_gcov_dump("Version flag (-v)", version_args, 1);
    total_tests++;
    
    /* Create test coverage file for remaining tests */
    printf("\n=== Preparing coverage data file ===\n");
    create_test_source();
    
    if (!compile_with_coverage()) {
        fprintf(stderr, "Failed to generate coverage data. Some tests will be skipped.\n");
        
        /* Test 7: Unknown flag (doesn't need coverage file) */
        char *unknown_args[] = {"-x", NULL};
        passed_tests += run_gcov_dump("Unknown flag (-x)", unknown_args, 0);
        total_tests++;
        
        printf("\n=== Summary ===\n");
        printf("Total tests: %d\n", total_tests);
        printf("Passed: %d\n", passed_tests);
        printf("Failed: %d\n", total_tests - passed_tests);
        
        return (passed_tests == total_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    
    /* Test 3: Dump contents flag (-l) */
    char *dump_contents_args[] = {"-l", TEST_GCNO, NULL};
    passed_tests += run_gcov_dump("Dump contents flag (-l)", dump_contents_args, 1);
    total_tests++;
    
    /* Test 4: Dump positions flag (-p) */
    char *dump_positions_args[] = {"-p", TEST_GCNO, NULL};
    passed_tests += run_gcov_dump("Dump positions flag (-p)", dump_positions_args, 1);
    total_tests++;
    
    /* Test 5: Dump raw flag (-r) */
    char *dump_raw_args[] = {"-r", TEST_GCNO, NULL};
    passed_tests += run_gcov_dump("Dump raw flag (-r)", dump_raw_args, 1);
    total_tests++;
    
    /* Test 6: Dump stable flag (-s) */
    char *dump_stable_args[] = {"-s", TEST_GCNO, NULL};
    passed_tests += run_gcov_dump("Dump stable flag (-s)", dump_stable_args, 1);
    total_tests++;
    
    /* Test 7: Unknown flag (-x) */
    char *unknown_args[] = {"-x", NULL};
    passed_tests += run_gcov_dump("Unknown flag (-x)", unknown_args, 0);
    total_tests++;
    
    /* Test 8: Another unknown flag (-Z) */
    char *unknown_args2[] = {"-Z", NULL};
    passed_tests += run_gcov_dump("Unknown flag (-Z)", unknown_args2, 0);
    total_tests++;
    
    /* Test 9: Unknown flag with question mark */
    char *unknown_args3[] = {"-?", NULL};
    passed_tests += run_gcov_dump("Unknown flag (-?)", unknown_args3, 0);
    total_tests++;
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    unlink(TEST_GCNO);
    printf("Removed temporary files\n");
    
    /* Summary */
    printf("\n=== Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", total_tests - passed_tests);
    
    return (passed_tests == total_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}
