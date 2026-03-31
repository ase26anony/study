#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_GCDA_FILE "/tmp/minimal.gcda"
#define TEMP_GCOV_DUMP "/tmp/gcov-dump-instrumented"

/* Minimal GCOV data file format:
 * - Magic number: 0x67636461 ('gcda')
 * - Version: 0x3430372a ('407*' for GCC 4.7 format)
 * - Empty record: 0 (zero length indicates end of file)
 */
static const unsigned char minimal_gcda[] = {
    0x67, 0x63, 0x64, 0x61,  /* 'gcda' magic */
    0x34, 0x30, 0x37, 0x2a,  /* '407*' version */
    0x00, 0x00, 0x00, 0x00   /* zero-length record */
};

/* Execute command and capture output */
static int execute_and_capture(const char *cmd, char *output, size_t output_size, 
                               int capture_stdout, int capture_stderr) {
    char buffer[1024];
    FILE *fp;
    int status;
    
    if (capture_stderr) {
        char cmd_with_stderr[1024];
        snprintf(cmd_with_stderr, sizeof(cmd_with_stderr), "%s 2>&1", cmd);
        fp = popen(cmd_with_stderr, "r");
    } else {
        fp = popen(cmd, "r");
    }
    
    if (!fp) {
        return -1;
    }
    
    output[0] = '\0';
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strlen(output) + strlen(buffer) < output_size) {
            strcat(output, buffer);
        }
    }
    
    status = pclose(fp);
    return WEXITSTATUS(status);
}

/* Create minimal valid .gcda file */
static int create_minimal_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create .gcda file");
        return -1;
    }
    
    size_t written = fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    return (written == sizeof(minimal_gcda)) ? 0 : -1;
}

/* Build instrumented gcov-dump */
static int build_gcov_dump(const char *source_path) {
    char cmd[2048];
    struct stat st;
    
    /* Check if source exists */
    if (stat(source_path, &st) != 0) {
        fprintf(stderr, "gcov-dump.cc not found at: %s\n", source_path);
        fprintf(stderr, "Trying common locations...\n");
        
        /* Try common GCC source locations */
        const char *locations[] = {
            "../gcc/gcov-dump.cc",
            "../../gcc/gcov-dump.cc",
            "../../../gcc/gcov-dump.cc",
            "/usr/src/gcc/gcc/gcov-dump.cc",
            NULL
        };
        
        for (int i = 0; locations[i]; i++) {
            if (stat(locations[i], &st) == 0) {
                source_path = locations[i];
                printf("Found gcov-dump.cc at: %s\n", source_path);
                break;
            }
        }
        
        if (stat(source_path, &st) != 0) {
            fprintf(stderr, "Please specify path to gcov-dump.cc\n");
            return -1;
        }
    }
    
    /* Build with coverage instrumentation */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             TEMP_GCOV_DUMP, source_path);
    
    printf("Building instrumented gcov-dump: %s\n", cmd);
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Failed to build gcov-dump\n");
        return -1;
    }
    
    /* Verify the binary was created */
    if (stat(TEMP_GCOV_DUMP, &st) != 0) {
        fprintf(stderr, "Binary not created: %s\n", TEMP_GCOV_DUMP);
        return -1;
    }
    
    printf("Successfully built instrumented gcov-dump\n");
    return 0;
}

/* Test -h flag (help) */
static int test_help_flag(void) {
    printf("Testing -h flag...\n");
    
    char cmd[256];
    char output[4096] = "";
    
    snprintf(cmd, sizeof(cmd), "%s -h", TEMP_GCOV_DUMP);
    int status = execute_and_capture(cmd, output, sizeof(output), 1, 0);
    
    if (status == 0 && strlen(output) > 0) {
        printf("✓ -h flag successful (exit code: %d)\n", status);
        return 0;
    } else {
        printf("✗ -h flag failed (exit code: %d)\n", status);
        return -1;
    }
}

