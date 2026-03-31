/**
 * gcov-dump_parser_test.c
 * 
 * A comprehensive test program for exercising gcov-dump's command-line
 * flag parsing logic, specifically targeting uncovered switch cases.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/**
 * Creates a minimal valid .gcda file for testing.
 * Returns 0 on success, -1 on failure.
 */
static int create_dummy_gcda(void) {
    /* Create a trivial C source file */
    FILE *src = fopen("dummy_test.c", "w");
    if (!src) {
        perror("fopen dummy_test.c");
        return -1;
    }
    fprintf(src, "int main() { return 0; }\n");
    fclose(src);
    
    /* Compile with coverage flags */
    int compile_status = system("gcc -fprofile-arcs -ftest-coverage -O0 "
                                "dummy_test.c -o dummy_test 2>/dev/null");
    if (compile_status != 0) {
        fprintf(stderr, "Warning: Failed to compile dummy_test.c\n");
        /* Try with clang as fallback */
        compile_status = system("clang -fprofile-arcs -ftest-coverage -O0 "
                                "dummy_test.c -o dummy_test 2>/dev/null");
        if (compile_status != 0) {
            fprintf(stderr, "Could not create dummy .gcda file\n");
            return -1;
        }
    }
    
    /* Run the program to generate .gcda file */
    int run_status = system("./dummy_test 2>/dev/null");
    if (run_status != 0) {
        fprintf(stderr, "Warning: dummy_test execution failed\n");
    }
    
    return 0;
}

/**
 * Execute gcov-dump with given arguments using execvp.
 * Returns exit status of gcov-dump.
 */
static int exec_gcov_dump(const char *args[]) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        execvp("gcov-dump", (char * const *)args);
        /* If execvp returns, it failed */
        fprintf(stderr, "execvp failed for gcov-dump: %s\n", strerror(errno));
        exit(127);
    } else {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return -1;
        }
    }
}

/**
 * Execute gcov-dump using system() call.
 */
static void system_gcov_dump(const char *cmd) {
    printf("Testing with system(): %s\n", cmd);
    int status = system(cmd);
    printf("Exit status: %d\n\n", status);
}

int main(void) {
    printf("=== Starting gcov-dump parser tests ===\n\n");
    
    /* Create dummy .gcda file for file-based tests */
    if (create_dummy_gcda() != 0) {
        fprintf(stderr, "Note: Some file-based tests may be skipped\n");
    }
    
    /* Test individual flags (uncovered switch cases) */
    printf("--- Testing individual flags ---\n");
    
    /* Help flag */
    const char *help_args[] = {"gcov-dump", "-h", NULL};
    exec_gcov_dump(help_args);
    
    /* Version flag */
    const char *version_args[] = {"gcov-dump", "-v", NULL};
    exec_gcov_dump(version_args);
    
    /* Contents dump flag */
    const char *contents_args[] = {"gcov-dump", "-l", NULL};
    exec_gcov_dump(contents_args);
    
    /* Positions dump flag */
    const char *positions_args[] = {"gcov-dump", "-p", NULL};
    exec_gcov_dump(positions_args);
    
    /* Raw dump flag */
    const char *raw_args[] = {"gcov-dump", "-r", NULL};
    exec_gcov_dump(raw_args);
    
    /* Stable dump flag */
    const char *stable_args[] = {"gcov-dump", "-s", NULL};
    exec_gcov_dump(stable_args);
    
    /* Invalid flag (triggers default case) */
    const char *invalid_args[] = {"gcov-dump", "-x", NULL};
    exec_gcov_dump(invalid_args);
    
    /* Test flag combinations */
    printf("\n--- Testing flag combinations ---\n");
    
    /* Multiple valid flags */
    const char *combo1[] = {"gcov-dump", "-l", "-p", NULL};
    exec_gcov_dump(combo1);
    
    const char *combo2[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    exec_gcov_dump(combo2);
    
    /* Help with other flags (may exit early) */
    const char *combo3[] = {"gcov-dump", "-h", "-l", NULL};
    exec_gcov_dump(combo3);
    
    /* Repeated flag */
    const char *repeat_args[] = {"gcov-dump", "-p", "-p", NULL};
    exec_gcov_dump(repeat_args);
    
    /* Test different syntactic styles */
    printf("\n--- Testing different syntactic styles ---\n");
    
    /* Combined short options (if supported) */
    const char *combined_args[] = {"gcov-dump", "-lp", NULL};
    exec_gcov_dump(combined_args);
    
    /* With positional arguments (gcov files) */
    const char *with_file[] = {"gcov-dump", "-l", "dummy_test.gcda", NULL};
    exec_gcov_dump(with_file);
    
    /* With -- delimiter */
    const char *with_delimiter[] = {"gcov-dump", "-l", "--", "dummy_test.gcda", NULL};
    exec_gcov_dump(with_delimiter);
    
    /* Test environment and error contexts */
    printf("\n--- Testing environment and error contexts ---\n");
    
    /* No arguments */
    const char *no_args[] = {"gcov-dump", NULL};
    exec_gcov_dump(no_args);
    
    /* Set environment variable (if supported) */
    printf("Setting GCOV_DUMP_OPTIONS environment variable\n");
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    exec_gcov_dump(no_args);
    unsetenv("GCOV_DUMP_OPTIONS");
    
    /* Test with system() calls for different path */
    printf("\n--- Testing with system() calls ---\n");
    
    /* Basic flag tests with system() */
    system_gcov_dump("gcov-dump -h 2>&1");
    system_gcov_dump("gcov-dump -v 2>&1");
    system_gcov_dump("gcov-dump -l 2>&1");
    
    /* Invalid flag with stderr redirection */
    system_gcov_dump("gcov-dump -x 2>&1");
    
    /* Complex combination with system() */
    system_gcov_dump("gcov-dump -l -p -r -s 2>&1");
    
    /* Test with file argument */
    system_gcov_dump("gcov-dump -l dummy_test.gcda 2>&1");
    
    /* Test error output specifically */
    printf("\n--- Testing stderr output for invalid flag ---\n");
    system("gcov-dump -x 2>&1 | grep -q 'unknown flag' && "
           "echo 'Successfully triggered unknown flag message'");
    
    /* Cleanup */
    printf("\n=== Cleaning up test files ===\n");
    system("rm -f dummy_test.c dummy_test dummy_test.gcda "
           "dummy_test.gcno dummy_test.gcov 2>/dev/null");
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
