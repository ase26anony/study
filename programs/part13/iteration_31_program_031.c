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
#define GCOV_DUMP_PATH "./gcov-dump"

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
        exit(1);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

// Compile with coverage flags to generate .gcno file
int compile_with_coverage(void) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: compile
        execlp("gcc", "gcc", "-O0", "--coverage", "-fprofile-arcs", 
               "-ftest-coverage", "-o", TEST_BINARY, TEST_SOURCE, NULL);
        perror("exec gcc failed");
        exit(1);
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
int run_test(const test_case *tc, const char *gcov_dump) {
    printf("Running: %s", tc->name);
    for (int i = 0; tc->args[i]; i++) {
        printf(" %s", tc->args[i]);
    }
    printf("\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: run gcov-dump
        // Prepare arguments
        char *args[10];
        int arg_count = 0;
        
        args[arg_count++] = (char *)gcov_dump;
        for (int i = 0; tc->args[i]; i++) {
            args[arg_count++] = (char *)tc->args[i];
        }
        args[arg_count] = NULL;
        
        // Execute
        execvp(gcov_dump, args);
        perror("execvp failed");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == tc->expected_exit) {
                printf("  ✓ Exit code %d as expected\n", exit_code);
                return 1;
            } else {
                printf("  ✗ Wrong exit code: got %d, expected %d\n", 
                       exit_code, tc->expected_exit);
                return 0;
            }
        } else {
            printf("  ✗ Process didn't exit normally\n");
            return 0;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    const char *gcov_dump = GCOV_DUMP_PATH;
    
    // Allow overriding gcov-dump path via environment
    char *env_path = getenv("GCOV_DUMP");
    if (env_path && env_path[0]) {
        gcov_dump = env_path;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump);
    
    // Create test files
    printf("Creating test source file...\n");
    create_test_source();
    
    printf("Compiling with coverage flags...\n");
    if (!compile_with_coverage()) {
        fprintf(stderr, "Failed to compile test program or generate .gcno file\n");
        fprintf(stderr, "Make sure gcc is installed and coverage flags are supported\n");
        return 1;
    }
    printf("Generated %s for testing\n\n", TEST_GCNO);
    
    // Define test cases
    test_case tests[] = {
        {
            "Help flag (-h)",
            {"-h", NULL},
            0,
            NULL
        },
        {
            "Version flag (-v)",
            {"-v", NULL},
            0,
            NULL
        },
        {
            "Dump contents flag (-l)",
            {"-l", TEST_GCNO, NULL},
            0,
            NULL
        },
        {
            "Dump positions flag (-p)",
            {"-p", TEST_GCNO, NULL},
            0,
            NULL
        },
        {
            "Dump raw flag (-r)",
            {"-r", TEST_GCNO, NULL},
            0,
            NULL
        },
        {
            "Dump stable flag (-s)",
            {"-s", TEST_GCNO, NULL},
            0,
            NULL
        },
        {
            "Unknown flag (-x)",
            {"-x", NULL},
            1,  // Should exit with error
            "unknown flag"
        },
        {
            "Unknown flag (-Z)",
            {"-Z", NULL},
            1,  // Should exit with error
            "unknown flag"
        },
        {
            "Unknown flag (-?)",
            {"-?", NULL},
            1,  // Should exit with error
            "unknown flag"
        }
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    
    // Run all tests
    for (int i = 0; i < num_tests; i++) {
        if (run_test(&tests[i], gcov_dump)) {
            passed++;
        }
        printf("\n");
    }
    
    // Cleanup
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    // Keep TEST_GCNO for inspection if needed
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d\n", passed, num_tests);
    
    if (passed == num_tests) {
        printf("All tests passed! ✓\n");
        return 0;
    } else {
        printf("Some tests failed ✗\n");
        return 1;
    }
}
