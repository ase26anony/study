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
    unsigned int magic = 0x67636461; // 'gcda' in little-endian
    unsigned int version = 0x4020000; // GCOV version 4.2
    
    fwrite(&magic, sizeof(magic), 1, fp);
    fwrite(&version, sizeof(version), 1, fp);
    
    // Write a simple tag with zero length
    unsigned int tag = 0; // GCOV_TAG_FUNCTION
    unsigned int length = 0;
    
    fwrite(&tag, sizeof(tag), 1, fp);
    fwrite(&length, sizeof(length), 1, fp);
    
    fclose(fp);
    return 0;
}

/**
 * Compiles a trivial C program with coverage flags and runs it
 * to generate a valid .gcda file.
 */
int generate_valid_gcda(const char *gcda_filename) {
    const char *c_source = "test_gcov.c";
    const char *executable = "test_gcov_prog";
    
    // Create a minimal C source file
    FILE *src = fopen(c_source, "w");
    if (!src) {
        perror("Failed to create test C source");
        return -1;
    }
    fprintf(src, "int main() { return 0; }\n");
    fclose(src);
    
    // Compile with coverage flags
    char compile_cmd[512];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -o %s %s",
             executable, c_source);
    
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        unlink(c_source);
        return -1;
    }
    
    // Run the program to generate .gcda
    if (system(executable) != 0) {
        fprintf(stderr, "Failed to run test program\n");
    }
    
    // Rename the generated .gcda to our desired filename
    if (rename("test_gcov.gcda", gcda_filename) != 0) {
        // Try alternative naming pattern
        char default_gcda[256];
        snprintf(default_gcda, sizeof(default_gcda), "%s.gcda", c_source);
        if (rename(default_gcda, gcda_filename) != 0) {
            fprintf(stderr, "Could not find generated .gcda file\n");
        }
    }
    
    // Cleanup temporary files
    unlink(c_source);
    unlink(executable);
    
    return 0;
}

/**
 * Execute gcov-dump using execvp for precise argument control.
 * Returns the exit status of gcov-dump.
 */
int exec_gcov_dump(char *const args[]) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        // Child process
        execvp("gcov-dump", args);
        // If execvp returns, it failed
        fprintf(stderr, "execvp failed: %s\n", strerror(errno));
        exit(127);
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
 * Execute gcov-dump using system() for shell interpretation testing.
 */
void system_gcov_dump(const char *cmd) {
    printf("\n=== Testing with system(): %s ===\n", cmd);
    int ret = system(cmd);
    printf("Exit status: %d\n", ret);
}

int main() {
    printf("=== Starting gcov-dump flag parser tests ===\n\n");
    
    // Create a dummy .gcda file for file-based tests
    const char *dummy_gcda = "dummy.gcda";
    
    printf("Generating test .gcda file...\n");
    if (generate_valid_gcda(dummy_gcda) != 0) {
        // Fall back to minimal dummy file
        printf("Falling back to minimal .gcda file...\n");
        create_dummy_gcda(dummy_gcda);
    }
    
    // Test cases for individual flags (uncovered switch cases)
    char *individual_tests[][4] = {
        {"gcov-dump", "-h", NULL},                     // Help flag
        {"gcov-dump", "-v", NULL},                     // Version flag
        {"gcov-dump", "-l", NULL},                     // Contents dump flag
        {"gcov-dump", "-p", NULL},                     // Positions dump flag
        {"gcov-dump", "-r", NULL},                     // Raw dump flag
        {"gcov-dump", "-s", NULL},                     // Stable dump flag
        {"gcov-dump", "-x", NULL},                     // Invalid flag (triggers default case)
    };
    
    printf("\n=== Testing individual flags with execvp ===\n");
    for (int i = 0; i < sizeof(individual_tests)/sizeof(individual_tests[0]); i++) {
        printf("\nTest %d: ", i+1);
        for (int j = 0; individual_tests[i][j] != NULL; j++) {
            printf("%s ", individual_tests[i][j]);
        }
        printf("\n");
        
        int status = exec_gcov_dump(individual_tests[i]);
        printf("Exit status: %d\n", status);
    }
    
    // Test combinations of valid flags
    char *combination_tests[][5] = {
        {"gcov-dump", "-l", "-p", NULL},               // Two flags
        {"gcov-dump", "-r", "-s", "-v", NULL},         // Three flags
        {"gcov-dump", "-p", "-p", NULL},               // Repeated flag
        {"gcov-dump", "-l", "-p", "-r", "-s", NULL},   // All dump flags
    };
    
    printf("\n=== Testing flag combinations with execvp ===\n");
    for (int i = 0; i < sizeof(combination_tests)/sizeof(combination_tests[0]); i++) {
        printf("\nCombination test %d: ", i+1);
        for (int j = 0; combination_tests[i][j] != NULL; j++) {
            printf("%s ", combination_tests[i][j]);
        }
        printf("\n");
        
        exec_gcov_dump(combination_tests[i]);
    }
    
    // Test with file arguments
    char *file_tests[][5] = {
        {"gcov-dump", "-l", dummy_gcda, NULL},         // Flag with file
        {"gcov-dump", "-p", "-r", dummy_gcda, NULL},   // Multiple flags with file
        {"gcov-dump", dummy_gcda, NULL},               // File only (no flags)
    };
    
    printf("\n=== Testing with file arguments ===\n");
    for (int i = 0; i < sizeof(file_tests)/sizeof(file_tests[0]); i++) {
        printf("\nFile test %d: ", i+1);
        for (int j = 0; file_tests[i][j] != NULL; j++) {
            printf("%s ", file_tests[i][j]);
        }
        printf("\n");
        
        exec_gcov_dump(file_tests[i]);
    }
    
    // Test different syntactic styles using system()
    printf("\n=== Testing different syntactic styles with system() ===\n");
    
    // Combined short options
    system_gcov_dump("gcov-dump -lp");
    system_gcov_dump("gcov-dump -lps");
    
    // With -- delimiter
    char cmd_with_delim[256];
    snprintf(cmd_with_delim, sizeof(cmd_with_delim), 
             "gcov-dump -l -- %s", dummy_gcda);
    system_gcov_dump(cmd_with_delim);
    
    // No arguments
    system_gcov_dump("gcov-dump");
    
    // Test environment variable influence
    printf("\n=== Testing with environment variables ===\n");
    setenv("GCOV_DUMP_OPTIONS", "-v", 1);
    system_gcov_dump("gcov-dump");
    unsetenv("GCOV_DUMP_OPTIONS");
    
    // Test error stream redirection for invalid flag
    printf("\n=== Testing stderr redirection for invalid flag ===\n");
    system("gcov-dump -x 2>&1 | grep -q 'unknown flag' && echo 'Invalid flag error captured'");
    
    // Test with optimization and fortification simulation
    printf("\n=== Testing parser stress ===\n");
    
    // Multiple invalid flags
    system_gcov_dump("gcov-dump -x -y -z");
    
    // Mixed valid and invalid
    system_gcov_dump("gcov-dump -l -x -p");
    
    // Very long argument list
    system_gcov_dump("gcov-dump -l -p -r -s -v -h 2>/dev/null");
    
    // Cleanup
    unlink(dummy_gcda);
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
