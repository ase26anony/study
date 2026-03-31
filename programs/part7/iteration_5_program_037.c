#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_GCDA_FILE "/tmp/minimal.gcda"
#define TEMP_GCOV_DUMP "/tmp/gcov-dump-instrumented"

/* Minimal GCOV data file format */
typedef struct {
    unsigned magic;     /* Magic number */
    unsigned version;   /* Version */
    unsigned stamp;     /* Time stamp */
} gcov_header;

/* Function to create a minimal valid .gcda file */
int create_minimal_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create temporary .gcda file");
        return 0;
    }
    
    /* Write GCOV header */
    gcov_header header;
    header.magic = 0x67636461;  /* 'gcda' in little-endian */
    header.version = 0x3430392a; /* '409*' in little-endian */
    header.stamp = 0x12345678;  /* Arbitrary timestamp */
    
    fwrite(&header, sizeof(header), 1, fp);
    
    /* Write a zero-length record to terminate the file */
    unsigned zero = 0;
    fwrite(&zero, sizeof(unsigned), 1, fp);
    
    fclose(fp);
    return 1;
}

/* Execute command and capture output */
char *execute_command(const char *cmd, int capture_stderr) {
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
        perror("popen failed");
        return NULL;
    }
    
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        size_t new_len = output_size + strlen(buffer) + 1;
        char *new_output = realloc(output, new_len);
        if (!new_output) {
            free(output);
            pclose(fp);
            return NULL;
        }
        output = new_output;
        if (output_size == 0) {
            output[0] = '\0';
        }
        strcat(output, buffer);
        output_size = new_len - 1;
    }
    
    pclose(fp);
    return output;
}

/* Check if string contains substring */
int contains_string(const char *str, const char *substr) {
    return str && substr && strstr(str, substr) != NULL;
}

/* Build gcov-dump with coverage instrumentation */
int build_gcov_dump(const char *source_path) {
    char cmd[1024];
    
    printf("Building instrumented gcov-dump...\n");
    
    /* Try to find gcov-dump.cc in common locations */
    const char *possible_paths[] = {
        source_path,
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "../../../gcc/gcov-dump.cc",
        "/usr/src/gcc/gcc/gcov-dump.cc",
        NULL
    };
    
    const char *actual_path = NULL;
    struct stat st;
    
    for (int i = 0; possible_paths[i]; i++) {
        if (stat(possible_paths[i], &st) == 0 && S_ISREG(st.st_mode)) {
            actual_path = possible_paths[i];
            printf("Found gcov-dump.cc at: %s\n", actual_path);
            break;
        }
    }
    
    if (!actual_path) {
        fprintf(stderr, "Could not find gcov-dump.cc\n");
        return 0;
    }
    
    /* Compile with coverage flags */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             TEMP_GCOV_DUMP, actual_path);
    
    printf("Compiling: %s\n", cmd);
    
    int result = system(cmd);
    if (result != 0) {
        fprintf(stderr, "Failed to compile gcov-dump\n");
        return 0;
    }
    
    /* Verify the binary was created */
    if (stat(TEMP_GCOV_DUMP, &st) != 0 || !S_ISREG(st.st_mode)) {
        fprintf(stderr, "Compiled binary not found\n");
        return 0;
    }
    
    printf("Successfully built instrumented gcov-dump\n");
    return 1;
}

