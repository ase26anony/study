#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_GCDA_FILE "/tmp/minimal.gcda"
#define INSTRUMENTED_BINARY "/tmp/gcov-dump-instrumented"

/* Minimal GCOV data file format:
 * - Magic number: 0x67636461 ('gcda')
 * - Version: 0x3430372a ('407*' for GCC 4.7+)
 * - Zero-length record (tag 0, length 0)
 */
static const unsigned char minimal_gcda[] = {
    0x67, 0x63, 0x64, 0x61,  /* magic: 'gcda' */
    0x34, 0x30, 0x37, 0x2a,  /* version: '407*' */
    0x00, 0x00, 0x00, 0x00,  /* tag: 0 (end of file) */
    0x00, 0x00, 0x00, 0x00   /* length: 0 */
};

/* Create a minimal valid .gcda file */
static int create_minimal_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("fopen");
        return -1;
    }
    
    size_t written = fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    return (written == sizeof(minimal_gcda)) ? 0 : -1;
}

/* Execute command and capture output */
static int execute_and_capture(const char *cmd, char *output, size_t output_size, 
                               int capture_stderr) {
    char full_cmd[1024];
    if (capture_stderr) {
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    } else {
        snprintf(full_cmd, sizeof(full_cmd), "%s", cmd);
    }
    
    FILE *fp = popen(full_cmd, "r");
    if (!fp) {
        perror("popen");
        return -1;
    }
    
    if (output && output_size > 0) {
        output[0] = '\0';
        size_t total_read = 0;
        while (fgets(output + total_read, output_size - total_read, fp)) {
            total_read = strlen(output);
            if (total_read >= output_size - 1) break;
        }
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
}

/* Check if string contains substring */
static int contains_string(const char *str, const char *substr) {
    return strstr(str, substr) != NULL;
}

/* Build instrumented gcov-dump */
static int build_instrumented_gcov_dump(const char *source_path) {
    char cmd[2048];
    
    printf("Building instrumented gcov-dump from: %s\n", source_path);
    
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Failed to build gcov-dump. Command: %s\n", cmd);
        return -1;
    }
    
    /* Verify the binary exists and is executable */
    if (access(INSTRUMENTED_BINARY, X_OK) != 0) {
        fprintf(stderr, "Instrumented binary not created or not executable\n");
        return -1;
    }
    
    printf("Successfully built instrumented gcov-dump\n");
    return 0;
}

