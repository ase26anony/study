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

int compile_test_file(void) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: compile test file with coverage
        execlp("gcc", "gcc", "-O0", "--coverage", "-fprofile-arcs", 
               "-ftest-coverage", "-o", TEST_BINARY, TEST_SOURCE, NULL);
        perror("execlp gcc failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Failed to compile test file\n");
            return -1;
        }
        return 0;
    } else {
        perror("fork failed");
        return -1;
    }
}

int run_gcov_dump(const char **args, int expected_exit, const char *test_name) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: run gcov-dump
        execvp(GCOV_DUMP_PATH, (char *const *)args);
        perror("execvp gcov-dump failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == expected_exit) {
                printf("✓ %s: PASSED (exit code %d)\n", test_name, exit_code);
                return 0;
            } else {
                printf("✗ %s: FAILED - expected exit code %d, got %d\n", 
                       test_name, expected_exit, exit_code);
                return -1;
            }
        } else {
            printf("✗ %s: FAILED - abnormal termination\n", test_name);
            return -1;
        }
    } else {
        perror("fork failed");
        return -1;
    }
}

int create_test_source(void) {
    FILE *fp = fopen(TEST_SOURCE, "w");
    if (!fp) {
        perror("Failed to create test.c");
        return -1;
    }
    
    fprintf(fp, "int main() { return 0; }\n");
    fclose(fp);
    return 0;
}

int main(void) {
    printf("=== gcov-dump Test Harness ===\n\n");
    
    // Create minimal test source file
    if (create_test_source() != 0) {
        return EXIT_FAILURE;
    }
    
    // Compile test file to generate .gcno
    printf("Compiling test file to generate .gcno...\n");
    if (compile_test_file() != 0) {
        return EXIT_FAILURE;
    }
    
    int passed = 0;
    int total = 0;
    
    // Test cases
    const char *test_cases[][10] = {
        // Test 1: -h (help)
        {GCOV_DUMP_PATH, "-h", NULL},
        // Test 2: -v (version)
        {GCOV_DUMP_PATH, "-v", NULL},
        // Test 3: -l (dump contents)
        {GCOV_DUMP_PATH, "-l", TEST_GCNO, NULL},
        // Test 4: -p (dump positions)
        {GCOV_DUMP_PATH, "-p", TEST_GCNO, NULL},
        // Test 5: -r (dump raw)
        {GCOV_DUMP_PATH, "-r", TEST_GCNO, NULL},
        // Test 6: -s (dump stable)
        {GCOV_DUMP_PATH, "-s", TEST_GCNO, NULL},
        // Test 7: -x (unknown flag)
        {GCOV_DUMP_PATH, "-x", NULL}
    };
    
    const char *test_names[] = {
        "Help flag (-h)",
        "Version flag (-v)",
        "Dump contents flag (-l)",
        "Dump positions flag (-p)",
        "Dump raw flag (-r)",
        "Dump stable flag (-s)",
        "Unknown flag (-x)"
    };
    
    int expected_exits[] = {0, 0, 0, 0, 0, 0, 1}; // -x should exit with error
    
    // Run all test cases
    for (int i = 0; i < 7; i++) {
        printf("\nTest %d: %s\n", i + 1, test_names[i]);
        if (run_gcov_dump(test_cases[i], expected_exits[i], test_names[i]) == 0) {
            passed++;
        }
        total++;
        
        // Small delay to avoid potential race conditions
        sleep(1);
    }
    
    // Cleanup
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    unlink(TEST_GCNO);
    unlink("test.gcda"); // In case it was created
    
    printf("\n=== Summary ===\n");
    printf("Passed: %d/%d tests\n", passed, total);
    
    return (passed == total) ? EXIT_SUCCESS : EXIT_FAILURE;
}
