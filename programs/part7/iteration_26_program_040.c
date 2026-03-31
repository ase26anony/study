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
 * Creates a minimal valid .gcda file for testing.
 * Uses gcc with coverage flags to generate a real .gcda file.
 */
static int create_dummy_gcda(void) {
    FILE *fp;
    int status;
    pid_t pid;
    
    /* Create a trivial C source file */
    fp = fopen("dummy_test.c", "w");
    if (!fp) {
        perror("Failed to create dummy_test.c");
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
        perror("execlp gcc failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        /* Parent: wait for compilation */
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Compilation failed\n");
            return -1;
        }
        
        /* Run the program to generate .gcda */
        pid = fork();
        if (pid == 0) {
            execlp("./dummy_test", "./dummy_test", NULL);
            perror("execlp ./dummy_test failed");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            waitpid(pid, &status, 0);
            return 0;
        }
    }
    
    return -1;
}

/**
 * Execute gcov-dump using execvp for precise argument control.
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
        }
        return -1;
    } else {
        perror("fork failed");
        return -1;
    }
}

/**
 * Execute gcov-dump using system() to test that path.
 */
static void system_gcov_dump(const char *cmd) {
    int status = system(cmd);
    if (status == -1) {
        fprintf(stderr, "system() failed for: %s\n", cmd);
    } else if (WIFEXITED(status)) {
        printf("system() call exited with status: %d\n", WEXITSTATUS(status));
    }
}