/* Test -v flag (version) */
static int test_version_flag(void) {
    printf("Testing -v flag...\n");
    
    char cmd[256];
    char output[4096] = "";
    
    snprintf(cmd, sizeof(cmd), "%s -v", TEMP_GCOV_DUMP);
    int status = execute_and_capture(cmd, output, sizeof(output), 1, 0);
    
    if (status == 0 && strstr(output, "gcov-dump") != NULL) {
        printf("✓ -v flag successful. Output contains version info\n");
        return 0;
    } else {
        printf("✗ -v flag failed (exit code: %d)\n", status);
        printf("Output: %s\n", output);
        return -1;
    }
}

/* Test invalid flag */
static int test_invalid_flag(void) {
    printf("Testing invalid flag -X...\n");
    
    char cmd[256];
    char output[4096] = "";
    
    snprintf(cmd, sizeof(cmd), "%s -X", TEMP_GCOV_DUMP);
    int status = execute_and_capture(cmd, output, sizeof(output), 0, 1);
    
    if (strstr(output, "unknown flag `X'") != NULL) {
        printf("✓ Invalid flag test successful. Got expected error message\n");
        return 0;
    } else {
        printf("✗ Invalid flag test failed\n");
        printf("Expected: 'unknown flag `X''\n");
        printf("Got: %s\n", output);
        return -1;
    }
}

/* Test flag with minimal .gcda file */
static int test_flag_with_file(const char *flag, const char *description) {
    printf("Testing %s flag (%s)...\n", flag, description);
    
    char cmd[256];
    char output[4096] = "";
    
    snprintf(cmd, sizeof(cmd), "%s %s %s", TEMP_GCOV_DUMP, flag, TEMP_GCDA_FILE);
    int status = execute_and_capture(cmd, output, sizeof(output), 1, 0);
    
    if (status == 0) {
        printf("✓ %s flag successful (exit code: %d)\n", flag, status);
        return 0;
    } else {
        printf("✗ %s flag failed (exit code: %d)\n", flag, status);
        if (strlen(output) > 0) {
            printf("Output: %s\n", output);
        }
        return -1;
    }
}

/* Test flag combination */
static int test_flag_combination(const char *flags, const char *description) {
    printf("Testing flag combination %s (%s)...\n", flags, description);
    
    char cmd[256];
    char output[4096] = "";
    
    snprintf(cmd, sizeof(cmd), "%s %s %s", TEMP_GCOV_DUMP, flags, TEMP_GCDA_FILE);
    int status = execute_and_capture(cmd, output, sizeof(output), 1, 0);
    
    if (status == 0) {
        printf("✓ Flag combination %s successful (exit code: %d)\n", flags, status);
        return 0;
    } else {
        printf("✗ Flag combination %s failed (exit code: %d)\n", flags, status);
        return -1;
    }
}

/* Test flag ordering variations */
static int test_flag_ordering(void) {
    printf("Testing flag ordering variations...\n");
    
    int failed = 0;
    char cmd[256];
    char output[4096] = "";
    
    /* Test -l -p */
    printf("  Testing -l -p...\n");
    snprintf(cmd, sizeof(cmd), "%s -l -p %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    if (execute_and_capture(cmd, output, sizeof(output), 1, 0) != 0) {
        printf("  ✗ -l -p failed\n");
        failed++;
    }
    
    /* Test -p -l */
    printf("  Testing -p -l...\n");
    snprintf(cmd, sizeof(cmd), "%s -p -l %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    if (execute_and_capture(cmd, output, sizeof(output), 1, 0) != 0) {
        printf("  ✗ -p -l failed\n");
        failed++;
    }
    
    /* Test -r -s */
    printf("  Testing -r -s...\n");
    snprintf(cmd, sizeof(cmd), "%s -r -s %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    if (execute_and_capture(cmd, output, sizeof(output), 1, 0) != 0) {
        printf("  ✗ -r -s failed\n");
        failed++;
    }
    
    /* Test -s -r */
    printf("  Testing -s -r...\n");
    snprintf(cmd, sizeof(cmd), "%s -s -r %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    if (execute_and_capture(cmd, output, sizeof(output), 1, 0) != 0) {
        printf("  ✗ -s -r failed\n");
        failed++;
    }
    
    if (failed == 0) {
        printf("✓ All flag ordering tests passed\n");
    }
    
    return failed;
}

