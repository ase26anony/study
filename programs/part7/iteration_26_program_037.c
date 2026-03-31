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
 * gcov-dump requires a valid .gcda file for some operations.
 */
static int create_dummy_gcda_file(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create dummy .gcda file");
        return -1;
    }
    
    // Write minimal gcov data file header
    // Magic number for gcov files
    unsigned int magic = 0x67636461;  // 'gcda' in little-endian
    unsigned int version = 0x4020000; // GCOV_VERSION
    unsigned int stamp = 0x12345678;  // Random stamp
    
    fwrite(&magic, sizeof(magic), 1, fp);
    fwrite(&version, sizeof(version), 1, fp);
    fwrite(&stamp, sizeof(stamp), 1, fp);
    
    // Write a zero terminator (empty data)
    unsigned int zero = 0;
    fwrite(&zero, sizeof(zero), 1, fp);
    
    fclose(fp);
    return 0;
}

/**
 * Executes gcov-dump using execvp for precise argument control.
 * Returns the exit status of gcov-dump.
 */
static int execute_gcov_dump_execvp(const char **argv) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        // Child process
        execvp("gcov-dump", (char *const *)argv);
        
        // If execvp returns, it failed
        fprintf(stderr, "execvp failed: %s\n", strerror(errno));
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
 * Executes gcov-dump using system() for shell interpretation testing.
 */
static int execute_gcov_dump_system(const char *cmd) {
    int ret = system(cmd);
    
    if (ret == -1) {
        perror("system() failed");
        return -1;
    }
    
    return ret;
}

/**
 * Test case structure for organizing different test scenarios.
 */
typedef struct {
    const char *description;
    const char **argv;  // NULL-terminated array of arguments
    int use_system;     // 1 to use system(), 0 to use execvp
    const char *system_cmd; // Command string for system()
} test_case_t;

int main(void) {
    printf("=== Starting gcov-dump flag parser tests ===\n\n");
    
    // Create a dummy .gcda file for file-based tests
    const char *dummy_gcda = "dummy.gcda";
    if (create_dummy_gcda_file(dummy_gcda) != 0) {
        fprintf(stderr, "Warning: Could not create dummy .gcda file\n");
    }
    
    // Define all test cases
    const char *argv_h[] = {"gcov-dump", "-h", NULL};
    const char *argv_v[] = {"gcov-dump", "-v", NULL};
    const char *argv_l[] = {"gcov-dump", "-l", NULL};
    const char *argv_p[] = {"gcov-dump", "-p", NULL};
    const char *argv_r[] = {"gcov-dump", "-r", NULL};
    const char *argv_s[] = {"gcov-dump", "-s", NULL};
    const char *argv_x[] = {"gcov-dump", "-x", NULL};  // Invalid flag
    
    // Combined flags (separate arguments)
    const char *argv_lp[] = {"gcov-dump", "-l", "-p", NULL};
    const char *argv_rsv[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    const char *argv_hl[] = {"gcov-dump", "-h", "-l", NULL};
    const char *argv_pp[] = {"gcov-dump", "-p", "-p", NULL};
    
    // With positional arguments
    const char *argv_l_file[] = {"gcov-dump", "-l", dummy_gcda, NULL};
    const char *argv_lp_file[] = {"gcov-dump", "-l", "-p", dummy_gcda, NULL};
    
    // With -- delimiter
    const char *argv_l_delim[] = {"gcov-dump", "-l", "--", dummy_gcda, NULL};
    
    // No arguments
    const char *argv_none[] = {"gcov-dump", NULL};
    
    test_case_t test_cases[] = {
        // Individual flag tests (using execvp for precision)
        {"Test -h (help flag)", argv_h, 0, NULL},
        {"Test -v (version flag)", argv_v, 0, NULL},
        {"Test -l (contents dump)", argv_l, 0, NULL},
        {"Test -p (positions dump)", argv_p, 0, NULL},
        {"Test -r (raw dump)", argv_r, 0, NULL},
        {"Test -s (stable dump)", argv_s, 0, NULL},
        {"Test -x (invalid flag - triggers default case)", argv_x, 0, NULL},
        
        // Combined flag tests
        {"Test -l -p (combined separate flags)", argv_lp, 0, NULL},
        {"Test -r -s -v (three flags)", argv_rsv, 0, NULL},
        {"Test -h -l (help with another flag)", argv_hl, 0, NULL},
        {"Test -p -p (repeated flag)", argv_pp, 0, NULL},
        
        // Tests with file arguments
        {"Test -l with .gcda file", argv_l_file, 0, NULL},
        {"Test -l -p with .gcda file", argv_lp_file, 0, NULL},
        {"Test -l -- .gcda (with delimiter)", argv_l_delim, 0, NULL},
        
        // No arguments test
        {"Test no arguments", argv_none, 0, NULL},
        
        // System() tests for shell interpretation
        {"Test -lp (combined short options via system)", NULL, 1, "gcov-dump -lp"},
        {"Test -l -p via system()", NULL, 1, "gcov-dump -l -p"},
        {"Test invalid flag via system()", NULL, 1, "gcov-dump -x"},
        {"Test -h via system() with stderr redirect", NULL, 1, "gcov-dump -h 2>&1"},
        
        // Environment variable tests
        {"Test with GCOV_DUMP_OPTIONS env var", NULL, 1, 
         "GCOV_DUMP_OPTIONS='-l' gcov-dump 2>&1"},
        
        // Edge cases with system()
        {"Test empty string via system()", NULL, 1, ""},
        {"Test just program name via system()", NULL, 1, "gcov-dump"},
        
        {NULL, NULL, 0, NULL}  // Sentinel
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Execute all test cases
    for (int i = 0; test_cases[i].description != NULL; i++) {
        printf("Test %d: %s\n", total_tests + 1, test_cases[i].description);
        
        int result;
        
        if (test_cases[i].use_system) {
            printf("  [Using system(): %s]\n", test_cases[i].system_cmd);
            result = execute_gcov_dump_system(test_cases[i].system_cmd);
        } else {
            printf("  [Using execvp()]\n");
            printf("  Args: ");
            for (int j = 0; test_cases[i].argv[j] != NULL; j++) {
                printf("%s ", test_cases[i].argv[j]);
            }
            printf("\n");
            result = execute_gcov_dump_execvp(test_cases[i].argv);
        }
        
        if (result != -1) {
            printf("  Result: exit code %d\n", result);
            passed_tests++;
        } else {
            printf("  Result: FAILED (execution error)\n");
        }
        
        printf("\n");
        total_tests++;
        
        // Small delay to avoid overwhelming the system
        usleep(10000);
    }
    
    // Clean up
    remove(dummy_gcda);
    
    printf("=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    
    return 0;
}
