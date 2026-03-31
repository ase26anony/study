/**
 * test_gcov_dump_flags.c
 * 
 * Comprehensive test for gcov-dump command-line flag parsing coverage.
 * Tests all switch cases in the argument parsing logic (lines 111-130).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_PATH 1024

/**
 * Compile gcov-dump with coverage instrumentation
 */
int compile_instrumented_gcov_dump(const char *source_dir) {
    char cmd[2048];
    const char *gcov_dump_source = "gcov-dump.cc";
    char source_path[MAX_PATH];
    
    // Find the source file
    if (source_dir && source_dir[0]) {
        snprintf(source_path, sizeof(source_path), "%s/%s", source_dir, gcov_dump_source);
    } else {
        // Try common locations
        const char *locations[] = {
            ".",
            "../gcc",
            "../../gcc",
            "/usr/src/gcc",
            NULL
        };
        
        int found = 0;
        for (int i = 0; locations[i]; i++) {
            snprintf(source_path, sizeof(source_path), "%s/%s", locations[i], gcov_dump_source);
            if (access(source_path, R_OK) == 0) {
                found = 1;
                break;
            }
        }
        
        if (!found) {
            fprintf(stderr, "Error: Could not find gcov-dump.cc source file\n");
            return 0;
        }
    }
    
    printf("Compiling instrumented gcov-dump from: %s\n", source_path);
    
    // Simple compilation command - adjust as needed for your environment
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include "
             "-I../../libiberty %s ../../libiberty/libiberty.a "
             "-o gcov-dump-instrumented",
             source_path);
    
    printf("Compilation command: %s\n", cmd);
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Warning: Compilation may have failed. Trying alternative...\n");
        
        // Try simpler compilation
        snprintf(cmd, sizeof(cmd),
                 "g++ -O0 -fprofile-arcs -ftest-coverage %s -o gcov-dump-instrumented",
                 source_path);
        
        printf("Alternative compilation: %s\n", cmd);
        result = system(cmd);
    }
    
    return (result == 0);
}

/**
 * Create a minimal C program to generate GCOV data
 */
