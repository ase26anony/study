#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_GCDA_FILE "/tmp/minimal.gcda"
#define INSTRUMENTED_BINARY "/tmp/gcov-dump-instrumented"

/* Minimal GCOV data file format */
static const unsigned char minimal_gcda[] = {
    /* Magic: 'gcda' */
    0x67, 0x63, 0x64, 0x61,
    /* Version: 0x7630302A (gcov 13 format) */
    0x2A, 0x30, 0x30, 0x76,
    /* Stamp */
    0x00, 0x00, 0x00, 0x00,
    /* Length of first record (0 = EOF marker) */
    0x00, 0x00, 0x00, 0x00
};

/* Create a minimal valid .gcda file */
static int create_minimal_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("fopen");
        return -1;
    }
    
    size_t written = fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    if (written != sizeof(minimal_gcda)) {
        fprintf(stderr, "Failed to write minimal .gcda file\n");
        return -1;
    }
    
    printf("Created minimal .gcda file: %s (%zu bytes)\n", 
           filename, sizeof(minimal_gcda));
    return 0;
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
        
        /* Try to find it in common locations */
        const char *possible_paths[] = {
            "../gcc/gcov-dump.cc",
            "../../gcc/gcov-dump.cc",
            "../../../gcc/gcov-dump.cc",
            "/usr/src/gcc/gcc/gcov-dump.cc",
            NULL
        };
        
        for (int i = 0; possible_paths[i]; i++) {
            if (stat(possible_paths[i], &st) == 0) {
                source_path = possible_paths[i];
                printf("Found source at: %s\n", source_path);
                break;
            }
        }
        
        if (stat(source_path, &st) != 0) {
            fprintf(stderr, "Could not find gcov-dump.cc\n");
            return -1;
        }
    }
    
    /* Compile with coverage instrumentation */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    printf("Compiling: %s\n", cmd);
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Compilation failed with code %d\n", result);
        return -1;
    }
    
    /* Verify the binary was created */
    if (stat(INSTRUMENTED_BINARY, &st) != 0) {
        fprintf(stderr, "Binary not created: %s\n", INSTRUMENTED_BINARY);
        return -1;
    }
    
    printf("Successfully built instrumented binary: %s\n", INSTRUMENTED_BINARY);
    return 0;
}

