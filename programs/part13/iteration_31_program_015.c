#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

#define TEST_SRC "test.c"
#define TEST_BIN "test"
#define TEST_GCNO "test.gcno"
#define GCOV_DUMP_PATH "./gcov-dump"

// Function to compile test program with coverage
int compile_test_program() {
    printf("Compiling test program with coverage flags...\n");
    
    // Create minimal C source file
    FILE *fp = fopen(TEST_SRC, "w");
    if (!fp) {
        perror("Failed to create test.c");
        return -1;
    }
    fprintf(fp, "int main() { return 0; }\n");
    fclose(fp);
    
    // Compile with coverage flags
    char *args[] = {"gcc", "-O0", "--coverage", "-fprofile-arcs", 
                    "-ftest-coverage", "-o", TEST_BIN, TEST_SRC, NULL};
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execvp("gcc", args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Test program compiled successfully. Generated %s\n", TEST_GCNO);
            return 0;
        } else {
            printf("Failed to compile test program\n");
            return -1;
        }
    } else {
        perror("fork failed");
        return -1;
    }
}

// Function to run gcov-dump with given arguments
int run_gcov_dump(char **args, const char *test_name) {
    printf("\nRunning test: %s\n", test_name);
    printf("Command: ");
    for (int i = 0; args[i] != NULL; i++) {
        printf("%s ", args[i]);
    }
    printf("\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execvp(GCOV_DUMP_PATH, args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Exit code: %d\n", exit_code);
            return exit_code;
        } else if (WIFSIGNALED(status)) {
            printf("Process terminated by signal: %d\n", WTERMSIG(status));
            return -1;
        }
    } else {
        perror("fork failed");
        return -1;
    }
    return -1;
}

int main() {
    int passed = 0;
    int total = 0;
    
    // Check if gcov-dump exists
    if (access(GCOV_DUMP_PATH, X_OK) != 0) {
        fprintf(stderr, "Error: %s not found or not executable\n", GCOV_DUMP_PATH);
        fprintf(stderr, "Please ensure gcov-dump is in the current directory\n");
        return EXIT_FAILURE;
    }
    
    // Compile test program to generate .gcno file
    if (compile_test_program() != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return EXIT_FAILURE;
    }
    
    // Test 1: -h flag (help)
    {
        char *args[] = {GCOV_DUMP_PATH, "-h", NULL};
        int result = run_gcov_dump(args, "Help flag (-h)");
        if (result == 0) {
            printf("✓ PASS: -h flag\n");
            passed++;
        } else {
            printf("✗ FAIL: -h flag (expected 0, got %d)\n", result);
        }
        total++;
    }
    
    // Test 2: -v flag (version)
    {
        char *args[] = {GCOV_DUMP_PATH, "-v", NULL};
        int result = run_gcov_dump(args, "Version flag (-v)");
        if (result == 0) {
            printf("✓ PASS: -v flag\n");
            passed++;
        } else {
            printf("✗ FAIL: -v flag (expected 0, got %d)\n", result);
        }
        total++;
    }
    
    // Test 3: -l flag (dump contents)
    {
        char *args[] = {GCOV_DUMP_PATH, "-l", TEST_GCNO, NULL};
        int result = run_gcov_dump(args, "Dump contents flag (-l)");
        if (result == 0) {
            printf("✓ PASS: -l flag\n");
            passed++;
        } else {
            printf("✗ FAIL: -l flag (expected 0, got %d)\n", result);
        }
        total++;
    }
    
    // Test 4: -p flag (dump positions)
    {
        char *args[] = {GCOV_DUMP_PATH, "-p", TEST_GCNO, NULL};
        int result = run_gcov_dump(args, "Dump positions flag (-p)");
        if (result == 0) {
            printf("✓ PASS: -p flag\n");
            passed++;
        } else {
            printf("✗ FAIL: -p flag (expected 0, got %d)\n", result);
        }
        total++;
    }
    
    // Test 5: -r flag (dump raw)
    {
        char *args[] = {GCOV_DUMP_PATH, "-r", TEST_GCNO, NULL};
        int result = run_gcov_dump(args, "Dump raw flag (-r)");
        if (result == 0) {
            printf("✓ PASS: -r flag\n");
            passed++;
        } else {
            printf("✗ FAIL: -r flag (expected 0, got %d)\n", result);
        }
        total++;
    }
    
    // Test 6: -s flag (dump stable)
    {
        char *args[] = {GCOV_DUMP_PATH, "-s", TEST_GCNO, NULL};
        int result = run_gcov_dump(args, "Dump stable flag (-s)");
        if (result == 0) {
            printf("✓ PASS: -s flag\n");
            passed++;
        } else {
            printf("✗ FAIL: -s flag (expected 0, got %d)\n", result);
        }
        total++;
    }
    
    // Test 7: Invalid flag (should trigger default case)
    {
        char *args[] = {GCOV_DUMP_PATH, "-x", NULL};
        int result = run_gcov_dump(args, "Invalid flag (-x)");
        if (result != 0) {
            printf("✓ PASS: Invalid flag returned non-zero exit code: %d\n", result);
            passed++;
        } else {
            printf("✗ FAIL: Invalid flag should return non-zero exit code\n");
        }
        total++;
    }
    
    // Test 8: Another invalid flag
    {
        char *args[] = {GCOV_DUMP_PATH, "-Z", NULL};
        int result = run_gcov_dump(args, "Invalid flag (-Z)");
        if (result != 0) {
            printf("✓ PASS: Invalid flag -Z returned non-zero exit code: %d\n", result);
            passed++;
        } else {
            printf("✗ FAIL: Invalid flag -Z should return non-zero exit code\n");
        }
        total++;
    }
    
    // Test 9: Yet another invalid flag
    {
        char *args[] = {GCOV_DUMP_PATH, "-?", NULL};
        int result = run_gcov_dump(args, "Invalid flag (-?)");
        if (result != 0) {
            printf("✓ PASS: Invalid flag -? returned non-zero exit code: %d\n", result);
            passed++;
        } else {
            printf("✗ FAIL: Invalid flag -? should return non-zero exit code\n");
        }
        total++;
    }
    
    // Cleanup
    printf("\nCleaning up...\n");
    unlink(TEST_SRC);
    unlink(TEST_BIN);
    // Note: Keeping test.gcno for inspection if needed
    
    // Summary
    printf("\n=== TEST SUMMARY ===\n");
    printf("Passed: %d/%d\n", passed, total);
    printf("Failed: %d/%d\n", total - passed, total);
    
    return (passed == total) ? EXIT_SUCCESS : EXIT_FAILURE;
}
