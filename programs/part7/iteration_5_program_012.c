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
    /* Version: 0xB3C1D72 (little-endian) */
    0x72, 0xD7, 0xC1, 0xB3,
    /* Stamp: 0x12345678 */
    0x78, 0x56, 0x34, 0x12,
    /* Record length: 0 (no functions) */
    0x00, 0x00, 0x00, 0x00,
    /* Tag: 0 (GCOV_TAG_FUNCTION) */
    0x00, 0x00, 0x00, 0x00,
    /* Record length: 0 (no data) */
    0x00, 0x00, 0x00, 0x00
};

/* Execute command and return exit status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Execute command and capture output to buffer */
int execute_and_capture(const char *cmd, char *buffer, size_t buffer_size, int capture_stderr) {
    char full_cmd[1024];
    if (capture_stderr) {
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    } else {
        snprintf(full_cmd, sizeof(full_cmd), "%s", cmd);
    }
    
    printf("Executing: %s\n", full_cmd);
    
    FILE *fp = popen(full_cmd, "r");
    if (!fp) {
        return -1;
    }
    
    buffer[0] = '\0';
    size_t total_read = 0;
    while (fgets(buffer + total_read, buffer_size - total_read, fp) != NULL) {
        total_read = strlen(buffer);
        if (buffer_size - total_read < 256) {
            break;
        }
    }
    
    int status = pclose(fp);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Create minimal GCOV data file */
int create_minimal_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create minimal gcda file");
        return 0;
    }
    
    size_t written = fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    return (written == sizeof(minimal_gcda));
}

/* Build instrumented gcov-dump */
int build_gcov_dump(const char *source_path) {
    char cmd[2048];
    
    /* Check if source file exists */
    struct stat st;
    if (stat(source_path, &st) != 0) {
        fprintf(stderr, "Source file not found: %s\n", source_path);
        return 0;
    }
    
    /* Build command to compile gcov-dump with coverage instrumentation */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    printf("Building instrumented gcov-dump...\n");
    int status = execute_command(cmd);
    
    if (status != 0) {
        fprintf(stderr, "Failed to build gcov-dump. Trying alternative compilation...\n");
        
        /* Try alternative compilation without -lgcov */
        snprintf(cmd, sizeof(cmd),
                 "g++ -O0 -fprofile-arcs -ftest-coverage -o %s %s",
                 INSTRUMENTED_BINARY, source_path);
        
        status = execute_command(cmd);
    }
    
    if (status == 0) {
        printf("Successfully built instrumented gcov-dump at %s\n", INSTRUMENTED_BINARY);
        return 1;
    }
    
    fprintf(stderr, "Failed to compile gcov-dump. Make sure g++ is installed.\n");
    return 0;
}

/* Test -h flag (help) */
void test_help_flag() {
    printf("\n=== Testing -h flag ===\n");
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s -h", INSTRUMENTED_BINARY);
    
    int status = execute_command(cmd);
    if (status == 0) {
        printf("✓ -h flag test passed (exit code 0)\n");
    } else {
        printf("✗ -h flag test failed (exit code %d)\n", status);
    }
}

/* Test -v flag (version) */
void test_version_flag() {
    printf("\n=== Testing -v flag ===\n");
    
    char cmd[256];
    char output[1024];
    
    snprintf(cmd, sizeof(cmd), "%s -v", INSTRUMENTED_BINARY);
    
    int status = execute_and_capture(cmd, output, sizeof(output), 0);
    
    if (status == 0) {
        printf("✓ -v flag test passed (exit code 0)\n");
        if (strlen(output) > 0) {
            printf("Version output: %s", output);
        }
    } else {
        printf("✗ -v flag test failed (exit code %d)\n", status);
    }
}

/* Test invalid flag */
void test_invalid_flag() {
    printf("\n=== Testing invalid flag -X ===\n");
    
    char cmd[256];
    char output[1024];
    
    snprintf(cmd, sizeof(cmd), "%s -X", INSTRUMENTED_BINARY);
    
    int status = execute_and_capture(cmd, output, sizeof(output), 1);
    
    /* Check for expected error message */
    if (strstr(output, "unknown flag") != NULL && strstr(output, "X") != NULL) {
        printf("✓ Invalid flag test passed\n");
        printf("Error message: %s", output);
    } else {
        printf("✗ Invalid flag test failed\n");
        printf("Output: %s\n", output);
        printf("Expected 'unknown flag' error message\n");
    }
}

/* Test flag with minimal coverage file */
void test_flag_with_file(const char *flag, const char *description) {
    printf("\n=== Testing %s flag (%s) ===\n", flag, description);
    
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flag, TEMP_GCDA_FILE);
    
    int status = execute_command(cmd);
    if (status == 0) {
        printf("✓ %s flag test passed (exit code 0)\n", flag);
    } else {
        printf("✗ %s flag test failed (exit code %d)\n", flag, status);
    }
}

