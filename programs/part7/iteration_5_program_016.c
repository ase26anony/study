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
    /* Length: 0 (no records) */
    0x00, 0x00, 0x00, 0x00
};

/* Function to compile gcov-dump with coverage instrumentation */
int compile_gcov_dump(const char *source_path) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    printf("Compiling gcov-dump: %s\n", cmd);
    int status = system(cmd);
    
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("Successfully compiled instrumented gcov-dump\n");
        return 1;
    } else {
        fprintf(stderr, "Failed to compile gcov-dump\n");
        return 0;
    }
}

/* Function to create minimal coverage data file */
int create_minimal_gcda() {
    FILE *fp = fopen(TEMP_GCDA_FILE, "wb");
    if (!fp) {
        perror("Failed to create temporary gcda file");
        return 0;
    }
    
    fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    printf("Created minimal coverage file: %s\n", TEMP_GCDA_FILE);
    return 1;
}

/* Function to execute gcov-dump and capture output */
int execute_gcov_dump(const char *args, char *output, size_t output_size, 
                      int capture_stderr) {
    char cmd[1024];
    if (capture_stderr) {
        snprintf(cmd, sizeof(cmd), "%s %s 2>&1", INSTRUMENTED_BINARY, args);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s", INSTRUMENTED_BINARY, args);
    }
    
    printf("Executing: %s\n", cmd);
    
    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        perror("popen failed");
        return -1;
    }
    
    if (output && output_size > 0) {
        output[0] = '\0';
        size_t total_read = 0;
        while (fgets(output + total_read, output_size - total_read, pipe)) {
            total_read = strlen(output);
            if (total_read >= output_size - 1) break;
        }
    }
    
    int status = pclose(pipe);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Test specific flag parsing */
void test_flag_h() {
    printf("\n=== Testing -h flag ===\n");
    int exit_code = execute_gcov_dump("-h", NULL, 0, 0);
    if (exit_code == 0) {
        printf("✓ -h flag test passed (exit code: %d)\n", exit_code);
    } else {
        printf("✗ -h flag test failed (exit code: %d)\n", exit_code);
    }
}

void test_flag_v() {
    printf("\n=== Testing -v flag ===\n");
    char output[4096] = {0};
    int exit_code = execute_gcov_dump("-v", output, sizeof(output), 0);
    
    if (exit_code == 0 && strstr(output, "gcov-dump") != NULL) {
        printf("✓ -v flag test passed\n");
        printf("Version output: %s", output);
    } else {
        printf("✗ -v flag test failed\n");
        printf("Output: %s\n", output);
    }
}

void test_flag_l() {
    printf("\n=== Testing -l flag ===\n");
    char args[256];
    snprintf(args, sizeof(args), "-l %s", TEMP_GCDA_FILE);
    int exit_code = execute_gcov_dump(args, NULL, 0, 0);
    
    if (exit_code == 0) {
        printf("✓ -l flag test passed\n");
    } else {
        printf("✗ -l flag test failed (exit code: %d)\n", exit_code);
    }
}

void test_flag_p() {
    printf("\n=== Testing -p flag ===\n");
    char args[256];
    snprintf(args, sizeof(args), "-p %s", TEMP_GCDA_FILE);
    int exit_code = execute_gcov_dump(args, NULL, 0, 0);
    
    if (exit_code == 0) {
        printf("✓ -p flag test passed\n");
    } else {
        printf("✗ -p flag test failed (exit code: %d)\n", exit_code);
    }
}

void test_flag_r() {
    printf("\n=== Testing -r flag ===\n");
    char args[256];
    snprintf(args, sizeof(args), "-r %s", TEMP_GCDA_FILE);
    int exit_code = execute_gcov_dump(args, NULL, 0, 0);
    
    if (exit_code == 0) {
        printf("✓ -r flag test passed\n");
    } else {
        printf("✗ -r flag test failed (exit code: %d)\n", exit_code);
    }
}

