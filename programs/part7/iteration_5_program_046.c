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
    /* Version: 0x3430372a (gcov 4.7) */
    0x2a, 0x37, 0x30, 0x34,
    /* Stamp */
    0x00, 0x00, 0x00, 0x00,
    /* Zero records tag */
    0x00, 0x00, 0x00, 0x00,
    /* Length = 0 */
    0x00, 0x00, 0x00, 0x00
};

/* Create a minimal valid .gcda file */
static int create_minimal_gcda(const char *filename) {
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
static int execute_and_capture(const char *cmd, char *output, size_t output_size, 
                               int capture_stdout, int capture_stderr) {
    char full_cmd[1024];
    FILE *fp;
    int result;
    
    if (capture_stderr) {
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    } else {
        snprintf(full_cmd, sizeof(full_cmd), "%s", cmd);
    }
    
    fp = popen(full_cmd, "r");
    if (!fp) {
        perror("popen failed");
        return -1;
    }
    
    if (output && output_size > 0) {
        size_t bytes_read = fread(output, 1, output_size - 1, fp);
        output[bytes_read] = '\0';
    }
    
    result = pclose(fp);
    return WEXITSTATUS(result);
}

/* Build instrumented gcov-dump */
static int build_instrumented_gcov_dump(const char *source_path) {
    char cmd[2048];
    int result;
    
    printf("Building instrumented gcov-dump from: %s\n", source_path);
    
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    result = system(cmd);
    if (result != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 0;
    }
    
    /* Verify the binary exists and is executable */
    if (access(INSTRUMENTED_BINARY, X_OK) != 0) {
        fprintf(stderr, "Instrumented binary not created or not executable\n");
        return 0;
    }
    
    return 1;
}

/* Test -h flag (help) */
static void test_help_flag(void) {
    char cmd[256];
    int exit_code;
    
    printf("Testing -h flag...\n");
    
    snprintf(cmd, sizeof(cmd), "%s -h", INSTRUMENTED_BINARY);
    exit_code = execute_and_capture(cmd, NULL, 0, 0, 0);
    
    if (exit_code == 0) {
        printf("  ✓ -h flag test passed (exit code: %d)\n", exit_code);
    } else {
        printf("  ✗ -h flag test failed (exit code: %d)\n", exit_code);
    }
}

/* Test -v flag (version) */
static void test_version_flag(void) {
    char cmd[256];
    char output[1024];
    int exit_code;
    
    printf("Testing -v flag...\n");
    
    snprintf(cmd, sizeof(cmd), "%s -v", INSTRUMENTED_BINARY);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1, 0);
    
    if (exit_code == 0 && strlen(output) > 0) {
        printf("  ✓ -v flag test passed\n");
        printf("    Version output: %s", output);
    } else {
        printf("  ✗ -v flag test failed (exit code: %d)\n", exit_code);
    }
}

/* Test invalid flag */
static void test_invalid_flag(void) {
    char cmd[256];
    char output[1024];
    int exit_code;
    
    printf("Testing invalid flag -X...\n");
    
    snprintf(cmd, sizeof(cmd), "%s -X", INSTRUMENTED_BINARY);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0, 1);
    
    if (strstr(output, "unknown flag `X'") != NULL) {
        printf("  ✓ Invalid flag test passed\n");
        printf("    Error message: %s", output);
    } else {
        printf("  ✗ Invalid flag test failed\n");
        printf("    Output: %s", output);
    }
}

/* Test flag with coverage file */
static void test_flag_with_file(const char *flag, const char *flag_name) {
    char cmd[256];
    int exit_code;
    
    printf("Testing %s flag with coverage file...\n", flag_name);
    
    snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flag, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, NULL, 0, 0, 0);
    
    if (exit_code == 0) {
        printf("  ✓ %s flag test passed (exit code: %d)\n", flag_name, exit_code);
    } else {
        printf("  ✗ %s flag test failed (exit code: %d)\n", flag_name, exit_code);
    }
}

/* Test flag combinations */
static void test_flag_combinations(void) {
    char cmd[256];
    int exit_code;
    
    printf("Testing flag combinations...\n");
    
    /* Test -l -p combination */
    snprintf(cmd, sizeof(cmd), "%s -l -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, NULL, 0, 0, 0);
    printf("  -l -p combination: %s\n", exit_code == 0 ? "✓ passed" : "✗ failed");
    
    /* Test -p -l (reverse order) */
    snprintf(cmd, sizeof(cmd), "%s -p -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, NULL, 0, 0, 0);
    printf("  -p -l combination: %s\n", exit_code == 0 ? "✓ passed" : "✗ failed");
    
    /* Test -r -s combination */
    snprintf(cmd, sizeof(cmd), "%s -r -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, NULL, 0, 0, 0);
    printf("  -r -s combination: %s\n", exit_code == 0 ? "✓ passed" : "✗ failed");
}

int main(int argc, char *argv[]) {
    const char *gcov_dump_source = NULL;
    
    printf("=== GCOV-Dump Coverage Test Program ===\n\n");
    
    /* Try to locate gcov-dump.cc */
    if (argc > 1) {
        gcov_dump_source = argv[1];
    } else {
        /* Try common locations */
        const char *possible_paths[] = {
            "../gcc/gcov-dump.cc",
            "../../gcc/gcov-dump.cc",
            "/usr/src/gcc/gcc/gcov-dump.cc",
            NULL
        };
        
        for (int i = 0; possible_paths[i]; i++) {
            struct stat st;
            if (stat(possible_paths[i], &st) == 0 && S_ISREG(st.st_mode)) {
                gcov_dump_source = possible_paths[i];
                break;
            }
        }
    }
    
    if (!gcov_dump_source) {
        fprintf(stderr, "Error: gcov-dump.cc not found. Please specify path as argument.\n");
        fprintf(stderr, "Usage: %s <path-to-gcov-dump.cc>\n", argv[0]);
        return 1;
    }
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_instrumented_gcov_dump(gcov_dump_source)) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    printf("\nCreating minimal coverage file...\n");
    if (!create_minimal_gcda(TEMP_GCDA_FILE)) {
        fprintf(stderr, "Failed to create minimal coverage file\n");
        unlink(INSTRUMENTED_BINARY);
        return 1;
    }
    printf("  ✓ Created %s\n", TEMP_GCDA_FILE);
    
    /* Step 3: Execute test sequence */
    printf("\n=== Running Test Sequence ===\n\n");
    
    /* Test flags without file dependencies */
    test_help_flag();
    printf("\n");
    
    test_version_flag();
    printf("\n");
    
    test_invalid_flag();
    printf("\n");
    
    /* Test flags requiring coverage file */
    test_flag_with_file("-l", "-l (dump contents)");
    printf("\n");
    
    test_flag_with_file("-p", "-p (dump positions)");
    printf("\n");
    
    test_flag_with_file("-r", "-r (dump raw)");
    printf("\n");
    
    test_flag_with_file("-s", "-s (dump stable)");
    printf("\n");
    
    /* Test flag combinations */
    test_flag_combinations();
    
    /* Step 4: Cleanup */
    printf("\n=== Cleaning Up ===\n");
    unlink(TEMP_GCDA_FILE);
    unlink(INSTRUMENTED_BINARY);
    printf("  ✓ Removed temporary files\n");
    
    printf("\n=== Test Complete ===\n");
    
    return 0;
}
