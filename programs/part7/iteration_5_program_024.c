#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_GCDA_FILE "/tmp/minimal.gcda"
#define TEMP_GCOV_DUMP "/tmp/gcov-dump-instrumented"

/* Minimal valid GCOV data file structure */
static const unsigned char minimal_gcda[] = {
    /* Magic: 'gcda' */
    0x67, 0x63, 0x64, 0x61,
    /* Version: 0x76312e2a (example version) */
    0x2a, 0x2e, 0x31, 0x76,
    /* Stamp: 0x00000000 */
    0x00, 0x00, 0x00, 0x00,
    /* Length: 0 (no records) */
    0x00, 0x00, 0x00, 0x00
};

/* Execute command and capture output */
static char *execute_cmd(const char *cmd, int capture_stderr) {
    char *output = NULL;
    size_t output_size = 0;
    FILE *fp;
    char buffer[1024];
    
    if (capture_stderr) {
        char cmd_with_stderr[2048];
        snprintf(cmd_with_stderr, sizeof(cmd_with_stderr), "%s 2>&1", cmd);
        fp = popen(cmd_with_stderr, "r");
    } else {
        fp = popen(cmd, "r");
    }
    
    if (!fp) {
        return NULL;
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        size_t len = strlen(buffer);
        output = realloc(output, output_size + len + 1);
        if (!output) {
            pclose(fp);
            return NULL;
        }
        memcpy(output + output_size, buffer, len + 1);
        output_size += len;
    }
    
    pclose(fp);
    return output;
}

/* Check if string contains substring */
static int contains_string(const char *str, const char *substr) {
    return str && substr && strstr(str, substr) != NULL;
}

/* Create minimal valid .gcda file */
static int create_minimal_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        return 0;
    }
    
    size_t written = fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    return written == sizeof(minimal_gcda);
}

/* Build instrumented gcov-dump */
static int build_gcov_dump(const char *source_path) {
    char cmd[2048];
    struct stat st;
    
    /* Check if source exists */
    if (stat(source_path, &st) != 0) {
        fprintf(stderr, "Error: gcov-dump.cc not found at %s\n", source_path);
        return 0;
    }
    
    /* Build command */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             TEMP_GCOV_DUMP, source_path);
    
    printf("Building instrumented gcov-dump...\n");
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Error: Failed to build gcov-dump\n");
        return 0;
    }
    
    /* Verify the binary was created */
    if (stat(TEMP_GCOV_DUMP, &st) != 0) {
        fprintf(stderr, "Error: Binary not created\n");
        return 0;
    }
    
    printf("Instrumented gcov-dump built successfully\n");
    return 1;
}

/* Test individual flag */
static int test_flag(const char *flag, const char *filename, 
                     int expect_success, const char *expected_output) {
    char cmd[1024];
    int success = 0;
    
    if (filename) {
        snprintf(cmd, sizeof(cmd), "%s %s %s", TEMP_GCOV_DUMP, flag, filename);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s", TEMP_GCOV_DUMP, flag);
    }
    
    printf("Testing: %s\n", cmd);
    
    if (expected_output) {
        /* Capture output and check for expected string */
        char *output = execute_cmd(cmd, (strcmp(flag, "-X") == 0) ? 1 : 0);
        if (output) {
            if (contains_string(output, expected_output)) {
                success = 1;
            }
            free(output);
        }
    } else {
        /* Just check exit status */
        int result = system(cmd);
        if (expect_success) {
            success = (WEXITSTATUS(result) == 0);
        } else {
            success = (WEXITSTATUS(result) != 0);
        }
    }
    
    if (success) {
        printf("  ✓ PASS\n");
    } else {
        printf("  ✗ FAIL\n");
    }
    
    return success;
}

/* Test flag combinations */
static int test_flag_combination(const char *flags, const char *filename) {
    char cmd[1024];
    
    snprintf(cmd, sizeof(cmd), "%s %s %s", TEMP_GCOV_DUMP, flags, filename);
    printf("Testing: %s\n", cmd);
    
    int result = system(cmd);
    int success = (WEXITSTATUS(result) == 0);
    
    if (success) {
        printf("  ✓ PASS\n");
    } else {
        printf("  ✗ FAIL\n");
    }
    
    return success;
}

