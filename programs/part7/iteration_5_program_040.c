#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define TEMP_GCDA "/tmp/minimal.gcda"
#define TEMP_GCOV_DUMP "/tmp/gcov-dump-instrumented"

/* Minimal valid GCOV data file structure */
static const unsigned char minimal_gcda[] = {
    /* Magic: 'gcda' */
    0x67, 0x63, 0x64, 0x61,
    /* Version: 0x3430372a (gcov 4.7 format) */
    0x2a, 0x37, 0x30, 0x34,
    /* Stamp */
    0x00, 0x00, 0x00, 0x00,
    /* Length of first record: 0 (empty function) */
    0x00, 0x00, 0x00, 0x00,
    /* Record tag: GCOV_TAG_FUNCTION (0x01000000) */
    0x00, 0x00, 0x00, 0x01,
    /* Record length: 2 (ident and checksum) */
    0x02, 0x00, 0x00, 0x00,
    /* Function ident */
    0x01, 0x00, 0x00, 0x00,
    /* Function checksum */
    0x00, 0x00, 0x00, 0x00,
    /* EOF marker: 0 */
    0x00, 0x00, 0x00, 0x00
};

/* Execute command and capture output */
int execute_and_capture(const char *cmd, char *output, size_t output_size, 
                       int capture_stdout, int capture_stderr) {
    char full_cmd[1024];
    FILE *fp;
    int result;
    
    if (capture_stderr) {
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    } else {
        strncpy(full_cmd, cmd, sizeof(full_cmd));
    }
    
    fp = popen(full_cmd, "r");
    if (!fp) {
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
int build_gcov_dump() {
    char cmd[2048];
    int status;
    
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
    
    printf("Found source at: %s\n", source_path);
    
    /* Build with coverage instrumentation */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             TEMP_GCOV_DUMP, source_path);
    
    status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Error: Failed to compile gcov-dump: %s\n", cmd);
        return 0;
    }
    
    if (stat(TEMP_GCOV_DUMP, &st) != 0) {
        fprintf(stderr, "Error: Compiled binary not found\n");
        return 0;
    }
    
    printf("Successfully built instrumented gcov-dump\n");
    return 1;
}

/* Create minimal valid coverage file */
int create_minimal_gcda() {
    FILE *fp = fopen(TEMP_GCDA, "wb");
    if (!fp) {
        perror("Error creating temporary gcda file");
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
    char output[1024];
    int exit_code;
    
    printf("\n=== Testing -h flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -h", TEMP_GCOV_DUMP);
    
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1, 0);
    
    if (exit_code == 0) {
        printf("✓ -h flag test passed (exit code: %d)\n", exit_code);
    } else {
        printf("✗ -h flag test failed (exit code: %d)\n", exit_code);
    }
}

/* Test -v flag (version) */
void test_version_flag() {
    char cmd[256];
    char output[1024];
    int exit_code;
    
    printf("\n=== Testing -v flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -v", TEMP_GCOV_DUMP);
    
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1, 0);
    
    if (exit_code == 0 && strstr(output, "gcov-dump") != NULL) {
        printf("✓ -v flag test passed\n");
        printf("Version output: %s", output);
    } else {
        printf("✗ -v flag test failed\n");
        printf("Output: %s\n", output);
    }
}

/* Test invalid flag */
void test_invalid_flag() {
    char cmd[256];
    char output[1024];
    int exit_code;
    
    printf("\n=== Testing invalid flag -X ===\n");
    snprintf(cmd, sizeof(cmd), "%s -X", TEMP_GCOV_DUMP);
    
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0, 1);
    
    if (strstr(output, "unknown flag `X'") != NULL) {
        printf("✓ Invalid flag test passed\n");
        printf("Expected error found: %s", output);
    } else {
        printf("✗ Invalid flag test failed\n");
        printf("Output: %s\n", output);
    }
}

