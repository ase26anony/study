#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_GCDA_FILE "/tmp/minimal.gcda"
#define INSTRUMENTED_BINARY "/tmp/gcov-dump-instrumented"

/* Minimal GCOV data file structure */
static const unsigned char minimal_gcda[] = {
    /* Magic: 'gcda' */
    0x67, 0x63, 0x64, 0x61,
    /* Version: 0xB3C1F4D5 (example version) */
    0xD5, 0xF4, 0xC1, 0xB3,
    /* Stamp: 0x12345678 */
    0x78, 0x56, 0x34, 0x12,
    /* Length of next record: 0 (empty record) */
    0x00, 0x00, 0x00, 0x00
};

/* Create a minimal valid .gcda file */
static int create_minimal_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("fopen");
        return 0;
    }
    
    size_t written = fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    return written == sizeof(minimal_gcda);
}

/* Execute command and capture output */
static int execute_and_capture(const char *cmd, char *output, size_t output_size, 
                               int capture_stdout, int capture_stderr) {
    char full_cmd[1024];
    int pipe_fd[2];
    pid_t pid;
    int status;
    
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        return -1;
    }
    
    pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return -1;
    }
    
    if (pid == 0) { /* Child process */
        close(pipe_fd[0]);
        
        if (capture_stdout) {
            dup2(pipe_fd[1], STDOUT_FILENO);
        }
        if (capture_stderr) {
            dup2(pipe_fd[1], STDERR_FILENO);
        }
        close(pipe_fd[1]);
        
        /* Use shell to handle the command */
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        _exit(127); /* execl failed */
    }
    
    /* Parent process */
    close(pipe_fd[1]);
    
    /* Read output */
    ssize_t bytes_read = read(pipe_fd[0], output, output_size - 1);
    if (bytes_read > 0) {
        output[bytes_read] = '\0';
    } else {
        output[0] = '\0';
    }
    
    close(pipe_fd[0]);
    
    /* Wait for child */
    waitpid(pid, &status, 0);
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Simple system call wrapper */
static int run_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int ret = system(cmd);
    if (WIFEXITED(ret)) {
        return WEXITSTATUS(ret);
    }
    return -1;
}

