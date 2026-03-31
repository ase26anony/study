#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEST_SRC "test_gcov.c"
#define TEST_EXE "test_gcov"
#define GCNO_FILE "test_gcov.gcno"

/* Create a minimal C source file for coverage testing */
void create_test_source(void) {
    FILE *f = fopen(TEST_SRC, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(EXIT_FAILURE);
    }
    
    fprintf(f, "int main() {\n");
    fprintf(f, "    int x = 0;\n");
    fprintf(f, "    if (x == 0) {\n");
    fprintf(f, "        x = 1;\n");
    fprintf(f, "    }\n");
    fprintf(f, "    return x;\n");
    fprintf(f, "}\n");
    
    fclose(f);
    printf("Created test source: %s\n", TEST_SRC);
}

/* Compile with coverage flags to generate .gcno file */
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
            "-o", TEST_EXE,
            TEST_SRC,
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
            printf("Compiled with coverage: generated %s\n", GCNO_FILE);
            
            /* Verify .gcno file exists */
            struct stat st;
            if (stat(GCNO_FILE, &st) == 0 && S_ISREG(st.st_mode)) {
                return 1; /* Success */
            } else {
                fprintf(stderr, "Warning: %s not found\n", GCNO_FILE);
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

/* Run gcov-dump with specified arguments and check exit code */
int run_gcov_dump(const char *description, char *const args[], int expect_success) {
    printf("\n=== Testing: %s ===\n", description);
    printf("Command: gcov-dump");
    for (int i = 0; args[i] != NULL; i++) {
        printf(" %s", args[i]);
    }
    printf("\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process: run gcov-dump */
        execvp("gcov-dump", (char *const *)args);
        
        /* If execvp fails, try with ./ prefix */
        char *alt_args[256];
        int i = 0;
        alt_args[i++] = "./gcov-dump";
        for (int j = 1; args[j] != NULL && i < 255; j++) {
            alt_args[i++] = args[j];
        }
        alt_args[i] = NULL;
        
        execvp("./gcov-dump", alt_args);
        
        /* If still fails, try using environment variable */
        char *gcov_dump_path = getenv("GCOV_DUMP");
        if (gcov_dump_path) {
            execvp(gcov_dump_path, (char *const *)args);
        }
        
        perror("Failed to execute gcov-dump");
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
                    printf("✗ FAIL: %s (expected success)\n", description);
                    return 0;
                }
            } else {
                if (exit_code != 0) {
                    printf("✓ PASS: %s (expected failure)\n", description);
                    return 1;
                } else {
                    printf("✗ FAIL: %s (expected failure but got success)\n", description);
                    return 0;
                }
            }
        } else {
            printf("✗ FAIL: %s (process didn't exit normally)\n", description);
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

int main(void) {
    int passed = 0;
    int total = 0;
    
    printf("=== GCOV-Dump Test Harness ===\n");
    
    /* Test 1: Help flag (-h) */
    {
        char *args[] = {"gcov-dump", "-h", NULL};
        passed += run_gcov_dump("Help flag (-h)", args, 1);
        total++;
    }
    
    /* Test 2: Version flag (-v) */
    {
        char *args[] = {"gcov-dump", "-v", NULL};
        passed += run_gcov_dump("Version flag (-v)", args, 1);
        total++;
    }
    
    /* Create test file for coverage tests */
    create_test_source();
    
    if (!compile_with_coverage()) {
        printf("\nWarning: Could not generate .gcno file. Some tests will be skipped.\n");
        printf("Make sure gcc with coverage support is installed.\n");
    } else {
        /* Test 3: Dump contents flag (-l) */
        {
            char *args[] = {"gcov-dump", "-l", GCNO_FILE, NULL};
            passed += run_gcov_dump("Dump contents flag (-l)", args, 1);
            total++;
        }
        
        /* Test 4: Dump positions flag (-p) */
        {
            char *args[] = {"gcov-dump", "-p", GCNO_FILE, NULL};
            passed += run_gcov_dump("Dump positions flag (-p)", args, 1);
            total++;
        }
        
        /* Test 5: Dump raw flag (-r) */
        {
            char *args[] = {"gcov-dump", "-r", GCNO_FILE, NULL};
            passed += run_gcov_dump("Dump raw flag (-r)", args, 1);
            total++;
        }
        
        /* Test 6: Dump stable flag (-s) */
        {
            char *args[] = {"gcov-dump", "-s", GCNO_FILE, NULL};
            passed += run_gcov_dump("Dump stable flag (-s)", args, 1);
            total++;
        }
    }
    
    /* Test 7: Unknown flag (-x) - should trigger default case */
    {
        char *args[] = {"gcov-dump", "-x", NULL};
        passed += run_gcov_dump("Unknown flag (-x)", args, 0);
        total++;
    }
    
    /* Test 8: Another unknown flag (-Z) */
    {
        char *args[] = {"gcov-dump", "-Z", NULL};
        passed += run_gcov_dump("Unknown flag (-Z)", args, 0);
        total++;
    }
    
    /* Test 9: Unknown flag with question mark */
    {
        char *args[] = {"gcov-dump", "-?", NULL};
        passed += run_gcov_dump("Unknown flag (-?)", args, 0);
        total++;
    }
    
    /* Cleanup */
    unlink(TEST_SRC);
    unlink(TEST_EXE);
    if (access(GCNO_FILE, F_OK) == 0) {
        unlink(GCNO_FILE);
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d\n", passed, total);
    printf("Coverage: %.1f%%\n", (passed * 100.0) / total);
    
    return (passed == total) ? EXIT_SUCCESS : EXIT_FAILURE;
}
