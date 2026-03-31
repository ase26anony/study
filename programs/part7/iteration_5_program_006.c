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
 * - Magic number: 0x67636461 ('gcda')
 * - Version: 0x3430372a ('407*' for GCC 4.7+ format)
 * - Stamp: 0 (can be any value)
 * - Zero-length record: 0 (indicates end of file)
 */
static const unsigned char minimal_gcda[] = {
    0x67, 0x63, 0x64, 0x61,     /* Magic: 'gcda' */
    0x34, 0x30, 0x37, 0x2a,     /* Version: '407*' */
    0x00, 0x00, 0x00, 0x00,     /* Stamp: 0 */
    0x00, 0x00, 0x00, 0x00      /* Zero record: 0 */
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

/* Compile gcov-dump with coverage instrumentation */
static int compile_gcov_dump(const char *source_path) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             TEMP_GCOV_DUMP, source_path);
    
    printf("Compiling gcov-dump: %s\n", cmd);
    int status = system(cmd);
    
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("Successfully compiled instrumented gcov-dump\n");
        return 1;
    } else {
        fprintf(stderr, "Failed to compile gcov-dump\n");
        return 0;
    }
}

/* Execute gcov-dump with given arguments and capture output */
static int execute_gcov_dump(const char *args, char *output, size_t output_size, 
                            int capture_stderr, int *exit_code) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s %s", TEMP_GCOV_DUMP, args);
    
    printf("Executing: %s\n", cmd);
    
    /* Use popen with stderr redirection if needed */
    const char *redir = capture_stderr ? " 2>&1" : "";
    strcat(cmd, redir);
    
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        return 0;
    }
    
    /* Read output */
    size_t total_read = 0;
    while (total_read < output_size - 1) {
        size_t n = fread(output + total_read, 1, output_size - total_read - 1, fp);
        if (n == 0) break;
        total_read += n;
    }
    output[total_read] = '\0';
    
    /* Get exit code */
    int status = pclose(fp);
    if (WIFEXITED(status)) {
        *exit_code = WEXITSTATUS(status);
    } else {
        *exit_code = -1;
    }
    
    return 1;
}

/* Test -h flag (help) */
static void test_help_flag(void) {
    printf("\n=== Testing -h flag ===\n");
    
    char output[4096] = {0};
    int exit_code;
    
    if (execute_gcov_dump("-h", output, sizeof(output), 0, &exit_code)) {
        printf("Exit code: %d\n", exit_code);
        printf("Output length: %zu bytes\n", strlen(output));
        
        if (exit_code == 0) {
            printf("✓ -h flag test passed\n");
        } else {
            printf("✗ -h flag test failed (non-zero exit code)\n");
        }
    }
}

/* Test -v flag (version) */
static void test_version_flag(void) {
    printf("\n=== Testing -v flag ===\n");
    
    char output[4096] = {0};
    int exit_code;
    
    if (execute_gcov_dump("-v", output, sizeof(output), 0, &exit_code)) {
        printf("Exit code: %d\n", exit_code);
        
        /* Check if version info is printed */
        if (strstr(output, "gcov-dump") || strstr(output, "GCC")) {
            printf("✓ Version information found\n");
        } else {
            printf("Output: %s\n", output);
            printf("✗ No version information found\n");
        }
    }
}

/* Test invalid flag */
static void test_invalid_flag(void) {
    printf("\n=== Testing invalid flag -X ===\n");
    
    char output[4096] = {0};
    int exit_code;
    
    if (execute_gcov_dump("-X", output, sizeof(output), 1, &exit_code)) {
        printf("Exit code: %d\n", exit_code);
        printf("Output: %s\n", output);
        
        /* Check for the exact error message from the uncovered lines */
        if (strstr(output, "unknown flag `X'")) {
            printf("✓ Invalid flag error message found\n");
        } else {
            printf("✗ Expected error message not found\n");
        }
    }
}

