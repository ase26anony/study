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
    status = system("gcc -fprofile-arcs -ftest-coverage -O0 -o dummy_test dummy_test.c 2>/dev/null");
    if (status != 0) {
        fprintf(stderr, "Warning: Failed to compile dummy_test.c\n");
        /* Continue anyway - some tests don't need the file */
        return 0;
    }
    
    /* Run the program to generate .gcda file */
    status = system("./dummy_test 2>/dev/null");
    if (status != 0) {
        fprintf(stderr, "Warning: Failed to run dummy_test\n");
    }
    
    return 0;
}

/**
 * Execute gcov-dump using execvp for precise argument control.
 * Returns the child's exit status.
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
    } else if (pid > 0) {
        /* Parent process */
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    } else {
        /* Fork failed */
        perror("fork failed");
        return -1;
    }
}

/**
 * Execute gcov-dump using system() for shell interpretation tests.
 */
static void system_gcov_dump(const char *cmd) {
    int status = system(cmd);
    printf("system() call returned: %d\n", status >> 8);
}

int main(void) {
    printf("=== Testing gcov-dump flag parsing ===\n\n");
    
    /* Create dummy .gcda file for file-based tests */
    create_dummy_gcda();
    
    /* Test 1: Individual flag tests (execvp) */
    printf("--- Test 1: Individual flags ---\n");
    
    /* Help flag */
    const char *help_args[] = {"gcov-dump", "-h", NULL};
    printf("Testing: gcov-dump -h\n");
    exec_gcov_dump(help_args);
    printf("\n");
    
    /* Version flag */
    const char *version_args[] = {"gcov-dump", "-v", NULL};
    printf("Testing: gcov-dump -v\n");
    exec_gcov_dump(version_args);
    printf("\n");
    
    /* Contents dump flag */
    const char *contents_args[] = {"gcov-dump", "-l", NULL};
    printf("Testing: gcov-dump -l\n");
    exec_gcov_dump(contents_args);
    printf("\n");
    
    /* Positions dump flag */
    const char *positions_args[] = {"gcov-dump", "-p", NULL};
    printf("Testing: gcov-dump -p\n");
    exec_gcov_dump(positions_args);
    printf("\n");
    
    /* Raw dump flag */
    const char *raw_args[] = {"gcov-dump", "-r", NULL};
    printf("Testing: gcov-dump -r\n");
    exec_gcov_dump(raw_args);
    printf("\n");
    
    /* Stable dump flag */
    const char *stable_args[] = {"gcov-dump", "-s", NULL};
    printf("Testing: gcov-dump -s\n");
    exec_gcov_dump(stable_args);
    printf("\n");
    
    /* Invalid flag (triggers default case) */
    const char *invalid_args[] = {"gcov-dump", "-x", NULL};
    printf("Testing: gcov-dump -x (should show 'unknown flag')\n");
    exec_gcov_dump(invalid_args);
    printf("\n");
    
    /* Test 2: Flag combinations (execvp) */
    printf("--- Test 2: Flag combinations ---\n");
    
    /* Two flags */
    const char *combo1_args[] = {"gcov-dump", "-l", "-p", NULL};
    printf("Testing: gcov-dump -l -p\n");
    exec_gcov_dump(combo1_args);
    printf("\n");
    
    /* Three flags */
    const char *combo2_args[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    printf("Testing: gcov-dump -r -s -v\n");
    exec_gcov_dump(combo2_args);
    printf("\n");
    
    /* Help with other flag (may exit early) */
    const char *combo3_args[] = {"gcov-dump", "-h", "-l", NULL};
    printf("Testing: gcov-dump -h -l\n");
    exec_gcov_dump(combo3_args);
    printf("\n");
    
    /* Repeated flag */
    const char *combo4_args[] = {"gcov-dump", "-p", "-p", NULL};
    printf("Testing: gcov-dump -p -p\n");
    exec_gcov_dump(combo4_args);
    printf("\n");
    
    /* Test 3: Different syntactic styles (system) */
    printf("--- Test 3: Different syntactic styles (system()) ---\n");
    
    /* Combined short options */
    printf("Testing: gcov-dump -lp (combined)\n");
    system_gcov_dump("gcov-dump -lp 2>&1");
    printf("\n");
    
    /* With positional argument */
    printf("Testing: gcov-dump -l dummy_test.gcda\n");
    system_gcov_dump("gcov-dump -l dummy_test.gcda 2>&1");
    printf("\n");
    
    /* With -- delimiter */
    printf("Testing: gcov-dump -l -- dummy_test.gcda\n");
    system_gcov_dump("gcov-dump -l -- dummy_test.gcda 2>&1");
    printf("\n");
    
    /* Test 4: Environment and error contexts */
    printf("--- Test 4: Environment and error contexts ---\n");
    
    /* No arguments */
    printf("Testing: gcov-dump (no arguments)\n");
    const char *no_args[] = {"gcov-dump", NULL};
    exec_gcov_dump(no_args);
    printf("\n");
    
    /* Set environment variable */
    printf("Testing with GCOV_DUMP_OPTIONS environment variable\n");
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    const char *env_args[] = {"gcov-dump", NULL};
    exec_gcov_dump(env_args);
    unsetenv("GCOV_DUMP_OPTIONS");
    printf("\n");
    
    /* Test 5: Edge cases with system() */
    printf("--- Test 5: Edge cases ---\n");
    
    /* Multiple combined flags */
    printf("Testing: gcov-dump -lprs\n");
    system_gcov_dump("gcov-dump -lprs 2>&1");
    printf("\n");
    
    /* Flag with invalid combination */
    printf("Testing: gcov-dump -l -x -p\n");
    system_gcov_dump("gcov-dump -l -x -p 2>&1");
    printf("\n");
    
    /* Empty flag (just '-') */
    printf("Testing: gcov-dump -\n");
    system_gcov_dump("gcov-dump - 2>&1");
    printf("\n");
    
    /* Long argument (should be ignored by getopt) */
    printf("Testing: gcov-dump --help\n");
    system_gcov_dump("gcov-dump --help 2>&1");
    printf("\n");
    
    /* Cleanup */
    printf("=== Cleaning up ===\n");
    system("rm -f dummy_test.c dummy_test dummy_test.gcno dummy_test.gcda 2>/dev/null");
    
    printf("\nAll tests completed.\n");
    return 0;
}
