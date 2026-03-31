#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH_LEN 1024
#define MAX_OUTPUT_LEN 4096

/* Test cases for invalid flags */
typedef struct {
    const char *description;
    const char *args[10];  /* NULL-terminated array of arguments */
} test_case_t;

/* Test cases designed to trigger the default case in the switch statement */
static test_case_t test_cases[] = {
    /* Single invalid flag */
    {"Single invalid flag -x", {"-x", NULL}},
    {"Single invalid flag -z", {"-z", NULL}},
    {"Single invalid flag -?", {"-?", NULL}},
    
    /* Invalid flag combined with valid flags */
    {"Valid -l followed by invalid -x", {"-l", "-x", NULL}},
    {"Invalid -x between valid -l and -p", {"-l", "-x", "-p", NULL}},
    {"Multiple invalid flags", {"-x", "-y", "-z", NULL}},
    
    /* Invalid flag after non-option argument */
    {"Invalid flag after filename", {"test.gcda", "-x", NULL}},
    
    /* Double dash with invalid single char */
    {"Double dash with invalid flag", {"--x", NULL}},
    
    /* Edge case: invalid flag as first char after dash */
    {"Invalid flag with combined format", {"-lxpr", NULL}},
    
    /* NULL terminator for array */
    {NULL, {NULL}}
};

/* Get the path to gcov-dump executable */
static char *get_gcov_dump_path(void) {
    static char path[MAX_PATH_LEN];
    const char *env_path;
    
    /* Try environment variable first */
    env_path = getenv("GCOV_DUMP");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, MAX_PATH_LEN - 1);
        path[MAX_PATH_LEN - 1] = '\0';
        return path;
    }
    
    /* Try common build locations */
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            strncpy(path, common_paths[i], MAX_PATH_LEN - 1);
            path[MAX_PATH_LEN - 1] = '\0';
            return path;
        }
    }
    
    return NULL;
}

/* Execute gcov-dump with given arguments and capture stderr */
static int execute_and_capture(const char *program, const char *args[], 
                               char *output, size_t output_size) {
    int pipefd[2];
    pid_t pid;
    int status;
    ssize_t bytes_read;
    
    /* Create pipe for stderr */
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
    
    if (pid == 0) {  /* Child process */
        /* Close read end of pipe */
        close(pipefd[0]);
        
        /* Redirect stderr to pipe */
        if (dup2(pipefd[1], STDERR_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(pipefd[1]);
        
        /* Prepare arguments for execvp */
        int arg_count = 0;
        while (args[arg_count] != NULL) {
            arg_count++;
        }
        
        char **exec_args = malloc((arg_count + 2) * sizeof(char *));
        if (exec_args == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        
        exec_args[0] = (char *)program;
        for (int i = 0; i < arg_count; i++) {
            exec_args[i + 1] = (char *)args[i];
        }
        exec_args[arg_count + 1] = NULL;
        
        /* Execute gcov-dump */
        execvp(program, exec_args);
        
        /* If we get here, exec failed */
        perror("execvp");
        free(exec_args);
        exit(EXIT_FAILURE);
    } else {  /* Parent process */
        /* Close write end of pipe */
        close(pipefd[1]);
        
        /* Read stderr output */
        bytes_read = read(pipefd[0], output, output_size - 1);
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
        } else {
            output[0] = '\0';
        }
        close(pipefd[0]);
        
        /* Wait for child to finish */
        waitpid(pid, &status, 0);
        
        return WEXITSTATUS(status);
    }
}

/* Check if output contains the target error message */
static int contains_unknown_flag_error(const char *output) {
    /* Look for various forms of the error message */
    const char *patterns[] = {
        "unknown flag",
        "unknown flag `",
        "unknown flag '",
        NULL
    };
    
    for (int i = 0; patterns[i] != NULL; i++) {
        if (strstr(output, patterns[i]) != NULL) {
            return 1;
        }
    }
    
    return 0;
}

/* Run a single test case */
static int run_test_case(const char *program, const test_case_t *test) {
    char output[MAX_OUTPUT_LEN];
    int exit_status;
    
    printf("Running test: %s\n", test->description);
    printf("  Arguments: ");
    for (int i = 0; test->args[i] != NULL; i++) {
        printf("%s ", test->args[i]);
    }
    printf("\n");
    
    exit_status = execute_and_capture(program, test->args, output, MAX_OUTPUT_LEN);
    
    if (strlen(output) > 0) {
        printf("  stderr output: %s", output);
    }
    
    if (contains_unknown_flag_error(output)) {
        printf("  ✓ Success: Triggered 'unknown flag' error\n");
        return 1;
    } else {
        printf("  ✗ Failure: Did not trigger expected error\n");
        printf("    Exit status: %d\n", exit_status);
        return 0;
    }
}

int main(void) {
    char *gcov_dump_path;
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("=== Testing gcov-dump invalid flag handling ===\n\n");
    
    /* Get path to gcov-dump */
    gcov_dump_path = get_gcov_dump_path();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    /* Run all test cases */
    for (int i = 0; test_cases[i].description != NULL; i++) {
        total_tests++;
        if (run_test_case(gcov_dump_path, &test_cases[i])) {
            passed_tests++;
        }
        printf("\n");
    }
    
    /* Summary */
    printf("=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", total_tests - passed_tests);
    
    if (passed_tests > 0) {
        printf("\n✓ Successfully triggered the uncovered default case in gcov-dump!\n");
        return EXIT_SUCCESS;
    } else {
        printf("\n✗ Failed to trigger the uncovered default case\n");
        return EXIT_FAILURE;
    }
}
