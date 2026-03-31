/**
 * Test driver for gcov-dump.cc uncovered lines (111-130)
 * Build with: gcc -O0 -g -Wno-deprecated-declarations -o test_gcov_dump test_gcov_dump.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_GCDA_FILE "/tmp/minimal.gcda"
#define TEMP_GCOV_DUMP "/tmp/gcov-dump-instrumented"

/**
 * Create a minimal valid .gcda file
 * Format: magic (0x67636461), version (0x*), stamp, length=0
 */
int create_minimal_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create minimal .gcda file");
        return -1;
    }
    
    /* GCOV data file header */
    unsigned int magic = 0x67636461; /* "gcda" in little-endian */
    unsigned int version = 0x3430392a; /* GCC 9.x format */
    unsigned int stamp = 0x12345678; /* arbitrary stamp */
    
    fwrite(&magic, sizeof(unsigned int), 1, fp);
    fwrite(&version, sizeof(unsigned int), 1, fp);
    fwrite(&stamp, sizeof(unsigned int), 1, fp);
    
    /* Zero records - just write length 0 */
    unsigned int length = 0;
    fwrite(&length, sizeof(unsigned int), 1, fp);
    
    fclose(fp);
    return 0;
}

/**
 * Execute command and capture output
 */
int execute_and_capture(const char *cmd, char *output, size_t output_size, 
                        int capture_stderr, int *exit_status) {
    FILE *fp;
    char buffer[1024];
    size_t total = 0;
    
    /* Build command with stderr redirection if needed */
    char full_cmd[2048];
    if (capture_stderr) {
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    } else {
        snprintf(full_cmd, sizeof(full_cmd), "%s", cmd);
    }
    
    fp = popen(full_cmd, "r");
    if (!fp) {
        perror("popen failed");
        return -1;
    }
    
    output[0] = '\0';
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        size_t len = strlen(buffer);
        if (total + len < output_size) {
            strcat(output, buffer);
            total += len;
        }
    }
    
    *exit_status = pclose(fp);
    if (WIFEXITED(*exit_status)) {
        *exit_status = WEXITSTATUS(*exit_status);
    }
    
    return 0;
}

/**
 * Check if string contains substring
 */
int contains_string(const char *str, const char *substr) {
    return strstr(str, substr) != NULL;
}

/**
 * Build instrumented gcov-dump
 */
int build_gcov_dump(const char *source_path) {
    char cmd[1024];
    int status;
    
    printf("Building instrumented gcov-dump from %s\n", source_path);
    
    /* Check if source file exists */
    struct stat st;
    if (stat(source_path, &st) != 0) {
        fprintf(stderr, "Source file not found: %s\n", source_path);
        return -1;
    }
    
    /* Compile with coverage instrumentation */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             TEMP_GCOV_DUMP, source_path);
    
    status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile gcov-dump\n");
        return -1;
    }
    
    /* Verify the binary was created */
    if (stat(TEMP_GCOV_DUMP, &st) != 0) {
        fprintf(stderr, "Compiled binary not found: %s\n", TEMP_GCOV_DUMP);
        return -1;
    }
    
    printf("Successfully built instrumented gcov-dump\n");
    return 0;
}

