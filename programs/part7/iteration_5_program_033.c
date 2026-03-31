#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_GCDA_FILE "/tmp/minimal.gcda"
#define INSTRUMENTED_BINARY "/tmp/gcov-dump-instrumented"

/* Minimal GCOV data file structure */
static const unsigned char minimal_gcda[] = {
    /* Magic: 'gcda' */
    0x67, 0x63, 0x64, 0x61,
    /* Version: 0x76312e2a (version string "*1v") */
    0x2a, 0x2e, 0x31, 0x76,
    /* Stamp: 0x00000000 */
    0x00, 0x00, 0x00, 0x00,
    /* Record type 0 (GCOV_TAG_FUNCTION) with length 2 */
    0x01, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00,
    /* Function info: ident=1, checksum=0, lineno_checksum=0 */
    0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    /* Record type 3 (GCOV_TAG_LINES) with length 0 */
    0x03, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    /* Record type 0 (GCOV_TAG_FUNCTION) with length 0 (terminator) */
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
    
    fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    printf("Created minimal .gcda file: %s (%zu bytes)\n", 
           filename, sizeof(minimal_gcda));
    return 1;
}

/* Compile gcov-dump with coverage instrumentation */
static int compile_gcov_dump(const char *source_path) {
    char cmd[1024];
    struct stat st;
    
    /* Check if source file exists */
    if (stat(source_path, &st) != 0) {
        fprintf(stderr, "Source file not found: %s\n", source_path);
        fprintf(stderr, "Trying common locations...\n");
        
        /* Try to find gcov-dump.cc in common GCC locations */
        const char *locations[] = {
            "../gcc/gcov-dump.cc",
            "../../gcc/gcov-dump.cc",
            "../../../gcc/gcov-dump.cc",
            "gcc/gcov-dump.cc",
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
            return 0;
        }
    }
    
    /* Compile with coverage instrumentation */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov "
             "-o %s %s 2>&1",
             INSTRUMENTED_BINARY, source_path);
    
    printf("Compiling gcov-dump: %s\n", cmd);
    
    FILE *compile_output = popen(cmd, "r");
    if (!compile_output) {
        perror("Failed to compile gcov-dump");
        return 0;
    }
    
    char buffer[1024];
    int has_errors = 0;
    while (fgets(buffer, sizeof(buffer), compile_output)) {
        if (strstr(buffer, "error:") || strstr(buffer, "Error:")) {
            has_errors = 1;
        }
        printf("  %s", buffer);
    }
    
    int status = pclose(compile_output);
    
    if (has_errors || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Compilation failed\n");
        return 0;
    }
    
    printf("Successfully compiled instrumented gcov-dump\n");
    return 1;
}

/* Run gcov-dump with given arguments and check exit status */
static int run_gcov_dump(const char *args, int expect_success, 
                         const char *expected_stderr) {
    char cmd[1024];
    char stderr_file[64];
    static int test_num = 0;
    
    test_num++;
    snprintf(stderr_file, sizeof(stderr_file), "/tmp/gcov_test_%d.stderr", test_num);
    
    /* Build command with stderr redirection */
    snprintf(cmd, sizeof(cmd), "%s %s 2>%s", 
             INSTRUMENTED_BINARY, args, stderr_file);
    
    printf("\n=== Test %d: gcov-dump %s ===\n", test_num, args);
    
    int status = system(cmd);
    int exit_status = WEXITSTATUS(status);
    
    /* Check stderr for expected content */
    if (expected_stderr) {
        FILE *fp = fopen(stderr_file, "r");
        if (fp) {
            char buffer[1024];
            int found = 0;
            while (fgets(buffer, sizeof(buffer), fp)) {
                if (strstr(buffer, expected_stderr)) {
                    found = 1;
                    printf("  ✓ Found expected stderr: %s", buffer);
                    break;
                }
            }
            fclose(fp);
            
            if (!found) {
                printf("  ✗ Expected stderr not found: %s\n", expected_stderr);
                /* Show what was actually in stderr */
                fp = fopen(stderr_file, "r");
                if (fp) {
                    printf("  Actual stderr:\n");
                    while (fgets(buffer, sizeof(buffer), fp)) {
                        printf("    %s", buffer);
                    }
                    fclose(fp);
                }
                return 0;
            }
        }
        unlink(stderr_file);
    }
    
    /* Check exit status */
    int success = (expect_success && exit_status == 0) || 
                  (!expect_success && exit_status != 0);
    
    if (success) {
        printf("  ✓ Exit status: %d (expected %s)\n", 
               exit_status, expect_success ? "0" : "non-zero");
    } else {
        printf("  ✗ Exit status: %d (expected %s)\n", 
               exit_status, expect_success ? "0" : "non-zero");
    }
    
    return success;
}

/* Test version output */
static int test_version_flag() {
    char cmd[1024];
    char output_file[64];
    static int version_test_num = 0;
    
    version_test_num++;
    snprintf(output_file, sizeof(output_file), "/tmp/gcov_version_%d.txt", version_test_num);
    
    snprintf(cmd, sizeof(cmd), "%s -v >%s 2>&1", 
             INSTRUMENTED_BINARY, output_file);
    
    printf("\n=== Testing -v flag ===\n");
    
    int status = system(cmd);
    
    /* Check if version info was printed */
    FILE *fp = fopen(output_file, "r");
    if (fp) {
        char buffer[1024];
        int has_version = 0;
        while (fgets(buffer, sizeof(buffer), fp)) {
            if (strstr(buffer, "gcov-dump") || 
                strstr(buffer, "version") ||
                strstr(buffer, "GCC")) {
                has_version = 1;
                printf("  ✓ Version output: %s", buffer);
            }
        }
        fclose(fp);
        
        if (!has_version) {
            printf("  ✗ No version information found\n");
            return 0;
        }
    }
    
    unlink(output_file);
    return WEXITSTATUS(status) == 0;
}

