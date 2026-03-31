#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEST_SOURCE "test_gcov.c"
#define TEST_EXECUTABLE "test_gcov"
#define GCOV_DUMP_EXECUTABLE "./gcov-dump"

typedef struct {
    const char *name;
    char **argv;
    int argc;
    int expected_exit;
    const char *expected_stderr;
} test_case_t;

void compile_test_program() {
    // Create minimal C source file
    FILE *fp = fopen(TEST_SOURCE, "w");
    if (!fp) {
        perror("Failed to create test source file");
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "int main() { return 0; }\n");
    fclose(fp);
    
    // Compile with coverage flags to generate .gcno file
    char compile_cmd[256];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -O0 --coverage -fprofile-arcs -ftest-coverage -o %s %s",
             TEST_EXECUTABLE, TEST_SOURCE);
    
    printf("Compiling test program: %s\n", compile_cmd);
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        exit(EXIT_FAILURE);
    }
    
    // Verify .gcno file was created
    struct stat st;
    if (stat(TEST_EXECUTABLE ".gcno", &st) != 0) {
        fprintf(stderr, "Failed to generate .gcno file\n");
        exit(EXIT_FAILURE);
    }
    printf("Generated %s.gcno\n", TEST_EXECUTABLE);
}

int run_test(const char *name, char *const argv[], int expected_exit, 
             const char *expected_stderr) {
    printf("\n=== Testing: %s ===\n", name);
    printf("Command: ");
    for (int i = 0; argv[i] != NULL; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 0;
    }
    
    if (pid == 0) {
        // Child process
        execvp(argv[0], argv);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Exit code: %d (expected: %d)\n", exit_code, expected_exit);
            
            if (exit_code == expected_exit) {
                printf("✓ PASS\n");
                return 1;
            } else {
                printf("✗ FAIL - Wrong exit code\n");
                return 0;
            }
        } else {
            printf("✗ FAIL - Process didn't exit normally\n");
            return 0;
        }
    }
}

int main() {
    // Check if gcov-dump exists
    struct stat st;
    if (stat(GCOV_DUMP_EXECUTABLE, &st) != 0) {
        fprintf(stderr, "Error: %s not found\n", GCOV_DUMP_EXECUTABLE);
        fprintf(stderr, "Please ensure gcov-dump is in the current directory\n");
        return EXIT_FAILURE;
    }
    
    // Generate test .gcno file
    compile_test_program();
    
    int passed = 0;
    int total = 0;
    
    // Test cases
    char *test_h[] = {GCOV_DUMP_EXECUTABLE, "-h", NULL};
    passed += run_test("Help flag (-h)", test_h, 0, NULL);
    total++;
    
    char *test_v[] = {GCOV_DUMP_EXECUTABLE, "-v", NULL};
    passed += run_test("Version flag (-v)", test_v, 0, NULL);
    total++;
    
    char *test_l[] = {GCOV_DUMP_EXECUTABLE, "-l", TEST_EXECUTABLE ".gcno", NULL};
    passed += run_test("Dump contents flag (-l)", test_l, 0, NULL);
    total++;
    
    char *test_p[] = {GCOV_DUMP_EXECUTABLE, "-p", TEST_EXECUTABLE ".gcno", NULL};
    passed += run_test("Dump positions flag (-p)", test_p, 0, NULL);
    total++;
    
    char *test_r[] = {GCOV_DUMP_EXECUTABLE, "-r", TEST_EXECUTABLE ".gcno", NULL};
    passed += run_test("Dump raw flag (-r)", test_r, 0, NULL);
    total++;
    
    char *test_s[] = {GCOV_DUMP_EXECUTABLE, "-s", TEST_EXECUTABLE ".gcno", NULL};
    passed += run_test("Dump stable flag (-s)", test_s, 0, NULL);
    total++;
    
    // Test unknown flag - should trigger default case
    char *test_x[] = {GCOV_DUMP_EXECUTABLE, "-x", NULL};
    passed += run_test("Unknown flag (-x)", test_x, 1, "unknown flag");
    total++;
    
    // Test another unknown flag
    char *test_z[] = {GCOV_DUMP_EXECUTABLE, "-Z", NULL};
    passed += run_test("Unknown flag (-Z)", test_z, 1, "unknown flag");
    total++;
    
    // Test invalid flag
    char *test_q[] = {GCOV_DUMP_EXECUTABLE, "-?", NULL};
    passed += run_test("Unknown flag (-?)", test_q, 1, "unknown flag");
    total++;
    
    // Summary
    printf("\n=== Summary ===\n");
    printf("Passed: %d/%d\n", passed, total);
    
    // Cleanup
    unlink(TEST_SOURCE);
    unlink(TEST_EXECUTABLE);
    unlink(TEST_EXECUTABLE ".gcno");
    unlink(TEST_EXECUTABLE ".gcda");
    
    return (passed == total) ? EXIT_SUCCESS : EXIT_FAILURE;
}
