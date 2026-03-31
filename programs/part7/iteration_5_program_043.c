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
    /* Version: 0x76312e2a (version magic for gcc) */
    0x2a, 0x2e, 0x31, 0x76,
    /* Zero timestamp */
    0x00, 0x00, 0x00, 0x00,
    /* Zero checksum */
    0x00, 0x00, 0x00, 0x00,
    /* Zero records (length = 0) */
    0x00, 0x00, 0x00, 0x00
};

/* Execute command and capture output */
char* execute_command(const char* cmd, int capture_stderr) {
    char* result = NULL;
    size_t result_size = 0;
    size_t result_len = 0;
    char buffer[1024];
    FILE* fp;
    
    if (capture_stderr) {
        fp = popen(cmd, "r");
    } else {
        /* For stderr capture, we need to redirect */
        char cmd_with_redirect[1024];
        snprintf(cmd_with_redirect, sizeof(cmd_with_redirect), "%s 2>&1", cmd);
        fp = popen(cmd_with_redirect, "r");
    }
    
    if (!fp) {
        perror("popen failed");
        return NULL;
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        size_t buffer_len = strlen(buffer);
        if (result_len + buffer_len + 1 > result_size) {
            result_size = (result_size == 0) ? 1024 : result_size * 2;
            result = realloc(result, result_size);
            if (!result) {
                perror("realloc failed");
                pclose(fp);
                return NULL;
            }
        }
        memcpy(result + result_len, buffer, buffer_len);
        result_len += buffer_len;
        result[result_len] = '\0';
    }
    
    pclose(fp);
    return result;
}

/* Check if string contains substring */
int contains_string(const char* str, const char* substr) {
    return str && substr && strstr(str, substr) != NULL;
}

/* Create minimal coverage file */
int create_minimal_gcda() {
    FILE* fp = fopen(TEMP_GCDA_FILE, "wb");
    if (!fp) {
        perror("Failed to create minimal gcda file");
        return 0;
    }
    
    fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    /* Verify file was created */
    struct stat st;
    if (stat(TEMP_GCDA_FILE, &st) != 0 || st.st_size == 0) {
        fprintf(stderr, "Failed to create valid gcda file\n");
        return 0;
    }
    
    return 1;
}

/* Build instrumented gcov-dump */
int build_instrumented_gcov_dump() {
    char cmd[1024];
    int status;
    
    printf("Building instrumented gcov-dump...\n");
    
    /* Try to find gcov-dump.cc in common locations */
    const char* possible_paths[] = {
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "../../../gcc/gcov-dump.cc",
        "/usr/src/gcc/gcc/gcov-dump.cc",
        NULL
    };
    
    const char* source_path = NULL;
    for (int i = 0; possible_paths[i]; i++) {
        if (access(possible_paths[i], R_OK) == 0) {
            source_path = possible_paths[i];
            break;
        }
    }
    
    if (!source_path) {
        fprintf(stderr, "Could not find gcov-dump.cc source file\n");
        return 0;
    }
    
    printf("Found source at: %s\n", source_path);
    
    /* Compile with coverage instrumentation */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    printf("Compiling: %s\n", cmd);
    status = system(cmd);
    
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        if (access(INSTRUMENTED_BINARY, X_OK) == 0) {
            printf("Successfully built instrumented gcov-dump\n");
            return 1;
        }
    }
    
    fprintf(stderr, "Failed to build instrumented gcov-dump\n");
    return 0;
}

/* Test individual flag */
void test_flag(const char* flag, const char* file_arg, 
               int expect_success, const char* expected_output) {
    char cmd[1024];
    int status;
    
    printf("\nTesting flag: %s\n", flag);
    
    if (file_arg) {
        snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flag, file_arg);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s", INSTRUMENTED_BINARY, flag);
    }
    
    printf("Executing: %s\n", cmd);
    
    if (expected_output && strstr(expected_output, "unknown flag")) {
        /* For invalid flags, capture stderr */
        char* output = execute_command(cmd, 0); /* 0 means capture stderr */
        if (output) {
            printf("Output: %s", output);
            if (contains_string(output, expected_output)) {
                printf("✓ Found expected error message\n");
            } else {
                printf("✗ Did not find expected error message\n");
            }
            free(output);
        }
    } else {
        /* For valid flags, just check exit status */
        status = system(cmd);
        if (expect_success) {
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                printf("✓ Command succeeded as expected\n");
            } else {
                printf("✗ Command failed unexpectedly\n");
            }
        } else {
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                printf("✓ Command failed as expected\n");
            } else {
                printf("✗ Command succeeded unexpectedly\n");
            }
        }
    }
}

/* Test flag combination */
void test_flag_combination(const char* flags, const char* file_arg) {
    char cmd[1024];
    int status;
    
    printf("\nTesting flag combination: %s\n", flags);
    
    snprintf(cmd, sizeof(cmd), "%s %s %s", INSTRUMENTED_BINARY, flags, file_arg);
    printf("Executing: %s\n", cmd);
    
    status = system(cmd);
    if (WIFEXITED(status)) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
    }
}

int main() {
    printf("=== Starting gcov-dump coverage test ===\n");
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_instrumented_gcov_dump()) {
        fprintf(stderr, "Failed to build instrumented binary\n");
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    if (!create_minimal_gcda()) {
        fprintf(stderr, "Failed to create minimal coverage file\n");
        unlink(INSTRUMENTED_BINARY);
        return 1;
    }
    
    /* Step 3: Execute test sequence */
    
    /* Test -h flag (help) */
    test_flag("-h", NULL, 1, NULL);
    
    /* Test -v flag (version) */
    test_flag("-v", NULL, 1, NULL);
    
    /* Test flags that require coverage file */
    test_flag("-l", TEMP_GCDA_FILE, 1, NULL);
    test_flag("-p", TEMP_GCDA_FILE, 1, NULL);
    test_flag("-r", TEMP_GCDA_FILE, 1, NULL);
    test_flag("-s", TEMP_GCDA_FILE, 1, NULL);
    
    /* Test flag combinations */
    test_flag_combination("-l -p", TEMP_GCDA_FILE);
    test_flag_combination("-p -l", TEMP_GCDA_FILE);
    test_flag_combination("-r -s", TEMP_GCDA_FILE);
    test_flag_combination("-l -p -r -s", TEMP_GCDA_FILE);
    
    /* Test invalid flag */
    test_flag("-X", TEMP_GCDA_FILE, 0, "unknown flag `X'");
    
    /* Test invalid flag without file */
    test_flag("-Y", NULL, 0, "unknown flag `Y'");
    
    /* Step 4: Cleanup */
    printf("\n=== Cleaning up ===\n");
    unlink(TEMP_GCDA_FILE);
    unlink(INSTRUMENTED_BINARY);
    
    /* Also clean up coverage files generated by instrumented binary */
    char coverage_files[][64] = {
        "gcov-dump-instrumented.gcda",
        "gcov-dump-instrumented.gcno",
        "gcov-dump.gcda",
        "gcov-dump.gcno"
    };
    
    for (size_t i = 0; i < sizeof(coverage_files)/sizeof(coverage_files[0]); i++) {
        if (access(coverage_files[i], F_OK) == 0) {
            unlink(coverage_files[i]);
        }
    }
    
    printf("Test completed\n");
    return 0;
}
