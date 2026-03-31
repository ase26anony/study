#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH 1024
#define MAX_OUTPUT 4096

/* Function to find gcov-dump executable */
static char *find_gcov_dump(void)
{
    static char path[MAX_PATH];
    
    /* Check environment variable first */
    char *env_path = getenv("GCOV_DUMP");
    if (env_path && access(env_path, X_OK) == 0) {
        strncpy(path, env_path, MAX_PATH - 1);
        path[MAX_PATH - 1] = '\0';
        return path;
    }
    
    /* Try common build locations */
    const char *candidates[] = {
        "./gcc/gcov-dump",
        "../gcc/gcov-dump",
        "../../gcc/gcov-dump",
        "../prev-gcc/build/gcc/gcov-dump",
        "../../prev-gcc/build/gcc/gcov-dump",
        "gcov-dump",  /* In PATH */
        NULL
    };
    
    for (int i = 0; candidates[i] != NULL; i++) {
        if (access(candidates[i], X_OK) == 0) {
            strncpy(path, candidates[i], MAX_PATH - 1);
            path[MAX_PATH - 1] = '\0';
            return path;
        }
    }
    
    return NULL;
}

/* Execute gcov-dump with given arguments and capture stderr */
static int test_gcov_dump(const char *gcov_dump_path, char *const argv[], 
                         char *output, size_t output_size)
{
    int pipefd[2];
    pid_t pid;
    
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
    
    if (pid == 0) { /* Child process */
        /* Redirect stderr to pipe */
        close(pipefd[0]);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        /* Execute gcov-dump */
        execvp(gcov_dump_path, argv);
        
        /* If we get here, exec failed */
        fprintf(stderr, "Failed to execute %s: %s\n", gcov_dump_path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    /* Parent process */
    close(pipefd[1]);
    
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

/* Test a specific invalid flag combination */
static int test_invalid_flag(const char *gcov_dump_path, 
                            const char *test_name,
                            char *const argv[])
{
    char output[MAX_OUTPUT];
    int exit_code;
    
    printf("Testing: %s\n", test_name);
    printf("Command: %s", gcov_dump_path);
    for (int i = 1; argv[i] != NULL; i++) {
        printf(" %s", argv[i]);
    }
    printf("\n");
    
    exit_code = test_gcov_dump(gcov_dump_path, (char **)argv, output, MAX_OUTPUT);
    
    printf("Exit code: %d\n", exit_code);
    printf("Stderr output:\n%s\n", output);
    
    /* Check for the error message */
    if (strstr(output, "unknown flag") != NULL) {
        printf("✓ Found 'unknown flag' message\n\n");
        return 1;  /* Success */
    } else {
        printf("✗ Did not find 'unknown flag' message\n\n");
        return 0;  /* Failure */
    }
}

int main(void)
{
    char *gcov_dump_path = find_gcov_dump();
    if (gcov_dump_path == NULL) {
        fprintf(stderr, "Error: Could not find gcov-dump executable\n");
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    int total_tests = 0;
    int passed_tests = 0;
    
    /* Test 1: Single invalid flag as first argument */
    {
        char *argv[] = { "gcov-dump", "-x", NULL };
        total_tests++;
        passed_tests += test_invalid_flag(gcov_dump_path, 
                                         "Single invalid flag '-x'", argv);
    }
    
    /* Test 2: Invalid flag between valid flags */
    {
        char *argv[] = { "gcov-dump", "-l", "-x", "-p", NULL };
        total_tests++;
        passed_tests += test_invalid_flag(gcov_dump_path,
                                         "Invalid flag '-x' between valid flags '-l -p'", argv);
    }
    
    /* Test 3: Multiple invalid flags */
    {
        char *argv[] = { "gcov-dump", "-z", "-?", "-@", NULL };
        total_tests++;
        passed_tests += test_invalid_flag(gcov_dump_path,
                                         "Multiple invalid flags '-z -? -@'", argv);
    }
    
    /* Test 4: Invalid flag after non-option argument (filename) */
    {
        char *argv[] = { "gcov-dump", "test.gcda", "-y", NULL };
        total_tests++;
        passed_tests += test_invalid_flag(gcov_dump_path,
                                         "Invalid flag '-y' after filename", argv);
    }
    
    /* Test 5: Double dash followed by invalid single-character flag */
    {
        char *argv[] = { "gcov-dump", "--", "-x", NULL };
        total_tests++;
        passed_tests += test_invalid_flag(gcov_dump_path,
                                         "Double dash '--' followed by '-x'", argv);
    }
    
    /* Test 6: Combination with valid flags and invalid flag at end */
    {
        char *argv[] = { "gcov-dump", "-v", "-l", "-r", "-z", NULL };
        total_tests++;
        passed_tests += test_invalid_flag(gcov_dump_path,
                                         "Valid flags '-v -l -r' followed by invalid '-z'", argv);
    }
    
    /* Test 7: Invalid flag with argument (edge case) */
    {
        char *argv[] = { "gcov-dump", "-xvalue", NULL };
        total_tests++;
        passed_tests += test_invalid_flag(gcov_dump_path,
                                         "Invalid flag with attached value '-xvalue'", argv);
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    
    if (passed_tests == total_tests) {
        printf("\n✓ All tests passed!\n");
        return EXIT_SUCCESS;
    } else {
        printf("\n✗ Some tests failed\n");
        return EXIT_FAILURE;
    }
}
