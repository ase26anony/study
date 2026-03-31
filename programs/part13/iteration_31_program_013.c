#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define TEST_SOURCE "test.c"
#define TEST_BINARY "test"
#define TEST_GCNO "test.gcno"

// Function to compile test file with coverage
int compile_test_file() {
    printf("Compiling test file with coverage flags...\n");
    
    // Create minimal C source file
    FILE *fp = fopen(TEST_SOURCE, "w");
    if (!fp) {
        perror("Failed to create test.c");
        return 0;
    }
    fprintf(fp, "int main() { return 0; }\n");
    fclose(fp);
    
    // Compile with coverage flags
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        char *args[] = {
            "gcc",
            "-O0",
            "--coverage",
            "-fprofile-arcs",
            "-ftest-coverage",
            "-o", TEST_BINARY,
            TEST_SOURCE,
            NULL
        };
        execvp("gcc", args);
        perror("execvp failed");
        exit(1);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Successfully compiled %s (generated %s)\n", TEST_SOURCE, TEST_GCNO);
            return 1;
        } else {
            printf("Failed to compile test file\n");
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

// Function to run gcov-dump with specific arguments
int run_gcov_dump(const char *gcov_dump_path, char *const args[], int expect_success) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execvp(gcov_dump_path, args);
        perror("execvp failed");
        exit(1);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (expect_success) {
                return (exit_code == 0) ? 1 : 0;
            } else {
                return (exit_code != 0) ? 1 : 0;
            }
        }
        return 0;
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
    
    printf("Using gcov-dump at: %s\n", gcov_dump_path);
    
    // First, compile test file to generate .gcno
    if (!compile_test_file()) {
        fprintf(stderr, "Failed to compile test file. Exiting.\n");
        return 1;
    }
    
    int passed = 0;
    int total = 0;
    
    // Test 1: -h flag (help)
    printf("\n=== Test 1: -h flag (help) ===\n");
    char *args1[] = { "gcov-dump", "-h", NULL };
    if (run_gcov_dump(gcov_dump_path, args1, 1)) {
        printf("PASS: -h flag executed successfully\n");
        passed++;
    } else {
        printf("FAIL: -h flag failed\n");
    }
    total++;
    
    // Test 2: -v flag (version)
    printf("\n=== Test 2: -v flag (version) ===\n");
    char *args2[] = { "gcov-dump", "-v", NULL };
    if (run_gcov_dump(gcov_dump_path, args2, 1)) {
        printf("PASS: -v flag executed successfully\n");
        passed++;
    } else {
        printf("FAIL: -v flag failed\n");
    }
    total++;
    
    // Test 3: -l flag (dump contents)
    printf("\n=== Test 3: -l flag (dump contents) ===\n");
    char *args3[] = { "gcov-dump", "-l", TEST_GCNO, NULL };
    if (run_gcov_dump(gcov_dump_path, args3, 1)) {
        printf("PASS: -l flag executed successfully\n");
        passed++;
    } else {
        printf("FAIL: -l flag failed\n");
    }
    total++;
    
    // Test 4: -p flag (dump positions)
    printf("\n=== Test 4: -p flag (dump positions) ===\n");
    char *args4[] = { "gcov-dump", "-p", TEST_GCNO, NULL };
    if (run_gcov_dump(gcov_dump_path, args4, 1)) {
        printf("PASS: -p flag executed successfully\n");
        passed++;
    } else {
        printf("FAIL: -p flag failed\n");
    }
    total++;
    
    // Test 5: -r flag (dump raw)
    printf("\n=== Test 5: -r flag (dump raw) ===\n");
    char *args5[] = { "gcov-dump", "-r", TEST_GCNO, NULL };
    if (run_gcov_dump(gcov_dump_path, args5, 1)) {
        printf("PASS: -r flag executed successfully\n");
        passed++;
    } else {
        printf("FAIL: -r flag failed\n");
    }
    total++;
    
    // Test 6: -s flag (dump stable)
    printf("\n=== Test 6: -s flag (dump stable) ===\n");
    char *args6[] = { "gcov-dump", "-s", TEST_GCNO, NULL };
    if (run_gcov_dump(gcov_dump_path, args6, 1)) {
        printf("PASS: -s flag executed successfully\n");
        passed++;
    } else {
        printf("FAIL: -s flag failed\n");
    }
    total++;
    
    // Test 7: Invalid flag (should trigger default case)
    printf("\n=== Test 7: Invalid flag (default case) ===\n");
    char *args7[] = { "gcov-dump", "-x", NULL };
    if (run_gcov_dump(gcov_dump_path, args7, 0)) {
        printf("PASS: Invalid flag correctly rejected\n");
        passed++;
    } else {
        printf("FAIL: Invalid flag not rejected\n");
    }
    total++;
    
    // Test 8: Another invalid flag
    printf("\n=== Test 8: Another invalid flag ===\n");
    char *args8[] = { "gcov-dump", "-Z", NULL };
    if (run_gcov_dump(gcov_dump_path, args8, 0)) {
        printf("PASS: Invalid flag -Z correctly rejected\n");
        passed++;
    } else {
        printf("FAIL: Invalid flag -Z not rejected\n");
    }
    total++;
    
    // Test 9: Yet another invalid flag
    printf("\n=== Test 9: Invalid flag '?' ===\n");
    char *args9[] = { "gcov-dump", "-?", NULL };
    if (run_gcov_dump(gcov_dump_path, args9, 0)) {
        printf("PASS: Invalid flag -? correctly rejected\n");
        passed++;
    } else {
        printf("FAIL: Invalid flag -? not rejected\n");
    }
    total++;
    
    // Summary
    printf("\n=== Summary ===\n");
    printf("Passed: %d/%d tests\n", passed, total);
    
    // Cleanup
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    // Note: test.gcno is kept for inspection if needed
    
    return (passed == total) ? 0 : 1;
}