int main(int argc, char *argv[]) {
    printf("=== Starting gcov-dump coverage test ===\n");
    
    /* Build the instrumented gcov-dump */
    const char *source_path = (argc > 1) ? argv[1] : "gcov-dump.cc";
    if (!build_gcov_dump(source_path)) {
        return 1;
    }
    
    /* Create minimal .gcda file */
    printf("\nCreating minimal .gcda file...\n");
    if (!create_minimal_gcda(TEMP_GCDA_FILE)) {
        unlink(TEMP_GCOV_DUMP);
        return 1;
    }
    printf("Created: %s\n", TEMP_GCDA_FILE);
    
    int all_tests_passed = 1;
    
    /* Test 1: -h flag (help) */
    printf("\n=== Test 1: Testing -h flag ===\n");
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "%s -h", TEMP_GCOV_DUMP);
    int result = system(cmd);
    if (result == 0) {
        printf("✓ -h flag test passed (exit code 0)\n");
    } else {
        printf("✗ -h flag test failed (exit code %d)\n", WEXITSTATUS(result));
        all_tests_passed = 0;
    }
    
    /* Test 2: -v flag (version) */
    printf("\n=== Test 2: Testing -v flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -v", TEMP_GCOV_DUMP);
    char *output = execute_command(cmd, 0);
    if (output && (contains_string(output, "gcov-dump") || 
                   contains_string(output, "GCC") ||
                   contains_string(output, "version"))) {
        printf("✓ -v flag test passed (version info found)\n");
        printf("Output: %s\n", output);
    } else {
        printf("✗ -v flag test failed\n");
        if (output) printf("Output: %s\n", output);
        all_tests_passed = 0;
    }
    free(output);
    
    /* Test 3: -l flag (dump contents) */
    printf("\n=== Test 3: Testing -l flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -l %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    result = system(cmd);
    if (result == 0) {
        printf("✓ -l flag test passed\n");
    } else {
        printf("✗ -l flag test failed (exit code %d)\n", WEXITSTATUS(result));
        all_tests_passed = 0;
    }
    
    /* Test 4: -p flag (dump positions) */
    printf("\n=== Test 4: Testing -p flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -p %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    result = system(cmd);
    if (result == 0) {
        printf("✓ -p flag test passed\n");
    } else {
        printf("✗ -p flag test failed (exit code %d)\n", WEXITSTATUS(result));
        all_tests_passed = 0;
    }
    
    /* Test 5: -r flag (dump raw) */
    printf("\n=== Test 5: Testing -r flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -r %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    result = system(cmd);
    if (result == 0) {
        printf("✓ -r flag test passed\n");
    } else {
        printf("✗ -r flag test failed (exit code %d)\n", WEXITSTATUS(result));
        all_tests_passed = 0;
    }
    
    /* Test 6: -s flag (dump stable) */
    printf("\n=== Test 6: Testing -s flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -s %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    result = system(cmd);
    if (result == 0) {
        printf("✓ -s flag test passed\n");
    } else {
        printf("✗ -s flag test failed (exit code %d)\n", WEXITSTATUS(result));
        all_tests_passed = 0;
    }
    
    /* Test 7: Combined flags -l -p */
    printf("\n=== Test 7: Testing combined flags -l -p ===\n");
    snprintf(cmd, sizeof(cmd), "%s -l -p %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    result = system(cmd);
    if (result == 0) {
        printf("✓ Combined -l -p test passed\n");
    } else {
        printf("✗ Combined -l -p test failed (exit code %d)\n", WEXITSTATUS(result));
        all_tests_passed = 0;
    }
    
    /* Test 8: Combined flags -r -s (different order) */
    printf("\n=== Test 8: Testing combined flags -r -s ===\n");
    snprintf(cmd, sizeof(cmd), "%s -r -s %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    result = system(cmd);
    if (result == 0) {
        printf("✓ Combined -r -s test passed\n");
    } else {
        printf("✗ Combined -r -s test failed (exit code %d)\n", WEXITSTATUS(result));
        all_tests_passed = 0;
    }
    
    /* Test 9: Combined flags -p -l (reverse order) */
    printf("\n=== Test 9: Testing combined flags -p -l (reverse order) ===\n");
    snprintf(cmd, sizeof(cmd), "%s -p -l %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    result = system(cmd);
    if (result == 0) {
        printf("✓ Combined -p -l test passed\n");
    } else {
        printf("✗ Combined -p -l test failed (exit code %d)\n", WEXITSTATUS(result));
        all_tests_passed = 0;
    }
    
    /* Test 10: Invalid flag -X */
    printf("\n=== Test 10: Testing invalid flag -X ===\n");
    snprintf(cmd, sizeof(cmd), "%s -X %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    output = execute_command(cmd, 1);  // Capture stderr
    if (output && contains_string(output, "unknown flag `X'")) {
        printf("✓ Invalid flag test passed (correct error message)\n");
        printf("Error output: %s\n", output);
    } else {
        printf("✗ Invalid flag test failed\n");
        if (output) printf("Output: %s\n", output);
        all_tests_passed = 0;
    }
    free(output);
    
    /* Test 11: Invalid flag without file argument */
    printf("\n=== Test 11: Testing invalid flag -Y (no file) ===\n");
    snprintf(cmd, sizeof(cmd), "%s -Y", TEMP_GCOV_DUMP);
    output = execute_command(cmd, 1);  // Capture stderr
    if (output && contains_string(output, "unknown flag `Y'")) {
        printf("✓ Invalid flag (no file) test passed\n");
        printf("Error output: %s\n", output);
    } else {
        printf("✗ Invalid flag (no file) test failed\n");
        if (output) printf("Output: %s\n", output);
        all_tests_passed = 0;
    }
    free(output);
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    unlink(TEMP_GCDA_FILE);
    unlink(TEMP_GCOV_DUMP);
    
    /* Clean up coverage files created by instrumented binary */
    char coverage_files[][64] = {
        TEMP_GCOV_DUMP ".gcda",
        TEMP_GCOV_DUMP ".gcno",
        "gcov-dump.gcda",
        "gcov-dump.gcno"
    };
    
    for (size_t i = 0; i < sizeof(coverage_files)/sizeof(coverage_files[0]); i++) {
        if (unlink(coverage_files[i]) == 0) {
            printf("Removed: %s\n", coverage_files[i]);
        }
    }
    
    printf("\n=== Test Summary ===\n");
    if (all_tests_passed) {
        printf("✓ All tests passed!\n");
        printf("\nThe following gcov-dump.cc lines should now be covered:\n");
        printf("  Lines 111-130: switch cases for -h, -v, -l, -p, -r, -s, and default case\n");
        printf("  Specifically:\n");
        printf("    - Line 111-113: case 'h': print_usage()\n");
        printf("    - Line 114-116: case 'v': print_version()\n");
        printf("    - Line 117-119: case 'l': flag_dump_contents = 1\n");
        printf("    - Line 120-122: case 'p': flag_dump_positions = 1\n");
        printf("    - Line 123-125: case 'r': flag_dump_raw = 1\n");
        printf("    - Line 126-128: case 's': flag_dump_stable = 1\n");
        printf("    - Line 129-130: default: fprintf(stderr, \"unknown flag\")\n");
    } else {
        printf("✗ Some tests failed\n");
    }
    
    return all_tests_passed ? 0 : 1;
}