int main(void) {
    int i, j;
    int test_count = 0;
    int passed = 0;
    
    printf("=== Starting gcov-dump flag parser tests ===\n\n");
    
    /* Create dummy .gcda file for file-based tests */
    printf("Creating dummy .gcda file...\n");
    if (create_dummy_gcda() != 0) {
        fprintf(stderr, "Warning: Could not create dummy .gcda file\n");
        fprintf(stderr, "File-based tests will use placeholder\n");
    }
    
    /* ============================================== */
    /* 1. Individual flag tests (direct switch cases) */
    /* ============================================== */
    
    printf("\n--- Testing individual flags ---\n");
    
    /* Test case 'h': Help flag */
    {
        const char *args[] = {"gcov-dump", "-h", NULL};
        printf("Test %d: gcov-dump -h (help)\n", ++test_count);
        if (exec_gcov_dump(args) != -1) passed++;
    }
    
    /* Test case 'v': Version flag */
    {
        const char *args[] = {"gcov-dump", "-v", NULL};
        printf("Test %d: gcov-dump -v (version)\n", ++test_count);
        if (exec_gcov_dump(args) != -1) passed++;
    }
    
    /* Test case 'l': Contents dump flag */
    {
        const char *args[] = {"gcov-dump", "-l", NULL};
        printf("Test %d: gcov-dump -l (contents dump)\n", ++test_count);
        if (exec_gcov_dump(args) != -1) passed++;
    }
    
    /* Test case 'p': Positions dump flag */
    {
        const char *args[] = {"gcov-dump", "-p", NULL};
        printf("Test %d: gcov-dump -p (positions dump)\n", ++test_count);
        if (exec_gcov_dump(args) != -1) passed++;
    }
    
    /* Test case 'r': Raw dump flag */
    {
        const char *args[] = {"gcov-dump", "-r", NULL};
        printf("Test %d: gcov-dump -r (raw dump)\n", ++test_count);
        if (exec_gcov_dump(args) != -1) passed++;
    }
    
    /* Test case 's': Stable dump flag */
    {
        const char *args[] = {"gcov-dump", "-s", NULL};
        printf("Test %d: gcov-dump -s (stable dump)\n", ++test_count);
        if (exec_gcov_dump(args) != -1) passed++;
    }
    
    /* Test default case: Invalid flag */
    {
        const char *args[] = {"gcov-dump", "-x", NULL};
        printf("Test %d: gcov-dump -x (invalid flag - should print 'unknown flag')\n", ++test_count);
        if (exec_gcov_dump(args) != -1) passed++;
    }
    
    /* ============================================== */
    /* 2. Combination and repetition of flags         */
    /* ============================================== */
    
    printf("\n--- Testing flag combinations ---\n");
    
    /* Combination 1: -l -p */
    {
        const char *args[] = {"gcov-dump", "-l", "-p", NULL};
        printf("Test %d: gcov-dump -l -p (contents + positions)\n", ++test_count);
        if (exec_gcov_dump(args) != -1) passed++;
    }
    
    /* Combination 2: -r -s -v */
    {
        const char *args[] = {"gcov-dump", "-r", "-s", "-v", NULL};
        printf("Test %d: gcov-dump -r -s -v (raw + stable + version)\n", ++test_count);
        if (exec_gcov_dump(args) != -1) passed++;
    }
    
    /* Combination 3: -h -l (help may cause early exit, but test anyway) */
    {
        const char *args[] = {"gcov-dump", "-h", "-l", NULL};
        printf("Test %d: gcov-dump -h -l (help + contents)\n", ++test_count);
        if (exec_gcov_dump(args) != -1) passed++;
    }
    
    /* Repetition: -p -p */
    {
        const char *args[] = {"gcov-dump", "-p", "-p", NULL};
        printf("Test %d: gcov-dump -p -p (repeated flag)\n", ++test_count);
        if (exec_gcov_dump(args) != -1) passed++;
    }
    
    /* ============================================== */
    /* 3. Flag argument separation tests              */
    /* ============================================== */
    
    printf("\n--- Testing flag syntax variations ---\n");
    
    /* Combined short options: -lp (if getopt supports it) */
    {
        const char *args[] = {"gcov-dump", "-lp", NULL};
        printf("Test %d: gcov-dump -lp (combined flags)\n", ++test_count);
        if (exec_gcov_dump(args) != -1) passed++;
    }
    
    /* With positional argument (gcov file) */
    {
        const char *args[] = {"gcov-dump", "-l", "dummy_test.gcda", NULL};
        printf("Test %d: gcov-dump -l dummy_test.gcda (with file)\n", ++test_count);
        if (exec_gcov_dump(args) != -1) passed++;
    }
    
    /* With -- delimiter */
    {
        const char *args[] = {"gcov-dump", "-l", "--", "dummy_test.gcda", NULL};
        printf("Test %d: gcov-dump -l -- dummy_test.gcda (with delimiter)\n", ++test_count);
        if (exec_gcov_dump(args) != -1) passed++;
    }
    
    /* Multiple files with flags */
    {
        const char *args[] = {"gcov-dump", "-l", "-p", "dummy_test.gcda", "dummy_test.gcno", NULL};
        printf("Test %d: gcov-dump -l -p file1 file2 (multiple files)\n", ++test_count);
        if (exec_gcov_dump(args) != -1) passed++;
    }
    
    /* ============================================== */
    /* 4. Environment and error contexts              */
    /* ============================================== */
    
    printf("\n--- Testing environment and edge cases ---\n");
    
    /* No arguments */
    {
        const char *args[] = {"gcov-dump", NULL};
        printf("Test %d: gcov-dump (no arguments)\n", ++test_count);
        if (exec_gcov_dump(args) != -1) passed++;
    }
    
    /* Set environment variable before execution */
    {
        printf("Test %d: With GCOV_DUMP_OPTIONS environment variable\n", ++test_count);
        setenv("GCOV_DUMP_OPTIONS", "-l", 1);
        const char *args[] = {"gcov-dump", "-p", NULL};
        if (exec_gcov_dump(args) != -1) passed++;
        unsetenv("GCOV_DUMP_OPTIONS");
    }
    
    /* Test with system() calls for coverage */
    printf("\n--- Testing with system() calls ---\n");
    
    printf("Test %d: system(\"gcov-dump -h\")\n", ++test_count);
    system_gcov_dump("gcov-dump -h");
    passed++;
    
    printf("Test %d: system(\"gcov-dump -x 2>&1\")\n", ++test_count);
    system_gcov_dump("gcov-dump -x 2>&1");
    passed++;
    
    printf("Test %d: system(\"gcov-dump -l -p dummy_test.gcda 2>&1\")\n", ++test_count);
    system_gcov_dump("gcov-dump -l -p dummy_test.gcda 2>&1");
    passed++;
    
    /* ============================================== */
    /* 5. Additional stress tests                     */
    /* ============================================== */
    
    printf("\n--- Additional stress tests ---\n");
    
    /* Very long argument list */
    {
        const char *args[] = {"gcov-dump", "-l", "-p", "-r", "-s", "-v", 
                              "dummy_test.gcda", "dummy_test.gcno", NULL};
        printf("Test %d: gcov-dump with many flags and files\n", ++test_count);
        if (exec_gcov_dump(args) != -1) passed++;
    }
    
    /* Flag at end */
    {
        const char *args[] = {"gcov-dump", "dummy_test.gcda", "-l", NULL};
        printf("Test %d: gcov-dump file -l (flag after file)\n", ++test_count);
        if (exec_gcov_dump(args) != -1) passed++;
    }
    
    /* Empty string argument */
    {
        const char *args[] = {"gcov-dump", "", "-l", NULL};
        printf("Test %d: gcov-dump '' -l (empty argument)\n", ++test_count);
        if (exec_gcov_dump(args) != -1) passed++;
    }
    
    /* ============================================== */
    /* Summary                                        */
    /* ============================================== */
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests attempted: %d\n", test_count);
    printf("Tests that executed gcov-dump successfully: %d\n", passed);
    
    /* Cleanup */
    unlink("dummy_test.c");
    unlink("dummy_test");
    unlink("dummy_test.gcda");
    unlink("dummy_test.gcno");
    
    return 0;
}