void test_flag_s() {
    printf("\n=== Testing -s flag ===\n");
    char args[256];
    snprintf(args, sizeof(args), "-s %s", TEMP_GCDA_FILE);
    int exit_code = execute_gcov_dump(args, NULL, 0, 0);
    
    if (exit_code == 0) {
        printf("✓ -s flag test passed\n");
    } else {
        printf("✗ -s flag test failed (exit code: %d)\n", exit_code);
    }
}

void test_flag_combinations() {
    printf("\n=== Testing flag combinations ===\n");
    
    char args[256];
    int passed = 0;
    int total = 0;
    
    /* Test -l -p */
    snprintf(args, sizeof(args), "-l -p %s", TEMP_GCDA_FILE);
    if (execute_gcov_dump(args, NULL, 0, 0) == 0) {
        printf("✓ -l -p combination passed\n");
        passed++;
    } else {
        printf("✗ -l -p combination failed\n");
    }
    total++;
    
    /* Test -p -l (different order) */
    snprintf(args, sizeof(args), "-p -l %s", TEMP_GCDA_FILE);
    if (execute_gcov_dump(args, NULL, 0, 0) == 0) {
        printf("✓ -p -l combination passed\n");
        passed++;
    } else {
        printf("✗ -p -l combination failed\n");
    }
    total++;
    
    /* Test -r -s */
    snprintf(args, sizeof(args), "-r -s %s", TEMP_GCDA_FILE);
    if (execute_gcov_dump(args, NULL, 0, 0) == 0) {
        printf("✓ -r -s combination passed\n");
        passed++;
    } else {
        printf("✗ -r -s combination failed\n");
    }
    total++;
    
    printf("Flag combinations: %d/%d passed\n", passed, total);
}

void test_invalid_flag() {
    printf("\n=== Testing invalid flag ===\n");
    char output[4096] = {0};
    char args[256];
    snprintf(args, sizeof(args), "-X %s", TEMP_GCDA_FILE);
    
    int exit_code = execute_gcov_dump(args, output, sizeof(output), 1);
    
    if (strstr(output, "unknown flag `X'") != NULL) {
        printf("✓ Invalid flag test passed - found expected error message\n");
        printf("Stderr output: %s", output);
    } else {
        printf("✗ Invalid flag test failed\n");
        printf("Expected 'unknown flag `X'' in output\n");
        printf("Actual output: %s\n", output);
    }
}

/* Cleanup temporary files */
void cleanup() {
    printf("\n=== Cleaning up ===\n");
    
    if (remove(TEMP_GCDA_FILE) == 0) {
        printf("Removed %s\n", TEMP_GCDA_FILE);
    }
    
    if (remove(INSTRUMENTED_BINARY) == 0) {
        printf("Removed %s\n", INSTRUMENTED_BINARY);
    }
}

int main(int argc, char *argv[]) {
    printf("=== GCOV-Dump Coverage Test Driver ===\n");
    
    /* Determine gcov-dump source path */
    const char *gcov_dump_source = NULL;
    
    /* Try to find gcov-dump.cc in common locations */
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
    
    if (!gcov_dump_source) {
        /* Try using command line argument */
        if (argc > 1) {
            gcov_dump_source = argv[1];
        } else {
            fprintf(stderr, "Error: Could not find gcov-dump.cc\n");
            fprintf(stderr, "Usage: %s [path/to/gcov-dump.cc]\n", argv[0]);
            return 1;
        }
    }
    
    printf("Using source file: %s\n", gcov_dump_source);
    
    /* Step 1: Build instrumented gcov-dump */
    if (!compile_gcov_dump(gcov_dump_source)) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    if (!create_minimal_gcda()) {
        return 1;
    }
    
    /* Step 3: Execute test sequence */
    test_flag_h();
    test_flag_v();
    test_flag_l();
    test_flag_p();
    test_flag_r();
    test_flag_s();
    test_flag_combinations();
    test_invalid_flag();
    
    /* Step 4: Cleanup */
    cleanup();
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
