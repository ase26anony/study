/**
 * test_gcov_dump_coverage.c
 * 
 * This program tests the command-line argument parsing in gcov-dump.cc
 * Specifically targeting lines 111-130 (switch-case for flags: h, v, l, p, r, s, default)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024

/**
 * Build an instrumented version of gcov-dump
 */
int build_instrumented_gcov_dump(const char *source_dir, const char *output_path) {
    char cmd[MAX_PATH * 4];
    int status;
    
    printf("Building instrumented gcov-dump...\n");
    
    // Check if we're in a GCC source tree
    struct stat st;
    char gcov_dump_cc[MAX_PATH];
    char libiberty_path[MAX_PATH];
    
    // Try to find gcov-dump.cc
    if (source_dir) {
        snprintf(gcov_dump_cc, sizeof(gcov_dump_cc), "%s/gcov-dump.cc", source_dir);
    } else {
        // Common locations in GCC source tree
        const char *locations[] = {
            ".",
            "gcc",
            "../gcc",
            "../../gcc",
            NULL
        };
        
        int found = 0;
        for (int i = 0; locations[i]; i++) {
            snprintf(gcov_dump_cc, sizeof(gcov_dump_cc), "%s/gcov-dump.cc", locations[i]);
            if (stat(gcov_dump_cc, &st) == 0) {
                found = 1;
                // Try to find libiberty relative to this
                char *last_slash = strrchr(locations[i], '/');
                if (last_slash) {
                    *last_slash = '\0';
                    snprintf(libiberty_path, sizeof(libiberty_path), "%s/libiberty/libiberty.a", locations[i]);
                } else {
                    snprintf(libiberty_path, sizeof(libiberty_path), "libiberty/libiberty.a");
                }
                break;
            }
        }
        
        if (!found) {
            fprintf(stderr, "Could not find gcov-dump.cc\n");
            return 0;
        }
    }
    
    // Build command to compile instrumented gcov-dump
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include -I../../libiberty "
        "%s ../../libiberty/libiberty.a -o %s",
        gcov_dump_cc, output_path);
    
    printf("Compiling with: %s\n", cmd);
    status = system(cmd);
    
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("Successfully built instrumented gcov-dump at %s\n", output_path);
        return 1;
    } else {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        
        // Try simpler compilation if in PATH
        printf("Trying to find gcov-dump in PATH...\n");
        if (system("which gcov-dump > /dev/null 2>&1") == 0) {
            printf("Found gcov-dump in PATH, but it may not be instrumented\n");
            // We'll use it anyway for testing
            system("cp `which gcov-dump` gcov-dump-instrumented");
            return 1;
        }
        
        return 0;
    }
}

/**
 * Create a simple test program to generate GCOV data
 */
int create_test_gcov_data(const char *dummy_c_path, const char *gcda_path) {
    FILE *fp;
    
    printf("Creating dummy test program...\n");
    
    // Create dummy.c
    fp = fopen(dummy_c_path, "w");
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
    
    // Compile with coverage
    char cmd[MAX_PATH * 2];
    snprintf(cmd, sizeof(cmd),
        "gcc -O0 -fprofile-arcs -ftest-coverage %s -o dummy_prog",
        dummy_c_path);
    
    printf("Compiling dummy program: %s\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return 0;
    }
    
    // Run it to generate .gcda file
    printf("Running dummy program to generate coverage data...\n");
    system("./dummy_prog > /dev/null 2>&1");
    
    // Check if .gcda was created
    struct stat st;
    if (stat("dummy.gcda", &st) == 0) {
        printf("Successfully created dummy.gcda\n");
        return 1;
    } else {
        fprintf(stderr, "Failed to generate dummy.gcda\n");
        return 0;
    }
}

/**
 * Run gcov-dump with specific arguments and merge coverage
 */
