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
    /* Version: 0x76312e2a (gcov 12*) */
    0x2a, 0x2e, 0x31, 0x76,
    /* Stamp: 0x12345678 */
    0x78, 0x56, 0x34, 0x12,
    /* Record type 0 (GCOV_TAG_FUNCTION) with length 2 */
    0x00, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00,
    /* Function info: ident=1, checksum=0x1234, lineno_checksum=0x5678 */
    0x01, 0x00, 0x00, 0x00,
    0x34, 0x12, 0x00, 0x00,
    0x78, 0x56, 0x00, 0x00,
    /* Record type 3 (GCOV_TAG_PROGRAM_SUMMARY) with length 0 */
    0x03, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

/* Execute command and return exit status */
static int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Execute command and capture stderr */
static int execute_and_capture_stderr(const char *cmd, char *output, size_t output_size) {
    printf("Executing: %s\n", cmd);
    
    /* Create pipe for stderr */
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }
    
    if (pid == 0) { /* Child process */
        close(pipefd[0]); /* Close read end */
        dup2(pipefd[1], STDERR_FILENO); /* Redirect stderr to pipe */
        close(pipefd[1]);
        
        /* Execute command */
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        _exit(127); /* execl failed */
    }
    
    /* Parent process */
    close(pipefd[1]); /* Close write end */
    
    /* Read stderr output */
    ssize_t bytes_read = read(pipefd[0], output, output_size - 1);
    if (bytes_read > 0) {
        output[bytes_read] = '\0';
    } else {
        output[0] = '\0';
    }
    close(pipefd[0]);
    
    /* Wait for child */
    int status;
    waitpid(pid, &status, 0);
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Build instrumented gcov-dump */
static int build_gcov_dump(const char *source_path) {
    printf("Building instrumented gcov-dump...\n");
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    int result = execute_command(cmd);
    if (result != 0) {
        fprintf(stderr, "Failed to build gcov-dump. Command: %s\n", cmd);
        return 0;
    }
    
    /* Verify the binary exists */
    struct stat st;
    if (stat(INSTRUMENTED_BINARY, &st) != 0) {
        fprintf(stderr, "Instrumented binary not created: %s\n", INSTRUMENTED_BINARY);
        return 0;
    }
    
    printf("Successfully built instrumented gcov-dump at %s\n", INSTRUMENTED_BINARY);
    return 1;
}

/* Create minimal coverage file */
static int create_minimal_gcda(void) {
    printf("Creating minimal coverage file...\n");
    
    FILE *fp = fopen(TEMP_GCDA_FILE, "wb");
    if (!fp) {
        perror("fopen");
        return 0;
    }
    
    size_t written = fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    if (written != sizeof(minimal_gcda)) {
        fprintf(stderr, "Failed to write minimal gcda file\n");
        return 0;
    }
    
    printf("Created minimal coverage file: %s\n", TEMP_GCDA_FILE);
    return 1;
}

/* Test -h flag (help) */
static void test_help_flag(void) {
    printf("\n=== Testing -h flag ===\n");
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s -h", INSTRUMENTED_BINARY);
    
    int status = execute_command(cmd);
    if (status == 0) {
        printf("✓ -h flag test passed (exit code 0)\n");
    } else {
        printf("✗ -h flag test failed (exit code %d)\n", status);
    }
}

/* Test -v flag (version) */
static void test_version_flag(void) {
    printf("\n=== Testing -v flag ===\n");
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s -v", INSTRUMENTED_BINARY);
    
    /* Capture stdout to verify version info */
    char output[1024];
    snprintf(cmd, sizeof(cmd), "%s -v 2>&1", INSTRUMENTED_BINARY);
    FILE *fp = popen(cmd, "r");
    if (fp) {
        if (fgets(output, sizeof(output), fp) != NULL) {
            printf("Version output: %s", output);
            if (strstr(output, "gcov") || strstr(output, "version") || 
                strstr(output, "GCC")) {
                printf("✓ -v flag test passed (version info found)\n");
            } else {
                printf("✗ -v flag test failed (no version info)\n");
            }
        }
        pclose(fp);
    }
}

/* Test invalid flag */
static void test_invalid_flag(void) {
    printf("\n=== Testing invalid flag -X ===\n");
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s -X", INSTRUMENTED_BINARY);
    
    char stderr_output[1024];
    int status = execute_and_capture_stderr(cmd, stderr_output, sizeof(stderr_output));
    
    printf("Stderr output: %s", stderr_output);
    
    if (strstr(stderr_output, "unknown flag `X'")) {
        printf("✓ Invalid flag test passed (correct error message)\n");
    } else {
        printf("✗ Invalid flag test failed (missing or incorrect error message)\n");
    }
    
    if (status != 0) {
        printf("✓ Invalid flag test passed (non-zero exit code: %d)\n", status);
    } else {
        printf("✗ Invalid flag test failed (exit code should be non-zero)\n");
    }
}

