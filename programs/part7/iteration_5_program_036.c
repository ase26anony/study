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
    /* Version: 0x7630302A (gcov 12 format) */
    0x2A, 0x30, 0x30, 0x76,
    /* Stamp: 0 */
    0x00, 0x00, 0x00, 0x00,
    /* Length: 0 (no records) */
    0x00, 0x00, 0x00, 0x00
};

/* Create a minimal valid .gcda file */
static int create_minimal_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("fopen");
        return 0;
    }
    
    size_t written = fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    return written == sizeof(minimal_gcda);
}

/* Execute command and capture output */
static char *execute_command(const char *cmd, int capture_stderr) {
    char *output = NULL;
    size_t output_size = 0;
    FILE *fp;
    
    if (capture_stderr) {
        char cmd_with_stderr[1024];
        snprintf(cmd_with_stderr, sizeof(cmd_with_stderr), "%s 2>&1", cmd);
        fp = popen(cmd_with_stderr, "r");
    } else {
        fp = popen(cmd, "r");
    }
    
    if (!fp) {
        perror("popen");
        return NULL;
    }
    
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        size_t len = strlen(buffer);
        output = realloc(output, output_size + len + 1);
        if (!output) {
            perror("realloc");
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

/* Test a specific flag */
static int test_flag(const char *flag, const char *file, 
                     int expect_success, const char *expected_output) {
    char cmd[1024];
    int success = 0;
    
    if (file) {
        snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flag, file);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s", INSTRUMENTED_BINARY, flag);
    }
    
    printf("Testing: %s\n", cmd);
    
    if (expected_output) {
        /* Capture output and check for expected string */
        char *output = execute_command(cmd, 1);
        if (output) {
            if (contains_string(output, expected_output)) {
                printf("  ✓ Found expected output: '%s'\n", expected_output);
                success = 1;
            } else {
                printf("  ✗ Expected '%s' not found in output\n", expected_output);
                printf("    Output was: %s\n", output);
            }
            free(output);
        }
    } else {
        /* Just check exit status */
        int status = system(cmd);
        if (expect_success) {
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                printf("  ✓ Command exited successfully\n");
                success = 1;
            } else {
                printf("  ✗ Command failed with status %d\n", status);
            }
        } else {
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                printf("  ✓ Command failed as expected (status %d)\n", WEXITSTATUS(status));
                success = 1;
            } else {
                printf("  ✗ Command should have failed but didn't\n");
            }
        }
    }
    
    return success;
}

/* Build instrumented gcov-dump */
static int build_gcov_dump(const char *source_path) {
    char cmd[2048];
    
    printf("Building instrumented gcov-dump...\n");
    
    /* Try to find gcov-dump.cc if not provided */
    if (!source_path) {
        /* Common locations in GCC source tree */
        const char *possible_paths[] = {
            "../gcc/gcov-dump.cc",
            "../../gcc/gcov-dump.cc",
            "gcc/gcov-dump.cc",
            NULL
        };
        
        for (int i = 0; possible_paths[i]; i++) {
            struct stat st;
            if (stat(possible_paths[i], &st) == 0) {
                source_path = possible_paths[i];
                break;
            }
        }
        
        if (!source_path) {
            printf("Could not find gcov-dump.cc\n");
            return 0;
        }
    }
    
    printf("Using source: %s\n", source_path);
    
    /* Compile with coverage instrumentation */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    int status = system(cmd);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("✓ Successfully built instrumented gcov-dump\n");
        return 1;
    } else {
        printf("✗ Failed to build gcov-dump (status %d)\n", status);
        return 0;
    }
}

int main(int argc, char *argv[]) {
    int tests_passed = 0;
    int total_tests = 0;
    
    printf("=== GCOV-DUMP Coverage Test ===\n\n");
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_gcov_dump(argc > 1 ? argv[1] : NULL)) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    printf("\nCreating minimal .gcda file...\n");
    if (!create_minimal_gcda(TEMP_GCDA_FILE)) {
        printf("✗ Failed to create minimal .gcda file\n");
        unlink(INSTRUMENTED_BINARY);
        return 1;
    }
    printf("✓ Created %s\n", TEMP_GCDA_FILE);
    
    /* Step 3: Execute test sequence */
    printf("\n=== Running Tests ===\n");
    
    /* Test -h flag (help) */
    total_tests++;
    if (test_flag("-h", NULL, 1, NULL)) {
        tests_passed++;
    }
    
    /* Test -v flag (version) */
    total_tests++;
    if (test_flag("-v", NULL, 1, "gcov-dump")) {
        tests_passed++;
    }
    
    /* Test -l flag (dump contents) */
    total_tests++;
    if (test_flag("-l", TEMP_GCDA_FILE, 1, NULL)) {
        tests_passed++;
    }
    
    /* Test -p flag (dump positions) */
    total_tests++;
    if (test_flag("-p", TEMP_GCDA_FILE, 1, NULL)) {
        tests_passed++;
    }
    
    /* Test -r flag (dump raw) */
    total_tests++;
    if (test_flag("-r", TEMP_GCDA_FILE, 1, NULL)) {
        tests_passed++;
    }
    
    /* Test -s flag (dump stable) */
    total_tests++;
    if (test_flag("-s", TEMP_GCDA_FILE, 1, NULL)) {
        tests_passed++;
    }
    
    /* Test flag combination -l -p */
    total_tests++;
    if (test_flag("-l -p", TEMP_GCDA_FILE, 1, NULL)) {
        tests_passed++;
    }
    
    /* Test flag combination -r -s */
    total_tests++;
    if (test_flag("-r -s", TEMP_GCDA_FILE, 1, NULL)) {
        tests_passed++;
    }
    
    /* Test flag ordering variation -p -l */
    total_tests++;
    if (test_flag("-p -l", TEMP_GCDA_FILE, 1, NULL)) {
        tests_passed++;
    }
    
    /* Test invalid flag -X */
    total_tests++;
    if (test_flag("-X", TEMP_GCDA_FILE, 0, "unknown flag `X'")) {
        tests_passed++;
    }
    
    /* Test another invalid flag -Z */
    total_tests++;
    if (test_flag("-Z", NULL, 0, "unknown flag `Z'")) {
        tests_passed++;
    }
    
    /* Step 4: Cleanup and report */
    printf("\n=== Cleanup ===\n");
    
    /* Remove temporary files */
    if (unlink(TEMP_GCDA_FILE) == 0) {
        printf("✓ Removed %s\n", TEMP_GCDA_FILE);
    }
    
    if (unlink(INSTRUMENTED_BINARY) == 0) {
        printf("✓ Removed %s\n", INSTRUMENTED_BINARY);
    }
    
    /* Remove coverage data files generated during execution */
    system("rm -f /tmp/gcov-dump-instrumented.gcda /tmp/gcov-dump-instrumented.gcno");
    
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d tests\n", tests_passed, total_tests);
    
    return tests_passed == total_tests ? 0 : 1;
}
