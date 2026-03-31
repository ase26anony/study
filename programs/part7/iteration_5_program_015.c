#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

#define TEMP_GCDA_FILE "/tmp/minimal.gcda"
#define INSTRUMENTED_BINARY "/tmp/gcov-dump-instrumented"

/* Minimal GCOV data file structure */
static const unsigned char minimal_gcda[] = {
    /* Magic: 'gcda' */
    0x67, 0x63, 0x64, 0x61,
    /* Version: 0xB3C0D4A7 (example version) */
    0xB3, 0xC0, 0xD4, 0xA7,
    /* Stamp: 0x00000000 */
    0x00, 0x00, 0x00, 0x00,
    /* Length of next record: 0 (empty file) */
    0x00, 0x00, 0x00, 0x00
};

/* Execute command and capture output */
int execute_and_capture(const char *cmd, char *output, size_t output_size, 
                        int capture_stdout, int *exit_status) {
    FILE *fp;
    char buffer[1024];
    size_t total = 0;
    
    if (capture_stdout) {
        fp = popen(cmd, "r");
    } else {
        /* For stderr, we need to redirect */
        char cmd_with_stderr[1024];
        snprintf(cmd_with_stderr, sizeof(cmd_with_stderr), "%s 2>&1", cmd);
        fp = popen(cmd_with_stderr, "r");
    }
    
    if (!fp) {
        return -1;
    }
    
    output[0] = '\0';
    while (fgets(buffer, sizeof(buffer), fp) != NULL && total < output_size - 1) {
        size_t len = strlen(buffer);
        if (total + len < output_size) {
            strcat(output, buffer);
            total += len;
        }
    }
    
    *exit_status = pclose(fp);
    if (WIFEXITED(*exit_status)) {
        *exit_status = WEXITSTATUS(*exit_status);
    }
    
    return 0;
}

/* Build instrumented gcov-dump */
int build_gcov_dump(const char *source_path) {
    char cmd[1024];
    int status;
    
    printf("Building instrumented gcov-dump from %s...\n", source_path);
    
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s 2>&1",
             INSTRUMENTED_BINARY, source_path);
    
    status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to build gcov-dump. Command: %s\n", cmd);
        return -1;
    }
    
    if (access(INSTRUMENTED_BINARY, X_OK) != 0) {
        fprintf(stderr, "Instrumented binary not created: %s\n", INSTRUMENTED_BINARY);
        return -1;
    }
    
    printf("Successfully built instrumented gcov-dump\n");
    return 0;
}

/* Create minimal valid .gcda file */
int create_minimal_gcda() {
    FILE *fp = fopen(TEMP_GCDA_FILE, "wb");
    if (!fp) {
        perror("Failed to create temporary .gcda file");
        return -1;
    }
    
    fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    printf("Created minimal .gcda file: %s\n", TEMP_GCDA_FILE);
    return 0;
}

/* Test -h flag (help) */
void test_help_flag() {
    char output[4096];
    int exit_status;
    char cmd[256];
    
    printf("\n=== Testing -h flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -h", INSTRUMENTED_BINARY);
    
    if (execute_and_capture(cmd, output, sizeof(output), 1, &exit_status) == 0) {
        printf("Exit status: %d\n", exit_status);
        if (exit_status == 0) {
            printf("✓ -h flag test passed (exited successfully)\n");
        } else {
            printf("✗ -h flag test failed (exit status: %d)\n", exit_status);
        }
    } else {
        printf("✗ Failed to execute -h test\n");
    }
}

/* Test -v flag (version) */
void test_version_flag() {
    char output[4096];
    int exit_status;
    char cmd[256];
    
    printf("\n=== Testing -v flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s -v", INSTRUMENTED_BINARY);
    
    if (execute_and_capture(cmd, output, sizeof(output), 1, &exit_status) == 0) {
        printf("Exit status: %d\n", exit_status);
        printf("Output (first 200 chars):\n%.200s\n", output);
        
        if (strstr(output, "gcov-dump") != NULL || 
            strstr(output, "version") != NULL ||
            strstr(output, "GCC") != NULL) {
            printf("✓ -v flag test passed (version info printed)\n");
        } else {
            printf("✗ -v flag test failed (no version info found)\n");
        }
    } else {
        printf("✗ Failed to execute -v test\n");
    }
}

/* Test invalid flag */
void test_invalid_flag() {
    char output[4096];
    int exit_status;
    char cmd[256];
    
    printf("\n=== Testing invalid flag (-X) ===\n");
    snprintf(cmd, sizeof(cmd), "%s -X", INSTRUMENTED_BINARY);
    
    if (execute_and_capture(cmd, output, sizeof(output), 0, &exit_status) == 0) {
        printf("Exit status: %d\n", exit_status);
        printf("Stderr output:\n%s\n", output);
        
        if (strstr(output, "unknown flag `X'") != NULL) {
            printf("✓ Invalid flag test passed (correct error message)\n");
        } else {
            printf("✗ Invalid flag test failed (wrong error message)\n");
        }
    } else {
        printf("✗ Failed to execute invalid flag test\n");
    }
}