int main(int argc, char *argv[]) {
    const char *gcov_dump_source = "gcov-dump.cc";
    
    if (argc > 1) {
        gcov_dump_source = argv[1];
    }
    
    printf("=== GCOV-Dump Coverage Test ===\n");
    printf("Source file: %s\n", gcov_dump_source);
    
    /* Step 1: Compile instrumented gcov-dump */
    if (!compile_gcov_dump(gcov_dump_source)) {
        fprintf(stderr, "Failed to compile gcov-dump\n");
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    if (!create_minimal_gcda(TEMP_GCDA_FILE)) {
        fprintf(stderr, "Failed to create minimal .gcda file\n");
        unlink(INSTRUMENTED_BINARY);
        return 1;
    }
    
    int all_tests_passed = 1;
    
    /* Step 3: Execute test sequence */
    
    /* Test -h flag (help) - no file needed */
    printf("\n=== Testing -h flag (help) ===\n");
    if (!run_gcov_dump("-h", 1, NULL)) {
        all_tests_passed = 0;
    }
    
    /* Test -v flag (version) - no file needed */
    if (!test_version_flag()) {
        all_tests_passed = 0;
    }
    
    /* Test -l flag (dump contents) with minimal file */
    if (!run_gcov_dump("-l " TEMP_GCDA_FILE, 1, NULL)) {
        all_tests_passed = 0;
    }
    
    /* Test -p flag (dump positions) with minimal file */
    if (!run_gcov_dump("-p " TEMP_GCDA_FILE, 1, NULL)) {
        all_tests_passed = 0;
    }
    
    /* Test -r flag (dump raw) with minimal file */
    if (!run_gcov_dump("-r " TEMP_GCDA_FILE, 1, NULL)) {
        all_tests_passed = 0;
    }
    
    /* Test -s flag (dump stable) with minimal file */
    if (!run_gcov_dump("-s " TEMP_GCDA_FILE, 1, NULL)) {
        all_tests_passed = 0;
    }
    
    /* Test flag combinations */
    printf("\n=== Testing flag combinations ===\n");
    
    /* Test -l -p combination */
    if (!run_gcov_dump("-l -p " TEMP_GCDA_FILE, 1, NULL)) {
        all_tests_passed = 0;
    }
    
    /* Test -p -l combination (different order) */
    if (!run_gcov_dump("-p -l " TEMP_GCDA_FILE, 1, NULL)) {
        all_tests_passed = 0;
    }
    
    /* Test -r -s combination */
    if (!run_gcov_dump("-r -s " TEMP_GCDA_FILE, 1, NULL)) {
        all_tests_passed = 0;
    }
    
    /* Test -s -r combination (different order) */
    if (!run_gcov_dump("-s -r " TEMP_GCDA_FILE, 1, NULL)) {
        all_tests_passed = 0;
    }
    
    /* Test multiple flags together */
    if (!run_gcov_dump("-l -p -r " TEMP_GCDA_FILE, 1, NULL)) {
        all_tests_passed = 0;
    }
    
    /* Test invalid flag -X (should trigger "unknown flag" error) */
    printf("\n=== Testing invalid flag -X ===\n");
    if (!run_gcov_dump("-X " TEMP_GCDA_FILE, 0, "unknown flag `X'")) {
        all_tests_passed = 0;
    }
    
    /* Test another invalid flag -Z */
    printf("\n=== Testing invalid flag -Z ===\n");
    if (!run_gcov_dump("-Z " TEMP_GCDA_FILE, 0, "unknown flag `Z'")) {
        all_tests_passed = 0;
    }
    
    /* Test invalid flag without file argument */
    printf("\n=== Testing invalid flag without file ===\n");
    if (!run_gcov_dump("-Y", 0, "unknown flag `Y'")) {
        all_tests_passed = 0;
    }
    
    /* Step 4: Cleanup */
    printf("\n=== Cleanup ===\n");
    
    if (unlink(TEMP_GCDA_FILE) == 0) {
        printf("Removed temporary file: %s\n", TEMP_GCDA_FILE);
    }
    
    if (unlink(INSTRUMENTED_BINARY) == 0) {
        printf("Removed instrumented binary: %s\n", INSTRUMENTED_BINARY);
    }
    
    /* Clean up any .gcno/.gcda files created during compilation */
    system("rm -f gcov-dump.gcno gcov-dump.gcda /tmp/gcov-dump-instrumented.gcno /tmp/gcov-dump-instrumented.gcda");
    
    printf("\n=== Test Summary ===\n");
    if (all_tests_passed) {
        printf("✓ All tests passed!\n");
        printf("\nThe following gcov-dump.cc lines should now be covered:\n");
        printf("  Lines 111-130: switch cases for -h, -v, -l, -p, -r, -s, and default case\n");
        printf("  Specifically:\n");
        printf("    - case 'h': print_usage()\n");
        printf("    - case 'v': print_version()\n");
        printf("    - case 'l': flag_dump_contents = 1\n");
        printf("    - case 'p': flag_dump_positions = 1\n");
        printf("    - case 'r': flag_dump_raw = 1\n");
        printf("    - case 's': flag_dump_stable = 1\n");
        printf("    - default: fprintf(stderr, \"unknown flag\")\n");
        return 0;
    } else {
        printf("✗ Some tests failed\n");
        return 1;
    }
}