int create_dummy_program() {
    FILE *fp = fopen("dummy.c", "w");
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
 * Compile and run dummy program to generate .gcda file
 */
int generate_gcda_file() {
    printf("Generating test GCOV data file...\n");
    
    // Create dummy program
    if (!create_dummy_program()) {
        return 0;
    }
    
    // Compile with coverage
    if (system("gcc -O0 -fprofile-arcs -ftest-coverage dummy.c -o dummy_prog") != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return 0;
    }
    
    // Run to generate .gcda
    if (system("./dummy_prog > /dev/null 2>&1") != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        return 0;
    }
    
    // Verify .gcda was created
    if (access("dummy.gcda", R_OK) != 0) {
        fprintf(stderr, "dummy.gcda not created\n");
        return 0;
    }
    
    printf("Generated dummy.gcda successfully\n");
    return 1;
}

/**
 * Run gcov-dump with specific arguments and merge coverage
 */
int run_gcov_dump_test(const char *args, const char *gcda_file, int expect_success) {
    char cmd[1024];
    int result;
    
    printf("Testing: gcov-dump-instrumented %s\n", args);
    
    // Build command
    if (gcda_file && gcda_file[0]) {
        snprintf(cmd, sizeof(cmd), "./gcov-dump-instrumented %s %s 2>&1", args, gcda_file);
    } else {
        snprintf(cmd, sizeof(cmd), "./gcov-dump-instrumented %s 2>&1", args);
    }
    
    // Execute
    result = system(cmd);
    
    // Check exit code expectation
    if (expect_success) {
        if (result != 0) {
            fprintf(stderr, "  FAIL: Expected success but got exit code %d\n", result);
            return 0;
        } else {
            printf("  PASS: Exit code 0 as expected\n");
        }
    } else {
        if (result == 0) {
            fprintf(stderr, "  FAIL: Expected failure but got exit code 0\n");
            return 0;
        } else {
            printf("  PASS: Non-zero exit code as expected\n");
        }
    }
    
    // Merge coverage data
    printf("  Merging coverage data...\n");
    system("gcov -i gcov-dump-instrumented-gcov-dump.cc > /dev/null 2>&1");
    
    return 1;
}

/**
 * Check if instrumented gcov-dump exists, compile if not
 */
int ensure_instrumented_gcov_dump(const char *source_dir) {
    if (access("gcov-dump-instrumented", X_OK) == 0) {
        printf("Found existing instrumented gcov-dump binary\n");
        return 1;
    }
    
    return compile_instrumented_gcov_dump(source_dir);
}

/**
 * Main test driver
 */
int main(int argc, char *argv[]) {
    const char *gcov_dump_source_dir = NULL;
    int all_tests_passed = 1;
    
    // Parse command line arguments
    if (argc > 1) {
        gcov_dump_source_dir = argv[1];
    }
    
    printf("=== GCOV-Dump Flag Parsing Coverage Test ===\n\n");
    
    // Step 1: Ensure we have instrumented gcov-dump
    if (!ensure_instrumented_gcov_dump(gcov_dump_source_dir)) {
        fprintf(stderr, "Failed to obtain instrumented gcov-dump binary\n");
        return 1;
    }
    
    // Step 2: Generate test GCOV data file
    if (!generate_gcda_file()) {
        fprintf(stderr, "Failed to generate test GCOV data\n");
        return 1;
    }
    
    printf("\n=== Running flag coverage tests ===\n\n");
    
    // Test individual flags (lines 111-130)
    
    // Test help flag (line 111-113)
    printf("\n1. Testing -h flag (help):\n");
    all_tests_passed &= run_gcov_dump_test("-h", NULL, 1);
    
    // Test version flag (line 114-116)
    printf("\n2. Testing -v flag (version):\n");
    all_tests_passed &= run_gcov_dump_test("-v", NULL, 1);
    
    // Test -l flag (line 117-118)
    printf("\n3. Testing -l flag (dump contents):\n");
    all_tests_passed &= run_gcov_dump_test("-l", "dummy.gcda", 1);
    
    // Test -p flag (line 119-120)
    printf("\n4. Testing -p flag (dump positions):\n");
    all_tests_passed &= run_gcov_dump_test("-p", "dummy.gcda", 1);
    
    // Test -r flag (line 121-122)
    printf("\n5. Testing -r flag (dump raw):\n");
    all_tests_passed &= run_gcov_dump_test("-r", "dummy.gcda", 1);
    
    // Test -s flag (line 123-124)
    printf("\n6. Testing -s flag (dump stable):\n");
    all_tests_passed &= run_gcov_dump_test("-s", "dummy.gcda", 1);
    
    // Test combined flags (space-separated)
    printf("\n7. Testing combined flags (-l -p -r -s):\n");
    all_tests_passed &= run_gcov_dump_test("-l -p -r -s", "dummy.gcda", 1);
    
    // Test concatenated flags
    printf("\n8. Testing concatenated flags (-lprs):\n");
    all_tests_passed &= run_gcov_dump_test("-lprs", "dummy.gcda", 1);
    
    // Test invalid flag (line 125-130, default case)
    printf("\n9. Testing invalid flag (-x) to trigger default case:\n");
    all_tests_passed &= run_gcov_dump_test("-x", "dummy.gcda", 0);
    
    // Test mixed valid and invalid flags
    printf("\n10. Testing mixed flags (-l -x):\n");
    all_tests_passed &= run_gcov_dump_test("-l -x", "dummy.gcda", 0);
    
    printf("\n=== Generating final coverage report ===\n");
    
    // Generate human-readable coverage report
    system("gcov -b gcov-dump-instrumented-gcov-dump.cc");
    
    // Check if coverage file was created
    if (access("gcov-dump.cc.gcov", R_OK) == 0) {
        printf("\nCoverage report generated: gcov-dump.cc.gcov\n");
        
        // Simple check for target lines
        printf("\nChecking coverage of target lines (111-130)...\n");
        system("grep -n '^[[:space:]]*[0-9][0-9]*:[[:space:]]*' gcov-dump.cc.gcov | "
               "awk -F: '$1 >= 111 && $1 <= 130' | head -20");
    } else {
        // Try alternative name
        system("gcov -b gcov-dump.cc");
    }
    
    // Cleanup
    printf("\n=== Cleaning up ===\n");
    remove("dummy.c");
    remove("dummy_prog");
    remove("dummy.gcda");
    remove("dummy.gcno");
    
    if (all_tests_passed) {
        printf("\n✅ All tests passed!\n");
        return 0;
    } else {
        printf("\n❌ Some tests failed\n");
        return 1;
    }
}
