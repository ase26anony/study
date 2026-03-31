#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEST_SOURCE "test.c"
#define TEST_EXECUTABLE "test"
#define TEST_GCNO "test.gcno"
#define GCOV_DUMP_EXECUTABLE "./gcov-dump"

typedef struct {
    const char *name;
    const char *args[4];
    int expected_exit;
    const char *expected_stderr;
} test_case_t;

// Create a minimal test source file
void create_test_source(void) {
    FILE *f = fopen(TEST_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

// Compile test source with coverage flags
void compile_with_coverage(void) {
    char compile_cmd[256];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -O0 --coverage -fprofile-arcs -ftest-coverage -o %s %s",
             TEST_EXECUTABLE, TEST_SOURCE);
    
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile test program with coverage\n");
        exit(EXIT_FAILURE);
    }
    
    // Verify .gcno file was created
    struct stat st;
    if (stat(TEST_GCNO, &st) != 0) {
        fprintf(stderr, "No .gcno file generated: %s\n", TEST_GCNO);
        exit(EXIT_FAILURE);
    }
}

// Run a single test case
int run_test_case(const test_case_t *test, int test_num) {
    printf("Test %d: %s\n", test_num, test->name);
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 0;
    }
    
    if (pid == 0) {
        // Child process
        if (test->expected_stderr) {
            // Redirect stderr to capture error messages
            int fd = open("stderr.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd >= 0) {
                dup2(fd, STDERR_FILENO);
                close(fd);
            }
        }
        
        // Prepare arguments for execvp
        char *argv[5] = {0};
        int i = 0;
        argv[i++] = (char *)GCOV_DUMP_EXECUTABLE;
        
        for (int j = 0; j < 3 && test->args[j]; j++) {
            argv[i++] = (char *)test->args[j];
        }
        argv[i] = NULL;
        
        execvp(GCOV_DUMP_EXECUTABLE, argv);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        int exit_code = -1;
        if (WIFEXITED(status)) {
            exit_code = WEXITSTATUS(status);
        }
        
        // Check stderr if expected
        if (test->expected_stderr) {
            FILE *f = fopen("stderr.txt", "r");
            if (f) {
                char buf[256];
                if (fgets(buf, sizeof(buf), f)) {
                    buf[strcspn(buf, "\n")] = 0;
                    if (strstr(buf, test->expected_stderr) != NULL) {
                        printf("  ✓ Got expected stderr: %s\n", buf);
                    } else {
                        printf("  ✗ Unexpected stderr: %s\n", buf);
                        fclose(f);
                        remove("stderr.txt");
                        return 0;
                    }
                }
                fclose(f);
                remove("stderr.txt");
            }
        }
        
        int passed = (exit_code == test->expected_exit);
        printf("  %s Exit code: %d (expected %d)\n",
               passed ? "✓" : "✗", exit_code, test->expected_exit);
        
        return passed;
    }
}

int main(void) {
    // Check if gcov-dump exists
    if (access(GCOV_DUMP_EXECUTABLE, X_OK) != 0) {
        fprintf(stderr, "gcov-dump executable not found: %s\n", GCOV_DUMP_EXECUTABLE);
        fprintf(stderr, "Set GCOV_DUMP environment variable or place gcov-dump in current directory\n");
        return EXIT_FAILURE;
    }
    
    // Create test infrastructure
    create_test_source();
    compile_with_coverage();
    
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
            .expected_stderr = "unknown flag `x'"
        },
        {
            .name = "Unknown flag (-Z)",
            .args = {"-Z", NULL},
            .expected_exit = 1,
            .expected_stderr = "unknown flag `Z'"
        },
        {
            .name = "Unknown flag (-?)",
            .args = {"-?", NULL},
            .expected_exit = 1,
            .expected_stderr = "unknown flag `?'"
        }
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    
    printf("Running gcov-dump test suite...\n");
    printf("Using test file: %s\n\n", TEST_GCNO);
    
    for (int i = 0; i < num_tests; i++) {
        passed += run_test_case(&tests[i], i + 1);
        printf("\n");
    }
    
    // Cleanup
    remove(TEST_SOURCE);
    remove(TEST_EXECUTABLE);
    remove(TEST_GCNO);
    remove("stderr.txt");
    
    printf("Summary: %d/%d tests passed\n", passed, num_tests);
    
    return (passed == num_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}
