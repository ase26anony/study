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
    /* Summary data: runs=0, sum_all=0, run_max=0, sum_max=0 */
    0x00, 0x00, 0x00, 0x00,
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

/* Capture stderr from command and check for expected string */
int check_stderr_contains(const char *cmd, const char *expected) {
    char buffer[1024];
    FILE *fp;
    int found = 0;
    
    /* Create a command that redirects stderr to stdout */
    char full_cmd[2048];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    printf("Executing (capturing stderr): %s\n", cmd);
    fp = popen(full_cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return 0;
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, expected) != NULL) {
            found = 1;
            break;
        }
    }
    
    pclose(fp);
    return found;
}

/* Capture stdout from command */
char* capture_stdout(const char *cmd) {
    static char buffer[4096];
    FILE *fp;
    size_t total = 0;
    
    buffer[0] = '\0';
    
    printf("Executing (capturing stdout): %s\n", cmd);
    fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return NULL;
    }
    
    while (fgets(buffer + total, sizeof(buffer) - total, fp) != NULL) {
        total = strlen(buffer);
        if (total >= sizeof(buffer) - 1) break;
    }
    
    pclose(fp);
    return buffer;
}

/* Create minimal .gcda file */
int create_minimal_gcda() {
    FILE *fp = fopen(TEMP_GCDA_FILE, "wb");
    if (fp == NULL) {
        perror("Failed to create temporary .gcda file");
        return 0;
    }
    
    size_t written = fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    if (written != sizeof(minimal_gcda)) {
        fprintf(stderr, "Failed to write complete .gcda file\n");
        return 0;
    }
    
    printf("Created minimal .gcda file: %s (%zu bytes)\n", 
           TEMP_GCDA_FILE, sizeof(minimal_gcda));
    return 1;
}

/* Build instrumented gcov-dump */
int build_instrumented_gcov_dump() {
    char cmd[1024];
    struct stat st;
    
    /* First check if gcov-dump.cc exists in common locations */
    const char *possible_paths[] = {
        "../../gcc/gcov-dump.cc",
        "../gcc/gcov-dump.cc",
        "gcc/gcov-dump.cc",
        "../../../gcc/gcov-dump.cc",
        NULL
    };
    
    const char *source_path = NULL;
    for (int i = 0; possible_paths[i] != NULL; i++) {
        if (stat(possible_paths[i], &st) == 0) {
            source_path = possible_paths[i];
            break;
        }
    }
    
    if (source_path == NULL) {
        /* Try to find it using find command */
        FILE *fp = popen("find . -name 'gcov-dump.cc' -type f 2>/dev/null | head -1", "r");
        if (fp) {
            char path[256];
            if (fgets(path, sizeof(path), fp)) {
                /* Remove newline */
                path[strcspn(path, "\n")] = 0;
                if (strlen(path) > 0) {
                    source_path = strdup(path);
                }
            }
            pclose(fp);
        }
    }
    
    if (source_path == NULL) {
        fprintf(stderr, "Could not find gcov-dump.cc\n");
        return 0;
    }
    
    printf("Found gcov-dump.cc at: %s\n", source_path);
    
    /* Build instrumented version */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    printf("Building instrumented gcov-dump...\n");
    int status = system(cmd);
    
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("Successfully built instrumented gcov-dump\n");
        return 1;
    } else {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 0;
    }
}

