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
 * This generates a simple C program, compiles it with coverage flags,
 * runs it to produce a .gcda file, and cleans up intermediate files.
 */
static int create_dummy_gcda(void) {
    const char *source = 
        "int main() { return 0; }\n";
    
    FILE *fp = fopen("dummy.c", "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return -1;
    }
    fputs(source, fp);
    fclose(fp);
    
    // Compile with coverage flags
    int ret = system("gcc -fprofile-arcs -ftest-coverage -O0 dummy.c -o dummy");
    if (ret != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return -1;
    }
    
    // Run to generate .gcda file
    ret = system("./dummy");
    if (ret != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        return -1;
    }
    
    // Clean up intermediate files (keep dummy.gcda)
    system("rm -f dummy dummy.c dummy.gcno");
    
    return 0;
}

/**
 * Executes gcov-dump with given arguments using execvp.
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
        perror("execvp failed");
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
 * Tests a specific argument set and prints the result.
 */
static void test_args(const char *test_name, const char *args[]) {
    printf("\n=== Testing: %s ===\n", test_name);
    printf("Command: ");
    
    for (int i = 0; args[i] != NULL; i++) {
        printf("%s ", args[i]);
    }
    printf("\n");
    
    fflush(stdout);
    
    int ret = exec_gcov_dump(args);
    printf("Exit status: %d\n", ret);
}

int main(void) {
    printf("=== Starting gcov-dump parser tests ===\n");
    printf("Targeting switch cases in gcov-dump.cc lines 111-130\n\n");
    
    // Create a dummy .gcda file for tests that need it
    printf("Creating dummy .gcda file...\n");
    if (create_dummy_gcda() != 0) {
        fprintf(stderr, "Warning: Failed to create dummy .gcda file\n");
        fprintf(stderr, "Tests requiring file arguments may fail\n");
    }
    
    // Test individual flags (direct switch cases)
    const char *help_args[] = {"gcov-dump", "-h", NULL};
    test_args("Help flag (-h)", help_args);
    
    const char *version_args[] = {"gcov-dump", "-v", NULL};
    test_args("Version flag (-v)", version_args);
    
    const char *contents_args[] = {"gcov-dump", "-l", NULL};
    test_args("Contents dump flag (-l)", contents_args);
    
    const char *positions_args[] = {"gcov-dump", "-p", NULL};
    test_args("Positions dump flag (-p)", positions_args);
    
    const char *raw_args[] = {"gcov-dump", "-r", NULL};
    test_args("Raw dump flag (-r)", raw_args);
    
    const char *stable_args[] = {"gcov-dump", "-s", NULL};
    test_args("Stable dump flag (-s)", stable_args);
    
    // Test invalid flag (default case)
    const char *invalid_args[] = {"gcov-dump", "-x", NULL};
    test_args("Invalid flag (-x) - should trigger 'unknown flag'", invalid_args);
    
    // Test flag combinations (multiple valid flags)
    const char *combo1_args[] = {"gcov-dump", "-l", "-p", NULL};
    test_args("Combination: -l -p", combo1_args);
    
    const char *combo2_args[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    test_args("Combination: -r -s -v", combo2_args);
    
    const char *combo3_args[] = {"gcov-dump", "-h", "-l", NULL};
    test_args("Combination: -h -l (h may cause early exit)", combo3_args);
    
    // Test repeated flags
    const char *repeat_args[] = {"gcov-dump", "-p", "-p", NULL};
    test_args("Repeated flag: -p -p", repeat_args);
    
    // Test combined short options (if supported by getopt)
    const char *combined_args[] = {"gcov-dump", "-lp", NULL};
    test_args("Combined short options: -lp", combined_args);
    
    // Test with positional arguments (gcov files)
    const char *with_file_args[] = {"gcov-dump", "-l", "dummy.gcda", NULL};
    test_args("With file argument: -l dummy.gcda", with_file_args);
    
    // Test with -- delimiter
    const char *with_delimiter_args[] = {"gcov-dump", "-l", "--", "dummy.gcda", NULL};
    test_args("With -- delimiter: -l -- dummy.gcda", with_delimiter_args);
    
    // Test no arguments
    const char *no_args[] = {"gcov-dump", NULL};
    test_args("No arguments", no_args);
    
    // Test using system() calls for different execution path
    printf("\n=== Testing with system() calls ===\n");
    
    // Set environment variable before invocation
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    
    const char *system_tests[] = {
        "gcov-dump -h",
        "gcov-dump -x 2>&1",  // Redirect stderr to capture "unknown flag"
        "gcov-dump -l -p dummy.gcda",
        "gcov-dump -r -- dummy.gcda",
        NULL
    };
    
    for (int i = 0; system_tests[i] != NULL; i++) {
        printf("\nSystem test: %s\n", system_tests[i]);
        int ret = system(system_tests[i]);
        printf("Exit status: %d\n", ret);
    }
    
    // Test with invalid environment variable
    setenv("GCOV_DUMP_OPTIONS", "-invalid", 1);
    printf("\nTesting with invalid GCOV_DUMP_OPTIONS environment variable\n");
    system("gcov-dump -v 2>&1");
    
    // Clean up
    printf("\n=== Cleaning up ===\n");
    system("rm -f dummy.gcda");
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
