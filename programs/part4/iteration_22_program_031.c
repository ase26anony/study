/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Tests the uncovered default case in gcov-dump.cc by invoking
 * gcov-dump with invalid command-line flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/* Default path to gcov-dump if GCov_DUMP env var is not set */
#define DEFAULT_GCOV_DUMP_PATH "./gcc/gcov-dump"

/* Invalid flags to test - these should trigger the default case */
static const char invalid_flags[] = "xzw?@#$%^&*";

/**
 * Find the gcov-dump executable path.
 * Returns: dynamically allocated string with the path, or NULL if not found.
 */
static char *find_gcov_dump_path(void)
{
    char *path = getenv("GCov_DUMP");
    
    if (path != NULL && access(path, X_OK) == 0) {
        return strdup(path);
    }
    
    /* Try default path */
    if (access(DEFAULT_GCOV_DUMP_PATH, X_OK) == 0) {
        return strdup(DEFAULT_GCOV_DUMP_PATH);
    }
    
    /* Try some other common locations in a GCC build tree */
    const char *common_paths[] = {
        "../prev-gcc/build/gcc/gcov-dump",
        "../gcc-build/gcc/gcov-dump",
        "gcc/gcov-dump",
        "./gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            return strdup(common_paths[i]);
        }
    }
    
    return NULL;
}

/**
 * Test a single invalid flag.
 * Returns: 1 if the error message was found, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, char invalid_flag)
{
    int found_error = 0;
    int pipe_fd[2];
    pid_t pid;
    
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        return 0;
    }
    
    pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return 0;
    }
    
    if (pid == 0) {
        /* Child process */
        close(pipe_fd[0]);  /* Close read end */
        
        /* Redirect stderr to pipe */
        dup2(pipe_fd[1], STDERR_FILENO);
        close(pipe_fd[1]);
        
        /* Construct argument list with invalid flag */
        char flag[3] = "-x";
        flag[1] = invalid_flag;
        
        /* Test different flag positions */
        const char *args[] = {
            gcov_dump_path,
            flag,           /* Invalid flag as first argument */
            NULL
        };
        
        execv(gcov_dump_path, (char *const *)args);
        
        /* If we get here, exec failed */
        perror("execv");
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        close(pipe_fd[1]);  /* Close write end */
        
        /* Read stderr output from child */
        char buffer[1024];
        ssize_t bytes_read;
        
        while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            
            /* Check for the error message */
            if (strstr(buffer, "unknown flag") != NULL) {
                found_error = 1;
                printf("Found error message for flag '-%c':\n  %s", 
                       invalid_flag, buffer);
            }
        }
        
        close(pipe_fd[0]);
        
        /* Wait for child to finish */
        int status;
        waitpid(pid, &status, 0);
    }
    
    return found_error;
}

/**
 * Test invalid flag in different positions.
 */
static int test_flag_positions(const char *gcov_dump_path, char invalid_flag)
{
    int success_count = 0;
    
    /* Test different argument combinations */
    const char *test_cases[][6] = {
        /* Invalid flag as first argument */
        {gcov_dump_path, "-x", NULL},
        /* Invalid flag between valid flags */
        {gcov_dump_path, "-l", "-x", "-p", NULL},
        /* Invalid flag after valid flags */
        {gcov_dump_path, "-l", "-p", "-x", NULL},
        /* Multiple invalid flags */
        {gcov_dump_path, "-x", "-z", NULL},
        /* Invalid flag with double dash (should still trigger error) */
        {gcov_dump_path, "--", "-x", NULL},
        /* Invalid flag after filename argument */
        {gcov_dump_path, "test.gcda", "-x", NULL},
    };
    
    char flag[3] = "-x";
    flag[1] = invalid_flag;
    
    for (int i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        /* Replace placeholder -x with actual invalid flag */
        char *args[10];
        int arg_count = 0;
        
        for (int j = 0; test_cases[i][j] != NULL && arg_count < 9; j++) {
            if (strcmp(test_cases[i][j], "-x") == 0) {
                args[arg_count++] = flag;
            } else {
                args[arg_count++] = (char *)test_cases[i][j];
            }
        }
        args[arg_count] = NULL;
        
        /* Execute test case */
        printf("Testing: ");
        for (int j = 0; j < arg_count; j++) {
            printf("%s ", args[j]);
        }
        printf("\n");
        
        /* Use popen to capture stderr */
        char command[1024] = "";
        for (int j = 0; j < arg_count; j++) {
            strcat(command, args[j]);
            strcat(command, " ");
        }
        strcat(command, "2>&1");  /* Redirect stderr to stdout */
        
        FILE *fp = popen(command, "r");
        if (fp == NULL) {
            perror("popen");
            continue;
        }
        
        char buffer[1024];
        int found_error = 0;
        
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            if (strstr(buffer, "unknown flag") != NULL) {
                found_error = 1;
                printf("  Success: %s", buffer);
                break;
            }
        }
        
        pclose(fp);
        
        if (found_error) {
            success_count++;
        } else {
            printf("  Failed: No error message found\n");
        }
    }
    
    return success_count;
}

int main(void)
{
    printf("=== Testing gcov-dump invalid flags ===\n");
    
    /* Find gcov-dump executable */
    char *gcov_dump_path = find_gcov_dump_path();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: gcov-dump executable not found\n");
        fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in a standard location\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    int total_tests = 0;
    int passed_tests = 0;
    
    /* Test each invalid flag */
    for (int i = 0; invalid_flags[i] != '\0'; i++) {
        printf("\n--- Testing invalid flag '-%c' ---\n", invalid_flags[i]);
        
        /* Test basic invalid flag */
        total_tests++;
        if (test_invalid_flag(gcov_dump_path, invalid_flags[i])) {
            passed_tests++;
        } else {
            printf("Basic test failed for flag '-%c'\n", invalid_flags[i]);
        }
        
        /* Test flag in different positions */
        int position_tests = test_flag_positions(gcov_dump_path, invalid_flags[i]);
        total_tests += position_tests;
        passed_tests += position_tests;
    }
    
    /* Test special case: invalid flag '?' (help-like but invalid) */
    printf("\n--- Testing special flag '-?' ---\n");
    total_tests++;
    if (test_invalid_flag(gcov_dump_path, '?')) {
        passed_tests++;
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    
    free(gcov_dump_path);
    
    if (passed_tests > 0) {
        printf("\nSUCCESS: Triggered the uncovered default case!\n");
        return EXIT_SUCCESS;
    } else {
        printf("\nFAILURE: Could not trigger the uncovered default case\n");
        return EXIT_FAILURE;
    }
}
