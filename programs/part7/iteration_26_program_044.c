/**
 * gcov-dump_parser_test.c
 * 
 * Comprehensive test program for gcov-dump command-line flag parsing.
 * Specifically targets uncovered lines 111-130 in gcov-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/**
 * Create a minimal dummy .gcda file for testing with file arguments
 */
static int create_dummy_gcda(void) {
    FILE *fp = fopen("dummy.gcda", "wb");
    if (!fp) {
        perror("Failed to create dummy.gcda");
        return -1;
    }
    
    // Write minimal GCOV data magic and version
    unsigned int magic = 0x67636461; // 'gcda' in little-endian
    unsigned int version = 0x4020000; // GCOV version 4.2
    
    fwrite(&magic, sizeof(magic), 1, fp);
    fwrite(&version, sizeof(version), 1, fp);
    
    // Write a terminator
    unsigned int tag = 0;
    fwrite(&tag, sizeof(tag), 1, fp);
    
    fclose(fp);
    printf("Created dummy.gcda for testing\n");
    return 0;
}

/**
 * Execute gcov-dump using execvp for precise argument control
 */
static int exec_gcov_dump(const char *args[], const char *test_name) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        // Child process
        printf("\n=== Test: %s ===\n", test_name);
        printf("Executing: ");
        for (int i = 0; args[i] != NULL; i++) {
            printf("%s ", args[i]);
        }
        printf("\n");
        
        execvp("gcov-dump", (char *const *)args);
        
        // If execvp returns, it failed
        fprintf(stderr, "Failed to execute gcov-dump: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("Exit status: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Terminated by signal: %d\n", WTERMSIG(status));
        }
        
        return status;
    }
}

/**
 * Execute gcov-dump using system() for shell interpretation testing
 */
static int system_gcov_dump(const char *cmd, const char *test_name) {
    printf("\n=== Test (system): %s ===\n", test_name);
    printf("Executing: %s\n", cmd);
    
    int status = system(cmd);
    
    if (status == -1) {
        perror("system() failed");
    } else if (WIFEXITED(status)) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
    }
    
    return status;
}

