#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

#define MAX_PATH_LEN 1024
#define MAX_OUTPUT_LEN 4096

/* Function to find gcov-dump executable */
char* find_gcov_dump() {
    char *path = getenv("GCOV_DUMP");
    if (path != NULL && access(path, X_OK) == 0) {
        return strdup(path);
    }
    
    /* Try common locations in a GCC build tree */
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "../../../gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            return strdup(common_paths[i]);
        }
    }
    
    return NULL;
}

/* Test a single invalid flag */
int test_invalid_flag(const char *gcov_dump_path, const char *flag, const char **args, int arg_count) {
    int pipefd[2];
    pid_t pid;
    char buffer[MAX_OUTPUT_LEN];
    int status;
    int found_error = 0;
    
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
    
    if (pid == 0) { /* Child process */
        close(pipefd[0]); /* Close read end */
        
        /* Redirect stderr to pipe */
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        /* Build argument list */
        char **argv = malloc((arg_count + 3) * sizeof(char*));
        int i = 0;
        argv[i++] = (char*)gcov_dump_path;
        
        /* Add the invalid flag */
        argv[i++] = (char*)flag;
        
        /* Add any additional arguments */
        for (int j = 0; j < arg_count; j++) {
            argv[i++] = (char*)args[j];
        }
        argv[i] = NULL;
        
        execvp(gcov_dump_path, argv);
        
        /* If we get here, exec failed */
        perror("execvp");
        free(argv);
        exit(EXIT_FAILURE);
    } else { /* Parent process */
        close(pipefd[1]); /* Close write end */
        
        /* Read stderr output */
        ssize_t bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            
            /* Check for error message */
            if (strstr(buffer, "unknown flag") != NULL) {
                printf("Found error message for flag '%s':\n", flag);
                printf("%s\n", buffer);
                found_error = 1;
            } else {
                printf("No error message found for flag '%s'. Output:\n%s\n", 
                       flag, buffer);
            }
        }
        
        close(pipefd[0]);
        
        /* Wait for child */
        waitpid(pid, &status, 0);
        
        /* Check if child exited with non-zero status (expected for invalid flag) */
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            printf("Process exited with status %d (expected for invalid flag)\n", 
                   WEXITSTATUS(status));
        }
    }
    
    return found_error;
}

int main() {
    char *gcov_dump_path = find_gcov_dump();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: gcov-dump executable not found\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in a standard location\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    int total_tests = 0;
    int passed_tests = 0;
    
    /* Test 1: Single invalid flag as first argument */
    printf("=== Test 1: Single invalid flag '-x' ===\n");
    total_tests++;
    if (test_invalid_flag(gcov_dump_path, "-x", NULL, 0)) {
        passed_tests++;
    }
    printf("\n");
    
    /* Test 2: Invalid flag between valid flags */
    printf("=== Test 2: Invalid flag '-z' between valid flags '-l' and '-p' ===\n");
    total_tests++;
    const char *args2[] = {"-l", "-p"};
    if (test_invalid_flag(gcov_dump_path, "-z", args2, 2)) {
        passed_tests++;
    }
    printf("\n");
    
    /* Test 3: Multiple invalid flags */
    printf("=== Test 3: Multiple invalid flags '-a -b -c' ===\n");
    total_tests++;
    const char *args3[] = {"-b", "-c"};
    if (test_invalid_flag(gcov_dump_path, "-a", args3, 2)) {
        passed_tests++;
    }
    printf("\n");
    
    /* Test 4: Invalid flag after non-option argument (simulated with a filename) */
    printf("=== Test 4: Invalid flag '-?' after filename ===\n");
    total_tests++;
    const char *args4[] = {"testfile.gcda", "-?"};
    if (test_invalid_flag(gcov_dump_path, "-l", args4, 2)) {
        passed_tests++;
    }
    printf("\n");
    
    /* Test 5: Double dash with invalid single-character flag */
    printf("=== Test 5: Double dash with invalid flag '--x' ===\n");
    total_tests++;
    if (test_invalid_flag(gcov_dump_path, "--x", NULL, 0)) {
        passed_tests++;
    }
    printf("\n");
    
    /* Test 6: Combination of valid and invalid flags */
    printf("=== Test 6: Combination '-l -x -p' ===\n");
    total_tests++;
    const char *args6[] = {"-x", "-p"};
    if (test_invalid_flag(gcov_dump_path, "-l", args6, 2)) {
        passed_tests++;
    }
    printf("\n");
    
    /* Test 7: Invalid flag with dash in different position */
    printf("=== Test 7: Invalid flag '-k' as only argument ===\n");
    total_tests++;
    if (test_invalid_flag(gcov_dump_path, "-k", NULL, 0)) {
        passed_tests++;
    }
    printf("\n");
    
    /* Summary */
    printf("=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Success rate: %.1f%%\n", (passed_tests * 100.0) / total_tests);
    
    free(gcov_dump_path);
    
    if (passed_tests > 0) {
        printf("\nSUCCESS: Triggered the uncovered default case in gcov-dump!\n");
        return EXIT_SUCCESS;
    } else {
        printf("\nFAILURE: Could not trigger the uncovered default case.\n");
        return EXIT_FAILURE;
    }
}
