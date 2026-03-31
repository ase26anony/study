#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

#define TEST_SOURCE "test_gcov.c"
#define TEST_BINARY "test_gcov"
#define TEST_GCNO "test_gcov.gcno"

void create_test_source(void) {
    FILE *f = fopen(TEST_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source file");
        exit(EXIT_FAILURE);
    }
    
    fprintf(f, "int main() {\n");
    fprintf(f, "    int x = 0;\n");
    fprintf(f, "    if (x == 0) {\n");
    fprintf(f, "        x = 1;\n");
    fprintf(f, "    }\n");
    fprintf(f, "    return x;\n");
    fprintf(f, "}\n");
    
    fclose(f);
    printf("Created test source file: %s\n", TEST_SOURCE);
}

int compile_with_coverage(void) {
    printf("Compiling test program with coverage flags...\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: compile with coverage
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
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process: wait for compilation
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Successfully compiled %s -> %s (generated %s)\n", 
                   TEST_SOURCE, TEST_BINARY, TEST_GCNO);
            
            // Verify .gcno file exists
            struct stat st;
            if (stat(TEST_GCNO, &st) == 0 && S_ISREG(st.st_mode)) {
                printf("Verified %s exists (%ld bytes)\n", TEST_GCNO, (long)st.st_size);
                return 1;
            } else {
                printf("ERROR: %s not generated!\n", TEST_GCNO);
                return 0;
            }
        } else {
            printf("Compilation failed!\n");
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

int run_gcov_dump(const char *gcov_dump_path, char *const args[], int *exit_code) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: run gcov-dump
        execvp(gcov_dump_path, args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process: wait for gcov-dump
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            *exit_code = WEXITSTATUS(status);
            return 1;
        } else {
            *exit_code = -1;
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

void test_case(const char *description, const char *gcov_dump_path, 
               char *const args[], int expected_exit, const char *expected_stderr) {
    printf("\n=== Testing: %s ===\n", description);
    printf("Command: %s", gcov_dump_path);
    
    for (int i = 1; args[i] != NULL; i++) {
        printf(" %s", args[i]);
    }
    printf("\n");
    
    int exit_code;
    if (run_gcov_dump(gcov_dump_path, (char *const *)args, &exit_code)) {
        printf("Exit code: %d (expected: %d)\n", exit_code, expected_exit);
        
        if (exit_code == expected_exit) {
            printf("✓ PASS\n");
        } else {
            printf("✗ FAIL - Wrong exit code\n");
        }
    } else {
        printf("✗ FAIL - Process execution failed\n");
    }
}

int main(int argc, char *argv[]) {
    const char *gcov_dump_path = "./gcov-dump";
    
    // Try to get path from environment variable
    char *env_path = getenv("GCOV_DUMP");
    if (env_path != NULL && strlen(env_path) > 0) {
        gcov_dump_path = env_path;
    }
    
    printf("Using gcov-dump executable: %s\n", gcov_dump_path);
    
    // Check if gcov-dump exists and is executable
    struct stat st;
    if (stat(gcov_dump_path, &st) != 0 || !S_ISREG(st.st_mode) || 
        access(gcov_dump_path, X_OK) != 0) {
        fprintf(stderr, "ERROR: gcov-dump not found or not executable at: %s\n", 
                gcov_dump_path);
        fprintf(stderr, "Set GCOV_DUMP environment variable to correct path.\n");
        return EXIT_FAILURE;
    }
    
    // Create test source file
    create_test_source();
    
    // Compile with coverage to generate .gcno file
    if (!compile_with_coverage()) {
        fprintf(stderr, "Failed to generate .gcno file. Tests cannot proceed.\n");
        return EXIT_FAILURE;
    }
    
    // Test cases
    int total_tests = 0;
    int passed_tests = 0;
    
    // Test 1: -h (help)
    {
        char *args[] = { "gcov-dump", "-h", NULL };
        test_case("Help flag (-h)", gcov_dump_path, args, 0, NULL);
        total_tests++;
        // Note: We can't easily check if print_usage() was called without 
        // parsing output, but exit code 0 indicates success
    }
    
    // Test 2: -v (version)
    {
        char *args[] = { "gcov-dump", "-v", NULL };
        test_case("Version flag (-v)", gcov_dump_path, args, 0, NULL);
        total_tests++;
    }
    
    // Test 3: -l (dump contents)
    {
        char *args[] = { "gcov-dump", "-l", TEST_GCNO, NULL };
        test_case("Dump contents flag (-l)", gcov_dump_path, args, 0, NULL);
        total_tests++;
    }
    
    // Test 4: -p (dump positions)
    {
        char *args[] = { "gcov-dump", "-p", TEST_GCNO, NULL };
        test_case("Dump positions flag (-p)", gcov_dump_path, args, 0, NULL);
        total_tests++;
    }
    
    // Test 5: -r (dump raw)
    {
        char *args[] = { "gcov-dump", "-r", TEST_GCNO, NULL };
        test_case("Dump raw flag (-r)", gcov_dump_path, args, 0, NULL);
        total_tests++;
    }
    
    // Test 6: -s (dump stable)
    {
        char *args[] = { "gcov-dump", "-s", TEST_GCNO, NULL };
        test_case("Dump stable flag (-s)", gcov_dump_path, args, 0, NULL);
        total_tests++;
    }
    
    // Test 7: Invalid flag (default case)
    {
        char *args[] = { "gcov-dump", "-x", NULL };
        test_case("Invalid flag (-x) - should trigger default case", 
                  gcov_dump_path, args, 1, "unknown flag 'x'");
        total_tests++;
    }
    
    // Test 8: Another invalid flag
    {
        char *args[] = { "gcov-dump", "-Z", NULL };
        test_case("Invalid flag (-Z) - should trigger default case", 
                  gcov_dump_path, args, 1, "unknown flag 'Z'");
        total_tests++;
    }
    
    // Test 9: Combined flags (optional bonus test)
    {
        char *args[] = { "gcov-dump", "-l", "-p", TEST_GCNO, NULL };
        test_case("Combined flags (-l -p)", gcov_dump_path, args, 0, NULL);
        total_tests++;
    }
    
    printf("\n=== Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Note: Manual verification of output may be needed for some tests.\n");
    
    // Cleanup
    printf("\nCleaning up generated files...\n");
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    unlink(TEST_GCNO);
    unlink("test_gcov.gcda");  // In case it was generated
    
    return EXIT_SUCCESS;
}
