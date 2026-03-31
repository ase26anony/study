#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_GCDA_FILE "/tmp/minimal.gcda"
#define TEMP_GCOV_DUMP "/tmp/gcov-dump-instrumented"

/* Minimal GCOV data file format:
   - Magic number: 0x67636461 ('gcda')
   - Version: 0x3430372a ('407*' for gcc 4.7 format)
   - Zero records: 0x00000000 (no data records)
*/
static const unsigned char minimal_gcda[] = {
    0x67, 0x63, 0x64, 0x61,  /* 'gcda' magic */
    0x34, 0x30, 0x37, 0x2a,  /* '407*' version */
    0x00, 0x00, 0x00, 0x00   /* zero records */
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
static int execute_and_capture(const char *cmd, char *output, size_t output_size, 
                              int capture_stdout, int *exit_status) {
    char buffer[1024];
    FILE *fp;
    int result = 0;
    
    if (capture_stdout) {
        fp = popen(cmd, "r");
    } else {
        /* For stderr, we need to redirect */
        char full_cmd[1024];
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
        fp = popen(full_cmd, "r");
    }
    
    if (!fp) {
        perror("popen");
        return 0;
    }
    
    output[0] = '\0';
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strlen(output) + strlen(buffer) < output_size) {
            strcat(output, buffer);
        }
    }
    
    result = pclose(fp);
    if (exit_status && WIFEXITED(result)) {
        *exit_status = WEXITSTATUS(result);
    }
    
    return 1;
}

/* Test specific flag */
static int test_flag(const char *flag, const char *filename, 
                    int expect_success, const char *expected_output) {
    char cmd[1024];
    char output[4096] = {0};
    int exit_status = -1;
    
    printf("Testing flag %s...\n", flag);
    
    if (filename) {
        snprintf(cmd, sizeof(cmd), "%s %s %s", TEMP_GCOV_DUMP, flag, filename);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s", TEMP_GCOV_DUMP, flag);
    }
    
    if (!execute_and_capture(cmd, output, sizeof(output), 
                            expect_success ? 1 : 0, &exit_status)) {
        return 0;
    }
    
    if (expect_success) {
        if (exit_status != 0) {
            printf("  FAIL: Expected success but got exit status %d\n", exit_status);
            return 0;
        }
        if (expected_output && strstr(output, expected_output) == NULL) {
            printf("  FAIL: Expected output containing '%s'\n", expected_output);
            printf("  Got: %s\n", output);
            return 0;
        }
    } else {
        if (expected_output && strstr(output, expected_output) == NULL) {
            printf("  FAIL: Expected error containing '%s'\n", expected_output);
            printf("  Got: %s\n", output);
            return 0;
        }
    }
    
    printf("  PASS\n");
    return 1;
}

/* Build instrumented gcov-dump */
static int build_gcov_dump() {
    char cmd[2048];
    int result;
    
    printf("Building instrumented gcov-dump...\n");
    
    /* Try to find gcov-dump.cc in common locations */
    const char *possible_paths[] = {
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "../../../gcc/gcov-dump.cc",
        "/usr/src/gcc/gcc/gcov-dump.cc",
        NULL
    };
    
    const char *source_file = NULL;
    struct stat st;
    
    for (int i = 0; possible_paths[i]; i++) {
        if (stat(possible_paths[i], &st) == 0) {
            source_file = possible_paths[i];
            break;
        }
    }
    
    if (!source_file) {
        /* Try current directory */
        if (stat("gcov-dump.cc", &st) == 0) {
            source_file = "gcov-dump.cc";
        } else {
            printf("ERROR: Could not find gcov-dump.cc\n");
            printf("Please copy gcov-dump.cc to current directory or adjust paths\n");
            return 0;
        }
    }
    
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             TEMP_GCOV_DUMP, source_file);
    
    printf("Compiling: %s\n", cmd);
    result = system(cmd);
    
    if (result != 0) {
        printf("ERROR: Failed to compile gcov-dump\n");
        return 0;
    }
    
    /* Verify the binary was created */
    if (stat(TEMP_GCOV_DUMP, &st) != 0) {
        printf("ERROR: Binary not created: %s\n", TEMP_GCOV_DUMP);
        return 0;
    }
    
    printf("Successfully built %s\n", TEMP_GCOV_DUMP);
    return 1;
}

int main() {
    int all_passed = 1;
    
    printf("=== Starting gcov-dump coverage tests ===\n\n");
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_gcov_dump()) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    printf("\nCreating minimal coverage file...\n");
    if (!create_minimal_gcda(TEMP_GCDA_FILE)) {
        printf("ERROR: Failed to create minimal .gcda file\n");
        unlink(TEMP_GCOV_DUMP);
        return 1;
    }
    printf("Created %s\n", TEMP_GCDA_FILE);
    
    /* Step 3: Execute test sequence */
    printf("\n=== Testing flag parsing ===\n");
    
    /* Test -h flag (help) - no file needed */
    all_passed &= test_flag("-h", NULL, 1, "Usage:");
    
    /* Test -v flag (version) - no file needed */
    all_passed &= test_flag("-v", NULL, 1, "gcov-dump");
    
    /* Test -l flag (dump contents) - requires file */
    all_passed &= test_flag("-l", TEMP_GCDA_FILE, 1, NULL);
    
    /* Test -p flag (dump positions) - requires file */
    all_passed &= test_flag("-p", TEMP_GCDA_FILE, 1, NULL);
    
    /* Test -r flag (dump raw) - requires file */
    all_passed &= test_flag("-r", TEMP_GCDA_FILE, 1, NULL);
    
    /* Test -s flag (dump stable) - requires file */
    all_passed &= test_flag("-s", TEMP_GCDA_FILE, 1, NULL);
    
    /* Test flag combinations */
    printf("\n=== Testing flag combinations ===\n");
    all_passed &= test_flag("-l -p", TEMP_GCDA_FILE, 1, NULL);
    all_passed &= test_flag("-p -l", TEMP_GCDA_FILE, 1, NULL);
    all_passed &= test_flag("-r -s", TEMP_GCDA_FILE, 1, NULL);
    all_passed &= test_flag("-s -r", TEMP_GCDA_FILE, 1, NULL);
    all_passed &= test_flag("-l -p -r", TEMP_GCDA_FILE, 1, NULL);
    
    /* Test invalid flag */
    printf("\n=== Testing invalid flag ===\n");
    all_passed &= test_flag("-X", TEMP_GCDA_FILE, 0, "unknown flag `X'");
    
    /* Test invalid flag without file */
    all_passed &= test_flag("-Z", NULL, 0, "unknown flag `Z'");
    
    /* Step 4: Cleanup */
    printf("\n=== Cleaning up ===\n");
    unlink(TEMP_GCDA_FILE);
    unlink(TEMP_GCOV_DUMP);
    
    /* Also remove coverage files created by instrumented binary */
    char coverage_files[][64] = {
        "gcov-dump-instrumented.gcda",
        "gcov-dump-instrumented.gcno",
        "gcov-dump.gcda",
        "gcov-dump.gcno"
    };
    
    for (size_t i = 0; i < sizeof(coverage_files)/sizeof(coverage_files[0]); i++) {
        if (unlink(coverage_files[i]) == 0) {
            printf("Removed %s\n", coverage_files[i]);
        }
    }
    
    printf("\n=== Test Summary ===\n");
    if (all_passed) {
        printf("All tests PASSED\n");
        return 0;
    } else {
        printf("Some tests FAILED\n");
        return 1;
    }
}
