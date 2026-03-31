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
 * This creates a valid gcov data file header that gcov-dump can parse.
 */
int create_dummy_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create dummy .gcda file");
        return -1;
    }
    
    // Write minimal gcov data file magic and version
    // GCOV data magic: 0x67636461 (gcda in little-endian)
    unsigned int magic = 0x67636461;
    unsigned int version = 0x20100528;  // Common gcov version
    
    fwrite(&magic, sizeof(magic), 1, fp);
    fwrite(&version, sizeof(version), 1, fp);
    
    // Write a zero tag to indicate end of file
    unsigned int zero_tag = 0;
    fwrite(&zero_tag, sizeof(zero_tag), 1, fp);
    
    fclose(fp);
    return 0;
}

/**
 * Execute gcov-dump using execvp for precise argument control.
 * Returns the child process exit status.
 */
int exec_gcov_dump(const char *args[]) {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        execvp("gcov-dump", (char *const *)args);
        // If execvp returns, it failed
        fprintf(stderr, "execvp failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    } else {
        // Fork failed
        perror("fork failed");
        return -1;
    }
}

/**
 * Execute gcov-dump using system() for shell interpretation testing.
 */
int system_gcov_dump(const char *cmd) {
    int status = system(cmd);
    if (status == -1) {
        perror("system() failed");
        return -1;
    }
    return WEXITSTATUS(status);
}

