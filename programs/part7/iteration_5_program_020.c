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
    /* Version: 0x3430372A (GCC 7*) */
    0x2A, 0x37, 0x30, 0x34,
    /* Stamp: 0 */
    0x00, 0x00, 0x00, 0x00,
    /* EOF marker */
    0x00, 0x00, 0x00, 0x00
};

/* Create a minimal valid .gcda file */
int create_minimal_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create minimal .gcda file");
        return 0;
    }
    
    size_t written = fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    return written == sizeof(minimal_gcda);
}

/* Execute command and capture output */
int execute_and_capture(const char *command, char *output, size_t output_size, 
                       int *exit_status, int capture_stderr) {
    char full_command[1024];
    if (capture_stderr) {
        snprintf(full_command, sizeof(full_command), "%s 2>&1", command);
    } else {
        snprintf(full_command, sizeof(full_command), "%s", command);
    }
    
    FILE *pipe = popen(full_command, "r");
    if (!pipe) {
        fprintf(stderr, "Failed to execute: %s\n", command);
        return 0;
    }
    
    if (output && output_size > 0) {
        output[0] = '\0';
        size_t total_read = 0;
        while (fgets(output + total_read, output_size - total_read, pipe)) {
            total_read = strlen(output);
            if (total_read >= output_size - 1) break;
        }
    }
    
    *exit_status = pclose(pipe);
    if (WIFEXITED(*exit_status)) {
        *exit_status = WEXITSTATUS(*exit_status);
    }
    
    return 1;
}

/* Check if string contains substring */
int contains_string(const char *haystack, const char *needle) {
    return strstr(haystack, needle) != NULL;
}

/* Build instrumented gcov-dump */
int build_instrumented_gcov_dump() {
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
    struct stat st;
    
    for (int i = 0; possible_paths[i]; i++) {
        if (stat(possible_paths[i], &st) == 0 && S_ISREG(st.st_mode)) {
            source_path = possible_paths[i];
            break;
        }
    }
    
    if (!source_path) {
        fprintf(stderr, "Could not find gcov-dump.cc source file\n");
        return 0;
    }
    
    printf("Found source at: %s\n", source_path);
    
    char compile_cmd[1024];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    printf("Compiling with: %s\n", compile_cmd);
    
    int status = system(compile_cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile gcov-dump\n");
        return 0;
    }
    
    if (access(INSTRUMENTED_BINARY, X_OK) != 0) {
        fprintf(stderr, "Instrumented binary not created\n");
        return 0;
    }
    
    printf("Instrumented binary created successfully\n");
    return 1;
}

