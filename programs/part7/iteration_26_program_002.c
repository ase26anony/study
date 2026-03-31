/**
 * gcov-dump_parser_test.c
 * 
 * Comprehensive test program for gcov-dump command-line flag parsing.
 * Tests all uncovered switch cases in gcov-dump.cc lines 111-130.
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
    
    /* Create a minimal C source file */
    fp = fopen("dummy_test.c", "w");
    if (!fp) {
        perror("Failed to create dummy_test.c");
        return -1;
    }
    fprintf(fp, "int main() { return 0; }\n");
    fclose(fp);
    
    /* Compile with coverage flags */
    status = system("gcc -fprofile-arcs -ftest-coverage -O0 dummy_test.c -o dummy_test");
    if (status != 0) {
        fprintf(stderr, "Failed to compile dummy_test.c\n");
        return -1;
    }
    
    /* Run to generate .gcda file */
    status = system("./dummy_test");
    if (status != 0) {
        fprintf(stderr, "Failed to run dummy_test\n");
        return -1;
    }
    
    /* Rename the generated .gcda file */
    if (rename("dummy_test.gcda", "dummy.gcda") != 0) {
        /* Try alternative name */
        if (rename("dummy_test.gcno", "dummy.gcno") != 0) {
            fprintf(stderr, "Warning: Could not rename gcda/gcno files\n");
        }
    }
    
    return 0;
}

/**
 * Execute gcov-dump using execvp for precise argument control.
 * Returns exit status of gcov-dump.
 */
