#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_GCDA_FILE "/tmp/minimal.gcda"
#define TEMP_GCOV_DUMP "/tmp/gcov-dump-instrumented"

/* Minimal GCOV data file structure */
static const unsigned char minimal_gcda[] = {
    /* Magic: 'gcda' */
    0x67, 0x63, 0x64, 0x61,
    /* Version: 0x3430372a (gcov 4.7 format) */
    0x2a, 0x37, 0x30, 0x34,
    /* Stamp: 0 */
    0x00, 0x00, 0x00, 0x00,
    /* Zero-length record terminator */
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
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
int check_stderr(const char *cmd, const char *expected) {
    char buffer[1024];
    FILE *fp;
    int found = 0;
    
    /* Create command to capture stderr */
    char full_cmd[2048];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    printf("Checking stderr for: %s\n", cmd);
    fp = popen(full_cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return 0;
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, expected) != NULL) {
            found = 1;
            break;
        }
    }
    
    pclose(fp);
    return found;
}

/* Create minimal .gcda file */
int create_minimal_gcda() {
    FILE *fp = fopen(TEMP_GCDA_FILE, "wb");
    if (!fp) {
        perror("Failed to create minimal .gcda file");
        return 0;
    }
    
    fwrite(minimal_gcda, 1, sizeof(minimal_gcda), fp);
    fclose(fp);
    printf("Created minimal .gcda file: %s\n", TEMP_GCDA_FILE);
    return 1;
}

/* Build instrumented gcov-dump */
int build_gcov_dump() {
    char cmd[1024];
    struct stat st;
    
    /* First check if source exists in common locations */
    const char *possible_paths[] = {
        "../../gcc/gcov-dump.cc",
        "../gcc/gcov-dump.cc",
        "gcc/gcov-dump.cc",
        "../../../gcc/gcov-dump.cc",
        NULL
    };
    
    const char *source_path = NULL;
    for (int i = 0; possible_paths[i]; i++) {
        if (stat(possible_paths[i], &st) == 0) {
            source_path = possible_paths[i];
            break;
        }
    }
    
    if (!source_path) {
        /* Try to find it using find command */
        printf("Searching for gcov-dump.cc...\n");
        FILE *find = popen("find . -name 'gcov-dump.cc' -type f 2>/dev/null | head -1", "r");
        if (find) {
            char path[256];
            if (fgets(path, sizeof(path), find)) {
                /* Remove newline */
                path[strcspn(path, "\n")] = 0;
                if (strlen(path) > 0) {
                    source_path = strdup(path);
                }
            }
            pclose(find);
        }
    }
    
    if (!source_path) {
        fprintf(stderr, "Could not find gcov-dump.cc\n");
        return 0;
    }
    
    printf("Found gcov-dump.cc at: %s\n", source_path);
    
    /* Build with coverage instrumentation */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             TEMP_GCOV_DUMP, source_path);
    
    printf("Building instrumented gcov-dump...\n");
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Failed to build gcov-dump\n");
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

