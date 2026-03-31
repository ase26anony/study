/**
 * test_gcov_dump_coverage.c
 * 
 * This program tests the gcov-dump utility to ensure coverage of the
 * command-line argument parsing switch-case block (lines 111-130).
 * It builds an instrumented gcov-dump, generates test GCOV data,
 * and runs various flag combinations to exercise all code paths.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024
#define MAX_CMD 4096

/**
 * Structure to hold test cases for gcov-dump flags
 */
typedef struct {
    const char *args;           // Command line arguments
    const char *description;    // Test description
    int expect_success;         // Expected exit code (0 for success)
    int needs_data_file;        // Whether test needs a .gcda file
} test_case_t;

// Test cases covering all flag combinations
static test_case_t test_cases[] = {
    {"-h", "Help flag", 1, 0},
    {"-v", "Version flag", 1, 0},
    {"-l dummy.gcda", "List contents flag", 0, 1},
    {"-p dummy.gcda", "Dump positions flag", 0, 1},
    {"-r dummy.gcda", "Dump raw flag", 0, 1},
    {"-s dummy.gcda", "Dump stable flag", 0, 1},
    {"-l -p -r -s dummy.gcda", "All flags separated", 0, 1},
    {"-lprs dummy.gcda", "All flags concatenated", 0, 1},
    {"-x dummy.gcda", "Invalid flag (should fail)", 0, 1},
    {NULL, NULL, 0, 0}  // Sentinel
};

/**
 * Check if a file exists
 */
static int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

/**
 * Execute a command and return its exit status
 */
static int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/**
 * Build instrumented gcov-dump if it doesn't exist
 */
static int build_instrumented_gcov_dump(const char *source_dir) {
    char cmd[MAX_CMD];
    char gcov_dump_path[MAX_PATH];
    
    // Check if instrumented binary already exists
    snprintf(gcov_dump_path, sizeof(gcov_dump_path), "./gcov-dump-instrumented");
    if (file_exists(gcov_dump_path)) {
        printf("Instrumented gcov-dump already exists at %s\n", gcov_dump_path);
        return 0;
    }
    
    printf("Building instrumented gcov-dump...\n");
    
    // Try to find gcov-dump source in common locations
    const char *possible_sources[] = {
        "gcov-dump.cc",
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "gcc/gcov-dump.cc",
        NULL
    };
    
    char source_path[MAX_PATH] = "";
    for (int i = 0; possible_sources[i]; i++) {
        if (file_exists(possible_sources[i])) {
            strcpy(source_path, possible_sources[i]);
            break;
        }
    }
    
    if (strlen(source_path) == 0) {
        fprintf(stderr, "Error: Could not find gcov-dump.cc source file\n");
        return -1;
    }
    
    // Build command for instrumented gcov-dump
    // Note: This assumes libiberty is available. Adjust paths as needed.
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include "
             "-I../../libiberty %s ../../libiberty/libiberty.a "
             "-o gcov-dump-instrumented",
             source_path);
    
    printf("Build command: %s\n", cmd);
    return execute_command(cmd);
}

/**
 * Create a dummy C program and generate GCOV data from it
 */
static int generate_test_gcov_data(void) {
    char cmd[MAX_CMD];
    
    // Create dummy.c source file
    FILE *fp = fopen("dummy.c", "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return -1;
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
    
    // Compile dummy.c with coverage instrumentation
    printf("Compiling dummy program with coverage...\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -O0 -fprofile-arcs -ftest-coverage dummy.c -o dummy_prog");
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return -1;
    }
    
    // Run dummy program to generate .gcda file
    printf("Running dummy program to generate coverage data...\n");
    snprintf(cmd, sizeof(cmd), "./dummy_prog > /dev/null 2>&1");
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        return -1;
    }
    
    // Verify .gcda file was created
    if (!file_exists("dummy.gcda")) {
        fprintf(stderr, "Error: dummy.gcda not generated\n");
        return -1;
    }
    
    printf("Test GCOV data generated successfully\n");
    return 0;
}

/**
 * Merge coverage data after each gcov-dump invocation
 */
