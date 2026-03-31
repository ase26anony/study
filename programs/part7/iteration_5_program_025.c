#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_GCDA_FILE "/tmp/minimal.gcda"
#define TEMP_GCOV_DUMP "/tmp/gcov-dump-instrumented"

/* Minimal GCOV data file structure:
   - Magic number: 0x67636461 ('gcda')
   - Version: 0x3430392a ('409*' for GCC 4.9 format)
   - Zero-length record: 0x00000000
*/
static const unsigned char minimal_gcda[] = {
    0x67, 0x63, 0x64, 0x61,  /* 'gcda' magic */
    0x34, 0x30, 0x39, 0x2a,  /* '409*' version */
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
    
    return (written == sizeof(minimal_gcda));
}

/* Execute command and capture output */
static int execute_and_capture(const char *cmd, char *output, size_t output_size, 
                               int *exit_status, int capture_stderr) {
    char full_cmd[1024];
    if (capture_stderr) {
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    } else {
        snprintf(full_cmd, sizeof(full_cmd), "%s", cmd);
    }
    
    FILE *fp = popen(full_cmd, "r");
    if (!fp) {
        perror("popen failed");
        return 0;
    }
    
    if (output && output_size > 0) {
        output[0] = '\0';
        size_t total_read = 0;
        while (fgets(output + total_read, output_size - total_read, fp) != NULL) {
            total_read = strlen(output);
            if (total_read >= output_size - 1) break;
        }
    }
    
    int status = pclose(fp);
    if (exit_status) {
        *exit_status = WEXITSTATUS(status);
    }
    
    return 1;
}

/* Build instrumented gcov-dump */
static int build_gcov_dump(const char *source_path) {
    char cmd[2048];
    
    printf("Building instrumented gcov-dump from: %s\n", source_path);
    
    /* Check if source file exists */
    struct stat st;
    if (stat(source_path, &st) != 0) {
        fprintf(stderr, "Source file not found: %s\n", source_path);
        
        /* Try to find it in common locations */
        const char *possible_paths[] = {
            "../gcc/gcov-dump.cc",
            "../../gcc/gcov-dump.cc",
            "../../../gcc/gcov-dump.cc",
            "/usr/src/gcc/gcc/gcov-dump.cc",
            NULL
        };
        
        for (int i = 0; possible_paths[i]; i++) {
            if (stat(possible_paths[i], &st) == 0) {
                source_path = possible_paths[i];
                printf("Found source at: %s\n", source_path);
                break;
            }
        }
        
        if (stat(source_path, &st) != 0) {
            fprintf(stderr, "Could not locate gcov-dump.cc\n");
            return 0;
        }
    }
    
    /* Compile with coverage instrumentation */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             TEMP_GCOV_DUMP, source_path);
    
    printf("Compiling: %s\n", cmd);
    
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Compilation failed with status %d\n", status);
        return 0;
    }
    
    /* Verify the binary was created */
    if (stat(TEMP_GCOV_DUMP, &st) != 0) {
        fprintf(stderr, "Binary not created: %s\n", TEMP_GCOV_DUMP);
        return 0;
    }
    
    printf("Successfully built instrumented gcov-dump\n");
    return 1;
}

/* Test individual flag */
static void test_flag(const char *flag, const char *filename, 
                      int expect_success, const char *expected_output,
                      int check_stderr) {
    char cmd[1024];
    char output[4096];
    int exit_status;
    
    printf("\n=== Testing flag: %s ===\n", flag);
    
    if (filename) {
        snprintf(cmd, sizeof(cmd), "%s %s %s", TEMP_GCOV_DUMP, flag, filename);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s", TEMP_GCOV_DUMP, flag);
    }
    
    printf("Command: %s\n", cmd);
    
    if (!execute_and_capture(cmd, output, sizeof(output), &exit_status, check_stderr)) {
        printf("FAILED: Command execution failed\n");
        return;
    }
    
    if (expect_success) {
        if (exit_status == 0) {
            printf("PASS: Exit status 0 as expected\n");
        } else {
            printf("FAIL: Expected exit status 0, got %d\n", exit_status);
        }
    } else {
        if (exit_status != 0) {
            printf("PASS: Non-zero exit status as expected\n");
        } else {
            printf("FAIL: Expected non-zero exit status, got 0\n");
        }
    }
    
    if (expected_output) {
        if (strstr(output, expected_output) != NULL) {
            printf("PASS: Found expected output: %s\n", expected_output);
        } else {
            printf("FAIL: Expected output not found. Got:\n%s\n", output);
        }
    }
    
    /* Print first few lines of output for debugging */
    if (strlen(output) > 0) {
        printf("Output (first 200 chars):\n%.200s\n", output);
        if (strlen(output) > 200) {
            printf("... (truncated)\n");
        }
    }
}