int main() {
    char cmd[1024];
    int all_tests_passed = 1;
    
    printf("=== Starting gcov-dump coverage tests ===\n\n");
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_gcov_dump()) {
        fprintf(stderr, "Failed to build gcov-dump. Exiting.\n");
        return 1;
    }
    
    /* Step 2: Create minimal coverage file */
    if (!create_minimal_gcda()) {
        fprintf(stderr, "Failed to create minimal .gcda file. Exiting.\n");
        return 1;
    }
    
    printf("\n=== Testing flag parsing ===\n\n");
    
    /* Test 1: -h flag (help) */
    printf("Test 1: Testing -h flag (help)...\n");
    snprintf(cmd, sizeof(cmd), "%s -h", TEMP_GCOV_DUMP);
    if (execute_command(cmd) != 0) {
        printf("FAIL: -h flag test failed\n");
        all_tests_passed = 0;
    } else {
        printf("PASS: -h flag test succeeded\n");
    }
    
    /* Test 2: -v flag (version) */
    printf("\nTest 2: Testing -v flag (version)...\n");
    snprintf(cmd, sizeof(cmd), "%s -v", TEMP_GCOV_DUMP);
    if (execute_command(cmd) != 0) {
        printf("FAIL: -v flag test failed\n");
        all_tests_passed = 0;
    } else {
        printf("PASS: -v flag test succeeded\n");
    }
    
    /* Test 3: -l flag (dump contents) */
    printf("\nTest 3: Testing -l flag (dump contents)...\n");
    snprintf(cmd, sizeof(cmd), "%s -l %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    if (execute_command(cmd) != 0) {
        printf("FAIL: -l flag test failed\n");
        all_tests_passed = 0;
    } else {
        printf("PASS: -l flag test succeeded\n");
    }
    
    /* Test 4: -p flag (dump positions) */
    printf("\nTest 4: Testing -p flag (dump positions)...\n");
    snprintf(cmd, sizeof(cmd), "%s -p %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    if (execute_command(cmd) != 0) {
        printf("FAIL: -p flag test failed\n");
        all_tests_passed = 0;
    } else {
        printf("PASS: -p flag test succeeded\n");
    }
    
    /* Test 5: -r flag (dump raw) */
    printf("\nTest 5: Testing -r flag (dump raw)...\n");
    snprintf(cmd, sizeof(cmd), "%s -r %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    if (execute_command(cmd) != 0) {
        printf("FAIL: -r flag test failed\n");
        all_tests_passed = 0;
    } else {
        printf("PASS: -r flag test succeeded\n");
    }
    
    /* Test 6: -s flag (dump stable) */
    printf("\nTest 6: Testing -s flag (dump stable)...\n");
    snprintf(cmd, sizeof(cmd), "%s -s %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    if (execute_command(cmd) != 0) {
        printf("FAIL: -s flag test failed\n");
        all_tests_passed = 0;
    } else {
        printf("PASS: -s flag test succeeded\n");
    }
    
    /* Test 7: Combined flags -l -p */
    printf("\nTest 7: Testing combined flags -l -p...\n");
    snprintf(cmd, sizeof(cmd), "%s -l -p %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    if (execute_command(cmd) != 0) {
        printf("FAIL: -l -p combined test failed\n");
        all_tests_passed = 0;
    } else {
        printf("PASS: -l -p combined test succeeded\n");
    }
    
    /* Test 8: Combined flags -r -s (different order) */
    printf("\nTest 8: Testing combined flags -r -s...\n");
    snprintf(cmd, sizeof(cmd), "%s -r -s %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    if (execute_command(cmd) != 0) {
        printf("FAIL: -r -s combined test failed\n");
        all_tests_passed = 0;
    } else {
        printf("PASS: -r -s combined test succeeded\n");
    }
    
    /* Test 9: Combined flags -p -l (reverse order) */
    printf("\nTest 9: Testing combined flags -p -l (reverse order)...\n");
    snprintf(cmd, sizeof(cmd), "%s -p -l %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    if (execute_command(cmd) != 0) {
        printf("FAIL: -p -l combined test failed\n");
        all_tests_passed = 0;
    } else {
        printf("PASS: -p -l combined test succeeded\n");
    }
    
    /* Test 10: Invalid flag -X */
    printf("\nTest 10: Testing invalid flag -X...\n");
    snprintf(cmd, sizeof(cmd), "%s -X", TEMP_GCOV_DUMP);
    if (!check_stderr(cmd, "unknown flag `X'")) {
        printf("FAIL: Invalid flag test did not produce expected error\n");
        all_tests_passed = 0;
    } else {
        printf("PASS: Invalid flag test produced correct error message\n");
    }
    
    /* Test 11: Invalid flag with file argument */
    printf("\nTest 11: Testing invalid flag -Y with file...\n");
    snprintf(cmd, sizeof(cmd), "%s -Y %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    if (!check_stderr(cmd, "unknown flag `Y'")) {
        printf("FAIL: Invalid flag with file test failed\n");
        all_tests_passed = 0;
    } else {
        printf("PASS: Invalid flag with file test succeeded\n");
    }
    
    /* Test 12: Multiple invalid flags */
    printf("\nTest 12: Testing multiple invalid flags -X -Y...\n");
    snprintf(cmd, sizeof(cmd), "%s -X -Y", TEMP_GCOV_DUMP);
    if (!check_stderr(cmd, "unknown flag `X'")) {
        printf("FAIL: Multiple invalid flags test failed\n");
        all_tests_passed = 0;
    } else {
        printf("PASS: Multiple invalid flags test succeeded\n");
    }
    
    printf("\n=== Cleanup ===\n");
    
    /* Remove temporary files */
    remove(TEMP_GCDA_FILE);
    remove(TEMP_GCOV_DUMP);
    
    /* Also remove coverage data files created by instrumented binary */
    char gcda_pattern[256];
    snprintf(gcda_pattern, sizeof(gcda_pattern), "%s*.gcda", TEMP_GCOV_DUMP);
    char cleanup_cmd[512];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -f %s", gcda_pattern);
    system(cleanup_cmd);
    
    printf("Removed temporary files\n");
    
    if (all_tests_passed) {
        printf("\n=== All tests PASSED ===\n");
        return 0;
    } else {
        printf("\n=== Some tests FAILED ===\n");
        return 1;
    }
}
