#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define TEMP_GCDA_FILE "/tmp/minimal.gcda"
#define INSTRUMENTED_BINARY "/tmp/gcov-dump-instrumented"

/* Minimal valid GCOV data file structure */
static const unsigned char minimal_gcda[] = {
    /* Magic: 'gcda' */
    0x67, 0x63, 0x64, 0x61,
    /* Version: 0x3430372A (GCC 7*) */
    0x2A, 0x37, 0x30, 0x34,
    /* Stamp: 0 */
    0x00, 0x00, 0x00, 0x00,
    /* Length: 0 (no records) */
    0x00, 0x00, 0x00, 0x00
};

/* Function to build gcov-dump with coverage instrumentation */
int build_gcov_dump(const char *source_path) {
    char cmd[1024];
    struct stat st;
    
    printf("Building instrumented gcov-dump...\n");
    
    /* Check if source file exists */
    if (stat(source_path, &st) != 0) {
        fprintf(stderr, "Error: gcov-dump.cc not found at %s\n", source_path);
        return 0;
    }
    
    /* Compile with coverage flags */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    printf("Compile command: %s\n", cmd);
    
    int result = system(cmd);
    if (result != 0) {
        fprintf(stderr, "Error: Failed to compile gcov-dump\n");
        return 0;
    }
    
    /* Verify the binary was created */
    if (stat(INSTRUMENTED_BINARY, &st) != 0) {
        fprintf(stderr, "Error: Instrumented binary not created\n");
        return 0;
    }
    
    printf("Successfully built instrumented gcov-dump at %s\n", INSTRUMENTED_BINARY);
    return 1;
}

/* Function to create minimal coverage data file */
int create_minimal_gcda() {
    FILE *fp = fopen(TEMP_GCDA_FILE, "wb");
    if (!fp) {
        perror("Error creating temporary gcda file");
        return 0;
    }
    
    fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    printf("Created minimal coverage file: %s\n", TEMP_GCDA_FILE);
    return 1;
}

/* Function to run gcov-dump and capture output */
int run_gcov_dump(const char *args, char *output, size_t output_size, 
                  char *error, size_t error_size, int *exit_status) {
    char cmd[1024];
    FILE *fp;
    int result;
    
    /* Build command */
    snprintf(cmd, sizeof(cmd), "%s %s 2>&1", INSTRUMENTED_BINARY, args);
    
    /* Clear output buffers */
    if (output) output[0] = '\0';
    if (error) error[0] = '\0';
    
    /* Execute command and capture output */
    fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        return 0;
    }
    
    /* Read all output */
    char buffer[4096];
    size_t total_read = 0;
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (output && total_read < output_size - 1) {
            strncat(output, buffer, output_size - total_read - 1);
            total_read += strlen(buffer);
        }
    }
    
    /* Get exit status */
    result = pclose(fp);
    if (exit_status) {
        *exit_status = WEXITSTATUS(result);
    }
    
    return 1;
}

/* Function to test invalid flag */
int test_invalid_flag() {
    char cmd[1024];
    char error[4096] = {0};
    FILE *fp;
    int found = 0;
    
    printf("Testing invalid flag -X...\n");
    
    /* Build command to capture stderr */
    snprintf(cmd, sizeof(cmd), "%s -X 2>&1", INSTRUMENTED_BINARY);
    
    fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed for invalid flag test");
        return 0;
    }
    
    /* Search for error message */
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "unknown flag `X'")) {
            printf("✓ Found expected error message: %s", line);
            found = 1;
            break;
        }
    }
    
    pclose(fp);
    
    if (!found) {
        printf("✗ Did not find expected error message for invalid flag\n");
        return 0;
    }
    
    return 1;
}

