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
    /* Version: 0xB1C1B1C1 (little-endian) */
    0xC1, 0xB1, 0xC1, 0xB1,
    /* Zero-length record tag */
    0x00, 0x00, 0x00, 0x00,
    /* Zero-length record length */
    0x00, 0x00, 0x00, 0x00
};

/* Execute command and capture output */
int execute_and_capture(const char *cmd, char *output, size_t output_size, 
                        int capture_stderr) {
    char full_cmd[1024];
    FILE *fp;
    
    if (capture_stderr) {
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    } else {
        snprintf(full_cmd, sizeof(full_cmd), "%s", cmd);
    }
    
    fp = popen(full_cmd, "r");
    if (!fp) {
        return -1;
    }
    
    if (output && output_size > 0) {
        size_t bytes_read = fread(output, 1, output_size - 1, fp);
        output[bytes_read] = '\0';
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
}

/* Create minimal valid .gcda file */
int create_minimal_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        return -1;
    }
    
    size_t written = fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    return (written == sizeof(minimal_gcda)) ? 0 : -1;
}

/* Build instrumented gcov-dump */
int build_instrumented_gcov_dump(const char *source_path) {
    char cmd[2048];
    struct stat st;
    
    printf("Building instrumented gcov-dump...\n");
    
    /* Check if source exists */
    if (stat(source_path, &st) != 0) {
        fprintf(stderr, "Source file not found: %s\n", source_path);
        return -1;
    }
    
    /* Compile with coverage instrumentation */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    printf("Compile command: %s\n", cmd);
    
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile gcov-dump\n");
        return -1;
    }
    
    /* Verify the binary was created */
    if (stat(INSTRUMENTED_BINARY, &st) != 0) {
        fprintf(stderr, "Instrumented binary not created\n");
        return -1;
    }
    
    printf("Instrumented binary created: %s\n", INSTRUMENTED_BINARY);
    return 0;
}

/* Test -h flag (help) */
int test_help_flag() {
    printf("Testing -h flag...\n");
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s -h", INSTRUMENTED_BINARY);
    
    int exit_code = execute_and_capture(cmd, NULL, 0, 0);
    
    if (exit_code == 0) {
        printf("✓ -h flag test passed (exit code 0)\n");
        return 0;
    } else {
        printf("✗ -h flag test failed (exit code %d)\n", exit_code);
        return -1;
    }
}

/* Test -v flag (version) */
int test_version_flag() {
    printf("Testing -v flag...\n");
    
    char cmd[256];
    char output[1024] = {0};
    
    snprintf(cmd, sizeof(cmd), "%s -v", INSTRUMENTED_BINARY);
    
    int exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    
    if (exit_code == 0 && strlen(output) > 0) {
        printf("✓ -v flag test passed. Output: %s", output);
        return 0;
    } else {
        printf("✗ -v flag test failed (exit code %d)\n", exit_code);
        return -1;
    }
}

/* Test invalid flag */
int test_invalid_flag() {
    printf("Testing invalid flag -X...\n");
    
    char cmd[256];
    char output[1024] = {0};
    
    snprintf(cmd, sizeof(cmd), "%s -X", INSTRUMENTED_BINARY);
    
    int exit_code = execute_and_capture(cmd, output, sizeof(output), 1);
    
    /* Check for the exact error message from uncovered lines */
    if (strstr(output, "unknown flag `X'") != NULL) {
        printf("✓ Invalid flag test passed. Error message found: %s", output);
        return 0;
    } else {
        printf("✗ Invalid flag test failed. Output: %s", output);
        return -1;
    }
}

/* Test flag with minimal coverage file */
int test_flag_with_file(const char *flag, const char *test_name) {
    printf("Testing %s flag with minimal .gcda file...\n", test_name);
    
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flag, TEMP_GCDA_FILE);
    
    int exit_code = execute_and_capture(cmd, NULL, 0, 0);
    
    if (exit_code == 0) {
        printf("✓ %s flag test passed\n", test_name);
        return 0;
    } else {
        printf("✗ %s flag test failed (exit code %d)\n", test_name, exit_code);
        return -1;
    }
}

/* Test flag combinations */
int test_flag_combination(const char *flags, const char *test_name) {
    printf("Testing flag combination %s...\n", test_name);
    
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flags, TEMP_GCDA_FILE);
    
    int exit_code = execute_and_capture(cmd, NULL, 0, 0);
    
    if (exit_code == 0) {
        printf("✓ Flag combination %s test passed\n", test_name);
        return 0;
    } else {
        printf("✗ Flag combination %s test failed (exit code %d)\n", test_name, exit_code);
        return -1;
    }
}