int main(int argc, char *argv[]) {
    const char *gcov_dump_source = "gcov-dump.cc";
    
    if (argc > 1) {
        gcov_dump_source = argv[1];
    }
    
    printf("=== GCOV-Dump Coverage Test ===\n");
    printf("Source: %s\n", gcov_dump_source);
    
    /* Step 1: Build instrumented gcov-dump */
    if (build_gcov_dump(gcov_dump_source) != 0) {
        return EXIT_FAILURE;
    }
    
    /* Step 2: Create minimal .gcda file */
    printf("Creating minimal .gcda file...\n");
    if (create_minimal_gcda(TEMP_GCDA_FILE) != 0) {
        fprintf(stderr, "Failed to create minimal .gcda file\n");
        unlink(TEMP_GCOV_DUMP);
        return EXIT_FAILURE;
    }
    printf("Created minimal .gcda file: %s\n", TEMP_GCDA_FILE);
    
    int tests_failed = 0;
    
    /* Step 3: Execute test sequence */
    
    /* Test flags without file arguments */
    printf("\n--- Testing flags without file arguments ---\n");
    if (test_help_flag() != 0) tests_failed++;
    if (test_version_flag() != 0) tests_failed++;
    if (test_invalid_flag() != 0) tests_failed++;
    
    /* Test flags with minimal .gcda file */
    printf("\n--- Testing flags with minimal .gcda file ---\n");
    if (test_flag_with_file("-l", "dump contents") != 0) tests_failed++;
    if (test_flag_with_file("-p", "dump positions") != 0) tests_failed++;
    if (test_flag_with_file("-r", "dump raw") != 0) tests_failed++;
    if (test_flag_with_file("-s", "dump stable") != 0) tests_failed++;
    
    /* Test flag combinations */
    printf("\n--- Testing flag combinations ---\n");
    if (test_flag_combination("-l -p", "contents + positions") != 0) tests_failed++;
    if (test_flag_combination("-r -s", "raw + stable") != 0) tests_failed++;
    
    /* Test flag ordering variations */
    printf("\n--- Testing flag ordering variations ---\n");
    tests_failed += test_flag_ordering();
    
    /* Step 4: Cleanup */
    printf("\n--- Cleanup ---\n");
    unlink(TEMP_GCDA_FILE);
    unlink(TEMP_GCOV_DUMP);
    
    /* Also remove coverage data files created by instrumented gcov-dump */
    char coverage_files[][64] = {
        "gcov-dump-instrumented.gcda",
        "gcov-dump-instrumented.gcno",
        TEMP_GCOV_DUMP ".gcda",
        TEMP_GCOV_DUMP ".gcno"
    };
    
    for (size_t i = 0; i < sizeof(coverage_files)/sizeof(coverage_files[0]); i++) {
        if (unlink(coverage_files[i]) == 0) {
            printf("Removed: %s\n", coverage_files[i]);
        }
    }
    
    printf("\n=== Test Summary ===\n");
    if (tests_failed == 0) {
        printf("✓ All tests passed!\n");
        printf("The uncovered lines in gcov-dump.cc should now be covered.\n");
        printf("Run 'gcov gcov-dump.cc' on the instrumented binary to verify.\n");
        return EXIT_SUCCESS;
    } else {
        printf("✗ %d test(s) failed\n", tests_failed);
        return EXIT_FAILURE;
    }
}
