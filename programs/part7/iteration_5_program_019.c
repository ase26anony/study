#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_GCDA_FILE "/tmp/minimal.gcda"
#define TEMP_GCOV_DUMP "/tmp/gcov-dump-instrumented"

/* Minimal GCOV data file structure */
static const unsigned char minimal_gcda[] = {
    /* Magic: 'gcda' */
    0x67, 0x63, 0x64, 0x61,
    /* Version: 0x76312e2a (example version) */
    0x2a, 0x2e, 0x31, 0x76,
    /* Stamp: 0x00000000 */
    0x00, 0x00, 0x00, 0x00,
    /* Length of first record: 0 (empty record) */
    0x00, 0x00, 0x00, 0x00
};

/* Execute command and capture output */
int execute_and_capture(const char *cmd, char *output, size_t output_size, 
                        int capture_stdout, int capture_stderr) {
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
            if (total_read >= output_size - 1) {
                break;
            }
        }
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
}

/* Build instrumented gcov-dump */
int build_gcov_dump() {
    printf("Building instrumented gcov-dump...\n");
    
    /* Try to find gcov-dump.cc in common locations */
    const char *possible_paths[] = {
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "../../../gcc/gcov-dump.cc",
        "gcov-dump.cc",
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
        fprintf(stderr, "Error: Could not find gcov-dump.cc\n");
        return 0;
    }
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             TEMP_GCOV_DUMP, source_path);
    
    printf("Compiling: %s\n", cmd);
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Error: Failed to compile gcov-dump\n");
        return 0;
    }
    
    /* Verify the binary was created */
    if (stat(TEMP_GCOV_DUMP, &st) != 0 || !S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Instrumented binary not created\n");
        return 0;
    }
    
    printf("Successfully built instrumented gcov-dump\n");
    return 1;
}

/* Create minimal coverage data file */
int create_minimal_gcda() {
    printf("Creating minimal coverage data file...\n");
    
    FILE *fp = fopen(TEMP_GCDA_FILE, "wb");
    if (!fp) {
        perror("Error creating temporary gcda file");
        return 0;
    }
    
    size_t written = fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    if (written != sizeof(minimal_gcda)) {
        fprintf(stderr, "Error writing gcda file\n");
        return 0;
    }
    
    printf("Created %s (%zu bytes)\n", TEMP_GCDA_FILE, sizeof(minimal_gcda));
    return 1;
}

/* Test -h flag (help) */
void test_help_flag() {
    printf("\n=== Testing -h flag ===\n");
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s -h", TEMP_GCOV_DUMP);
    
    char output[4096] = "";
    int exit_code = execute_and_capture(cmd, output, sizeof(output), 1, 0);
    
    printf("Exit code: %d\n", exit_code);
    if (strstr(output, "Usage:") || strstr(output, "usage:")) {
        printf("✓ Help text printed\n");
    } else {
        printf("✗ Help text not found in output\n");
    }
}

/* Test -v flag (version) */
void test_version_flag() {
    printf("\n=== Testing -v flag ===\n");
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s -v", TEMP_GCOV_DUMP);
    
    char output[4096] = "";
    int exit_code = execute_and_capture(cmd, output, sizeof(output), 1, 0);
    
    printf("Exit code: %d\n", exit_code);
    if (strstr(output, "gcov-dump") || strstr(output, "GCC")) {
        printf("✓ Version information printed\n");
    } else {
        printf("✗ Version information not found\n");
    }
}

/* Test invalid flag */
void test_invalid_flag() {
    printf("\n=== Testing invalid flag -X ===\n");
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s -X", TEMP_GCOV_DUMP);
    
    char output[4096] = "";
    int exit_code = execute_and_capture(cmd, output, sizeof(output), 0, 1);
    
    printf("Exit code: %d\n", exit_code);
    if (strstr(output, "unknown flag `X'")) {
        printf("✓ Correct error message for invalid flag\n");
    } else {
        printf("✗ Expected error message not found. Output:\n%s\n", output);
    }
}

