/**
 * Test driver for gcov-dump.cc uncovered lines (111-130)
 * Builds instrumented gcov-dump, creates minimal coverage files,
 * and tests all command-line flag parsing logic.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_GCDA_FILE "/tmp/minimal_test.gcda"
#define TEMP_GCNO_FILE "/tmp/minimal_test.gcno"
#define INSTRUMENTED_BINARY "/tmp/gcov-dump-instrumented"

/* GCOV file format constants */
#define GCOV_DATA_MAGIC 0x67636461  /* "gcda" */
#define GCOV_NOTE_MAGIC 0x67636e6f  /* "gcno" */
#define GCOV_VERSION   0x3430372a   /* GCC 9.x format */

/**
 * Create a minimal valid .gcda file for testing
 */
int create_minimal_gcda(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create .gcda file");
        return 0;
    }
    
    /* Write GCOV header: magic, version, stamp */
    unsigned int magic = GCOV_DATA_MAGIC;
    unsigned int version = GCOV_VERSION;
    unsigned int stamp = 0x12345678;  /* arbitrary stamp */
    
    fwrite(&magic, sizeof(magic), 1, fp);
    fwrite(&version, sizeof(version), 1, fp);
    fwrite(&stamp, sizeof(stamp), 1, fp);
    
    /* Write a zero-length record (tag=0, length=0) to terminate file */
    unsigned int zero_tag = 0;
    unsigned int zero_length = 0;
    fwrite(&zero_tag, sizeof(zero_tag), 1, fp);
    fwrite(&zero_length, sizeof(zero_length), 1, fp);
    
    fclose(fp);
    return 1;
}

/**
 * Create a minimal valid .gcno file for testing
 */
int create_minimal_gcno(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create .gcno file");
        return 0;
    }
    
    /* Write GCOV header: magic, version, stamp */
    unsigned int magic = GCOV_NOTE_MAGIC;
    unsigned int version = GCOV_VERSION;
    unsigned int stamp = 0x12345678;  /* arbitrary stamp */
    
    fwrite(&magic, sizeof(magic), 1, fp);
    fwrite(&version, sizeof(version), 1, fp);
    fwrite(&stamp, sizeof(stamp), 1, fp);
    
    /* Write basic function info record */
    unsigned int tag = 0x01000000;  /* GCOV_TAG_FUNCTION */
    unsigned int length = 2;        /* 2 unsigned ints: ident, checksum */
    unsigned int ident = 1;
    unsigned int checksum = 0xabcdef;
    
    fwrite(&tag, sizeof(tag), 1, fp);
    fwrite(&length, sizeof(length), 1, fp);
    fwrite(&ident, sizeof(ident), 1, fp);
    fwrite(&checksum, sizeof(checksum), 1, fp);
    
    /* Write zero tag to terminate */
    unsigned int zero_tag = 0;
    unsigned int zero_length = 0;
    fwrite(&zero_tag, sizeof(zero_tag), 1, fp);
    fwrite(&zero_length, sizeof(zero_length), 1, fp);
    
    fclose(fp);
    return 1;
}

/**
 * Execute gcov-dump with given arguments and capture output
 */
int run_gcov_dump(const char *args, char *output, size_t output_size, 
                  int capture_stderr, int *exit_status) {
    char command[1024];
    FILE *fp;
    int status;
    
    snprintf(command, sizeof(command), "%s %s 2>&1", INSTRUMENTED_BINARY, args);
    
    fp = popen(command, "r");
    if (!fp) {
        perror("popen failed");
        return 0;
    }
    
    if (output && output_size > 0) {
        size_t bytes_read = fread(output, 1, output_size - 1, fp);
        output[bytes_read] = '\0';
    }
    
    status = pclose(fp);
    if (exit_status) {
        *exit_status = WEXITSTATUS(status);
    }
    
    return 1;
}

/**
 * Check if string contains substring (case-sensitive)
 */
int contains_string(const char *str, const char *substr) {
    return strstr(str, substr) != NULL;
}