int main(int argc, char *argv[]) {
    char output[4096];
    int exit_code;
    int tests_passed = 0;
    int total_tests = 0;
    
    printf("=== Starting gcov-dump coverage tests ===\n\n");
    
    /* Step 1: Build instrumented gcov-dump */
    const char *source_path = "gcov-dump.cc";
    if (argc > 1) {
        source_path = argv[1];
    }
    
    if (build_instrumented_gcov_dump(source_path) != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    if (create_minimal_gcda(TEMP_GCDA_FILE) != 0) {
        fprintf(stderr, "Failed to create minimal .gcda file\n");
        unlink(INSTRUMENTED_BINARY);
        return 1;
    }
    
    printf("\n=== Testing command-line flag parsing ===\n\n");
    
    /* Test 1: -h flag (help) */
    printf("Test 1: Testing -h flag (help)...\n");
    total_tests++;
    exit_code = execute_and_capture(INSTRUMENTED_BINARY " -h", output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("✓ -h flag passed (exit code: %d)\n", exit_code);
        tests_passed++;
    } else {
        printf("✗ -h flag failed (exit code: %d)\n", exit_code);
    }
    
    /* Test 2: -v flag (version) */
    printf("\nTest 2: Testing -v flag (version)...\n");
    total_tests++;
    exit_code = execute_and_capture(INSTRUMENTED_BINARY " -v", output, sizeof(output), 0);
    if (exit_code == 0 && contains_string(output, "gcov-dump") && 
        contains_string(output, "version")) {
        printf("✓ -v flag passed. Output contains version info\n");
        tests_passed++;
    } else {
        printf("✗ -v flag failed (exit code: %d)\n", exit_code);
        printf("Output: %s\n", output);
    }
    
    printf("\n=== Testing flags requiring coverage files ===\n\n");
    
    /* Test 3: -l flag (dump contents) */
    printf("Test 3: Testing -l flag...\n");
    total_tests++;
    exit_code = execute_and_capture(INSTRUMENTED_BINARY " -l " TEMP_GCDA_FILE, 
                                   output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("✓ -l flag passed (exit code: %d)\n", exit_code);
        tests_passed++;
    } else {
        printf("✗ -l flag failed (exit code: %d)\n", exit_code);
    }
    
    /* Test 4: -p flag (dump positions) */
    printf("\nTest 4: Testing -p flag...\n");
    total_tests++;
    exit_code = execute_and_capture(INSTRUMENTED_BINARY " -p " TEMP_GCDA_FILE, 
                                   output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("✓ -p flag passed (exit code: %d)\n", exit_code);
        tests_passed++;
    } else {
        printf("✗ -p flag failed (exit code: %d)\n", exit_code);
    }
    
    /* Test 5: -r flag (dump raw) */
    printf("\nTest 5: Testing -r flag...\n");
    total_tests++;
    exit_code = execute_and_capture(INSTRUMENTED_BINARY " -r " TEMP_GCDA_FILE, 
                                   output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("✓ -r flag passed (exit code: %d)\n", exit_code);
        tests_passed++;
    } else {
        printf("✗ -r flag failed (exit code: %d)\n", exit_code);
    }
    
    /* Test 6: -s flag (dump stable) */
    printf("\nTest 6: Testing -s flag...\n");
    total_tests++;
    exit_code = execute_and_capture(INSTRUMENTED_BINARY " -s " TEMP_GCDA_FILE, 
                                   output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("✓ -s flag passed (exit code: %d)\n", exit_code);
        tests_passed++;
    } else {
        printf("✗ -s flag failed (exit code: %d)\n", exit_code);
    }
    
    printf("\n=== Testing flag combinations ===\n\n");
    
    /* Test 7: -l -p combination */
    printf("Test 7: Testing -l -p combination...\n");
    total_tests++;
    exit_code = execute_and_capture(INSTRUMENTED_BINARY " -l -p " TEMP_GCDA_FILE, 
                                   output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("✓ -l -p combination passed (exit code: %d)\n", exit_code);
        tests_passed++;
    } else {
        printf("✗ -l -p combination failed (exit code: %d)\n", exit_code);
    }
    
    /* Test 8: -p -l combination (different order) */
    printf("\nTest 8: Testing -p -l combination (different order)...\n");
    total_tests++;
    exit_code = execute_and_capture(INSTRUMENTED_BINARY " -p -l " TEMP_GCDA_FILE, 
                                   output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("✓ -p -l combination passed (exit code: %d)\n", exit_code);
        tests_passed++;
    } else {
        printf("✗ -p -l combination failed (exit code: %d)\n", exit_code);
    }
    
    /* Test 9: -r -s combination */
    printf("\nTest 9: Testing -r -s combination...\n");
    total_tests++;
    exit_code = execute_and_capture(INSTRUMENTED_BINARY " -r -s " TEMP_GCDA_FILE, 
                                   output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("✓ -r -s combination passed (exit code: %d)\n", exit_code);
        tests_passed++;
    } else {
        printf("✗ -r -s combination failed (exit code: %d)\n", exit_code);
    }
    
    printf("\n=== Testing invalid flag handling ===\n\n");
    
    /* Test 10: Invalid flag -X */
    printf("Test 10: Testing invalid flag -X...\n");
    total_tests++;
    exit_code = execute_and_capture(INSTRUMENTED_BINARY " -X " TEMP_GCDA_FILE, 
                                   output, sizeof(output), 1);
    if (exit_code != 0 && contains_string(output, "unknown flag `X'")) {
        printf("✓ Invalid flag -X correctly rejected\n");
        printf("  Expected error found: 'unknown flag `X''\n");
        tests_passed++;
    } else {
        printf("✗ Invalid flag test failed (exit code: %d)\n", exit_code);
        printf("  Output: %s\n", output);
    }
    
    /* Test 11: Invalid flag with no file argument */
    printf("\nTest 11: Testing invalid flag -Y (no file)...\n");
    total_tests++;
    exit_code = execute_and_capture(INSTRUMENTED_BINARY " -Y", 
                                   output, sizeof(output), 1);
    if (exit_code != 0 && contains_string(output, "unknown flag `Y'")) {
        printf("✓ Invalid flag -Y correctly rejected\n");
        printf("  Expected error found: 'unknown flag `Y''\n");
        tests_passed++;
    } else {
        printf("✗ Invalid flag test failed (exit code: %d)\n", exit_code);
        printf("  Output: %s\n", output);
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    
    /* Cleanup */
    printf("\nCleaning up temporary files...\n");
    unlink(TEMP_GCDA_FILE);
    unlink(INSTRUMENTED_BINARY);
    
    /* Clean up coverage data files from instrumented binary */
    char coverage_files[3][64];
    snprintf(coverage_files[0], sizeof(coverage_files[0]), "%s.gcda", INSTRUMENTED_BINARY);
    snprintf(coverage_files[1], sizeof(coverage_files[1]), "%s.gcno", INSTRUMENTED_BINARY);
    snprintf(coverage_files[2], sizeof(coverage_files[2]), "gcov-dump-instrumented.gcda");
    
    for (int i = 0; i < 3; i++) {
        if (unlink(coverage_files[i]) == 0) {
            printf("Removed: %s\n", coverage_files[i]);
        }
    }
    
    if (tests_passed == total_tests) {
        printf("\n✓ All tests passed!\n");
        return 0;
    } else {
        printf("\n✗ Some tests failed\n");
        return 1;
    }
}