int main(void) {
    printf("Starting gcov-dump flag parser tests...\n");
    printf("Targeting uncovered lines 111-130 in gcov-dump.cc\n\n");
    
    // Create dummy .gcda file for tests that require file arguments
    if (create_dummy_gcda() != 0) {
        fprintf(stderr, "Warning: Could not create dummy.gcda, some tests may fail\n");
    }
    
    // Test 1: Individual flag tests (direct switch cases)
    printf("\n========== INDIVIDUAL FLAG TESTS ==========\n");
    
    // Case 'h': Help flag
    const char *test1_args[] = {"gcov-dump", "-h", NULL};
    exec_gcov_dump(test1_args, "Help flag (-h)");
    
    // Case 'v': Version flag
    const char *test2_args[] = {"gcov-dump", "-v", NULL};
    exec_gcov_dump(test2_args, "Version flag (-v)");
    
    // Case 'l': Contents dump flag
    const char *test3_args[] = {"gcov-dump", "-l", NULL};
    exec_gcov_dump(test3_args, "Contents dump flag (-l)");
    
    // Case 'p': Positions dump flag
    const char *test4_args[] = {"gcov-dump", "-p", NULL};
    exec_gcov_dump(test4_args, "Positions dump flag (-p)");
    
    // Case 'r': Raw dump flag
    const char *test5_args[] = {"gcov-dump", "-r", NULL};
    exec_gcov_dump(test5_args, "Raw dump flag (-r)");
    
    // Case 's': Stable dump flag
    const char *test6_args[] = {"gcov-dump", "-s", NULL};
    exec_gcov_dump(test6_args, "Stable dump flag (-s)");
    
    // Default case: Invalid flag
    const char *test7_args[] = {"gcov-dump", "-x", NULL};
    exec_gcov_dump(test7_args, "Invalid flag (-x) to trigger default case");
    
    // Test 2: No arguments (may trigger default behavior)
    printf("\n========== NO ARGUMENTS TEST ==========\n");
    const char *test8_args[] = {"gcov-dump", NULL};
    exec_gcov_dump(test8_args, "No arguments");
    
    // Test 3: Flag combinations
    printf("\n========== FLAG COMBINATION TESTS ==========\n");
    
    // Combination 1: -l -p
    const char *test9_args[] = {"gcov-dump", "-l", "-p", NULL};
    exec_gcov_dump(test9_args, "Combination: -l -p");
    
    // Combination 2: -r -s -v
    const char *test10_args[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    exec_gcov_dump(test10_args, "Combination: -r -s -v");
    
    // Combination 3: -h -l (help may cause early exit)
    const char *test11_args[] = {"gcov-dump", "-h", "-l", NULL};
    exec_gcov_dump(test11_args, "Combination: -h -l (help with other flag)");
    
    // Test 4: Repeated flags
    printf("\n========== REPEATED FLAG TESTS ==========\n");
    
    // Repeated -p flag
    const char *test12_args[] = {"gcov-dump", "-p", "-p", NULL};
    exec_gcov_dump(test12_args, "Repeated flag: -p -p");
    
    // Test 5: Different syntactic styles using system()
    printf("\n========== SYNTAX VARIATION TESTS (system) ==========\n");
    
    // Combined short options (-lp)
    system_gcov_dump("gcov-dump -lp", "Combined short options: -lp");
    
    // With positional arguments (gcov files)
    system_gcov_dump("gcov-dump -l dummy.gcda", "With file argument: -l dummy.gcda");
    
    // With -- delimiter
    system_gcov_dump("gcov-dump -l -- dummy.gcda", "With -- delimiter: -l -- dummy.gcda");
    
    // Multiple files
    system_gcov_dump("gcov-dump -p dummy.gcda dummy.gcda", "Multiple file arguments");
    
    // Test 6: Environment variable testing
    printf("\n========== ENVIRONMENT VARIABLE TESTS ==========\n");
    
    // Set environment variable before execution
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    const char *test13_args[] = {"gcov-dump", "-l", NULL};
    exec_gcov_dump(test13_args, "With GCOV_DUMP_OPTIONS=-v and -l flag");
    
    // Test with different env var
    setenv("GCOV_DUMP_OPTIONS", "-p -r", 1);
    const char *test14_args[] = {"gcov-dump", "-s", NULL};
    exec_gcov_dump(test14_args, "With GCOV_DUMP_OPTIONS=-p -r and -s flag");
    
    // Clear environment variable
    unsetenv("GCOV_DUMP_OPTIONS");
    
    // Test 7: Edge cases with system()
    printf("\n========== EDGE CASE TESTS (system) ==========\n");
    
    // Empty string as argument
    system_gcov_dump("gcov-dump \"\"", "Empty string argument");
    
    // Only dashes
    system_gcov_dump("gcov-dump --", "Only -- argument");
    
    // Mixed valid and invalid flags
    system_gcov_dump("gcov-dump -l -x -p", "Mixed valid (-l, -p) and invalid (-x) flags");
    
    // Test 8: Output redirection tests
    printf("\n========== OUTPUT REDIRECTION TESTS ==========\n");
    
    // Redirect stderr to capture unknown flag message
    system_gcov_dump("gcov-dump -x 2>&1", "Invalid flag with stderr redirect");
    
    // Redirect both stdout and stderr
    system_gcov_dump("gcov-dump -v -l 2>&1", "Multiple flags with combined output redirect");
    
    // Test 9: Long argument list stress test
    printf("\n========== STRESS TESTS ==========\n");
    
    // Many flags
    const char *test15_args[] = {
        "gcov-dump", "-l", "-p", "-r", "-s", "-v", 
        "-l", "-p", "-r", "-s", NULL
    };
    exec_gcov_dump(test15_args, "Many repeated flags");
    
    // Flags with file arguments
    const char *test16_args[] = {
        "gcov-dump", "-l", "-p", "dummy.gcda", 
        "-r", "-s", "dummy.gcda", NULL
    };
    exec_gcov_dump(test16_args, "Flags interspersed with file arguments");
    
    // Clean up
    remove("dummy.gcda");
    
    printf("\n========== ALL TESTS COMPLETED ==========\n");
    printf("The parser switch cases for flags h, v, l, p, r, s, and default\n");
    printf("should have been exercised multiple times with various combinations.\n");
    
    return 0;
}
