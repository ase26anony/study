// test_gcov_dump_coverage.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024

// Function prototypes
int build_instrumented_gcov_dump();
int create_test_gcda_file();
int run_gcov_dump_with_flags(const char *flags, const char *gcda_file, int expect_success);
void merge_coverage_data();
void check_final_coverage();

int main() {
    printf("=== Starting gcov-dump coverage test ===\n\n");
    
    // Step 1: Build instrumented gcov-dump
    printf("1. Building instrumented gcov-dump...\n");
    if (build_instrumented_gcov_dump() != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    // Step 2: Create test GCOV data file
    printf("2. Creating test GCOV data file...\n");
    if (create_test_gcda_file() != 0) {
        fprintf(stderr, "Failed to create test GCOV data file\n");
        return 1;
    }
    
    // Step 3: Execute flag coverage series
    printf("3. Executing flag coverage series...\n\n");
    
    // Test cases with expected outcomes
    struct {
        const char *flags;
        int expect_success;
        const char *description;
    } test_cases[] = {
        {"-h", 1, "Help flag"},
        {"-v", 1, "Version flag"},
        {"-l dummy.gcda", 1, "Dump contents flag"},
        {"-p dummy.gcda", 1, "Dump positions flag"},
        {"-r dummy.gcda", 1, "Dump raw flag"},
        {"-s dummy.gcda", 1, "Dump stable flag"},
        {"-l -p -r -s dummy.gcda", 1, "Combined flags (space-separated)"},
        {"-lprs dummy.gcda", 1, "Combined flags (concatenated)"},
        {"-x dummy.gcda", 0, "Invalid flag (should fail)"},
        {NULL, 0, NULL}
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    
    for (int i = 0; test_cases[i].flags != NULL; i++) {
        printf("Test %d: %s\n", i + 1, test_cases[i].description);
        printf("  Command: gcov-dump-instrumented %s\n", test_cases[i].flags);
        
        if (run_gcov_dump_with_flags(test_cases[i].flags, "dummy.gcda", 
                                     test_cases[i].expect_success) == 0) {
            printf("  ✓ PASSED\n");
            passed_tests++;
        } else {
            printf("  ✗ FAILED\n");
        }
        
        // Merge coverage after each test
        merge_coverage_data();
        printf("\n");
        total_tests++;
    }
    
    printf("Test summary: %d/%d tests passed\n\n", passed_tests, total_tests);
    
    // Step 4: Check final coverage
    printf("4. Checking final coverage...\n");
    check_final_coverage();
    
    printf("\n=== Coverage test completed ===\n");
    return (passed_tests == total_tests) ? 0 : 1;
}

int build_instrumented_gcov_dump() {
    // First, create a simple dummy.c file for testing
    FILE *dummy_src = fopen("dummy.c", "w");
    if (!dummy_src) {
        perror("Failed to create dummy.c");
        return 1;
    }
    
    fprintf(dummy_src, "#include <stdio.h>\n\n");
    fprintf(dummy_src, "int main() {\n");
    fprintf(dummy_src, "    int i;\n");
    fprintf(dummy_src, "    for (i = 0; i < 5; i++) {\n");
    fprintf(dummy_src, "        printf(\"Test iteration %%d\\n\", i);\n");
    fprintf(dummy_src, "    }\n");
    fprintf(dummy_src, "    return 0;\n");
    fprintf(dummy_src, "}\n");
    fclose(dummy_src);
    
    // Try to find or build gcov-dump
    // First, check if we can find gcov-dump source
    char cmd[MAX_PATH * 2];
    int ret;
    
    // Try to compile gcov-dump if we can find the source
    // This assumes gcov-dump.cc is in the current directory
    struct stat st;
    if (stat("gcov-dump.cc", &st) == 0) {
        printf("   Found gcov-dump.cc, compiling with coverage instrumentation...\n");
        
        // Try to compile with minimal dependencies
        // This is a simplified version - adjust based on actual build requirements
        snprintf(cmd, sizeof(cmd),
                 "g++ -O0 -fprofile-arcs -ftest-coverage -I. "
                 "-I../../include -I../../libiberty "
                 "gcov-dump.cc ../../libiberty/libiberty.a "
                 "-o gcov-dump-instrumented 2>&1");
        
        printf("   Compilation command: %s\n", cmd);
        ret = system(cmd);
        
        if (ret != 0) {
            // Try a simpler compilation if the above fails
            printf("   First attempt failed, trying simpler compilation...\n");
            snprintf(cmd, sizeof(cmd),
                     "g++ -O0 -fprofile-arcs -ftest-coverage "
                     "gcov-dump.cc -lgcov -o gcov-dump-instrumented 2>&1");
            ret = system(cmd);
        }
        
        if (ret != 0) {
            fprintf(stderr, "   Compilation failed\n");
            return 1;
        }
        
        printf("   Successfully built gcov-dump-instrumented\n");
        return 0;
    } else {
        // Try to find existing gcov-dump and instrument it
        printf("   gcov-dump.cc not found, looking for existing binary...\n");
        
        // Check common locations
        const char *paths[] = {
            "/usr/bin/gcov-dump",
            "/usr/local/bin/gcov-dump",
            "/bin/gcov-dump",
            NULL
        };
        
        for (int i = 0; paths[i] != NULL; i++) {
            if (access(paths[i], X_OK) == 0) {
                printf("   Found gcov-dump at %s\n", paths[i]);
                // Create a symlink
                if (symlink(paths[i], "gcov-dump-instrumented") == 0) {
                    printf("   Created symlink to existing gcov-dump\n");
                    return 0;
                }
            }
        }
        
        fprintf(stderr, "   Could not find or build gcov-dump\n");
        return 1;
    }
}

int create_test_gcda_file() {
    // Compile dummy.c with coverage
    printf("   Compiling dummy.c with coverage...\n");
    
    if (system("gcc -O0 -fprofile-arcs -ftest-coverage dummy.c -o dummy_prog") != 0) {
        fprintf(stderr, "   Failed to compile dummy.c\n");
        return 1;
    }
    
    // Run the program to generate .gcda file
    printf("   Running dummy_prog to generate coverage data...\n");
    
    if (system("./dummy_prog > /dev/null 2>&1") != 0) {
        fprintf(stderr, "   Failed to run dummy_prog\n");
        return 1;
    }
    
    // Check if .gcda file was created
    if (access("dummy.gcda", R_OK) != 0) {
        // Try alternative naming
        if (system("mv dummy_prog.gcda dummy.gcda 2>/dev/null") != 0) {
            fprintf(stderr, "   No .gcda file generated\n");
            return 1;
        }
    }
    
    printf("   Successfully created dummy.gcda\n");
    return 0;
}

int run_gcov_dump_with_flags(const char *flags, const char *gcda_file, int expect_success) {
    char cmd[MAX_PATH * 2];
    int ret;
    
    // Build the command
    if (strstr(flags, gcda_file) == NULL) {
        // Flags that don't need a gcda file (like -h, -v)
        snprintf(cmd, sizeof(cmd), "./gcov-dump-instrumented %s 2>&1", flags);
    } else {
        // Flags already include the gcda file
        snprintf(cmd, sizeof(cmd), "./gcov-dump-instrumented %s 2>&1", flags);
    }
    
    printf("   Executing: %s\n", cmd);
    
    // Execute the command
    ret = system(cmd);
    int exit_status = WEXITSTATUS(ret);
    
    // Check if result matches expectation
    if (expect_success) {
        if (exit_status == 0) {
            return 0;  // Success
        } else {
            printf("   Expected success but got exit code %d\n", exit_status);
            return 1;  // Failure
        }
    } else {
        // For invalid flag, we expect non-zero exit
        if (exit_status != 0) {
            // Also check that error message was printed
            FILE *fp = popen(cmd, "r");
            if (fp) {
                char buffer[256];
                int found_error = 0;
                while (fgets(buffer, sizeof(buffer), fp)) {
                    if (strstr(buffer, "unknown flag")) {
                        found_error = 1;
                        break;
                    }
                }
                pclose(fp);
                
                if (found_error) {
                    return 0;  // Success - got expected error
                }
            }
        }
        printf("   Expected failure but got exit code %d\n", exit_status);
        return 1;  // Failure
    }
}

void merge_coverage_data() {
    // Merge coverage data by running gcov on the instrumented binary
    // This accumulates coverage from all invocations
    
    // First, find the .gcda file for gcov-dump
    char cmd[MAX_PATH];
    
    // Try different possible locations for the .gcda file
    const char *gcda_files[] = {
        "gcov-dump-instrumented.gcda",
        "gcov-dump.gcda",
        NULL
    };
    
    for (int i = 0; gcda_files[i] != NULL; i++) {
        if (access(gcda_files[i], R_OK) == 0) {
            // Found a .gcda file, merge it
            snprintf(cmd, sizeof(cmd), "gcov -i %s > /dev/null 2>&1", gcda_files[i]);
            system(cmd);
            break;
        }
    }
    
    // Also try to merge using lcov if available
    if (system("which lcov > /dev/null 2>&1") == 0) {
        system("lcov --capture --directory . --output-file coverage.info > /dev/null 2>&1");
    }
}

void check_final_coverage() {
    printf("   Generating final coverage report...\n");
    
    // Generate coverage report for gcov-dump.cc
    if (system("gcov -b gcov-dump.cc > coverage_report.txt 2>&1") == 0) {
        printf("   Coverage report saved to coverage_report.txt\n");
        
        // Display summary
        FILE *fp = fopen("coverage_report.txt", "r");
        if (fp) {
            char line[256];
            int in_target_section = 0;
            
            printf("\n   Coverage summary for target lines (111-130):\n");
            printf("   ---------------------------------------------\n");
            
            while (fgets(line, sizeof(line), fp)) {
                // Look for lines in our target range
                if (strstr(line, ":") && !strstr(line, "-")) {
                    int line_num;
                    if (sscanf(line, "%d:", &line_num) == 1) {
                        if (line_num >= 111 && line_num <= 130) {
                            printf("   %s", line);
                        }
                    }
                }
                
                // Also show overall summary
                if (strstr(line, "Lines executed:") || 
                    strstr(line, "Branches executed:") ||
                    strstr(line, "Taken at least once:")) {
                    printf("   %s", line);
                }
            }
            fclose(fp);
        }
        
        // Check if target lines were covered
        printf("\n   Checking if target switch cases were executed:\n");
        
        // Create a simple check by looking at the .gcov file
        if (system("grep -n '^[^#]*:.*111-130' gcov-dump.cc.gcov > /dev/null") == 0) {
            printf("   ✓ Target lines appear in coverage data\n");
        } else {
            printf("   ⚠ Target lines may not be fully covered\n");
        }
    } else {
        printf("   Failed to generate coverage report\n");
    }
    
    // Clean up temporary files
    system("rm -f dummy_prog dummy.gcda dummy.gcno 2>/dev/null");
    system("rm -f *.gcov coverage.info 2>/dev/null");
}