/* Test flags that require a .gcda file */
static void test_data_flags(void) {
    printf("\n=== Testing flags requiring .gcda file ===\n");
    
    char output[4096] = {0};
    int exit_code;
    
    /* Test individual flags */
    const char *flags[] = {"-l", "-p", "-r", "-s", NULL};
    
    for (int i = 0; flags[i]; i++) {
        printf("\nTesting flag %s:\n", flags[i]);
        
        char args[256];
        snprintf(args, sizeof(args), "%s %s", flags[i], TEMP_GCDA_FILE);
        
        memset(output, 0, sizeof(output));
        if (execute_gcov_dump(args, output, sizeof(output), 1, &exit_code)) {
            printf("  Exit code: %d\n", exit_code);
            
            /* These might produce errors with minimal file, but should parse flags */
            if (exit_code != 0 && strstr(output, "not a gcov data file")) {
                printf("  ✓ Flag parsed (got expected file format error)\n");
            } else if (exit_code == 0) {
                printf("  ✓ Flag parsed successfully\n");
            }
        }
    }
    
    /* Test flag combinations */
    printf("\nTesting flag combinations:\n");
    
    const char *combinations[] = {
        "-l -p",
        "-p -l",  /* Different order */
        "-r -s",
        "-l -p -r -s",
        NULL
    };
    
    for (int i = 0; combinations[i]; i++) {
        printf("\nTesting combination: %s\n", combinations[i]);
        
        char args[256];
        snprintf(args, sizeof(args), "%s %s", combinations[i], TEMP_GCDA_FILE);
        
        memset(output, 0, sizeof(output));
        if (execute_gcov_dump(args, output, sizeof(output), 1, &exit_code)) {
            printf("  Exit code: %d\n", exit_code);
        }
    }
}

int main(int argc, char *argv[]) {
    printf("=== GCOV-Dump Coverage Test Program ===\n");
    
    /* Step 1: Find and compile gcov-dump */
    const char *gcov_dump_source = NULL;
    
    /* Try to find gcov-dump.cc in common locations */
    const char *possible_paths[] = {
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "../../../gcc/gcov-dump.cc",
        "/usr/src/gcc/gcc/gcov-dump.cc",
        NULL
    };
    
    for (int i = 0; possible_paths[i]; i++) {
        struct stat st;
        if (stat(possible_paths[i], &st) == 0 && S_ISREG(st.st_mode)) {
            gcov_dump_source = possible_paths[i];
            printf("Found gcov-dump.cc at: %s\n", gcov_dump_source);
            break;
        }
    }
    
    if (!gcov_dump_source) {
        /* Try using command line argument */
        if (argc > 1) {
            gcov_dump_source = argv[1];
        } else {
            fprintf(stderr, "Could not find gcov-dump.cc\n");
            fprintf(stderr, "Usage: %s [path/to/gcov-dump.cc]\n", argv[0]);
            return 1;
        }
    }
    
    if (!compile_gcov_dump(gcov_dump_source)) {
        return 1;
    }
    
    /* Step 2: Create minimal .gcda file */
    printf("\nCreating minimal .gcda file...\n");
    if (!create_minimal_gcda(TEMP_GCDA_FILE)) {
        fprintf(stderr, "Failed to create minimal .gcda file\n");
        unlink(TEMP_GCOV_DUMP);
        return 1;
    }
    printf("Created %s (%zu bytes)\n", TEMP_GCDA_FILE, sizeof(minimal_gcda));
    
    /* Step 3: Execute test sequence */
    test_help_flag();
    test_version_flag();
    test_data_flags();
    test_invalid_flag();
    
    /* Step 4: Cleanup */
    printf("\n=== Cleaning up ===\n");
    unlink(TEMP_GCDA_FILE);
    unlink(TEMP_GCOV_DUMP);
    printf("Removed temporary files\n");
    
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
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
