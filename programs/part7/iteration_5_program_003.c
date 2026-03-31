/**
 * Test driver for gcov-dump.cc uncovered lines (111-130)
 * Compile with: gcc -O0 -g -Wno-deprecated-declarations -o test_gcov_dump test_gcov_dump.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_GCDA_FILE "/tmp/minimal.gcda"
#define INSTRUMENTED_BINARY "/tmp/gcov-dump-instrumented"

/**
 * Create a minimal valid .gcda file
 * GCOV data format: magic (0x67636461), version (0x*), stamp, length
 */
int create_minimal_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create minimal .gcda file");
        return 0;
    }
    
    /* GCOV magic number for .gcda files */
    unsigned magic = 0x67636461; /* 'gcda' in little-endian */
    
    /* GCOV version - use a known version (407*) */
    unsigned version = 0x3430372a; /* '407*' */
    
    /* Time stamp */
    unsigned stamp = 0x12345678;
    
    /* Write file header */
    fwrite(&magic, sizeof(unsigned), 1, fp);
    fwrite(&version, sizeof(unsigned), 1, fp);
    fwrite(&stamp, sizeof(unsigned), 1, fp);
    
    /* Write a zero-length record to terminate the file */
    unsigned zero_record = 0;
    fwrite(&zero_record, sizeof(unsigned), 1, fp);
    
    fclose(fp);
    return 1;
}

/**
 * Execute command and capture output
 */
