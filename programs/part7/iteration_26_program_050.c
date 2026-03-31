/**
 * gcov-dump_parser_test.c
 * 
 * Comprehensive test program for gcov-dump command-line flag parsing.
 * Specifically targets the switch statement in gcov-dump.cc lines 111-130.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/**
 * Creates a minimal C program, compiles it with coverage flags,
 * runs it to generate a .gcda file for testing.
 */
static int create_dummy_gcda(void) {
    FILE *fp;
    int status;
    pid_t pid;
    
    /* Create a minimal C source file */
    fp = fopen("dummy_test.c", "w");
    if (!fp) {
        perror("fopen dummy_test.c");
        return -1;
    }
    fprintf(fp, "int main() { return 0; }\n");
    fclose(fp);
    
    /* Compile with coverage flags */
    pid = fork();
    if (pid == 0) {
        /* Child process: compile */
        execlp("gcc", "gcc", "-fprofile-arcs", "-ftest-coverage", 
               "dummy_test.c", "-o", "dummy_test", NULL);
        perror("execlp gcc");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Compilation failed\n");
            return -1;
        }
    } else {
        perror("fork");
        return -1;
    }
    
    /* Run the program to generate .gcda */
    pid = fork();
    if (pid == 0) {
        execl("./dummy_test", "./dummy_test", NULL);
        perror("execl ./dummy_test");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
    } else {
        perror("fork");
        return -1;
    }
    
    return 0;
}

/**
 * Execute gcov-dump with given arguments using execvp.
 * Returns the exit status of gcov-dump.
 */
static int exec_gcov_dump(const char *args[]) {
    pid_t pid;
    int status;
    
    pid = fork();
    if (pid == 0) {
        /* Child process */
        execvp("gcov-dump", (char *const *)args);
        /* If execvp returns, it failed */
        fprintf(stderr, "execvp failed for gcov-dump: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        /* Parent process */
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return -1;
        }
    } else {
        perror("fork");
        return -1;
    }
}

/**
 * Execute gcov-dump using system() call.
 */
static void system_gcov_dump(const char *cmd) {
    int status = system(cmd);
    if (status == -1) {
        fprintf(stderr, "system() failed for: %s\n", cmd);
    } else if (WIFEXITED(status)) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
    }
}

