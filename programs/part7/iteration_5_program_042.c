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
    /* Version: 0xB3C1F4D5 (example version, adjust if needed) */
    0xD5, 0xF4, 0xC1, 0xB3,
    /* Stamp: 0x00000000 */
    0x00, 0x00, 0x00, 0x00,
    /* Zero-length record terminator */
    0x00, 0x00, 0x00, 0x00,
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
    if (capture_stderr) {
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    } else {
        snprintf(full_cmd, sizeof(full_cmd), "%s", cmd);
    }
    
    FILE *fp = popen(full_cmd, "r");
    if (!fp) {
        perror("popen");
        return -1;
    }
    
    if (output && output_size > 0) {
        size_t total = 0;
        while (!feof(fp) && total < output_size - 1) {
            size_t n = fread(output + total, 1, output_size - total - 1, fp);
            total += n;
        }
        output[total] = '\0';
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
}

/* Build instrumented gcov-dump */
static int build_instrumented_gcov_dump() {
    printf("Building instrumented gcov-dump...\n");
    
    /* Try to find gcov-dump.cc in common locations */
    const char *possible_paths[] = {
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "../../../gcc/gcov-dump.cc",
        "/usr/src/gcc/gcc/gcov-dump.cc",
        NULL
    };
    
    const char *source_path = NULL;
    struct stat st;
    
    for (int i = 0; possible_paths[i]; i++) {
        if (stat(possible_paths[i], &st) == 0) {
            source_path = possible_paths[i];
            break;
        }
    }
    
    if (!source_path) {
        /* Try current directory */
        if (stat("gcov-dump.cc", &st) == 0) {
            source_path = "gcov-dump.cc";
        } else {
            fprintf(stderr, "Could not find gcov-dump.cc\n");
            return 0;
        }
    }
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    printf("Compiling: %s\n", cmd);
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Compilation failed with code %d\n", result);
        return 0;
    }
    
    /* Verify the binary was created */
    if (stat(INSTRUMENTED_BINARY, &st) != 0) {
        fprintf(stderr, "Binary not created: %s\n", INSTRUMENTED_BINARY);
        return 0;
    }
    
    printf("Instrumented binary created: %s\n", INSTRUMENTED_BINARY);
    return 1;
}

/* Test individual flag */
static void test_flag(const char *flag, const char *filename, 
                      int expect_success, const char *expected_output_substr,
                      int capture_stderr) {
    printf("\nTesting flag '%s'...\n", flag);
    
    char cmd[1024];
    if (filename) {
        snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flag, filename);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s", INSTRUMENTED_BINARY, flag);
    }
    
    char output[4096] = {0};
    int exit_code = execute_and_capture(cmd, output, sizeof(output), 1, capture_stderr);
    
    printf("Command: %s\n", cmd);
    printf("Exit code: %d\n", exit_code);
    
    if (expected_output_substr) {
        if (strstr(output, expected_output_substr) != NULL) {
            printf("✓ Found expected output: '%s'\n", expected_output_substr);
        } else {
            printf("✗ Expected output not found: '%s'\n", expected_output_substr);
            printf("Actual output:\n%s\n", output);
        }
    }
    
    if (expect_success) {
        if (exit_code == 0) {
            printf("✓ Exit code as expected (0)\n");
        } else {
            printf("✗ Unexpected exit code: %d\n", exit_code);
        }
    }
}

int main() {
    printf("=== GCOV-Dump Coverage Test ===\n");
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_instrumented_gcov_dump()) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
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
    
    /* Test -h flag (help) - no file needed */
    test_flag("-h", NULL, 1, "Usage:", 0);
    
    /* Test -v flag (version) - no file needed */
    test_flag("-v", NULL, 1, "gcov-dump", 0);
    
    /* Test -l flag (dump contents) - requires file */
    test_flag("-l", TEMP_GCDA_FILE, 1, NULL, 0);
    
    /* Test -p flag (dump positions) - requires file */
    test_flag("-p", TEMP_GCDA_FILE, 1, NULL, 0);
    
    /* Test -r flag (dump raw) - requires file */
    test_flag("-r", TEMP_GCDA_FILE, 1, NULL, 0);
    
    /* Test -s flag (dump stable) - requires file */
    test_flag("-s", TEMP_GCDA_FILE, 1, NULL, 0);
    
    /* Test flag combinations */
    printf("\nTesting flag combinations...\n");
    test_flag("-l -p", TEMP_GCDA_FILE, 1, NULL, 0);
    test_flag("-p -l", TEMP_GCDA_FILE, 1, NULL, 0);
    test_flag("-r -s", TEMP_GCDA_FILE, 1, NULL, 0);
    test_flag("-s -r", TEMP_GCDA_FILE, 1, NULL, 0);
    test_flag("-l -p -r -s", TEMP_GCDA_FILE, 1, NULL, 0);
    
    /* Test invalid flag -X */
    printf("\nTesting invalid flag '-X'...\n");
    char invalid_cmd[1024];
    snprintf(invalid_cmd, sizeof(invalid_cmd), "%s -X", INSTRUMENTED_BINARY);
    
    char stderr_output[1024] = {0};
    int exit_code = execute_and_capture(invalid_cmd, stderr_output, 
                                        sizeof(stderr_output), 1, 1);
    
    printf("Command: %s\n", invalid_cmd);
    printf("Exit code: %d\n", exit_code);
    printf("Stderr output: %s", stderr_output);
    
    if (strstr(stderr_output, "unknown flag `X'") != NULL) {
        printf("✓ Found expected error message\n");
    } else {
        printf("✗ Expected error message not found\n");
    }
    
    /* Test invalid flag with file */
    printf("\nTesting invalid flag '-Y' with file...\n");
    snprintf(invalid_cmd, sizeof(invalid_cmd), "%s -Y %s", 
             INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    
    memset(stderr_output, 0, sizeof(stderr_output));
    exit_code = execute_and_capture(invalid_cmd, stderr_output, 
                                    sizeof(stderr_output), 1, 1);
    
    printf("Command: %s\n", invalid_cmd);
    printf("Exit code: %d\n", exit_code);
    printf("Stderr output: %s", stderr_output);
    
    if (strstr(stderr_output, "unknown flag `Y'") != NULL) {
        printf("✓ Found expected error message\n");
    } else {
        printf("✗ Expected error message not found\n");
    }
    
    /* Step 4: Cleanup */
    printf("\n=== Cleaning up ===\n");
    unlink(TEMP_GCDA_FILE);
    unlink(INSTRUMENTED_BINARY);
    
    /* Also remove coverage data files generated by instrumented binary */
    char coverage_files[][64] = {
        INSTRUMENTED_BINARY ".gcda",
        INSTRUMENTED_BINARY ".gcno",
        "gcov-dump.gcda",
        "gcov-dump.gcno"
    };
    
    for (size_t i = 0; i < sizeof(coverage_files)/sizeof(coverage_files[0]); i++) {
        if (unlink(coverage_files[i]) == 0) {
            printf("Removed: %s\n", coverage_files[i]);
        }
    }
    
    printf("\n=== Test completed ===\n");
    printf("All test cases executed. Check coverage with:\n");
    printf("  gcov %s\n", INSTRUMENTED_BINARY);
    
    return 0;
}
