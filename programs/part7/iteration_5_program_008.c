#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_GCDA_FILE "/tmp/minimal.gcda"
#define TEMP_GCOV_DUMP "/tmp/gcov-dump-instrumented"

/* Minimal GCOV data file format */
static const unsigned char minimal_gcda[] = {
    /* Magic: 'gcda' */
    0x67, 0x63, 0x64, 0x61,
    /* Version: 0x3430372A (B70*) */
    0x2A, 0x37, 0x30, 0x34,
    /* Stamp: 0 */
    0x00, 0x00, 0x00, 0x00,
    /* Zero-length record terminator */
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
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
    
    return written == sizeof(minimal_gcda);
}

/* Build instrumented gcov-dump */
static int build_instrumented_gcov_dump(const char *source_path) {
    char cmd[1024];
    
    /* Check if source file exists */
    struct stat st;
    if (stat(source_path, &st) != 0) {
        fprintf(stderr, "Source file not found: %s\n", source_path);
        return 0;
    }
    
    /* Build command to compile with coverage instrumentation */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             TEMP_GCOV_DUMP, source_path);
    
    printf("Building instrumented gcov-dump: %s\n", cmd);
    
    int result = system(cmd);
    if (result != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump (exit code: %d)\n", 
                WEXITSTATUS(result));
        return 0;
    }
    
    /* Verify the binary was created */
    if (stat(TEMP_GCOV_DUMP, &st) != 0) {
        fprintf(stderr, "Instrumented binary not created: %s\n", TEMP_GCOV_DUMP);
        return 0;
    }
    
    return 1;
}

/* Run gcov-dump with specified arguments and capture output */
static int run_gcov_dump(const char *args, char *output, size_t output_size, 
                         int capture_stderr) {
    char cmd[1024];
    FILE *fp;
    int result;
    
    snprintf(cmd, sizeof(cmd), "%s %s 2>&1", TEMP_GCOV_DUMP, args);
    
    if (output_size > 0) {
        output[0] = '\0';
    }
    
    fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        return -1;
    }
    
    if (output_size > 0) {
        size_t total_read = 0;
        while (fgets(output + total_read, output_size - total_read, fp) != NULL) {
            total_read = strlen(output);
            if (total_read >= output_size - 1) {
                break;
            }
        }
    } else {
        /* Just consume output */
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            /* Do nothing, just read */
        }
    }
    
    result = pclose(fp);
    return WEXITSTATUS(result);
}

/* Test -h flag (help) */
static int test_help_flag(void) {
    printf("Testing -h flag...\n");
    
    int exit_code = run_gcov_dump("-h", NULL, 0, 0);
    
    if (exit_code == 0) {
        printf("  ✓ -h flag exited successfully (code: %d)\n", exit_code);
        return 1;
    } else {
        printf("  ✗ -h flag failed (exit code: %d)\n", exit_code);
        return 0;
    }
}

/* Test -v flag (version) */
static int test_version_flag(void) {
    printf("Testing -v flag...\n");
    
    char output[1024];
    int exit_code = run_gcov_dump("-v", output, sizeof(output), 0);
    
    if (exit_code == 0) {
        printf("  ✓ -v flag exited successfully\n");
        /* Check if version info is printed */
        if (strstr(output, "gcov-dump") != NULL || 
            strstr(output, "GCC") != NULL ||
            strstr(output, "version") != NULL) {
            printf("  ✓ Version information printed\n");
        } else {
            printf("  ? Version output format unexpected\n");
        }
        return 1;
    } else {
        printf("  ✗ -v flag failed (exit code: %d)\n", exit_code);
        return 0;
    }
}

/* Test invalid flag */
static int test_invalid_flag(void) {
    printf("Testing invalid flag -X...\n");
    
    char output[1024];
    int exit_code = run_gcov_dump("-X", output, sizeof(output), 1);
    
    /* Search for the expected error message */
    if (strstr(output, "unknown flag") != NULL && 
        strstr(output, "X") != NULL) {
        printf("  ✓ Invalid flag error message found: %s", output);
        return 1;
    } else {
        printf("  ✗ Expected error message not found. Output: %s\n", output);
        return 0;
    }
}

