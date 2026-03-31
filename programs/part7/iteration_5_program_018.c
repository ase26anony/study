#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_GCDA_FILE "/tmp/minimal.gcda"
#define INSTRUMENTED_BINARY "/tmp/gcov-dump-instrumented"

/* Minimal GCOV data file format */
static const unsigned char minimal_gcda[] = {
    /* Magic: 'gcda' */
    0x67, 0x63, 0x64, 0x61,
    /* Version: 0x76303030 (v000) */
    0x76, 0x30, 0x30, 0x30,
    /* Stamp */
    0x00, 0x00, 0x00, 0x00,
    /* Zero-length record (tag=0, length=0) */
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

/* Execute command and capture output */
static char *run_command(const char *cmd, int capture_stdout, int *exit_status) {
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        return NULL;
    }
    
    char *output = NULL;
    size_t output_size = 0;
    size_t output_len = 0;
    
    if (capture_stdout) {
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            size_t len = strlen(buffer);
            char *new_output = realloc(output, output_len + len + 1);
            if (!new_output) {
                free(output);
                pclose(fp);
                return NULL;
            }
            output = new_output;
            memcpy(output + output_len, buffer, len);
            output_len += len;
            output[output_len] = '\0';
        }
    } else {
        /* Just consume output without capturing */
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            /* Do nothing, just read to avoid blocking */
        }
    }
    
    *exit_status = pclose(fp);
    return output;
}

/* Execute command and capture stderr */
static char *run_command_stderr(const char *cmd, int *exit_status) {
    /* Use a wrapper script or pipe to capture stderr */
    char full_cmd[1024];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    return run_command(full_cmd, 1, exit_status);
}

/* Create minimal GCOV data file */
static int create_minimal_gcda(void) {
    FILE *fp = fopen(TEMP_GCDA_FILE, "wb");
    if (!fp) {
        perror("Failed to create minimal gcda file");
        return 0;
    }
    
    size_t written = fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    return written == sizeof(minimal_gcda);
}

/* Build instrumented gcov-dump */
static int build_instrumented_gcov_dump(void) {
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
        fprintf(stderr, "Could not find gcov-dump.cc\n");
        return 0;
    }
    
    printf("Found gcov-dump.cc at: %s\n", source_path);
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    printf("Compiling: %s\n", cmd);
    int status = system(cmd);
    
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("Successfully built instrumented gcov-dump\n");
        return 1;
    } else {
        fprintf(stderr, "Failed to build gcov-dump (status=%d)\n", status);
        return 0;
    }
}

/* Test -h flag (help) */
static void test_help_flag(void) {
    printf("Testing -h flag...\n");
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s -h", INSTRUMENTED_BINARY);
    
    int exit_status;
    char *output = run_command(cmd, 1, &exit_status);
    
    if (output) {
        printf("Help output (first 100 chars): %.100s\n", output);
        free(output);
    }
    
    if (WIFEXITED(exit_status) && WEXITSTATUS(exit_status) == 0) {
        printf("✓ -h flag test passed (exit code 0)\n");
    } else {
        printf("✗ -h flag test failed (exit code %d)\n", WEXITSTATUS(exit_status));
    }
}

/* Test -v flag (version) */
static void test_version_flag(void) {
    printf("Testing -v flag...\n");
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s -v", INSTRUMENTED_BINARY);
    
    int exit_status;
    char *output = run_command(cmd, 1, &exit_status);
    
    if (output) {
        printf("Version output: %s", output);
        if (strstr(output, "gcov-dump") || strstr(output, "version") || 
            strstr(output, "GCC")) {
            printf("✓ -v flag test passed (version info found)\n");
        } else {
            printf("✗ -v flag test failed (no version info)\n");
        }
        free(output);
    } else {
        printf("✗ -v flag test failed (no output)\n");
    }
}

/* Test invalid flag */
static void test_invalid_flag(void) {
    printf("Testing invalid flag -X...\n");
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s -X", INSTRUMENTED_BINARY);
    
    int exit_status;
    char *output = run_command_stderr(cmd, &exit_status);
    
    if (output) {
        printf("Stderr output: %s", output);
        if (strstr(output, "unknown flag `X'")) {
            printf("✓ Invalid flag test passed (correct error message)\n");
        } else {
            printf("✗ Invalid flag test failed (wrong error message)\n");
        }
        free(output);
    } else {
        printf("✗ Invalid flag test failed (no stderr output)\n");
    }
}

/* Test flag with minimal gcda file */
static void test_flag_with_file(const char *flag, const char *description) {
    printf("Testing %s flag (%s)...\n", flag, description);
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flag, TEMP_GCDA_FILE);
    
    int exit_status;
    char *output = run_command(cmd, 1, &exit_status);
    
    if (output) {
        printf("Output length: %zu bytes\n", strlen(output));
        free(output);
    }
    
    if (WIFEXITED(exit_status)) {
        printf("✓ %s flag test completed (exit code %d)\n", flag, WEXITSTATUS(exit_status));
    } else {
        printf("✗ %s flag test failed\n", flag);
    }
}

