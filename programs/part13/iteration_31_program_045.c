#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

#define TEST_SOURCE "test.c"
#define TEST_BINARY "test"
#define TEST_GCNO "test.gcno"
#define GCOV_DUMP_PATH "./gcov-dump"

// Function to compile test program with coverage
int compile_test_program() {
    printf("Compiling test program with coverage flags...\n");
    
    // Create minimal C source file
    FILE *fp = fopen(TEST_SOURCE, "w");
    if (!fp) {
        perror("Failed to create test.c");
        return -1;
    }
    fprintf(fp, "int main() { return 0; }\n");
    fclose(fp);
    
    // Compile with coverage flags
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execlp("gcc", "gcc", "-O0", "--coverage", "-fprofile-arcs", 
               "-ftest-coverage", "-o", TEST_BINARY, TEST_SOURCE, NULL);
        perror("exec gcc failed");
        exit(1);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Failed to compile test program\n");
            return -1;
        }
        printf("Test program compiled successfully, %s generated\n", TEST_GCNO);
        return 0;
    } else {
        perror("fork failed");
        return -1;
    }
}

// Function to run gcov-dump with given arguments
int run_gcov_dump(const char *description, char *const args[], int expect_success) {
    printf("\n=== Testing: %s ===\n", description);
    printf("Command: gcov-dump");
    for (int i = 0; args[i] != NULL; i++) {
        printf(" %s", args[i]);
    }
    printf("\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execvp(GCOV_DUMP_PATH, args);
        // If execvp returns, it failed
        fprintf(stderr, "Failed to execute %s: %s\n", GCOV_DUMP_PATH, strerror(errno));
        exit(1);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Exit code: %d\n", exit_code);
            
            if (expect_success) {
                if (exit_code == 0) {
                    printf("✓ PASS: Expected success (0), got %d\n", exit_code);
                    return 1;
                } else {
                    printf("✗ FAIL: Expected success (0), got %d\n", exit_code);
                    return 0;
                }
            } else {
                if (exit_code != 0) {
                    printf("✓ PASS: Expected failure (non-zero), got %d\n", exit_code);
                    return 1;
                } else {
                    printf("✗ FAIL: Expected failure (non-zero), got %d\n", exit_code);
                    return 0;
                }
            }
        } else {
            printf("✗ FAIL: Process did not exit normally\n");
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

int main() {
    printf("=== GCOV-DUMP Test Harness ===\n");
    
    // Check if gcov-dump exists
    if (access(GCOV_DUMP_PATH, X_OK) != 0) {
        fprintf(stderr, "Error: %s not found or not executable\n", GCOV_DUMP_PATH);
        fprintf(stderr, "Please ensure gcov-dump is in the current directory\n");
        return 1;
    }
    
    // Compile test program to generate .gcno file
    if (compile_test_program() != 0) {
        fprintf(stderr, "Failed to compile test program, exiting\n");
        return 1;
    }
    
    int passed = 0;
    int total = 0;
    
    // Test cases
    char *args[10];
    
    // Test 1: -h (help)
    args[0] = "gcov-dump";
    args[1] = "-h";
    args[2] = NULL;
    passed += run_gcov_dump("Help flag (-h)", args, 1);
    total++;
    
    // Test 2: -v (version)
    args[1] = "-v";
    args[2] = NULL;
    passed += run_gcov_dump("Version flag (-v)", args, 1);
    total++;
    
    // Test 3: -l (dump contents)
    args[1] = "-l";
    args[2] = TEST_GCNO;
    args[3] = NULL;
    passed += run_gcov_dump("Dump contents flag (-l)", args, 1);
    total++;
    
    // Test 4: -p (dump positions)
    args[1] = "-p";
    args[2] = TEST_GCNO;
    args[3] = NULL;
    passed += run_gcov_dump("Dump positions flag (-p)", args, 1);
    total++;
    
    // Test 5: -r (dump raw)
    args[1] = "-r";
    args[2] = TEST_GCNO;
    args[3] = NULL;
    passed += run_gcov_dump("Dump raw flag (-r)", args, 1);
    total++;
    
    // Test 6: -s (dump stable)
    args[1] = "-s";
    args[2] = TEST_GCNO;
    args[3] = NULL;
    passed += run_gcov_dump("Dump stable flag (-s)", args, 1);
    total++;
    
    // Test 7: -x (unknown flag - triggers default case)
    args[1] = "-x";
    args[2] = NULL;
    passed += run_gcov_dump("Unknown flag (-x)", args, 0);
    total++;
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d\n", passed, total);
    
    // Cleanup
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    // Note: test.gcno is kept for inspection if needed
    
    return (passed == total) ? 0 : 1;
}