int main(int argc, char *argv[]) {
    printf("=== Testing gcov-dump flag parsing logic ===\n\n");
    
    // Create a dummy .gcda file for file-based tests
    const char *dummy_file = "dummy.gcda";
    if (create_dummy_gcda(dummy_file) != 0) {
        fprintf(stderr, "Warning: Could not create dummy .gcda file\n");
        dummy_file = NULL;
    }
    
    // Test 1: Individual flag tests (direct switch cases)
    printf("--- Test 1: Individual flags ---\n");
    
    // Case 'h': Help flag
    {
        const char *args[] = {"gcov-dump", "-h", NULL};
        printf("Testing -h (help): ");
        fflush(stdout);
        exec_gcov_dump(args);
        printf("Done\n");
    }
    
    // Case 'v': Version flag
    {
        const char *args[] = {"gcov-dump", "-v", NULL};
        printf("Testing -v (version): ");
        fflush(stdout);
        exec_gcov_dump(args);
        printf("Done\n");
    }
    
    // Case 'l': Contents dump flag
    {
        const char *args[] = {"gcov-dump", "-l", NULL};
        printf("Testing -l (contents dump): ");
        fflush(stdout);
        exec_gcov_dump(args);
        printf("Done\n");
    }
    
    // Case 'p': Positions dump flag
    {
        const char *args[] = {"gcov-dump", "-p", NULL};
        printf("Testing -p (positions dump): ");
        fflush(stdout);
        exec_gcov_dump(args);
        printf("Done\n");
    }
    
    // Case 'r': Raw dump flag
    {
        const char *args[] = {"gcov-dump", "-r", NULL};
        printf("Testing -r (raw dump): ");
        fflush(stdout);
        exec_gcov_dump(args);
        printf("Done\n");
    }
    
    // Case 's': Stable dump flag
    {
        const char *args[] = {"gcov-dump", "-s", NULL};
        printf("Testing -s (stable dump): ");
        fflush(stdout);
        exec_gcov_dump(args);
        printf("Done\n");
    }
    
    // Default case: Invalid flag
    {
        const char *args[] = {"gcov-dump", "-x", NULL};
        printf("Testing -x (invalid flag, should trigger default case): ");
        fflush(stdout);
        exec_gcov_dump(args);
        printf("Done\n");
    }
    
    printf("\n--- Test 2: Flag combinations ---\n");
    
    // Multiple valid flags in separate arguments
    {
        const char *args[] = {"gcov-dump", "-l", "-p", NULL};
        printf("Testing -l -p (separate): ");
        fflush(stdout);
        exec_gcov_dump(args);
        printf("Done\n");
    }
    
    {
        const char *args[] = {"gcov-dump", "-r", "-s", "-v", NULL};
        printf("Testing -r -s -v: ");
        fflush(stdout);
        exec_gcov_dump(args);
        printf("Done\n");
    }
    
    // Same flag repeated
    {
        const char *args[] = {"gcov-dump", "-p", "-p", NULL};
        printf("Testing -p -p (repeated flag): ");
        fflush(stdout);
        exec_gcov_dump(args);
        printf("Done\n");
    }
    
    // Help with other flags (may exit early)
    {
        const char *args[] = {"gcov-dump", "-h", "-l", NULL};
        printf("Testing -h -l (help with other flag): ");
        fflush(stdout);
        exec_gcov_dump(args);
        printf("Done\n");
    }
    
    printf("\n--- Test 3: Different flag syntax styles ---\n");
    
    // Combined short options (if supported by getopt)
    {
        const char *args[] = {"gcov-dump", "-lp", NULL};
        printf("Testing -lp (combined): ");
        fflush(stdout);
        exec_gcov_dump(args);
        printf("Done\n");
    }
    
    {
        const char *args[] = {"gcov-dump", "-rs", NULL};
        printf("Testing -rs (combined): ");
        fflush(stdout);
        exec_gcov_dump(args);
        printf("Done\n");
    }
    
    // With positional arguments (gcov files)
    if (dummy_file) {
        const char *args[] = {"gcov-dump", "-l", dummy_file, NULL};
        printf("Testing -l with file argument: ");
        fflush(stdout);
        exec_gcov_dump(args);
        printf("Done\n");
    }
    
    // With -- delimiter
    if (dummy_file) {
        const char *args[] = {"gcov-dump", "-l", "--", dummy_file, NULL};
        printf("Testing -l -- file (with delimiter): ");
        fflush(stdout);
        exec_gcov_dump(args);
        printf("Done\n");
    }
    
    printf("\n--- Test 4: Environment and error contexts ---\n");
    
    // No arguments
    {
        const char *args[] = {"gcov-dump", NULL};
        printf("Testing no arguments: ");
        fflush(stdout);
        exec_gcov_dump(args);
        printf("Done\n");
    }
    
    // Set environment variable (if supported)
    {
        printf("Testing with GCOV_DUMP_OPTIONS environment variable: ");
        setenv("GCOV_DUMP_OPTIONS", "-v", 1);
        const char *args[] = {"gcov-dump", NULL};
        exec_gcov_dump(args);
        unsetenv("GCOV_DUMP_OPTIONS");
        printf("Done\n");
    }
    
    // Using system() for shell interpretation
    printf("\n--- Test 5: Using system() calls ---\n");
    
    // Test invalid flag with stderr redirection
    {
        printf("Testing invalid flag with system() and stderr capture: ");
        fflush(stdout);
        system_gcov_dump("gcov-dump -x 2>&1");
        printf("Done\n");
    }
    
    // Test combined flags with system()
    {
        printf("Testing -l -p with system(): ");
        fflush(stdout);
        system_gcov_dump("gcov-dump -l -p");
        printf("Done\n");
    }
    
    // Test with file argument using system()
    if (dummy_file) {
        printf("Testing with file using system(): ");
        fflush(stdout);
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", dummy_file);
        system_gcov_dump(cmd);
        printf("Done\n");
    }
    
    // Test error output redirection
    {
        printf("Testing invalid flag with stderr to file: ");
        fflush(stdout);
        system_gcov_dump("gcov-dump -x 2> /tmp/gcov-dump-error.txt");
        printf("(Check /tmp/gcov-dump-error.txt for 'unknown flag' message)\n");
    }
    
    // Clean up dummy file
    if (dummy_file) {
        unlink(dummy_file);
    }
    
    printf("\n=== All tests completed ===\n");
    printf("Note: Some tests may fail if gcov-dump is not in PATH\n");
    printf("or if it requires valid .gcda files for certain operations.\n");
    
    return 0;
}
