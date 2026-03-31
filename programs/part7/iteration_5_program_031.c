#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define TEMP_GCDA_FILE "/tmp/minimal.gcda"
#define INSTRUMENTED_BINARY "/tmp/gcov-dump-instrumented"

/* Minimal GCOV data file structure */
static const unsigned char minimal_gcda[] = {
    /* Magic number: 'gcda' */
    0x67, 0x63, 0x64, 0x61,
    /* Version: 0x3430372a (gcov 4.7 format) */
    0x2a, 0x37, 0x30, 0x34,
    /* Stamp (timestamp) - zero */
    0x00, 0x00, 0x00, 0x00,
    /* Length of next record: 0 (empty function record) */
    0x00, 0x00, 0x00, 0x00,
    /* Record tag: GCOV_TAG_FUNCTION (0x01000000) */
    0x00, 0x00, 0x00, 0x01,
    /* Record length: 2 (ident + checksum) */
    0x02, 0x00, 0x00, 0x00,
    /* Function ident: 0 */
    0x00, 0x00, 0x00, 0x00,
    /* Function checksum: 0 */
    0x00, 0x00, 0x00, 0x00,
    /* Length of next record: 0 (end marker) */
    0x00, 0x00, 0x00, 0x00
};

/* Build instrumented gcov-dump binary */
int build_instrumented_gcov_dump() {
    printf("Building instrumented gcov-dump...\n");
    
    /* Try to find gcov-dump.cc in common locations */
    const char* possible_paths[] = {
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "../../../gcc/gcov-dump.cc",
        "gcc/gcov-dump.cc",
        NULL
    };
    
    const char* source_path = NULL;
    struct stat st;
    
    for (int i = 0; possible_paths[i] != NULL; i++) {
        if (stat(possible_paths[i], &st) == 0) {
            source_path = possible_paths[i];
            break;
        }
    }
    
    if (source_path == NULL) {
        fprintf(stderr, "ERROR: Could not find gcov-dump.cc\n");
        return 0;
    }
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    printf("Compiling with: %s\n", cmd);
    
    int result = system(cmd);
    if (result != 0) {
        fprintf(stderr, "ERROR: Failed to compile gcov-dump\n");
        return 0;
    }
    
    if (stat(INSTRUMENTED_BINARY, &st) != 0) {
        fprintf(stderr, "ERROR: Instrumented binary not created\n");
        return 0;
    }
    
    printf("Successfully built instrumented gcov-dump at %s\n", INSTRUMENTED_BINARY);
    return 1;
}

/* Create minimal valid .gcda file */
int create_minimal_gcda() {
    printf("Creating minimal .gcda file...\n");
    
    FILE* fp = fopen(TEMP_GCDA_FILE, "wb");
    if (!fp) {
        perror("Failed to create temporary .gcda file");
        return 0;
    }
    
    size_t written = fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    if (written != sizeof(minimal_gcda)) {
        fprintf(stderr, "ERROR: Failed to write complete .gcda file\n");
        return 0;
    }
    
    printf("Created minimal .gcda file at %s\n", TEMP_GCDA_FILE);
    return 1;
}

/* Run command and capture output */
int run_command(const char* cmd, char* output, size_t output_size, int capture_stderr) {
    char full_cmd[1024];
    if (capture_stderr) {
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    } else {
        snprintf(full_cmd, sizeof(full_cmd), "%s", cmd);
    }
    
    FILE* fp = popen(full_cmd, "r");
    if (!fp) {
        perror("popen failed");
        return -1;
    }
    
    if (output && output_size > 0) {
        output[0] = '\0';
        size_t total_read = 0;
        while (fgets(output + total_read, output_size - total_read, fp) != NULL) {
            total_read = strlen(output);
            if (total_read >= output_size - 1) break;
        }
    } else {
        /* Just consume output */
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {}
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
}

/* Test specific flag */
void test_flag(const char* flag, const char* file_arg, int expect_success, 
               const char* expected_output, int capture_stderr) {
    printf("\nTesting flag: %s\n", flag);
    
    char cmd[1024];
    if (file_arg) {
        snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flag, file_arg);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s", INSTRUMENTED_BINARY, flag);
    }
    
    char output[4096] = {0};
    int exit_code = run_command(cmd, output, sizeof(output), capture_stderr);
    
    printf("Command: %s\n", cmd);
    printf("Exit code: %d\n", exit_code);
    
    if (expected_output) {
        if (strstr(output, expected_output) != NULL) {
            printf("✓ Found expected output: '%s'\n", expected_output);
        } else {
            printf("✗ Did not find expected output: '%s'\n", expected_output);
            printf("Actual output:\n%s\n", output);
        }
    }
    
    if (expect_success) {
        if (exit_code == 0) {
            printf("✓ Exit code as expected (0)\n");
        } else {
            printf("✗ Unexpected exit code: %d\n", exit_code);
        }
    } else {
        if (exit_code != 0) {
            printf("✓ Non-zero exit code as expected\n");
        } else {
            printf("✗ Expected non-zero exit code but got 0\n");
        }
    }
}