/**
 * Build instrumented gcov-dump binary
 */
int build_instrumented_gcov_dump() {
    const char *source_files[] = {
        "gcov-dump.cc",
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "/usr/src/gcc/gcc/gcov-dump.cc",
        NULL
    };
    
    char source_path[1024];
    FILE *test;
    int i;
    
    /* Try to find gcov-dump.cc */
    for (i = 0; source_files[i]; i++) {
        test = fopen(source_files[i], "r");
        if (test) {
            fclose(test);
            strcpy(source_path, source_files[i]);
            break;
        }
    }
    
    if (!source_files[i]) {
        fprintf(stderr, "Could not find gcov-dump.cc\n");
        fprintf(stderr, "Please run this test from GCC build directory\n");
        return 0;
    }
    
    printf("Found gcov-dump.cc at: %s\n", source_path);
    
    /* Build command */
    char build_cmd[2048];
    snprintf(build_cmd, sizeof(build_cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov "
             "-o %s %s",
             INSTRUMENTED_BINARY, source_path);
    
    printf("Building instrumented gcov-dump: %s\n", build_cmd);
    
    int result = system(build_cmd);
    if (result != 0) {
        fprintf(stderr, "Failed to build gcov-dump\n");
        return 0;
    }
    
    /* Verify the binary exists */
    if (access(INSTRUMENTED_BINARY, X_OK) != 0) {
        fprintf(stderr, "Instrumented binary not created\n");
        return 0;
    }
    
    return 1;
}

int main() {
    char output[4096];
    int exit_status;
    int tests_passed = 0;
    int total_tests = 0;
    
    printf("=== Testing gcov-dump uncovered lines (111-130) ===\n\n");
    
    /* Step 1: Build instrumented gcov-dump */
    printf("1. Building instrumented gcov-dump...\n");
    if (!build_instrumented_gcov_dump()) {
        fprintf(stderr, "Failed to build gcov-dump. Exiting.\n");
        return 1;
    }
    printf("   ✓ Built successfully\n\n");
    
    /* Step 2: Create minimal coverage files */
    printf("2. Creating minimal coverage files...\n");
    if (!create_minimal_gcda(TEMP_GCDA_FILE)) {
        cleanup();
        return 1;
    }
    printf("   ✓ Created %s\n", TEMP_GCDA_FILE);
    
    if (!create_minimal_gcno(TEMP_GCNO_FILE)) {
        cleanup();
        return 1;
    }
    printf("   ✓ Created %s\n\n", TEMP_GCNO_FILE);
    
    /* Step 3: Test flag parsing without file dependencies */
    printf("3. Testing flag parsing without file dependencies:\n");
    
    /* Test -h flag (help) */
    printf("   Testing -h flag... ");
    total_tests++;
    if (run_gcov_dump("-h", output, sizeof(output), 0, &exit_status)) {
        if (exit_status == 0 && contains_string(output, "Usage:")) {
            printf("✓ PASSED\n");
            tests_passed++;
        } else {
            printf("✗ FAILED (exit=%d)\n", exit_status);
        }
    }
    
    /* Test -v flag (version) */
    printf("   Testing -v flag... ");
    total_tests++;
    if (run_gcov_dump("-v", output, sizeof(output), 0, &exit_status)) {
        if (exit_status == 0 && contains_string(output, "gcov-dump")) {
            printf("✓ PASSED\n");
            tests_passed++;
        } else {
            printf("✗ FAILED\n");
        }
    }
    
    /* Test invalid flag -X */
    printf("   Testing invalid flag -X... ");
    total_tests++;
    if (run_gcov_dump("-X", output, sizeof(output), 1, &exit_status)) {
        if (contains_string(output, "unknown flag `X'")) {
            printf("✓ PASSED\n");
            tests_passed++;
        } else {
            printf("✗ FAILED (output: %s)\n", output);
        }
    }
    
    /* Step 4: Test flags requiring coverage files */
    printf("\n4. Testing flags requiring coverage files:\n");
    
    /* Test -l flag (dump contents) */
    printf("   Testing -l flag with .gcda... ");
    total_tests++;
    if (run_gcov_dump("-l " TEMP_GCDA_FILE, output, sizeof(output), 0, &exit_status)) {
        if (exit_status == 0) {
            printf("✓ PASSED\n");
            tests_passed++;
        } else {
            printf("✗ FAILED (exit=%d)\n", exit_status);
        }
    }
    
    printf("   Testing -l flag with .gcno... ");
    total_tests++;
    if (run_gcov_dump("-l " TEMP_GCNO_FILE, output, sizeof(output), 0, &exit_status)) {
        if (exit_status == 0) {
            printf("✓ PASSED\n");
            tests_passed++;
        } else {
            printf("✗ FAILED (exit=%d)\n", exit_status);
        }
    }
    
    /* Test -p flag (dump positions) */
    printf("   Testing -p flag... ");
    total_tests++;
    if (run_gcov_dump("-p " TEMP_GCDA_FILE, output, sizeof(output), 0, &exit_status)) {
        if (exit_status == 0) {
            printf("✓ PASSED\n");
            tests_passed++;
        } else {
            printf("✗ FAILED\n");
        }
    }
    
    /* Test -r flag (dump raw) */
    printf("   Testing -r flag... ");
    total_tests++;
    if (run_gcov_dump("-r " TEMP_GCDA_FILE, output, sizeof(output), 0, &exit_status)) {
        if (exit_status == 0) {
            printf("✓ PASSED\n");
            tests_passed++;
        } else {
            printf("✗ FAILED\n");
        }
    }
    
    /* Test -s flag (dump stable) */
    printf("   Testing -s flag... ");
    total_tests++;
    if (run_gcov_dump("-s " TEMP_GCDA_FILE, output, sizeof(output), 0, &exit_status)) {
        if (exit_status == 0) {
            printf("✓ PASSED\n");
            tests_passed++;
        } else {
            printf("✗ FAILED\n");
        }
    }
    
    /* Step 5: Test flag combinations */
    printf("\n5. Testing flag combinations:\n");
    
    printf("   Testing -l -p combination... ");
    total_tests++;
    if (run_gcov_dump("-l -p " TEMP_GCDA_FILE, output, sizeof(output), 0, &exit_status)) {
        if (exit_status == 0) {
            printf("✓ PASSED\n");
            tests_passed++;
        } else {
            printf("✗ FAILED\n");
        }
    }
    
    printf("   Testing -p -l (reversed order)... ");
    total_tests++;
    if (run_gcov_dump("-p -l " TEMP_GCDA_FILE, output, sizeof(output), 0, &exit_status)) {
        if (exit_status == 0) {
            printf("✓ PASSED\n");
            tests_passed++;
        } else {
            printf("✗ FAILED\n");
        }
    }
    
    printf("   Testing -r -s combination... ");
    total_tests++;
    if (run_gcov_dump("-r -s " TEMP_GCDA_FILE, output, sizeof(output), 0, &exit_status)) {
        if (exit_status == 0) {
            printf("✓ PASSED\n");
            tests_passed++;
        } else {
            printf("✗ FAILED\n");
        }
    }
    
    /* Step 6: Cleanup */
    printf("\n6. Cleaning up temporary files...\n");
    unlink(TEMP_GCDA_FILE);
    unlink(TEMP_GCNO_FILE);
    unlink(INSTRUMENTED_BINARY);
    printf("   ✓ Cleanup complete\n");
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    printf("Coverage: %.1f%%\n", (tests_passed * 100.0) / total_tests);
    
    if (tests_passed == total_tests) {
        printf("\n✓ All tests passed! The uncovered lines should now be covered.\n");
        return 0;
    } else {
        printf("\n✗ Some tests failed. Review output above.\n");
        return 1;
    }
}

/* Helper cleanup function */
void cleanup() {
    unlink(TEMP_GCDA_FILE);
    unlink(TEMP_GCNO_FILE);
    unlink(INSTRUMENTED_BINARY);
}
