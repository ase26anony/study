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
    /* Version: 0x76312e2a (version string pointer) */
    0x2a, 0x2e, 0x31, 0x76,
    /* Stamp: 0 */
    0x00, 0x00, 0x00, 0x00,
    /* Length: 0 (no records) */
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
        char redirect_cmd[1024];
        snprintf(redirect_cmd, sizeof(redirect_cmd), "%s 2>&1", cmd);
        fp = popen(redirect_cmd, "r");
    }
    
    if (!fp) {
        return -1;
    }
    
    output[0] = '\0';
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        size_t len = strlen(buffer);
        if (total + len < output_size) {
            strcat(output, buffer);
            total += len;
        }
    }
    
    *exit_status = pclose(fp);
    return 0;
}

/* Check if string contains substring */
int contains_string(const char *haystack, const char *needle) {
    return strstr(haystack, needle) != NULL;
}

/* Build instrumented gcov-dump */
int build_gcov_dump(const char *source_path) {
    char cmd[1024];
    struct stat st;
    
    printf("Building instrumented gcov-dump...\n");
    
    /* Check if source exists */
    if (stat(source_path, &st) != 0) {
        fprintf(stderr, "Error: gcov-dump.cc not found at %s\n", source_path);
        return -1;
    }
    
    /* Compile with coverage flags */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    printf("Compile command: %s\n", cmd);
    
    int result = system(cmd);
    if (result != 0) {
        fprintf(stderr, "Error: Failed to compile gcov-dump\n");
        return -1;
    }
    
    /* Verify the binary was created */
    if (stat(INSTRUMENTED_BINARY, &st) != 0) {
        fprintf(stderr, "Error: Instrumented binary not created\n");
        return -1;
    }
    
    printf("Instrumented gcov-dump built successfully\n");
    return 0;
}

/* Create minimal coverage file */
int create_minimal_gcda() {
    FILE *fp = fopen(TEMP_GCDA_FILE, "wb");
    if (!fp) {
        perror("Error creating temporary gcda file");
        return -1;
    }
    
    fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    printf("Created minimal coverage file: %s\n", TEMP_GCDA_FILE);
    return 0;
}

/* Test -h flag (help) */
void test_help_flag() {
    printf("\n=== Testing -h flag ===\n");
    
    char cmd[256];
    char output[4096];
    int exit_status;
    
    snprintf(cmd, sizeof(cmd), "%s -h", INSTRUMENTED_BINARY);
    
    if (execute_and_capture(cmd, output, sizeof(output), 1, &exit_status) == 0) {
        printf("Exit status: %d\n", WEXITSTATUS(exit_status));
        printf("Output (first 200 chars): %.200s...\n", output);
        
        if (WEXITSTATUS(exit_status) == 0) {
            printf("✓ -h flag test passed\n");
        } else {
            printf("✗ -h flag test failed\n");
        }
    } else {
        printf("✗ Failed to execute -h test\n");
    }
}

/* Test -v flag (version) */
void test_version_flag() {
    printf("\n=== Testing -v flag ===\n");
    
    char cmd[256];
    char output[4096];
    int exit_status;
    
    snprintf(cmd, sizeof(cmd), "%s -v", INSTRUMENTED_BINARY);
    
    if (execute_and_capture(cmd, output, sizeof(output), 1, &exit_status) == 0) {
        printf("Exit status: %d\n", WEXITSTATUS(exit_status));
        printf("Output: %s", output);
        
        if (WEXITSTATUS(exit_status) == 0 && strlen(output) > 0) {
            printf("✓ -v flag test passed\n");
        } else {
            printf("✗ -v flag test failed\n");
        }
    } else {
        printf("✗ Failed to execute -v test\n");
    }
}

/* Test invalid flag */
void test_invalid_flag() {
    printf("\n=== Testing invalid flag -X ===\n");
    
    char cmd[256];
    char output[4096];
    int exit_status;
    
    snprintf(cmd, sizeof(cmd), "%s -X", INSTRUMENTED_BINARY);
    
    if (execute_and_capture(cmd, output, sizeof(output), 0, &exit_status) == 0) {
        printf("Exit status: %d\n", WEXITSTATUS(exit_status));
        printf("Stderr output: %s", output);
        
        if (contains_string(output, "unknown flag `X'")) {
            printf("✓ Invalid flag test passed - found expected error message\n");
        } else {
            printf("✗ Invalid flag test failed - missing expected error message\n");
        }
    } else {
        printf("✗ Failed to execute invalid flag test\n");
    }
}

