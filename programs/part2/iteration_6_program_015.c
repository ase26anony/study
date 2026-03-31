/**
 * test_gcov_dump_invalid_flags.c
 * 
 * Test program to cover the default case in gcov-dump.cc switch statement
 * that handles unknown single-character command-line flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_CMD_LEN 1024
#define MAX_OUTPUT_LEN 4096

/**
 * Creates a temporary helper C program, compiles it with coverage flags,
 * runs it to generate .gcda file, and returns the path to the .gcda file.
 */
static char* create_gcov_data(void) {
    char template[] = "/tmp/gcov_test_XXXXXX";
    char *dir = mkdtemp(template);
    if (!dir) {
        perror("mkdtemp failed");
        return NULL;
    }
    
    // Create helper.c source file
    char helper_c_path[256];
    snprintf(helper_c_path, sizeof(helper_c_path), "%s/helper.c", dir);
    
    FILE *fp = fopen(helper_c_path, "w");
    if (!fp) {
        perror("fopen helper.c failed");
        return NULL;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    printf(\"Generating GCOV data...\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    fclose(fp);
    
    // Compile with coverage flags
    char compile_cmd[512];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -o %s/helper %s/helper.c 2>&1",
             dir, dir);
    
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Compilation failed: %s\n", compile_cmd);
        return NULL;
    }
    
    // Run the helper to generate .gcda file
    char run_cmd[256];
    snprintf(run_cmd, sizeof(run_cmd), "%s/helper", dir);
    
    if (system(run_cmd) != 0) {
        fprintf(stderr, "Running helper failed\n");
        return NULL;
    }
    
    // Return the path to .gcda file
    char *gcda_path = malloc(256);
    snprintf(gcda_path, 256, "%s/helper.gcda", dir);
    
    // Verify the file exists
    struct stat st;
    if (stat(gcda_path, &st) != 0 || !S_ISREG(st.st_mode)) {
        fprintf(stderr, "GCOV data file not created: %s\n", gcda_path);
        free(gcda_path);
        return NULL;
    }
    
    return gcda_path;
}

/**
 * Executes a gcov-dump command and checks if the output contains "unknown flag".
 * Returns 1 if the error message is found (covering the default case), 0 otherwise.
 */
static int test_invalid_flag(const char *flag, const char *gcda_path) {
    char cmd[MAX_CMD_LEN];
    char output[MAX_OUTPUT_LEN];
    FILE *fp;
    int found = 0;
    
    // Build command with stderr redirected to stdout
    snprintf(cmd, sizeof(cmd), "gcov-dump %s \"%s\" 2>&1", flag, gcda_path);
    
    printf("Testing: %s\n", cmd);
    
    fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        return 0;
    }
    
    // Read the output
    while (fgets(output, sizeof(output), fp) != NULL) {
        // Check for the exact error message from the default case
        if (strstr(output, "unknown flag") != NULL) {
            printf("  ✓ Found expected error: %s", output);
            found = 1;
        }
    }
    
    int status = pclose(fp);
    
    // Check exit status - should be non-zero for invalid flag
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        printf("  ✓ Command exited with non-zero status: %d\n", WEXITSTATUS(status));
    } else {
        printf("  ✗ Unexpected exit status\n");
        found = 0;
    }
    
    return found;
}

/**
 * Tests a valid flag to ensure gcov-dump works correctly.
 */
static void test_valid_flag(const char *gcda_path) {
    char cmd[MAX_CMD_LEN];
    
    snprintf(cmd, sizeof(cmd), "gcov-dump -l \"%s\" 2>&1", gcda_path);
    printf("\nTesting valid flag to verify gcov-dump works: %s\n", cmd);
    
    int result = system(cmd);
    if (WIFEXITED(result) && WEXITSTATUS(result) == 0) {
        printf("  ✓ Valid flag works correctly\n");
    } else {
        printf("  ✗ Valid flag test failed\n");
    }
}

int main(void) {
    printf("=== GCOV-Dump Invalid Flag Coverage Test ===\n\n");
    
    // Step 1: Generate GCOV data file
    printf("1. Generating GCOV data file...\n");
    char *gcda_path = create_gcov_data();
    if (!gcda_path) {
        fprintf(stderr, "Failed to create GCOV data\n");
        return 1;
    }
    printf("   Created: %s\n\n", gcda_path);
    
    // Step 2: Test valid flag first (to show normal operation)
    test_valid_flag(gcda_path);
    
    // Step 3: Test various invalid single-character flags
    printf("\n2. Testing invalid single-character flags:\n");
    
    // Array of invalid flags to test
    const char *invalid_flags[] = {
        "-a",  // alphabetic, not in {h,v,l,p,r,s}
        "-z",  // another alphabetic
        "-x",  // another alphabetic
        "-?",  // non-alphabetic
        "-1",  // numeric
        "-@",  // symbol
        "-A",  // uppercase (should still trigger default case)
        "-",   // just a dash (might trigger different error)
        NULL
    };
    
    int tests_passed = 0;
    int total_tests = 0;
    
    for (int i = 0; invalid_flags[i] != NULL; i++) {
        total_tests++;
        if (test_invalid_flag(invalid_flags[i], gcda_path)) {
            tests_passed++;
        }
        printf("\n");
    }
    
    // Step 4: Cleanup
    printf("\n3. Cleaning up...\n");
    
    // Extract directory from gcda_path
    char *last_slash = strrchr(gcda_path, '/');
    if (last_slash) {
        *last_slash = '\0';  // Truncate at last slash
        char cleanup_cmd[256];
        snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", gcda_path);
        system(cleanup_cmd);
    }
    
    free(gcda_path);
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total invalid flag tests: %d\n", total_tests);
    printf("Tests that triggered 'unknown flag' error: %d\n", tests_passed);
    
    if (tests_passed > 0) {
        printf("\n✅ SUCCESS: Successfully covered the default case in gcov-dump switch statement!\n");
        return 0;
    } else {
        printf("\n❌ FAILURE: Did not trigger the 'unknown flag' error message\n");
        return 1;
    }
}
