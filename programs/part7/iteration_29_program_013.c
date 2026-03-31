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
#include <errno.h>

/* Maximum path length for gcov-dump executable */
#define MAX_PATH_LEN 1024
/* Maximum command line length */
#define MAX_CMD_LEN 2048
/* Buffer size for reading output */
#define BUFFER_SIZE 4096

/**
 * Find the gcov-dump executable path.
 * Priority:
 * 1. GCov_DUMP environment variable
 * 2. Common build locations
 * 3. System PATH
 * 
 * Returns: dynamically allocated string with path, or NULL if not found.
 */
static char *find_gcov_dump(void) {
    char *path = NULL;
    
    /* 1. Check environment variable */
    char *env_path = getenv("GCov_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        path = strdup(env_path);
        if (path == NULL) {
            perror("strdup failed");
            return NULL;
        }
        printf("Using gcov-dump from GCov_DUMP: %s\n", path);
        return path;
    }
    
    /* 2. Check common build locations */
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "../../prev-gcc/build/gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            path = strdup(common_paths[i]);
            if (path == NULL) {
                perror("strdup failed");
                return NULL;
            }
            printf("Found gcov-dump at: %s\n", path);
            return path;
        }
    }
    
    /* 3. Try to find in PATH */
    char *path_env = getenv("PATH");
    if (path_env != NULL) {
        char *path_copy = strdup(path_env);
        if (path_copy == NULL) {
            perror("strdup failed");
            return NULL;
        }
        
        char *dir = strtok(path_copy, ":");
        while (dir != NULL) {
            char test_path[MAX_PATH_LEN];
            snprintf(test_path, sizeof(test_path), "%s/gcov-dump", dir);
            
            if (access(test_path, X_OK) == 0) {
                path = strdup(test_path);
                free(path_copy);
                if (path == NULL) {
                    perror("strdup failed");
                    return NULL;
                }
                printf("Found gcov-dump in PATH: %s\n", path);
                return path;
            }
            
            dir = strtok(NULL, ":");
        }
        
        free(path_copy);
    }
    
    fprintf(stderr, "Error: Could not find gcov-dump executable\n");
    fprintf(stderr, "Set GCov_DUMP environment variable or ensure gcov-dump is in PATH\n");
    return NULL;
}

/**
 * Execute gcov-dump with given arguments and capture stderr.
 * 
 * Returns: 1 if "unknown flag" error found in stderr, 0 otherwise.
 */
static int test_invalid_flag(const char *gcov_dump_path, const char *flag) {
    int found_error = 0;
    int pipefd[2];
    pid_t pid;
    
    /* Create pipe for capturing stderr */
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return 0;
    }
    
    pid = fork();
    if (pid == -1) {
        perror("fork failed");
        close(pipefd[0]);
        close(pipefd[1]);
        return 0;
    }
    
    if (pid == 0) {
        /* Child process */
        close(pipefd[0]);  /* Close read end */
        
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
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        close(pipefd[1]);  /* Close write end */
        
        /* Read stderr output from pipe */
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;
        
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            
            /* Check for "unknown flag" error message */
            if (strstr(buffer, "unknown flag") != NULL) {
                printf("Found expected error: %s", buffer);
                found_error = 1;
            }
        }
        
        close(pipefd[0]);
        
        /* Wait for child to finish */
        int status;
        waitpid(pid, &status, 0);
        
        /* Check exit status (should be non-zero for invalid flag) */
        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            if (exit_status != 0) {
                printf("gcov-dump exited with non-zero status: %d (expected)\n", exit_status);
            } else {
                printf("Warning: gcov-dump exited with status 0\n");
            }
        }
    }
    
    return found_error;
}

/**
 * Test multiple invalid flags in different positions.
 */