/* Test combined flags */
static void test_combined_flags(const char *flags, const char *description) {
    printf("Testing combined flags %s (%s)...\n", flags, description);
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flags, TEMP_GCDA_FILE);
    
    int exit_status;
    char *output = run_command(cmd, 1, &exit_status);
    
    if (output) {
        printf("Output length: %zu bytes\n", strlen(output));
        free(output);
    }
    
    if (WIFEXITED(exit_status)) {
        printf("✓ Combined flags %s test completed (exit code %d)\n", flags, WEXITSTATUS(exit_status));
    } else {
        printf("✗ Combined flags %s test failed\n", flags);
    }
}

/* Test flag ordering variations */
static void test_flag_ordering(void) {
    printf("Testing flag ordering variations...\n");
    
    /* Test -l -p */
    {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "%s -l -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
        
        int exit_status;
        char *output = run_command(cmd, 1, &exit_status);
        
        if (output) {
            free(output);
        }
        
        if (WIFEXITED(exit_status)) {
            printf("✓ -l -p ordering test passed\n");
        }
    }
    
    /* Test -p -l */
    {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "%s -p -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
        
        int exit_status;
        char *output = run_command(cmd, 1, &exit_status);
        
        if (output) {
            free(output);
        }
        
        if (WIFEXITED(exit_status)) {
            printf("✓ -p -l ordering test passed\n");
        }
    }
    
    /* Test -r -s */
    {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "%s -r -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
        
        int exit_status;
        char *output = run_command(cmd, 1, &exit_status);
        
        if (output) {
            free(output);
        }
        
        if (WIFEXITED(exit_status)) {
            printf("✓ -r -s ordering test passed\n");
        }
    }
}

/* Cleanup temporary files */
static void cleanup(void) {
    printf("Cleaning up temporary files...\n");
    
    if (unlink(TEMP_GCDA_FILE) == 0) {
        printf("Removed %s\n", TEMP_GCDA_FILE);
    }
    
    if (unlink(INSTRUMENTED_BINARY) == 0) {
        printf("Removed %s\n", INSTRUMENTED_BINARY);
    }
    
    /* Also remove coverage data files generated by the instrumented binary */
    char coverage_files[][64] = {
        "gcov-dump-instrumented.gcda",
        "gcov-dump-instrumented.gcno",
        INSTRUMENTED_BINARY ".gcda",
        INSTRUMENTED_BINARY ".gcno"
    };
    
    for (size_t i = 0; i < sizeof(coverage_files)/sizeof(coverage_files[0]); i++) {
        if (unlink(coverage_files[i]) == 0) {
            printf("Removed %s\n", coverage_files[i]);
        }
    }
}

int main(void) {
    printf("=== Starting gcov-dump coverage test ===\n\n");
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_instrumented_gcov_dump()) {
        fprintf(stderr, "Failed to build instrumented gcov-dump. Exiting.\n");
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    if (!create_minimal_gcda()) {
        fprintf(stderr, "Failed to create minimal gcda file. Exiting.\n");
        cleanup();
        return 1;
    }
    
    printf("\n=== Running flag tests ===\n\n");
    
    /* Step 3: Execute test sequence */
    
    /* Test flags without file arguments */
    test_help_flag();
    printf("\n");
    
    test_version_flag();
    printf("\n");
    
    test_invalid_flag();
    printf("\n");
    
    /* Test flags with minimal gcda file */
    test_flag_with_file("-l", "dump contents");
    printf("\n");
    
    test_flag_with_file("-p", "dump positions");
    printf("\n");
    
    test_flag_with_file("-r", "dump raw");
    printf("\n");
    
    test_flag_with_file("-s", "dump stable");
    printf("\n");
    
    /* Test combined flags */
    test_combined_flags("-l -p", "contents and positions");
    printf("\n");
    
    test_combined_flags("-r -s", "raw and stable");
    printf("\n");
    
    /* Test flag ordering */
    test_flag_ordering();
    printf("\n");
    
    /* Additional invalid flag test with file */
    printf("Testing invalid flag -X with file...\n");
    {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "%s -X %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
        
        int exit_status;
        char *output = run_command_stderr(cmd, &exit_status);
        
        if (output) {
            if (strstr(output, "unknown flag `X'")) {
                printf("✓ Invalid flag with file test passed\n");
            }
            free(output);
        }
    }
    
    printf("\n=== All tests completed ===\n");
    
    /* Step 4: Cleanup */
    cleanup();
    
    printf("\nTo generate coverage report for gcov-dump:\n");
    printf("1. Run: gcov %s\n", INSTRUMENTED_BINARY);
    printf("2. Check gcov-dump.cc.gcov for covered lines\n");
    
    return 0;
}
