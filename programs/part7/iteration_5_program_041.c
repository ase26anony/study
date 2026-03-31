#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define TEMP_GCDA "/tmp/minimal.gcda"
#define TEMP_GCOV_DUMP "/tmp/gcov-dump-instrumented"

/* Minimal GCOV data file format constants */
#define GCOV_DATA_MAGIC 0x67636461  /* "gcda" */
#define GCOV_VERSION 0x3430372A     /* "407*" */

/* Write a minimal valid .gcda file */
int create_minimal_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create minimal .gcda file");
        return 0;
    }
    
    /* Write GCOV magic number */
    unsigned magic = GCOV_DATA_MAGIC;
    fwrite(&magic, sizeof(unsigned), 1, fp);
    
    /* Write version */
    unsigned version = GCOV_VERSION;
    fwrite(&version, sizeof(unsigned), 1, fp);
    
    /* Write zero-length record (tag 0, length 0) to indicate end */
    unsigned zero_tag = 0;
    unsigned zero_length = 0;
    fwrite(&zero_tag, sizeof(unsigned), 1, fp);
    fwrite(&zero_length, sizeof(unsigned), 1, fp);
    
    fclose(fp);
    return 1;
}

/* Execute command and capture output */
int execute_and_capture(const char *cmd, char *output, size_t output_size, 
                        int capture_stderr) {
    char full_cmd[1024];
    if (capture_stderr) {
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    } else {
        snprintf(full_cmd, sizeof(full_cmd), "%s", cmd);
    }
    
    FILE *fp = popen(full_cmd, "r");
    if (!fp) {
        return -1;
    }
    
    output[0] = '\0';
    size_t total_read = 0;
    while (fgets(output + total_read, output_size - total_read, fp)) {
        total_read = strlen(output);
        if (total_read >= output_size - 1) break;
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
}

/* Check if string contains substring */
int contains_string(const char *str, const char *substr) {
    return strstr(str, substr) != NULL;
}

int main() {
    printf("=== Starting gcov-dump coverage test ===\n");
    
    /* Step 1: Build instrumented gcov-dump */
    printf("1. Building instrumented gcov-dump...\n");
    
    /* Try to find gcov-dump.cc in common locations */
    const char *possible_paths[] = {
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "/usr/src/gcc/gcc/gcov-dump.cc",
        "gcov-dump.cc",
        NULL
    };
    
    const char *gcov_dump_cc = NULL;
    for (int i = 0; possible_paths[i]; i++) {
        if (access(possible_paths[i], R_OK) == 0) {
            gcov_dump_cc = possible_paths[i];
            break;
        }
    }
    
    if (!gcov_dump_cc) {
        fprintf(stderr, "Error: Could not find gcov-dump.cc\n");
        return 1;
    }
    
    printf("   Found source at: %s\n", gcov_dump_cc);
    
    char compile_cmd[1024];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             TEMP_GCOV_DUMP, gcov_dump_cc);
    
    printf("   Compiling with: %s\n", compile_cmd);
    
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Error: Failed to compile gcov-dump\n");
        return 1;
    }
    
    if (access(TEMP_GCOV_DUMP, X_OK) != 0) {
        fprintf(stderr, "Error: Compiled binary not found\n");
        return 1;
    }
    
    printf("   Instrumented binary created: %s\n", TEMP_GCOV_DUMP);
    
    /* Step 2: Create minimal coverage file */
    printf("\n2. Creating minimal .gcda file...\n");
    if (!create_minimal_gcda(TEMP_GCDA)) {
        return 1;
    }
    printf("   Created: %s\n", TEMP_GCDA);
    
    /* Step 3: Execute test sequence */
    printf("\n3. Executing test sequence...\n");
    
    int all_tests_passed = 1;
    char output[4096];
    int exit_status;
    
    /* Test -h flag (help) */
    printf("   Testing -h flag...\n");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s -h", TEMP_GCOV_DUMP);
    exit_status = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_status != 0) {
        printf("   FAIL: -h flag returned non-zero exit code: %d\n", exit_status);
        all_tests_passed = 0;
    } else {
        printf("   PASS: -h flag executed successfully\n");
    }
    
    /* Test -v flag (version) */
    printf("   Testing -v flag...\n");
    snprintf(cmd, sizeof(cmd), "%s -v", TEMP_GCOV_DUMP);
    exit_status = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_status != 0) {
        printf("   FAIL: -v flag returned non-zero exit code: %d\n", exit_status);
        all_tests_passed = 0;
    } else if (strlen(output) == 0) {
        printf("   FAIL: -v flag produced no output\n");
        all_tests_passed = 0;
    } else {
        printf("   PASS: -v flag printed version info\n");
    }
    
    /* Test -l flag (dump contents) */
    printf("   Testing -l flag...\n");
    snprintf(cmd, sizeof(cmd), "%s -l %s", TEMP_GCOV_DUMP, TEMP_GCDA);
    exit_status = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_status != 0) {
        printf("   FAIL: -l flag returned non-zero exit code: %d\n", exit_status);
        all_tests_passed = 0;
    } else {
        printf("   PASS: -l flag executed successfully\n");
    }
    
    /* Test -p flag (dump positions) */
    printf("   Testing -p flag...\n");
    snprintf(cmd, sizeof(cmd), "%s -p %s", TEMP_GCOV_DUMP, TEMP_GCDA);
    exit_status = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_status != 0) {
        printf("   FAIL: -p flag returned non-zero exit code: %d\n", exit_status);
        all_tests_passed = 0;
    } else {
        printf("   PASS: -p flag executed successfully\n");
    }
    
    /* Test -r flag (dump raw) */
    printf("   Testing -r flag...\n");
    snprintf(cmd, sizeof(cmd), "%s -r %s", TEMP_GCOV_DUMP, TEMP_GCDA);
    exit_status = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_status != 0) {
        printf("   FAIL: -r flag returned non-zero exit code: %d\n", exit_status);
        all_tests_passed = 0;
    } else {
        printf("   PASS: -r flag executed successfully\n");
    }
    
    /* Test -s flag (dump stable) */
    printf("   Testing -s flag...\n");
    snprintf(cmd, sizeof(cmd), "%s -s %s", TEMP_GCOV_DUMP, TEMP_GCDA);
    exit_status = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_status != 0) {
        printf("   FAIL: -s flag returned non-zero exit code: %d\n", exit_status);
        all_tests_passed = 0;
    } else {
        printf("   PASS: -s flag executed successfully\n");
    }
    
    /* Test flag combinations */
    printf("   Testing -l -p combination...\n");
    snprintf(cmd, sizeof(cmd), "%s -l -p %s", TEMP_GCOV_DUMP, TEMP_GCDA);
    exit_status = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_status != 0) {
        printf("   FAIL: -l -p combination failed\n");
        all_tests_passed = 0;
    } else {
        printf("   PASS: -l -p combination executed successfully\n");
    }
    
    printf("   Testing -p -l combination (reverse order)...\n");
    snprintf(cmd, sizeof(cmd), "%s -p -l %s", TEMP_GCOV_DUMP, TEMP_GCDA);
    exit_status = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_status != 0) {
        printf("   FAIL: -p -l combination failed\n");
        all_tests_passed = 0;
    } else {
        printf("   PASS: -p -l combination executed successfully\n");
    }
    
    printf("   Testing -r -s combination...\n");
    snprintf(cmd, sizeof(cmd), "%s -r -s %s", TEMP_GCOV_DUMP, TEMP_GCDA);
    exit_status = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_status != 0) {
        printf("   FAIL: -r -s combination failed\n");
        all_tests_passed = 0;
    } else {
        printf("   PASS: -r -s combination executed successfully\n");
    }
    
    /* Test invalid flag */
    printf("   Testing invalid flag -X...\n");
    snprintf(cmd, sizeof(cmd), "%s -X %s", TEMP_GCOV_DUMP, TEMP_GCDA);
    exit_status = execute_and_capture(cmd, output, sizeof(output), 1);
    
    if (!contains_string(output, "unknown flag")) {
        printf("   FAIL: Expected 'unknown flag' error message\n");
        printf("   Output was: %s\n", output);
        all_tests_passed = 0;
    } else if (!contains_string(output, "`X'")) {
        printf("   FAIL: Expected flag 'X' in error message\n");
        printf("   Output was: %s\n", output);
        all_tests_passed = 0;
    } else {
        printf("   PASS: Invalid flag correctly rejected\n");
    }
    
    /* Test invalid flag without file argument */
    printf("   Testing invalid flag -Y (no file)...\n");
    snprintf(cmd, sizeof(cmd), "%s -Y", TEMP_GCOV_DUMP);
    exit_status = execute_and_capture(cmd, output, sizeof(output), 1);
    
    if (!contains_string(output, "unknown flag")) {
        printf("   FAIL: Expected 'unknown flag' error message\n");
        printf("   Output was: %s\n", output);
        all_tests_passed = 0;
    } else {
        printf("   PASS: Invalid flag correctly rejected without file\n");
    }
    
    /* Step 4: Cleanup */
    printf("\n4. Cleaning up temporary files...\n");
    unlink(TEMP_GCDA);
    unlink(TEMP_GCOV_DUMP);
    printf("   Removed: %s\n", TEMP_GCDA);
    printf("   Removed: %s\n", TEMP_GCOV_DUMP);
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    if (all_tests_passed) {
        printf("All tests PASSED!\n");
        printf("The uncovered lines in gcov-dump.cc should now be covered.\n");
        printf("Run 'gcov gcov-dump.cc' to see coverage results.\n");
        return 0;
    } else {
        printf("Some tests FAILED!\n");
        return 1;
    }
}