/* Test flag with coverage file */
static void test_flag_with_file(const char *flag, const char *description) {
    printf("\n=== Testing %s flag (%s) ===\n", flag, description);
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flag, TEMP_GCDA_FILE);
    
    int status = execute_command(cmd);
    if (status == 0) {
        printf("✓ %s flag test passed (exit code 0)\n", flag);
    } else {
        printf("✗ %s flag test failed (exit code %d)\n", flag, status);
    }
}

/* Test flag combinations */
static void test_flag_combination(const char *flags, const char *description) {
    printf("\n=== Testing flag combination %s (%s) ===\n", flags, description);
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flags, TEMP_GCDA_FILE);
    
    int status = execute_command(cmd);
    if (status == 0) {
        printf("✓ Flag combination %s test passed (exit code 0)\n", flags);
    } else {
        printf("✗ Flag combination %s test failed (exit code %d)\n", flags, status);
    }
}

/* Cleanup temporary files */
static void cleanup(void) {
    printf("\n=== Cleaning up ===\n");
    
    if (remove(TEMP_GCDA_FILE) == 0) {
        printf("Removed: %s\n", TEMP_GCDA_FILE);
    }
    
    if (remove(INSTRUMENTED_BINARY) == 0) {
        printf("Removed: %s\n", INSTRUMENTED_BINARY);
    }
}

int main(int argc, char *argv[]) {
    printf("=== Starting gcov-dump coverage test ===\n");
    
    /* Determine gcov-dump source path */
    const char *gcov_dump_source = NULL;
    
    /* Try to find gcov-dump.cc in common locations */
    const char *possible_paths[] = {
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "/usr/src/gcc/gcc/gcov-dump.cc",
        "/usr/local/src/gcc/gcc/gcov-dump.cc",
        NULL
    };
    
    for (int i = 0; possible_paths[i] != NULL; i++) {
        struct stat st;
        if (stat(possible_paths[i], &st) == 0 && S_ISREG(st.st_mode)) {
            gcov_dump_source = possible_paths[i];
            break;
        }
    }
    
    if (!gcov_dump_source) {
        /* Use command line argument if provided */
        if (argc > 1) {
            gcov_dump_source = argv[1];
        } else {
            fprintf(stderr, "Error: gcov-dump.cc not found.\n");
            fprintf(stderr, "Usage: %s [path/to/gcov-dump.cc]\n", argv[0]);
            fprintf(stderr, "Or place gcov-dump.cc in ../gcc/ relative to this program.\n");
            return 1;
        }
    }
    
    printf("Using gcov-dump source: %s\n", gcov_dump_source);
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_gcov_dump(gcov_dump_source)) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    if (!create_minimal_gcda()) {
        return 1;
    }
    
    /* Step 3: Execute test sequence */
    
    /* Test flags without file argument */
    test_help_flag();
    test_version_flag();
    test_invalid_flag();
    
    /* Test flags with minimal coverage file */
    test_flag_with_file("-l", "dump contents");
    test_flag_with_file("-p", "dump positions");
    test_flag_with_file("-r", "dump raw");
    test_flag_with_file("-s", "dump stable");
    
    /* Test flag combinations */
    test_flag_combination("-l -p", "dump contents and positions");
    test_flag_combination("-p -l", "positions and contents (reversed order)");
    test_flag_combination("-r -s", "dump raw and stable");
    test_flag_combination("-s -r", "stable and raw (reversed order)");
    test_flag_combination("-l -p -r -s", "all flags combined");
    
    /* Test with multiple flags in different orders */
    printf("\n=== Testing flag ordering variations ===\n");
    char cmd[256];
    
    /* Test -l -p */
    snprintf(cmd, sizeof(cmd), "%s -l -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (execute_command(cmd) == 0) {
        printf("✓ -l -p ordering test passed\n");
    }
    
    /* Test -p -l */
    snprintf(cmd, sizeof(cmd), "%s -p -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (execute_command(cmd) == 0) {
        printf("✓ -p -l ordering test passed\n");
    }
    
    /* Test -r -s */
    snprintf(cmd, sizeof(cmd), "%s -r -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (execute_command(cmd) == 0) {
        printf("✓ -r -s ordering test passed\n");
    }
    
    /* Test -s -r */
    snprintf(cmd, sizeof(cmd), "%s -s -r %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (execute_command(cmd) == 0) {
        printf("✓ -s -r ordering test passed\n");
    }
    
    /* Step 4: Cleanup */
    cleanup();
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
