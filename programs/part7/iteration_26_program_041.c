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
    unsigned int magic = 0x67636461;  // 'gcda' in little-endian
    unsigned int version = 0x4020000; // GCOV version 4.2
    
    fwrite(&magic, sizeof(magic), 1, fp);
    fwrite(&version, sizeof(version), 1, fp);
    
    // Write a simple tag with zero length
    unsigned int tag = 0;  // GCOV_TAG_FUNCTION
    unsigned int length = 0;
    
    fwrite(&tag, sizeof(tag), 1, fp);
    fwrite(&length, sizeof(length), 1, fp);
    
    fclose(fp);
    return 0;
}

/**
 * Executes gcov-dump with given arguments using execvp.
 * Returns exit status of gcov-dump.
 */
int exec_gcov_dump(const char *args[]) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        // Child process
        execvp("gcov-dump", (char * const *)args);
        
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
 * Executes gcov-dump using system() call.
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
    
    // Test 1: Individual flag tests (direct switch cases)
    printf("--- Test 1: Individual flags ---\n");
    
    // Case 'h': Help flag
    const char *help_args[] = {"gcov-dump", "-h", NULL};
    printf("Testing -h (help):\n");
    exec_gcov_dump(help_args);
    
    // Case 'v': Version flag
    const char *version_args[] = {"gcov-dump", "-v", NULL};
    printf("\nTesting -v (version):\n");
    exec_gcov_dump(version_args);
    
    // Case 'l': Contents dump flag
    const char *contents_args[] = {"gcov-dump", "-l", NULL};
    printf("\nTesting -l (contents dump):\n");
    exec_gcov_dump(contents_args);
    
    // Case 'p': Positions dump flag
    const char *positions_args[] = {"gcov-dump", "-p", NULL};
    printf("\nTesting -p (positions dump):\n");
    exec_gcov_dump(positions_args);
    
    // Case 'r': Raw dump flag
    const char *raw_args[] = {"gcov-dump", "-r", NULL};
    printf("\nTesting -r (raw dump):\n");
    exec_gcov_dump(raw_args);
    
    // Case 's': Stable dump flag
    const char *stable_args[] = {"gcov-dump", "-s", NULL};
    printf("\nTesting -s (stable dump):\n");
    exec_gcov_dump(stable_args);
    
    // Default case: Invalid flag
    const char *invalid_args[] = {"gcov-dump", "-x", NULL};
    printf("\nTesting -x (invalid flag, should trigger default case):\n");
    exec_gcov_dump(invalid_args);
    
    // Test 2: Flag combinations
    printf("\n--- Test 2: Flag combinations ---\n");
    
    // Combination 1: -l -p
    const char *combo1_args[] = {"gcov-dump", "-l", "-p", NULL};
    printf("Testing -l -p:\n");
    exec_gcov_dump(combo1_args);
    
    // Combination 2: -r -s -v
    const char *combo2_args[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    printf("\nTesting -r -s -v:\n");
    exec_gcov_dump(combo2_args);
    
    // Combination 3: -h -l (h should cause early exit)
    const char *combo3_args[] = {"gcov-dump", "-h", "-l", NULL};
    printf("\nTesting -h -l (help with extra flag):\n");
    exec_gcov_dump(combo3_args);
    
    // Combination 4: Repeated flag -p -p
    const char *combo4_args[] = {"gcov-dump", "-p", "-p", NULL};
    printf("\nTesting -p -p (repeated flag):\n");
    exec_gcov_dump(combo4_args);
    
    // Test 3: Different flag syntax styles
    printf("\n--- Test 3: Different flag syntax ---\n");
    
    // Combined short options: -lp (if supported)
    const char *combined_args[] = {"gcov-dump", "-lp", NULL};
    printf("Testing -lp (combined flags):\n");
    exec_gcov_dump(combined_args);
    
    // With positional argument (gcov file)
    const char *with_file_args[] = {"gcov-dump", "-l", dummy_gcda, NULL};
    printf("\nTesting -l with file argument:\n");
    exec_gcov_dump(with_file_args);
    
    // With -- delimiter
    const char *with_delimiter_args[] = {"gcov-dump", "-l", "--", dummy_gcda, NULL};
    printf("\nTesting -l -- file (with delimiter):\n");
    exec_gcov_dump(with_delimiter_args);
    
    // Test 4: Environment and error contexts
    printf("\n--- Test 4: Environment and error contexts ---\n");
    
    // No arguments
    const char *no_args[] = {"gcov-dump", NULL};
    printf("Testing with no arguments:\n");
    exec_gcov_dump(no_args);
    
    // Set environment variable (if supported)
    printf("\nTesting with GCOV_DUMP_OPTIONS environment variable:\n");
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    exec_gcov_dump(no_args);
    unsetenv("GCOV_DUMP_OPTIONS");
    
    // Test 5: Using system() calls for different code path
    printf("\n--- Test 5: system() call tests ---\n");
    
    system_gcov_dump("gcov-dump -h");
    system_gcov_dump("gcov-dump -v");
    system_gcov_dump("gcov-dump -l -p dummy.gcda 2>&1");
    system_gcov_dump("gcov-dump -x 2>&1");  // Redirect stderr to see "unknown flag"
    
    // Test 6: Edge cases
    printf("\n--- Test 6: Edge cases ---\n");
    
    // Empty flag
    const char *empty_flag_args[] = {"gcov-dump", "-", NULL};
    printf("Testing empty flag '-':\n");
    exec_gcov_dump(empty_flag_args);
    
    // Long argument (should be ignored by getopt for single char opts)
    const char *long_arg_args[] = {"gcov-dump", "--help", NULL};
    printf("\nTesting --help (long argument):\n");
    exec_gcov_dump(long_arg_args);
    
    // Multiple files
    const char *multi_file_args[] = {"gcov-dump", "-l", dummy_gcda, dummy_gcda, NULL};
    printf("\nTesting with multiple files:\n");
    exec_gcov_dump(multi_file_args);
    
    // Clean up
    remove(dummy_gcda);
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