/* Test flag with coverage file */
void test_flag_with_file(const char *flag, const char *description) {
    printf("\n=== Testing %s flag (%s) ===\n", flag, description);
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s %s %s", TEMP_GCOV_DUMP, flag, TEMP_GCDA_FILE);
    
    char output[4096] = "";
    int exit_code = execute_and_capture(cmd, output, sizeof(output), 1, 1);
    
    printf("Exit code: %d\n", exit_code);
    printf("Command: %s\n", cmd);
    
    if (exit_code == 0) {
        printf("✓ Flag %s executed successfully\n", flag);
    } else {
        printf("✗ Flag %s failed with exit code %d\n", flag, exit_code);
        if (strlen(output) > 0) {
            printf("Output:\n%s\n", output);
        }
    }
}

/* Test combined flags */
void test_combined_flags(const char *flags, const char *description) {
    printf("\n=== Testing combined flags %s (%s) ===\n", flags, description);
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s %s %s", TEMP_GCOV_DUMP, flags, TEMP_GCDA_FILE);
    
    char output[4096] = "";
    int exit_code = execute_and_capture(cmd, output, sizeof(output), 1, 1);
    
    printf("Exit code: %d\n", exit_code);
    printf("Command: %s\n", cmd);
    
    if (exit_code == 0) {
        printf("✓ Combined flags %s executed successfully\n", flags);
    } else {
        printf("✗ Combined flags %s failed\n", flags);
    }
}

/* Test flag ordering variations */
void test_flag_ordering() {
    printf("\n=== Testing flag ordering variations ===\n");
    
    /* Test -l -p */
    {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "%s -l -p %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
        int exit_code = execute_and_capture(cmd, NULL, 0, 0, 0);
        printf("  -l -p: exit code %d\n", exit_code);
    }
    
    /* Test -p -l */
    {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "%s -p -l %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
        int exit_code = execute_and_capture(cmd, NULL, 0, 0, 0);
        printf("  -p -l: exit code %d\n", exit_code);
    }
    
    /* Test -r -s */
    {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "%s -r -s %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
        int exit_code = execute_and_capture(cmd, NULL, 0, 0, 0);
        printf("  -r -s: exit code %d\n", exit_code);
    }
    
    /* Test -s -r */
    {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "%s -s -r %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
        int exit_code = execute_and_capture(cmd, NULL, 0, 0, 0);
        printf("  -s -r: exit code %d\n", exit_code);
    }
}

/* Cleanup temporary files */
void cleanup() {
    printf("\n=== Cleaning up ===\n");
    
    if (remove(TEMP_GCDA_FILE) == 0) {
        printf("Removed %s\n", TEMP_GCDA_FILE);
    }
    
    if (remove(TEMP_GCOV_DUMP) == 0) {
        printf("Removed %s\n", TEMP_GCOV_DUMP);
    }
    
    /* Also remove coverage files generated by the instrumented binary */
    char coverage_files[][64] = {
        TEMP_GCOV_DUMP ".gcda",
        TEMP_GCOV_DUMP ".gcno",
        "gcov-dump-instrumented.gcda",
        "gcov-dump-instrumented.gcno",
    };
    
    for (size_t i = 0; i < sizeof(coverage_files)/sizeof(coverage_files[0]); i++) {
        if (remove(coverage_files[i]) == 0) {
            printf("Removed %s\n", coverage_files[i]);
        }
    }
}

int main() {
    printf("=== GCOV-Dump Coverage Test Program ===\n");
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_gcov_dump()) {
        fprintf(stderr, "Failed to build gcov-dump. Exiting.\n");
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    if (!create_minimal_gcda()) {
        fprintf(stderr, "Failed to create coverage file. Exiting.\n");
        return 1;
    }
    
    /* Step 3: Execute test sequence */
    
    /* Test flags without file arguments */
    test_help_flag();      /* Tests case 'h': print_usage() */
    test_version_flag();   /* Tests case 'v': print_version() */
    test_invalid_flag();   /* Tests default case: "unknown flag" error */
    
    /* Test flags that require coverage file */
    test_flag_with_file("-l", "dump contents");      /* Tests case 'l' */
    test_flag_with_file("-p", "dump positions");     /* Tests case 'p' */
    test_flag_with_file("-r", "dump raw");           /* Tests case 'r' */
    test_flag_with_file("-s", "dump stable");        /* Tests case 's' */
    
    /* Test flag combinations */
    test_combined_flags("-l -p", "contents and positions");
    test_combined_flags("-r -s", "raw and stable");
    
    /* Test flag ordering variations */
    test_flag_ordering();
    
    /* Step 4: Cleanup */
    cleanup();
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
