#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEST_SOURCE "test_coverage.c"
#define TEST_BINARY "test_coverage"
#define TEST_GCNO "test_coverage.gcno"

typedef struct {
    const char *name;
    const char *args[4];
    int expected_exit;
    const char *expected_stderr;
} test_case;

// Create a minimal C source file for coverage testing
void create_test_source(void) {
    FILE *f = fopen(TEST_SOURCE, "w");
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
}

// Compile with coverage flags to generate .gcno file
int compile_with_coverage(void) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: compile the test program
        execlp("gcc", "gcc", "-O0", "--coverage", "-fprofile-arcs", 
               "-ftest-coverage", "-o", TEST_BINARY, TEST_SOURCE, NULL);
        perror("exec gcc failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Compilation failed\n");
            return 0;
        }
        
        // Check if .gcno file was created
        struct stat st;
        if (stat(TEST_GCNO, &st) == 0 && S_ISREG(st.st_mode)) {
            return 1;
        }
    }
    return 0;
}

// Run a single test case
int run_test(const char *gcov_dump_path, const test_case *test) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: run gcov-dump
        char *argv[10];
        int argc = 0;
        
        argv[argc++] = (char *)"gcov-dump";
        
        // Copy arguments
        for (int i = 0; test->args[i] != NULL; i++) {
            argv[argc++] = (char *)test->args[i];
        }
        argv[argc] = NULL;
        
        execvp(gcov_dump_path, argv);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            
            if (test->expected_stderr) {
                // For unknown flag test, we expect specific error message
                // Note: We can't easily capture stderr without pipes in this simple version
                // In a more complete test, we'd use pipe() to capture stderr
            }
            
            return (exit_code == test->expected_exit);
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    const char *gcov_dump_path = "./gcov-dump";
    
    // Try to get path from environment variable
    char *env_path = getenv("GCOV_DUMP");
    if (env_path && strlen(env_path) > 0) {
        gcov_dump_path = env_path;
    }
    
    // Check if gcov-dump exists
    if (access(gcov_dump_path, X_OK) != 0) {
        fprintf(stderr, "gcov-dump not found at %s\n", gcov_dump_path);
        fprintf(stderr, "Set GCOV_DUMP environment variable to correct path\n");
        return EXIT_FAILURE;
    }
    
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Create test source and compile with coverage
    printf("Creating test coverage files...\n");
    create_test_source();
    
    if (!compile_with_coverage()) {
        fprintf(stderr, "Failed to create .gcno file\n");
        // Clean up
        unlink(TEST_SOURCE);
        unlink(TEST_BINARY);
        return EXIT_FAILURE;
    }
    
    printf("Generated %s for testing\n\n", TEST_GCNO);
    
    // Define test cases
    test_case tests[] = {
        // Test -h flag (help)
        {
            "Help flag (-h)",
            {"-h", NULL},
            0,
            NULL
        },
        
        // Test -v flag (version)
        {
            "Version flag (-v)",
            {"-v", NULL},
            0,
            NULL
        },
        
        // Test -l flag (dump contents)
        {
            "Dump contents flag (-l)",
            {"-l", TEST_GCNO, NULL},
            0,
            NULL
        },
        
        // Test -p flag (dump positions)
        {
            "Dump positions flag (-p)",
            {"-p", TEST_GCNO, NULL},
            0,
            NULL
        },
        
        // Test -r flag (dump raw)
        {
            "Dump raw flag (-r)",
            {"-r", TEST_GCNO, NULL},
            0,
            NULL
        },
        
        // Test -s flag (dump stable)
        {
            "Dump stable flag (-s)",
            {"-s", TEST_GCNO, NULL},
            0,
            NULL
        },
        
        // Test unknown flag (should trigger default case)
        {
            "Unknown flag (-x)",
            {"-x", NULL},
            1,  // Non-zero exit expected
            "unknown flag"
        }
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    
    // Run all tests
    for (int i = 0; i < num_tests; i++) {
        printf("Test %d/%d: %s... ", i + 1, num_tests, tests[i].name);
        fflush(stdout);
        
        if (run_test(gcov_dump_path, &tests[i])) {
            printf("PASSED\n");
            passed++;
        } else {
            printf("FAILED\n");
        }
    }
    
    // Clean up
    printf("\nCleaning up test files...\n");
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    unlink(TEST_GCNO);
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", num_tests);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", num_tests - passed);
    
    return (passed == num_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}
