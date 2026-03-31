#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEST_SOURCE "test.c"
#define TEST_BINARY "test"
#define TEST_GCNO "test.gcno"
#define GCOV_DUMP_PATH "./gcov-dump"

typedef struct {
    const char *name;
    const char **args;
    int expected_exit;
    const char *expected_stderr;
} test_case_t;

void create_test_source(void) {
    FILE *f = fopen(TEST_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(1);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

void compile_with_coverage(void) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: compile with coverage
        execlp("gcc", "gcc", "-O0", "--coverage", "-fprofile-arcs", 
               "-ftest-coverage", "-o", TEST_BINARY, TEST_SOURCE, NULL);
        perror("Failed to execute gcc");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Failed to compile test program\n");
            exit(1);
        }
    } else {
        perror("fork failed");
        exit(1);
    }
}

int run_gcov_dump(const char **args, char *stderr_output, size_t stderr_size) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: run gcov-dump
        int dev_null = open("/dev/null", O_WRONLY);
        if (dev_null >= 0) {
            dup2(dev_null, STDOUT_FILENO);  // Redirect stdout to /dev/null
            close(dev_null);
        }
        
        // Create pipe for stderr
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("pipe failed");
            exit(1);
        }
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        execvp(GCOV_DUMP_PATH, (char *const *)args);
        perror("Failed to execute gcov-dump");
        exit(1);
    } else if (pid > 0) {
        int status;
        
        // Read stderr from pipe
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("pipe failed");
            exit(1);
        }
        
        waitpid(pid, &status, 0);
        
        // Read stderr output
        ssize_t bytes_read = read(pipefd[0], stderr_output, stderr_size - 1);
        if (bytes_read > 0) {
            stderr_output[bytes_read] = '\0';
        } else {
            stderr_output[0] = '\0';
        }
        close(pipefd[0]);
        
        return status;
    } else {
        perror("fork failed");
        exit(1);
    }
}

int main(void) {
    // Check if gcov-dump exists
    if (access(GCOV_DUMP_PATH, X_OK) != 0) {
        fprintf(stderr, "gcov-dump not found at %s\n", GCOV_DUMP_PATH);
        return 1;
    }
    
    // Create test source and compile with coverage
    printf("Creating test source file...\n");
    create_test_source();
    
    printf("Compiling with coverage flags...\n");
    compile_with_coverage();
    
    // Define test cases
    const char *args_h[] = {GCOV_DUMP_PATH, "-h", NULL};
    const char *args_v[] = {GCOV_DUMP_PATH, "-v", NULL};
    const char *args_l[] = {GCOV_DUMP_PATH, "-l", TEST_GCNO, NULL};
    const char *args_p[] = {GCOV_DUMP_PATH, "-p", TEST_GCNO, NULL};
    const char *args_r[] = {GCOV_DUMP_PATH, "-r", TEST_GCNO, NULL};
    const char *args_s[] = {GCOV_DUMP_PATH, "-s", TEST_GCNO, NULL};
    const char *args_x[] = {GCOV_DUMP_PATH, "-x", NULL};
    const char *args_z[] = {GCOV_DUMP_PATH, "-Z", NULL};
    const char *args_q[] = {GCOV_DUMP_PATH, "-?", NULL};
    
    test_case_t tests[] = {
        {"-h (help)", args_h, 0, ""},
        {"-v (version)", args_v, 0, ""},
        {"-l (dump contents)", args_l, 0, ""},
        {"-p (dump positions)", args_p, 0, ""},
        {"-r (dump raw)", args_r, 0, ""},
        {"-s (dump stable)", args_s, 0, ""},
        {"-x (unknown flag)", args_x, 1, "unknown flag `x'"},
        {"-Z (unknown flag)", args_z, 1, "unknown flag `Z'"},
        {"-? (unknown flag)", args_q, 1, "unknown flag `?'"},
    };
    
    int total_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    
    printf("\nRunning gcov-dump tests:\n");
    printf("========================\n");
    
    for (int i = 0; i < total_tests; i++) {
        printf("Test %d/%d: %s... ", i + 1, total_tests, tests[i].name);
        fflush(stdout);
        
        char stderr_buf[256];
        int status = run_gcov_dump(tests[i].args, stderr_buf, sizeof(stderr_buf));
        
        int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
        
        // Check if test passed
        int passed_test = 1;
        
        // Check exit code
        if (exit_code != tests[i].expected_exit) {
            printf("FAIL (exit code %d, expected %d)\n", 
                   exit_code, tests[i].expected_exit);
            passed_test = 0;
        }
        // Check stderr for unknown flag tests
        else if (tests[i].expected_stderr[0] != '\0') {
            if (strstr(stderr_buf, tests[i].expected_stderr) == NULL) {
                printf("FAIL (stderr mismatch)\n");
                printf("  Expected to contain: '%s'\n", tests[i].expected_stderr);
                printf("  Got: '%s'\n", stderr_buf);
                passed_test = 0;
            } else {
                printf("PASS\n");
            }
        } else {
            printf("PASS\n");
        }
        
        if (passed_test) {
            passed++;
        }
    }
    
    // Cleanup
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    unlink(TEST_GCNO);
    unlink("test.gcda");  // In case it was created
    
    printf("\nSummary: %d/%d tests passed\n", passed, total_tests);
    
    return (passed == total_tests) ? 0 : 1;
}
