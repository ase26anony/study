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
 * Creates a minimal dummy .gcda file for testing.
 * This creates a valid gcov data file header that gcov-dump can parse.
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
    
    fwrite(&magic, sizeof(magic), 1, fp);
    fwrite(&version, sizeof(version), 1, fp);
    
    /* Write a zero tag to indicate end of file */
    unsigned int zero_tag = 0;
    fwrite(&zero_tag, sizeof(zero_tag), 1, fp);
    
    fclose(fp);
    return 0;
}

/**
 * Execute gcov-dump using execvp for precise argument control.
 * Returns the child process exit status.
 */
static int exec_gcov_dump(char *const args[]) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        execvp("gcov-dump", args);
        /* If execvp returns, it failed */
        fprintf(stderr, "execvp failed: %s\n", strerror(errno));
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
 * Execute gcov-dump using system() for shell interpretation testing.
 */
static int system_gcov_dump(const char *cmd) {
    printf("Executing via system(): %s\n", cmd);
    int ret = system(cmd);
    printf("Exit status: %d\n\n", ret);
    return ret;
}

int main(void) {
    printf("=== Starting gcov-dump flag parser tests ===\n\n");
    
    /* Create a dummy .gcda file for file-based tests */
    const char *dummy_file = "dummy.gcda";
    if (create_dummy_gcda(dummy_file) != 0) {
        fprintf(stderr, "Warning: Could not create dummy .gcda file\n");
    }
    
    /* Test 1: Individual flag tests (execvp method) */
    printf("--- Test 1: Individual flags (execvp) ---\n");
    
    /* Array of argument sets for individual flag testing */
    char *individual_tests[][4] = {
        {"gcov-dump", "-h", NULL},
        {"gcov-dump", "-v", NULL},
        {"gcov-dump", "-l", NULL},
        {"gcov-dump", "-p", NULL},
        {"gcov-dump", "-r", NULL},
        {"gcov-dump", "-s", NULL},
        {"gcov-dump", "-x", NULL},  /* Invalid flag for default case */
        {NULL}  /* Sentinel */
    };
    
    for (int i = 0; individual_tests[i][0] != NULL; i++) {
        printf("Testing: ");
        for (int j = 0; individual_tests[i][j] != NULL; j++) {
            printf("%s ", individual_tests[i][j]);
        }
        printf("\n");
        
        int status = exec_gcov_dump(individual_tests[i]);
        printf("Exit status: %d\n\n", status);
    }
    
    /* Test 2: Flag combinations (execvp method) */
    printf("--- Test 2: Flag combinations (execvp) ---\n");
    
    char *combo_tests[][6] = {
        {"gcov-dump", "-l", "-p", NULL},
        {"gcov-dump", "-r", "-s", "-v", NULL},
        {"gcov-dump", "-h", "-l", NULL},  /* -h may cause early exit */
        {"gcov-dump", "-p", "-p", NULL},  /* Repeated flag */
        {"gcov-dump", "-l", "-p", "-r", "-s", NULL},
        {NULL}
    };
    
    for (int i = 0; combo_tests[i][0] != NULL; i++) {
        printf("Testing: ");
        for (int j = 0; combo_tests[i][j] != NULL; j++) {
            printf("%s ", combo_tests[i][j]);
        }
        printf("\n");
        
        int status = exec_gcov_dump(combo_tests[i]);
        printf("Exit status: %d\n\n", status);
    }
    
    /* Test 3: Different flag syntax styles (system method) */
    printf("--- Test 3: Different flag syntax (system) ---\n");
    
    const char *syntax_tests[] = {
        "gcov-dump -l -p",              /* Separate arguments */
        "gcov-dump -lp",                /* Combined short options */
        "gcov-dump -l dummy.gcda",      /* With positional argument */
        "gcov-dump -l -- dummy.gcda",   /* With -- delimiter */
        "gcov-dump -l -p dummy.gcda",   /* Multiple flags with file */
        NULL
    };
    
    for (int i = 0; syntax_tests[i] != NULL; i++) {
        system_gcov_dump(syntax_tests[i]);
    }
    
    /* Test 4: Edge cases and environment (system method) */
    printf("--- Test 4: Edge cases and environment (system) ---\n");
    
    /* Test with no arguments */
    printf("Testing: gcov-dump (no arguments)\n");
    system("gcov-dump");
    printf("\n");
    
    /* Test with environment variable if supported */
    printf("Testing with GCOV_DUMP_OPTIONS environment variable\n");
    setenv("GCOV_DUMP_OPTIONS", "-l", 1);
    system_gcov_dump("gcov-dump");
    unsetenv("GCOV_DUMP_OPTIONS");
    
    /* Test error stream redirection for default case */
    printf("Testing invalid flag with stderr redirection\n");
    system("gcov-dump -x 2>&1");
    printf("\n");
    
    /* Test 5: Stress test with many flags */
    printf("--- Test 5: Stress test ---\n");
    
    char *stress_args[] = {
        "gcov-dump",
        "-l", "-p", "-r", "-s", "-v",
        "dummy.gcda",
        NULL
    };
    
    printf("Testing many flags: ");
    for (int i = 0; stress_args[i] != NULL; i++) {
        printf("%s ", stress_args[i]);
    }
    printf("\n");
    
    int status = exec_gcov_dump(stress_args);
    printf("Exit status: %d\n\n", status);
    
    /* Test 6: Empty flag argument (edge case) */
    printf("--- Test 6: Edge cases ---\n");
    
    /* Test with empty string as flag */
    char *edge_args[] = {"gcov-dump", "", NULL};
    printf("Testing empty flag argument\n");
    status = exec_gcov_dump(edge_args);
    printf("Exit status: %d\n\n", status);
    
    /* Test with only -- */
    char *double_dash_args[] = {"gcov-dump", "--", NULL};
    printf("Testing only --\n");
    status = exec_gcov_dump(double_dash_args);
    printf("Exit status: %d\n\n", status);
    
    /* Cleanup */
    remove(dummy_file);
    
    printf("=== All tests completed ===\n");
    return 0;
}
