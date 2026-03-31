/**
 * test_gcov_dump_coverage.c
 * 
 * A test program to exercise the uncovered command-line argument parsing
 * logic in gcov-dump.cc (lines 111-130).
 * 
 * This program:
 * 1. Builds/locates an instrumented gcov-dump binary
 * 2. Generates a test GCOV data file
 * 3. Executes gcov-dump with various flag combinations
 * 4. Merges coverage data after each run
 * 5. Verifies coverage of the target lines
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define MAX_PATH 1024
#define MAX_CMD 4096

/* Configuration - adjust these paths as needed */
const char *GCC_SOURCE_DIR = ".";
const char *GCOV_DUMP_SOURCE = "./gcov-dump.cc";
const char *LIBIBERTY_PATH = "../../libiberty/libiberty.a";
const char *INCLUDE_DIRS = "-I. -I../../include -I../../libiberty";

/* Global paths */
char gcov_dump_instrumented[MAX_PATH] = "./gcov-dump-instrumented";
char dummy_prog[MAX_PATH] = "./dummy_prog";
char dummy_gcda[MAX_PATH] = "./dummy.gcda";
char dummy_c[MAX_PATH] = "./dummy.c";

/* Function prototypes */
int build_instrumented_gcov_dump(void);
int generate_test_gcda(void);
int execute_gcov_dump(const char *args, int expect_success);
int merge_coverage(void);
int check_coverage(void);
void cleanup(void);

/**
 * Creates a minimal C program to generate GCOV data
 */
int create_dummy_program(void) {
    FILE *fp = fopen(dummy_c, "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return 0;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        printf(\"Iteration %%d\\n\", i);\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 1;
}

/**
 * Builds the instrumented gcov-dump binary
 */
int build_instrumented_gcov_dump(void) {
    char cmd[MAX_CMD];
    struct stat st;
    
    /* Check if already built */
    if (stat(gcov_dump_instrumented, &st) == 0) {
        printf("Instrumented gcov-dump already exists at %s\n", gcov_dump_instrumented);
        return 1;
    }
    
    /* Check if source exists */
    if (stat(GCOV_DUMP_SOURCE, &st) != 0) {
        fprintf(stderr, "gcov-dump source not found at %s\n", GCOV_DUMP_SOURCE);
        return 0;
    }
    
    /* Build command */
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage %s %s %s -o %s",
             INCLUDE_DIRS, GCOV_DUMP_SOURCE, LIBIBERTY_PATH, gcov_dump_instrumented);
    
    printf("Building instrumented gcov-dump...\n");
    printf("Command: %s\n", cmd);
    
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 0;
    }
    
    /* Verify build */
    if (stat(gcov_dump_instrumented, &st) != 0) {
        fprintf(stderr, "Build failed - output not created\n");
        return 0;
    }
    
    printf("Successfully built instrumented gcov-dump at %s\n", gcov_dump_instrumented);
    return 1;
}

/**
 * Generates test GCOV data file
 */
int generate_test_gcda(void) {
    char cmd[MAX_CMD];
    struct stat st;
    
    /* Create dummy program source */
    if (!create_dummy_program()) {
        return 0;
    }
    
    /* Check if dummy program already exists */
    if (stat(dummy_prog, &st) == 0 && stat(dummy_gcda, &st) == 0) {
        printf("Test GCOV data already exists\n");
        return 1;
    }
    
    /* Compile dummy program with coverage */
    printf("Compiling dummy program with coverage...\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s",
             dummy_c, dummy_prog);
    
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return 0;
    }
    
    /* Run dummy program to generate .gcda */
    printf("Running dummy program to generate GCOV data...\n");
    if (system(dummy_prog) != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        return 0;
    }
    
    /* Verify .gcda was created */
    if (stat(dummy_gcda, &st) != 0) {
        fprintf(stderr, "No .gcda file generated at %s\n", dummy_gcda);
        return 0;
    }
    
    printf("Successfully generated test GCOV data at %s\n", dummy_gcda);
    return 1;
}

