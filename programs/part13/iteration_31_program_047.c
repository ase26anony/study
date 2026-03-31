#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

#define TEST_SOURCE "test_coverage.c"
#define TEST_EXECUTABLE "test_coverage"
#define GCNO_FILE "test_coverage.gcno"
#define GCOV_DUMP_EXECUTABLE "./gcov-dump"

typedef struct {
    const char *name;
    const char *args[4];
    int expected_exit;
    int check_stderr;
    const char *expected_stderr;
} test_case;

// Create a minimal C source file for coverage testing
void create_test_source(void) {
    FILE *f = fopen(TEST_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source file");
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
    printf("Created test source file: %s\n", TEST_SOURCE);
}

// Compile with coverage flags to generate .gcno file
int compile_with_coverage(void) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: compile with coverage
        char *args[] = {
            "gcc",
            "-O0",
            "--coverage",
            "-fprofile-arcs",
            "-ftest-coverage",
            "-o", TEST_EXECUTABLE,
            TEST_SOURCE,
            NULL
        };
        
        execvp("gcc", args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process: wait for compilation
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Successfully compiled with coverage\n");
            
            // Check if .gcno file was created
            struct stat st;
            if (stat(GCNO_FILE, &st) == 0) {
                printf("Generated %s (size: %ld bytes)\n", GCNO_FILE, (long)st.st_size);
                return 1;
            } else {
                printf("Warning: %s not found after compilation\n", GCNO_FILE);
                return 0;
            }
        } else {
            printf("Compilation failed\n");
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

// Run a single test case
int run_test(const test_case *test, const char *gcov_dump_path) {
    printf("\nRunning test: %s\n", test->name);
    printf("Command: %s", gcov_dump_path);
    
    for (int i = 0; test->args[i] != NULL; i++) {
        printf(" %s", test->args[i]);
    }
    printf("\n");
    
    // Create pipes for stderr if we need to check it
    int stderr_pipe[2] = {-1, -1};
    if (test->check_stderr) {
        if (pipe(stderr_pipe) == -1) {
            perror("pipe failed");
            return 0;
        }
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        if (test->check_stderr) {
            close(stderr_pipe[0]);  // Close read end
            dup2(stderr_pipe[1], STDERR_FILENO);  // Redirect stderr to pipe
            close(stderr_pipe[1]);
        }
        
        // Build argument list
        char **args = malloc(sizeof(char *) * 10);
        int arg_count = 0;
        args[arg_count++] = (char *)gcov_dump_path;
        
        for (int i = 0; test->args[i] != NULL; i++) {
            args[arg_count++] = (char *)test->args[i];
        }
        args[arg_count] = NULL;
        
        execvp(gcov_dump_path, args);
        perror("execvp failed");
        free(args);
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        if (test->check_stderr) {
            close(stderr_pipe[1]);  // Close write end
        }
        
        int status;
        waitpid(pid, &status, 0);
        
        // Check stderr output if needed
        if (test->check_stderr) {
            char buffer[256] = {0};
            ssize_t bytes_read = read(stderr_pipe[0], buffer, sizeof(buffer) - 1);
            close(stderr_pipe[0]);
            
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                printf("Stderr output: %s", buffer);
                
                if (test->expected_stderr && 
                    strstr(buffer, test->expected_stderr) == NULL) {
                    printf("ERROR: Expected stderr to contain '%s'\n", 
                           test->expected_stderr);
                    return 0;
                }
            }
        }
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Exit code: %d (expected: %d)\n", exit_code, test->expected_exit);
            
            if (exit_code == test->expected_exit) {
                printf("✓ Test passed\n");
                return 1;
            } else {
                printf("✗ Test failed - wrong exit code\n");
                return 0;
            }
        } else {
            printf("✗ Test failed - child did not exit normally\n");
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

int main(int argc, char *argv[]) {
    printf("=== GCOV-Dump Test Harness ===\n");
    
    // Determine gcov-dump executable path
    const char *gcov_dump_path = GCOV_DUMP_EXECUTABLE;
    char *env_path = getenv("GCOV_DUMP");
    if (env_path != NULL && strlen(env_path) > 0) {
        gcov_dump_path = env_path;
    }
    
    printf("Using gcov-dump executable: %s\n", gcov_dump_path);
    
    // Check if gcov-dump exists
    struct stat st;
    if (stat(gcov_dump_path, &st) != 0) {
        fprintf(stderr, "Error: gcov-dump executable not found at %s\n", gcov_dump_path);
        fprintf(stderr, "Set GCOV_DUMP environment variable to specify path\n");
        return EXIT_FAILURE;
    }
    
    // Create test source and compile with coverage
    create_test_source();
    int has_gcno = compile_with_coverage();
    
    if (!has_gcno) {
        printf("Warning: No .gcno file available for tests requiring it\n");
        printf("Some tests will be skipped\n");
    }
    
    // Define test cases
    test_case tests[] = {
        // Test 1: -h flag (help)
        {
            "Help flag (-h)",
            {"-h", NULL},
            0,  // Expected exit code
            0,  // Don't check stderr
            NULL
        },
        
        // Test 2: -v flag (version)
        {
            "Version flag (-v)",
            {"-v", NULL},
            0,
            0,
            NULL
        },
        
        // Test 3: -l flag (dump contents) - requires .gcno file
        {
            "Dump contents flag (-l)",
            {"-l", GCNO_FILE, NULL},
            0,
            0,
            NULL
        },
        
        // Test 4: -p flag (dump positions) - requires .gcno file
        {
            "Dump positions flag (-p)",
            {"-p", GCNO_FILE, NULL},
            0,
            0,
            NULL
        },
        
        // Test 5: -r flag (dump raw) - requires .gcno file
        {
            "Dump raw flag (-r)",
            {"-r", GCNO_FILE, NULL},
            0,
            0,
            NULL
        },
        
        // Test 6: -s flag (dump stable) - requires .gcno file
        {
            "Dump stable flag (-s)",
            {"-s", GCNO_FILE, NULL},
            0,
            0,
            NULL
        },
        
        // Test 7: Invalid flag (should trigger default case)
        {
            "Invalid flag (-x)",
            {"-x", NULL},
            EXIT_FAILURE,  // Should exit with error
            1,  // Check stderr
            "unknown flag"
        },
        
        // Test 8: Another invalid flag
        {
            "Invalid flag (-Z)",
            {"-Z", NULL},
            EXIT_FAILURE,
            1,
            "unknown flag"
        },
        
        // Test 9: Combined flags
        {
            "Combined flags (-l -p)",
            {"-l", "-p", GCNO_FILE, NULL},
            0,
            0,
            NULL
        }
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    int skipped = 0;
    
    // Run tests
    for (int i = 0; i < num_tests; i++) {
        // Skip tests requiring .gcno file if we don't have one
        if ((i >= 2 && i <= 6) && !has_gcno) {
            printf("\nSkipping test '%s' (no .gcno file)\n", tests[i].name);
            skipped++;
            continue;
        }
        
        if (run_test(&tests[i], gcov_dump_path)) {
            passed++;
        }
    }
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", num_tests);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", num_tests - passed - skipped);
    printf("Skipped: %d\n", skipped);
    
    // Cleanup
    unlink(TEST_SOURCE);
    unlink(TEST_EXECUTABLE);
    if (has_gcno) {
        unlink(GCNO_FILE);
        unlink("test_coverage.gcda");  // Remove any gcda file created
    }
    
    return (passed == (num_tests - skipped)) ? EXIT_SUCCESS : EXIT_FAILURE;
}