/* Test flag that requires a coverage file */
void test_file_flag(const char *flag, const char *description) {
    char output[4096];
    int exit_status;
    char cmd[256];
    
    printf("\n=== Testing %s flag (%s) ===\n", flag, description);
    snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flag, TEMP_GCDA_FILE);
    
    if (execute_and_capture(cmd, output, sizeof(output), 1, &exit_status) == 0) {
        printf("Exit status: %d\n", exit_status);
        printf("Output (first 200 chars):\n%.200s\n", output);
        
        if (exit_status == 0) {
            printf("✓ %s flag test passed\n", flag);
        } else {
            printf("✗ %s flag test failed (exit status: %d)\n", flag, exit_status);
        }
    } else {
        printf("✗ Failed to execute %s test\n", flag);
    }
}

/* Test flag combinations */
void test_flag_combination(const char *flags, const char *description) {
    char output[4096];
    int exit_status;
    char cmd[256];
    
    printf("\n=== Testing flag combination: %s ===\n", description);
    snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flags, TEMP_GCDA_FILE);
    
    if (execute_and_capture(cmd, output, sizeof(output), 1, &exit_status) == 0) {
        printf("Exit status: %d\n", exit_status);
        printf("Output (first 200 chars):\n%.200s\n", output);
        
        if (exit_status == 0) {
            printf("✓ Flag combination %s test passed\n", description);
        } else {
            printf("✗ Flag combination %s test failed (exit status: %d)\n", 
                   description, exit_status);
        }
    } else {
        printf("✗ Failed to execute flag combination test\n");
    }
}

/* Cleanup temporary files */
void cleanup() {
    printf("\n=== Cleaning up ===\n");
    
    if (remove(TEMP_GCDA_FILE) == 0) {
        printf("Removed temporary file: %s\n", TEMP_GCDA_FILE);
    }
    
    if (remove(INSTRUMENTED_BINARY) == 0) {
        printf("Removed instrumented binary: %s\n", INSTRUMENTED_BINARY);
    }
}

int main(int argc, char *argv[]) {
    const char *gcov_dump_source = NULL;
    
    /* Try to find gcov-dump.cc */
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
        
        for (int i = 0; possible_paths[i] != NULL; i++) {
            if (access(possible_paths[i], R_OK) == 0) {
                gcov_dump_source = possible_paths[i];
                break;
            }
        }
        
        if (!gcov_dump_source) {
            fprintf(stderr, "Error: gcov-dump.cc not found.\n");
            fprintf(stderr, "Usage: %s <path-to-gcov-dump.cc>\n", argv[0]);
            fprintf(stderr, "Or place this program in gcc build directory.\n");
            return 1;
        }
    }
    
    printf("Using gcov-dump source: %s\n", gcov_dump_source);
    
    /* Step 1: Build instrumented gcov-dump */
    if (build_gcov_dump(gcov_dump_source) != 0) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    if (create_minimal_gcda() != 0) {
        return 1;
    }
    
    /* Step 3: Execute test sequence */
    
    /* Test flags without file arguments */
    test_help_flag();
    test_version_flag();
    test_invalid_flag();
    
    /* Test flags that require coverage file */
    test_file_flag("-l", "dump contents");
    test_file_flag("-p", "dump positions");
    test_file_flag("-r", "dump raw");
    test_file_flag("-s", "dump stable");
    
    /* Test flag combinations and ordering */
    test_flag_combination("-l -p", "-l -p");
    test_flag_combination("-p -l", "-p -l (reverse order)");
    test_flag_combination("-r -s", "-r -s");
    test_flag_combination("-l -p -r -s", "all flags combined");
    
    /* Test invalid flag with file (should still trigger unknown flag error) */
    printf("\n=== Testing invalid flag with file ===\n");
    {
        char output[4096];
        int exit_status;
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "%s -X %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
        
        if (execute_and_capture(cmd, output, sizeof(output), 0, &exit_status) == 0) {
            printf("Exit status: %d\n", exit_status);
            printf("Stderr output:\n%s\n", output);
            
            if (strstr(output, "unknown flag `X'") != NULL) {
                printf("✓ Invalid flag with file test passed\n");
            } else {
                printf("✗ Invalid flag with file test failed\n");
            }
        }
    }
    
    /* Step 4: Cleanup */
    cleanup();
    
    printf("\n=== All tests completed ===\n");
    
    return 0;
}
