#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_GCDA_FILE "/tmp/minimal.gcda"
#define INSTRUMENTED_BINARY "/tmp/gcov-dump-instrumented"

/* Minimal GCOV data file structure:
   - Magic number: 0x67636461 ('gcda')
   - Version: 0x3430392a ('409*' for gcc 4.9 format)
   - Zero-length record: 0x00000000
*/
static const unsigned char minimal_gcda[] = {
    0x67, 0x63, 0x64, 0x61,  /* 'gcda' magic */
    0x34, 0x30, 0x39, 0x2a,  /* '409*' version */
    0x00, 0x00, 0x00, 0x00   /* zero-length record */
};

/* Execute command and return exit status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Capture stderr from command and check for expected string */
int check_stderr_contains(const char *cmd, const char *expected) {
    char buffer[1024];
    char full_cmd[2048];
    FILE *fp;
    
    /* Redirect stderr to stdout and capture */
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    fp = popen(full_cmd, "r");
    if (!fp) {
        perror("popen failed");
        return 0;
    }
    
    int found = 0;
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, expected) != NULL) {
            found = 1;
            break;
        }
    }
    
    pclose(fp);
    return found;
}

/* Create minimal valid .gcda file */
int create_minimal_gcda(void) {
    FILE *fp = fopen(TEMP_GCDA_FILE, "wb");
    if (!fp) {
        perror("Failed to create temporary .gcda file");
        return 0;
    }
    
    size_t written = fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    
    return (written == sizeof(minimal_gcda));
}

/* Find gcov-dump.cc in common locations */
const char *find_gcov_dump_source(void) {
    static const char *possible_paths[] = {
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "../../../gcc/gcov-dump.cc",
        "/usr/src/gcc/gcc/gcov-dump.cc",
        NULL
    };
    
    struct stat st;
    for (int i = 0; possible_paths[i] != NULL; i++) {
        if (stat(possible_paths[i], &st) == 0 && S_ISREG(st.st_mode)) {
            return possible_paths[i];
        }
    }
    
    return NULL;
}

int main(void) {
    const char *source_file;
    char cmd[1024];
    int all_tests_passed = 1;
    
    printf("=== Starting gcov-dump coverage tests ===\n");
    
    /* Step 1: Find and build instrumented gcov-dump */
    source_file = find_gcov_dump_source();
    if (!source_file) {
        fprintf(stderr, "Error: Could not find gcov-dump.cc source file\n");
        fprintf(stderr, "Please run this test from gcc build directory or specify path\n");
        return 1;
    }
    
    printf("Found source: %s\n", source_file);
    
    /* Build instrumented gcov-dump */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             INSTRUMENTED_BINARY, source_file);
    
    printf("Building instrumented gcov-dump...\n");
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    /* Verify the binary was created */
    if (access(INSTRUMENTED_BINARY, X_OK) != 0) {
        perror("Instrumented binary not created or not executable");
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    printf("\nCreating minimal .gcda file...\n");
    if (!create_minimal_gcda()) {
        fprintf(stderr, "Failed to create minimal .gcda file\n");
        unlink(INSTRUMENTED_BINARY);
        return 1;
    }
    
    /* Step 3: Execute test sequence */
    
    /* Test -h flag (help) */
    printf("\n--- Testing -h flag (help) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -h", INSTRUMENTED_BINARY);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "FAIL: -h flag test failed\n");
        all_tests_passed = 0;
    } else {
        printf("PASS: -h flag test succeeded\n");
    }
    
    /* Test -v flag (version) */
    printf("\n--- Testing -v flag (version) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -v", INSTRUMENTED_BINARY);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "FAIL: -v flag test failed\n");
        all_tests_passed = 0;
    } else {
        printf("PASS: -v flag test succeeded\n");
    }
    
    /* Test -l flag (dump contents) */
    printf("\n--- Testing -l flag (dump contents) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "FAIL: -l flag test failed\n");
        all_tests_passed = 0;
    } else {
        printf("PASS: -l flag test succeeded\n");
    }
    
    /* Test -p flag (dump positions) */
    printf("\n--- Testing -p flag (dump positions) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "FAIL: -p flag test failed\n");
        all_tests_passed = 0;
    } else {
        printf("PASS: -p flag test succeeded\n");
    }
    
    /* Test -r flag (dump raw) */
    printf("\n--- Testing -r flag (dump raw) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -r %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "FAIL: -r flag test failed\n");
        all_tests_passed = 0;
    } else {
        printf("PASS: -r flag test succeeded\n");
    }
    
    /* Test -s flag (dump stable) */
    printf("\n--- Testing -s flag (dump stable) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "FAIL: -s flag test failed\n");
        all_tests_passed = 0;
    } else {
        printf("PASS: -s flag test succeeded\n");
    }
    
    /* Test flag combinations */
    printf("\n--- Testing flag combinations ---\n");
    
    /* -l -p combination */
    snprintf(cmd, sizeof(cmd), "%s -l -p %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "FAIL: -l -p combination test failed\n");
        all_tests_passed = 0;
    } else {
        printf("PASS: -l -p combination succeeded\n");
    }
    
    /* -p -l combination (different order) */
    snprintf(cmd, sizeof(cmd), "%s -p -l %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "FAIL: -p -l combination test failed\n");
        all_tests_passed = 0;
    } else {
        printf("PASS: -p -l combination succeeded\n");
    }
    
    /* -r -s combination */
    snprintf(cmd, sizeof(cmd), "%s -r -s %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "FAIL: -r -s combination test failed\n");
        all_tests_passed = 0;
    } else {
        printf("PASS: -r -s combination succeeded\n");
    }
    
    /* Test invalid flag (-X) */
    printf("\n--- Testing invalid flag (-X) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -X %s", INSTRUMENTED_BINARY, TEMP_GCDA_FILE);
    if (!check_stderr_contains(cmd, "unknown flag `X'")) {
        fprintf(stderr, "FAIL: Invalid flag test - expected error message not found\n");
        all_tests_passed = 0;
    } else {
        printf("PASS: Invalid flag correctly rejected with expected error\n");
    }
    
    /* Test invalid flag without file argument */
    printf("\n--- Testing invalid flag without file (-Z) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -Z", INSTRUMENTED_BINARY);
    if (!check_stderr_contains(cmd, "unknown flag `Z'")) {
        fprintf(stderr, "FAIL: Invalid flag test (no file) - expected error message not found\n");
        all_tests_passed = 0;
    } else {
        printf("PASS: Invalid flag (no file) correctly rejected\n");
    }
    
    /* Step 4: Cleanup */
    printf("\n=== Cleaning up ===\n");
    unlink(TEMP_GCDA_FILE);
    unlink(INSTRUMENTED_BINARY);
    
    /* Also clean up coverage data files created by instrumented binary */
    char coverage_files[][64] = {
        "gcov-dump-instrumented.gcda",
        "gcov-dump-instrumented.gcno",
        "minimal.gcda.gcda",
        "minimal.gcda.gcno"
    };
    
    for (size_t i = 0; i < sizeof(coverage_files)/sizeof(coverage_files[0]); i++) {
        if (unlink(coverage_files[i]) == 0) {
            printf("Removed: %s\n", coverage_files[i]);
        }
    }
    
    printf("\n=== Test Summary ===\n");
    if (all_tests_passed) {
        printf("All tests PASSED\n");
        return 0;
    } else {
        printf("Some tests FAILED\n");
        return 1;
    }
}