/* Test invalid flag */
static void test_invalid_flag(const char *invalid_flag) {
    char cmd[1024];
    char output[4096];
    int exit_status;
    char expected_error[256];
    
    printf("\n=== Testing invalid flag: %s ===\n", invalid_flag);
    
    snprintf(cmd, sizeof(cmd), "%s %s %s", TEMP_GCOV_DUMP, invalid_flag, TEMP_GCDA_FILE);
    printf("Command: %s\n", cmd);
    
    /* Capture stderr */
    if (!execute_and_capture(cmd, output, sizeof(output), &exit_status, 1)) {
        printf("FAILED: Command execution failed\n");
        return;
    }
    
    /* Check for expected error message */
    snprintf(expected_error, sizeof(expected_error), "unknown flag `%c'", invalid_flag[1]);
    
    if (strstr(output, expected_error) != NULL) {
        printf("PASS: Found expected error: %s\n", expected_error);
    } else {
        printf("FAIL: Expected error not found. Got:\n%s\n", output);
    }
    
    if (exit_status != 0) {
        printf("PASS: Non-zero exit status as expected\n");
    } else {
        printf("FAIL: Expected non-zero exit status, got 0\n");
    }
}

int main(int argc, char *argv[]) {
    printf("=== GCOV-Dump Coverage Test Program ===\n");
    
    /* Determine source path */
    const char *source_path = "gcov-dump.cc";
    if (argc > 1) {
        source_path = argv[1];
    }
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_gcov_dump(source_path)) {
        fprintf(stderr, "Failed to build gcov-dump. Exiting.\n");
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    printf("\nCreating minimal .gcda file...\n");
    if (!create_minimal_gcda(TEMP_GCDA_FILE)) {
        fprintf(stderr, "Failed to create minimal .gcda file\n");
        unlink(TEMP_GCOV_DUMP);
        return 1;
    }
    printf("Created: %s\n", TEMP_GCDA_FILE);
    
    /* Step 3: Execute test sequence */
    
    /* Test -h flag (help) - no file needed */
    test_flag("-h", NULL, 1, "Usage:", 0);
    
    /* Test -v flag (version) - no file needed */
    test_flag("-v", NULL, 1, "gcov-dump", 0);
    
    /* Test flags that require a coverage file */
    test_flag("-l", TEMP_GCDA_FILE, 1, NULL, 0);
    test_flag("-p", TEMP_GCDA_FILE, 1, NULL, 0);
    test_flag("-r", TEMP_GCDA_FILE, 1, NULL, 0);
    test_flag("-s", TEMP_GCDA_FILE, 1, NULL, 0);
    
    /* Test flag combinations and ordering */
    test_flag("-l -p", TEMP_GCDA_FILE, 1, NULL, 0);
    test_flag("-p -l", TEMP_GCDA_FILE, 1, NULL, 0);
    test_flag("-r -s", TEMP_GCDA_FILE, 1, NULL, 0);
    test_flag("-s -r", TEMP_GCDA_FILE, 1, NULL, 0);
    
    /* Test invalid flag */
    test_invalid_flag("-X");
    
    /* Test another invalid flag to ensure coverage */
    test_invalid_flag("-Z");
    
    /* Step 4: Cleanup */
    printf("\n=== Cleaning up ===\n");
    
    if (unlink(TEMP_GCDA_FILE) == 0) {
        printf("Removed: %s\n", TEMP_GCDA_FILE);
    } else {
        perror("Failed to remove .gcda file");
    }
    
    if (unlink(TEMP_GCOV_DUMP) == 0) {
        printf("Removed: %s\n", TEMP_GCOV_DUMP);
    } else {
        perror("Failed to remove gcov-dump binary");
    }
    
    /* Also remove coverage data files generated by running the instrumented binary */
    char coverage_files[][64] = {
        TEMP_GCOV_DUMP ".gcda",
        TEMP_GCOV_DUMP ".gcno",
        NULL
    };
    
    for (int i = 0; coverage_files[i][0]; i++) {
        if (unlink(coverage_files[i]) == 0) {
            printf("Removed: %s\n", coverage_files[i]);
        }
    }
    
    printf("\n=== Test Complete ===\n");
    printf("Now run 'gcov gcov-dump.cc' to see coverage of the switch cases\n");
    
    return 0;
}