/* Test flag ordering variations */
int test_flag_ordering() {
    printf("Testing flag ordering variations...\n");
    
    int passed = 0;
    char cmd[512];
    
    /* Test -l -p */
    snprintf(cmd, sizeof(cmd), "%s -l -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (execute_and_capture(cmd, NULL, 0, 0) == 0) {
        printf("✓ -l -p ordering test passed\n");
        passed++;
    } else {
        printf("✗ -l -p ordering test failed\n");
    }
    
    /* Test -p -l */
    snprintf(cmd, sizeof(cmd), "%s -p -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (execute_and_capture(cmd, NULL, 0, 0) == 0) {
        printf("✓ -p -l ordering test passed\n");
        passed++;
    } else {
        printf("✗ -p -l ordering test failed\n");
    }
    
    /* Test -r -s */
    snprintf(cmd, sizeof(cmd), "%s -r -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (execute_and_capture(cmd, NULL, 0, 0) == 0) {
        printf("✓ -r -s ordering test passed\n");
        passed++;
    } else {
        printf("✗ -r -s ordering test failed\n");
    }
    
    /* Test -s -r */
    snprintf(cmd, sizeof(cmd), "%s -s -r %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (execute_and_capture(cmd, NULL, 0, 0) == 0) {
        printf("✓ -s -r ordering test passed\n");
        passed++;
    } else {
        printf("✗ -s -r ordering test failed\n");
    }
    
    return (passed == 4) ? 0 : -1;
}

int main(int argc, char *argv[]) {
    int overall_result = 0;
    
    printf("=== Starting gcov-dump coverage test ===\n\n");
    
    /* Try to find gcov-dump.cc in common locations */
    const char *source_paths[] = {
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "../../../gcc/gcov-dump.cc",
        "gcov-dump.cc",
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
        return 1;
    }
    
    printf("Using source file: %s\n", source_path);
    
    /* Step 1: Build instrumented gcov-dump */
    if (build_instrumented_gcov_dump(source_path) != 0) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    printf("\nCreating minimal .gcda file...\n");
    if (create_minimal_gcda(TEMP_GCDA_FILE) != 0) {
        fprintf(stderr, "Failed to create minimal .gcda file\n");
        unlink(INSTRUMENTED_BINARY);
        return 1;
    }
    printf("Created: %s\n", TEMP_GCDA_FILE);
    
    /* Step 3: Execute test sequence */
    printf("\n=== Running test sequence ===\n");
    
    /* Test flags without file argument */
    if (test_help_flag() != 0) overall_result = 1;
    if (test_version_flag() != 0) overall_result = 1;
    if (test_invalid_flag() != 0) overall_result = 1;
    
    /* Test flags with minimal coverage file */
    if (test_flag_with_file("-l", "-l") != 0) overall_result = 1;
    if (test_flag_with_file("-p", "-p") != 0) overall_result = 1;
    if (test_flag_with_file("-r", "-r") != 0) overall_result = 1;
    if (test_flag_with_file("-s", "-s") != 0) overall_result = 1;
    
    /* Test flag combinations */
    if (test_flag_combination("-l -p", "-l -p") != 0) overall_result = 1;
    if (test_flag_combination("-r -s", "-r -s") != 0) overall_result = 1;
    
    /* Test flag ordering */
    if (test_flag_ordering() != 0) overall_result = 1;
    
    /* Step 4: Cleanup */
    printf("\n=== Cleaning up ===\n");
    unlink(TEMP_GCDA_FILE);
    unlink(INSTRUMENTED_BINARY);
    
    /* Also clean up coverage data files created by instrumented binary */
    char coverage_files[][64] = {
        "gcov-dump-instrumented.gcda",
        "gcov-dump-instrumented.gcno",
        "gcov-dump.gcda",
        "gcov-dump.gcno"
    };
    
    for (size_t i = 0; i < sizeof(coverage_files)/sizeof(coverage_files[0]); i++) {
        if (unlink(coverage_files[i]) == 0) {
            printf("Removed: %s\n", coverage_files[i]);
        }
    }
    
    printf("\n=== Test completed ===\n");
    if (overall_result == 0) {
        printf("✓ All tests passed!\n");
    } else {
        printf("✗ Some tests failed\n");
    }
    
    return overall_result;
}