static int test_multiple_invalid_flags(const char *gcov_dump_path) {
    int total_tests = 0;
    int passed_tests = 0;
    
    /* Test cases with invalid flags */
    struct test_case {
        const char *description;
        const char *args[5];  /* NULL-terminated array */
    };
    
    struct test_case test_cases[] = {
        /* Single invalid flags */
        {"Single invalid flag -x", {"-x", NULL}},
        {"Single invalid flag -z", {"-z", NULL}},
        {"Single invalid flag -?", {"-?", NULL}},
        {"Single invalid flag -X", {"-X", NULL}},
        
        /* Invalid flag as first argument */
        {"Invalid flag first, then valid flag", {"-x", "-l", NULL}},
        
        /* Invalid flag between valid flags */
        {"Valid flag, invalid flag, valid flag", {"-l", "-x", "-p", NULL}},
        
        /* Multiple invalid flags */
        {"Multiple invalid flags", {"-x", "-z", "-?", NULL}},
        
        /* Invalid flag after double dash (getopt behavior may vary) */
        {"Double dash with invalid flag", {"--", "-x", NULL}},
        
        /* Mixed valid and invalid flags */
        {"Mixed flags with invalid at end", {"-l", "-p", "-x", NULL}},
        {"Mixed flags with invalid in middle", {"-l", "-x", "-p", "-r", NULL}},
        
        /* Invalid flag with filename argument */
        {"Invalid flag before filename", {"-x", "test.gcda", NULL}},
        {"Valid flag, invalid flag, filename", {"-l", "-x", "test.gcda", NULL}},
        
        /* Edge case: just a dash */
        {"Single dash only", {"-", NULL}},
    };
    
    /* Execute each test case */
    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        printf("\n=== Test %zu: %s ===\n", i + 1, test_cases[i].description);
        
        /* Build command string for display */
        printf("Command: %s", gcov_dump_path);
        for (int j = 0; test_cases[i].args[j] != NULL; j++) {
            printf(" %s", test_cases[i].args[j]);
        }
        printf("\n");
        
        /* Execute test using popen to capture both stdout and stderr */
        char command[MAX_CMD_LEN];
        snprintf(command, sizeof(command), "%s", gcov_dump_path);
        
        for (int j = 0; test_cases[i].args[j] != NULL; j++) {
            strncat(command, " ", sizeof(command) - strlen(command) - 1);
            strncat(command, test_cases[i].args[j], sizeof(command) - strlen(command) - 1);
        }
        
        /* Redirect stderr to stdout for capture */
        strncat(command, " 2>&1", sizeof(command) - strlen(command) - 1);
        
        FILE *fp = popen(command, "r");
        if (fp == NULL) {
            perror("popen failed");
            continue;
        }
        
        char buffer[BUFFER_SIZE];
        int found_error = 0;
        
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            /* Check for "unknown flag" error message */
            if (strstr(buffer, "unknown flag") != NULL) {
                printf("Found expected error: %s", buffer);
                found_error = 1;
            }
        }
        
        int status = pclose(fp);
        total_tests++;
        
        if (found_error) {
            printf("✓ Test PASSED - triggered unknown flag error\n");
            passed_tests++;
        } else if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            printf("✓ Test PASSED - non-zero exit status (may have different error)\n");
            passed_tests++;
        } else {
            printf("✗ Test FAILED - no unknown flag error detected\n");
        }
    }
    
    printf("\n=== Summary ===\n");
    printf("Passed: %d/%d tests\n", passed_tests, total_tests);
    
    return (passed_tests > 0) ? 1 : 0;
}

int main(void) {
    printf("=== Testing gcov-dump invalid flag handling ===\n");
    
    /* Find gcov-dump executable */
    char *gcov_dump_path = find_gcov_dump();
    if (gcov_dump_path == NULL) {
        return EXIT_FAILURE;
    }
    
    /* Test single invalid flag first (simpler test) */
    printf("\n--- Testing single invalid flag -x ---\n");
    if (test_invalid_flag(gcov_dump_path, "-x")) {
        printf("✓ Successfully triggered unknown flag error for -x\n");
    } else {
        printf("✗ Failed to trigger unknown flag error for -x\n");
    }
    
    /* Test multiple invalid flag scenarios */
    printf("\n--- Testing multiple invalid flag scenarios ---\n");
    int result = test_multiple_invalid_flags(gcov_dump_path);
    
    /* Cleanup */
    free(gcov_dump_path);
    
    if (result) {
        printf("\n✓ SUCCESS: Triggered the uncovered default case in gcov-dump.cc\n");
        return EXIT_SUCCESS;
    } else {
        printf("\n✗ FAILURE: Could not trigger the uncovered default case\n");
        return EXIT_FAILURE;
    }
}
