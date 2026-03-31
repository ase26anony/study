#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEST_SOURCE "test.c"
#define TEST_EXECUTABLE "test"
#define TEST_GCNO "test.gcno"
#define TEST_GCDA "test.gcda"

void create_test_source(void) {
    FILE *f = fopen(TEST_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

void compile_test_with_coverage(void) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: compile with coverage
        execlp("gcc", "gcc", "-O0", "--coverage", "-fprofile-arcs", 
               "-ftest-coverage", "-o", TEST_EXECUTABLE, TEST_SOURCE, NULL);
        perror("Failed to execute gcc");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Failed to compile test program\n");
            exit(EXIT_FAILURE);
        }
    } else {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
}

int run_gcov_dump(const char *gcov_dump_path, char *const args[], int *exit_status) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: run gcov-dump
        execvp(gcov_dump_path, args);
        perror("Failed to execute gcov-dump");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            *exit_status = WEXITSTATUS(status);
            return 1;
        } else {
            *exit_status = -1;
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

void test_case(const char *name, const char *gcov_dump_path, char *const args[], 
               int expected_exit, const char *expected_stderr_prefix) {
    printf("Testing %s: ", name);
    fflush(stdout);
    
    int exit_status;
    if (run_gcov_dump(gcov_dump_path, (char *const *)args, &exit_status)) {
        if (exit_status == expected_exit) {
            printf("PASS (exit code %d)\n", exit_status);
        } else {
            printf("FAIL (expected %d, got %d)\n", expected_exit, exit_status);
        }
    } else {
        printf("FAIL (execution failed)\n");
    }
}

int main(int argc, char *argv[]) {
    const char *gcov_dump_path = "./gcov-dump";
    
    // Try to get path from environment variable
    char *env_path = getenv("GCOV_DUMP");
    if (env_path && strlen(env_path) > 0) {
        gcov_dump_path = env_path;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Create test files
    printf("Creating test files...\n");
    create_test_source();
    compile_test_with_coverage();
    
    // Test cases
    printf("\nRunning test cases:\n");
    
    // Test 1: -h (help)
    {
        char *args[] = { "gcov-dump", "-h", NULL };
        test_case("-h (help)", gcov_dump_path, args, 0, NULL);
    }
    
    // Test 2: -v (version)
    {
        char *args[] = { "gcov-dump", "-v", NULL };
        test_case("-v (version)", gcov_dump_path, args, 0, NULL);
    }
    
    // Test 3: -l (dump contents)
    {
        char *args[] = { "gcov-dump", "-l", TEST_GCNO, NULL };
        test_case("-l (dump contents)", gcov_dump_path, args, 0, NULL);
    }
    
    // Test 4: -p (dump positions)
    {
        char *args[] = { "gcov-dump", "-p", TEST_GCNO, NULL };
        test_case("-p (dump positions)", gcov_dump_path, args, 0, NULL);
    }
    
    // Test 5: -r (dump raw)
    {
        char *args[] = { "gcov-dump", "-r", TEST_GCNO, NULL };
        test_case("-r (dump raw)", gcov_dump_path, args, 0, NULL);
    }
    
    // Test 6: -s (dump stable)
    {
        char *args[] = { "gcov-dump", "-s", TEST_GCNO, NULL };
        test_case("-s (dump stable)", gcov_dump_path, args, 0, NULL);
    }
    
    // Test 7: -x (unknown flag)
    {
        char *args[] = { "gcov-dump", "-x", NULL };
        test_case("-x (unknown flag)", gcov_dump_path, args, 1, "unknown flag");
    }
    
    // Test 8: -? (unknown flag)
    {
        char *args[] = { "gcov-dump", "-?", NULL };
        test_case("-? (unknown flag)", gcov_dump_path, args, 1, "unknown flag");
    }
    
    // Test 9: -Z (unknown flag)
    {
        char *args[] = { "gcov-dump", "-Z", NULL };
        test_case("-Z (unknown flag)", gcov_dump_path, args, 1, "unknown flag");
    }
    
    // Cleanup
    printf("\nCleaning up test files...\n");
    unlink(TEST_SOURCE);
    unlink(TEST_EXECUTABLE);
    unlink(TEST_GCNO);
    unlink(TEST_GCDA);
    
    printf("\nAll tests completed.\n");
    return 0;
}
