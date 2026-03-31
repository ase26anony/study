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
   - Version: 0x3430392a ('409*')
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
    
    return written == sizeof(minimal_gcda);
}

/* Execute command and capture output */
static int execute_and_capture(const char *cmd, char *output, size_t output_size, 
                               int capture_stderr) {
    char full_cmd[1024];
    if (capture_stderr) {
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    } else {
        snprintf(full_cmd, sizeof(full_cmd), "%s", cmd);
    }
    
    FILE *fp = popen(full_cmd, "r");
    if (!fp) {
        return -1;
    }
    
    output[0] = '\0';
    size_t total_read = 0;
    while (fgets(output + total_read, output_size - total_read, fp) != NULL) {
        total_read = strlen(output);
        if (total_read >= output_size - 1) break;
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
}

/* Check if string contains substring */
static int contains_string(const char *str, const char *substr) {
    return strstr(str, substr) != NULL;
}

/* Build instrumented gcov-dump */
static int build_gcov_dump(const char *source_path) {
    char cmd[2048];
    
    /* First check if source file exists */
    struct stat st;
    if (stat(source_path, &st) != 0) {
        fprintf(stderr, "Source file not found: %s\n", source_path);
        return 0;
    }
    
    /* Build with coverage instrumentation */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -lgcov -o %s %s",
             TEMP_GCOV_DUMP, source_path);
    
    printf("Building instrumented gcov-dump...\n");
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Failed to build gcov-dump. Command: %s\n", cmd);
        return 0;
    }
    
    /* Verify the binary was created */
    if (stat(TEMP_GCOV_DUMP, &st) != 0) {
        fprintf(stderr, "Binary not created: %s\n", TEMP_GCOV_DUMP);
        return 0;
    }
    
    printf("Successfully built instrumented gcov-dump at %s\n", TEMP_GCOV_DUMP);
    return 1;
}

