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
 * Create a minimal dummy .gcda file for testing.
 * This creates a valid gcov data file header structure.
 */
static int create_dummy_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create dummy .gcda file");
        return -1;
    }
    
    /* Write minimal gcov data file magic and version */
    unsigned int magic = 0x67636461; /* 'gcda' in little-endian */
    unsigned int version = 0x4020000; /* GCC 4.2 format */
    unsigned int stamp = 0x12345678;
    
    fwrite(&magic, sizeof(magic), 1, fp);
    fwrite(&version, sizeof(version), 1, fp);
    fwrite(&stamp, sizeof(stamp), 1, fp);
    
    /* Write zero records to indicate end of file */
    unsigned int zero = 0;
    fwrite(&zero, sizeof(zero), 1, fp);
    
    fclose(fp);
    return 0;
}

/**
 * Execute gcov-dump using execvp for precise argument control.
 * Returns exit status of gcov-dump.
 */
static int exec_gcov_dump(const char *args[]) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        execvp("gcov-dump", (char *const *)args);
        
        /* If execvp returns, it failed */
        fprintf(stderr, "Failed to execute gcov-dump: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
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
 * Execute gcov-dump using system() for comparison.
 */
static void system_gcov_dump(const char *cmd) {
    printf("Executing via system(): %s\n", cmd);
    int ret = system(cmd);
    printf("Exit status: %d\n\n", ret);
}

int main(void) {
    printf("=== Testing gcov-dump Flag Parser ===\n\n");
    
    /* Create dummy .gcda file for tests that require file arguments */
    if (create_dummy_gcda("dummy.gcda") != 0) {
        fprintf(stderr, "Warning: Could not create dummy.gcda file\n");
    }
    
    /* Test 1: Individual flag cases (lines 111-130) */
    printf("--- Test 1: Individual Flag Cases ---\n");
    
    /* Case 'h': Help flag */
    const char *help_args[] = {"gcov-dump", "-h", NULL};
    printf("Testing -h (help):\n");
    exec_gcov_dump(help_args);
    
    /* Case 'v': Version flag */
    const char *version_args[] = {"gcov-dump", "-v", NULL};
    printf("\nTesting -v (version):\n");
    exec_gcov_dump(version_args);
    
    /* Case 'l': Contents dump flag */
    const char *contents_args[] = {"gcov-dump", "-l", NULL};
    printf("\nTesting -l (contents dump):\n");
    exec_gcov_dump(contents_args);
    
    /* Case 'p': Positions dump flag */
    const char *positions_args[] = {"gcov-dump", "-p", NULL};
    printf("\nTesting -p (positions dump):\n");
    exec_gcov_dump(positions_args);
    
    /* Case 'r': Raw dump flag */
    const char *raw_args[] = {"gcov-dump", "-r", NULL};
    printf("\nTesting -r (raw dump):\n");
    exec_gcov_dump(raw_args);
    
    /* Case 's': Stable dump flag */
    const char *stable_args[] = {"gcov-dump", "-s", NULL};
    printf("\nTesting -s (stable dump):\n");
    exec_gcov_dump(stable_args);
    
    /* Default case: Invalid flag */
    const char *invalid_args[] = {"gcov-dump", "-x", NULL};
    printf("\nTesting -x (invalid flag - should trigger default case):\n");
    exec_gcov_dump(invalid_args);
    
    /* Test 2: Flag combinations */
    printf("\n--- Test 2: Flag Combinations ---\n");
    
    /* Combination 1: -l -p */
    const char *combo1_args[] = {"gcov-dump", "-l", "-p", NULL};
    printf("Testing -l -p:\n");
    exec_gcov_dump(combo1_args);
    
    /* Combination 2: -r -s -v */
    const char *combo2_args[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    printf("\nTesting -r -s -v:\n");
    exec_gcov_dump(combo2_args);
    
    /* Combination 3: -h -l (help may exit early) */
    const char *combo3_args[] = {"gcov-dump", "-h", "-l", NULL};
    printf("\nTesting -h -l (help with extra flag):\n");
    exec_gcov_dump(combo3_args);
    
    /* Test 3: Repeated flags */
    printf("\n--- Test 3: Repeated Flags ---\n");
    
    /* Repeated -p flag */
    const char *repeat_args[] = {"gcov-dump", "-p", "-p", NULL};
    printf("Testing -p -p (repeated flag):\n");
    exec_gcov_dump(repeat_args);
    
    /* Test 4: Different flag syntax styles */
    printf("\n--- Test 4: Different Syntax Styles ---\n");
    
    /* Combined short options: -lp */
    const char *combined_args[] = {"gcov-dump", "-lp", NULL};
    printf("Testing -lp (combined flags):\n");
    exec_gcov_dump(combined_args);
    
    /* With positional argument (gcov file) */
    const char *with_file_args[] = {"gcov-dump", "-l", "dummy.gcda", NULL};
    printf("\nTesting -l with file argument:\n");
    exec_gcov_dump(with_file_args);
    
    /* With -- delimiter */
    const char *with_delimiter_args[] = {"gcov-dump", "-l", "--", "dummy.gcda", NULL};
    printf("\nTesting -l -- dummy.gcda (with delimiter):\n");
    exec_gcov_dump(with_delimiter_args);
    
    /* Test 5: Environment and error contexts */
    printf("\n--- Test 5: Environment and Error Contexts ---\n");
    
    /* No arguments */
    const char *no_args[] = {"gcov-dump", NULL};
    printf("Testing with no arguments:\n");
    exec_gcov_dump(no_args);
    
    /* Set environment variable if supported */
    printf("\nTesting with GCOV_DUMP_OPTIONS environment variable:\n");
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    exec_gcov_dump(no_args);
    unsetenv("GCOV_DUMP_OPTIONS");
    
    /* Test 6: Using system() calls for comparison */
    printf("\n--- Test 6: system() Call Tests ---\n");
    
    system_gcov_dump("gcov-dump -h 2>&1");
    system_gcov_dump("gcov-dump -v 2>&1");
    system_gcov_dump("gcov-dump -l -p 2>&1");
    system_gcov_dump("gcov-dump -x 2>&1");  /* Should show "unknown flag" error */
    
    /* Test with output redirection to capture stderr */
    printf("\nTesting stderr capture for invalid flag:\n");
    system("gcov-dump -x 2>&1 | grep -q 'unknown flag' && echo 'Default case triggered successfully'");
    
    /* Cleanup */
    remove("dummy.gcda");
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
