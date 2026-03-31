#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

#define TEST_SOURCE "test_coverage.c"
#define TEST_EXECUTABLE "test_coverage"
#define GCOV_DUMP_EXECUTABLE "./gcov-dump"

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
    int ret = system(compile_cmd);
    if (ret != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        exit(EXIT_FAILURE);
    }
    
    // Verify .gcno file was created
    struct stat st;
    if (stat("test_coverage.gcno", &st) != 0) {
        fprintf(stderr, "No .gcno file generated\n");
        exit(EXIT_FAILURE);
    }
    printf("Generated test_coverage.gcno (%ld bytes)\n", (long)st.st_size);
}

int run_gcov_dump(const char *args[], int expected_exit, const char *test_name) {
    printf("\n=== Testing: %s ===\n", test_name);
    printf("Command: gcov-dump");
    for (int i = 0; args[i] != NULL; i++) {
        printf(" %s", args[i]);
    }
    printf("\n");
    
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return 0;
    }
    
    if (pid == 0) {
        // Child process
        execvp(GCOV_DUMP_EXECUTABLE, (char *const *)args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == expected_exit) {
                printf("✓ PASS: Exit code %d as expected\n", exit_code);
                return 1;
            } else {
                printf("✗ FAIL: Expected exit code %d, got %d\n", 
                       expected_exit, exit_code);
                return 0;
            }
        } else {
            printf("✗ FAIL: Process did not exit normally\n");
            return 0;
        }
    }
}

int main() {
    int total_tests = 0;
    int passed_tests = 0;
    
    // Check if gcov-dump exists
    struct stat st;
    if (stat(GCOV_DUMP_EXECUTABLE, &st) != 0) {
        fprintf(stderr, "Error: %s not found\n", GCOV_DUMP_EXECUTABLE);
        fprintf(stderr, "Please build gcov-dump first or adjust the path\n");
        return EXIT_FAILURE;
    }
    
    // Generate test .gcno file
    compile_test_program();
    
    // Test cases
    const char *help_args[] = {GCOV_DUMP_EXECUTABLE, "-h", NULL};
    passed_tests += run_gcov_dump(help_args, 0, "Help flag (-h)");
    total_tests++;
    
    const char *version_args[] = {GCOV_DUMP_EXECUTABLE, "-v", NULL};
    passed_tests += run_gcov_dump(version_args, 0, "Version flag (-v)");
    total_tests++;
    
    const char *dump_contents_args[] = {GCOV_DUMP_EXECUTABLE, "-l", "test_coverage.gcno", NULL};
    passed_tests += run_gcov_dump(dump_contents_args, 0, "Dump contents flag (-l)");
    total_tests++;
    
    const char *dump_positions_args[] = {GCOV_DUMP_EXECUTABLE, "-p", "test_coverage.gcno", NULL};
    passed_tests += run_gcov_dump(dump_positions_args, 0, "Dump positions flag (-p)");
    total_tests++;
    
    const char *dump_raw_args[] = {GCOV_DUMP_EXECUTABLE, "-r", "test_coverage.gcno", NULL};
    passed_tests += run_gcov_dump(dump_raw_args, 0, "Dump raw flag (-r)");
    total_tests++;
    
    const char *dump_stable_args[] = {GCOV_DUMP_EXECUTABLE, "-s", "test_coverage.gcno", NULL};
    passed_tests += run_gcov_dump(dump_stable_args, 0, "Dump stable flag (-s)");
    total_tests++;
    
    const char *unknown_flag_args[] = {GCOV_DUMP_EXECUTABLE, "-x", NULL};
    passed_tests += run_gcov_dump(unknown_flag_args, 1, "Unknown flag (-x)");
    total_tests++;
    
    // Also test other unknown flags
    const char *unknown_flag2_args[] = {GCOV_DUMP_EXECUTABLE, "-?", NULL};
    passed_tests += run_gcov_dump(unknown_flag2_args, 1, "Unknown flag (-?)");
    total_tests++;
    
    const char *unknown_flag3_args[] = {GCOV_DUMP_EXECUTABLE, "-Z", NULL};
    passed_tests += run_gcov_dump(unknown_flag3_args, 1, "Unknown flag (-Z)");
    total_tests++;
    
    // Cleanup
    unlink(TEST_SOURCE);
    unlink(TEST_EXECUTABLE);
    unlink("test_coverage.gcno");
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", total_tests - passed_tests);
    
    return (passed_tests == total_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}