int main(int argc, char *argv[]) {
    char output[4096];
    int exit_status;
    int tests_passed = 0;
    int total_tests = 0;
    
    /* Try to locate gcov-dump.cc */
    const char *source_paths[] = {
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "../../../gcc/gcov-dump.cc",
        "gcov-dump.cc",
        NULL
    };
    
    const char *gcov_dump_source = NULL;
    for (int i = 0; source_paths[i] != NULL; i++) {
        struct stat st;
        if (stat(source_paths[i], &st) == 0) {
            gcov_dump_source = source_paths[i];
            break;
        }
    }
    
    if (!gcov_dump_source) {
        fprintf(stderr, "Could not find gcov-dump.cc source file\n");
        fprintf(stderr, "Please specify path as argument: %s <path-to-gcov-dump.cc>\n", argv[0]);
        if (argc > 1) {
            gcov_dump_source = argv[1];
        } else {
            return 1;
        }
    }
    
    /* Step 1: Build instrumented gcov-dump */
    if (build_gcov_dump(gcov_dump_source) != 0) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    printf("\nCreating minimal .gcda file...\n");
    if (create_minimal_gcda(TEMP_GCDA_FILE) != 0) {
        return 1;
    }
    
    printf("\n=== Starting gcov-dump flag coverage tests ===\n\n");
    
    /* Test 1: -h flag (help) */
    printf("Test 1: Testing -h flag (help)...\n");
    total_tests++;
    if (execute_and_capture(TEMP_GCOV_DUMP " -h", output, sizeof(output), 0, &exit_status) == 0) {
        if (exit_status == 0) {
            printf("✓ -h flag test passed (exit code: %d)\n", exit_status);
            tests_passed++;
        } else {
            printf("✗ -h flag test failed (exit code: %d)\n", exit_status);
        }
    }
    
    /* Test 2: -v flag (version) */
    printf("\nTest 2: Testing -v flag (version)...\n");
    total_tests++;
    if (execute_and_capture(TEMP_GCOV_DUMP " -v", output, sizeof(output), 0, &exit_status) == 0) {
        if (exit_status == 0 && strlen(output) > 0) {
            printf("✓ -v flag test passed. Output: %s", output);
            tests_passed++;
        } else {
            printf("✗ -v flag test failed (exit code: %d, output: %s)\n", exit_status, output);
        }
    }
    
    /* Test 3: -l flag (dump contents) */
    printf("\nTest 3: Testing -l flag (dump contents)...\n");
    total_tests++;
    if (execute_and_capture(TEMP_GCOV_DUMP " -l " TEMP_GCDA_FILE, 
                          output, sizeof(output), 0, &exit_status) == 0) {
        if (exit_status == 0) {
            printf("✓ -l flag test passed (exit code: %d)\n", exit_status);
            tests_passed++;
        } else {
            printf("✗ -l flag test failed (exit code: %d)\n", exit_status);
        }
    }
    
    /* Test 4: -p flag (dump positions) */
    printf("\nTest 4: Testing -p flag (dump positions)...\n");
    total_tests++;
    if (execute_and_capture(TEMP_GCOV_DUMP " -p " TEMP_GCDA_FILE, 
                          output, sizeof(output), 0, &exit_status) == 0) {
        if (exit_status == 0) {
            printf("✓ -p flag test passed (exit code: %d)\n", exit_status);
            tests_passed++;
        } else {
            printf("✗ -p flag test failed (exit code: %d)\n", exit_status);
        }
    }
    
    /* Test 5: -r flag (dump raw) */
    printf("\nTest 5: Testing -r flag (dump raw)...\n");
    total_tests++;
    if (execute_and_capture(TEMP_GCOV_DUMP " -r " TEMP_GCDA_FILE, 
                          output, sizeof(output), 0, &exit_status) == 0) {
        if (exit_status == 0) {
            printf("✓ -r flag test passed (exit code: %d)\n", exit_status);
            tests_passed++;
        } else {
            printf("✗ -r flag test failed (exit code: %d)\n", exit_status);
        }
    }
    
    /* Test 6: -s flag (dump stable) */
    printf("\nTest 6: Testing -s flag (dump stable)...\n");
    total_tests++;
    if (execute_and_capture(TEMP_GCOV_DUMP " -s " TEMP_GCDA_FILE, 
                          output, sizeof(output), 0, &exit_status) == 0) {
        if (exit_status == 0) {
            printf("✓ -s flag test passed (exit code: %d)\n", exit_status);
            tests_passed++;
        } else {
            printf("✗ -s flag test failed (exit code: %d)\n", exit_status);
        }
    }
    
    /* Test 7: Combined flags -l -p */
    printf("\nTest 7: Testing combined flags -l -p...\n");
    total_tests++;
    if (execute_and_capture(TEMP_GCOV_DUMP " -l -p " TEMP_GCDA_FILE, 
                          output, sizeof(output), 0, &exit_status) == 0) {
        if (exit_status == 0) {
            printf("✓ Combined -l -p test passed (exit code: %d)\n", exit_status);
            tests_passed++;
        } else {
            printf("✗ Combined -l -p test failed (exit code: %d)\n", exit_status);
        }
    }
    
    /* Test 8: Combined flags -r -s (different order) */
    printf("\nTest 8: Testing combined flags -r -s...\n");
    total_tests++;
    if (execute_and_capture(TEMP_GCOV_DUMP " -r -s " TEMP_GCDA_FILE, 
                          output, sizeof(output), 0, &exit_status) == 0) {
        if (exit_status == 0) {
            printf("✓ Combined -r -s test passed (exit code: %d)\n", exit_status);
            tests_passed++;
        } else {
            printf("✗ Combined -r -s test failed (exit code: %d)\n", exit_status);
        }
    }
    
    /* Test 9: Invalid flag -X */
    printf("\nTest 9: Testing invalid flag -X...\n");
    total_tests++;
    if (execute_and_capture(TEMP_GCOV_DUMP " -X " TEMP_GCDA_FILE, 
                          output, sizeof(output), 1, &exit_status) == 0) {
        if (contains_string(output, "unknown flag `X'")) {
            printf("✓ Invalid flag test passed. Error message found: %s", output);
            tests_passed++;
        } else {
            printf("✗ Invalid flag test failed. Expected 'unknown flag' message.\n");
            printf("  Actual output: %s", output);
        }
    }
    
    /* Test 10: Invalid flag without file argument */
    printf("\nTest 10: Testing invalid flag -Y (no file)...\n");
    total_tests++;
    if (execute_and_capture(TEMP_GCOV_DUMP " -Y", 
                          output, sizeof(output), 1, &exit_status) == 0) {
        if (contains_string(output, "unknown flag `Y'")) {
            printf("✓ Invalid flag (no file) test passed. Error message found: %s", output);
            tests_passed++;
        } else {
            printf("✗ Invalid flag (no file) test failed.\n");
            printf("  Actual output: %s", output);
        }
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    
    /* Cleanup */
    printf("\nCleaning up temporary files...\n");
    unlink(TEMP_GCDA_FILE);
    unlink(TEMP_GCOV_DUMP);
    
    /* Clean up coverage files from instrumented binary */
    char gcda_file[256];
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", TEMP_GCOV_DUMP);
    unlink(gcda_file);
    
    char gcno_file[256];
    snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", TEMP_GCOV_DUMP);
    unlink(gcno_file);
    
    return (tests_passed == total_tests) ? 0 : 1;
}
