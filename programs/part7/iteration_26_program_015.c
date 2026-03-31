#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* Create a minimal valid .gcda file for testing */
void create_dummy_gcda(void) {
    /* Create a simple C source file */
    FILE *src = fopen("dummy.c", "w");
    if (src) {
        fprintf(src, "int main() { return 0; }\n");
        fclose(src);
    }
    
    /* Compile with coverage flags */
    system("gcc -fprofile-arcs -ftest-coverage -o dummy dummy.c 2>/dev/null");
    
    /* Run to generate .gcda file */
    system("./dummy 2>/dev/null");
    
    /* Clean up intermediate files */
    remove("dummy.c");
    remove("dummy");
}

/* Execute gcov-dump using execvp with precise argument control */
void test_with_execvp(const char *args[]) {
    pid_t pid = fork();
    
    if (pid == 0) {
        /* Child process */
        execvp("gcov-dump", (char *const *)args);
        /* If execvp fails */
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        /* Parent process - wait for child */
        int status;
        waitpid(pid, &status, 0);
        printf("Exit status: %d\n", WEXITSTATUS(status));
    } else {
        perror("fork failed");
    }
}

/* Execute using system() for shell interpretation tests */
void test_with_system(const char *cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    printf("System exit status: %d\n\n", WEXITSTATUS(status));
}

int main(void) {
    printf("=== Testing gcov-dump flag parsing ===\n\n");
    
    /* Create dummy .gcda file for file-based tests */
    create_dummy_gcda();
    
    /* Set environment variable that might affect parsing */
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    
    /* Test 1: Individual flags (direct switch cases) */
    printf("--- Individual flag tests ---\n");
    
    const char *help_args[] = {"gcov-dump", "-h", NULL};
    test_with_execvp(help_args);
    
    const char *version_args[] = {"gcov-dump", "-v", NULL};
    test_with_execvp(version_args);
    
    const char *contents_args[] = {"gcov-dump", "-l", NULL};
    test_with_execvp(contents_args);
    
    const char *positions_args[] = {"gcov-dump", "-p", NULL};
    test_with_execvp(positions_args);
    
    const char *raw_args[] = {"gcov-dump", "-r", NULL};
    test_with_execvp(raw_args);
    
    const char *stable_args[] = {"gcov-dump", "-s", NULL};
    test_with_execvp(stable_args);
    
    /* Test 2: Invalid flag (default case) */
    printf("\n--- Invalid flag test (default case) ---\n");
    const char *invalid_args[] = {"gcov-dump", "-x", NULL};
    test_with_execvp(invalid_args);
    
    /* Test 3: No arguments */
    printf("\n--- No arguments test ---\n");
    const char *no_args[] = {"gcov-dump", NULL};
    test_with_execvp(no_args);
    
    /* Test 4: Combined valid flags in single command */
    printf("\n--- Combined valid flags ---\n");
    const char *combined1[] = {"gcov-dump", "-l", "-p", NULL};
    test_with_execvp(combined1);
    
    const char *combined2[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    test_with_execvp(combined2);
    
    const char *combined3[] = {"gcov-dump", "-h", "-l", NULL};
    test_with_execvp(combined3);
    
    /* Test 5: Repeated flags */
    printf("\n--- Repeated flags ---\n");
    const char *repeated[] = {"gcov-dump", "-p", "-p", NULL};
    test_with_execvp(repeated);
    
    /* Test 6: With positional arguments (gcov files) */
    printf("\n--- With file arguments ---\n");
    const char *with_file1[] = {"gcov-dump", "-l", "dummy.gcda", NULL};
    test_with_execvp(with_file1);
    
    const char *with_file2[] = {"gcov-dump", "-p", "-r", "dummy.gcda", NULL};
    test_with_execvp(with_file2);
    
    /* Test 7: Using -- delimiter */
    printf("\n--- With -- delimiter ---\n");
    const char *with_delim[] = {"gcov-dump", "-l", "--", "dummy.gcda", NULL};
    test_with_execvp(with_delim);
    
    /* Test 8: Using system() for shell-style parsing */
    printf("\n--- Shell-style parsing tests (system()) ---\n");
    
    /* Combined short options (if supported by getopt) */
    test_with_system("gcov-dump -lp");
    test_with_system("gcov-dump -rsv");
    
    /* With output redirection to capture stderr */
    test_with_system("gcov-dump -x 2>&1");
    
    /* Complex combination with file */
    test_with_system("gcov-dump -l -p dummy.gcda 2>&1");
    
    /* Test with environment variable cleared */
    printf("\n--- With cleared environment ---\n");
    unsetenv("GCOV_DUMP_OPTIONS");
    const char *cleared_env[] = {"gcov-dump", "-v", NULL};
    test_with_execvp(cleared_env);
    
    /* Test 9: Edge cases */
    printf("\n--- Edge cases ---\n");
    
    /* Empty string as argument */
    const char *edge1[] = {"gcov-dump", "", NULL};
    test_with_execvp(edge1);
    
    /* Multiple invalid flags */
    const char *edge2[] = {"gcov-dump", "-x", "-y", "-z", NULL};
    test_with_execvp(edge2);
    
    /* Mix valid and invalid */
    const char *edge3[] = {"gcov-dump", "-l", "-x", "-p", NULL};
    test_with_execvp(edge3);
    
    /* Clean up */
    remove("dummy.gcda");
    remove("dummy.gcno");
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