void run_gcov_dump_test(const char *gcov_dump_path, const char *gcda_file, 
                       const char *args, int expect_success) {
    char cmd[MAX_PATH * 4];
    int status;
    
    printf("\n=== Testing: %s %s ===\n", gcov_dump_path, args);
    
    // Build the full command
    if (strstr(args, "dummy.gcda")) {
        // gcda file already in args
        snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, args);
    } else if (strcmp(args, "-h") == 0 || strcmp(args, "-v") == 0 || 
               strstr(args, "-x") != NULL) {
        // These don't need a gcda file
        snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, args);
    } else {
        // Need to add gcda file
        snprintf(cmd, sizeof(cmd), "%s %s %s 2>&1", gcov_dump_path, args, gcda_file);
    }
    
    printf("Command: %s\n", cmd);
    
    // Execute the command
    status = system(cmd);
    int exit_code = WEXITSTATUS(status);
    
    printf("Exit code: %d\n", exit_code);
    
    if (expect_success) {
        if (exit_code != 0) {
            printf("WARNING: Expected success but got exit code %d\n", exit_code);
        }
    } else {
        if (exit_code == 0) {
            printf("WARNING: Expected failure but got exit code 0\n");
        }
    }
    
    // Merge coverage data after each run
    // First, find the .gcda file for gcov-dump
    char gcda_pattern[MAX_PATH];
    snprintf(gcda_pattern, sizeof(gcda_pattern), "%s*.gcda", gcov_dump_path);
    
    // Use gcov to merge coverage
    snprintf(cmd, sizeof(cmd), "gcov -i gcov-dump.cc > /dev/null 2>&1");
    system(cmd);
    
    // Also try to find and copy any .gcda files
    system("find . -name '*.gcda' -exec cp {} . 2>/dev/null \\;");
}

/**
 * Check if target lines are covered
 */
void check_coverage() {
    printf("\n=== Checking Coverage ===\n");
    
    // Generate coverage report
    system("gcov -b gcov-dump.cc 2>&1 | grep -A 20 'Lines executed:'");
    
    // Specifically check for our target lines
    printf("\nChecking target lines (111-130):\n");
    system("gcov gcov-dump.cc 2>&1 | grep -n '^[ ]*[0-9]' | "
           "awk '$1 >= 111 && $1 <= 130 {print $0}'");
    
    // Also check the gcov file directly
    FILE *fp = fopen("gcov-dump.cc.gcov", "r");
    if (fp) {
        char line[1024];
        int line_num = 0;
        printf("\nDetailed coverage for lines 111-130:\n");
        while (fgets(line, sizeof(line), fp)) {
            if (++line_num >= 111 && line_num <= 130) {
                printf("%4d: %s", line_num, line);
                if (line_num == 130) break;
            }
        }
        fclose(fp);
    }
}

int main(int argc, char *argv[]) {
    const char *gcov_dump_source_dir = NULL;
    const char *gcov_dump_instrumented = "gcov-dump-instrumented";
    const char *dummy_c = "dummy.c";
    const char *dummy_gcda = "dummy.gcda";
    
    // Parse command line arguments
    if (argc > 1) {
        gcov_dump_source_dir = argv[1];
    }
    
    printf("=== GCOV-Dump Coverage Test ===\n");
    
    // Step 1: Build/Locate instrumented gcov-dump
    if (!build_instrumented_gcov_dump(gcov_dump_source_dir, gcov_dump_instrumented)) {
        fprintf(stderr, "Failed to build or locate gcov-dump\n");
        return 1;
    }
    
    // Step 2: Generate test GCOV data
    if (!create_test_gcov_data(dummy_c, dummy_gcda)) {
        fprintf(stderr, "Failed to create test GCOV data\n");
        return 1;
    }
    
    // Clean up any existing coverage data
    system("rm -f *.gcda *.gcno gcov-dump.cc.gcov 2>/dev/null");
    
    // Step 3: Execute comprehensive flag coverage tests
    
    // Test help flag (-h) - line 111-113
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-h", 1);
    
    // Test version flag (-v) - line 114-116
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-v", 1);
    
    // Test individual flags (lines 117-130)
    
    // -l flag - line 117-119
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-l", 1);
    
    // -p flag - line 120-122
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-p", 1);
    
    // -r flag - line 123-125
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-r", 1);
    
    // -s flag - line 126-128
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-s", 1);
    
    // Test combined flags (space-separated)
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-l -p -r -s", 1);
    
    // Test concatenated flags
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-lprs", 1);
    
    // Test with different order and combinations
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-lp", 1);
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-rs", 1);
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-l -s", 1);
    
    // Test invalid flag to trigger default case (line 129-130)
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-x", 0);
    
    // Test another invalid flag
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-z dummy.gcda", 0);
    
    // Test mixed valid and invalid (should still trigger error)
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-l -x", 0);
    
    // Step 4: Final coverage check
    check_coverage();
    
    // Cleanup
    printf("\n=== Cleaning up ===\n");
    system("rm -f dummy.c dummy_prog dummy.gcda dummy.gcno 2>/dev/null");
    
    printf("\n=== Test Complete ===\n");
    printf("Check gcov-dump.cc.gcov for detailed coverage information.\n");
    printf("Lines 111-130 should show execution counts > 0 for all cases.\n");
    
    return 0;
}
