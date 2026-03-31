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
    /* Version: 0x76312e2a (version 1.12*) */
    0x2a, 0x2e, 0x31, 0x76,
    /* Stamp: 0x00000000 */
    0x00, 0x00, 0x00, 0x00,
    /* Record type 0 (GCOV_TAG_FUNCTION) with length 0 */
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    /* Record type 3 (GCOV_TAG_OBJECT_SUMMARY) with length 9 */
    0x03, 0x00, 0x00, 0x00,
    0x09, 0x00, 0x00, 0x00,
    /* Summary data: runs=1, sum_max=0 */
    0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

/* Execute command and return exit status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Execute command and capture output to buffer */
int execute_and_capture(const char *cmd, char *buffer, size_t buf_size, int capture_stderr) {
    char full_cmd[1024];
    if (capture_stderr) {
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    } else {
        snprintf(full_cmd, sizeof(full_cmd), "%s", cmd);
    }
    
    printf("Executing: %s\n", full_cmd);
    
    FILE *fp = popen(full_cmd, "r");
    if (!fp) {
        return -1;
    }
    
    buffer[0] = '\0';
    size_t total_read = 0;
    while (fgets(buffer + total_read, buf_size - total_read, fp) != NULL) {
        total_read = strlen(buffer);
        if (total_read >= buf_size - 1) {
            break;
        }
    }
    
    int status = pclose(fp);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Create minimal valid .gcda file */
int create_minimal_gcda() {
    FILE *fp = fopen(TEMP_GCDA_FILE, "wb");
    if (!fp) {
        perror("Failed to create temporary .gcda file");
        return 0;
    }
    
    size_t written = fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    if (written != sizeof(minimal_gcda)) {
        fprintf(stderr, "Failed to write complete .gcda file\n");
        return 0;
    }
    
    printf("Created minimal .gcda file: %s (%zu bytes)\n", TEMP_GCDA_FILE, sizeof(minimal_gcda));
    return 1;
}

/* Build instrumented gcov-dump */
int build_gcov_dump() {
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
    
    for (int i = 0; possible_paths[i] != NULL; i++) {
        if (stat(possible_paths[i], &st) == 0 && S_ISREG(st.st_mode)) {
            source_path = possible_paths[i];
            break;
        }
    }
    
    if (!source_path) {
        /* Try current directory */
        if (stat("gcov-dump.cc", &st) == 0 && S_ISREG(st.st_mode)) {
            source_path = "gcov-dump.cc";
        } else {
            fprintf(stderr, "Could not find gcov-dump.cc\n");
            return 0;
        }
    }
    
    printf("Found gcov-dump.cc at: %s\n", source_path);
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    printf("Building instrumented gcov-dump...\n");
    int result = execute_command(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Failed to build gcov-dump\n");
        return 0;
    }
    
    if (stat(INSTRUMENTED_BINARY, &st) != 0 || !S_ISREG(st.st_mode)) {
        fprintf(stderr, "Instrumented binary not created\n");
        return 0;
    }
    
    printf("Successfully built instrumented gcov-dump: %s\n", INSTRUMENTED_BINARY);
    return 1;
}

/* Test invalid flag */
int test_invalid_flag() {
    printf("\n=== Testing invalid flag ===\n");
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s -X", INSTRUMENTED_BINARY);
    
    char output[1024];
    int status = execute_and_capture(cmd, output, sizeof(output), 1);
    
    printf("Exit status: %d\n", status);
    printf("Output:\n%s\n", output);
    
    /* Check for expected error message */
    if (strstr(output, "unknown flag `X'") != NULL) {
        printf("✓ Correctly detected invalid flag\n");
        return 1;
    } else {
        printf("✗ Did not detect invalid flag correctly\n");
        return 0;
    }
}

/* Test -h flag */
int test_help_flag() {
    printf("\n=== Testing -h flag ===\n");
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s -h", INSTRUMENTED_BINARY);
    
    int status = execute_command(cmd);
    printf("Exit status: %d\n", status);
    
    if (status == 0) {
        printf("✓ -h flag executed successfully\n");
        return 1;
    } else {
        printf("✗ -h flag failed\n");
        return 0;
    }
}

/* Test -v flag */
int test_version_flag() {
    printf("\n=== Testing -v flag ===\n");
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s -v", INSTRUMENTED_BINARY);
    
    char output[1024];
    int status = execute_and_capture(cmd, output, sizeof(output), 0);
    
    printf("Exit status: %d\n", status);
    printf("Output (first 100 chars):\n%.100s\n", output);
    
    if (status == 0 && strlen(output) > 0) {
        printf("✓ -v flag printed version information\n");
        return 1;
    } else {
        printf("✗ -v flag failed\n");
        return 0;
    }
}

/* Test flag with coverage file */
int test_flag_with_file(const char *flag, const char *description) {
    printf("\n=== Testing %s flag ===\n", description);
    
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flag, TEMP_GCDA_FILE);
    
    int status = execute_command(cmd);
    printf("Exit status: %d\n", status);
    
    if (status == 0) {
        printf("✓ %s flag executed successfully\n", description);
        return 1;
    } else {
        printf("✗ %s flag failed\n", description);
        return 0;
    }
}