/* Test flag with minimal .gcda file */
static int test_flag_with_file(const char *flag, const char *description) {
    printf("Testing %s flag (%s)...\n", flag, description);
    
    char args[256];
    snprintf(args, sizeof(args), "%s %s", flag, TEMP_GCDA_FILE);
    
    int exit_code = run_gcov_dump(args, NULL, 0, 0);
    
    if (exit_code == 0) {
        printf("  ✓ %s flag executed successfully\n", flag);
        return 1;
    } else {
        printf("  ✗ %s flag failed (exit code: %d)\n", flag, exit_code);
        return 0;
    }
}

/* Test flag combinations */
static int test_flag_combination(const char *flags, const char *description) {
    printf("Testing flag combination: %s (%s)...\n", flags, description);
    
    char args[256];
    snprintf(args, sizeof(args), "%s %s", flags, TEMP_GCDA_FILE);
    
    int exit_code = run_gcov_dump(args, NULL, 0, 0);
    
    if (exit_code == 0) {
        printf("  ✓ Combination %s executed successfully\n", flags);
        return 1;
    } else {
        printf("  ✗ Combination %s failed (exit code: %d)\n", flags, exit_code);
        return 0;
    }
}

int main(int argc, char *argv[]) {
    const char *gcov_dump_source = NULL;
    int tests_passed = 0;
    int total_tests = 0;
    
    /* Determine gcov-dump source path */
    if (argc > 1) {
        gcov_dump_source = argv[1];
    } else {
        /* Try common locations */
        const char *possible_paths[] = {
            "../gcc/gcov-dump.cc",
            "../../gcc/gcov-dump.cc",
            "gcov-dump.cc",
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
            fprintf(stderr, "Error: gcov-dump.cc not found.\n");
            fprintf(stderr, "Usage: %s <path-to-gcov-dump.cc>\n", argv[0]);
            fprintf(stderr, "Or place gcov-dump.cc in current directory.\n");
            return 1;
        }
    }
    
    printf("Using gcov-dump source: %s\n", gcov_dump_source);
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_instrumented_gcov_dump(gcov_dump_source)) {
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    printf("Creating minimal .gcda file: %s\n", TEMP_GCDA_FILE);
    if (!create_minimal_gcda(TEMP_GCDA_FILE)) {
        fprintf(stderr, "Failed to create minimal .gcda file\n");
        unlink(TEMP_GCOV_DUMP);
        return 1;
    }
    
    /* Step 3: Execute test sequence */
    
    /* Test -h flag */
    total_tests++;
    if (test_help_flag()) tests_passed++;
    
    /* Test -v flag */
    total_tests++;
    if (test_version_flag()) tests_passed++;
    
    /* Test flags that require a file */
    total_tests++;
    if (test_flag_with_file("-l", "dump contents")) tests_passed++;
    
    total_tests++;
    if (test_flag_with_file("-p", "dump positions")) tests_passed++;
    
    total_tests++;
    if (test_flag_with_file("-r", "dump raw")) tests_passed++;
    
    total_tests++;
    if (test_flag_with_file("-s", "dump stable")) tests_passed++;
    
    /* Test flag combinations */
    total_tests++;
    if (test_flag_combination("-l -p", "contents and positions")) tests_passed++;
    
    total_tests++;
    if (test_flag_combination("-p -l", "positions and contents (reversed)")) tests_passed++;
    
    total_tests++;
    if (test_flag_combination("-r -s", "raw and stable")) tests_passed++;
    
    total_tests++;
    if (test_flag_combination("-l -p -r -s", "all flags")) tests_passed++;
    
    /* Test invalid flag */
    total_tests++;
    if (test_invalid_flag()) tests_passed++;
    
    /* Step 4: Cleanup */
    printf("\nCleaning up temporary files...\n");
    unlink(TEMP_GCDA_FILE);
    unlink(TEMP_GCOV_DUMP);
    
    /* Also remove coverage data files generated by instrumented binary */
    char coverage_files[][64] = {
        TEMP_GCOV_DUMP ".gcda",
        TEMP_GCOV_DUMP ".gcno",
        "gcov-dump-instrumented.gcda",
        "gcov-dump-instrumented.gcno"
    };
    
    for (size_t i = 0; i < sizeof(coverage_files)/sizeof(coverage_files[0]); i++) {
        if (unlink(coverage_files[i]) == 0) {
            printf("Removed: %s\n", coverage_files[i]);
        }
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    printf("Success rate: %.1f%%\n", (tests_passed * 100.0) / total_tests);
    
    return tests_passed == total_tests ? 0 : 1;
}
