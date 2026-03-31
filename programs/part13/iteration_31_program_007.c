#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

#define TEST_SOURCE "test.c"
#define TEST_BINARY "test"
#define TEST_GCNO "test.gcno"

typedef struct {
    const char *name;
    const char *args[4];
    int expected_exit;
    const char *expected_stderr;
} test_case_t;

void create_test_source(void) {
    FILE *f = fopen(TEST_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source file");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

int compile_with_coverage(void) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: compile with coverage
        execlp("gcc", "gcc", "-O0", "--coverage", "-fprofile-arcs", 
               "-ftest-coverage", "-o", TEST_BINARY, TEST_SOURCE, NULL);
        perror("exec gcc failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Successfully compiled test program with coverage\n");
            return 1;
        } else {
            fprintf(stderr, "Failed to compile test program\n");
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

const char *find_gcov_dump(void) {
    const char *env_path = getenv("GCOV_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        return env_path;
    }
    
    // Try common locations
    const char *paths[] = {
        "./gcov-dump",
        "../gcov-dump/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; paths[i]; i++) {
        if (access(paths[i], X_OK) == 0) {
            return paths[i];
        }
    }
    
    return NULL;
}

int run_test(const char *gcov_dump, const test_case_t *test) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: run gcov-dump
        char *argv[10];
        int argc = 0;
        
        argv[argc++] = (char *)gcov_dump;
        
        for (int i = 0; test->args[i] != NULL; i++) {
            argv[argc++] = (char *)test->args[i];
        }
        argv[argc] = NULL;
        
        execvp(gcov_dump, argv);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == test->expected_exit) {
                printf("  ✓ %s: exited with code %d\n", test->name, exit_code);
                return 1;
            } else {
                printf("  ✗ %s: expected exit code %d, got %d\n", 
                       test->name, test->expected_exit, exit_code);
                return 0;
            }
        } else {
            printf("  ✗ %s: did not exit normally\n", test->name);
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

int main(void) {
    printf("=== GCOV-Dump Test Harness ===\n\n");
    
    // Find gcov-dump executable
    const char *gcov_dump = find_gcov_dump();
    if (!gcov_dump) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    printf("Using gcov-dump: %s\n\n", gcov_dump);
    
    // Create test source file and compile with coverage
    printf("Creating test source file...\n");
    create_test_source();
    
    printf("Compiling with coverage flags...\n");
    if (!compile_with_coverage()) {
        fprintf(stderr, "Failed to compile test program. Make sure gcc is installed.\n");
        return EXIT_FAILURE;
    }
    
    // Verify .gcno file was created
    if (access(TEST_GCNO, R_OK) != 0) {
        fprintf(stderr, "Error: %s not created. Coverage compilation may have failed.\n", TEST_GCNO);
        return EXIT_FAILURE;
    }
    printf("Generated %s for testing\n\n", TEST_GCNO);
    
    // Define test cases
    test_case_t tests[] = {
        {
            .name = "Help flag (-h)",
            .args = {"-h", NULL},
            .expected_exit = 0,
            .expected_stderr = NULL
        },
        {
            .name = "Version flag (-v)",
            .args = {"-v", NULL},
            .expected_exit = 0,
            .expected_stderr = NULL
        },
        {
            .name = "Dump contents flag (-l)",
            .args = {"-l", TEST_GCNO, NULL},
            .expected_exit = 0,
            .expected_stderr = NULL
        },
        {
            .name = "Dump positions flag (-p)",
            .args = {"-p", TEST_GCNO, NULL},
            .expected_exit = 0,
            .expected_stderr = NULL
        },
        {
            .name = "Dump raw flag (-r)",
            .args = {"-r", TEST_GCNO, NULL},
            .expected_exit = 0,
            .expected_stderr = NULL
        },
        {
            .name = "Dump stable flag (-s)",
            .args = {"-s", TEST_GCNO, NULL},
            .expected_exit = 0,
            .expected_stderr = NULL
        },
        {
            .name = "Unknown flag (-x)",
            .args = {"-x", NULL},
            .expected_exit = 1,  // Non-zero exit for error
            .expected_stderr = "unknown flag"
        }
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    
    printf("Running test cases:\n");
    printf("===================\n");
    
    for (int i = 0; i < num_tests; i++) {
        printf("\nTest %d/%d: %s\n", i + 1, num_tests, tests[i].name);
        printf("Command: %s", gcov_dump);
        for (int j = 0; tests[i].args[j] != NULL; j++) {
            printf(" %s", tests[i].args[j]);
        }
        printf("\n");
        
        if (run_test(gcov_dump, &tests[i])) {
            passed++;
        }
    }
    
    // Cleanup
    printf("\n=== Cleaning up ===\n");
    remove(TEST_SOURCE);
    remove(TEST_BINARY);
    remove(TEST_GCNO);
    
    printf("\n=== Summary ===\n");
    printf("Passed: %d/%d tests\n", passed, num_tests);
    
    return (passed == num_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}
