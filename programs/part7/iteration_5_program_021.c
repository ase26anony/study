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
    /* Version: 0x76312e2a (gcov 12*) */
    0x2a, 0x2e, 0x31, 0x76,
    /* Stamp: 0x12345678 */
    0x78, 0x56, 0x34, 0x12,
    /* Length: 0 (no records) */
    0x00, 0x00, 0x00, 0x00
};

/* Execute command and capture output */
static char* run_command(const char* cmd, int capture_stderr) {
    char* result = NULL;
    size_t result_size = 0;
    FILE* fp;
    char buffer[1024];
    
    if (capture_stderr) {
        char cmd_with_stderr[2048];
        snprintf(cmd_with_stderr, sizeof(cmd_with_stderr), "%s 2>&1", cmd);
        fp = popen(cmd_with_stderr, "r");
    } else {
        fp = popen(cmd, "r");
    }
    
    if (!fp) {
        perror("popen failed");
        return NULL;
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        size_t buffer_len = strlen(buffer);
        result = realloc(result, result_size + buffer_len + 1);
        if (!result) {
            perror("realloc failed");
            pclose(fp);
            return NULL;
        }
        memcpy(result + result_size, buffer, buffer_len);
        result_size += buffer_len;
        result[result_size] = '\0';
    }
    
    pclose(fp);
    return result;
}

/* Check if string contains substring */
static int contains_string(const char* str, const char* substr) {
    return str && substr && strstr(str, substr) != NULL;
}

/* Build instrumented gcov-dump */
static int build_gcov_dump(const char* gcov_dump_source) {
    char cmd[1024];
    struct stat st;
    
    printf("Building instrumented gcov-dump...\n");
    
    /* Check if source file exists */
    if (stat(gcov_dump_source, &st) != 0) {
        fprintf(stderr, "Source file not found: %s\n", gcov_dump_source);
        return 0;
    }
    
    /* Compile with coverage instrumentation */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, gcov_dump_source);
    
    printf("Compiling: %s\n", cmd);
    
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Compilation failed with code %d\n", ret);
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

/* Create minimal coverage data file */
static int create_minimal_gcda(void) {
    FILE* fp = fopen(TEMP_GCDA_FILE, "wb");
    if (!fp) {
        perror("Failed to create temporary gcda file");
        return 0;
    }
    
    fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    printf("Created minimal coverage file: %s\n", TEMP_GCDA_FILE);
    return 1;
}

/* Test -h flag (help) */
static int test_help_flag(void) {
    printf("Testing -h flag...\n");
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s -h", INSTRUMENTED_BINARY);
    
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "-h flag test failed: exit code %d\n", ret);
        return 0;
    }
    
    printf("✓ -h flag test passed\n");
    return 1;
}

/* Test -v flag (version) */
static int test_version_flag(void) {
    printf("Testing -v flag...\n");
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s -v", INSTRUMENTED_BINARY);
    
    char* output = run_command(cmd, 0);
    if (!output) {
        fprintf(stderr, "Failed to capture version output\n");
        return 0;
    }
    
    /* Check for version-like output */
    int passed = contains_string(output, "version") || 
                 contains_string(output, "gcov") ||
                 contains_string(output, "GCC");
    
    free(output);
    
    if (!passed) {
        fprintf(stderr, "-v flag test failed: no version info found\n");
        return 0;
    }
    
    printf("✓ -v flag test passed\n");
    return 1;
}

/* Test invalid flag */
static int test_invalid_flag(void) {
    printf("Testing invalid flag -X...\n");
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s -X", INSTRUMENTED_BINARY);
    
    char* output = run_command(cmd, 1);  /* Capture stderr */
    if (!output) {
        fprintf(stderr, "Failed to capture error output\n");
        return 0;
    }
    
    /* Check for the exact error message from uncovered lines */
    int passed = contains_string(output, "unknown flag `X'");
    
    free(output);
    
    if (!passed) {
        fprintf(stderr, "Invalid flag test failed: expected 'unknown flag `X'' not found\n");
        return 0;
    }
    
    printf("✓ Invalid flag test passed\n");
    return 1;
}

/* Test flag with coverage file */
static int test_flag_with_file(const char* flag, const char* test_name) {
    printf("Testing %s flag with coverage file...\n", test_name);
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flag, TEMP_GCDA_FILE);
    
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "%s flag test failed: exit code %d\n", test_name, ret);
        return 0;
    }
    
    printf("✓ %s flag test passed\n", test_name);
    return 1;
}

/* Test flag combinations */
static int test_flag_combinations(void) {
    printf("Testing flag combinations...\n");
    
    int all_passed = 1;
    
    /* Test -l -p combination */
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s -l -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "-l -p combination test failed: exit code %d\n", ret);
        all_passed = 0;
    } else {
        printf("✓ -l -p combination test passed\n");
    }
    
    /* Test -p -l (different order) */
    snprintf(cmd, sizeof(cmd), "%s -p -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "-p -l combination test failed: exit code %d\n", ret);
        all_passed = 0;
    } else {
        printf("✓ -p -l combination test passed\n");
    }
    
    /* Test -r -s combination */
    snprintf(cmd, sizeof(cmd), "%s -r -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "-r -s combination test failed: exit code %d\n", ret);
        all_passed = 0;
    } else {
        printf("✓ -r -s combination test passed\n");
    }
    
    return all_passed;
}

/* Cleanup temporary files */
static void cleanup(void) {
    printf("Cleaning up...\n");
    
    if (remove(TEMP_GCDA_FILE) == 0) {
        printf("Removed: %s\n", TEMP_GCDA_FILE);
    }
    
    if (remove(INSTRUMENTED_BINARY) == 0) {
        printf("Removed: %s\n", INSTRUMENTED_BINARY);
    }
}

int main(int argc, char** argv) {
    printf("=== gcov-dump Coverage Test Program ===\n\n");
    
    /* Determine gcov-dump source path */
    const char* gcov_dump_source = "gcov-dump.cc";
    if (argc > 1) {
        gcov_dump_source = argv[1];
    }
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_gcov_dump(gcov_dump_source)) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    if (!create_minimal_gcda()) {
        fprintf(stderr, "Failed to create minimal coverage file\n");
        cleanup();
        return 1;
    }
    
    int all_tests_passed = 1;
    
    /* Step 3: Execute test sequence */
    
    /* Test flags without file argument */
    if (!test_help_flag()) all_tests_passed = 0;
    if (!test_version_flag()) all_tests_passed = 0;
    if (!test_invalid_flag()) all_tests_passed = 0;
    
    /* Test flags with coverage file */
    if (!test_flag_with_file("-l", "-l")) all_tests_passed = 0;
    if (!test_flag_with_file("-p", "-p")) all_tests_passed = 0;
    if (!test_flag_with_file("-r", "-r")) all_tests_passed = 0;
    if (!test_flag_with_file("-s", "-s")) all_tests_passed = 0;
    
    /* Test flag combinations */
    if (!test_flag_combinations()) all_tests_passed = 0;
    
    /* Step 4: Cleanup */
    cleanup();
    
    printf("\n=== Test Summary ===\n");
    if (all_tests_passed) {
        printf("✓ All tests passed!\n");
        printf("\nThe following uncovered lines in gcov-dump.cc should now be covered:\n");
        printf("  - Lines 111-130: switch cases for -h, -v, -l, -p, -r, -s, and default case\n");
        printf("  - Specifically: print_usage(), print_version(), flag assignments, and 'unknown flag' error\n");
        return 0;
    } else {
        printf("✗ Some tests failed\n");
        return 1;
    }
}
