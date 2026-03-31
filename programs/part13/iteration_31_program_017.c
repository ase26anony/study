#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEST_SRC "test.c"
#define TEST_EXE "test"
#define TEST_GCNO "test.gcno"

// Create minimal test source file
void create_test_source(void) {
    FILE *f = fopen(TEST_SRC, "w");
    if (!f) {
        perror("Failed to create test.c");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

// Compile test source with coverage flags
int compile_with_coverage(void) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: compile
        execlp("gcc", "gcc", "-O0", "--coverage", "-fprofile-arcs", 
               "-ftest-coverage", "-o", TEST_EXE, TEST_SRC, NULL);
        perror("exec gcc failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent: wait for compilation
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Compilation failed\n");
            return 0;
        }
        
        // Verify gcno file was created
        struct stat st;
        if (stat(TEST_GCNO, &st) != 0) {
            fprintf(stderr, "No gcno file generated\n");
            return 0;
        }
        return 1;
    } else {
        perror("fork failed");
        return 0;
    }
}

// Run gcov-dump with specified arguments and check result
int run_gcov_dump(const char *gcov_dump_path, char *const argv[], 
                  int expect_success, const char *test_name) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: run gcov-dump
        execvp(gcov_dump_path, argv);
        perror("exec gcov-dump failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent: wait and check result
        int status;
        waitpid(pid, &status, 0);
        
        int success = 0;
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (expect_success) {
                success = (exit_code == 0);
                if (!success) {
                    printf("FAIL: %s exited with code %d (expected 0)\n", 
                           test_name, exit_code);
                }
            } else {
                success = (exit_code != 0);
                if (!success) {
                    printf("FAIL: %s exited with code %d (expected non-zero)\n", 
                           test_name, exit_code);
                }
            }
        } else {
            printf("FAIL: %s terminated abnormally\n", test_name);
        }
        
        return success;
    } else {
        perror("fork failed");
        return 0;
    }
}

int main(int argc, char *argv[]) {
    // Determine gcov-dump path
    const char *gcov_dump_path = getenv("GCOV_DUMP");
    if (!gcov_dump_path) {
        gcov_dump_path = "gcov-dump";  // Assume in PATH
    }
    
    printf("Testing gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Create test files
    printf("Creating test source file...\n");
    create_test_source();
    
    printf("Compiling with coverage flags...\n");
    if (!compile_with_coverage()) {
        fprintf(stderr, "Failed to create test.gcno file\n");
        return EXIT_FAILURE;
    }
    printf("Created %s successfully\n\n", TEST_GCNO);
    
    int passed = 0;
    int total = 0;
    
    // Test cases
    char *args[10];
    
    // 1. Test -h (help)
    printf("Test 1: -h (help flag)\n");
    args[0] = (char*)gcov_dump_path;
    args[1] = "-h";
    args[2] = NULL;
    if (run_gcov_dump(gcov_dump_path, args, 1, "-h")) {
        printf("PASS: -h flag\n");
        passed++;
    }
    total++;
    printf("\n");
    
    // 2. Test -v (version)
    printf("Test 2: -v (version flag)\n");
    args[1] = "-v";
    if (run_gcov_dump(gcov_dump_path, args, 1, "-v")) {
        printf("PASS: -v flag\n");
        passed++;
    }
    total++;
    printf("\n");
    
    // 3. Test -l (dump contents)
    printf("Test 3: -l (dump contents)\n");
    args[1] = "-l";
    args[2] = TEST_GCNO;
    args[3] = NULL;
    if (run_gcov_dump(gcov_dump_path, args, 1, "-l")) {
        printf("PASS: -l flag\n");
        passed++;
    }
    total++;
    printf("\n");
    
    // 4. Test -p (dump positions)
    printf("Test 4: -p (dump positions)\n");
    args[1] = "-p";
    if (run_gcov_dump(gcov_dump_path, args, 1, "-p")) {
        printf("PASS: -p flag\n");
        passed++;
    }
    total++;
    printf("\n");
    
    // 5. Test -r (dump raw)
    printf("Test 5: -r (dump raw)\n");
    args[1] = "-r";
    if (run_gcov_dump(gcov_dump_path, args, 1, "-r")) {
        printf("PASS: -r flag\n");
        passed++;
    }
    total++;
    printf("\n");
    
    // 6. Test -s (dump stable)
    printf("Test 6: -s (dump stable)\n");
    args[1] = "-s";
    if (run_gcov_dump(gcov_dump_path, args, 1, "-s")) {
        printf("PASS: -s flag\n");
        passed++;
    }
    total++;
    printf("\n");
    
    // 7. Test invalid flag (default case)
    printf("Test 7: -x (invalid flag - should trigger default case)\n");
    args[1] = "-x";
    args[2] = NULL;
    if (run_gcov_dump(gcov_dump_path, args, 0, "-x")) {
        printf("PASS: -x flag (correctly rejected)\n");
        passed++;
    }
    total++;
    printf("\n");
    
    // Cleanup
    unlink(TEST_SRC);
    unlink(TEST_EXE);
    
    // Summary
    printf("========================================\n");
    printf("Test Summary: %d/%d tests passed\n", passed, total);
    printf("========================================\n");
    
    return (passed == total) ? EXIT_SUCCESS : EXIT_FAILURE;
}