int main(int argc, char *argv[]) {
    char output[4096];
    int exit_code;
    int all_tests_passed = 1;
    
    /* Determine gcov-dump source path */
    const char *gcov_dump_source = "gcov-dump.cc";
    if (argc > 1) {
        gcov_dump_source = argv[1];
    }
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_gcov_dump(gcov_dump_source)) {
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
    printf("Created minimal .gcda file at %s\n", TEMP_GCDA_FILE);
    
    /* Step 3: Execute test sequence */
    printf("\n=== Starting test sequence ===\n");
    
    /* Test 1: -h flag (help) */
    printf("\n1. Testing -h flag (help)...\n");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s -h", TEMP_GCOV_DUMP);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("  ✓ -h flag exited successfully (code %d)\n", exit_code);
    } else {
        printf("  ✗ -h flag failed (code %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test 2: -v flag (version) */
    printf("\n2. Testing -v flag (version)...\n");
    snprintf(cmd, sizeof(cmd), "%s -v", TEMP_GCOV_DUMP);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_code == 0 && strlen(output) > 0) {
        printf("  ✓ -v flag printed version info:\n    %s", output);
    } else {
        printf("  ✗ -v flag failed or produced no output\n");
        all_tests_passed = 0;
    }
    
    /* Test 3: -l flag (dump contents) */
    printf("\n3. Testing -l flag (dump contents)...\n");
    snprintf(cmd, sizeof(cmd), "%s -l %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("  ✓ -l flag executed successfully\n");
    } else {
        printf("  ✗ -l flag failed (code %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test 4: -p flag (dump positions) */
    printf("\n4. Testing -p flag (dump positions)...\n");
    snprintf(cmd, sizeof(cmd), "%s -p %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("  ✓ -p flag executed successfully\n");
    } else {
        printf("  ✗ -p flag failed (code %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test 5: -r flag (dump raw) */
    printf("\n5. Testing -r flag (dump raw)...\n");
    snprintf(cmd, sizeof(cmd), "%s -r %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("  ✓ -r flag executed successfully\n");
    } else {
        printf("  ✗ -r flag failed (code %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test 6: -s flag (dump stable) */
    printf("\n6. Testing -s flag (dump stable)...\n");
    snprintf(cmd, sizeof(cmd), "%s -s %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("  ✓ -s flag executed successfully\n");
    } else {
        printf("  ✗ -s flag failed (code %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test 7: Combined flags -l -p */
    printf("\n7. Testing combined flags -l -p...\n");
    snprintf(cmd, sizeof(cmd), "%s -l -p %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("  ✓ -l -p combination executed successfully\n");
    } else {
        printf("  ✗ -l -p combination failed (code %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test 8: Combined flags -r -s (different order) */
    printf("\n8. Testing combined flags -r -s...\n");
    snprintf(cmd, sizeof(cmd), "%s -r -s %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("  ✓ -r -s combination executed successfully\n");
    } else {
        printf("  ✗ -r -s combination failed (code %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test 9: Combined flags -p -l (reverse order) */
    printf("\n9. Testing combined flags -p -l (reverse order)...\n");
    snprintf(cmd, sizeof(cmd), "%s -p -l %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 0);
    if (exit_code == 0) {
        printf("  ✓ -p -l combination executed successfully\n");
    } else {
        printf("  ✗ -p -l combination failed (code %d)\n", exit_code);
        all_tests_passed = 0;
    }
    
    /* Test 10: Invalid flag -X */
    printf("\n10. Testing invalid flag -X...\n");
    snprintf(cmd, sizeof(cmd), "%s -X %s", TEMP_GCOV_DUMP, TEMP_GCDA_FILE);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1); /* Capture stderr */
    
    /* Check for the exact error message from the uncovered lines */
    if (contains_string(output, "unknown flag `X'")) {
        printf("  ✓ Invalid flag correctly detected: \"unknown flag `X'\"\n");
        printf("    Exit code: %d (expected non-zero)\n", exit_code);
    } else {
        printf("  ✗ Invalid flag test failed. Output:\n%s\n", output);
        all_tests_passed = 0;
    }
    
    /* Test 11: Invalid flag without file argument */
    printf("\n11. Testing invalid flag -Y without file...\n");
    snprintf(cmd, sizeof(cmd), "%s -Y", TEMP_GCOV_DUMP);
    exit_code = execute_and_capture(cmd, output, sizeof(output), 1);
    
    if (contains_string(output, "unknown flag `Y'")) {
        printf("  ✓ Invalid flag without file correctly detected\n");
    } else {
        printf("  ✗ Invalid flag without file test failed\n");
        all_tests_passed = 0;
    }
    
    /* Step 4: Cleanup */
    printf("\n=== Cleaning up ===\n");
    
    /* Remove temporary files */
    if (unlink(TEMP_GCDA_FILE) == 0) {
        printf("Removed %s\n", TEMP_GCDA_FILE);
    }
    
    if (unlink(TEMP_GCOV_DUMP) == 0) {
        printf("Removed %s\n", TEMP_GCOV_DUMP);
    }
    
    /* Also remove coverage files generated by the instrumented binary */
    char coverage_file[1024];
    snprintf(coverage_file, sizeof(coverage_file), "%s.gcda", TEMP_GCOV_DUMP);
    unlink(coverage_file);
    
    snprintf(coverage_file, sizeof(coverage_file), "%s.gcno", TEMP_GCOV_DUMP);
    unlink(coverage_file);
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    if (all_tests_passed) {
        printf("All tests passed! The uncovered lines in gcov-dump.cc should now be covered.\n");
        printf("\nTo verify coverage:\n");
        printf("1. Run: gcov %s\n", TEMP_GCOV_DUMP);
        printf("2. Check gcov-dump.cc.gcov for coverage of lines 111-130\n");
        return 0;
    } else {
        printf("Some tests failed. Check the output above.\n");
        return 1;
    }
}