int main() {
    int all_tests_passed = 1;
    char cmd[1024];
    
    printf("=== Starting gcov-dump coverage tests ===\n\n");
    
    /* Step 1: Build instrumented gcov-dump */
    printf("1. Building instrumented gcov-dump...\n");
    if (!build_instrumented_gcov_dump()) {
        fprintf(stderr, "Failed to build instrumented gcov-dump. Exiting.\n");
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    printf("\n2. Creating minimal .gcda file...\n");
    if (!create_minimal_gcda()) {
        fprintf(stderr, "Failed to create minimal .gcda file. Exiting.\n");
        return 1;
    }
    
    /* Step 3: Execute test sequence */
    printf("\n3. Executing test sequence...\n");
    
    /* Test -h flag (help) */
    printf("\n--- Testing -h flag (help) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -h", INSTRUMENTED_BINARY);
    int exit_status = execute_command(cmd);
    if (exit_status != 0) {
        printf("FAIL: -h flag returned non-zero exit status: %d\n", exit_status);
        all_tests_passed = 0;
    } else {
        printf("PASS: -h flag executed successfully\n");
    }
    
    /* Test -v flag (version) */
    printf("\n--- Testing -v flag (version) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -v", INSTRUMENTED_BINARY);
    char *output = capture_stdout(cmd);
    if (output && strlen(output) > 0) {
        printf("Version output (first line): %.*s\n", 
               (int)strcspn(output, "\n"), output);
        printf("PASS: -v flag printed version information\n");
    } else {
        printf("FAIL: -v flag produced no output\n");
        all_tests_passed = 0;
    }
    
    /* Test -l flag (dump contents) */
    printf("\n--- Testing -l flag (dump contents) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_status = execute_command(cmd);
    if (exit_status != 0) {
        printf("FAIL: -l flag returned non-zero exit status: %d\n", exit_status);
        all_tests_passed = 0;
    } else {
        printf("PASS: -l flag executed successfully\n");
    }
    
    /* Test -p flag (dump positions) */
    printf("\n--- Testing -p flag (dump positions) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_status = execute_command(cmd);
    if (exit_status != 0) {
        printf("FAIL: -p flag returned non-zero exit status: %d\n", exit_status);
        all_tests_passed = 0;
    } else {
        printf("PASS: -p flag executed successfully\n");
    }
    
    /* Test -r flag (dump raw) */
    printf("\n--- Testing -r flag (dump raw) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -r %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_status = execute_command(cmd);
    if (exit_status != 0) {
        printf("FAIL: -r flag returned non-zero exit status: %d\n", exit_status);
        all_tests_passed = 0;
    } else {
        printf("PASS: -r flag executed successfully\n");
    }
    
    /* Test -s flag (dump stable) */
    printf("\n--- Testing -s flag (dump stable) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_status = execute_command(cmd);
    if (exit_status != 0) {
        printf("FAIL: -s flag returned non-zero exit status: %d\n", exit_status);
        all_tests_passed = 0;
    } else {
        printf("PASS: -s flag executed successfully\n");
    }
    
    /* Test flag combinations */
    printf("\n--- Testing flag combinations ---\n");
    
    /* -l -p combination */
    printf("\nTesting -l -p combination...\n");
    snprintf(cmd, sizeof(cmd), "%s -l -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_status = execute_command(cmd);
    if (exit_status != 0) {
        printf("FAIL: -l -p combination returned non-zero exit status: %d\n", exit_status);
        all_tests_passed = 0;
    } else {
        printf("PASS: -l -p combination executed successfully\n");
    }
    
    /* -p -l combination (different order) */
    printf("\nTesting -p -l combination (different order)...\n");
    snprintf(cmd, sizeof(cmd), "%s -p -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_status = execute_command(cmd);
    if (exit_status != 0) {
        printf("FAIL: -p -l combination returned non-zero exit status: %d\n", exit_status);
        all_tests_passed = 0;
    } else {
        printf("PASS: -p -l combination executed successfully\n");
    }
    
    /* -r -s combination */
    printf("\nTesting -r -s combination...\n");
    snprintf(cmd, sizeof(cmd), "%s -r -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_status = execute_command(cmd);
    if (exit_status != 0) {
        printf("FAIL: -r -s combination returned non-zero exit status: %d\n", exit_status);
        all_tests_passed = 0;
    } else {
        printf("PASS: -r -s combination executed successfully\n");
    }
    
    /* Test invalid flag */
    printf("\n--- Testing invalid flag (-X) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -X %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    int found_error = check_stderr_contains(cmd, "unknown flag `X'");
    if (found_error) {
        printf("PASS: Invalid flag correctly detected and error message printed\n");
    } else {
        printf("FAIL: Invalid flag error message not found\n");
        all_tests_passed = 0;
    }
    
    /* Test invalid flag without file argument */
    printf("\n--- Testing invalid flag (-X) without file ---\n");
    snprintf(cmd, sizeof(cmd), "%s -X", INSTRUMENTED_BINARY);
    found_error = check_stderr_contains(cmd, "unknown flag `X'");
    if (found_error) {
        printf("PASS: Invalid flag without file correctly detected\n");
    } else {
        printf("FAIL: Invalid flag without file error message not found\n");
        all_tests_passed = 0;
    }
    
    /* Step 4: Cleanup */
    printf("\n4. Cleaning up temporary files...\n");
    unlink(TEMP_GCDA_FILE);
    unlink(INSTRUMENTED_BINARY);
    
    /* Also clean up coverage files generated by instrumented binary */
    char coverage_files[1024];
    snprintf(coverage_files, sizeof(coverage_files), 
             "rm -f %s.gcno %s.gcda *.gcno *.gcda 2>/dev/null", 
             INSTRUMENTED_BINARY, INSTRUMENTED_BINARY);
    system(coverage_files);
    
    printf("\n=== Test Summary ===\n");
    if (all_tests_passed) {
        printf("All tests PASSED!\n");
        return 0;
    } else {
        printf("Some tests FAILED!\n");
        return 1;
    }
}