int main(int argc, char *argv[]) {
    int all_tests_passed = 1;
    const char *gcov_dump_source = NULL;
    
    /* Try to locate gcov-dump.cc */
    if (argc > 1) {
        gcov_dump_source = argv[1];
    } else {
        /* Try common locations */
        const char *possible_paths[] = {
            "../gcc/gcov-dump.cc",
            "../../gcc/gcov-dump.cc",
            "/usr/src/gcc/gcc/gcov-dump.cc",
            NULL
        };
        
        for (int i = 0; possible_paths[i]; i++) {
            FILE *fp = fopen(possible_paths[i], "r");
            if (fp) {
                fclose(fp);
                gcov_dump_source = possible_paths[i];
                break;
            }
        }
    }
    
    if (!gcov_dump_source) {
        fprintf(stderr, "Error: gcov-dump.cc not found. Please specify path as argument.\n");
        fprintf(stderr, "Usage: %s <path-to-gcov-dump.cc>\n", argv[0]);
        return 1;
    }
    
    printf("Using source: %s\n", gcov_dump_source);
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_gcov_dump(gcov_dump_source)) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    printf("\nCreating minimal .gcda file...\n");
    if (!create_minimal_gcda(TEMP_GCDA_FILE)) {
        fprintf(stderr, "Error: Failed to create minimal .gcda file\n");
        unlink(TEMP_GCOV_DUMP);
        return 1;
    }
    printf("Created %s\n", TEMP_GCDA_FILE);
    
    /* Step 3: Execute test sequence */
    printf("\n=== Starting test sequence ===\n\n");
    
    /* Test -h flag (help) */
    printf("1. Testing -h flag (help)...\n");
    if (!test_flag("-h", NULL, 1, NULL)) {
        all_tests_passed = 0;
    }
    
    /* Test -v flag (version) */
    printf("\n2. Testing -v flag (version)...\n");
    if (!test_flag("-v", NULL, 1, "gcov-dump")) {
        all_tests_passed = 0;
    }
    
    /* Test -l flag with minimal file */
    printf("\n3. Testing -l flag with minimal .gcda...\n");
    if (!test_flag("-l", TEMP_GCDA_FILE, 1, NULL)) {
        all_tests_passed = 0;
    }
    
    /* Test -p flag with minimal file */
    printf("\n4. Testing -p flag with minimal .gcda...\n");
    if (!test_flag("-p", TEMP_GCDA_FILE, 1, NULL)) {
        all_tests_passed = 0;
    }
    
    /* Test -r flag with minimal file */
    printf("\n5. Testing -r flag with minimal .gcda...\n");
    if (!test_flag("-r", TEMP_GCDA_FILE, 1, NULL)) {
        all_tests_passed = 0;
    }
    
    /* Test -s flag with minimal file */
    printf("\n6. Testing -s flag with minimal .gcda...\n");
    if (!test_flag("-s", TEMP_GCDA_FILE, 1, NULL)) {
        all_tests_passed = 0;
    }
    
    /* Test flag combinations */
    printf("\n7. Testing flag combination -l -p...\n");
    if (!test_flag_combination("-l -p", TEMP_GCDA_FILE)) {
        all_tests_passed = 0;
    }
    
    printf("\n8. Testing flag combination -p -l (different order)...\n");
    if (!test_flag_combination("-p -l", TEMP_GCDA_FILE)) {
        all_tests_passed = 0;
    }
    
    printf("\n9. Testing flag combination -r -s...\n");
    if (!test_flag_combination("-r -s", TEMP_GCDA_FILE)) {
        all_tests_passed = 0;
    }
    
    /* Test invalid flag */
    printf("\n10. Testing invalid flag -X...\n");
    if (!test_flag("-X", TEMP_GCDA_FILE, 0, "unknown flag `X'")) {
        all_tests_passed = 0;
    }
    
    /* Test invalid flag without file */
    printf("\n11. Testing invalid flag -X without file...\n");
    if (!test_flag("-X", NULL, 0, "unknown flag `X'")) {
        all_tests_passed = 0;
    }
    
    /* Step 4: Cleanup */
    printf("\n=== Cleaning up ===\n");
    unlink(TEMP_GCDA_FILE);
    unlink(TEMP_GCOV_DUMP);
    
    /* Remove coverage data files generated by instrumented binary */
    char coverage_files[][64] = {
        "gcov-dump-instrumented.gcda",
        "gcov-dump-instrumented.gcno",
        "minimal.gcda.gcda",
        "minimal.gcda.gcno"
    };
    
    for (size_t i = 0; i < sizeof(coverage_files)/sizeof(coverage_files[0]); i++) {
        if (unlink(coverage_files[i]) == 0) {
            printf("Removed %s\n", coverage_files[i]);
        }
    }
    
    printf("\n=== Test Summary ===\n");
    if (all_tests_passed) {
        printf("All tests PASSED!\n");
        return 0;
    } else {
        printf("Some tests FAILED!\n");
        return 1;
    }
}
