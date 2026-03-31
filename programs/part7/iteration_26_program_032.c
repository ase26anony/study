/**
 * gcov-dump_parser_test.c
 * 
 * Comprehensive test program for gcov-dump command-line flag parsing.
 * Specifically targets the switch-case logic in gcov-dump.cc lines 111-130.
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
    const char *source = 
        "int main() { return 0; }\n";
    
    const char *source_file = "dummy_test.c";
    const char *executable = "dummy_test";
    const char *gcda_file = "dummy_test.gcda";
    
    FILE *fp = fopen(source_file, "w");
    if (!fp) {
        perror("Failed to create dummy source file");
        return -1;
    }
    fputs(source, fp);
    fclose(fp);
    
    // Compile with coverage flags
    char compile_cmd[256];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s 2>/dev/null",
             executable, source_file);
    
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Warning: Failed to compile dummy test program\n");
        // Continue anyway - some tests don't need the file
        return 0;
    }
    
    // Run to generate .gcda
    char run_cmd[256];
    snprintf(run_cmd, sizeof(run_cmd), "./%s 2>/dev/null", executable);
    system(run_cmd);
    
    // Clean up intermediate files (keep .gcda)
    remove(source_file);
    remove(executable);
    remove("dummy_test.gcno");
    
    // Check if .gcda was created
    if (access(gcda_file, F_OK) != 0) {
        fprintf(stderr, "Warning: Failed to create dummy .gcda file\n");
        return 0;
    }
    
    return 0;
}

/**
 * Execute gcov-dump with given arguments using execvp.
 * Returns child process exit status.
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
        fprintf(stderr, "Failed to execute gcov-dump: %s\n", strerror(errno));
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
static void system_gcov_dump(const char *cmd) {
    printf("Testing with system(): %s\n", cmd);
    int ret = system(cmd);
    printf("Exit status: %d\n\n", ret);
}

/**
 * Main test driver.
 */
int main(void) {
    printf("=== Testing gcov-dump Flag Parser (Targeting lines 111-130) ===\n\n");
    
    // Create dummy .gcda file for tests that need it
    printf("Creating dummy .gcda file...\n");
    if (create_dummy_gcda() < 0) {
        fprintf(stderr, "Failed to create dummy .gcda, continuing anyway\n");
    }
    printf("\n");
    
    // Test cases for individual flags (using execvp for precise control)
    printf("--- Testing Individual Flags with execvp ---\n\n");
    
    // Array of argument sets for individual flag tests
    const char *individual_tests[][4] = {
        {"gcov-dump", "-h", NULL},                     // Help flag
        {"gcov-dump", "-v", NULL},                     // Version flag
        {"gcov-dump", "-l", NULL},                     // Contents dump flag
        {"gcov-dump", "-p", NULL},                     // Positions dump flag
        {"gcov-dump", "-r", NULL},                     // Raw dump flag
        {"gcov-dump", "-s", NULL},                     // Stable dump flag
        {"gcov-dump", "-x", NULL},                     // Invalid flag (triggers default case)
        {"gcov-dump", NULL},                           // No arguments
        {NULL}  // Sentinel
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
    
    // Test combinations of valid flags (using execvp)
    printf("--- Testing Flag Combinations with execvp ---\n\n");
    
    const char *combo_tests[][6] = {
        {"gcov-dump", "-l", "-p", NULL},               // Two flags separate
        {"gcov-dump", "-r", "-s", "-v", NULL},         // Three flags
        {"gcov-dump", "-h", "-l", NULL},               // Help with another flag
        {"gcov-dump", "-p", "-p", NULL},               // Repeated flag
        {"gcov-dump", "-l", "-p", "-r", "-s", NULL},   // All valid flags
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
    
    // Test different flag syntax styles (using system() for shell interpretation)
    printf("--- Testing Different Flag Syntax with system() ---\n\n");
    
    const char *syntax_tests[] = {
        "gcov-dump -l -p",                     // Separate arguments
        "gcov-dump -lp",                       // Combined short options
        "gcov-dump -l dummy_test.gcda",        // With positional argument
        "gcov-dump -l -- dummy_test.gcda",     // With -- delimiter
        "gcov-dump -l -p dummy_test.gcda",     // Multiple flags with file
        "gcov-dump dummy_test.gcda -l",        // Flag after file
        NULL
    };
    
    for (int i = 0; syntax_tests[i] != NULL; i++) {
        system_gcov_dump(syntax_tests[i]);
    }
    
    // Test with environment variables
    printf("--- Testing with Environment Variables ---\n\n");
    
    // Set environment variable if gcov-dump reads it
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    system_gcov_dump("gcov-dump -l");
    
    setenv("GCOV_DUMP_OPTIONS", "-x", 1);  // Invalid flag in env
    system_gcov_dump("gcov-dump -h");
    
    unsetenv("GCOV_DUMP_OPTIONS");
    
    // Test error stream redirection for invalid flag
    printf("--- Testing Error Output Capture ---\n\n");
    
    printf("Testing invalid flag with stderr redirection:\n");
    system("gcov-dump -x 2>&1 | grep -q 'unknown flag' && echo 'Found error message' || echo 'No error message'");
    printf("\n");
    
    // Test with non-existent file
    printf("Testing with non-existent file:\n");
    system_gcov_dump("gcov-dump -l nonexistent.gcda 2>&1");
    
    // Clean up
    printf("--- Cleaning up ---\n");
    remove("dummy_test.gcda");
    
    printf("\n=== All tests completed ===\n");
    
    return 0;
}