/* Test invalid flag */
void test_invalid_flag() {
    printf("\nTesting invalid flag: -X\n");
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s -X %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    
    char output[4096] = {0};
    int exit_code = run_command(cmd, output, sizeof(output), 1);
    
    printf("Command: %s\n", cmd);
    printf("Exit code: %d\n", exit_code);
    
    /* Check for the exact error message from uncovered lines */
    if (strstr(output, "unknown flag `X'") != NULL) {
        printf("✓ Found expected error message: 'unknown flag `X''\n");
    } else {
        printf("✗ Did not find expected error message\n");
        printf("Actual output:\n%s\n", output);
    }
    
    if (exit_code != 0) {
        printf("✓ Non-zero exit code as expected\n");
    } else {
        printf("✗ Expected non-zero exit code but got 0\n");
    }
}

/* Clean up temporary files */
void cleanup() {
    printf("\nCleaning up temporary files...\n");
    
    if (remove(TEMP_GCDA_FILE) == 0) {
        printf("Removed %s\n", TEMP_GCDA_FILE);
    }
    
    if (remove(INSTRUMENTED_BINARY) == 0) {
        printf("Removed %s\n", INSTRUMENTED_BINARY);
    }
    
    /* Also clean up coverage data files generated by instrumented binary */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "rm -f %s.gcno %s.gcda", INSTRUMENTED_BINARY, INSTRUMENTED_BINARY);
    system(cmd);
}

int main() {
    printf("=== GCOV-Dump Coverage Test Program ===\n");
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_instrumented_gcov_dump()) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    if (!create_minimal_gcda()) {
        return 1;
    }
    
    /* Step 3: Execute test sequence */
    
    /* Test -h flag (help) - no file argument needed */
    test_flag("-h", NULL, 1, "Usage:", 0);
    
    /* Test -v flag (version) - no file argument needed */
    test_flag("-v", NULL, 1, "gcov-dump", 0);
    
    /* Test flags that require a coverage file */
    test_flag("-l", TEMP_GCDA_FILE, 1, NULL, 0);
    test_flag("-p", TEMP_GCDA_FILE, 1, NULL, 0);
    test_flag("-r", TEMP_GCDA_FILE, 1, NULL, 0);
    test_flag("-s", TEMP_GCDA_FILE, 1, NULL, 0);
    
    /* Test flag combinations */
    test_flag("-l -p", TEMP_GCDA_FILE, 1, NULL, 0);
    test_flag("-p -l", TEMP_GCDA_FILE, 1, NULL, 0);
    test_flag("-r -s", TEMP_GCDA_FILE, 1, NULL, 0);
    test_flag("-s -r", TEMP_GCDA_FILE, 1, NULL, 0);
    
    /* Test invalid flag */
    test_invalid_flag();
    
    /* Additional test: multiple valid flags combined */
    printf("\nTesting multiple flags combined: -l -p -r -s\n");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s -l -p -r -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    int exit_code = run_command(cmd, NULL, 0, 0);
    printf("Exit code for -l -p -r -s: %d\n", exit_code);
    
    /* Step 4: Cleanup */
    cleanup();
    
    printf("\n=== Test Complete ===\n");
    printf("All test cases executed. Check gcov output for coverage of lines 111-130.\n");
    printf("Generated coverage data files:\n");
    printf("  - %s.gcno (notes file)\n", INSTRUMENTED_BINARY);
    printf("  - %s.gcda (data file)\n", INSTRUMENTED_BINARY);
    printf("\nTo generate coverage report:\n");
    printf("  gcov %s.gcda\n", INSTRUMENTED_BINARY);
    printf("  lcov --capture --directory . --output-file coverage.info\n");
    printf("  genhtml coverage.info --output-directory coverage_report\n");
    
    return 0;
}
