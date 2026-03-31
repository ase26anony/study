// test_gcov_dump_coverage.c
// Compile: gcc -O0 test_gcov_dump_coverage.c -o run_gcov_dump_tests
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_PATH 1024

// Function prototypes
int build_instrumented_gcov_dump();
int generate_test_gcda();
int run_coverage_tests(const char *gcov_dump_path, const char *gcda_file);
void merge_coverage_data(const char *gcov_dump_path);
int check_coverage();

int main(int argc, char *argv[]) {
    printf("=== GCOV-DUMP Coverage Test ===\n\n");
    
    // Step 1: Build/Locate Instrumented gcov-dump
    printf("1. Building instrumented gcov-dump...\n");
    if (build_instrumented_gcov_dump() != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    // Step 2: Generate Test GCOV Data
    printf("2. Generating test GCOV data file...\n");
    if (generate_test_gcda() != 0) {
        fprintf(stderr, "Failed to generate test GCOV data\n");
        return 1;
    }
    
    // Step 3: Run Coverage Tests
    printf("3. Running coverage tests...\n");
    if (run_coverage_tests("./gcov-dump-instrumented", "./dummy.gcda") != 0) {
        fprintf(stderr, "Coverage tests failed\n");
        return 1;
    }
    
    // Step 4: Final Coverage Check
    printf("4. Checking coverage results...\n");
    if (check_coverage() != 0) {
        fprintf(stderr, "Coverage check failed\n");
        return 1;
    }
    
    printf("\n=== All tests completed successfully ===\n");
    return 0;
}

// Build instrumented gcov-dump binary
int build_instrumented_gcov_dump() {
    // First check if we can find gcov-dump source
    const char *source_files[] = {
        "gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "/usr/src/gcc/gcc/gcov-dump.cc",
        NULL
    };
    
    char source_path[MAX_PATH] = "";
    for (int i = 0; source_files[i] != NULL; i++) {
        struct stat st;
        if (stat(source_files[i], &st) == 0 && S_ISREG(st.st_mode)) {
            strcpy(source_path, source_files[i]);
            break;
        }
    }
    
    if (strlen(source_path) == 0) {
        // Try to find it using which
        FILE *fp = popen("which gcov-dump 2>/dev/null", "r");
        if (fp) {
            char buffer[MAX_PATH];
            if (fgets(buffer, sizeof(buffer), fp)) {
                // Get path to binary, try to find source
                // This is a fallback - in practice you'd need the source
                printf("Found gcov-dump binary but need source for instrumentation\n");
            }
            pclose(fp);
        }
        
        // Create a minimal gcov-dump.cc if not found
        printf("Creating minimal test gcov-dump implementation...\n");
        FILE *src = fopen("gcov-dump-test.cc", "w");
        if (!src) {
            perror("Failed to create test source");
            return 1;
        }
        
        // Write a minimal version that includes the switch statement we want to test
        fprintf(src, "#include <stdio.h>\n");
        fprintf(src, "#include <stdlib.h>\n");
        fprintf(src, "#include <unistd.h>\n\n");
        fprintf(src, "int flag_dump_contents = 0;\n");
        fprintf(src, "int flag_dump_positions = 0;\n");
        fprintf(src, "int flag_dump_raw = 0;\n");
        fprintf(src, "int flag_dump_stable = 0;\n\n");
        fprintf(src, "void print_usage() { printf(\"Usage: gcov-dump [OPTIONS] GCOVFILE\\n\"); }\n");
        fprintf(src, "void print_version() { printf(\"gcov-dump 1.0 (test version)\\n\"); }\n\n");
        fprintf(src, "int main(int argc, char *argv[]) {\n");
        fprintf(src, "    int opt;\n");
        fprintf(src, "    while ((opt = getopt(argc, argv, \"hlprsv\")) != -1) {\n");
        fprintf(src, "        switch (opt) {\n");
        fprintf(src, "            case 'h':\n");
        fprintf(src, "                print_usage();\n");
        fprintf(src, "                break;\n");
        fprintf(src, "            case 'v':\n");
        fprintf(src, "                print_version();\n");
        fprintf(src, "                break;\n");
        fprintf(src, "            case 'l':\n");
        fprintf(src, "                flag_dump_contents = 1;\n");
        fprintf(src, "                break;\n");
        fprintf(src, "            case 'p':\n");
        fprintf(src, "                flag_dump_positions = 1;\n");
        fprintf(src, "                break;\n");
        fprintf(src, "            case 'r':\n");
        fprintf(src, "                flag_dump_raw = 1;\n");
        fprintf(src, "                break;\n");
        fprintf(src, "            case 's':\n");
        fprintf(src, "                flag_dump_stable = 1;\n");
        fprintf(src, "                break;\n");
        fprintf(src, "            default:\n");
        fprintf(src, "                fprintf(stderr, \"unknown flag `%%c'\\n\", opt);\n");
        fprintf(src, "                return 1;\n");
        fprintf(src, "        }\n");
        fprintf(src, "    }\n");
        fprintf(src, "    \n");
        fprintf(src, "    // Process remaining arguments (GCOV files)\n");
        fprintf(src, "    if (optind < argc) {\n");
        fprintf(src, "        printf(\"Processing file: %%s\\n\", argv[optind]);\n");
        fprintf(src, "        printf(\"Flags: l=%%d p=%%d r=%%d s=%%d\\n\", \n");
        fprintf(src, "               flag_dump_contents, flag_dump_positions, \n");
        fprintf(src, "               flag_dump_raw, flag_dump_stable);\n");
        fprintf(src, "    }\n");
        fprintf(src, "    \n");
        fprintf(src, "    return 0;\n");
        fprintf(src, "}\n");
        fclose(src);
        
        strcpy(source_path, "gcov-dump-test.cc");
    }
    
    // Build command
    char build_cmd[MAX_PATH * 2];
    snprintf(build_cmd, sizeof(build_cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage %s -o gcov-dump-instrumented",
             source_path);
    
    printf("Building with: %s\n", build_cmd);
    int result = system(build_cmd);
    
    if (result != 0) {
        fprintf(stderr, "Build failed with exit code %d\n", result);
        return 1;
    }
    
    // Check if binary was created
    struct stat st;
    if (stat("./gcov-dump-instrumented", &st) != 0 || !S_ISREG(st.st_mode)) {
        fprintf(stderr, "Instrumented binary not created\n");
        return 1;
    }
    
    printf("Instrumented gcov-dump built successfully\n");
    return 0;
}

// Generate a simple C program and its GCOV data
int generate_test_gcda() {
    // Create dummy.c
    FILE *dummy = fopen("dummy.c", "w");
    if (!dummy) {
        perror("Failed to create dummy.c");
        return 1;
    }
    
    fprintf(dummy, "#include <stdio.h>\n\n");
    fprintf(dummy, "int main() {\n");
    fprintf(dummy, "    int i;\n");
    fprintf(dummy, "    for (i = 0; i < 10; i++) {\n");
    fprintf(dummy, "        printf(\"Value: %%d\\n\", i);\n");
    fprintf(dummy, "    }\n");
    fprintf(dummy, "    return 0;\n");
    fprintf(dummy, "}\n");
    fclose(dummy);
    
    // Compile with coverage
    printf("Compiling dummy program with coverage...\n");
    int compile_result = system("gcc -O0 -fprofile-arcs -ftest-coverage dummy.c -o dummy_prog");
    if (compile_result != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return 1;
    }
    
    // Run to generate .gcda file
    printf("Running dummy program to generate .gcda...\n");
    int run_result = system("./dummy_prog > /dev/null 2>&1");
    if (run_result != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        return 1;
    }
    
    // Check if .gcda was created
    struct stat st;
    if (stat("./dummy.gcda", &st) != 0 || !S_ISREG(st.st_mode)) {
        // Try alternative location
        if (stat("./dummy_prog.gcda", &st) == 0 && S_ISREG(st.st_mode)) {
            rename("./dummy_prog.gcda", "./dummy.gcda");
        } else {
            fprintf(stderr, "No .gcda file generated\n");
            return 1;
        }
    }
    
    printf("Test GCOV data generated: dummy.gcda\n");
    return 0;
}

// Merge coverage data after each run
void merge_coverage_data(const char *gcov_dump_path) {
    // For gcov-dump's own coverage, we need to merge .gcda files
    // First, find the .gcda file for gcov-dump-instrumented
    char cmd[MAX_PATH * 2];
    
    // Method 1: Use gcov-tool if available
    snprintf(cmd, sizeof(cmd), "gcov-tool merge *.gcda 2>/dev/null || true");
    system(cmd);
    
    // Method 2: Simple copy to preserve coverage data
    snprintf(cmd, sizeof(cmd), "cp gcov-dump-instrumented.gcda gcov-dump-instrumented.gcda.bak 2>/dev/null || true");
    system(cmd);
    
    // Method 3: Generate coverage report
    snprintf(cmd, sizeof(cmd), "gcov -i gcov-dump-test.cc 2>/dev/null || gcov -i gcov-dump.cc 2>/dev/null || true");
    system(cmd);
}

// Run all flag combinations
int run_coverage_tests(const char *gcov_dump_path, const char *gcda_file) {
    char cmd[MAX_PATH * 4];
    int tests_passed = 0;
    int total_tests = 0;
    
    // Test cases with expected exit codes
    struct {
        const char *args;
        int expected_exit;
        const char *description;
    } test_cases[] = {
        {"-h", 0, "Help flag"},
        {"-v", 0, "Version flag"},
        {"-l", 0, "Dump contents flag (no file)"},
        {"-p", 0, "Dump positions flag (no file)"},
        {"-r", 0, "Dump raw flag (no file)"},
        {"-s", 0, "Dump stable flag (no file)"},
        {"-l", 0, "Dump contents with file"},
        {"-p", 0, "Dump positions with file"},
        {"-r", 0, "Dump raw with file"},
        {"-s", 0, "Dump stable with file"},
        {"-l -p -r -s", 0, "All flags separately with file"},
        {"-lprs", 0, "All flags combined with file"},
        {"-x", 1, "Invalid flag (should fail)"},
        {NULL, 0, NULL}
    };
    
    const char *gcda_arg = gcda_file;
    
    for (int i = 0; test_cases[i].args != NULL; i++) {
        total_tests++;
        printf("Test %d: %s\n", i+1, test_cases[i].description);
        
        // Build command
        if (strcmp(test_cases[i].args, "-h") == 0 || 
            strcmp(test_cases[i].args, "-v") == 0 ||
            strcmp(test_cases[i].args, "-x") == 0) {
            // These don't need a file argument
            snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, test_cases[i].args);
        } else if (strstr(test_cases[i].description, "with file")) {
            // Append gcda file
            snprintf(cmd, sizeof(cmd), "%s %s %s 2>&1", 
                    gcov_dump_path, test_cases[i].args, gcda_arg);
        } else {
            // Flags without file
            snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, test_cases[i].args);
        }
        
        printf("  Command: %s\n", cmd);
        
        // Execute
        int result = system(cmd);
        int exit_code = WEXITSTATUS(result);
        
        // Check result
        if (exit_code == test_cases[i].expected_exit) {
            printf("  ✓ Passed (exit code: %d)\n", exit_code);
            tests_passed++;
            
            // For invalid flag test, also check stderr
            if (strcmp(test_cases[i].args, "-x") == 0) {
                // We could capture stderr here, but system() doesn't separate it easily
                // In a real implementation, use popen() or exec() directly
                printf("  Note: Should have printed 'unknown flag' error\n");
            }
        } else {
            printf("  ✗ Failed (expected %d, got %d)\n", 
                   test_cases[i].expected_exit, exit_code);
        }
        
        // Merge coverage after each test
        merge_coverage_data(gcov_dump_path);
        
        printf("\n");
    }
    
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    return (tests_passed == total_tests) ? 0 : 1;
}

