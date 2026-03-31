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
    /* Version: 0xB3C1F4D5 (example version) */
    0xD5, 0xF4, 0xC1, 0xB3,
    /* Stamp: 0x12345678 */
    0x78, 0x56, 0x34, 0x12,
    /* Length of first record: 0 (empty function) */
    0x00, 0x00, 0x00, 0x00
};

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
    
    size_t total_read = 0;
    while (fgets(output + total_read, output_size - total_read, fp) != NULL) {
        total_read = strlen(output);
        if (total_read >= output_size - 1) {
            break;
        }
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
}

/* Check if string contains substring */
int contains_string(const char *str, const char *substr) {
    return strstr(str, substr) != NULL;
}

/* Create minimal valid .gcda file */
int create_minimal_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create .gcda file");
        return 0;
    }
    
    size_t written = fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    return written == sizeof(minimal_gcda);
}

/* Build instrumented gcov-dump */
int build_instrumented_gcov_dump(const char *source_path) {
    char cmd[2048];
    
    /* Check if source file exists */
    struct stat st;
    if (stat(source_path, &st) != 0) {
        fprintf(stderr, "Source file not found: %s\n", source_path);
        return 0;
    }
    
    /* Compile with coverage instrumentation */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    printf("Building instrumented gcov-dump: %s\n", cmd);
    
    int result = system(cmd);
    if (result != 0) {
        fprintf(stderr, "Failed to compile gcov-dump\n");
        return 0;
    }
    
    /* Verify the binary was created */
    if (stat(INSTRUMENTED_BINARY, &st) != 0) {
        fprintf(stderr, "Instrumented binary not created\n");
        return 0;
    }
    
    return 1;
}

int main(int argc, char *argv[]) {
    char output[4096];
    int exit_code;
    int all_tests_passed = 1;
    
    /* Try to locate gcov-dump.cc */
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
    
    printf("Found source at: %s\n", source_path);
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_instrumented_gcov_dump(source_path)) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    printf("Creating minimal .gcda file...\n");
    if (!create_minimal_gcda(TEMP_GCDA_FILE)) {
        fprintf(stderr, "Failed to create minimal .gcda file\n");
        unlink(INSTRUMENTED_BINARY);
        return 1;
    }
    
    /* Step 3: Execute test sequence */
    
    /* Test -h flag (help) */
    printf("\n=== Testing -h flag ===\n");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s -h", INSTRUMENTED_BINARY);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("✓ -h flag test passed (exit code: %d)\n", exit_code);
    } else {
        printf("✗ -h flag test failed (exit code: %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test -v flag (version) */
    printf("\n=== Testing -v flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -v", INSTRUMENTED_BINARY);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_code == 0 && contains_string(output, "gcov-dump")) {
        printf("✓ -v flag test passed\n");
        printf("Output: %s\n", output);
    } else {
        printf("✗ -v flag test failed\n");
        printf("Output: %s\n", output);
        all_tests_passed = 0;
    }
    
    /* Test -l flag (dump contents) */
    printf("\n=== Testing -l flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    printf("-l flag exit code: %d\n", exit_code);
    
    /* Test -p flag (dump positions) */
    printf("\n=== Testing -p flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    printf("-p flag exit code: %d\n", exit_code);
    
    /* Test -r flag (dump raw) */
    printf("\n=== Testing -r flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -r %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    printf("-r flag exit code: %d\n", exit_code);
    
    /* Test -s flag (dump stable) */
    printf("\n=== Testing -s flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    printf("-s flag exit code: %d\n", exit_code);
    
    /* Test flag combinations */
    printf("\n=== Testing flag combinations ===\n");
    
    /* -l -p combination */
    snprintf(cmd, sizeof(cmd), "%s -l -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    printf("-l -p exit code: %d\n", exit_code);
    
    /* -p -l combination (different order) */
    snprintf(cmd, sizeof(cmd), "%s -p -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    printf("-p -l exit code: %d\n", exit_code);
    
    /* -r -s combination */
    snprintf(cmd, sizeof(cmd), "%s -r -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    printf("-r -s exit code: %d\n", exit_code);
    
    /* Test invalid flag -X */
    printf("\n=== Testing invalid flag -X ===\n");
    snprintf(cmd, sizeof(cmd), "%s -X %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1);
    
    if (contains_string(output, "unknown flag") && contains_string(output, "X")) {
        printf("✓ Invalid flag test passed\n");
        printf("Error message: %s\n", output);
    } else {
        printf("✗ Invalid flag test failed\n");
        printf("Output: %s\n", output);
        all_tests_passed = 0;
    }
    
    /* Test invalid flag without file argument */
    printf("\n=== Testing invalid flag -X (no file) ===\n");
    snprintf(cmd, sizeof(cmd), "%s -X", INSTRUMENTED_BINARY);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1);
    
    if (contains_string(output, "unknown flag") && contains_string(output, "X")) {
        printf("✓ Invalid flag (no file) test passed\n");
        printf("Error message: %s\n", output);
    } else {
        printf("✗ Invalid flag (no file) test failed\n");
        printf("Output: %s\n", output);
        all_tests_passed = 0;
    }
    
    /* Step 4: Cleanup */
    printf("\n=== Cleaning up ===\n");
    unlink(TEMP_GCDA_FILE);
    unlink(INSTRUMENTED_BINARY);
    
    /* Clean up coverage files generated by instrumented binary */
    unlink("gcov-dump-instrumented.gcda");
    unlink("gcov-dump-instrumented.gcno");
    
    printf("\n=== Test Summary ===\n");
    if (all_tests_passed) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Some tests failed\n");
        return 1;
    }
}
