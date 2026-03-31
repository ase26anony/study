/**
 * test_gcov_dump_flags.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by invoking
 * gcov-dump with invalid command-line flags.
 * 
 * Compile with: gcc -std=c99 -Wall -O0 -g -o test_gcov_dump test_gcov_dump_flags.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

/* Default paths to try if GCov_DUMP environment variable is not set */
static const char *DEFAULT_PATHS[] = {
    "./gcc/gcov-dump",
    "./gcov-dump",
    "../gcc/gcov-dump",
    "../../gcc/gcov-dump",
    "/usr/bin/gcov-dump",
    "/usr/local/bin/gcov-dump",
    NULL  /* Sentinel */
};

/**
 * Find the gcov-dump executable.
 * Returns a dynamically allocated string with the path, or NULL if not found.
 */
static char *find_gcov_dump(void) {
    char *path = getenv("GCOV_DUMP");
    if (path != NULL && access(path, X_OK) == 0) {
        return strdup(path);
    }
    
    /* Try environment variable GCov_DUMP (note capital C) */
    path = getenv("GCov_DUMP");
    if (path != NULL && access(path, X_OK) == 0) {
        return strdup(path);
    }
    
    /* Try default paths */
    for (int i = 0; DEFAULT_PATHS[i] != NULL; i++) {
        if (access(DEFAULT_PATHS[i], X_OK) == 0) {
            return strdup(DEFAULT_PATHS[i]);
        }
    }
    
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * Returns 1 if "unknown flag" error is found in stderr, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
    int found_error = 0;
    int pipefd[2];
    pid_t pid;
    
    /* Create pipe for capturing stderr */
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 0;
    }
    
    pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return 0;
    }
    
    if (pid == 0) {  /* Child process */
        /* Close read end of pipe */
        close(pipefd[0]);
        
        /* Redirect stderr to pipe */
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        /* Prepare arguments */
        char *args[4];
        args[0] = (char *)gcov_dump_path;
        args[1] = (char *)flag;
        args[2] = NULL;
        
        /* Execute gcov-dump */
        execvp(gcov_dump_path, args);
        
        /* If we get here, exec failed */
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {  /* Parent process */
        /* Close write end of pipe */
        close(pipefd[1]);
        
        /* Read stderr output from pipe */
        char buffer[1024];
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            
            /* Check for "unknown flag" error message */
            if (strstr(buffer, "unknown flag") != NULL) {
                found_error = 1;
                /* Print for debugging */
                printf("Found error for flag '%s': %s", flag, buffer);
            }
        }
        
        close(pipefd[0]);
        
        /* Wait for child to avoid zombie */
        int status;
        waitpid(pid, &status, 0);
        
        /* Also check exit status (should be non-zero for error) */
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            /* Non-zero exit is expected for invalid flags */
        }
    }
    
    return found_error;
}

/**
 * Test multiple invalid flags in different positions.
 */
static int test_multiple_invalid_flags(const char *gcov_dump_path) {
    struct test_case {
        const char *description;
        const char *args[5];  /* NULL-terminated array */
    };
    
    /* Test cases covering different scenarios */
    struct test_case test_cases[] = {
        /* Single invalid flag */
        {"Single invalid flag -x", {"-x", NULL}},
        {"Single invalid flag -z", {"-z", NULL}},
        {"Single invalid flag -?", {"-?", NULL}},
        
        /* Invalid flag as first argument */
        {"Invalid flag first: -x -l", {"-x", "-l", NULL}},
        
        /* Invalid flag between valid flags */
        {"Invalid flag middle: -l -x -p", {"-l", "-x", "-p", NULL}},
        
        /* Invalid flag after valid flags */
        {"Invalid flag last: -l -p -x", {"-l", "-p", "-x", NULL}},
        
        /* Multiple invalid flags */
        {"Multiple invalid: -x -y -z", {"-x", "-y", "-z", NULL}},
        
        /* Double dash with invalid flag (getopt may treat differently) */
        {"Double dash: -- -x", {"--", "-x", NULL}},
        
        /* Invalid flag after filename argument */
        {"With filename: test.gcda -x", {"test.gcda", "-x", NULL}},
        
        /* Combined valid and invalid */
        {"Mixed: -l -x -p -y -s", {"-l", "-x", "-p", "-y", "-s", NULL}},
    };
    
    int total_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int passed_tests = 0;
    
    printf("Testing %d invalid flag scenarios...\n\n", total_tests);
    
    for (int i = 0; i < total_tests; i++) {
        printf("Test %d: %s\n", i + 1, test_cases[i].description);
        
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("pipe");
            continue;
        }
        
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            close(pipefd[0]);
            close(pipefd[1]);
            continue;
        }
        
        if (pid == 0) {  /* Child */
            close(pipefd[0]);
            dup2(pipefd[1], STDERR_FILENO);
            close(pipefd[1]);
            
            /* Count arguments */
            int arg_count = 0;
            while (test_cases[i].args[arg_count] != NULL) {
                arg_count++;
            }
            
            /* Prepare argument array */
            char **args = malloc((arg_count + 2) * sizeof(char *));
            args[0] = (char *)gcov_dump_path;
            for (int j = 0; j < arg_count; j++) {
                args[j + 1] = (char *)test_cases[i].args[j];
            }
            args[arg_count + 1] = NULL;
            
            execvp(gcov_dump_path, args);
            perror("execvp");
            free(args);
            exit(EXIT_FAILURE);
        } else {  /* Parent */
            close(pipefd[1]);
            
            char buffer[1024];
            ssize_t bytes_read;
            int found_error = 0;
            
            while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
                buffer[bytes_read] = '\0';
                if (strstr(buffer, "unknown flag") != NULL) {
                    found_error = 1;
                }
            }
            
            close(pipefd[0]);
            
            int status;
            waitpid(pid, &status, 0);
            
            if (found_error) {
                printf("  ✓ PASS: Triggered 'unknown flag' error\n");
                passed_tests++;
            } else {
                printf("  ✗ FAIL: Did not trigger expected error\n");
            }
        }
    }
    
    printf("\nSummary: %d/%d tests passed\n", passed_tests, total_tests);
    return passed_tests;
}

int main(void) {
    printf("=== Testing gcov-dump invalid flag handling ===\n\n");
    
    /* Find gcov-dump executable */
    char *gcov_dump_path = find_gcov_dump();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCOV_DUMP or GCov_DUMP environment variable,\n");
        fprintf(stderr, "or ensure gcov-dump is in one of the default paths.\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    /* Test single invalid flags first */
    printf("Testing single invalid flags...\n");
    const char *invalid_flags[] = {"-x", "-z", "-?", "-X", "-0", NULL};
    
    int single_flag_tests = 0;
    int single_flag_passed = 0;
    
    for (int i = 0; invalid_flags[i] != NULL; i++) {
        single_flag_tests++;
        if (test_invalid_flag(gcov_dump_path, invalid_flags[i])) {
            single_flag_passed++;
        }
    }
    
    printf("Single flag tests: %d/%d passed\n\n", 
           single_flag_passed, single_flag_tests);
    
    /* Test more complex scenarios */
    int complex_tests_passed = test_multiple_invalid_flags(gcov_dump_path);
    
    /* Cleanup */
    free(gcov_dump_path);
    
    /* Determine overall success */
    if (single_flag_passed > 0 || complex_tests_passed > 0) {
        printf("\n✓ SUCCESS: Triggered the uncovered default case in gcov-dump\n");
        return EXIT_SUCCESS;
    } else {
        printf("\n✗ FAILURE: Did not trigger the expected error messages\n");
        return EXIT_FAILURE;
    }
}