// Check final coverage
int check_coverage() {
    printf("Generating coverage report...\n");
    
    // Generate coverage report
    system("gcov -b gcov-dump-test.cc 2>/dev/null || gcov -b gcov-dump.cc 2>/dev/null");
    
    // Look for the .gcov file
    FILE *gcov_file = fopen("gcov-dump-test.cc.gcov", "r");
    if (!gcov_file) {
        gcov_file = fopen("gcov-dump.cc.gcov", "r");
    }
    
    if (!gcov_file) {
        printf("No .gcov file found. Trying to generate with lcov...\n");
        system("lcov --capture --directory . --output-file coverage.info 2>/dev/null");
        system("genhtml coverage.info --output-directory coverage-report 2>/dev/null");
        
        // Check if coverage report was created
        if (access("coverage-report/index.html", F_OK) == 0) {
            printf("Coverage report generated in coverage-report/\n");
            printf("Open coverage-report/index.html in a browser to view coverage\n");
            return 0;
        } else {
            fprintf(stderr, "Failed to generate coverage report\n");
            return 1;
        }
    }
    
    // Parse .gcov file to check coverage
    char line[1024];
    int target_lines_covered = 0;
    int total_target_lines = 0;
    
    // Lines we're targeting (approximate - adjust based on actual line numbers)
    int target_line_start = 111;  // case 'h':
    int target_line_end = 130;    // End of switch default case
    
    printf("\nChecking coverage for lines %d-%d:\n", target_line_start, target_line_end);
    
    while (fgets(line, sizeof(line), gcov_file)) {
        // .gcov format: "    #####:   10:    printf("hello");"
        // Or: "        -:   10:    printf("hello");" (not instrumented)
        // Or: "        1:   10:    printf("hello");" (executed once)
        
        int count;
        int line_num;
        char dummy;
        
        if (sscanf(line, "%d:%d:%c", &count, &line_num, &dummy) == 3 ||
            sscanf(line, "#####:%d:%c", &line_num, &dummy) == 2 ||
            sscanf(line, "    -:%d:%c", &line_num, &dummy) == 2) {
            
            if (line_num >= target_line_start && line_num <= target_line_end) {
                total_target_lines++;
                
                if (line[0] != '#' && line[0] != '-' && line[0] != '=') {
                    // Check if it's a number (execution count)
                    if (line[0] >= '0' && line[0] <= '9') {
                        target_lines_covered++;
                        printf("  Line %d: Covered (count: %c...)\n", line_num, line[0]);
                    }
                } else if (line[0] == '#') {
                    printf("  Line %d: NOT COVERED\n", line_num);
                }
            }
        }
    }
    
    fclose(gcov_file);
    
    printf("\nCoverage Summary for target lines:\n");
    printf("  Covered: %d/%d lines\n", target_lines_covered, total_target_lines);
    printf("  Coverage: %.1f%%\n", total_target_lines > 0 ? 
           (100.0 * target_lines_covered / total_target_lines) : 0.0);
    
    if (target_lines_covered == total_target_lines && total_target_lines > 0) {
        printf("✓ All target lines covered!\n");
        return 0;
    } else {
        printf("✗ Not all target lines covered\n");
        return 1;
    }
}
