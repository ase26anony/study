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

/* Execute command and capture output */
char* capture_output(const char *cmd) {
    FILE *fp = popen(cmd, "r");
    if (!fp) return NULL;
    
    char buffer[4096];
    size_t total_size = 0;
    char *result = malloc(4096);
    result[0] = '\0';
    
    while (fgets(buffer, sizeof(buffer), fp)) {
        total_size += strlen(buffer);
        if (total_size >= 4096) {
            result = realloc(result, total_size + 4096);
        }
        strcat(result, buffer);
    }
    
    pclose(fp);
    return result;
}

/* Check if stderr contains specific text */
int check_stderr_contains(const char *cmd, const char *search_text) {
    char full_cmd[1024];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    char *output = capture_output(full_cmd);
    if (!output) return 0;
    
    int found = (strstr(output, search_text) != NULL);
    free(output);
    return found;
}

/* Create minimal GCOV data file */
int create_minimal_gcda() {
    FILE *fp = fopen(TEMP_GCDA_FILE, "wb");
    if (!fp) {
        perror("Failed to create temporary gcda file");
        return 0;
    }
    
    fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    printf("Created minimal gcda file: %s (%zu bytes)\n", 
           TEMP_GCDA_FILE, sizeof(minimal_gcda));
    return 1;
}

/* Build instrumented gcov-dump */
int build_gcov_dump() {
    printf("Building instrumented gcov-dump...\n");
    
    /* Try to find gcov-dump.cc in common locations */
    const char *possible_paths[] = {
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "../../../gcc/gcov-dump.cc",
        "/usr/src/gcc/gcc/gcov-dump.cc",
        NULL
    };
    
    const char *source_path = NULL;
    for (int i = 0; possible_paths[i]; i++) {
        if (access(possible_paths[i], R_OK) == 0) {
            source_path = possible_paths[i];
            break;
        }
    }
    
    if (!source_path) {
        /* Try current directory */
        if (access("gcov-dump.cc", R_OK) == 0) {
            source_path = "gcov-dump.cc";
        } else {
            fprintf(stderr, "Could not find gcov-dump.cc\n");
            return 0;
        }
    }
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Failed to build gcov-dump\n");
        return 0;
    }
    
    printf("Built instrumented gcov-dump at: %s\n", INSTRUMENTED_BINARY);
    return 1;
}

int main() {
    printf("=== Starting gcov-dump coverage test ===\n");
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_gcov_dump()) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    if (!create_minimal_gcda()) {
        unlink(INSTRUMENTED_BINARY);
        return 1;
    }
    
    int all_tests_passed = 1;
    
    /* Step 3: Execute test sequence */
    
    /* Test -h flag (help) */
    printf("\n=== Testing -h flag ===\n");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s -h", INSTRUMENTED_BINARY);
    int status = execute_command(cmd);
    if (status != 0) {
        printf("FAIL: -h flag returned non-zero exit code: %d\n", status);
        all_tests_passed = 0;
    } else {
        printf("PASS: -h flag exited successfully\n");
    }
    
    /* Test -v flag (version) */
    printf("\n=== Testing -v flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -v", INSTRUMENTED_BINARY);
    char *output = capture_output(cmd);
    if (output && strstr(output, "gcov-dump")) {
        printf("PASS: -v flag shows version info\n");
        printf("Output: %s", output);
    } else {
        printf("FAIL: -v flag didn't show expected version info\n");
        all_tests_passed = 0;
    }
    free(output);
    
    /* Test -l flag (dump contents) */
    printf("\n=== Testing -l flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    status = execute_command(cmd);
    if (status != 0) {
        printf("FAIL: -l flag returned non-zero exit code: %d\n", status);
        all_tests_passed = 0;
    } else {
        printf("PASS: -l flag executed successfully\n");
    }
    
    /* Test -p flag (dump positions) */
    printf("\n=== Testing -p flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    status = execute_command(cmd);
    if (status != 0) {
        printf("FAIL: -p flag returned non-zero exit code: %d\n", status);
        all_tests_passed = 0;
    } else {
        printf("PASS: -p flag executed successfully\n");
    }
    
    /* Test -r flag (dump raw) */
    printf("\n=== Testing -r flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -r %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    status = execute_command(cmd);
    if (status != 0) {
        printf("FAIL: -r flag returned non-zero exit code: %d\n", status);
        all_tests_passed = 0;
    } else {
        printf("PASS: -r flag executed successfully\n");
    }
    
    /* Test -s flag (dump stable) */
    printf("\n=== Testing -s flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    status = execute_command(cmd);
    if (status != 0) {
        printf("FAIL: -s flag returned non-zero exit code: %d\n", status);
        all_tests_passed = 0;
    } else {
        printf("PASS: -s flag executed successfully\n");
    }
    
    /* Test flag combinations */
    printf("\n=== Testing flag combinations ===\n");
    
    /* -l -p combination */
    snprintf(cmd, sizeof(cmd), "%s -l -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    status = execute_command(cmd);
    if (status != 0) {
        printf("FAIL: -l -p combination failed: %d\n", status);
        all_tests_passed = 0;
    } else {
        printf("PASS: -l -p combination executed successfully\n");
    }
    
    /* -p -l combination (different order) */
    snprintf(cmd, sizeof(cmd), "%s -p -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    status = execute_command(cmd);
    if (status != 0) {
        printf("FAIL: -p -l combination failed: %d\n", status);
        all_tests_passed = 0;
    } else {
        printf("PASS: -p -l combination executed successfully\n");
    }
    
    /* -r -s combination */
    snprintf(cmd, sizeof(cmd), "%s -r -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    status = execute_command(cmd);
    if (status != 0) {
        printf("FAIL: -r -s combination failed: %d\n", status);
        all_tests_passed = 0;
    } else {
        printf("PASS: -r -s combination executed successfully\n");
    }
    
    /* Test invalid flag */
    printf("\n=== Testing invalid flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -X %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    
    if (check_stderr_contains(cmd, "unknown flag `X'")) {
        printf("PASS: Invalid flag correctly detected\n");
    } else {
        printf("FAIL: Invalid flag error message not found\n");
        all_tests_passed = 0;
    }
    
    /* Also test invalid flag without file argument */
    printf("\n=== Testing invalid flag without file ===\n");
    snprintf(cmd, sizeof(cmd), "%s -X", INSTRUMENTED_BINARY);
    
    if (check_stderr_contains(cmd, "unknown flag `X'")) {
        printf("PASS: Invalid flag without file correctly detected\n");
    } else {
        printf("FAIL: Invalid flag error message not found\n");
        all_tests_passed = 0;
    }
    
    /* Step 4: Cleanup */
    printf("\n=== Cleaning up ===\n");
    unlink(TEMP_GCDA_FILE);
    unlink(INSTRUMENTED_BINARY);
    
    /* Also remove coverage data files generated by instrumented binary */
    char gcda_file[1024];
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", INSTRUMENTED_BINARY);
    unlink(gcda_file);
    
    char gcno_file[1024];
    snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", INSTRUMENTED_BINARY);
    unlink(gcno_file);
    
    printf("Removed temporary files\n");
    
    if (all_tests_passed) {
        printf("\n=== ALL TESTS PASSED ===\n");
        return 0;
    } else {
        printf("\n=== SOME TESTS FAILED ===\n");
        return 1;
    }
}