int main() {
    char output[4096];
    int exit_status;
    
    printf("=== Starting gcov-dump coverage tests ===\n\n");
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_instrumented_gcov_dump()) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    printf("\nCreating minimal .gcda file...\n");
    if (!create_minimal_gcda(TEMP_GCDA_FILE)) {
        unlink(INSTRUMENTED_BINARY);
        return 1;
    }
    printf("Created: %s\n", TEMP_GCDA_FILE);
    
    /* Step 3: Execute test sequence */
    
    /* Test -h flag (help) */
    printf("\n=== Testing -h flag ===\n");
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "%s -h", INSTRUMENTED_BINARY);
    if (execute_and_capture(cmd, output, sizeof(output), &exit_status, 0)) {
        printf("Exit status: %d\n", exit_status);
        if (exit_status == 0) {
            printf("✓ -h flag test passed\n");
        } else {
            printf("✗ -h flag test failed\n");
        }
    }
    
    /* Test -v flag (version) */
    printf("\n=== Testing -v flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -v", INSTRUMENTED_BINARY);
    if (execute_and_capture(cmd, output, sizeof(output), &exit_status, 0)) {
        printf("Exit status: %d\n", exit_status);
        if (exit_status == 0 && contains_string(output, "gcov-dump")) {
            printf("Output contains version info: %.100s...\n", output);
            printf("✓ -v flag test passed\n");
        } else {
            printf("✗ -v flag test failed\n");
        }
    }
    
    /* Test -l flag (dump contents) */
    printf("\n=== Testing -l flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (execute_and_capture(cmd, output, sizeof(output), &exit_status, 0)) {
        printf("Exit status: %d\n", exit_status);
        printf("✓ -l flag executed (may produce warnings for minimal file)\n");
    }
    
    /* Test -p flag (dump positions) */
    printf("\n=== Testing -p flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (execute_and_capture(cmd, output, sizeof(output), &exit_status, 0)) {
        printf("Exit status: %d\n", exit_status);
        printf("✓ -p flag executed\n");
    }
    
    /* Test -r flag (dump raw) */
    printf("\n=== Testing -r flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -r %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (execute_and_capture(cmd, output, sizeof(output), &exit_status, 0)) {
        printf("Exit status: %d\n", exit_status);
        printf("✓ -r flag executed\n");
    }
    
    /* Test -s flag (dump stable) */
    printf("\n=== Testing -s flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (execute_and_capture(cmd, output, sizeof(output), &exit_status, 0)) {
        printf("Exit status: %d\n", exit_status);
        printf("✓ -s flag executed\n");
    }
    
    /* Test flag combinations */
    printf("\n=== Testing flag combinations ===\n");
    
    /* Test -l -p combination */
    printf("\nTesting -l -p combination...\n");
    snprintf(cmd, sizeof(cmd), "%s -l -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (execute_and_capture(cmd, output, sizeof(output), &exit_status, 0)) {
        printf("Exit status: %d\n", exit_status);
        printf("✓ -l -p combination executed\n");
    }
    
    /* Test -p -l combination (different order) */
    printf("\nTesting -p -l combination...\n");
    snprintf(cmd, sizeof(cmd), "%s -p -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (execute_and_capture(cmd, output, sizeof(output), &exit_status, 0)) {
        printf("Exit status: %d\n", exit_status);
        printf("✓ -p -l combination executed\n");
    }
    
    /* Test -r -s combination */
    printf("\nTesting -r -s combination...\n");
    snprintf(cmd, sizeof(cmd), "%s -r -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (execute_and_capture(cmd, output, sizeof(output), &exit_status, 0)) {
        printf("Exit status: %d\n", exit_status);
        printf("✓ -r -s combination executed\n");
    }
    
    /* Test invalid flag -X */
    printf("\n=== Testing invalid flag -X ===\n");
    snprintf(cmd, sizeof(cmd), "%s -X", INSTRUMENTED_BINARY);
    if (execute_and_capture(cmd, output, sizeof(output), &exit_status, 1)) {
        printf("Exit status: %d\n", exit_status);
        printf("Stderr output: %s", output);
        
        if (contains_string(output, "unknown flag `X'")) {
            printf("✓ Invalid flag test passed - correct error message found\n");
        } else {
            printf("✗ Invalid flag test failed - expected error message not found\n");
        }
    }
    
    /* Test invalid flag with file argument */
    printf("\n=== Testing invalid flag -Y with file ===\n");
    snprintf(cmd, sizeof(cmd), "%s -Y %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (execute_and_capture(cmd, output, sizeof(output), &exit_status, 1)) {
        printf("Exit status: %d\n", exit_status);
        printf("Stderr output: %s", output);
        
        if (contains_string(output, "unknown flag `Y'")) {
            printf("✓ Invalid flag with file test passed\n");
        } else {
            printf("✗ Invalid flag with file test failed\n");
        }
    }
    
    /* Step 4: Cleanup */
    printf("\n=== Cleaning up ===\n");
    if (unlink(TEMP_GCDA_FILE) == 0) {
        printf("Removed: %s\n", TEMP_GCDA_FILE);
    }
    
    if (unlink(INSTRUMENTED_BINARY) == 0) {
        printf("Removed: %s\n", INSTRUMENTED_BINARY);
    }
    
    /* Also remove coverage data files created by instrumented binary */
    char coverage_files[][64] = {
        "gcov-dump-instrumented.gcda",
        "gcov-dump-instrumented.gcno",
        INSTRUMENTED_BINARY ".gcda",
        INSTRUMENTED_BINARY ".gcno"
    };
    
    for (size_t i = 0; i < sizeof(coverage_files)/sizeof(coverage_files[0]); i++) {
        if (unlink(coverage_files[i]) == 0) {
            printf("Removed: %s\n", coverage_files[i]);
        }
    }
    
    printf("\n=== All tests completed ===\n");
    
    return 0;
}