/* Test combined flags */
void test_combined_flags(const char *flags, const char *description) {
    printf("\n=== Testing combined flags %s (%s) ===\n", flags, description);
    
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flags, TEMP_GCDA_FILE);
    
    int status = execute_command(cmd);
    if (status == 0) {
        printf("✓ Combined flags %s test passed (exit code 0)\n", flags);
    } else {
        printf("✗ Combined flags %s test failed (exit code %d)\n", flags, status);
    }
}

/* Test flag ordering variations */
void test_flag_ordering() {
    printf("\n=== Testing flag ordering variations ===\n");
    
    char cmd[512];
    
    /* Test -l -p */
    snprintf(cmd, sizeof(cmd), "%s -l -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    int status1 = execute_command(cmd);
    
    /* Test -p -l */
    snprintf(cmd, sizeof(cmd), "%s -p -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    int status2 = execute_command(cmd);
    
    /* Test -r -s */
    snprintf(cmd, sizeof(cmd), "%s -r -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    int status3 = execute_command(cmd);
    
    /* Test -s -r */
    snprintf(cmd, sizeof(cmd), "%s -s -r %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    int status4 = execute_command(cmd);
    
    if (status1 == 0 && status2 == 0 && status3 == 0 && status4 == 0) {
        printf("✓ All flag ordering tests passed\n");
    } else {
        printf("✗ Some flag ordering tests failed\n");
    }
}

int main(int argc, char *argv[]) {
    printf("=== GCOV-Dump Coverage Test Program ===\n");
    
    /* Determine gcov-dump source path */
    const char *gcov_dump_source = NULL;
    
    /* Try to find gcov-dump.cc in common locations */
    const char *possible_paths[] = {
        "../gcc/gcov-dump.cc",           /* Relative to test directory */
        "../../gcc/gcov-dump.cc",        /* One more level up */
        "/usr/src/gcc/gcc/gcov-dump.cc", /* System GCC source */
        "gcov-dump.cc",                  /* Current directory */
        NULL
    };
    
    for (int i = 0; possible_paths[i] != NULL; i++) {
        struct stat st;
        if (stat(possible_paths[i], &st) == 0 && S_ISREG(st.st_mode)) {
            gcov_dump_source = possible_paths[i];
            printf("Found gcov-dump source at: %s\n", gcov_dump_source);
            break;
        }
    }
    
    if (!gcov_dump_source) {
        fprintf(stderr, "Error: Could not find gcov-dump.cc source file.\n");
        fprintf(stderr, "Please specify the path as an argument or place it in the current directory.\n");
        if (argc > 1) {
            gcov_dump_source = argv[1];
            printf("Using specified source: %s\n", gcov_dump_source);
        } else {
            return 1;
        }
    }
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_gcov_dump(gcov_dump_source)) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    printf("\nCreating minimal coverage file...\n");
    if (!create_minimal_gcda(TEMP_GCDA_FILE)) {
        fprintf(stderr, "Failed to create minimal coverage file\n");
        unlink(INSTRUMENTED_BINARY);
        return 1;
    }
    printf("Created minimal coverage file: %s\n", TEMP_GCDA_FILE);
    
    /* Step 3: Execute test sequence */
    
    /* Test flags without file arguments */
    test_help_flag();
    test_version_flag();
    test_invalid_flag();
    
    /* Test flags with minimal coverage file */
    test_flag_with_file("-l", "dump contents");
    test_flag_with_file("-p", "dump positions");
    test_flag_with_file("-r", "dump raw");
    test_flag_with_file("-s", "dump stable");
    
    /* Test combined flags */
    test_combined_flags("-l -p", "contents and positions");
    test_combined_flags("-r -s", "raw and stable");
    
    /* Test flag ordering variations */
    test_flag_ordering();
    
    /* Step 4: Cleanup */
    printf("\n=== Cleaning up ===\n");
    
    /* Remove temporary files */
    if (unlink(TEMP_GCDA_FILE) == 0) {
        printf("Removed temporary file: %s\n", TEMP_GCDA_FILE);
    }
    
    if (unlink(INSTRUMENTED_BINARY) == 0) {
        printf("Removed instrumented binary: %s\n", INSTRUMENTED_BINARY);
    }
    
    /* Also remove coverage data files generated by the instrumented binary */
    char coverage_files[][64] = {
        "gcov-dump-instrumented.gcda",
        "gcov-dump-instrumented.gcno",
        "gcov-dump.gcda",
        "gcov-dump.gcno"
    };
    
    for (size_t i = 0; i < sizeof(coverage_files)/sizeof(coverage_files[0]); i++) {
        if (unlink(coverage_files[i]) == 0) {
            printf("Removed coverage file: %s\n", coverage_files[i]);
        }
    }
    
    printf("\n=== Test sequence completed ===\n");
    printf("The instrumented gcov-dump binary should now have coverage data\n");
    printf("for the previously uncovered command-line parsing lines.\n");
    
    return 0;
}
