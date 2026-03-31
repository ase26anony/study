#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define TEST_SOURCE "test.c"
#define TEST_EXECUTABLE "test"
#define TEST_GCNO "test.gcno"

// Function to compile test program with coverage
int compile_test_program() {
    printf("Compiling test program for coverage data...\n");
    
    // Create minimal C source file
    FILE *fp = fopen(TEST_SOURCE, "w");
    if (!fp) {
        perror("Failed to create test.c");
        return 0;
    }
    fprintf(fp, "int main() { return 0; }\n");
    fclose(fp);
    
    // Compile with coverage flags
    char cmd[256];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 --coverage -fprofile-arcs -ftest-coverage -o %s %s",
             TEST_EXECUTABLE, TEST_SOURCE);
    
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return 0;
    }
    
    // Verify .gcno file was created
    if (access(TEST_GCNO, F_OK) != 0) {
        fprintf(stderr, "No .gcno file generated\n");
        return 0;
    }
    
    printf("Test program compiled successfully, %s created\n", TEST_GCNO);
    return 1;
}

// Function to run gcov-dump with given arguments
int run_gcov_dump(const char *gcov_dump_path, char *const argv[], 
                  int expect_success, const char *test_name) {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        execvp(gcov_dump_path, argv);
        // If execvp returns, it failed
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            int success = (expect_success && exit_code == 0) || 
                         (!expect_success && exit_code != 0);
            
            printf("Test '%s': %s (exit code: %d)\n", 
                   test_name, 
                   success ? "PASSED" : "FAILED",
                   exit_code);
            return success;
        } else {
            printf("Test '%s': FAILED (child terminated abnormally)\n", test_name);
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

int main(int argc, char *argv[]) {
    const char *gcov_dump_path = NULL;
    
    // Try to get gcov-dump path from environment or use default
    gcov_dump_path = getenv("GCOV_DUMP");
    if (!gcov_dump_path) {
        gcov_dump_path = "gcov-dump";
    }
    
    // Verify gcov-dump exists
    if (access(gcov_dump_path, X_OK) != 0) {
        fprintf(stderr, "gcov-dump not found at '%s'\n", gcov_dump_path);
        fprintf(stderr, "Set GCOV_DUMP environment variable or ensure gcov-dump is in PATH\n");
        return EXIT_FAILURE;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Compile test program to generate .gcno file
    if (!compile_test_program()) {
        fprintf(stderr, "Failed to prepare test coverage data\n");
        return EXIT_FAILURE;
    }
    
    int passed = 0;
    int total = 0;
    
    // Test 1: -h flag (help)
    {
        char *test_args[] = { "gcov-dump", "-h", NULL };
        passed += run_gcov_dump(gcov_dump_path, test_args, 1, "-h (help)");
        total++;
    }
    
    // Test 2: -v flag (version)
    {
        char *test_args[] = { "gcov-dump", "-v", NULL };
        passed += run_gcov_dump(gcov_dump_path, test_args, 1, "-v (version)");
        total++;
    }
    
    // Test 3: -l flag (dump contents)
    {
        char *test_args[] = { "gcov-dump", "-l", TEST_GCNO, NULL };
        passed += run_gcov_dump(gcov_dump_path, test_args, 1, "-l (dump contents)");
        total++;
    }
    
    // Test 4: -p flag (dump positions)
    {
        char *test_args[] = { "gcov-dump", "-p", TEST_GCNO, NULL };
        passed += run_gcov_dump(gcov_dump_path, test_args, 1, "-p (dump positions)");
        total++;
    }
    
    // Test 5: -r flag (dump raw)
    {
        char *test_args[] = { "gcov-dump", "-r", TEST_GCNO, NULL };
        passed += run_gcov_dump(gcov_dump_path, test_args, 1, "-r (dump raw)");
        total++;
    }
    
    // Test 6: -s flag (dump stable)
    {
        char *test_args[] = { "gcov-dump", "-s", TEST_GCNO, NULL };
        passed += run_gcov_dump(gcov_dump_path, test_args, 1, "-s (dump stable)");
        total++;
    }
    
    // Test 7: Invalid flag (should trigger default case)
    {
        char *test_args[] = { "gcov-dump", "-x", NULL };
        passed += run_gcov_dump(gcov_dump_path, test_args, 0, "-x (invalid flag)");
        total++;
    }
    
    // Test 8: Another invalid flag
    {
        char *test_args[] = { "gcov-dump", "-Z", NULL };
        passed += run_gcov_dump(gcov_dump_path, test_args, 0, "-Z (invalid flag)");
        total++;
    }
    
    // Test 9: Yet another invalid flag
    {
        char *test_args[] = { "gcov-dump", "-?", NULL };
        passed += run_gcov_dump(gcov_dump_path, test_args, 0, "-? (invalid flag)");
        total++;
    }
    
    // Cleanup
    unlink(TEST_SOURCE);
    unlink(TEST_EXECUTABLE);
    // Keep TEST_GCNO for inspection if needed
    
    printf("\n=== Summary ===\n");
    printf("Passed: %d/%d tests\n", passed, total);
    
    return (passed == total) ? EXIT_SUCCESS : EXIT_FAILURE;
}
