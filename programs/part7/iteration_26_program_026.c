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
int create_dummy_gcda_file(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create dummy .gcda file");
        return -1;
    }
    
    // Write minimal .gcda header (magic + version)
    unsigned int magic = 0x67636461;  // 'gcda' in little-endian
    unsigned int version = 0x4020000; // GCC 8.2.0 version format
    
    fwrite(&magic, sizeof(magic), 1, fp);
    fwrite(&version, sizeof(version), 1, fp);
    
    // Write a simple tag (function tag) with zero length
    unsigned int tag = 0x01000000; // GCOV_TAG_FUNCTION
    unsigned int length = 0;
    
    fwrite(&tag, sizeof(tag), 1, fp);
    fwrite(&length, sizeof(length), 1, fp);
    
    fclose(fp);
    return 0;
}

/**
 * Compiles a trivial C program with coverage flags.
 * Returns 0 on success, -1 on failure.
 */
int compile_test_program(void) {
    const char *source = 
        "int main() {\n"
        "    return 0;\n"
        "}\n";
    
    FILE *fp = fopen("test_coverage.c", "w");
    if (!fp) {
        perror("Failed to create test source file");
        return -1;
    }
    fputs(source, fp);
    fclose(fp);
    
    // Compile with coverage flags
    int result = system("gcc -fprofile-arcs -ftest-coverage -O0 test_coverage.c -o test_coverage");
    if (result != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return -1;
    }
    
    // Run it to generate .gcda file
    result = system("./test_coverage");
    if (result != 0) {
        fprintf(stderr, "Failed to run test program\n");
        return -1;
    }
    
    return 0;
}

/**
 * Executes gcov-dump with given arguments using execvp.
 * Returns exit status of gcov-dump.
 */
int exec_gcov_dump(char *const args[]) {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        execvp("gcov-dump", args);
        // If execvp returns, it failed
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            fprintf(stderr, "gcov-dump terminated abnormally\n");
            return -1;
        }
    } else {
        perror("fork failed");
        return -1;
    }
}

/**
 * Executes gcov-dump using system() call.
 */
void system_gcov_dump(const char *command) {
    printf("Testing with system(): %s\n", command);
    int result = system(command);
    printf("Exit status: %d\n\n", result);
}

int main(void) {
    printf("=== Testing gcov-dump Flag Parser ===\n\n");
    
    // Create test files
    printf("Creating test files...\n");
    if (create_dummy_gcda_file("dummy.gcda") != 0) {
        fprintf(stderr, "Warning: Using placeholder .gcda file\n");
    }
    
    if (compile_test_program() != 0) {
        fprintf(stderr, "Warning: Could not generate proper .gcda file\n");
    }
    
    // Test individual flags (REQUIREMENT 1)
    printf("\n--- Testing Individual Flags ---\n");
    
    // Help flag
    char *help_args[] = {"gcov-dump", "-h", NULL};
    exec_gcov_dump(help_args);
    
    // Version flag
    char *version_args[] = {"gcov-dump", "-v", NULL};
    exec_gcov_dump(version_args);
    
    // Contents dump flag
    char *contents_args[] = {"gcov-dump", "-l", NULL};
    exec_gcov_dump(contents_args);
    
    // Positions dump flag
    char *positions_args[] = {"gcov-dump", "-p", NULL};
    exec_gcov_dump(positions_args);
    
    // Raw dump flag
    char *raw_args[] = {"gcov-dump", "-r", NULL};
    exec_gcov_dump(raw_args);
    
    // Stable dump flag
    char *stable_args[] = {"gcov-dump", "-s", NULL};
    exec_gcov_dump(stable_args);
    
    // Invalid flag (to trigger default case)
    char *invalid_args[] = {"gcov-dump", "-x", NULL};
    exec_gcov_dump(invalid_args);
    
    // Test flag combinations (REQUIREMENT 2)
    printf("\n--- Testing Flag Combinations ---\n");
    
    // Two flags
    char *combo1[] = {"gcov-dump", "-l", "-p", NULL};
    exec_gcov_dump(combo1);
    
    // Three flags
    char *combo2[] = {"gcov-dump", "-r", "-s", "-v", NULL};
    exec_gcov_dump(combo2);
    
    // Help with other flags (may exit early)
    char *combo3[] = {"gcov-dump", "-h", "-l", NULL};
    exec_gcov_dump(combo3);
    
    // Repeated flag
    char *combo4[] = {"gcov-dump", "-p", "-p", NULL};
    exec_gcov_dump(combo4);
    
    // Multiple dump flags together
    char *combo5[] = {"gcov-dump", "-l", "-p", "-r", "-s", NULL};
    exec_gcov_dump(combo5);
    
    // Test different flag syntaxes (REQUIREMENT 3)
    printf("\n--- Testing Different Flag Syntaxes ---\n");
    
    // Combined short options (if supported)
    char *combined[] = {"gcov-dump", "-lp", NULL};
    exec_gcov_dump(combined);
    
    // With positional arguments
    char *with_file1[] = {"gcov-dump", "-l", "test_coverage.gcda", NULL};
    exec_gcov_dump(with_file1);
    
    char *with_file2[] = {"gcov-dump", "-l", "dummy.gcda", NULL};
    exec_gcov_dump(with_file2);
    
    // With -- delimiter
    char *with_delim[] = {"gcov-dump", "-l", "--", "dummy.gcda", NULL};
    exec_gcov_dump(with_delim);
    
    // Multiple files
    char *multi_files[] = {"gcov-dump", "-p", "dummy.gcda", "test_coverage.gcda", NULL};
    exec_gcov_dump(multi_files);
    
    // Test environment and error contexts (REQUIREMENT 4)
    printf("\n--- Testing Environment and Error Contexts ---\n");
    
    // No arguments
    char *no_args[] = {"gcov-dump", NULL};
    exec_gcov_dump(no_args);
    
    // Set environment variable (if supported)
    printf("Setting GCOV_DUMP_OPTIONS environment variable...\n");
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    exec_gcov_dump(version_args);
    unsetenv("GCOV_DUMP_OPTIONS");
    
    // Test with system() calls for different path (REQUIREMENT 4)
    printf("\n--- Testing with system() calls ---\n");
    
    // Redirect stderr to capture "unknown flag" message
    system_gcov_dump("gcov-dump -x 2>&1");
    
    // Test with output redirection
    system_gcov_dump("gcov-dump -v > version_output.txt 2>&1");
    system_gcov_dump("gcov-dump -x 2> error_output.txt");
    
    // Test invalid flag with system
    system_gcov_dump("gcov-dump -z 2>&1");
    
    // Test empty command line
    system_gcov_dump("gcov-dump 2>&1");
    
    // Test with non-existent file
    system_gcov_dump("gcov-dump -l non_existent.gcda 2>&1");
    
    // Test complex combinations with system
    system_gcov_dump("gcov-dump -l -p -r test_coverage.gcda 2>&1");
    system_gcov_dump("gcov-dump -s -- dummy.gcda 2>&1");
    
    // Cleanup
    printf("\n=== Cleaning up test files ===\n");
    remove("dummy.gcda");
    remove("test_coverage.c");
    remove("test_coverage");
    remove("test_coverage.gcda");
    remove("test_coverage.gcno");
    remove("version_output.txt");
    remove("error_output.txt");
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