/* Test combined flags */
int test_combined_flags(const char *flags, const char *description) {
    printf("\n=== Testing combined flags: %s ===\n", description);
    
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flags, TEMP_GCDA_FILE);
    
    int status = execute_command(cmd);
    printf("Exit status: %d\n", status);
    
    if (status == 0) {
        printf("✓ Combined flags %s executed successfully\n", description);
        return 1;
    } else {
        printf("✗ Combined flags %s failed\n", description);
        return 0;
    }
}

/* Cleanup temporary files */
void cleanup() {
    printf("\n=== Cleaning up ===\n");
    
    if (remove(TEMP_GCDA_FILE) == 0) {
        printf("Removed: %s\n", TEMP_GCDA_FILE);
    }
    
    if (remove(INSTRUMENTED_BINARY) == 0) {
        printf("Removed: %s\n", INSTRUMENTED_BINARY);
    }
}

int main() {
    printf("=== GCOV-Dump Coverage Test Program ===\n");
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_gcov_dump()) {
        fprintf(stderr, "Failed to build gcov-dump. Exiting.\n");
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    if (!create_minimal_gcda()) {
        fprintf(stderr, "Failed to create minimal .gcda file. Exiting.\n");
        return 1;
    }
    
    int all_tests_passed = 1;
    
    /* Step 3: Execute test sequence */
    
    /* Test flags without file arguments */
    all_tests_passed &= test_help_flag();
    all_tests_passed &= test_version_flag();
    
    /* Test invalid flag */
    all_tests_passed &= test_invalid_flag();
    
    /* Test flags requiring coverage file */
    all_tests_passed &= test_flag_with_file("-l", "-l (dump contents)");
    all_tests_passed &= test_flag_with_file("-p", "-p (dump positions)");
    all_tests_passed &= test_flag_with_file("-r", "-r (dump raw)");
    all_tests_passed &= test_flag_with_file("-s", "-s (dump stable)");
    
    /* Test flag combinations */
    all_tests_passed &= test_combined_flags("-l -p", "-l -p");
    all_tests_passed &= test_combined_flags("-p -l", "-p -l (reverse order)");
    all_tests_passed &= test_combined_flags("-r -s", "-r -s");
    all_tests_passed &= test_combined_flags("-l -p -r -s", "-l -p -r -s (all flags)");
    
    /* Step 4: Cleanup */
    cleanup();
    
    printf("\n=== Test Summary ===\n");
    if (all_tests_passed) {
        printf("✓ All tests passed!\n");
        return 0;
    } else {
        printf("✗ Some tests failed\n");
        return 1;
    }
}
