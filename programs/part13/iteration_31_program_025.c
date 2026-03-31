#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEST_SOURCE "test.c"
#define TEST_BINARY "test"
#define GCNO_FILE "test.gcno"
#define GCOV_DUMP_EXEC "./gcov-dump"

void run_gcov_dump(const char **args, const char *test_name) {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        execvp(GCOV_DUMP_EXEC, (char *const *)args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("%s: %s (exit code: %d)\n", 
                   test_name, 
                   exit_code == 0 ? "PASSED" : "FAILED", 
                   exit_code);
        } else {
            printf("%s: FAILED (did not exit normally)\n", test_name);
        }
    } else {
        perror("fork failed");
    }
}

int create_test_gcno() {
    // Create minimal C source file
    FILE *fp = fopen(TEST_SOURCE, "w");
    if (!fp) {
        perror("Failed to create test.c");
        return 0;
    }
    fprintf(fp, "int main() { return 0; }\n");
    fclose(fp);
    
    // Compile with coverage flags
    const char *compile_args[] = {
        "gcc", "-O0", "--coverage", "-fprofile-arcs", "-ftest-coverage",
        "-o", TEST_BINARY, TEST_SOURCE, NULL
    };
    
    pid_t pid = fork();
    if (pid == 0) {
        execvp("gcc", (char *const *)compile_args);
        perror("gcc compilation failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Failed to compile test program\n");
            return 0;
        }
    } else {
        perror("fork failed for compilation");
        return 0;
    }
    
    // Verify gcno file exists
    struct stat st;
    if (stat(GCNO_FILE, &st) != 0) {
        fprintf(stderr, "No .gcno file generated\n");
        return 0;
    }
    
    return 1;
}

void cleanup() {
    // Remove generated files
    remove(TEST_SOURCE);
    remove(TEST_BINARY);
    remove(GCNO_FILE);
    remove("test.gcda");  // In case it was created
}

int main() {
    printf("=== Testing gcov-dump uncovered lines ===\n\n");
    
    // Test 1: -h flag (help)
    printf("1. Testing -h flag (help):\n");
    const char *args_h[] = {GCOV_DUMP_EXEC, "-h", NULL};
    run_gcov_dump(args_h, "  -h flag");
    
    // Test 2: -v flag (version)
    printf("\n2. Testing -v flag (version):\n");
    const char *args_v[] = {GCOV_DUMP_EXEC, "-v", NULL};
    run_gcov_dump(args_v, "  -v flag");
    
    // Create test .gcno file for remaining tests
    printf("\n3. Creating test .gcno file...\n");
    if (!create_test_gcno()) {
        fprintf(stderr, "Failed to create test .gcno file\n");
        cleanup();
        return EXIT_FAILURE;
    }
    printf("   Created %s successfully\n", GCNO_FILE);
    
    // Test 3: -l flag (dump contents)
    printf("\n4. Testing -l flag (dump contents):\n");
    const char *args_l[] = {GCOV_DUMP_EXEC, "-l", GCNO_FILE, NULL};
    run_gcov_dump(args_l, "  -l flag");
    
    // Test 4: -p flag (dump positions)
    printf("\n5. Testing -p flag (dump positions):\n");
    const char *args_p[] = {GCOV_DUMP_EXEC, "-p", GCNO_FILE, NULL};
    run_gcov_dump(args_p, "  -p flag");
    
    // Test 5: -r flag (dump raw)
    printf("\n6. Testing -r flag (dump raw):\n");
    const char *args_r[] = {GCOV_DUMP_EXEC, "-r", GCNO_FILE, NULL};
    run_gcov_dump(args_r, "  -r flag");
    
    // Test 6: -s flag (dump stable)
    printf("\n7. Testing -s flag (dump stable):\n");
    const char *args_s[] = {GCOV_DUMP_EXEC, "-s", GCNO_FILE, NULL};
    run_gcov_dump(args_s, "  -s flag");
    
    // Test 7: Invalid flag (default case)
    printf("\n8. Testing invalid flag (default case):\n");
    const char *args_invalid[] = {GCOV_DUMP_EXEC, "-x", NULL};
    run_gcov_dump(args_invalid, "  -x flag (invalid)");
    
    // Test another invalid flag
    printf("\n9. Testing another invalid flag:\n");
    const char *args_invalid2[] = {GCOV_DUMP_EXEC, "-Z", NULL};
    run_gcov_dump(args_invalid2, "  -Z flag (invalid)");
    
    // Test yet another invalid flag
    printf("\n10. Testing another invalid flag:\n");
    const char *args_invalid3[] = {GCOV_DUMP_EXEC, "-?", NULL};
    run_gcov_dump(args_invalid3, "  -? flag (invalid)");
    
    // Cleanup
    printf("\n11. Cleaning up...\n");
    cleanup();
    printf("   Cleanup complete\n");
    
    printf("\n=== All tests completed ===\n");
    
    return EXIT_SUCCESS;
}