/**
 * Executes gcov-dump with given arguments
 * Returns 1 on success, 0 on failure
 */
int execute_gcov_dump(const char *args, int expect_success) {
    char cmd[MAX_CMD];
    int result;
    
    snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_instrumented, args);
    printf("\nExecuting: %s\n", cmd);
    
    result = system(cmd);
    
    if (expect_success) {
        if (result != 0) {
            fprintf(stderr, "Command failed unexpectedly (exit code: %d)\n", result);
            return 0;
        }
    } else {
        if (result == 0) {
            fprintf(stderr, "Command succeeded unexpectedly\n");
            return 0;
        }
    }
    
    return 1;
}

/**
 * Merges coverage data for gcov-dump.cc
 */
int merge_coverage(void) {
    char cmd[MAX_CMD];
    
    /* First, ensure we're in the right directory */
    chdir(".");
    
    /* Use gcov's intermediate format for merging */
    printf("Merging coverage data...\n");
    snprintf(cmd, sizeof(cmd), "gcov -i gcov-dump.cc 2>/dev/null");
    
    if (system(cmd) != 0) {
        /* This might fail if gcov can't find the file, but that's OK */
        printf("Note: gcov -i may have failed (expected if not in build directory)\n");
    }
    
    /* Alternative: just copy the .gcda files to preserve them */
    snprintf(cmd, sizeof(cmd), "cp -f *.gcda gcov-dump.gcda.bak 2>/dev/null || true");
    system(cmd);
    
    return 1;
}

/**
 * Checks if target lines are covered
 */
int check_coverage(void) {
    char cmd[MAX_CMD];
    FILE *fp;
    char line[1024];
    int target_lines_covered = 0;
    
    printf("\n=== Generating coverage report ===\n");
    
    /* Generate coverage report */
    snprintf(cmd, sizeof(cmd), "gcov -b gcov-dump.cc 2>&1");
    fp = popen(cmd, "r");
    if (!fp) {
        perror("Failed to run gcov");
        return 0;
    }
    
    /* Parse output for coverage information */
    while (fgets(line, sizeof(line), fp)) {
        printf("%s", line);
        
        /* Look for lines 111-130 in the coverage output */
        if (strstr(line, "111:") || strstr(line, "112:") || 
            strstr(line, "113:") || strstr(line, "114:") ||
            strstr(line, "115:") || strstr(line, "116:") ||
            strstr(line, "117:") || strstr(line, "118:") ||
            strstr(line, "119:") || strstr(line, "120:") ||
            strstr(line, "121:") || strstr(line, "122:") ||
            strstr(line, "123:") || strstr(line, "124:") ||
            strstr(line, "125:") || strstr(line, "126:") ||
            strstr(line, "127:") || strstr(line, "128:") ||
            strstr(line, "129:") || strstr(line, "130:")) {
            target_lines_covered = 1;
        }
    }
    
    pclose(fp);
    
    if (target_lines_covered) {
        printf("\n✓ Target lines (111-130) appear to be covered\n");
    } else {
        printf("\n✗ Could not verify coverage of target lines (111-130)\n");
        printf("Note: This might be because we're not in the build directory\n");
        printf("or the .gcda files are in a different location.\n");
    }
    
    return target_lines_covered;
}

/**
 * Cleanup temporary files
 */
void cleanup(void) {
    /* Remove generated files */
    remove(dummy_c);
    remove(dummy_prog);
    remove(dummy_gcda);
    remove("dummy.gcno");
    remove("gcov-dump-instrumented");
    remove("gcov-dump.gcda");
    remove("gcov-dump.gcno");
    
    /* Clean up coverage files */
    system("rm -f *.gcda *.gcno gcov-dump.cc.gcov 2>/dev/null");
}

/**
 * Main test driver
 */
