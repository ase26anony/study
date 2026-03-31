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

void run_gcov_dump(const char **args, const char *description, int expect_success) {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        execvp(GCOV_DUMP_PATH, (char *const *)args);
        // If execvp returns, it failed
        fprintf(stderr, "Failed to execute %s: %s\n", GCOV_DUMP_PATH, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            int passed = (expect_success && exit_code == 0) || 
                        (!expect_success && exit_code != 0);
            
            printf("%-30s: %s (exit code: %d)\n", 
                   description, 
                   passed ? "PASS" : "FAIL", 
                   exit_code);
            
            if (!passed) {
                printf("  Expected %s, got exit code %d\n",
                       expect_success ? "success (0)" : "failure (non-zero)",
                       exit_code);
            }
        } else {
            printf("%-30s: FAIL (did not exit normally)\n", description);
        }
    } else {
        fprintf(stderr, "fork() failed: %s\n", strerror(errno));
    }
}

void create_test_source() {
    FILE *fp = fopen(TEST_SOURCE, "w");
    if (!fp) {
        perror("Failed to create test source file");
        exit(EXIT_FAILURE);
    }
    
    fprintf(fp, "int main() { return 0; }\n");
    fclose(fp);
}

void compile_with_coverage() {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Compile with coverage flags to generate .gcno file
        const char *args[] = {"gcc", "-O0", "--coverage", "-fprofile-arcs", 
                             "-ftest-coverage", "-o", TEST_BINARY, TEST_SOURCE, NULL};
        execvp("gcc", (char *const *)args);
        fprintf(stderr, "Failed to compile test program: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Failed to compile test program\n");
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "fork() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int main() {
    printf("=== GCOV-DUMP Test Harness ===\n\n");
    
    // Check if gcov-dump exists
    if (access(GCOV_DUMP_PATH, X_OK) != 0) {
        fprintf(stderr, "Error: %s not found or not executable\n", GCOV_DUMP_PATH);
        fprintf(stderr, "Please ensure gcov-dump is in the current directory\n");
        return EXIT_FAILURE;
    }
    
    // Create test source and compile with coverage
    printf("Creating test source file...\n");
    create_test_source();
    
    printf("Compiling with coverage flags...\n");
    compile_with_coverage();
    
    printf("\nRunning gcov-dump tests:\n");
    printf("========================\n");
    
    // Test 1: -h (help)
    {
        const char *args[] = {GCOV_DUMP_PATH, "-h", NULL};
        run_gcov_dump(args, "Test -h (help)", 1);
    }
    
    // Test 2: -v (version)
    {
        const char *args[] = {GCOV_DUMP_PATH, "-v", NULL};
        run_gcov_dump(args, "Test -v (version)", 1);
    }
    
    // Test 3: -l (dump contents)
    {
        const char *args[] = {GCOV_DUMP_PATH, "-l", TEST_GCNO, NULL};
        run_gcov_dump(args, "Test -l (dump contents)", 1);
    }
    
    // Test 4: -p (dump positions)
    {
        const char *args[] = {GCOV_DUMP_PATH, "-p", TEST_GCNO, NULL};
        run_gcov_dump(args, "Test -p (dump positions)", 1);
    }
    
    // Test 5: -r (dump raw)
    {
        const char *args[] = {GCOV_DUMP_PATH, "-r", TEST_GCNO, NULL};
        run_gcov_dump(args, "Test -r (dump raw)", 1);
    }
    
    // Test 6: -s (dump stable)
    {
        const char *args[] = {GCOV_DUMP_PATH, "-s", TEST_GCNO, NULL};
        run_gcov_dump(args, "Test -s (dump stable)", 1);
    }
    
    // Test 7: -x (unknown flag) - test multiple unknown flags
    {
        const char *unknown_flags[] = {"-x", "-?", "-Z"};
        for (int i = 0; i < 3; i++) {
            char description[50];
            snprintf(description, sizeof(description), "Test %s (unknown flag)", unknown_flags[i]);
            
            const char *args[] = {GCOV_DUMP_PATH, unknown_flags[i], NULL};
            run_gcov_dump(args, description, 0);
        }
    }
    
    // Cleanup
    printf("\nCleaning up...\n");
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    unlink(TEST_GCNO);
    
    printf("\n=== Test Complete ===\n");
    return EXIT_SUCCESS;
}