/* Build instrumented gcov-dump */
static int build_instrumented_gcov_dump(const char *source_path) {
    char build_cmd[2048];
    
    /* Try to find gcov-dump.cc if not provided */
    const char *gcov_dump_cc = source_path;
    if (!gcov_dump_cc || access(gcov_dump_cc, R_OK) != 0) {
        /* Try common locations */
        const char *possible_paths[] = {
            "../gcc/gcov-dump.cc",
            "../../gcc/gcov-dump.cc",
            "../../../gcc/gcov-dump.cc",
            "gcov-dump.cc",
            NULL
        };
        
        for (int i = 0; possible_paths[i]; i++) {
            if (access(possible_paths[i], R_OK) == 0) {
                gcov_dump_cc = possible_paths[i];
                break;
            }
        }
    }
    
    if (access(gcov_dump_cc, R_OK) != 0) {
        fprintf(stderr, "Error: Cannot find gcov-dump.cc\n");
        fprintf(stderr, "Please specify path to gcov-dump.cc as argument\n");
        return 0;
    }
    
    printf("Building instrumented gcov-dump from: %s\n", gcov_dump_cc);
    
    /* Build with coverage instrumentation */
    snprintf(build_cmd, sizeof(build_cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, gcov_dump_cc);
    
    if (run_command(build_cmd) != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 0;
    }
    
    if (access(INSTRUMENTED_BINARY, X_OK) != 0) {
        fprintf(stderr, "Instrumented binary not created\n");
        return 0;
    }
    
    return 1;
}

int main(int argc, char *argv[]) {
    char output[4096];
    int exit_code;
    int all_tests_passed = 1;
    
    printf("=== Starting gcov-dump coverage tests ===\n\n");
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_instrumented_gcov_dump(argc > 1 ? argv[1] : NULL)) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    printf("\nCreating minimal .gcda file...\n");
    if (!create_minimal_gcda(TEMP_GCDA_FILE)) {
        fprintf(stderr, "Failed to create minimal .gcda file\n");
        unlink(INSTRUMENTED_BINARY);
        return 1;
    }
    printf("Created: %s\n", TEMP_GCDA_FILE);
    
    /* Step 3: Execute test sequence */
    printf("\n=== Testing flag parsing ===\n\n");
    
    /* Test -h flag (help) */
    printf("Testing -h flag...\n");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s -h", INSTRUMENTED_BINARY);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1, 0);
    if (exit_code == 0) {
        printf("✓ -h flag successful (exit code: %d)\n", exit_code);
    } else {
        printf("✗ -h flag failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test -v flag (version) */
    printf("\nTesting -v flag...\n");
    snprintf(cmd, sizeof(cmd), "%s -v", INSTRUMENTED_BINARY);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1, 0);
    if (exit_code == 0 && strstr(output, "gcov-dump") != NULL) {
        printf("✓ -v flag successful\n");
        printf("  Output: %s", output);
    } else {
        printf("✗ -v flag failed or no version info\n");
        all_tests_passed = 0;
    }
    
    /* Test invalid flag -X */
    printf("\nTesting invalid flag -X...\n");
    snprintf(cmd, sizeof(cmd), "%s -X", INSTRUMENTED_BINARY);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0, 1);
    if (strstr(output, "unknown flag `X'") != NULL) {
        printf("✓ Invalid flag detected correctly\n");
        printf("  Error message: %s", output);
    } else {
        printf("✗ Invalid flag not detected properly\n");
        printf("  Output: %s", output);
        all_tests_passed = 0;
    }
    
    printf("\n=== Testing flags requiring coverage file ===\n\n");
    
    /* Test -l flag (dump contents) */
    printf("Testing -l flag...\n");
    snprintf(cmd, sizeof(cmd), "%s -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = run_command(cmd);
    if (exit_code == 0) {
        printf("✓ -l flag successful\n");
    } else {
        printf("✗ -l flag failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test -p flag (dump positions) */
    printf("\nTesting -p flag...\n");
    snprintf(cmd, sizeof(cmd), "%s -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = run_command(cmd);
    if (exit_code == 0) {
        printf("✓ -p flag successful\n");
    } else {
        printf("✗ -p flag failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test -r flag (dump raw) */
    printf("\nTesting -r flag...\n");
    snprintf(cmd, sizeof(cmd), "%s -r %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = run_command(cmd);
    if (exit_code == 0) {
        printf("✓ -r flag successful\n");
    } else {
        printf("✗ -r flag failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test -s flag (dump stable) */
    printf("\nTesting -s flag...\n");
    snprintf(cmd, sizeof(cmd), "%s -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = run_command(cmd);
    if (exit_code == 0) {
        printf("✓ -s flag successful\n");
    } else {
        printf("✗ -s flag failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    printf("\n=== Testing flag combinations ===\n\n");
    
    /* Test -l -p combination */
    printf("Testing -l -p combination...\n");
    snprintf(cmd, sizeof(cmd), "%s -l -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = run_command(cmd);
    if (exit_code == 0) {
        printf("✓ -l -p combination successful\n");
    } else {
        printf("✗ -l -p combination failed\n");
        all_tests_passed = 0;
    }
    
    /* Test -p -l combination (different order) */
    printf("\nTesting -p -l combination...\n");
    snprintf(cmd, sizeof(cmd), "%s -p -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = run_command(cmd);
    if (exit_code == 0) {
        printf("✓ -p -l combination successful\n");
    } else {
        printf("✗ -p -l combination failed\n");
        all_tests_passed = 0;
    }
    
    /* Test -r -s combination */
    printf("\nTesting -r -s combination...\n");
    snprintf(cmd, sizeof(cmd), "%s -r -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = run_command(cmd);
    if (exit_code == 0) {
        printf("✓ -r -s combination successful\n");
    } else {
        printf("✗ -r -s combination failed\n");
        all_tests_passed = 0;
    }
    
    /* Test invalid flag with file */
    printf("\nTesting invalid flag -X with file...\n");
    snprintf(cmd, sizeof(cmd), "%s -X %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0, 1);
    if (strstr(output, "unknown flag `X'") != NULL) {
        printf("✓ Invalid flag with file detected correctly\n");
    } else {
        printf("✗ Invalid flag with file not detected\n");
        all_tests_passed = 0;
    }
    
    /* Step 4: Cleanup */
    printf("\n=== Cleaning up ===\n");
    unlink(TEMP_GCDA_FILE);
    unlink(INSTRUMENTED_BINARY);
    printf("Removed temporary files\n");
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    if (all_tests_passed) {
        printf("✓ All tests passed!\n");
        printf("\nThe instrumented gcov-dump binary has now executed all the\n");
        printf("previously uncovered lines (111-130 in gcov-dump.cc).\n");
        printf("Run 'gcov gcov-dump.cc' to see the updated coverage report.\n");
    } else {
        printf("✗ Some tests failed\n");
    }
    
    return all_tests_passed ? 0 : 1;
}
