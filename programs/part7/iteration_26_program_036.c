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
 * Creates a minimal valid .gcda file for testing.
 * Uses gcc's coverage instrumentation to generate a real .gcda file.
 */
static int create_dummy_gcda(void) {
    const char *source_code = 
        "int main() { return 0; }\n";
    
    const char *source_file = "dummy_test.c";
    const char *gcda_file = "dummy_test.gcda";
    
    FILE *fp = fopen(source_file, "w");
    if (!fp) {
        perror("Failed to create dummy source file");
        return -1;
    }
    fputs(source_code, fp);
    fclose(fp);
    
    // Compile with coverage flags
    char compile_cmd[256];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -O0 -o dummy_test %s 2>/dev/null",
             source_file);
    
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Warning: Failed to compile dummy test program\n");
        // Continue anyway - some tests don't need actual .gcda files
        return 0;
    }
    
    // Run the program to generate .gcda
    if (system("./dummy_test 2>/dev/null") != 0) {
        fprintf(stderr, "Warning: Failed to run dummy test program\n");
    }
    
    // Clean up intermediate files (keep .gcda)
    remove("dummy_test");
    remove(source_file);
    remove("dummy_test.gcno");
    
    return 0;
}

/**
 * Executes gcov-dump using execvp for precise argument control.
 * Returns the exit status of gcov-dump.
 */
static int exec_gcov_dump(const char *args[]) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        // Child process
        execvp("gcov-dump", (char *const *)args);
        // If execvp returns, it failed
        fprintf(stderr, "execvp failed for gcov-dump: %s\n", strerror(errno));
        exit(127);
    }
    
    // Parent process
    int status;
    waitpid(pid, &status, 0);
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    } else {
        return -1;
    }
}

/**
 * Executes gcov-dump using system() to test that path.
 */
static void system_gcov_dump(const char *cmd) {
    printf("[system() test] Executing: %s\n", cmd);
    int ret = system(cmd);
    printf("Exit status: %d\n\n", ret);
}

int main(void) {
    printf("=== gcov-dump Flag Parser Test ===\n\n");
    
    // Create a dummy .gcda file for tests that need it
    printf("Creating dummy .gcda file for testing...\n");
    create_dummy_gcda();
    printf("\n");
    
    // Test cases targeting specific switch cases
    const char *test_cases[][5] = {
        // Individual flags (direct switch cases)
        {"gcov-dump", "-h", NULL},                     // case 'h'
        {"gcov-dump", "-v", NULL},                     // case 'v'
        {"gcov-dump", "-l", NULL},                     // case 'l'
        {"gcov-dump", "-p", NULL},                     // case 'p'
        {"gcov-dump", "-r", NULL},                     // case 'r'
        {"gcov-dump", "-s", NULL},                     // case 's'
        {"gcov-dump", "-x", NULL},                     // default case (invalid flag)
        
        // Flag combinations (multiple valid flags)
        {"gcov-dump", "-l", "-p", NULL},               // -l -p combination
        {"gcov-dump", "-r", "-s", "-v", NULL},         // -r -s -v combination
        {"gcov-dump", "-h", "-l", NULL},               // -h with other flag
        
        // Repeated flags
        {"gcov-dump", "-p", "-p", NULL},               // -p repeated
        {"gcov-dump", "-l", "-l", "-l", NULL},         // -l repeated multiple times
        
        // With positional arguments (.gcda files)
        {"gcov-dump", "-l", "dummy_test.gcda", NULL},  // -l with file
        {"gcov-dump", "-p", "-r", "dummy_test.gcda", NULL}, // Multiple flags with file
        
        // No arguments (should trigger some default behavior or error)
        {"gcov-dump", NULL},
        
        // Combined short options (if supported by getopt)
        {"gcov-dump", "-lp", NULL},                    // Combined -l -p
        {"gcov-dump", "-rs", NULL},                    // Combined -r -s
        
        // With -- delimiter
        {"gcov-dump", "-l", "--", "dummy_test.gcda", NULL},
        
        // Edge cases
        {"gcov-dump", "--", "-l", NULL},               // -- with flag after
        {"gcov-dump", "-", NULL},                      // Just a dash
    };
    
    // Execute all test cases using execvp
    printf("=== Testing with execvp() ===\n");
    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        printf("Test %zu: ", i + 1);
        for (int j = 0; test_cases[i][j] != NULL; j++) {
            printf("%s ", test_cases[i][j]);
        }
        printf("\n");
        
        int status = exec_gcov_dump(test_cases[i]);
        printf("Exit status: %d\n\n", status);
    }
    
    // Additional tests using system() for different execution path
    printf("\n=== Testing with system() ===\n");
    
    // Set environment variable that might affect gcov-dump
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    
    // Test various command formats with system()
    const char *system_tests[] = {
        "gcov-dump -h 2>&1",
        "gcov-dump -v 2>&1",
        "gcov-dump -l -p 2>&1",
        "gcov-dump -lp 2>&1",           // Combined flags
        "gcov-dump -x 2>&1",            // Invalid flag (should go to stderr)
        "gcov-dump -l dummy_test.gcda 2>&1",
        "gcov-dump -- -l 2>&1",         // Flag after --
        "gcov-dump 2>&1",               // No arguments
    };
    
    for (size_t i = 0; i < sizeof(system_tests) / sizeof(system_tests[0]); i++) {
        system_gcov_dump(system_tests[i]);
    }
    
    // Test with different environment variable
    printf("\n=== Testing with different environment ===\n");
    setenv("GCOV_DUMP_OPTIONS", "-s -r", 1);
    system_gcov_dump("gcov-dump -v 2>&1");
    
    // Unset environment variable
    unsetenv("GCOV_DUMP_OPTIONS");
    system_gcov_dump("gcov-dump -v 2>&1");
    
    // Clean up
    printf("\n=== Cleaning up ===\n");
    remove("dummy_test.gcda");
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