/* Test flag with coverage file */
void test_flag_with_file(const char *flag, const char *description) {
    char cmd[256];
    char output[1024];
    int exit_code;
    
    printf("\n=== Testing %s flag (%s) ===\n", flag, description);
    snprintf(cmd, sizeof(cmd), "%s %s %s", TEMP_GCOV_DUMP, flag, TEMP_GCDA);
    
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1, 0);
    
    if (exit_code == 0) {
        printf("✓ %s flag test passed\n", flag);
    } else {
        printf("✗ %s flag test failed (exit code: %d)\n", flag, exit_code);
        printf("Output: %s\n", output);
    }
}

/* Test flag combination */
void test_flag_combination(const char *flags, const char *description) {
    char cmd[256];
    char output[1024];
    int exit_code;
    
    printf("\n=== Testing flag combination %s (%s) ===\n", flags, description);
    snprintf(cmd, sizeof(cmd), "%s %s %s", TEMP_GCOV_DUMP, flags, TEMP_GCDA);
    
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1, 0);
    
    if (exit_code == 0) {
        printf("✓ Flag combination %s test passed\n", flags);
    } else {
        printf("✗ Flag combination %s test failed (exit code: %d)\n", flags, exit_code);
    }
}

/* Test flag ordering variations */
void test_flag_ordering() {
    char cmd[256];
    char output[1024];
    int exit_code;
    
    printf("\n=== Testing flag ordering (-l -p vs -p -l) ===\n");
    
    /* Test -l -p */
    snprintf(cmd, sizeof(cmd), "%s -l -p %s", TEMP_GCOV_DUMP, TEMP_GCDA);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1, 0);
    printf("  -l -p: exit code %d\n", exit_code);
    
    /* Test -p -l */
    snprintf(cmd, sizeof(cmd), "%s -p -l %s", TEMP_GCOV_DUMP, TEMP_GCDA);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1, 0);
    printf("  -p -l: exit code %d\n", exit_code);
    
    /* Test -r -s */
    snprintf(cmd, sizeof(cmd), "%s -r -s %s", TEMP_GCOV_DUMP, TEMP_GCDA);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1, 0);
    printf("  -r -s: exit code %d\n", exit_code);
    
    /* Test -s -r */
    snprintf(cmd, sizeof(cmd), "%s -s -r %s", TEMP_GCOV_DUMP, TEMP_GCDA);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1, 0);
    printf("  -s -r: exit code %d\n", exit_code);
}

/* Cleanup temporary files */
void cleanup() {
    printf("\n=== Cleaning up ===\n");
    
    if (remove(TEMP_GCDA) == 0) {
        printf("Removed: %s\n", TEMP_GCDA);
    }
    
    if (remove(TEMP_GCOV_DUMP) == 0) {
        printf("Removed: %s\n", TEMP_GCOV_DUMP);
    }
    
    /* Also remove coverage files generated by instrumented gcov-dump */
    char gcov_files[256];
    snprintf(gcov_files, sizeof(gcov_files), "rm -f %s.gcno %s.gcda", 
             TEMP_GCOV_DUMP, TEMP_GCOV_DUMP);
    system(gcov_files);
}

int main() {
    printf("=== GCOV-Dump Coverage Test Program ===\n");
    
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
    
    /* Test flags that require coverage file */
    test_flag_with_file("-l", "dump contents");
    test_flag_with_file("-p", "dump positions");
    test_flag_with_file("-r", "dump raw");
    test_flag_with_file("-s", "dump stable");
    
    /* Test flag combinations */
    test_flag_combination("-l -p", "contents and positions");
    test_flag_combination("-r -s", "raw and stable");
    
    /* Test flag ordering variations */
    test_flag_ordering();
    
    /* Additional invalid flag test with file */
    printf("\n=== Testing invalid flag with file ===\n");
    char cmd[256];
    char output[1024];
    snprintf(cmd, sizeof(cmd), "%s -X %s", TEMP_GCOV_DUMP, TEMP_GCDA);
    int exit_code = execute_and_capture(cmd, output, sizeof(output), 0, 1);
    if (strstr(output, "unknown flag `X'") != NULL) {
        printf("✓ Invalid flag with file test passed\n");
    } else {
        printf("✗ Invalid flag with file test failed\n");
    }
    
    /* Step 4: Cleanup */
    cleanup();
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