int main(int argc, char *argv[]) {
    char output[4096];
    int ret;
    
    /* Determine gcov-dump source path */
    const char *gcov_dump_source = NULL;
    
    /* Try to find gcov-dump.cc in common locations */
    const char *possible_paths[] = {
        "../../gcc/gcov-dump.cc",          /* Relative from build dir */
        "../gcc/gcov-dump.cc",             /* Another relative path */
        "/usr/src/gcc/gcc/gcov-dump.cc",   /* System source location */
        NULL
    };
    
    for (int i = 0; possible_paths[i]; i++) {
        if (access(possible_paths[i], R_OK) == 0) {
            gcov_dump_source = possible_paths[i];
            break;
        }
    }
    
    if (!gcov_dump_source) {
        /* If not found, try to use first argument or default */
        if (argc > 1) {
            gcov_dump_source = argv[1];
        } else {
            fprintf(stderr, "Could not find gcov-dump.cc. Please specify path as argument.\n");
            fprintf(stderr, "Usage: %s [path/to/gcov-dump.cc]\n", argv[0]);
            return 1;
        }
    }
    
    /* Step 1: Build instrumented gcov-dump */
    if (build_instrumented_gcov_dump(gcov_dump_source) != 0) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    printf("Creating minimal .gcda file...\n");
    if (create_minimal_gcda(TEMP_GCDA_FILE) != 0) {
        fprintf(stderr, "Failed to create minimal .gcda file\n");
        unlink(INSTRUMENTED_BINARY);
        return 1;
    }
    
    /* Step 3: Execute test sequence */
    
    /* Test -h flag (help) */
    printf("\n=== Testing -h flag (help) ===\n");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s -h", INSTRUMENTED_BINARY);
    ret = execute_and_capture(cmd, output, sizeof(output), 0);
    if (ret == 0) {
        printf("✓ -h flag test passed (exit code: %d)\n", ret);
    } else {
        printf("✗ -h flag test failed (exit code: %d)\n", ret);
    }
    
    /* Test -v flag (version) */
    printf("\n=== Testing -v flag (version) ===\n");
    snprintf(cmd, sizeof(cmd), "%s -v", INSTRUMENTED_BINARY);
    ret = execute_and_capture(cmd, output, sizeof(output), 0);
    if (ret == 0 && contains_string(output, "gcov-dump") && 
        (contains_string(output, "version") || contains_string(output, "GCC"))) {
        printf("✓ -v flag test passed. Output contains version info.\n");
    } else {
        printf("✗ -v flag test failed. Exit code: %d, Output: %s\n", ret, output);
    }
    
    /* Test -l flag (dump contents) */
    printf("\n=== Testing -l flag (dump contents) ===\n");
    snprintf(cmd, sizeof(cmd), "%s -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    ret = execute_and_capture(cmd, output, sizeof(output), 0);
    printf("-l flag exit code: %d\n", ret);
    
    /* Test -p flag (dump positions) */
    printf("\n=== Testing -p flag (dump positions) ===\n");
    snprintf(cmd, sizeof(cmd), "%s -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    ret = execute_and_capture(cmd, output, sizeof(output), 0);
    printf("-p flag exit code: %d\n", ret);
    
    /* Test -r flag (dump raw) */
    printf("\n=== Testing -r flag (dump raw) ===\n");
    snprintf(cmd, sizeof(cmd), "%s -r %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    ret = execute_and_capture(cmd, output, sizeof(output), 0);
    printf("-r flag exit code: %d\n", ret);
    
    /* Test -s flag (dump stable) */
    printf("\n=== Testing -s flag (dump stable) ===\n");
    snprintf(cmd, sizeof(cmd), "%s -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    ret = execute_and_capture(cmd, output, sizeof(output), 0);
    printf("-s flag exit code: %d\n", ret);
    
    /* Test flag combinations */
    printf("\n=== Testing flag combinations ===\n");
    
    /* -l -p combination */
    printf("Testing -l -p combination...\n");
    snprintf(cmd, sizeof(cmd), "%s -l -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    ret = execute_and_capture(cmd, output, sizeof(output), 0);
    printf("-l -p exit code: %d\n", ret);
    
    /* -p -l combination (different order) */
    printf("Testing -p -l combination (different order)...\n");
    snprintf(cmd, sizeof(cmd), "%s -p -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    ret = execute_and_capture(cmd, output, sizeof(output), 0);
    printf("-p -l exit code: %d\n", ret);
    
    /* -r -s combination */
    printf("Testing -r -s combination...\n");
    snprintf(cmd, sizeof(cmd), "%s -r -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    ret = execute_and_capture(cmd, output, sizeof(output), 0);
    printf("-r -s exit code: %d\n", ret);
    
    /* Test invalid flag (-X) */
    printf("\n=== Testing invalid flag (-X) ===\n");
    snprintf(cmd, sizeof(cmd), "%s -X %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    ret = execute_and_capture(cmd, output, sizeof(output), 1);  /* Capture stderr */
    
    if (contains_string(output, "unknown flag") && contains_string(output, "X")) {
        printf("✓ Invalid flag test passed. Got expected error message.\n");
        printf("  Error output: %s\n", output);
    } else {
        printf("✗ Invalid flag test failed. Expected 'unknown flag' message.\n");
        printf("  Exit code: %d, Output: %s\n", ret, output);
    }
    
    /* Test invalid flag without file argument */
    printf("\n=== Testing invalid flag without file argument ===\n");
    snprintf(cmd, sizeof(cmd), "%s -X", INSTRUMENTED_BINARY);
    ret = execute_and_capture(cmd, output, sizeof(output), 1);
    
    if (contains_string(output, "unknown flag") && contains_string(output, "X")) {
        printf("✓ Invalid flag (no file) test passed.\n");
    } else {
        printf("✗ Invalid flag (no file) test failed. Output: %s\n", output);
    }
    
    /* Step 4: Cleanup */
    printf("\n=== Cleaning up ===\n");
    unlink(TEMP_GCDA_FILE);
    unlink(INSTRUMENTED_BINARY);
    
    /* Also clean up coverage data files created by instrumented binary */
    char coverage_files[][64] = {
        "gcov-dump-instrumented.gcda",
        "gcov-dump-instrumented.gcno",
        INSTRUMENTED_BINARY ".gcda",
        INSTRUMENTED_BINARY ".gcno"
    };
    
    for (size_t i = 0; i < sizeof(coverage_files)/sizeof(coverage_files[0]); i++) {
        if (access(coverage_files[i], F_OK) == 0) {
            unlink(coverage_files[i]);
        }
    }
    
    printf("Test completed. All temporary files removed.\n");
    
    return 0;
}
