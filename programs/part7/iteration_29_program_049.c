#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_OUTPUT 4096
#define DEFAULT_GCOV_DUMP_PATH "./gcc/gcov-dump"

/* Test cases for invalid flags */
typedef struct {
    char **argv;        /* Argument vector */
    int argc;           /* Argument count */
    char *description;  /* Test description */
} test_case_t;

/* Helper to create test cases */
test_case_t create_test_case(const char *description, int count, ...) {
    test_case_t tc;
    tc.description = strdup(description);
    tc.argv = malloc((count + 2) * sizeof(char *)); /* +2 for program name and NULL */
    tc.argc = count + 1;
    
    /* First argument is program name */
    tc.argv[0] = "gcov-dump";
    
    /* Variable arguments */
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        tc.argv[i + 1] = va_arg(args, char *);
    }
    va_end(args);
    
    tc.argv[count + 1] = NULL;
    return tc;
}

/* Free test case memory */
void free_test_case(test_case_t *tc) {
    free(tc->description);
    free(tc->argv);
}

/* Find gcov-dump executable */
char *find_gcov_dump() {
    char *path = getenv("GCOV_DUMP");
    if (path && access(path, X_OK) == 0) {
        return strdup(path);
    }
    
    /* Try common build locations */
    const char *candidates[] = {
        "./gcc/gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "../../prev-gcc/build/gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; candidates[i]; i++) {
        if (access(candidates[i], X_OK) == 0) {
            return strdup(candidates[i]);
        }
    }
    
    return strdup(DEFAULT_GCOV_DUMP_PATH);
}

/* Execute test case and check for error message */
int run_test_case(const char *gcov_dump_path, test_case_t *tc) {
    printf("Testing: %s\n", tc->description);
    printf("Command: %s", gcov_dump_path);
    for (int i = 1; i < tc->argc; i++) {
        printf(" %s", tc->argv[i]);
    }
    printf("\n");
    
    /* Create pipes for stdout and stderr */
    int stdout_pipe[2], stderr_pipe[2];
    if (pipe(stdout_pipe) == -1 || pipe(stderr_pipe) == -1) {
        perror("pipe");
        return 0;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        close(stdout_pipe[0]); close(stdout_pipe[1]);
        close(stderr_pipe[0]); close(stderr_pipe[1]);
        return 0;
    }
    
    if (pid == 0) { /* Child process */
        /* Close read ends */
        close(stdout_pipe[0]);
        close(stderr_pipe[0]);
        
        /* Redirect stdout and stderr */
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);
        
        /* Close original write ends */
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);
        
        /* Prepare arguments */
        char **exec_argv = malloc((tc->argc + 1) * sizeof(char *));
        exec_argv[0] = (char *)gcov_dump_path;
        for (int i = 1; i < tc->argc; i++) {
            exec_argv[i] = tc->argv[i];
        }
        exec_argv[tc->argc] = NULL;
        
        /* Execute */
        execvp(gcov_dump_path, exec_argv);
        
        /* If we get here, exec failed */
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        free(exec_argv);
        exit(EXIT_FAILURE);
    }
    
    /* Parent process */
    close(stdout_pipe[1]);
    close(stderr_pipe[1]);
    
    /* Read stderr output */
    char stderr_output[MAX_OUTPUT] = {0};
    ssize_t bytes_read = read(stderr_pipe[0], stderr_output, MAX_OUTPUT - 1);
    close(stderr_pipe[0]);
    close(stdout_pipe[0]);
    
    /* Wait for child */
    int status;
    waitpid(pid, &status, 0);
    
    /* Check for error message */
    int found = 0;
    if (bytes_read > 0) {
        stderr_output[bytes_read] = '\0';
        printf("Stderr output: %s", stderr_output);
        
        /* Look for error message */
        if (strstr(stderr_output, "unknown flag") != NULL ||
            strstr(stderr_output, "unknown flag `") != NULL) {
            found = 1;
            printf("✓ Found 'unknown flag' error message\n");
        } else {
            printf("✗ Did not find 'unknown flag' error message\n");
        }
    } else {
        printf("✗ No stderr output\n");
    }
    
    printf("Exit status: %d\n\n", WEXITSTATUS(status));
    return found;
}

int main() {
    char *gcov_dump_path = find_gcov_dump();
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    /* Check if executable exists */
    if (access(gcov_dump_path, X_OK) != 0) {
        fprintf(stderr, "Error: Cannot execute %s: %s\n", 
                gcov_dump_path, strerror(errno));
        fprintf(stderr, "Set GCOV_DUMP environment variable to correct path.\n");
        free(gcov_dump_path);
        return EXIT_FAILURE;
    }
    
    /* Define test cases targeting the uncovered lines */
    test_case_t tests[] = {
        /* Single invalid flags */
        create_test_case("Single invalid flag '-x'", 1, "-x"),
        create_test_case("Single invalid flag '-z'", 1, "-z"),
        create_test_case("Single invalid flag '-?'", 1, "-?"),
        create_test_case("Single invalid flag '-0'", 1, "-0"),
        
        /* Invalid flag in different positions */
        create_test_case("Invalid flag between valid flags", 3, "-l", "-x", "-p"),
        create_test_case("Invalid flag after valid flags", 3, "-l", "-p", "-x"),
        create_test_case("Invalid flag before valid flags", 3, "-x", "-l", "-p"),
        
        /* Invalid flag with filename argument */
        create_test_case("Invalid flag before filename", 2, "-x", "test.gcda"),
        create_test_case("Valid flag, invalid flag, filename", 3, "-l", "-x", "test.gcda"),
        create_test_case("Filename, then invalid flag", 2, "test.gcda", "-x"),
        
        /* Double dash edge cases */
        create_test_case("Double dash with invalid flag '--x'", 1, "--x"),
        create_test_case("Double dash with invalid flag '--z'", 1, "--z"),
        create_test_case("Valid flag, double dash invalid", 2, "-l", "--x"),
        
        /* Multiple invalid flags */
        create_test_case("Multiple invalid flags combined '-xz'", 1, "-xz"),
        create_test_case("Multiple invalid flags separate '-x -z'", 2, "-x", "-z"),
        
        /* Mixed valid and invalid combined */
        create_test_case("Mixed valid/invalid combined '-lx'", 1, "-lx"),
        create_test_case("Mixed valid/invalid combined '-xp'", 1, "-xp"),
        
        /* Boundary: just dash */
        create_test_case("Single dash '-'", 1, "-"),
        
        /* All valid flags plus one invalid */
        create_test_case("All valid flags plus invalid", 6, "-l", "-p", "-r", "-s", "-v", "-x"),
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    
    /* Run all tests */
    for (int i = 0; i < num_tests; i++) {
        if (run_test_case(gcov_dump_path, &tests[i])) {
            passed++;
        }
        free_test_case(&tests[i]);
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", num_tests);
    printf("Tests that triggered 'unknown flag': %d\n", passed);
    printf("Success rate: %.1f%%\n", (passed * 100.0) / num_tests);
    
    free(gcov_dump_path);
    
    /* Consider it a success if at least one test triggered the error */
    return (passed > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
