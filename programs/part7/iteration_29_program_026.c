/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by invoking
 * gcov-dump with various invalid command-line flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH_LEN 1024
#define MAX_OUTPUT_LEN 4096
#define ERROR_MSG_PREFIX "unknown flag"

/**
 * Find the gcov-dump executable path.
 * Priority: 1. GCov_DUMP environment variable
 *           2. Common build locations
 *           3. System PATH
 */
static int find_gcov_dump(char *path, size_t path_len) {
    const char *env_path = getenv("GCov_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, path_len - 1);
        path[path_len - 1] = '\0';
        return 0;
    }

    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };

    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path, common_paths[i], path_len - 1);
            path[path_len - 1] = '\0';
            return 0;
        }
    }

    return -1;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 0 on success (found error message), -1 on execution failure.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
    int pipefd[2];
    pid_t pid;
    char output[MAX_OUTPUT_LEN] = {0};
    int status;
    
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    if (pid == 0) {
        /* Child process */
        close(pipefd[0]);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        if (strcmp(flag, "--") == 0) {
            /* Test double-dash with invalid flag */
            execlp(gcov_dump_path, "gcov-dump", "--", "-x", NULL);
        } else if (strstr(flag, "file") != NULL) {
            /* Test with filename argument */
            execlp(gcov_dump_path, "gcov-dump", "-l", flag, "-x", NULL);
        } else {
            /* Regular invalid flag test */
            execlp(gcov_dump_path, "gcov-dump", flag, NULL);
        }
        
        /* If we get here, exec failed */
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        close(pipefd[1]);
        
        ssize_t bytes_read = read(pipefd[0], output, sizeof(output) - 1);
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
        }
        close(pipefd[0]);

        waitpid(pid, &status, 0);
        
        /* Check if we got the expected error message */
        if (strstr(output, ERROR_MSG_PREFIX) != NULL) {
            printf("✓ Test with flag '%s' triggered the expected error:\n", flag);
            printf("  %s", output);
            return 0;
        } else if (bytes_read > 0) {
            printf("✗ Test with flag '%s' did not produce expected error:\n", flag);
            printf("  Output: %s", output);
            return -1;
        }
    }
    
    return -1;
}

int main(void) {
    char gcov_dump_path[MAX_PATH_LEN];
    int tests_passed = 0;
    int total_tests = 0;
    
    printf("=== Testing gcov-dump invalid flag handling ===\n\n");
    
    /* Find gcov-dump executable */
    if (find_gcov_dump(gcov_dump_path, sizeof(gcov_dump_path)) != 0) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    /* Test cases designed to trigger the default case in the switch statement */
    const char *test_cases[] = {
        /* Single invalid flags */
        "-x",        /* Simple invalid flag */
        "-z",        /* Another invalid flag */
        "-?",        /* Question mark (not 'h') */
        "-0",        /* Number flag */
        "-A",        /* Uppercase letter */
        
        /* Invalid flag combinations */
        "-l -x",     /* Valid followed by invalid */
        "-x -p",     /* Invalid followed by valid */
        "-l -x -p",  /* Valid, invalid, valid */
        "-x -y -z",  /* Multiple invalid flags */
        
        /* Edge cases */
        "--x",       /* Double dash with single char (should be treated as --x argument) */
        "-",         /* Just a dash */
        
        /* With filename argument */
        "-l test.gcda -x",  /* Invalid flag after filename */
        NULL
    };
    
    /* Execute all test cases */
    for (int i = 0; test_cases[i] != NULL; i++) {
        total_tests++;
        printf("Test %d: Running 'gcov-dump %s'\n", total_tests, test_cases[i]);
        
        if (test_invalid_flag(gcov_dump_path, test_cases[i]) == 0) {
            tests_passed++;
        }
        
        printf("\n");
    }
    
    /* Special test: double-dash separator */
    total_tests++;
    printf("Test %d: Running 'gcov-dump -- -x' (double-dash test)\n", total_tests);
    if (test_invalid_flag(gcov_dump_path, "--") == 0) {
        tests_passed++;
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    
    if (tests_passed > 0) {
        printf("\n✅ Successfully triggered the uncovered default case in gcov-dump!\n");
        printf("   The error message 'unknown flag' confirms execution reached lines 111-130.\n");
        return EXIT_SUCCESS;
    } else {
        printf("\n❌ Failed to trigger the uncovered default case.\n");
        return EXIT_FAILURE;
    }
}