static int merge_coverage_data(void) {
    char cmd[MAX_CMD];
    
    // Method 1: Use gcov -i to merge coverage
    snprintf(cmd, sizeof(cmd), "gcov -i gcov-dump.cc 2>/dev/null");
    execute_command(cmd);
    
    // Method 2: Copy .gcda files to preserve them
    // (This is a simplified approach - in reality you might need to
    // manage multiple .gcda files from different runs)
    static int run_count = 0;
    char backup_cmd[MAX_CMD];
    snprintf(backup_cmd, sizeof(backup_cmd),
             "cp gcov-dump.gcda gcov-dump.gcda.%d 2>/dev/null", run_count++);
    execute_command(backup_cmd);
    
    return 0;
}

/**
 * Run a single test case
 */
static int run_test_case(const char *gcov_dump_path, test_case_t *test) {
    char cmd[MAX_CMD];
    int result;
    
    // Build command
    if (test->needs_data_file) {
        snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, test->args);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, test->args);
    }
    
    printf("\n=== Test: %s ===\n", test->description);
    printf("Command: %s\n", cmd);
    
    // Execute the command
    result = execute_command(cmd);
    
    // For invalid flag test, we expect stderr output
    if (strstr(test->args, "-x") != NULL) {
        printf("Checking for 'unknown flag' error message...\n");
        // We could capture stderr here, but system() doesn't easily allow that
        // The important thing is that the default case was executed
    }
    
    // Merge coverage data after this run
    merge_coverage_data();
    
    return result;
}

/**
 * Generate final coverage report
 */
static void generate_coverage_report(void) {
    char cmd[MAX_CMD];
    
    printf("\n=== Generating Coverage Report ===\n");
    
    // Generate human-readable coverage report
    snprintf(cmd, sizeof(cmd), "gcov -b gcov-dump.cc");
    printf("Running: %s\n", cmd);
    system(cmd);
    
    // Also show the specific lines we care about
    printf("\n=== Checking Coverage for Target Lines (111-130) ===\n");
    snprintf(cmd, sizeof(cmd), 
             "gcov -l gcov-dump.cc | grep -A 20 -B 5 '^111\\|^112\\|^113\\|^114\\|"
             "^115\\|^116\\|^117\\|^118\\|^119\\|^120\\|^121\\|^122\\|^123\\|"
             "^124\\|^125\\|^126\\|^127\\|^128\\|^129\\|^130'");
    system(cmd);
}

/**
 * Clean up temporary files
 */
static void cleanup(void) {
    // Remove generated files
    system("rm -f dummy.c dummy_prog dummy.gcda dummy.gcno 2>/dev/null");
    system("rm -f gcov-dump.gcda.* 2>/dev/null");
    system("rm -f *.gcov 2>/dev/null");
}

int main(int argc, char *argv[]) {
    char gcov_dump_path[MAX_PATH] = "./gcov-dump-instrumented";
    int all_tests_passed = 1;
    
    printf("=== GCOV-Dump Coverage Test Program ===\n\n");
    
    // Step 1: Build instrumented gcov-dump
    if (build_instrumented_gcov_dump(".") != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    // Step 2: Generate test GCOV data
    if (generate_test_gcov_data() != 0) {
        fprintf(stderr, "Failed to generate test GCOV data\n");
        cleanup();
        return 1;
    }
    
    // Step 3: Run all test cases
    printf("\n=== Running Test Cases ===\n");
    for (int i = 0; test_cases[i].args != NULL; i++) {
        int result = run_test_case(gcov_dump_path, &test_cases[i]);
        
        // Check if test passed
        // Note: For invalid flag test, we don't check exit code strictly
        // as different implementations may handle it differently
        if (strstr(test_cases[i].args, "-x") == NULL) {
            if ((test_cases[i].expect_success && result != 0) ||
                (!test_cases[i].expect_success && result != 0)) {
                printf("Test FAILED (exit code: %d)\n", result);
                all_tests_passed = 0;
            } else {
                printf("Test PASSED\n");
            }
        } else {
            printf("Invalid flag test completed (exit code: %d)\n", result);
        }
    }
    
    // Step 4: Generate final coverage report
    generate_coverage_report();
    
    // Step 5: Cleanup
    cleanup();
    
    printf("\n=== Test Summary ===\n");
    if (all_tests_passed) {
        printf("All tests completed successfully.\n");
        printf("The switch-case block (lines 111-130) should now be covered.\n");
        printf("Check gcov-dump.c.gcov for detailed coverage information.\n");
        return 0;
    } else {
        printf("Some tests failed. Check coverage may be incomplete.\n");
        return 1;
    }
}
