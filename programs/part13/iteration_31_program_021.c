#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEST_SOURCE "test_gcov.c"
#define TEST_BINARY "test_gcov"
#define TEST_GCNO "test_gcov.gcno"

typedef struct {
    const char *name;
    const char *args[4];
    int expected_exit;
    const char *expected_stderr;
} test_case;

// Create a minimal C source file for generating .gcno
void create_test_source(void) {
    FILE *f = fopen(TEST_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

// Compile with coverage to generate .gcno file
int generate_gcno_file(void) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: compile with coverage
        execlp("gcc", "gcc", "-O0", "--coverage", "-fprofile-arcs", 
               "-ftest-coverage", "-o", TEST_BINARY, TEST_SOURCE, NULL);
        perror("Failed to execute gcc");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Failed to compile test program\n");
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
        // Child process: execute gcov-dump
        char *argv[10];
        int i = 0;
        
        argv[i++] = (char *)gcov_dump_path;
        
        // Copy arguments
        for (int j = 0; test->args[j] != NULL; j++) {
            argv[i++] = (char *)test->args[j];
        }
        argv[i] = NULL;
        
        execvp(gcov_dump_path, argv);
        perror("Failed to execute gcov-dump");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == test->expected_exit) {
                return 1;  // Test passed
            } else {
                printf("  Expected exit code %d, got %d\n", 
                       test->expected_exit, exit_code);
                return 0;  // Test failed
            }
        } else {
            printf("  Process did not exit normally\n");
            return 0;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    const char *gcov_dump_path = "./gcov-dump";
    
    // Try to get path from environment variable
    char *env_path = getenv("GCOV_DUMP");
    if (env_path != NULL) {
        gcov_dump_path = env_path;
    }
    
    // Check if gcov-dump exists
    struct stat st;
    if (stat(gcov_dump_path, &st) != 0 || !S_ISREG(st.st_mode)) {
        fprintf(stderr, "gcov-dump not found at '%s'\n", gcov_dump_path);
        fprintf(stderr, "Set GCOV_DUMP environment variable or copy gcov-dump to current directory\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Create test source and generate .gcno file
    printf("Generating test .gcno file...\n");
    create_test_source();
    if (!generate_gcno_file()) {
        fprintf(stderr, "Failed to generate .gcno file\n");
        fprintf(stderr, "Make sure gcc is installed and supports coverage flags\n");
        // Clean up
        unlink(TEST_SOURCE);
        return EXIT_FAILURE;
    }
    
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
        }
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    
    printf("Running test cases...\n");
    printf("=====================\n");
    
    for (int i = 0; i < num_tests; i++) {
        printf("Test %d: %s\n", i + 1, tests[i].name);
        printf("  Command: %s", gcov_dump_path);
        for (int j = 0; tests[i].args[j] != NULL; j++) {
            printf(" %s", tests[i].args[j]);
        }
        printf("\n");
        
        if (run_test(gcov_dump_path, &tests[i])) {
            printf("  ✓ PASSED\n");
            passed++;
        } else {
            printf("  ✗ FAILED\n");
        }
        printf("\n");
    }
    
    // Clean up generated files
    printf("Cleaning up...\n");
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    unlink(TEST_GCNO);
    
    // Remove any .gcda file that might have been created
    char gcda_file[256];
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", TEST_BINARY);
    unlink(gcda_file);
    
    printf("\nSummary: %d/%d tests passed\n", passed, num_tests);
    
    return (passed == num_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}
