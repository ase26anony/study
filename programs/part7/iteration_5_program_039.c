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
    /* Magic: 'gcda' */
    0x67, 0x63, 0x64, 0x61,
    /* Version: 0x76312e2a (example version) */
    0x2a, 0x2e, 0x31, 0x76,
    /* Stamp: 0x12345678 */
    0x78, 0x56, 0x34, 0x12,
    /* Record type 0 (GCOV_TAG_FUNCTION) with length 2 */
    0x00, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00,
    /* Function info: ident=1, lineno_checksum=0, cfg_checksum=0 */
    0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    /* Record type 1 (GCOV_TAG_COUNTER_BASE) with length 0 */
    0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    /* EOF marker */
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

/* Create a minimal valid .gcda file */
static int create_minimal_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create minimal .gcda file");
        return 0;
    }
    
    fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    printf("Created minimal .gcda file: %s (%zu bytes)\n", 
           filename, sizeof(minimal_gcda));
    return 1;
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
        perror("popen failed");
        return -1;
    }
    
    if (output && output_size > 0) {
        size_t bytes_read = fread(output, 1, output_size - 1, fp);
        output[bytes_read] = '\0';
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
    
    printf("Building instrumented gcov-dump from: %s\n", source_path);
    
    /* Check if source file exists */
    struct stat st;
    if (stat(source_path, &st) != 0) {
        fprintf(stderr, "Source file not found: %s\n", source_path);
        return 0;
    }
    
    /* Compile with coverage instrumentation */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov "
             "-o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    printf("Compilation command: %s\n", cmd);
    
    int result = system(cmd);
    if (result != 0) {
        fprintf(stderr, "Failed to compile instrumented gcov-dump\n");
        return 0;
    }
    
    /* Verify the binary was created */
    if (stat(INSTRUMENTED_BINARY, &st) != 0) {
        fprintf(stderr, "Instrumented binary not created: %s\n", INSTRUMENTED_BINARY);
        return 0;
    }
    
    printf("Successfully built instrumented gcov-dump: %s\n", INSTRUMENTED_BINARY);
    return 1;
}

