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
 * Creates a minimal .gcda file for testing.
 * gcov-dump requires a valid .gcda file for some operations.
 */
static int create_dummy_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create dummy .gcda file");
        return -1;
    }
    
    // Write minimal gcov data header (magic + version)
    // This is just enough to make gcov-dump not reject it immediately
    unsigned int magic = 0x67636461; // 'gcda' in little-endian
    unsigned int version = 0x4020000; // GCC 8.2.0 version marker
    
    fwrite(&magic, sizeof(magic), 1, fp);
    fwrite(&version, sizeof(version), 1, fp);
    
    // Add a simple tag to mark end
    unsigned int tag = 0;
    fwrite(&tag, sizeof(tag), 1, fp);
    
    fclose(fp);
    return 0;
}

/**
 * Execute gcov-dump using execvp for precise argument control.
 * Returns exit status of gcov-dump.
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
    printf("Executing via system(): %s\n", cmd);
    int ret = system(cmd);
    printf("Exit status: %d\n\n", ret);
}

int main(int argc, char *argv[]) {
    printf("=== Testing gcov-dump flag parsing logic ===\n\n");
    
    // Create a dummy .gcda file for file-based tests
    const char *dummy_gcda = "test_dummy.gcda";
    if (create_dummy_gcda(dummy_gcda) != 0) {
        fprintf(stderr, "Warning: Could not create dummy .gcda file\n");
    }
    
    // Test 1: Individual flag cases (lines 111-130)
    printf("--- Test 1: Individual flag cases ---\n");
    
    // Case 'h': Help flag
    const char *help_args[] = {"gcov-dump", "-h", NULL};
    printf("Testing -h (help):\n");
    exec_gcov_dump(help_args);
    printf("\n");
    
    // Case 'v': Version flag
    const char *version_args[] = {"gcov-dump", "-v", NULL};
    printf("Testing -v (version):\n");
    exec_gcov_dump(version_args);
    printf("\n");
    
    // Case 'l': Contents dump flag
    const char *contents_args[] = {"gcov-dump", "-l", dummy_gcda, NULL};
    printf("Testing -l (contents dump):\n");
    exec_gcov_dump(contents_args);
    printf("\n");
    
    // Case 'p': Positions dump flag
    const char *positions_args[] = {"gcov-dump", "-p", dummy_gcda, NULL};
    printf("Testing -p (positions dump):\n");
    exec_gcov_dump(positions_args);
    printf("\n");
    
    // Case 'r': Raw dump flag
    const char *raw_args[] = {"gcov-dump", "-r", dummy_gcda, NULL};
    printf("Testing -r (raw dump):\n");
    exec_gcov_dump(raw_args);
    printf("\n");
    
    // Case 's': Stable dump flag
    const char *stable_args[] = {"gcov-dump", "-s", dummy_gcda, NULL};
    printf("Testing -s (stable dump):\n");
    exec_gcov_dump(stable_args);
    printf("\n");
    
    // Default case: Invalid flag
    const char *invalid_args[] = {"gcov-dump", "-x", NULL};
    printf("Testing -x (invalid flag, should trigger default case):\n");
    exec_gcov_dump(invalid_args);
    printf("\n");
    
    // Test 2: Combination of valid flags
    printf("--- Test 2: Flag combinations ---\n");
    
    // Combination 1: -l -p
    const char *combo1_args[] = {"gcov-dump", "-l", "-p", dummy_gcda, NULL};
    printf("Testing -l -p combination:\n");
    exec_gcov_dump(combo1_args);
    printf("\n");
    
    // Combination 2: -r -s -v
    const char *combo2_args[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    printf("Testing -r -s -v combination:\n");
    exec_gcov_dump(combo2_args);
    printf("\n");
    
    // Combination 3: -h -l (h should cause early exit)
    const char *combo3_args[] = {"gcov-dump", "-h", "-l", NULL};
    printf("Testing -h -l combination (h may cause early exit):\n");
    exec_gcov_dump(combo3_args);
    printf("\n");
    
    // Test 3: Repeated flags
    printf("--- Test 3: Repeated flags ---\n");
    
    const char *repeat_args[] = {"gcov-dump", "-p", "-p", dummy_gcda, NULL};
    printf("Testing -p -p (repeated flag):\n");
    exec_gcov_dump(repeat_args);
    printf("\n");
    
    // Test 4: Different flag syntax styles using system()
    printf("--- Test 4: Different flag syntax (using system()) ---\n");
    
    // Separate arguments
    system_gcov_dump("gcov-dump -l -p test_dummy.gcda 2>&1");
    
    // Combined short options (if supported by getopt)
    system_gcov_dump("gcov-dump -lp test_dummy.gcda 2>&1");
    
    // With -- delimiter
    system_gcov_dump("gcov-dump -l -- test_dummy.gcda 2>&1");
    
    // Test 5: Environment and error contexts
    printf("--- Test 5: Environment and error contexts ---\n");
    
    // No arguments
    printf("Testing with no arguments:\n");
    const char *no_args[] = {"gcov-dump", NULL};
    exec_gcov_dump(no_args);
    printf("\n");
    
    // Set environment variable (if gcov-dump reads it)
    printf("Testing with GCOV_DUMP_OPTIONS environment variable:\n");
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    exec_gcov_dump(no_args);
    unsetenv("GCOV_DUMP_OPTIONS");
    printf("\n");
    
    // Test 6: Invalid flag with stderr redirection
    printf("--- Test 6: Invalid flag with stderr capture ---\n");
    printf("Testing invalid flag with stderr redirection:\n");
    system("gcov-dump -x 2>&1 | grep -q 'unknown flag' && echo 'Default case triggered correctly'");
    printf("\n");
    
    // Test 7: Multiple invalid flags
    printf("--- Test 7: Multiple invalid flags ---\n");
    const char *multi_invalid_args[] = {"gcov-dump", "-x", "-y", "-z", NULL};
    printf("Testing multiple invalid flags:\n");
    exec_gcov_dump(multi_invalid_args);
    printf("\n");
    
    // Test 8: Mixed valid and invalid flags
    printf("--- Test 8: Mixed valid and invalid flags ---\n");
    const char *mixed_args[] = {"gcov-dump", "-l", "-x", "-p", dummy_gcda, NULL};
    printf("Testing mixed valid/invalid flags:\n");
    exec_gcov_dump(mixed_args);
    printf("\n");
    
    // Test 9: Flag with argument (if any flag takes arguments)
    printf("--- Test 9: Flag with potential arguments ---\n");
    const char *with_arg_args[] = {"gcov-dump", "-l", "non_existent.gcda", NULL};
    printf("Testing with non-existent file (error case):\n");
    exec_gcov_dump(with_arg_args);
    printf("\n");
    
    // Cleanup
    remove(dummy_gcda);
    
    printf("=== All tests completed ===\n");
    return 0;
}
