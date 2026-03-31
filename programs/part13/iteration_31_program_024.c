#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEST_SOURCE "test_coverage.c"
#define TEST_BINARY "test_coverage"
#define GCNO_FILE "test_coverage.gcno"

void create_test_source(void) {
    FILE *f = fopen(TEST_SOURCE, "w");
    if (!f) {
        perror("Failed to create test source");
        exit(1);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

void compile_with_coverage(void) {
    char *args[] = {"gcc", "-O0", "--coverage", "-fprofile-arcs", 
                    "-ftest-coverage", "-o", TEST_BINARY, TEST_SOURCE, NULL};
    
    pid_t pid = fork();
    if (pid == 0) {
        execvp("gcc", args);
        perror("execvp failed");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Failed to compile test program\n");
            exit(1);
        }
    } else {
        perror("fork failed");
        exit(1);
    }
}

int run_gcov_dump(const char *gcov_dump_path, char *const args[], 
                  int *exit_status, char *output, size_t output_size) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return -1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        execvp(gcov_dump_path, args);
        perror("execvp failed");
        exit(1);
    } else if (pid > 0) {
        close(pipefd[1]);
        
        ssize_t bytes_read = read(pipefd[0], output, output_size - 1);
        if (bytes_read > 0) {
            output[bytes_read] = '\0';
        } else {
            output[0] = '\0';
        }
        close(pipefd[0]);
        
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            *exit_status = WEXITSTATUS(status);
            return 0;
        } else {
            return -1;
        }
    } else {
        perror("fork failed");
        return -1;
    }
}

void test_case(const char *name, const char *gcov_dump_path, 
               char *const args[], int expected_exit, 
               const char *expected_output_contains) {
    printf("Testing: %-30s", name);
    fflush(stdout);
    
    char output[4096];
    int exit_status;
    
    if (run_gcov_dump(gcov_dump_path, args, &exit_status, output, sizeof(output)) != 0) {
        printf("FAILED (execution error)\n");
        return;
    }
    
    int passed = 1;
    if (exit_status != expected_exit) {
        printf("FAILED (exit code: %d, expected: %d)\n", exit_status, expected_exit);
        passed = 0;
    }
    
    if (expected_output_contains && 
        strstr(output, expected_output_contains) == NULL) {
        printf("FAILED (output doesn't contain '%s')\n", expected_output_contains);
        passed = 0;
    }
    
    if (passed) {
        printf("PASSED\n");
    }
}

int main(int argc, char *argv[]) {
    const char *gcov_dump_path = "./gcov-dump";
    
    // Try to get path from environment
    char *env_path = getenv("GCOV_DUMP");
    if (env_path && env_path[0]) {
        gcov_dump_path = env_path;
    }
    
    printf("Using gcov-dump at: %s\n\n", gcov_dump_path);
    
    // Create test coverage file
    printf("Creating test coverage file...\n");
    create_test_source();
    compile_with_coverage();
    
    // Test cases
    char *help_args[] = {gcov_dump_path, "-h", NULL};
    test_case("Help flag (-h)", gcov_dump_path, help_args, 0, "Usage:");
    
    char *version_args[] = {gcov_dump_path, "-v", NULL};
    test_case("Version flag (-v)", gcov_dump_path, version_args, 0, "gcov-dump");
    
    char *dump_contents_args[] = {gcov_dump_path, "-l", GCNO_FILE, NULL};
    test_case("Dump contents (-l)", gcov_dump_path, dump_contents_args, 0, NULL);
    
    char *dump_positions_args[] = {gcov_dump_path, "-p", GCNO_FILE, NULL};
    test_case("Dump positions (-p)", gcov_dump_path, dump_positions_args, 0, NULL);
    
    char *dump_raw_args[] = {gcov_dump_path, "-r", GCNO_FILE, NULL};
    test_case("Dump raw (-r)", gcov_dump_path, dump_raw_args, 0, NULL);
    
    char *dump_stable_args[] = {gcov_dump_path, "-s", GCNO_FILE, NULL};
    test_case("Dump stable (-s)", gcov_dump_path, dump_stable_args, 0, NULL);
    
    char *unknown_flag_args[] = {gcov_dump_path, "-x", NULL};
    test_case("Unknown flag (-x)", gcov_dump_path, unknown_flag_args, 1, "unknown flag");
    
    // Test multiple unknown flags
    char *unknown_flag2_args[] = {gcov_dump_path, "-Z", NULL};
    test_case("Unknown flag (-Z)", gcov_dump_path, unknown_flag2_args, 1, "unknown flag");
    
    // Cleanup
    unlink(TEST_SOURCE);
    unlink(TEST_BINARY);
    unlink(GCNO_FILE);
    
    printf("\nAll tests completed.\n");
    return 0;
}
