#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

#define TEST_SRC "test.c"
#define TEST_EXE "test"
#define TEST_GCNO "test.gcno"
#define GCOV_DUMP_PATH "./gcov-dump"

typedef struct {
    const char *name;
    char **argv;
    int expected_exit;
    const char *expected_stderr;
} test_case_t;

void compile_test_gcno(void) {
    // Create minimal test source file
    FILE *fp = fopen(TEST_SRC, "w");
    if (!fp) {
        perror("Failed to create test.c");
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "int main() { return 0; }\n");
    fclose(fp);
    
    // Compile with coverage flags to generate .gcno file
    printf("Compiling test program with coverage flags...\n");
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execlp("gcc", "gcc", "-O0", "--coverage", "-fprofile-arcs", 
               "-ftest-coverage", "-o", TEST_EXE, TEST_SRC, NULL);
        perror("exec gcc failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Failed to compile test program\n");
            exit(EXIT_FAILURE);
        }
        printf("Generated %s successfully\n", TEST_GCNO);
    } else {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
}

void cleanup_files(void) {
    unlink(TEST_SRC);
    unlink(TEST_EXE);
    unlink(TEST_GCNO);
    unlink("test.gcda");
}

int run_test(const char *name, char *const argv[], int expected_exit, 
             const char *expected_stderr) {
    printf("\n=== Testing: %s ===\n", name);
    printf("Command:");
    for (int i = 0; argv[i] != NULL; i++) {
        printf(" %s", argv[i]);
    }
    printf("\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execvp(GCOV_DUMP_PATH, argv);
        // If execvp returns, it failed
        fprintf(stderr, "Failed to execute %s: %s\n", GCOV_DUMP_PATH, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Exit code: %d (expected: %d)\n", exit_code, expected_exit);
            
            if (exit_code == expected_exit) {
                printf("✓ PASS: %s\n", name);
                return 1;
            } else {
                printf("✗ FAIL: %s - Wrong exit code\n", name);
                return 0;
            }
        } else {
            printf("✗ FAIL: %s - Process didn't exit normally\n", name);
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

int main(void) {
    printf("=== GCOV-DUMP Test Harness ===\n");
    
    // Check if gcov-dump exists
    if (access(GCOV_DUMP_PATH, X_OK) != 0) {
        fprintf(stderr, "Error: %s not found or not executable\n", GCOV_DUMP_PATH);
        fprintf(stderr, "Please ensure gcov-dump is in the current directory\n");
        return EXIT_FAILURE;
    }
    
    // Generate test .gcno file
    compile_test_gcno();
    
    int passed = 0;
    int total = 0;
    
    // Test cases
    char *test_h[] = {GCOV_DUMP_PATH, "-h", NULL};
    passed += run_test("Help flag (-h)", test_h, 0, NULL);
    total++;
    
    char *test_v[] = {GCOV_DUMP_PATH, "-v", NULL};
    passed += run_test("Version flag (-v)", test_v, 0, NULL);
    total++;
    
    char *test_l[] = {GCOV_DUMP_PATH, "-l", TEST_GCNO, NULL};
    passed += run_test("Dump contents flag (-l)", test_l, 0, NULL);
    total++;
    
    char *test_p[] = {GCOV_DUMP_PATH, "-p", TEST_GCNO, NULL};
    passed += run_test("Dump positions flag (-p)", test_p, 0, NULL);
    total++;
    
    char *test_r[] = {GCOV_DUMP_PATH, "-r", TEST_GCNO, NULL};
    passed += run_test("Dump raw flag (-r)", test_r, 0, NULL);
    total++;
    
    char *test_s[] = {GCOV_DUMP_PATH, "-s", TEST_GCNO, NULL};
    passed += run_test("Dump stable flag (-s)", test_s, 0, NULL);
    total++;
    
    char *test_x[] = {GCOV_DUMP_PATH, "-x", NULL};
    passed += run_test("Unknown flag (-x)", test_x, 1, "unknown flag");
    total++;
    
    // Test with another unknown flag
    char *test_z[] = {GCOV_DUMP_PATH, "-Z", NULL};
    passed += run_test("Unknown flag (-Z)", test_z, 1, "unknown flag");
    total++;
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d\n", passed, total);
    
    // Cleanup
    cleanup_files();
    
    return (passed == total) ? EXIT_SUCCESS : EXIT_FAILURE;
}