int main(void) {
    int i, j;
    
    printf("=== Starting gcov-dump flag parser tests ===\n\n");
    
    /* Create dummy .gcda file for file-based tests */
    if (create_dummy_gcda() != 0) {
        fprintf(stderr, "Warning: Could not create dummy .gcda file\n");
        fprintf(stderr, "File-based tests will use placeholder\n");
    }
    
    /******************************************************************
     * Test 1: Individual flag cases (direct switch coverage)
     ******************************************************************/
    printf("--- Test 1: Individual flag cases ---\n");
    
    /* Array of individual flag tests for execvp */
    const char *individual_flags[][3] = {
        {"gcov-dump", "-h", NULL},      /* Help flag - line 111 */
        {"gcov-dump", "-v", NULL},      /* Version flag - line 114 */
        {"gcov-dump", "-l", NULL},      /* Contents dump - line 117 */
        {"gcov-dump", "-p", NULL},      /* Positions dump - line 120 */
        {"gcov-dump", "-r", NULL},      /* Raw dump - line 123 */
        {"gcov-dump", "-s", NULL},      /* Stable dump - line 126 */
        {"gcov-dump", "-x", NULL},      /* Invalid flag - default case line 129 */
        {NULL}
    };
    
    for (i = 0; individual_flags[i][0] != NULL; i++) {
        printf("Testing: ");
        for (j = 0; individual_flags[i][j] != NULL; j++) {
            printf("%s ", individual_flags[i][j]);
        }
        printf("\n");
        
        int status = exec_gcov_dump(individual_flags[i]);
        printf("Exit code: %d\n\n", status);
    }
    
    /******************************************************************
     * Test 2: Flag combinations (sequential parsing)
     ******************************************************************/
    printf("--- Test 2: Flag combinations ---\n");
    
    const char *combinations[][5] = {
        {"gcov-dump", "-l", "-p", NULL},           /* Two valid flags */
        {"gcov-dump", "-r", "-s", "-v", NULL},     /* Three flags including -v */
        {"gcov-dump", "-h", "-l", NULL},           /* -h with another flag */
        {"gcov-dump", "-p", "-p", NULL},           /* Repeated flag */
        {"gcov-dump", "-l", "-p", "-r", "-s", NULL}, /* All dump flags */
        {NULL}
    };
    
    for (i = 0; combinations[i][0] != NULL; i++) {
        printf("Testing: ");
        for (j = 0; combinations[i][j] != NULL; j++) {
            printf("%s ", combinations[i][j]);
        }
        printf("\n");
        
        exec_gcov_dump(combinations[i]);
        printf("\n");
    }
    
    /******************************************************************
     * Test 3: Different flag syntax styles (using system())
     ******************************************************************/
    printf("--- Test 3: Different flag syntax styles (system calls) ---\n");
    
    const char *syntax_tests[] = {
        /* Separate arguments */
        "gcov-dump -l -p",
        
        /* Combined short options (if supported by getopt) */
        "gcov-dump -lp",
        
        /* With positional argument (gcov file) */
        "gcov-dump -l dummy_test.gcda",
        
        /* With -- delimiter */
        "gcov-dump -l -- dummy_test.gcda",
        
        /* Multiple combined flags with file */
        "gcov-dump -lprs dummy_test.gcda",
        
        /* Invalid combined flags */
        "gcov-dump -lx",
        
        NULL
    };
    
    for (i = 0; syntax_tests[i] != NULL; i++) {
        printf("Testing: %s\n", syntax_tests[i]);
        system_gcov_dump(syntax_tests[i]);
        printf("\n");
    }
    
    /******************************************************************
     * Test 4: Environment and error contexts
     ******************************************************************/
    printf("--- Test 4: Environment and error contexts ---\n");
    
    /* Test with no arguments */
    printf("Testing: gcov-dump (no arguments)\n");
    {
        const char *args[] = {"gcov-dump", NULL};
        exec_gcov_dump(args);
    }
    printf("\n");
    
    /* Test with environment variable (if supported) */
    printf("Testing with GCOV_DUMP_OPTIONS environment variable\n");
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    {
        const char *args[] = {"gcov-dump", NULL};
        exec_gcov_dump(args);
    }
    unsetenv("GCOV_DUMP_OPTIONS");
    printf("\n");
    
    /* Test error output redirection for invalid flag */
    printf("Testing invalid flag with stderr redirection\n");
    system("gcov-dump -x 2>&1");
    printf("\n");
    
    /* Test with non-existent file */
    printf("Testing with non-existent file\n");
    {
        const char *args[] = {"gcov-dump", "-l", "non_existent.gcda", NULL};
        exec_gcov_dump(args);
    }
    printf("\n");
    
    /******************************************************************
     * Test 5: Edge cases and stress tests
     ******************************************************************/
    printf("--- Test 5: Edge cases ---\n");
    
    /* Empty string as argument */
    printf("Testing with empty string argument\n");
    {
        const char *args[] = {"gcov-dump", "", NULL};
        exec_gcov_dump(args);
    }
    printf("\n");
    
    /* Very long flag string */
    printf("Testing with long flag string\n");
    {
        char long_flag[100] = "-";
        for (j = 0; j < 20; j++) {
            strcat(long_flag, "l");
        }
        const char *args[] = {"gcov-dump", long_flag, NULL};
        exec_gcov_dump(args);
    }
    printf("\n");
    
    /* Multiple invalid flags */
    printf("Testing multiple invalid flags\n");
    {
        const char *args[] = {"gcov-dump", "-x", "-y", "-z", NULL};
        exec_gcov_dump(args);
    }
    printf("\n");
    
    /* Flag at end with file */
    printf("Testing flag after file argument\n");
    {
        const char *args[] = {"gcov-dump", "dummy_test.gcda", "-v", NULL};
        exec_gcov_dump(args);
    }
    
    printf("\n=== All tests completed ===\n");
    
    /* Cleanup */
    unlink("dummy_test.c");
    unlink("dummy_test");
    unlink("dummy_test.gcda");
    unlink("dummy_test.gcno");
    
    return 0;
}
