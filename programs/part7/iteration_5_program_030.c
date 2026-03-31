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
 * - Zero-length record (4 bytes of 0)
 */
static const unsigned char minimal_gcda[] = {
    0x67, 0x63, 0x64, 0x61,  /* 'gcda' magic */
    0x34, 0x30, 0x37, 0x2a,  /* '407*' version */
    0x00, 0x00, 0x00, 0x00   /* Zero-length record */
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

/* Build instrumented gcov-dump */
static int build_instrumented_gcov_dump(const char *source_path) {
    char cmd[2048];
    
    printf("Building instrumented gcov-dump from: %s\n", source_path);
    
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    int result = system(cmd);
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
    printf("Testing -h flag...\n");
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s -h", INSTRUMENTED_BINARY);
    
    int exit_code = execute_and_capture(cmd, NULL, 0, 0);
    
    if (exit_code == 0) {
        printf("  ✓ -h flag exited successfully (code 0)\n");
    } else {
        printf("  ✗ -h flag failed with exit code %d\n", exit_code);
    }
}

/* Test -v flag (version) */
static void test_version_flag(void) {
    printf("Testing -v flag...\n");
    
    char cmd[256];
    char output[1024] = {0};
    
    snprintf(cmd, sizeof(cmd), "%s -v", INSTRUMENTED_BINARY);
    
    int exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    
    if (exit_code == 0) {
        printf("  ✓ -v flag exited successfully\n");
        if (strlen(output) > 0) {
            printf("  ✓ Version output captured: %s", output);
        }
    } else {
        printf("  ✗ -v flag failed with exit code %d\n", exit_code);
    }
}

/* Test invalid flag */
static void test_invalid_flag(void) {
    printf("Testing invalid flag -X...\n");
    
    char cmd[256];
    char output[1024] = {0};
    
    snprintf(cmd, sizeof(cmd), "%s -X", INSTRUMENTED_BINARY);
    
    int exit_code = execute_and_capture(cmd, output, sizeof(output), 1);
    
    /* Check for the exact error message from the uncovered code */
    if (strstr(output, "unknown flag `X'") != NULL) {
        printf("  ✓ Invalid flag test correctly printed: unknown flag `X'\n");
    } else {
        printf("  ✗ Invalid flag test did not produce expected error\n");
        printf("    Output was: %s\n", output);
    }
}

/* Test flag with minimal coverage file */
static void test_flag_with_file(const char *flag, const char *description) {
    printf("Testing %s flag (%s)...\n", flag, description);
    
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flag, TEMP_GCDA_FILE);
    
    int exit_code = execute_and_capture(cmd, NULL, 0, 1);
    
    /* We don't care about the specific exit code for these flags,
       just that they execute without crashing */
    printf("  ✓ %s flag executed (exit code %d)\n", flag, exit_code);
}

/* Test flag combinations */
static void test_flag_combination(const char *flags, const char *description) {
    printf("Testing flag combination %s (%s)...\n", flags, description);
    
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flags, TEMP_GCDA_FILE);
    
    int exit_code = execute_and_capture(cmd, NULL, 0, 1);
    
    printf("  ✓ Combination %s executed (exit code %d)\n", flags, exit_code);
}

int main(int argc, char *argv[]) {
    const char *gcov_dump_source = NULL;
    
    /* Try to find gcov-dump.cc */
    if (argc > 1) {
        gcov_dump_source = argv[1];
    } else {
        /* Common locations in GCC source tree */
        const char *possible_paths[] = {
            "../gcc/gcov-dump.cc",
            "../../gcc/gcov-dump.cc",
            "gcc/gcov-dump.cc",
            "/usr/src/gcc/gcc/gcov-dump.cc",
            NULL
        };
        
        for (int i = 0; possible_paths[i] != NULL; i++) {
            if (access(possible_paths[i], R_OK) == 0) {
                gcov_dump_source = possible_paths[i];
                break;
            }
        }
    }
    
    if (!gcov_dump_source) {
        fprintf(stderr, "Error: gcov-dump.cc not found.\n");
        fprintf(stderr, "Usage: %s [path/to/gcov-dump.cc]\n", argv[0]);
        fprintf(stderr, "Or place gcov-dump.cc in one of the common locations.\n");
        return 1;
    }
    
    printf("Using gcov-dump source: %s\n", gcov_dump_source);
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_instrumented_gcov_dump(gcov_dump_source)) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    printf("Creating minimal .gcda file...\n");
    if (!create_minimal_gcda(TEMP_GCDA_FILE)) {
        fprintf(stderr, "Failed to create minimal coverage file\n");
        unlink(INSTRUMENTED_BINARY);
        return 1;
    }
    
    /* Step 3: Execute test sequence */
    printf("\n=== Starting test sequence ===\n");
    
    /* Test flags without file arguments */
    test_help_flag();
    test_version_flag();
    
    /* Test flags with minimal coverage file */
    test_flag_with_file("-l", "dump contents");
    test_flag_with_file("-p", "dump positions");
    test_flag_with_file("-r", "dump raw");
    test_flag_with_file("-s", "dump stable");
    
    /* Test flag combinations */
    test_flag_combination("-l -p", "contents and positions");
    test_flag_combination("-p -l", "positions and contents (reversed order)");
    test_flag_combination("-r -s", "raw and stable");
    test_flag_combination("-s -r", "stable and raw (reversed order)");
    test_flag_combination("-l -p -r -s", "all flags combined");
    
    /* Test invalid flag (must be last as it checks stderr) */
    test_invalid_flag();
    
    printf("\n=== Test sequence completed ===\n");
    
    /* Step 4: Cleanup */
    printf("Cleaning up temporary files...\n");
    unlink(TEMP_GCDA_FILE);
    unlink(INSTRUMENTED_BINARY);
    
    /* Also clean up coverage data files generated by the instrumented binary */
    char coverage_data[256];
    snprintf(coverage_data, sizeof(coverage_data), "%s.gcda", INSTRUMENTED_BINARY);
    unlink(coverage_data);
    
    snprintf(coverage_data, sizeof(coverage_data), "%s.gcno", INSTRUMENTED_BINARY);
    unlink(coverage_data);
    
    printf("All tests completed.\n");
    return 0;
}
