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
    /* Version: 0x3430372a (gcov 4.7 format) */
    0x2a, 0x37, 0x30, 0x34,
    /* Stamp: 0x12345678 */
    0x78, 0x56, 0x34, 0x12,
    /* Length: 0 (no records) */
    0x00, 0x00, 0x00, 0x00
};

/* Build the instrumented gcov-dump binary */
int build_gcov_dump(const char *source_path) {
    char cmd[1024];
    struct stat st;
    
    printf("Building instrumented gcov-dump...\n");
    
    /* Check if source file exists */
    if (stat(source_path, &st) != 0) {
        fprintf(stderr, "Error: gcov-dump.cc not found at %s\n", source_path);
        return 0;
    }
    
    /* Compile with coverage instrumentation */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    printf("Compile command: %s\n", cmd);
    
    int result = system(cmd);
    if (result != 0) {
        fprintf(stderr, "Error: Failed to compile gcov-dump\n");
        return 0;
    }
    
    /* Verify the binary was created */
    if (stat(INSTRUMENTED_BINARY, &st) != 0) {
        fprintf(stderr, "Error: Instrumented binary not created\n");
        return 0;
    }
    
    printf("Successfully built instrumented gcov-dump\n");
    return 1;
}

/* Create a minimal valid .gcda file */
int create_minimal_gcda() {
    FILE *fp = fopen(TEMP_GCDA_FILE, "wb");
    if (!fp) {
        perror("Error creating temporary .gcda file");
        return 0;
    }
    
    fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    printf("Created minimal .gcda file at %s\n", TEMP_GCDA_FILE);
    return 1;
}

/* Execute gcov-dump with given arguments and capture output */
int execute_and_check(const char *args, int expect_success, 
                      const char *expected_output, int check_stderr) {
    char cmd[1024];
    char buffer[4096];
    FILE *fp;
    int found = 0;
    
    printf("\nTesting: gcov-dump %s\n", args);
    
    /* Build the full command */
    snprintf(cmd, sizeof(cmd), "%s %s 2>&1", INSTRUMENTED_BINARY, args);
    
    /* Execute and capture output */
    fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        return 0;
    }
    
    /* Read output */
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (expected_output && strstr(buffer, expected_output)) {
            found = 1;
        }
        printf("  Output: %s", buffer);
    }
    
    /* Get exit status */
    int status = pclose(fp);
    int exit_code = WEXITSTATUS(status);
    
    printf("  Exit code: %d\n", exit_code);
    
    /* Check expectations */
    if (expect_success) {
        if (exit_code != 0) {
            printf("  FAIL: Expected success (exit code 0), got %d\n", exit_code);
            return 0;
        }
    } else {
        if (exit_code == 0) {
            printf("  FAIL: Expected failure (non-zero exit code), got 0\n");
            return 0;
        }
    }
    
    if (expected_output && !found) {
        printf("  FAIL: Expected output not found: %s\n", expected_output);
        return 0;
    }
    
    printf("  PASS\n");
    return 1;
}

/* Test invalid flag specifically to check stderr message */
int test_invalid_flag() {
    char cmd[1024];
    char buffer[4096];
    FILE *fp;
    int found = 0;
    
    printf("\nTesting invalid flag -X...\n");
    
    /* Build command, redirecting stderr to stdout */
    snprintf(cmd, sizeof(cmd), "%s -X 2>&1", INSTRUMENTED_BINARY);
    
    fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        return 0;
    }
    
    /* Read output and look for the specific error message */
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("  Output: %s", buffer);
        if (strstr(buffer, "unknown flag `X'")) {
            found = 1;
        }
    }
    
    int status = pclose(fp);
    int exit_code = WEXITSTATUS(status);
    
    printf("  Exit code: %d\n", exit_code);
    
    if (exit_code == 0) {
        printf("  FAIL: Invalid flag should return non-zero exit code\n");
        return 0;
    }
    
    if (!found) {
        printf("  FAIL: 'unknown flag' message not found in output\n");
        return 0;
    }
    
    printf("  PASS\n");
    return 1;
}

int main(int argc, char *argv[]) {
    int all_passed = 1;
    char source_path[1024];
    
    printf("=== GCOV-Dump Coverage Test Driver ===\n");
    
    /* Try to find gcov-dump.cc */
    const char *possible_paths[] = {
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "../../../gcc/gcov-dump.cc",
        "gcov-dump.cc",
        NULL
    };
    
    const char **path = possible_paths;
    struct stat st;
    
    while (*path) {
        if (stat(*path, &st) == 0) {
            strcpy(source_path, *path);
            break;
        }
        path++;
    }
    
    if (!*path) {
        fprintf(stderr, "Error: Could not find gcov-dump.cc\n");
        fprintf(stderr, "Please run this test from GCC build directory\n");
        return 1;
    }
    
    printf("Found gcov-dump.cc at: %s\n", source_path);
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_gcov_dump(source_path)) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    if (!create_minimal_gcda()) {
        return 1;
    }
    
    /* Step 3: Execute test sequence */
    
    /* Test -h flag (help) */
    if (!execute_and_check("-h", 1, "Usage:", 0)) {
        all_passed = 0;
    }
    
    /* Test -v flag (version) */
    if (!execute_and_check("-v", 1, "gcov-dump", 0)) {
        all_passed = 0;
    }
    
    /* Test -l flag with minimal .gcda */
    if (!execute_and_check("-l " TEMP_GCDA_FILE, 1, NULL, 0)) {
        all_passed = 0;
    }
    
    /* Test -p flag with minimal .gcda */
    if (!execute_and_check("-p " TEMP_GCDA_FILE, 1, NULL, 0)) {
        all_passed = 0;
    }
    
    /* Test -r flag with minimal .gcda */
    if (!execute_and_check("-r " TEMP_GCDA_FILE, 1, NULL, 0)) {
        all_passed = 0;
    }
    
    /* Test -s flag with minimal .gcda */
    if (!execute_and_check("-s " TEMP_GCDA_FILE, 1, NULL, 0)) {
        all_passed = 0;
    }
    
    /* Test flag combinations */
    if (!execute_and_check("-l -p " TEMP_GCDA_FILE, 1, NULL, 0)) {
        all_passed = 0;
    }
    
    if (!execute_and_check("-p -l " TEMP_GCDA_FILE, 1, NULL, 0)) {
        all_passed = 0;
    }
    
    if (!execute_and_check("-r -s " TEMP_GCDA_FILE, 1, NULL, 0)) {
        all_passed = 0;
    }
    
    if (!execute_and_check("-s -r " TEMP_GCDA_FILE, 1, NULL, 0)) {
        all_passed = 0;
    }
    
    /* Test invalid flag */
    if (!test_invalid_flag()) {
        all_passed = 0;
    }
    
    /* Step 4: Cleanup */
    printf("\n=== Cleaning up ===\n");
    
    if (remove(TEMP_GCDA_FILE) == 0) {
        printf("Removed temporary .gcda file\n");
    }
    
    if (remove(INSTRUMENTED_BINARY) == 0) {
        printf("Removed instrumented binary\n");
    }
    
    /* Also remove coverage data files generated by instrumented binary */
    char coverage_files[][64] = {
        "gcov-dump-instrumented.gcda",
        "gcov-dump-instrumented.gcno",
        "gcov-dump.gcda",
        "gcov-dump.gcno"
    };
    
    for (size_t i = 0; i < sizeof(coverage_files)/sizeof(coverage_files[0]); i++) {
        if (remove(coverage_files[i]) == 0) {
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