/* Test flag with coverage file */
void test_flag_with_file(const char *flag, const char *description) {
    printf("\n=== Testing %s flag (%s) ===\n", flag, description);
    
    char cmd[256];
    char output[4096];
    int exit_status;
    
    snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flag, TEMP_GCDA_FILE);
    
    if (execute_and_capture(cmd, output, sizeof(output), 1, &exit_status) == 0) {
        printf("Exit status: %d\n", WEXITSTATUS(exit_status));
        printf("Output (first 100 chars): %.100s...\n", output);
        
        if (WEXITSTATUS(exit_status) == 0) {
            printf("✓ %s flag test passed\n", flag);
        } else {
            printf("✗ %s flag test failed\n", flag);
        }
    } else {
        printf("✗ Failed to execute %s test\n", flag);
    }
}

/* Test flag combination */
void test_flag_combination(const char *flags, const char *description) {
    printf("\n=== Testing flag combination %s (%s) ===\n", flags, description);
    
    char cmd[256];
    char output[4096];
    int exit_status;
    
    snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flags, TEMP_GCDA_FILE);
    
    if (execute_and_capture(cmd, output, sizeof(output), 1, &exit_status) == 0) {
        printf("Exit status: %d\n", WEXITSTATUS(exit_status));
        printf("Output (first 100 chars): %.100s...\n", output);
        
        if (WEXITSTATUS(exit_status) == 0) {
            printf("✓ Flag combination %s test passed\n", flags);
        } else {
            printf("✗ Flag combination %s test failed\n", flags);
        }
    } else {
        printf("✗ Failed to execute flag combination %s test\n", flags);
    }
}

/* Test flag ordering variations */
void test_flag_ordering() {
    printf("\n=== Testing flag ordering variations ===\n");
    
    char cmd1[256], cmd2[256];
    char output1[4096], output2[4096];
    int exit_status1, exit_status2;
    
    /* Test -l -p vs -p -l */
    snprintf(cmd1, sizeof(cmd1), "%s -l -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    snprintf(cmd2, sizeof(cmd2), "%s -p -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    
    execute_and_capture(cmd1, output1, sizeof(output1), 1, &exit_status1);
    execute_and_capture(cmd2, output2, sizeof(output2), 1, &exit_status2);
    
    printf("Order -l -p: exit status %d\n", WEXITSTATUS(exit_status1));
    printf("Order -p -l: exit status %d\n", WEXITSTATUS(exit_status2));
    
    if (WEXITSTATUS(exit_status1) == 0 && WEXITSTATUS(exit_status2) == 0) {
        printf("✓ Flag ordering test passed\n");
    } else {
        printf("✗ Flag ordering test failed\n");
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
            struct stat st;
            if (stat(possible_paths[i], &st) == 0) {
                gcov_dump_source = possible_paths[i];
                break;
            }
        }
        
        if (!gcov_dump_source) {
            fprintf(stderr, "Error: gcov-dump.cc not found. Please specify path as argument.\n");
            fprintf(stderr, "Usage: %s <path-to-gcov-dump.cc>\n", argv[0]);
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
    
    /* Test flags without file argument */
    test_help_flag();
    test_version_flag();
    test_invalid_flag();
    
    /* Test flags with coverage file */
    test_flag_with_file("-l", "dump contents");
    test_flag_with_file("-p", "dump positions");
    test_flag_with_file("-r", "dump raw");
    test_flag_with_file("-s", "dump stable");
    
    /* Test flag combinations */
    test_flag_combination("-l -p", "dump contents and positions");
    test_flag_combination("-r -s", "dump raw and stable");
    
    /* Test flag ordering */
    test_flag_ordering();
    
    /* Step 4: Cleanup */
    cleanup();
    
    printf("\n=== All tests completed ===\n");
    
    return 0;
}