int main(int argc, char *argv[]) {
    char output[4096];
    int exit_code;
    int all_tests_passed = 1;
    
    /* Try to locate gcov-dump.cc */
    const char *source_paths[] = {
        "gcov-dump.cc",
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
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
        fprintf(stderr, "Could not find gcov-dump.cc source file\n");
        fprintf(stderr, "Please specify path as argument: %s <path-to-gcov-dump.cc>\n", argv[0]);
        if (argc > 1) {
            source_path = argv[1];
        } else {
            return 1;
        }
    }
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_instrumented_gcov_dump(source_path)) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    if (!create_minimal_gcda(TEMP_GCDA_FILE)) {
        unlink(INSTRUMENTED_BINARY);
        return 1;
    }
    
    printf("\n=== Starting gcov-dump flag coverage tests ===\n\n");
    
    /* Test 1: -h flag (help) */
    printf("Testing -h flag (help)...\n");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s -h", INSTRUMENTED_BINARY);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    
    if (exit_code == 0) {
        printf("✓ -h flag test passed (exit code: %d)\n", exit_code);
    } else {
        printf("✗ -h flag test failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    printf("\n");
    
    /* Test 2: -v flag (version) */
    printf("Testing -v flag (version)...\n");
    snprintf(cmd, sizeof(cmd), "%s -v", INSTRUMENTED_BINARY);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    
    if (exit_code == 0 && contains_string(output, "gcov-dump") && 
        contains_string(output, "version")) {
        printf("✓ -v flag test passed\n");
        printf("  Output: %s\n", output);
    } else {
        printf("✗ -v flag test failed (exit code: %d)\n", exit_code);
        printf("  Output: %s\n", output);
        all_tests_passed = 0;
    }
    printf("\n");
    
    /* Test 3: -l flag (dump contents) */
    printf("Testing -l flag (dump contents)...\n");
    snprintf(cmd, sizeof(cmd), "%s -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    
    if (exit_code == 0) {
        printf("✓ -l flag test passed (exit code: %d)\n", exit_code);
    } else {
        printf("✗ -l flag test failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    printf("\n");
    
    /* Test 4: -p flag (dump positions) */
    printf("Testing -p flag (dump positions)...\n");
    snprintf(cmd, sizeof(cmd), "%s -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    
    if (exit_code == 0) {
        printf("✓ -p flag test passed (exit code: %d)\n", exit_code);
    } else {
        printf("✗ -p flag test failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    printf("\n");
    
    /* Test 5: -r flag (dump raw) */
    printf("Testing -r flag (dump raw)...\n");
    snprintf(cmd, sizeof(cmd), "%s -r %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    
    if (exit_code == 0) {
        printf("✓ -r flag test passed (exit code: %d)\n", exit_code);
    } else {
        printf("✗ -r flag test failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    printf("\n");
    
    /* Test 6: -s flag (dump stable) */
    printf("Testing -s flag (dump stable)...\n");
    snprintf(cmd, sizeof(cmd), "%s -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    
    if (exit_code == 0) {
        printf("✓ -s flag test passed (exit code: %d)\n", exit_code);
    } else {
        printf("✗ -s flag test failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    printf("\n");
    
    /* Test 7: Combined flags -l -p */
    printf("Testing combined flags -l -p...\n");
    snprintf(cmd, sizeof(cmd), "%s -l -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    
    if (exit_code == 0) {
        printf("✓ Combined -l -p test passed (exit code: %d)\n", exit_code);
    } else {
        printf("✗ Combined -l -p test failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    printf("\n");
    
    /* Test 8: Combined flags -r -s (different order) */
    printf("Testing combined flags -r -s (different order)...\n");
    snprintf(cmd, sizeof(cmd), "%s -r -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    
    if (exit_code == 0) {
        printf("✓ Combined -r -s test passed (exit code: %d)\n", exit_code);
    } else {
        printf("✗ Combined -r -s test failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    printf("\n");
    
    /* Test 9: Invalid flag -X */
    printf("Testing invalid flag -X...\n");
    snprintf(cmd, sizeof(cmd), "%s -X %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1);
    
    if (contains_string(output, "unknown flag `X'")) {
        printf("✓ Invalid flag test passed - found expected error message\n");
        printf("  Error output: %s\n", output);
    } else {
        printf("✗ Invalid flag test failed - expected 'unknown flag `X'' not found\n");
        printf("  Output: %s\n", output);
        all_tests_passed = 0;
    }
    printf("\n");
    
    /* Test 10: Another invalid flag -Z */
    printf("Testing invalid flag -Z...\n");
    snprintf(cmd, sizeof(cmd), "%s -Z", INSTRUMENTED_BINARY);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1);
    
    if (contains_string(output, "unknown flag `Z'")) {
        printf("✓ Invalid flag -Z test passed - found expected error message\n");
        printf("  Error output: %s\n", output);
    } else {
        printf("✗ Invalid flag -Z test failed\n");
        printf("  Output: %s\n", output);
        all_tests_passed = 0;
    }
    printf("\n");
    
    /* Cleanup */
    printf("Cleaning up temporary files...\n");
    unlink(TEMP_GCDA_FILE);
    unlink(INSTRUMENTED_BINARY);
    
    /* Clean up coverage data files from instrumented binary */
    char coverage_files[][64] = {
        INSTRUMENTED_BINARY ".gcda",
        INSTRUMENTED_BINARY ".gcno",
        "gcov-dump-instrumented.gcda",
        "gcov-dump-instrumented.gcno"
    };
    
    for (size_t i = 0; i < sizeof(coverage_files)/sizeof(coverage_files[0]); i++) {
        if (unlink(coverage_files[i]) == 0) {
            printf("Removed: %s\n", coverage_files[i]);
        }
    }
    
    printf("\n=== Test Summary ===\n");
    if (all_tests_passed) {
        printf("✓ All tests passed!\n");
        printf("The uncovered lines in gcov-dump.cc (111-130) should now be covered.\n");
        printf("Run 'gcov gcov-dump.cc' to verify coverage.\n");
    } else {
        printf("✗ Some tests failed\n");
    }
    
    return all_tests_passed ? 0 : 1;
}