static int exec_gcov_dump(const char *args[]) {
    pid_t pid;
    int status;
    
    pid = fork();
    if (pid == 0) {
        /* Child process */
        execvp("gcov-dump", (char *const *)args);
        /* If execvp returns, it failed */
        fprintf(stderr, "Failed to execute gcov-dump: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        /* Fork failed */
        perror("fork failed");
        return -1;
    } else {
        /* Parent process */
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    }
}

/**
 * Execute gcov-dump using system() to test shell interpretation.
 */
static void system_gcov_dump(const char *cmd) {
    int status = system(cmd);
    printf("system() call returned: %d\n", status);
}

int main(void) {
    int i, j;
    
    printf("=== Starting gcov-dump flag parser tests ===\n\n");
    
    /* Create dummy .gcda file for file-based tests */
    if (create_dummy_gcda() != 0) {
        fprintf(stderr, "Warning: Could not create dummy .gcda file\n");
        printf("Continuing with tests that don't require .gcda file...\n\n");
    }
    
    /* Test 1: Individual flag cases (lines 111-130) */
    printf("--- Test 1: Individual flag cases ---\n");
    
    const char *individual_flags[][3] = {
        {"gcov-dump", "-h", NULL},        /* Help flag */
        {"gcov-dump", "-v", NULL},        /* Version flag */
        {"gcov-dump", "-l", NULL},        /* Contents dump flag */
        {"gcov-dump", "-p", NULL},        /* Positions dump flag */
        {"gcov-dump", "-r", NULL},        /* Raw dump flag */
        {"gcov-dump", "-s", NULL},        /* Stable dump flag */
        {"gcov-dump", "-x", NULL},        /* Invalid flag (triggers default case) */
        {NULL, NULL, NULL}
    };
    
    for (i = 0; individual_flags[i][0] != NULL; i++) {
        printf("Testing: ");
        for (j = 0; individual_flags[i][j] != NULL; j++) {
            printf("%s ", individual_flags[i][j]);
        }
        printf("\n");
        
        int status = exec_gcov_dump(individual_flags[i]);
        printf("Exit status: %d\n\n", status);
    }
    
    /* Test 2: Flag combinations */
    printf("--- Test 2: Flag combinations ---\n");
    
    const char *flag_combinations[][5] = {
        {"gcov-dump", "-l", "-p", NULL},          /* Two flags */
        {"gcov-dump", "-r", "-s", "-v", NULL},    /* Three flags */
        {"gcov-dump", "-h", "-l", NULL},          /* Help with another flag */
        {"gcov-dump", "-p", "-p", NULL},          /* Repeated flag */
        {"gcov-dump", "-l", "-p", "-r", "-s", NULL}, /* All valid flags */
        {NULL, NULL, NULL, NULL, NULL}
    };
    
    for (i = 0; flag_combinations[i][0] != NULL; i++) {
        printf("Testing: ");
        for (j = 0; flag_combinations[i][j] != NULL; j++) {
            printf("%s ", flag_combinations[i][j]);
        }
        printf("\n");
        
        int status = exec_gcov_dump(flag_combinations[i]);
        printf("Exit status: %d\n\n", status);
    }
    
    /* Test 3: Different flag syntax styles */
    printf("--- Test 3: Different flag syntax styles ---\n");
    
    /* Using system() for these to test shell interpretation */
    const char *syntax_tests[] = {
        "gcov-dump -lp",                 /* Combined short options */
        "gcov-dump -l -p",               /* Separate arguments */
        "gcov-dump -l dummy.gcda",       /* With positional argument */
        "gcov-dump -l -- dummy.gcda",    /* With -- delimiter */
        "gcov-dump -l -p dummy.gcda",    /* Multiple flags with file */
        NULL
    };
    
    for (i = 0; syntax_tests[i] != NULL; i++) {
        printf("Testing: %s\n", syntax_tests[i]);
        system_gcov_dump(syntax_tests[i]);
        printf("\n");
    }
    
    /* Test 4: Environment and error contexts */
    printf("--- Test 4: Environment and error contexts ---\n");
    
    /* Test with no arguments */
    printf("Testing: gcov-dump (no arguments)\n");
    {
        const char *args[] = {"gcov-dump", NULL};
        int status = exec_gcov_dump(args);
        printf("Exit status: %d\n\n", status);
    }
    
    /* Test with environment variable */
    printf("Testing with GCOV_DUMP_OPTIONS environment variable\n");
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    {
        const char *args[] = {"gcov-dump", NULL};
        int status = exec_gcov_dump(args);
        printf("Exit status: %d\n\n", status);
    }
    unsetenv("GCOV_DUMP_OPTIONS");
    
    /* Test invalid flag with stderr redirection */
    printf("Testing invalid flag with stderr capture simulation\n");
    printf("(Redirecting stderr to see 'unknown flag' message)\n");
    system("gcov-dump -x 2>&1");
    printf("\n");
    
    /* Test 5: Edge cases */
    printf("--- Test 5: Edge cases ---\n");
    
    const char *edge_cases[][4] = {
        {"gcov-dump", "--", NULL},               /* Just -- */
        {"gcov-dump", "-", NULL},                /* Single dash */
        {"gcov-dump", "-l", "--", "-p", NULL},   /* Flag after -- */
        {"gcov-dump", "-lps", NULL},             /* All flags combined */
        {NULL, NULL, NULL, NULL}
    };
    
    for (i = 0; edge_cases[i][0] != NULL; i++) {
        printf("Testing: ");
        for (j = 0; edge_cases[i][j] != NULL; j++) {
            printf("%s ", edge_cases[i][j]);
        }
        printf("\n");
        
        int status = exec_gcov_dump(edge_cases[i]);
        printf("Exit status: %d\n\n", status);
    }
    
    /* Test 6: Long arguments (if supported) */
    printf("--- Test 6: Long argument tests ---\n");
    printf("Testing: gcov-dump --help\n");
    system("gcov-dump --help 2>&1 | head -5");
    printf("\n");
    
    printf("Testing: gcov-dump --version\n");
    system("gcov-dump --version 2>&1");
    printf("\n");
    
    /* Cleanup */
    printf("=== Cleaning up ===\n");
    system("rm -f dummy_test.c dummy_test dummy.gcda dummy.gcno dummy_test.gcda dummy_test.gcno 2>/dev/null");
    
    printf("=== All tests completed ===\n");
    return 0;
}
