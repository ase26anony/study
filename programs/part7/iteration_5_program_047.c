#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define TEMP_FILE_TEMPLATE "/tmp/gcov_test_XXXXXX"

/* Minimal GCOV data file structure */
typedef struct {
    unsigned magic;      /* GCOV data magic */
    unsigned version;    /* GCOV version */
    unsigned stamp;      /* Uniquifying time stamp */
} gcov_header;

/* Function to create a minimal valid .gcda file */
int create_minimal_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create temporary .gcda file");
        return 0;
    }
    
    /* Write a minimal valid GCOV data file */
    gcov_header header;
    header.magic = 0x67636461;  /* 'gcda' in little-endian */
    header.version = 0x3430372a; /* GCOV version */
    header.stamp = 0x12345678;  /* Arbitrary stamp */
    
    /* Write header */
    fwrite(&header, sizeof(header), 1, fp);
    
    /* Write zero tag (indicates end of data) */
    unsigned zero_tag = 0;
    fwrite(&zero_tag, sizeof(zero_tag), 1, fp);
    
    fclose(fp);
    return 1;
}

/* Execute command and capture output */
char *execute_command(const char *cmd, int capture_stdout, int capture_stderr) {
    char *output = NULL;
    FILE *fp;
    char buffer[1024];
    size_t total_size = 0;
    
    /* Build command with redirection */
    char full_cmd[2048];
    if (capture_stderr) {
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    } else {
        strncpy(full_cmd, cmd, sizeof(full_cmd));
    }
    
    fp = popen(full_cmd, "r");
    if (!fp) {
        perror("popen failed");
        return NULL;
    }
    
    /* Read output */
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        size_t len = strlen(buffer);
        output = realloc(output, total_size + len + 1);
        if (!output) {
            pclose(fp);
            return NULL;
        }
        memcpy(output + total_size, buffer, len);
        total_size += len;
        output[total_size] = '\0';
    }
    
    pclose(fp);
    return output;
}

/* Check if string contains substring */
int contains_string(const char *str, const char *substr) {
    return str && substr && strstr(str, substr) != NULL;
}

/* Build instrumented gcov-dump */
char *build_instrumented_gcov_dump() {
    char *binary_path = strdup("/tmp/gcov-dump-instrumented");
    
    printf("Building instrumented gcov-dump...\n");
    
    /* Try to find gcov-dump.cc in common locations */
    const char *source_paths[] = {
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "../../../gcc/gcov-dump.cc",
        "gcov-dump.cc",
        NULL
    };
    
    const char *source_path = NULL;
    struct stat st;
    
    for (int i = 0; source_paths[i]; i++) {
        if (stat(source_paths[i], &st) == 0) {
            source_path = source_paths[i];
            break;
        }
    }
    
    if (!source_path) {
        fprintf(stderr, "Could not find gcov-dump.cc\n");
        free(binary_path);
        return NULL;
    }
    
    /* Build command */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             binary_path, source_path);
    
    printf("Compiling: %s\n", cmd);
    
    int result = system(cmd);
    if (result != 0) {
        fprintf(stderr, "Failed to compile gcov-dump\n");
        free(binary_path);
        return NULL;
    }
    
    /* Verify the binary exists */
    if (stat(binary_path, &st) != 0) {
        fprintf(stderr, "Binary not created: %s\n", binary_path);
        free(binary_path);
        return NULL;
    }
    
    printf("Instrumented gcov-dump built successfully: %s\n", binary_path);
    return binary_path;
}