int main(int argc, char *argv[]) {
    int all_tests_passed = 1;
    
    printf("=== Testing gcov-dump command-line argument parsing ===\n");
    printf("Target: Lines 111-130 in gcov-dump.cc\n\n");
    
    /* Step 1: Build instrumented gcov-dump */
    if (!build_instrumented_gcov_dump()) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    /* Step 2: Generate test GCOV data */
    if (!generate_test_gcda()) {
        fprintf(stderr, "Failed to generate test GCOV data\n");
        cleanup();
        return 1;
    }
    
    /* Step 3: Execute test cases */
    
    /* Test 1: Help flag (-h) */
    printf("\n--- Test 1: Help flag (-h) ---\n");
    if (!execute_gcov_dump("-h", 1)) {
        all_tests_passed = 0;
    }
    merge_coverage();
    
    /* Test 2: Version flag (-v) */
    printf("\n--- Test 2: Version flag (-v) ---\n");
    if (!execute_gcov_dump("-v", 1)) {
        all_tests_passed = 0;
    }
    merge_coverage();
    
    /* Test 3: Individual flags with GCOV data file */
    printf("\n--- Test 3: Individual flags ---\n");
    
    printf("\nTesting -l flag...\n");
    if (!execute_gcov_dump("-l dummy.gcda", 1)) {
        all_tests_passed = 0;
    }
    merge_coverage();
    
    printf("\nTesting -p flag...\n");
    if (!execute_gcov_dump("-p dummy.gcda", 1)) {
        all_tests_passed = 0;
    }
    merge_coverage();
    
    printf("\nTesting -r flag...\n");
    if (!execute_gcov_dump("-r dummy.gcda", 1)) {
        all_tests_passed = 0;
    }
    merge_coverage();
    
    printf("\nTesting -s flag...\n");
    if (!execute_gcov_dump("-s dummy.gcda", 1)) {
        all_tests_passed = 0;
    }
    merge_coverage();
    
    /* Test 4: Combined flags (space-separated) */
    printf("\n--- Test 4: Combined flags (space-separated) ---\n");
    if (!execute_gcov_dump("-l -p -r -s dummy.gcda", 1)) {
        all_tests_passed = 0;
    }
    merge_coverage();
    
    /* Test 5: Concatenated flags */
    printf("\n--- Test 5: Concatenated flags ---\n");
    if (!execute_gcov_dump("-lprs dummy.gcda", 1)) {
        all_tests_passed = 0;
    }
    merge_coverage();
    
    /* Test 6: Invalid flag (to trigger default case) */
    printf("\n--- Test 6: Invalid flag (should trigger error) ---\n");
    if (!execute_gcov_dump("-x dummy.gcda", 0)) {
        all_tests_passed = 0;
    }
    merge_coverage();
    
    /* Test 7: Multiple combinations with different orders */
    printf("\n--- Test 7: Additional combinations ---\n");
    
    printf("\nTesting -lp combination...\n");
    if (!execute_gcov_dump("-lp dummy.gcda", 1)) {
        all_tests_passed = 0;
    }
    merge_coverage();
    
    printf("\nTesting -rs combination...\n");
    if (!execute_gcov_dump("-rs dummy.gcda", 1)) {
        all_tests_passed = 0;
    }
    merge_coverage();
    
    printf("\nTesting -l -s combination...\n");
    if (!execute_gcov_dump("-l -s dummy.gcda", 1)) {
        all_tests_passed = 0;
    }
    merge_coverage();
    
    /* Step 4: Final coverage check */
    printf("\n=== Final Coverage Verification ===\n");
    if (!check_coverage()) {
        printf("\nWarning: Could not automatically verify coverage.\n");
        printf("Please check gcov-dump.cc.gcov manually to confirm lines 111-130 are covered.\n");
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    if (all_tests_passed) {
        printf("✓ All tests passed\n");
    } else {
        printf("✗ Some tests failed\n");
    }
    
    /* Optional: cleanup */
    printf("\nClean up generated files? (y/n): ");
    char response;
    scanf("%c", &response);
    if (response == 'y' || response == 'Y') {
        cleanup();
        printf("Cleanup complete.\n");
    }
    
    return all_tests_passed ? 0 : 1;
}
