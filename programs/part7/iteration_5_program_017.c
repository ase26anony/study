#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define TEMP_GCDA "/tmp/minimal.gcda"
#define TEMP_GCOV_DUMP "/tmp/gcov-dump-instrumented"

/* Minimal GCOV data file structure */
static const unsigned char minimal_gcda[] = {
    /* Magic: 'gcda' */
    0x67, 0x63, 0x64, 0x61,
    /* Version: 0xB1C2D3E4 (example version, adjust if needed) */
    0xB1, 0xC2, 0xD3, 0xE4,
    /* Zero-length record (tag 0, length 0) */
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

/* Execute command and capture output */
int execute_and_capture(const char *cmd, char *output, size_t output_size, 
                        int capture_stderr) {
    char full_cmd[1024];
    FILE *fp;
    
    if (capture_stderr) {
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    } else {
        snprintf(full_cmd, sizeof(full_cmd), "%s", cmd);
    }
    
    fp = popen(full_cmd, "r");
    if (!fp) {
        return -1;
    }
    
    if (output && output_size > 0) {
        size_t bytes_read = fread(output, 1, output_size - 1, fp);
        output[bytes_read] = '\0';
    }
    
    return pclose(fp);
}

/* Build instrumented gcov-dump */
int build_gcov_dump() {
    char cmd[2048];
    int ret;
    
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
        if (stat(possible_paths[i], &st) == 0) {
            source_path = possible_paths[i];
            break;
        }
    }
    
    if (!source_path) {
        /* Try current directory */
        if (stat("gcov-dump.cc", &st) == 0) {
            source_path = "gcov-dump.cc";
        } else {
            fprintf(stderr, "Error: Could not find gcov-dump.cc\n");
            return 0;
        }
    }
    
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             TEMP_GCOV_DUMP, source_path);
    
    ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Error: Failed to compile gcov-dump: %s\n", cmd);
        return 0;
    }
    
    if (stat(TEMP_GCOV_DUMP, &st) != 0) {
        fprintf(stderr, "Error: Compiled binary not found\n");
        return 0;
    }
    
    printf("Successfully built %s\n", TEMP_GCOV_DUMP);
    return 1;
}

/* Create minimal coverage file */
int create_minimal_gcda() {
    FILE *fp = fopen(TEMP_GCDA, "wb");
    if (!fp) {
        perror("Failed to create temporary gcda file");
        return 0;
    }
    
    fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    printf("Created minimal coverage file: %s\n", TEMP_GCDA);
    return 1;
}

/* Test -h flag (help) */
void test_help_flag() {
    char cmd[256];
    int ret;
    
    printf("\n=== Testing -h flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -h", TEMP_GCOV_DUMP);
    
    ret = system(cmd);
    if (WIFEXITED(ret) && WEXITSTATUS(ret) == 0) {
        printf("✓ -h flag test passed (exit code 0)\n");
    } else {
        printf("✗ -h flag test failed (exit code %d)\n", WEXITSTATUS(ret));
    }
}

/* Test -v flag (version) */
void test_version_flag() {
    char output[1024];
    char cmd[256];
    int ret;
    
    printf("\n=== Testing -v flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -v", TEMP_GCOV_DUMP);
    
    ret = execute_and_capture(cmd, output, sizeof(output), 0);
    
    if (WIFEXITED(ret) && WEXITSTATUS(ret) == 0) {
        if (strstr(output, "gcov-dump") || strstr(output, "version") || 
            strstr(output, "GCC")) {
            printf("✓ -v flag test passed (version info found)\n");
            printf("  Output: %s\n", output);
        } else {
            printf("✗ -v flag test failed (no version info in output)\n");
        }
    } else {
        printf("✗ -v flag test failed (exit code %d)\n", WEXITSTATUS(ret));
    }
}

