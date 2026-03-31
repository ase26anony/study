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
 * Returns 0 on success, -1 on failure.
 */
int create_dummy_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create dummy .gcda file");
        return -1;
    }
    
    // Write minimal GCOV data magic and version
    unsigned int magic = 0x67636461; // 'gcda' in little-endian
    unsigned int version = 0x4020000; // GCOV version 4.2
    
    fwrite(&magic, sizeof(magic), 1, fp);
    fwrite(&version, sizeof(version), 1, fp);
    
    // Write a zero terminator
    unsigned int zero = 0;
    fwrite(&zero, sizeof(zero), 1, fp);
    
    fclose(fp);
    return 0;
}

/**
 * Execute gcov-dump using execvp for precise argument control.
 * Returns exit status of gcov-dump.
 */
int exec_gcov_dump(char *const args[]) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        // Child process
        execvp("gcov-dump", args);
        // If execvp returns, it failed
        fprintf(stderr, "execvp failed for gcov-dump: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        // Parent process
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
void system_gcov_dump(const char *cmd) {
    printf("Executing via system(): %s\n", cmd);
    int ret = system(cmd);
    printf("Exit status: %d\n\n", ret);
}

int main(int argc, char *argv[]) {
    printf("=== Testing gcov-dump flag parsing logic ===\n\n");
    
    // Create dummy .gcda file for file-based tests
    const char *dummy_gcda = "dummy.gcda";
    if (create_dummy_gcda(dummy_gcda) != 0) {
        fprintf(stderr, "Warning: Could not create dummy .gcda file\n");
    }
    
    // Test 1: Individual flag tests (execvp method)
    printf("--- Individual Flag Tests (execvp) ---\n");
    
    // Array of argument sets for individual flags
    char *individual_tests[][4] = {
        {"gcov-dump", "-h", NULL},
        {"gcov-dump", "-v", NULL},
        {"gcov-dump", "-l", NULL},
        {"gcov-dump", "-p", NULL},
        {"gcov-dump", "-r", NULL},
        {"gcov-dump", "-s", NULL},
        {"gcov-dump", "-x", NULL},  // Invalid flag for default case
        {NULL}
    };
    
    for (int i = 0; individual_tests[i][0] != NULL; i++) {
        printf("Test %d: ", i + 1);
        for (int j = 0; individual_tests[i][j] != NULL; j++) {
            printf("%s ", individual_tests[i][j]);
        }
        printf("\n");
        
        int status = exec_gcov_dump(individual_tests[i]);
        printf("Exit status: %d\n\n", status);
    }
    
    // Test 2: Flag combinations (execvp method)
    printf("--- Flag Combination Tests (execvp) ---\n");
    
    char *combo_tests[][6] = {
        {"gcov-dump", "-l", "-p", NULL},
        {"gcov-dump", "-r", "-s", "-v", NULL},
        {"gcov-dump", "-h", "-l", NULL},  // -h may cause early exit
        {"gcov-dump", "-p", "-p", NULL},  // Repeated flag
        {"gcov-dump", "-l", "-p", "-r", "-s", NULL},
        {NULL}
    };
    
    for (int i = 0; combo_tests[i][0] != NULL; i++) {
        printf("Combo test %d: ", i + 1);
        for (int j = 0; combo_tests[i][j] != NULL; j++) {
            printf("%s ", combo_tests[i][j]);
        }
        printf("\n");
        
        int status = exec_gcov_dump(combo_tests[i]);
        printf("Exit status: %d\n\n", status);
    }
    
    // Test 3: Different flag syntax styles (system method)
    printf("--- Flag Syntax Style Tests (system) ---\n");
    
    const char *syntax_tests[] = {
        // Separate arguments
        "gcov-dump -l -p",
        // Combined short options (if supported)
        "gcov-dump -lp",
        // With positional arguments
        "gcov-dump -l dummy.gcda",
        // With -- delimiter
        "gcov-dump -l -- dummy.gcda",
        // Multiple combined flags
        "gcov-dump -lprs",
        NULL
    };
    
    for (int i = 0; syntax_tests[i] != NULL; i++) {
        system_gcov_dump(syntax_tests[i]);
    }
    
    // Test 4: Environment and edge cases
    printf("--- Environment and Edge Case Tests ---\n");
    
    // No arguments
    printf("Test: gcov-dump (no arguments)\n");
    char *no_args[] = {"gcov-dump", NULL};
    int status = exec_gcov_dump(no_args);
    printf("Exit status: %d\n\n", status);
    
    // Set environment variable if supported
    printf("Test with GCOV_DUMP_OPTIONS environment variable\n");
    setenv("GCOV_DUMP_OPTIONS", "-l", 1);
    char *env_test[] = {"gcov-dump", NULL};
    status = exec_gcov_dump(env_test);
    printf("Exit status: %d\n\n", status);
    unsetenv("GCOV_DUMP_OPTIONS");
    
    // Test with output redirection to capture stderr
    printf("Test invalid flag with stderr capture (system)\n");
    system("gcov-dump -x 2>&1 | head -5");
    printf("\n");
    
    // Test 5: Long argument list stress test
    printf("--- Long Argument Stress Test ---\n");
    
    // Build a command with many flags
    char *stress_args[] = {
        "gcov-dump",
        "-l", "-p", "-r", "-s",
        "-l", "-p", "-r", "-s",  // Repeated
        dummy_gcda,
        NULL
    };
    
    printf("Stress test with multiple repeated flags\n");
    status = exec_gcov_dump(stress_args);
    printf("Exit status: %d\n\n", status);
    
    // Test 6: Mixed valid and invalid flags
    printf("--- Mixed Valid and Invalid Flags ---\n");
    
    char *mixed_args[] = {"gcov-dump", "-l", "-x", "-p", NULL};
    printf("Test: gcov-dump -l -x -p (mixed valid/invalid)\n");
    status = exec_gcov_dump(mixed_args);
    printf("Exit status: %d\n\n", status);
    
    // Cleanup
    remove(dummy_gcda);
    
    printf("=== All tests completed ===\n");
    
    return 0;
}
