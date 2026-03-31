#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEST_SOURCE "test_gcov.c"
#define TEST_BINARY "test_gcov"
#define TEST_GCNO "test_gcov.gcno"

// Function to create a minimal C source file for coverage testing
void create_test_source(void) {
    FILE *fp = fopen(TEST_SOURCE, "w");
    if (!fp) {
        perror("Failed to create test source file");
        exit(EXIT_FAILURE);
    }
    
    fprintf(fp, "int main() { \n");
    fprintf(fp, "    int x = 0;\n");
    fprintf(fp, "    if (x == 0) {\n");
    fprintf(fp, "        x = 1;\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return x;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    printf("Created test source file: %s\n", TEST_SOURCE);
}

// Function to compile the test program with coverage flags
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
            printf("Successfully compiled test program\n");
            
            // Verify .gcno file was created
            struct stat st;
            if (stat(TEST_GCNO, &st) == 0) {
                printf("Generated coverage file: %s (size: %ld bytes)\n", 
                       TEST_GCNO, (long)st.st_size);
                return 1;
            } else {
                printf("Warning: %s not found\n", TEST_GCNO);
                return 0;
            }
        } else {
            printf("Compilation failed\n");
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

// Function to run gcov-dump with specified arguments
int run_gcov_dump(const char *gcov_dump_path, char *const args[], 
                  int expect_success, const char *test_name) {
    printf("\n=== Testing: %s ===\n", test_name);
    printf("Command: %s", gcov_dump_path);
    
    for (int i = 0; args[i] != NULL; i++) {
        printf(" %s", args[i]);
    }
    printf("\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: execute gcov-dump
        execvp(gcov_dump_path, (char *const *)args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process: wait and check result
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            
            if (expect_success) {
                if (exit_code == 0) {
                    printf("✓ PASS: %s (exit code: %d)\n", test_name, exit_code);
                    return 1;
                } else {
                    printf("✗ FAIL: %s - Expected success, got exit code: %d\n", 
                           test_name, exit_code);
                    return 0;
                }
            } else {
                if (exit_code != 0) {
                    printf("✓ PASS: %s (exit code: %d)\n", test_name, exit_code);
                    return 1;
                } else {
                    printf("✗ FAIL: %s - Expected failure, got exit code: %d\n", 
                           test_name, exit_code);
                    return 0;
                }
            }
        } else {
            printf("✗ FAIL: %s - Process terminated abnormally\n", test_name);
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

int main(int argc, char *argv[]) {
    const char *gcov_dump_path = NULL;
    
    // Try to get gcov-dump path from environment or use default
    gcov_dump_path = getenv("GCOV_DUMP");
    if (gcov_dump_path == NULL) {
        gcov_dump_path = "gcov-dump";
    }
    
    printf("Using gcov-dump executable: %s\n", gcov_dump_path);
    
    // Create test source file
    create_test_source();
    
    // Compile with coverage to generate .gcno file
    int has_gcno = compile_with_coverage();
    
    if (!has_gcno) {
        printf("Warning: No .gcno file available for testing\n");
        printf("Tests requiring .gcno files will be skipped\n");
    }
    
    int passed = 0;
    int total = 0;
    
    // Test 1: -h (help)
    {
        char *args[] = { "gcov-dump", "-h", NULL };
        passed += run_gcov_dump(gcov_dump_path, args, 1, "Help flag (-h)");
        total++;
    }
    
    // Test 2: -v (version)
    {
        char *args[] = { "gcov-dump", "-v", NULL };
        passed += run_gcov_dump(gcov_dump_path, args, 1, "Version flag (-v)");
        total++;
    }
    
    // Test 3: -l (dump contents) - requires .gcno file
    if (has_gcno) {
        char *args[] = { "gcov-dump", "-l", TEST_GCNO, NULL };
        passed += run_gcov_dump(gcov_dump_path, args, 1, "Dump contents flag (-l)");
        total++;
    } else {
        printf("\nSkipping -l test (no .gcno file)\n");
    }
    
    // Test 4: -p (dump positions) - requires .gcno file
    if (has_gcno) {
        char *args[] = { "gcov-dump", "-p", TEST_GCNO, NULL };
        passed += run_gcov_dump(gcov_dump_path, args, 1, "Dump positions flag (-p)");
        total++;
    } else {
        printf("\nSkipping -p test (no .gcno file)\n");
    }
    
    // Test 5: -r (dump raw) - requires .gcno file
    if (has_gcno) {
        char *args[] = { "gcov-dump", "-r", TEST_GCNO, NULL };
        passed += run_gcov_dump(gcov_dump_path, args, 1, "Dump raw flag (-r)");
        total++;
    } else {
        printf("\nSkipping -r test (no .gcno file)\n");
    }
    
    // Test 6: -s (dump stable) - requires .gcno file
    if (has_gcno) {
        char *args[] = { "gcov-dump", "-s", TEST_GCNO, NULL };
        passed += run_gcov_dump(gcov_dump_path, args, 1, "Dump stable flag (-s)");
        total++;
    } else {
        printf("\nSkipping -s test (no .gcno file)\n");
    }
    
    // Test 7: Invalid flag (-x) - should trigger default case
    {
        char *args[] = { "gcov-dump", "-x", NULL };
        passed += run_gcov_dump(gcov_dump_path, args, 0, "Invalid flag (-x)");
        total++;
    }
    
    // Test 8: Another invalid flag (-?) - should trigger default case
    {
        char *args[] = { "gcov-dump", "-?", NULL };
        passed += run_gcov_dump(gcov_dump_path, args, 0, "Invalid flag (-?)");
        total++;
    }
    
    // Test 9: Yet another invalid flag (-Z) - should trigger default case
    {
        char *args[] = { "gcov-dump", "-Z", NULL };
        passed += run_gcov_dump(gcov_dump_path, args, 0, "Invalid flag (-Z)");
        total++;
    }
    
    // Summary
    printf("\n=== TEST SUMMARY ===\n");
    printf("Passed: %d/%d tests\n", passed, total);
    
    // Cleanup
    printf("\nCleaning up...\n");
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    if (has_gcno) {
        unlink(TEST_GCNO);
    }
    
    return (passed == total) ? EXIT_SUCCESS : EXIT_FAILURE;
}
