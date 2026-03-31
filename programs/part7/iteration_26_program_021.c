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
#include <sys/wait.h>
#include <sys/types.h>
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
    unsigned int magic = 0x67636461;  // 'gcda' in little-endian
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
 * Execute gcov-dump using execvp (precise argument control)
 */
static int test_with_execvp(const char *args[]) {
    pid_t pid = fork();
    
    if (pid == -1) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        // Child process
        execvp("gcov-dump", (char *const *)args);
        
        // If execvp returns, it failed
        fprintf(stderr, "execvp failed for gcov-dump");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    }
}

/**
 * Execute gcov-dump using system() (shell interpretation)
 */
static int test_with_system(const char *cmd) {
    int ret = system(cmd);
    
    if (ret == -1) {
        perror("system() failed");
        return -1;
    }
    
    return WEXITSTATUS(ret);
}

int main(void) {
    printf("=== Testing gcov-dump flag parsing (targeting lines 111-130) ===\n\n");
    
    // Create dummy .gcda file for file argument tests
    if (create_dummy_gcda() != 0) {
        fprintf(stderr, "Warning: Could not create dummy.gcda, file-based tests may fail\n");
    }
    
    // Test Case 1: Individual flag tests (direct switch cases)
    printf("1. Testing individual flag cases:\n");
    printf("---------------------------------\n");
    
    // -h: Help flag
    {
        const char *args[] = {"gcov-dump", "-h", NULL};
        printf("Testing: gcov-dump -h\n");
        test_with_execvp(args);
        printf("\n");
    }
    
    // -v: Version flag
    {
        const char *args[] = {"gcov-dump", "-v", NULL};
        printf("Testing: gcov-dump -v\n");
        test_with_execvp(args);
        printf("\n");
    }
    
    // -l: Contents dump flag
    {
        const char *args[] = {"gcov-dump", "-l", NULL};
        printf("Testing: gcov-dump -l (no file)\n");
        test_with_execvp(args);
        printf("\n");
    }
    
    // -p: Positions dump flag
    {
        const char *args[] = {"gcov-dump", "-p", NULL};
        printf("Testing: gcov-dump -p (no file)\n");
        test_with_execvp(args);
        printf("\n");
    }
    
    // -r: Raw dump flag
    {
        const char *args[] = {"gcov-dump", "-r", NULL};
        printf("Testing: gcov-dump -r (no file)\n");
        test_with_execvp(args);
        printf("\n");
    }
    
    // -s: Stable dump flag
    {
        const char *args[] = {"gcov-dump", "-s", NULL};
        printf("Testing: gcov-dump -s (no file)\n");
        test_with_execvp(args);
        printf("\n");
    }
    
    // -x: Invalid flag (triggers default case)
    {
        const char *args[] = {"gcov-dump", "-x", NULL};
        printf("Testing: gcov-dump -x (invalid flag, should trigger 'unknown flag')\n");
        test_with_execvp(args);
        printf("\n");
    }
    
    // Test Case 2: Flag combinations
    printf("\n2. Testing flag combinations:\n");
    printf("-----------------------------\n");
    
    // Combination 1: -l -p
    {
        const char *args[] = {"gcov-dump", "-l", "-p", NULL};
        printf("Testing: gcov-dump -l -p\n");
        test_with_execvp(args);
        printf("\n");
    }
    
    // Combination 2: -r -s -v
    {
        const char *args[] = {"gcov-dump", "-r", "-s", "-v", NULL};
        printf("Testing: gcov-dump -r -s -v\n");
        test_with_execvp(args);
        printf("\n");
    }
    
    // Combination 3: -h -l (help may cause early exit)
    {
        const char *args[] = {"gcov-dump", "-h", "-l", NULL};
        printf("Testing: gcov-dump -h -l (help with other flags)\n");
        test_with_execvp(args);
        printf("\n");
    }
    
    // Combination 4: -p -p (repeated flag)
    {
        const char *args[] = {"gcov-dump", "-p", "-p", NULL};
        printf("Testing: gcov-dump -p -p (repeated flag)\n");
        test_with_execvp(args);
        printf("\n");
    }
    
    // Test Case 3: Different flag syntax styles
    printf("\n3. Testing different flag syntax styles:\n");
    printf("---------------------------------------\n");
    
    // Combined short options: -lp (if getopt supports it)
    {
        const char *args[] = {"gcov-dump", "-lp", NULL};
        printf("Testing: gcov-dump -lp (combined flags)\n");
        test_with_execvp(args);
        printf("\n");
    }
    
    // Combined short options: -lprs
    {
        const char *args[] = {"gcov-dump", "-lprs", NULL};
        printf("Testing: gcov-dump -lprs (multiple combined flags)\n");
        test_with_execvp(args);
        printf("\n");
    }
    
    // Test Case 4: Flags with positional arguments
    printf("\n4. Testing flags with positional arguments:\n");
    printf("------------------------------------------\n");
    
    // -l with file argument
    {
        const char *args[] = {"gcov-dump", "-l", "dummy.gcda", NULL};
        printf("Testing: gcov-dump -l dummy.gcda\n");
        test_with_execvp(args);
        printf("\n");
    }
    
    // -p -r with file argument
    {
        const char *args[] = {"gcov-dump", "-p", "-r", "dummy.gcda", NULL};
        printf("Testing: gcov-dump -p -r dummy.gcda\n");
        test_with_execvp(args);
        printf("\n");
    }
    
    // With -- delimiter
    {
        const char *args[] = {"gcov-dump", "-l", "--", "dummy.gcda", NULL};
        printf("Testing: gcov-dump -l -- dummy.gcda\n");
        test_with_execvp(args);
        printf("\n");
    }
    
    // Test Case 5: Environment and error contexts
    printf("\n5. Testing environment and error contexts:\n");
    printf("-----------------------------------------\n");
    
    // No arguments (may trigger default behavior or error)
    {
        const char *args[] = {"gcov-dump", NULL};
        printf("Testing: gcov-dump (no arguments)\n");
        test_with_execvp(args);
        printf("\n");
    }
    
    // Set environment variable (if supported)
    {
        printf("Testing with GCOV_DUMP_OPTIONS environment variable\n");
        setenv("GCOV_DUMP_OPTIONS", "-l -p", 1);
        const char *args[] = {"gcov-dump", "dummy.gcda", NULL};
        test_with_execvp(args);
        unsetenv("GCOV_DUMP_OPTIONS");
        printf("\n");
    }
    
    // Test Case 6: Using system() calls (shell interpretation)
    printf("\n6. Testing with system() calls:\n");
    printf("-------------------------------\n");
    
    // Test invalid flag with stderr redirection
    {
        printf("Testing: gcov-dump -x 2>&1 (capturing stderr)\n");
        test_with_system("gcov-dump -x 2>&1");
        printf("\n");
    }
    
    // Test with shell interpretation of combined flags
    {
        printf("Testing: gcov-dump -l -p dummy.gcda\n");
        test_with_system("gcov-dump -l -p dummy.gcda 2>&1");
        printf("\n");
    }
    
    // Test with quoted arguments
    {
        printf("Testing: gcov-dump '-l' '-p' 'dummy.gcda'\n");
        test_with_system("gcov-dump '-l' '-p' 'dummy.gcda' 2>&1");
        printf("\n");
    }
    
    // Test Case 7: Edge cases
    printf("\n7. Testing edge cases:\n");
    printf("----------------------\n");
    
    // Empty flag argument
    {
        const char *args[] = {"gcov-dump", "-", NULL};
        printf("Testing: gcov-dump - (single dash)\n");
        test_with_execvp(args);
        printf("\n");
    }
    
    // Double dash alone
    {
        const char *args[] = {"gcov-dump", "--", NULL};
        printf("Testing: gcov-dump -- (double dash alone)\n");
        test_with_execvp(args);
        printf("\n");
    }
    
    // Flag with equals sign (not typical for short options)
    {
        printf("Testing: gcov-dump -l=dummy.gcda (with equals)\n");
        test_with_system("gcov-dump -l=dummy.gcda 2>&1");
        printf("\n");
    }
    
    // Clean up
    remove("dummy.gcda");
    
    printf("\n=== All gcov-dump flag parsing tests completed ===\n");
    
    return 0;
}
