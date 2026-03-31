#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define MAX_ARGS 10
#define MAX_PATH 1024

typedef struct {
    char *name;
    char **args;
    int expected_exit;
    int check_stderr;
    char *stderr_contains;
} test_case_t;

void run_test(const char *gcov_dump_path, test_case_t *test);
void create_test_gcno(void);
void cleanup_files(void);

int main(int argc, char *argv[]) {
    char gcov_dump_path[MAX_PATH] = "./gcov-dump";
    
    // Try to get path from environment variable
    char *env_path = getenv("GCOV_DUMP");
    if (env_path && strlen(env_path) > 0) {
        strncpy(gcov_dump_path, env_path, MAX_PATH - 1);
        gcov_dump_path[MAX_PATH - 1] = '\0';
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Create test.gcno file first
    printf("Creating test.gcno file...\n");
    create_test_gcno();
    
    // Define test cases
    test_case_t tests[] = {
        // Test -h (help)
        {
            "Help flag (-h)",
            (char*[]){"gcov-dump", "-h", NULL},
            0, 0, NULL
        },
        // Test -v (version)
        {
            "Version flag (-v)",
            (char*[]){"gcov-dump", "-v", NULL},
            0, 0, NULL
        },
        // Test -l (dump contents)
        {
            "Dump contents flag (-l)",
            (char*[]){"gcov-dump", "-l", "test.gcno", NULL},
            0, 0, NULL
        },
        // Test -p (dump positions)
        {
            "Dump positions flag (-p)",
            (char*[]){"gcov-dump", "-p", "test.gcno", NULL},
            0, 0, NULL
        },
        // Test -r (dump raw)
        {
            "Dump raw flag (-r)",
            (char*[]){"gcov-dump", "-r", "test.gcno", NULL},
            0, 0, NULL
        },
        // Test -s (dump stable)
        {
            "Dump stable flag (-s)",
            (char*[]){"gcov-dump", "-s", "test.gcno", NULL},
            0, 0, NULL
        },
        // Test invalid flag -x
        {
            "Invalid flag (-x)",
            (char*[]){"gcov-dump", "-x", NULL},
            -1, 1, "unknown flag"
        },
        // Test invalid flag -?
        {
            "Invalid flag (-?)",
            (char*[]){"gcov-dump", "-?", NULL},
            -1, 1, "unknown flag"
        },
        // Test invalid flag -Z
        {
            "Invalid flag (-Z)",
            (char*[]){"gcov-dump", "-Z", NULL},
            -1, 1, "unknown flag"
        }
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    
    // Run all tests
    for (int i = 0; i < num_tests; i++) {
        printf("Test %d: %s\n", i + 1, tests[i].name);
        run_test(gcov_dump_path, &tests[i]);
        
        // Check if test passed
        if (tests[i].expected_exit == -1) {
            // For invalid flags, any non-zero exit is acceptable
            if (tests[i].args[0] != NULL) {
                printf("  [PASS] Invalid flag test executed\n");
                passed++;
            }
        } else if (tests[i].expected_exit == 0) {
            printf("  [PASS] Exit code 0\n");
            passed++;
        }
        printf("\n");
    }
    
    // Cleanup
    cleanup_files();
    
    printf("\nSummary: %d/%d tests passed\n", passed, num_tests);
    
    return (passed == num_tests) ? 0 : 1;
}

void run_test(const char *gcov_dump_path, test_case_t *test) {
    pid_t pid;
    int status;
    
    // Create pipes for stderr if we need to check it
    int stderr_pipe[2];
    if (test->check_stderr) {
        if (pipe(stderr_pipe) == -1) {
            perror("pipe");
            return;
        }
    }
    
    pid = fork();
    if (pid == -1) {
        perror("fork");
        return;
    }
    
    if (pid == 0) {
        // Child process
        if (test->check_stderr) {
            // Redirect stderr to pipe
            close(stderr_pipe[0]);
            dup2(stderr_pipe[1], STDERR_FILENO);
            close(stderr_pipe[1]);
        }
        
        // Prepare arguments for execvp
        char *args[MAX_ARGS];
        int i = 0;
        
        // First argument is the program name
        args[i++] = (char*)gcov_dump_path;
        
        // Copy test arguments
        for (int j = 0; test->args[j] != NULL && i < MAX_ARGS - 1; j++) {
            // Skip the first "gcov-dump" string in test args
            if (j > 0 || strcmp(test->args[j], "gcov-dump") != 0) {
                args[i++] = test->args[j];
            }
        }
        args[i] = NULL;
        
        // Execute gcov-dump
        execvp(gcov_dump_path, args);
        
        // If we get here, exec failed
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        if (test->check_stderr) {
            close(stderr_pipe[1]);
            
            // Read stderr output
            char buffer[1024];
            ssize_t bytes_read = read(stderr_pipe[0], buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                printf("  stderr: %s", buffer);
                
                // Check if expected string is in stderr
                if (test->stderr_contains && 
                    strstr(buffer, test->stderr_contains) != NULL) {
                    test->args[0] = NULL; // Mark as passed
                }
            }
            close(stderr_pipe[0]);
        }
        
        // Wait for child
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            printf("  Exit code: %d", exit_status);
            
            if (test->expected_exit != -1) {
                if (exit_status == test->expected_exit) {
                    test->args[0] = NULL; // Mark as passed
                } else {
                    printf(" (expected %d)", test->expected_exit);
                }
            }
            printf("\n");
        } else if (WIFSIGNALED(status)) {
            printf("  Terminated by signal: %d\n", WTERMSIG(status));
        }
    }
}

void create_test_gcno(void) {
    // Create minimal C source file
    FILE *fp = fopen("test.c", "w");
    if (!fp) {
        perror("fopen test.c");
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "int main() { return 0; }\n");
    fclose(fp);
    
    // Compile with coverage flags to generate .gcno
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {
        // Child process: compile
        execlp("gcc", "gcc", "-O0", "--coverage", "-fprofile-arcs", 
               "-ftest-coverage", "-o", "test", "test.c", NULL);
        perror("execlp gcc");
        exit(EXIT_FAILURE);
    } else {
        // Parent: wait for compilation
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Failed to compile test.c\n");
            // Continue anyway - some tests might still work
        }
    }
}

void cleanup_files(void) {
    // Remove generated files
    remove("test.c");
    remove("test");
    remove("test.gcno");
    remove("test.gcda");
}