int main() {
    char *gcov_dump_path = NULL;
    char gcda_file[256];
    int all_tests_passed = 1;
    
    /* Create temporary .gcda filename */
    strcpy(gcda_file, TEMP_FILE_TEMPLATE);
    int fd = mkstemp(gcda_file);
    if (fd < 0) {
        perror("Failed to create temporary file");
        return 1;
    }
    close(fd);
    
    /* Build instrumented gcov-dump */
    gcov_dump_path = build_instrumented_gcov_dump();
    if (!gcov_dump_path) {
        unlink(gcda_file);
        return 1;
    }
    
    /* Create minimal .gcda file */
    printf("\nCreating minimal .gcda file: %s\n", gcda_file);
    if (!create_minimal_gcda(gcda_file)) {
        free(gcov_dump_path);
        unlink(gcda_file);
        return 1;
    }
    
    /* Test 1: -h flag (help) */
    printf("\n=== Testing -h flag ===\n");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s -h", gcov_dump_path);
    int result = system(cmd);
    if (result == 0) {
        printf("✓ -h flag test passed (exit code: %d)\n", WEXITSTATUS(result));
    } else {
        printf("✗ -h flag test failed (exit code: %d)\n", WEXITSTATUS(result));
        all_tests_passed = 0;
    }
    
    /* Test 2: -v flag (version) */
    printf("\n=== Testing -v flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -v", gcov_dump_path);
    char *output = execute_command(cmd, 1, 0);
    if (output && contains_string(output, "gcov-dump") && contains_string(output, "version")) {
        printf("✓ -v flag test passed\n");
        printf("Output: %s\n", output);
    } else {
        printf("✗ -v flag test failed\n");
        if (output) printf("Output: %s\n", output);
        all_tests_passed = 0;
    }
    free(output);
    
    /* Test 3: -l flag (dump contents) */
    printf("\n=== Testing -l flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -l %s", gcov_dump_path, gcda_file);
    result = system(cmd);
    if (result == 0) {
        printf("✓ -l flag test passed (exit code: %d)\n", WEXITSTATUS(result));
    } else {
        printf("✗ -l flag test failed (exit code: %d)\n", WEXITSTATUS(result));
        all_tests_passed = 0;
    }
    
    /* Test 4: -p flag (dump positions) */
    printf("\n=== Testing -p flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -p %s", gcov_dump_path, gcda_file);
    result = system(cmd);
    if (result == 0) {
        printf("✓ -p flag test passed (exit code: %d)\n", WEXITSTATUS(result));
    } else {
        printf("✗ -p flag test failed (exit code: %d)\n", WEXITSTATUS(result));
        all_tests_passed = 0;
    }
    
    /* Test 5: -r flag (dump raw) */
    printf("\n=== Testing -r flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -r %s", gcov_dump_path, gcda_file);
    result = system(cmd);
    if (result == 0) {
        printf("✓ -r flag test passed (exit code: %d)\n", WEXITSTATUS(result));
    } else {
        printf("✗ -r flag test failed (exit code: %d)\n", WEXITSTATUS(result));
        all_tests_passed = 0;
    }
    
    /* Test 6: -s flag (dump stable) */
    printf("\n=== Testing -s flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -s %s", gcov_dump_path, gcda_file);
    result = system(cmd);
    if (result == 0) {
        printf("✓ -s flag test passed (exit code: %d)\n", WEXITSTATUS(result));
    } else {
        printf("✗ -s flag test failed (exit code: %d)\n", WEXITSTATUS(result));
        all_tests_passed = 0;
    }
    
    /* Test 7: Combined flags -l -p */
    printf("\n=== Testing combined flags -l -p ===\n");
    snprintf(cmd, sizeof(cmd), "%s -l -p %s", gcov_dump_path, gcda_file);
    result = system(cmd);
    if (result == 0) {
        printf("✓ -l -p flags test passed (exit code: %d)\n", WEXITSTATUS(result));
    } else {
        printf("✗ -l -p flags test failed (exit code: %d)\n", WEXITSTATUS(result));
        all_tests_passed = 0;
    }
    
    /* Test 8: Combined flags -r -s */
    printf("\n=== Testing combined flags -r -s ===\n");
    snprintf(cmd, sizeof(cmd), "%s -r -s %s", gcov_dump_path, gcda_file);
    result = system(cmd);
    if (result == 0) {
        printf("✓ -r -s flags test passed (exit code: %d)\n", WEXITSTATUS(result));
    } else {
        printf("✗ -r -s flags test failed (exit code: %d)\n", WEXITSTATUS(result));
        all_tests_passed = 0;
    }
    
    /* Test 9: Different flag ordering -p -l */
    printf("\n=== Testing flag ordering -p -l ===\n");
    snprintf(cmd, sizeof(cmd), "%s -p -l %s", gcov_dump_path, gcda_file);
    result = system(cmd);
    if (result == 0) {
        printf("✓ -p -l flags test passed (exit code: %d)\n", WEXITSTATUS(result));
    } else {
        printf("✗ -p -l flags test failed (exit code: %d)\n", WEXITSTATUS(result));
        all_tests_passed = 0;
    }
    
    /* Test 10: Invalid flag -X */
    printf("\n=== Testing invalid flag -X ===\n");
    snprintf(cmd, sizeof(cmd), "%s -X %s", gcov_dump_path, gcda_file);
    output = execute_command(cmd, 0, 1);  // Capture stderr
    if (output && contains_string(output, "unknown flag")) {
        printf("✓ Invalid flag test passed\n");
        printf("Error message: %s\n", output);
    } else {
        printf("✗ Invalid flag test failed\n");
        if (output) printf("Output: %s\n", output);
        all_tests_passed = 0;
    }
    free(output);
    
    /* Test 11: Invalid flag without file argument */
    printf("\n=== Testing invalid flag -Y (no file) ===\n");
    snprintf(cmd, sizeof(cmd), "%s -Y", gcov_dump_path);
    output = execute_command(cmd, 0, 1);  // Capture stderr
    if (output && contains_string(output, "unknown flag")) {
        printf("✓ Invalid flag (no file) test passed\n");
        printf("Error message: %s\n", output);
    } else {
        printf("✗ Invalid flag (no file) test failed\n");
        if (output) printf("Output: %s\n", output);
        all_tests_passed = 0;
    }
    free(output);
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    unlink(gcda_file);
    printf("Removed temporary file: %s\n", gcda_file);
    
    unlink(gcov_dump_path);
    printf("Removed instrumented binary: %s\n", gcov_dump_path);
    
    /* Remove coverage data files from instrumented binary */
    char coverage_file[256];
    snprintf(coverage_file, sizeof(coverage_file), "%s.gcda", gcov_dump_path);
    unlink(coverage_file);
    
    snprintf(coverage_file, sizeof(coverage_file), "%s.gcno", gcov_dump_path);
    unlink(coverage_file);
    
    free(gcov_dump_path);
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    if (all_tests_passed) {
        printf("✓ All tests passed!\n");
        return 0;
    } else {
        printf("✗ Some tests failed\n");
        return 1;
    }
}