/* Test invalid flag */
void test_invalid_flag() {
    char output[1024];
    char cmd[256];
    int ret;
    
    printf("\n=== Testing invalid flag -X ===\n");
    snprintf(cmd, sizeof(cmd), "%s -X", TEMP_GCOV_DUMP);
    
    ret = execute_and_capture(cmd, output, sizeof(output), 1);
    
    if (strstr(output, "unknown flag `X'")) {
        printf("✓ Invalid flag test passed (correct error message)\n");
        printf("  Error: %s\n", output);
    } else {
        printf("✗ Invalid flag test failed (missing or wrong error message)\n");
        printf("  Output: %s\n", output);
    }
}

/* Test flag with coverage file */
void test_flag_with_file(const char *flag, const char *description) {
    char cmd[256];
    int ret;
    
    printf("\n=== Testing %s flag (%s) ===\n", flag, description);
    snprintf(cmd, sizeof(cmd), "%s %s %s", TEMP_GCOV_DUMP, flag, TEMP_GCDA);
    
    ret = system(cmd);
    if (WIFEXITED(ret)) {
        printf("✓ %s flag test completed (exit code %d)\n", 
               flag, WEXITSTATUS(ret));
    } else {
        printf("✗ %s flag test failed\n", flag);
    }
}

/* Test combined flags */
void test_combined_flags(const char *flags, const char *description) {
    char cmd[256];
    int ret;
    
    printf("\n=== Testing combined flags %s (%s) ===\n", flags, description);
    snprintf(cmd, sizeof(cmd), "%s %s %s", TEMP_GCOV_DUMP, flags, TEMP_GCDA);
    
    ret = system(cmd);
    if (WIFEXITED(ret)) {
        printf("✓ Combined flags %s test completed (exit code %d)\n", 
               flags, WEXITSTATUS(ret));
    } else {
        printf("✗ Combined flags %s test failed\n", flags);
    }
}

/* Cleanup temporary files */
void cleanup() {
    printf("\n=== Cleaning up ===\n");
    
    if (unlink(TEMP_GCDA) == 0) {
        printf("Removed %s\n", TEMP_GCDA);
    }
    
    if (unlink(TEMP_GCOV_DUMP) == 0) {
        printf("Removed %s\n", TEMP_GCOV_DUMP);
    }
    
    /* Also remove coverage data files generated by instrumented gcov-dump */
    char coverage_files[][64] = {
        TEMP_GCOV_DUMP ".gcda",
        TEMP_GCOV_DUMP ".gcno",
        "gcov-dump-instrumented.gcda",
        "gcov-dump-instrumented.gcno"
    };
    
    for (size_t i = 0; i < sizeof(coverage_files)/sizeof(coverage_files[0]); i++) {
        if (unlink(coverage_files[i]) == 0) {
            printf("Removed %s\n", coverage_files[i]);
        }
    }
}

int main() {
    printf("=== GCOV-Dump Coverage Test Driver ===\n");
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_gcov_dump()) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    if (!create_minimal_gcda()) {
        return 1;
    }
    
    /* Step 3: Execute test sequence */
    
    /* Test flags without file arguments */
    test_help_flag();
    test_version_flag();
    test_invalid_flag();
    
    /* Test flags requiring coverage file */
    test_flag_with_file("-l", "dump contents");
    test_flag_with_file("-p", "dump positions");
    test_flag_with_file("-r", "dump raw");
    test_flag_with_file("-s", "dump stable");
    
    /* Test flag combinations and ordering */
    test_combined_flags("-l -p", "contents and positions");
    test_combined_flags("-p -l", "positions and contents (reversed)");
    test_combined_flags("-r -s", "raw and stable");
    test_combined_flags("-s -r", "stable and raw (reversed)");
    test_combined_flags("-l -p -r -s", "all flags combined");
    
    /* Test invalid flag with file (should still trigger unknown flag error) */
    printf("\n=== Testing invalid flag -X with file ===\n");
    {
        char output[1024];
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "%s -X %s", TEMP_GCOV_DUMP, TEMP_GCDA);
        int ret = execute_and_capture(cmd, output, sizeof(output), 1);
        
        if (strstr(output, "unknown flag `X'")) {
            printf("✓ Invalid flag with file test passed\n");
        } else {
            printf("✗ Invalid flag with file test failed\n");
        }
    }
    
    /* Step 4: Cleanup */
    cleanup();
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
