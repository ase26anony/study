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

/* Create a minimal valid .gcda file */
static int create_minimal_gcda(const char *filename) {
    FILE *f = fopen(filename, "wb");
    if (!f) {
        perror("fopen");
        return 0;
    }
    
    size_t written = fwrite(minimal_gcda, 1, sizeof(minimal_gcda), f);
    fclose(f);
    
    return written == sizeof(minimal_gcda);
}

/* Execute command and capture output */
static int execute_and_capture(const char *cmd, char *output, size_t output_size, 
                               int capture_stderr) {
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

/* Check if string contains substring */
static int contains_string(const char *str, const char *substr) {
    return strstr(str, substr) != NULL;
}

/* Build instrumented gcov-dump */
static int build_instrumented_gcov_dump(const char *source_path) {
    char cmd[2048];
    
    /* Check if source file exists */
    struct stat st;
    if (stat(source_path, &st) != 0) {
        fprintf(stderr, "Source file not found: %s\n", source_path);
        return 0;
    }
    
    /* Compile with coverage instrumentation */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    printf("Building instrumented gcov-dump: %s\n", cmd);
    
    int result = system(cmd);
    if (result != 0) {
        fprintf(stderr, "Failed to compile gcov-dump\n");
        return 0;
    }
    
    /* Verify the binary was created */
    if (stat(INSTRUMENTED_BINARY, &st) != 0) {
        fprintf(stderr, "Instrumented binary not created\n");
        return 0;
    }
    
    return 1;
}

int main(int argc, char *argv[]) {
    char output[4096];
    int exit_code;
    int all_tests_passed = 1;
    
    /* Determine gcov-dump source path */
    const char *gcov_dump_source = "gcov-dump.cc";
    if (argc > 1) {
        gcov_dump_source = argv[1];
    }
    
    /* Step 1: Build instrumented gcov-dump */
    printf("=== Step 1: Building instrumented gcov-dump ===\n");
    if (!build_instrumented_gcov_dump(gcov_dump_source)) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    printf("\n=== Step 2: Creating minimal coverage file ===\n");
    if (!create_minimal_gcda(TEMP_GCDA_FILE)) {
        fprintf(stderr, "Failed to create minimal .gcda file\n");
        unlink(INSTRUMENTED_BINARY);
        return 1;
    }
    printf("Created minimal coverage file: %s\n", TEMP_GCDA_FILE);
    
    /* Step 3: Execute test sequence */
    printf("\n=== Step 3: Executing test sequence ===\n");
    
    /* Test -h flag (help) */
    printf("\nTesting -h flag...\n");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s -h", INSTRUMENTED_BINARY);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("✓ -h flag test passed (exit code: %d)\n", exit_code);
    } else {
        printf("✗ -h flag test failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test -v flag (version) */
    printf("\nTesting -v flag...\n");
    snprintf(cmd, sizeof(cmd), "%s -v", INSTRUMENTED_BINARY);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_code == 0 && contains_string(output, "gcov-dump")) {
        printf("✓ -v flag test passed\n");
        printf("  Output: %s", output);
    } else {
        printf("✗ -v flag test failed (exit code: %d)\n", exit_code);
        printf("  Output: %s", output);
        all_tests_passed = 0;
    }
    
    /* Test -l flag (dump contents) */
    printf("\nTesting -l flag...\n");
    snprintf(cmd, sizeof(cmd), "%s -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("✓ -l flag test passed\n");
    } else {
        printf("✗ -l flag test failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test -p flag (dump positions) */
    printf("\nTesting -p flag...\n");
    snprintf(cmd, sizeof(cmd), "%s -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("✓ -p flag test passed\n");
    } else {
        printf("✗ -p flag test failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test -r flag (dump raw) */
    printf("\nTesting -r flag...\n");
    snprintf(cmd, sizeof(cmd), "%s -r %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("✓ -r flag test passed\n");
    } else {
        printf("✗ -r flag test failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test -s flag (dump stable) */
    printf("\nTesting -s flag...\n");
    snprintf(cmd, sizeof(cmd), "%s -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("✓ -s flag test passed\n");
    } else {
        printf("✗ -s flag test failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test flag combinations */
    printf("\nTesting flag combination -l -p...\n");
    snprintf(cmd, sizeof(cmd), "%s -l -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("✓ -l -p combination test passed\n");
    } else {
        printf("✗ -l -p combination test failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    printf("\nTesting flag combination -r -s...\n");
    snprintf(cmd, sizeof(cmd), "%s -r -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("✓ -r -s combination test passed\n");
    } else {
        printf("✗ -r -s combination test failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test different flag ordering */
    printf("\nTesting flag ordering -p -l...\n");
    snprintf(cmd, sizeof(cmd), "%s -p -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("✓ -p -l ordering test passed\n");
    } else {
        printf("✗ -p -l ordering test failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test invalid flag -X */
    printf("\nTesting invalid flag -X...\n");
    snprintf(cmd, sizeof(cmd), "%s -X %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1);
    
    /* Check for the exact error message from uncovered lines */
    if (contains_string(output, "unknown flag `X'")) {
        printf("✓ Invalid flag test passed - found expected error message\n");
        printf("  Error output: %s", output);
    } else {
        printf("✗ Invalid flag test failed - missing expected error message\n");
        printf("  Output: %s", output);
        all_tests_passed = 0;
    }
    
    /* Test invalid flag without file argument */
    printf("\nTesting invalid flag -Y (no file)...\n");
    snprintf(cmd, sizeof(cmd), "%s -Y", INSTRUMENTED_BINARY);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1);
    
    if (contains_string(output, "unknown flag `Y'")) {
        printf("✓ Invalid flag (no file) test passed\n");
        printf("  Error output: %s", output);
    } else {
        printf("✗ Invalid flag (no file) test failed\n");
        printf("  Output: %s", output);
        all_tests_passed = 0;
    }
    
    /* Step 4: Cleanup */
    printf("\n=== Step 4: Cleanup ===\n");
    unlink(TEMP_GCDA_FILE);
    unlink(INSTRUMENTED_BINARY);
    
    /* Also clean up coverage files generated by the instrumented binary */
    char coverage_files[][64] = {
        "gcov-dump-instrumented.gcda",
        "gcov-dump-instrumented.gcno",
        "gcov-dump.gcda",
        "gcov-dump.gcno"
    };
    
    for (size_t i = 0; i < sizeof(coverage_files)/sizeof(coverage_files[0]); i++) {
        if (unlink(coverage_files[i]) == 0) {
            printf("Removed coverage file: %s\n", coverage_files[i]);
        }
    }
    
    printf("\n=== Test Summary ===\n");
    if (all_tests_passed) {
        printf("✓ All tests passed!\n");
        return 0;
    } else {
        printf("✗ Some tests failed\n");
        return 1;
    }
}
