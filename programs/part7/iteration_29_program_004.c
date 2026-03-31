#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH 1024
#define MAX_OUTPUT 4096

/* Test cases for invalid flags */
typedef struct {
    const char *description;
    const char *args[8];  /* NULL-terminated array of arguments */
} test_case;

/* Test cases designed to trigger the default case in the switch statement */
static test_case test_cases[] = {
    /* Single invalid flag */
    {"Single invalid flag -x", {"-x", NULL}},
    {"Single invalid flag -z", {"-z", NULL}},
    {"Single invalid flag -?", {"-?", NULL}},
    
    /* Invalid flag mixed with valid flags */
    {"Valid -l then invalid -x", {"-l", "-x", NULL}},
    {"Invalid -x between valid -l and -p", {"-l", "-x", "-p", NULL}},
    {"Multiple invalid flags", {"-x", "-y", "-z", NULL}},
    
    /* Invalid flag after non-option argument */
    {"Invalid flag after filename", {"test.gcda", "-x", NULL}},
    
    /* Double dash with invalid single char (getopt behavior test) */
    {"Double dash with invalid flag", {"--", "-x", NULL}},
    
    /* Edge case: just a dash */
    {"Single dash only", {"-", NULL}},
    
    /* NULL terminator for array */
    {NULL, {NULL}}
};

/* Get the path to gcov-dump executable */
static char *get_gcov_dump_path(void) {
    char *path = getenv("GCOV_DUMP");
    
    if (path && access(path, X_OK) == 0) {
        return strdup(path);
    }
    
    /* Try common locations in GCC build tree */
    const char *common_paths[] = {
        "./gcc/gcov-dump",
        "./gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    for (int i = 0; common_paths[i]; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            return strdup(common_paths[i]);
        }
    }
    
    return NULL;
}

/* Execute gcov-dump with given arguments and capture stderr */
static int execute_and_capture(const char *program, char **args, 
                               char *output, size_t output_size) {
    int pipefd[2];
    pid_t pid;
    
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
        /* Redirect stderr to pipe */
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        
        /* Execute gcov-dump */
        execvp(program, args);
        
        /* If we get here, exec failed */
        fprintf(stderr, "Failed to execute %s: %s\n", program, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    /* Parent process */
    close(pipefd[1]);  /* Close write end */
    
    /* Read stderr output */
    ssize_t bytes_read = read(pipefd[0], output, output_size - 1);
    if (bytes_read > 0) {
        output[bytes_read] = '\0';
    } else {
        output[0] = '\0';
    }
    
    close(pipefd[0]);
    
    /* Wait for child */
    int status;
    waitpid(pid, &status, 0);
    
    return WEXITSTATUS(status);
}

/* Check if output contains the target error message */
static int contains_unknown_flag_error(const char *output) {
    /* Look for the exact error string or variations */
    const char *patterns[] = {
        "unknown flag",
        "unknown flag `",
        "unknown option",
        NULL
    };
    
    for (int i = 0; patterns[i]; i++) {
        if (strstr(output, patterns[i])) {
            return 1;
        }
    }
    
    return 0;
}

/* Run a single test case */
static int run_test_case(const char *program, const test_case *tc) {
    char output[MAX_OUTPUT];
    char *args[16];
    int arg_count = 0;
    
    /* Build argument array */
    args[arg_count++] = (char *)program;
    
    for (int i = 0; tc->args[i] && arg_count < 15; i++) {
        args[arg_count++] = (char *)tc->args[i];
    }
    args[arg_count] = NULL;
    
    printf("Test: %s\n", tc->description);
    printf("  Command: %s", program);
    for (int i = 1; i < arg_count; i++) {
        printf(" %s", args[i]);
    }
    printf("\n");
    
    int exit_code = execute_and_capture(program, args, output, sizeof(output));
    
    printf("  Exit code: %d\n", exit_code);
    
    if (strlen(output) > 0) {
        printf("  Stderr output: %s", output);
    }
    
    int found_error = contains_unknown_flag_error(output);
    printf("  Found 'unknown flag' error: %s\n\n", found_error ? "YES" : "NO");
    
    return found_error;
}

int main(void) {
    char *gcov_dump_path = get_gcov_dump_path();
    
    if (!gcov_dump_path) {
        fprintf(stderr, "Error: gcov-dump executable not found.\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in PATH.\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    int total_tests = 0;
    int passed_tests = 0;
    
    /* Run all test cases */
    for (int i = 0; test_cases[i].description; i++) {
        total_tests++;
        if (run_test_case(gcov_dump_path, &test_cases[i])) {
            passed_tests++;
        }
    }
    
    /* Additional test: Create a dummy .gcda file for filename-based tests */
    FILE *dummy = fopen("test_dummy.gcda", "wb");
    if (dummy) {
        /* Write minimal valid header for gcov file */
        unsigned int magic = 0x67636461; /* "gcda" in little endian */
        unsigned int version = 0x4020000; /* Typical version */
        fwrite(&magic, sizeof(magic), 1, dummy);
        fwrite(&version, sizeof(version), 1, dummy);
        fclose(dummy);
        
        /* Test with actual file */
        test_case file_test = {
            "Invalid flag with actual .gcda file",
            {"-x", "test_dummy.gcda", NULL}
        };
        
        total_tests++;
        printf("Additional test with dummy .gcda file:\n");
        if (run_test_case(gcov_dump_path, &file_test)) {
            passed_tests++;
        }
        
        /* Clean up */
        remove("test_dummy.gcda");
    }
    
    printf("\n=== Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Tests triggering 'unknown flag' error: %d\n", passed_tests);
    
    free(gcov_dump_path);
    
    /* Consider success if at least one test triggered the error */
    return (passed_tests > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