int execute_and_capture(const char *cmd, char *output, size_t output_size, 
                        int capture_stdout, int capture_stderr) {
    char full_cmd[1024];
    if (capture_stderr) {
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    } else {
        snprintf(full_cmd, sizeof(full_cmd), "%s", cmd);
    }
    
    FILE *fp = popen(full_cmd, "r");
    if (!fp) {
        return -1;
    }
    
    if (output && output_size > 0) {
        output[0] = '\0';
        size_t total_read = 0;
        while (fgets(output + total_read, output_size - total_read, fp)) {
            total_read = strlen(output);
            if (total_read >= output_size - 1) break;
        }
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
}

/**
 * Check if string contains substring
 */
int contains_string(const char *str, const char *substr) {
    return strstr(str, substr) != NULL;
}

/**
 * Build instrumented gcov-dump binary
 */
int build_instrumented_gcov_dump() {
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
        /* Try to find it using find command */
        FILE *fp = popen("find . -name 'gcov-dump.cc' -type f 2>/dev/null | head -1", "r");
        char found_path[1024];
        if (fp && fgets(found_path, sizeof(found_path), fp)) {
            /* Remove newline */
            found_path[strcspn(found_path, "\n")] = 0;
            if (stat(found_path, &st) == 0) {
                source_path = strdup(found_path);
            }
        }
        if (fp) pclose(fp);
    }
    
    if (!source_path) {
        printf("ERROR: Could not find gcov-dump.cc source file\n");
        printf("Please run this test from GCC source/build directory\n");
        return 0;
    }
    
    printf("Found gcov-dump.cc at: %s\n", source_path);
    
    /* Build command */
    char build_cmd[2048];
    snprintf(build_cmd, sizeof(build_cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    printf("Executing: %s\n", build_cmd);
    
    int result = system(build_cmd);
    if (result != 0) {
        printf("ERROR: Failed to build instrumented gcov-dump\n");
        return 0;
    }
    
    /* Verify the binary was created */
    if (stat(INSTRUMENTED_BINARY, &st) != 0) {
        printf("ERROR: Instrumented binary not created\n");
        return 0;
    }
    
    printf("Successfully built instrumented gcov-dump\n");
    return 1;
}

int main() {
    printf("=== Test Driver for gcov-dump.cc uncovered lines (111-130) ===\n\n");
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_instrumented_gcov_dump()) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    printf("\nCreating minimal .gcda file...\n");
    if (!create_minimal_gcda(TEMP_GCDA_FILE)) {
        unlink(INSTRUMENTED_BINARY);
        return 1;
    }
    printf("Created: %s\n", TEMP_GCDA_FILE);
    
    /* Step 3: Execute test sequence */
    printf("\n=== Starting test sequence ===\n\n");
    
    int all_tests_passed = 1;
    char output[4096];
    
    /* Test 1: -h flag (help) */
    printf("Test 1: Testing -h flag (help)...\n");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s -h", INSTRUMENTED_BINARY);
    int exit_code = execute_and_capture(cmd, output, sizeof(output), 1, 0);
    if (exit_code == 0) {
        printf("✓ -h flag test passed (exit code: %d)\n", exit_code);
    } else {
        printf("✗ -h flag test failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test 2: -v flag (version) */
    printf("\nTest 2: Testing -v flag (version)...\n");
    snprintf(cmd, sizeof(cmd), "%s -v", INSTRUMENTED_BINARY);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1, 0);
    if (exit_code == 0 && contains_string(output, "gcov-dump")) {
        printf("✓ -v flag test passed\n");
        printf("  Output: %s", output);
    } else {
        printf("✗ -v flag test failed\n");
        printf("  Output: %s", output);
        all_tests_passed = 0;
    }
    
    /* Test 3: -l flag (dump contents) */
    printf("\nTest 3: Testing -l flag (dump contents)...\n");
    snprintf(cmd, sizeof(cmd), "%s -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1, 1);
    if (exit_code == 0) {
        printf("✓ -l flag test passed\n");
    } else {
        printf("✗ -l flag test failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test 4: -p flag (dump positions) */
    printf("\nTest 4: Testing -p flag (dump positions)...\n");
    snprintf(cmd, sizeof(cmd), "%s -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1, 1);
    if (exit_code == 0) {
        printf("✓ -p flag test passed\n");
    } else {
        printf("✗ -p flag test failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test 5: -r flag (dump raw) */
    printf("\nTest 5: Testing -r flag (dump raw)...\n");
    snprintf(cmd, sizeof(cmd), "%s -r %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1, 1);
    if (exit_code == 0) {
        printf("✓ -r flag test passed\n");
    } else {
        printf("✗ -r flag test failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test 6: -s flag (dump stable) */
    printf("\nTest 6: Testing -s flag (dump stable)...\n");
    snprintf(cmd, sizeof(cmd), "%s -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1, 1);
    if (exit_code == 0) {
        printf("✓ -s flag test passed\n");
    } else {
        printf("✗ -s flag test failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test 7: Combined flags -l -p */
    printf("\nTest 7: Testing combined flags -l -p...\n");
    snprintf(cmd, sizeof(cmd), "%s -l -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1, 1);
    if (exit_code == 0) {
        printf("✓ Combined -l -p flags test passed\n");
    } else {
        printf("✗ Combined -l -p flags test failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test 8: Combined flags -r -s (different order) */
    printf("\nTest 8: Testing combined flags -r -s (different order)...\n");
    snprintf(cmd, sizeof(cmd), "%s -r -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1, 1);
    if (exit_code == 0) {
        printf("✓ Combined -r -s flags test passed\n");
    } else {
        printf("✗ Combined -r -s flags test failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test 9: Invalid flag -X */
    printf("\nTest 9: Testing invalid flag -X...\n");
    snprintf(cmd, sizeof(cmd), "%s -X %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0, 1);
    
    /* Check for the exact error message from the uncovered lines */
    if (contains_string(output, "unknown flag") && contains_string(output, "X")) {
        printf("✓ Invalid flag test passed - found expected error message\n");
        printf("  Error output: %s", output);
    } else {
        printf("✗ Invalid flag test failed\n");
        printf("  Expected: 'unknown flag `X''\n");
        printf("  Got: %s", output);
        all_tests_passed = 0;
    }
    
    /* Test 10: Invalid flag without file argument */
    printf("\nTest 10: Testing invalid flag -Y (no file argument)...\n");
    snprintf(cmd, sizeof(cmd), "%s -Y", INSTRUMENTED_BINARY);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0, 1);
    
    if (contains_string(output, "unknown flag") && contains_string(output, "Y")) {
        printf("✓ Invalid flag (no file) test passed\n");
        printf("  Error output: %s", output);
    } else {
        printf("✗ Invalid flag (no file) test failed\n");
        printf("  Got: %s", output);
        all_tests_passed = 0;
    }
    
    /* Step 4: Cleanup */
    printf("\n=== Cleaning up ===\n");
    unlink(TEMP_GCDA_FILE);
    unlink(INSTRUMENTED_BINARY);
    
    /* Also clean up coverage data files created by instrumented binary */
    char coverage_files[1024];
    snprintf(coverage_files, sizeof(coverage_files), 
             "rm -f %s.gcno %s.gcda 2>/dev/null", INSTRUMENTED_BINARY, INSTRUMENTED_BINARY);
    system(coverage_files);
    
    printf("\n=== Test Summary ===\n");
    if (all_tests_passed) {
        printf("✓ All tests passed!\n");
        printf("\nThe uncovered lines in gcov-dump.cc (111-130) should now be covered.\n");
        printf("Run 'gcov gcov-dump.cc' on the instrumented binary to verify coverage.\n");
        return 0;
    } else {
        printf("✗ Some tests failed\n");
        return 1;
    }
}