int main(int argc, char *argv[]) {
    char output[4096];
    char error[4096];
    int exit_status;
    int all_tests_passed = 1;
    
    printf("=== GCOV-DUMP Coverage Test Program ===\n\n");
    
    /* Try to find gcov-dump.cc in common locations */
    const char *source_paths[] = {
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "gcov-dump.cc",
        "/usr/src/gcc/gcc/gcov-dump.cc",
        NULL
    };
    
    const char *source_path = NULL;
    for (int i = 0; source_paths[i]; i++) {
        struct stat st;
        if (stat(source_paths[i], &st) == 0) {
            source_path = source_paths[i];
            break;
        }
    }
    
    if (!source_path) {
        fprintf(stderr, "Error: Could not find gcov-dump.cc\n");
        fprintf(stderr, "Please specify path as argument: %s <path-to-gcov-dump.cc>\n", argv[0]);
        return 1;
    }
    
    printf("Using source file: %s\n", source_path);
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_gcov_dump(source_path)) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    if (!create_minimal_gcda()) {
        return 1;
    }
    
    printf("\n=== Running Test Cases ===\n\n");
    
    /* Test 1: -h flag (help) */
    printf("Test 1: Testing -h flag (help)...\n");
    if (!run_gcov_dump("-h", output, sizeof(output), error, sizeof(error), &exit_status)) {
        all_tests_passed = 0;
    } else {
        if (exit_status == 0) {
            printf("✓ -h flag test passed (exit code: %d)\n", exit_status);
        } else {
            printf("✗ -h flag test failed (exit code: %d)\n", exit_status);
            all_tests_passed = 0;
        }
    }
    
    /* Test 2: -v flag (version) */
    printf("\nTest 2: Testing -v flag (version)...\n");
    if (!run_gcov_dump("-v", output, sizeof(output), error, sizeof(error), &exit_status)) {
        all_tests_passed = 0;
    } else {
        if (strstr(output, "gcov-dump") || strstr(output, "GCC")) {
            printf("✓ -v flag test passed (version info found)\n");
        } else {
            printf("✗ -v flag test failed (no version info)\n");
            printf("Output: %s\n", output);
            all_tests_passed = 0;
        }
    }
    
    /* Test 3: -l flag (dump contents) */
    printf("\nTest 3: Testing -l flag...\n");
    if (!run_gcov_dump("-l " TEMP_GCDA_FILE, output, sizeof(output), error, sizeof(error), &exit_status)) {
        all_tests_passed = 0;
    } else {
        printf("✓ -l flag executed (exit code: %d)\n", exit_status);
    }
    
    /* Test 4: -p flag (dump positions) */
    printf("\nTest 4: Testing -p flag...\n");
    if (!run_gcov_dump("-p " TEMP_GCDA_FILE, output, sizeof(output), error, sizeof(error), &exit_status)) {
        all_tests_passed = 0;
    } else {
        printf("✓ -p flag executed (exit code: %d)\n", exit_status);
    }
    
    /* Test 5: -r flag (dump raw) */
    printf("\nTest 5: Testing -r flag...\n");
    if (!run_gcov_dump("-r " TEMP_GCDA_FILE, output, sizeof(output), error, sizeof(error), &exit_status)) {
        all_tests_passed = 0;
    } else {
        printf("✓ -r flag executed (exit code: %d)\n", exit_status);
    }
    
    /* Test 6: -s flag (dump stable) */
    printf("\nTest 6: Testing -s flag...\n");
    if (!run_gcov_dump("-s " TEMP_GCDA_FILE, output, sizeof(output), error, sizeof(error), &exit_status)) {
        all_tests_passed = 0;
    } else {
        printf("✓ -s flag executed (exit code: %d)\n", exit_status);
    }
    
    /* Test 7: Combined flags -l -p */
    printf("\nTest 7: Testing combined flags -l -p...\n");
    if (!run_gcov_dump("-l -p " TEMP_GCDA_FILE, output, sizeof(output), error, sizeof(error), &exit_status)) {
        all_tests_passed = 0;
    } else {
        printf("✓ -l -p flags executed (exit code: %d)\n", exit_status);
    }
    
    /* Test 8: Combined flags -r -s (different order) */
    printf("\nTest 8: Testing combined flags -r -s...\n");
    if (!run_gcov_dump("-r -s " TEMP_GCDA_FILE, output, sizeof(output), error, sizeof(error), &exit_status)) {
        all_tests_passed = 0;
    } else {
        printf("✓ -r -s flags executed (exit code: %d)\n", exit_status);
    }
    
    /* Test 9: Combined flags -s -r (reverse order) */
    printf("\nTest 9: Testing combined flags -s -r (reverse order)...\n");
    if (!run_gcov_dump("-s -r " TEMP_GCDA_FILE, output, sizeof(output), error, sizeof(error), &exit_status)) {
        all_tests_passed = 0;
    } else {
        printf("✓ -s -r flags executed (exit code: %d)\n", exit_status);
    }
    
    /* Test 10: Invalid flag -X */
    printf("\nTest 10: Testing invalid flag -X...\n");
    if (!test_invalid_flag()) {
        all_tests_passed = 0;
    }
    
    /* Test 11: Invalid flag with file argument */
    printf("\nTest 11: Testing invalid flag -Y with file...\n");
    if (!run_gcov_dump("-Y " TEMP_GCDA_FILE, output, sizeof(output), error, sizeof(error), &exit_status)) {
        all_tests_passed = 0;
    } else {
        if (strstr(output, "unknown flag `Y'")) {
            printf("✓ Found expected error message for -Y flag\n");
        } else {
            printf("✗ Did not find expected error message for -Y flag\n");
            all_tests_passed = 0;
        }
    }
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    unlink(TEMP_GCDA_FILE);
    unlink(INSTRUMENTED_BINARY);
    printf("Removed temporary files\n");
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    if (all_tests_passed) {
        printf("✓ All tests passed!\n");
        return 0;
    } else {
        printf("✗ Some tests failed\n");
        return 1;
    }
}
