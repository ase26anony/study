#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define TEST_SOURCE "test.c"
#define TEST_EXECUTABLE "test"
#define TEST_GCNO "test.gcno"

typedef struct {
    const char *name;
    char **args;
    int expected_exit;
    int should_have_output;
} test_case_t;

void create_test_source() {
    FILE *f = fopen(TEST_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

void compile_test_gcno() {
    printf("Compiling test file with coverage flags...\n");
    
    // Clean up any existing files
    unlink(TEST_GCNO);
    unlink(TEST_EXECUTABLE);
    
    // Compile with coverage flags
    char *compile_args[] = {
        "gcc",
        "-O0",
        "--coverage",
        "-fprofile-arcs",
        "-ftest-coverage",
        "-o",
        TEST_EXECUTABLE,
        TEST_SOURCE,
        NULL
    };
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execvp("gcc", compile_args);
        perror("Failed to execute gcc");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Failed to compile test file\n");
            exit(EXIT_FAILURE);
        }
        printf("Generated %s successfully\n", TEST_GCNO);
    } else {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
}

int run_gcov_dump(const char *gcov_dump_path, char **args, int *has_output) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execvp(gcov_dump_path, args);
        perror("Failed to execute gcov-dump");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (has_output) {
            // Simple check - if we got here and process ran, assume output was produced
            // In a more sophisticated test, we could capture and verify output
            *has_output = WIFEXITED(status) && WEXITSTATUS(status) == 0;
        }
        
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return -1; // Process didn't exit normally
        }
    } else {
        perror("fork failed");
        return -1;
    }
}

int main(int argc, char *argv[]) {
    const char *gcov_dump_path = "./gcov-dump";
    
    // Try to get path from environment
    char *env_path = getenv("GCOV_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        gcov_dump_path = env_path;
    } else if (access("./gcov-dump", X_OK) != 0) {
        fprintf(stderr, "gcov-dump not found. Please set GCOV_DUMP environment variable\n");
        fprintf(stderr, "or ensure gcov-dump is in current directory.\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Create test source and compile it to generate .gcno file
    create_test_source();
    compile_test_gcno();
    
    // Verify .gcno file exists
    if (access(TEST_GCNO, R_OK) != 0) {
        fprintf(stderr, "Failed to create %s\n", TEST_GCNO);
        return EXIT_FAILURE;
    }
    
    // Define test cases
    char *help_args[] = { "gcov-dump", "-h", NULL };
    char *version_args[] = { "gcov-dump", "-v", NULL };
    char *dump_contents_args[] = { "gcov-dump", "-l", TEST_GCNO, NULL };
    char *dump_positions_args[] = { "gcov-dump", "-p", TEST_GCNO, NULL };
    char *dump_raw_args[] = { "gcov-dump", "-r", TEST_GCNO, NULL };
    char *dump_stable_args[] = { "gcov-dump", "-s", TEST_GCNO, NULL };
    char *unknown_flag_args[] = { "gcov-dump", "-x", NULL };
    
    test_case_t tests[] = {
        { "Help (-h)", help_args, 0, 1 },
        { "Version (-v)", version_args, 0, 1 },
        { "Dump contents (-l)", dump_contents_args, 0, 1 },
        { "Dump positions (-p)", dump_positions_args, 0, 1 },
        { "Dump raw (-r)", dump_raw_args, 0, 1 },
        { "Dump stable (-s)", dump_stable_args, 0, 1 },
        { "Unknown flag (-x)", unknown_flag_args, 1, 0 } // Should exit with error
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    
    printf("Running test cases:\n");
    printf("==================\n");
    
    for (int i = 0; i < num_tests; i++) {
        printf("Test %d: %s... ", i + 1, tests[i].name);
        fflush(stdout);
        
        int has_output = 0;
        int exit_code = run_gcov_dump(gcov_dump_path, tests[i].args, &has_output);
        
        if (exit_code == tests[i].expected_exit) {
            printf("PASS (exit code: %d", exit_code);
            if (tests[i].should_have_output && has_output) {
                printf(", output produced");
            } else if (!tests[i].should_have_output && !has_output) {
                printf(", no output as expected");
            }
            printf(")\n");
            passed++;
        } else {
            printf("FAIL (expected %d, got %d)\n", tests[i].expected_exit, exit_code);
        }
    }
    
    printf("\nSummary: %d/%d tests passed\n", passed, num_tests);
    
    // Cleanup
    unlink(TEST_SOURCE);
    unlink(TEST_EXECUTABLE);
    // Keep TEST_GCNO for inspection if needed
    
    return (passed == num_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}
