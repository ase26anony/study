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
int check_coverage();

int main() {
    printf("=== Starting gcov-dump coverage test ===\n");
    
    // Step 1: Build instrumented gcov-dump
    printf("\n1. Building instrumented gcov-dump...\n");
    if (build_instrumented_gcov_dump() != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    // Step 2: Generate test GCOV data file
    printf("\n2. Generating test GCOV data file...\n");
    if (generate_test_gcda() != 0) {
        fprintf(stderr, "Failed to generate test GCOV data\n");
        return 1;
    }
    
    // Step 3: Run coverage tests
    printf("\n3. Running coverage tests...\n");
    if (run_coverage_tests("./gcov-dump-instrumented", "./dummy.gcda") != 0) {
        fprintf(stderr, "Failed to run coverage tests\n");
        return 1;
    }
    
    // Step 4: Check coverage results
    printf("\n4. Checking coverage results...\n");
    if (check_coverage() != 0) {
        fprintf(stderr, "Coverage check failed\n");
        return 1;
    }
    
    printf("\n=== Coverage test completed successfully ===\n");
    return 0;
}

int build_instrumented_gcov_dump() {
    // Check if gcov-dump-instrumented already exists
    struct stat st;
    if (stat("./gcov-dump-instrumented", &st) == 0) {
        printf("Instrumented gcov-dump already exists, skipping build\n");
        return 0;
    }
    
    // Try to find gcov-dump source
    const char *source_paths[] = {
        "./gcov-dump.cc",
        "../gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        NULL
    };
    
    const char *source_file = NULL;
    for (int i = 0; source_paths[i] != NULL; i++) {
        if (stat(source_paths[i], &st) == 0) {
            source_file = source_paths[i];
            break;
        }
    }
    
    if (source_file == NULL) {
        fprintf(stderr, "Could not find gcov-dump.cc source file\n");
        return 1;
    }
    
    printf("Found source at: %s\n", source_file);
    
    // Build command
    char cmd[MAX_PATH * 4];
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include -I../../libiberty "
        "%s ../../libiberty/libiberty.a -o gcov-dump-instrumented",
        source_file);
    
    printf("Building with: %s\n", cmd);
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Build failed with exit code %d\n", result);
        return 1;
    }
    
    printf("Successfully built instrumented gcov-dump\n");
    return 0;
}

int generate_test_gcda() {
    // Create dummy.c test program
    FILE *fp = fopen("dummy.c", "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return 1;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        printf(\"Value: %%d\\n\", i);\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    fclose(fp);
    
    // Compile dummy program with coverage
    printf("Compiling dummy program with coverage...\n");
    int result = system("gcc -O0 -fprofile-arcs -ftest-coverage dummy.c -o dummy_prog");
    if (result != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return 1;
    }
    
    // Run dummy program to generate .gcda file
    printf("Running dummy program to generate .gcda...\n");
    result = system("./dummy_prog > /dev/null 2>&1");
    if (result != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        return 1;
    }
    
    // Verify .gcda file was created
    if (stat("dummy.gcda", &st) != 0) {
        fprintf(stderr, "dummy.gcda not created\n");
        return 1;
    }
    
    printf("Generated dummy.gcda successfully\n");
    return 0;
}

int run_coverage_tests(const char *gcov_dump_path, const char *gcda_file) {
    char cmd[MAX_PATH * 4];
    int exit_code;
    
    // Clean any existing coverage data
    system("rm -f gcov-dump-instrumented.gcda gcov-dump-instrumented.gcno");
    
    printf("\nRunning test cases:\n");
    
    // Test 1: Help flag (-h)
    printf("  Test 1: -h flag\n");
    snprintf(cmd, sizeof(cmd), "%s -h", gcov_dump_path);
    exit_code = system(cmd);
    if (exit_code != 0) {
        printf("    WARNING: -h returned non-zero: %d\n", exit_code);
    }
    system("gcov -i gcov-dump.cc > /dev/null 2>&1");
    
    // Test 2: Version flag (-v)
    printf("  Test 2: -v flag\n");
    snprintf(cmd, sizeof(cmd), "%s -v", gcov_dump_path);
    exit_code = system(cmd);
    if (exit_code != 0) {
        printf("    WARNING: -v returned non-zero: %d\n", exit_code);
    }
    system("gcov -i gcov-dump.cc > /dev/null 2>&1");
    
    // Test 3: -l flag with gcda file
    printf("  Test 3: -l flag\n");
    snprintf(cmd, sizeof(cmd), "%s -l %s", gcov_dump_path, gcda_file);
    exit_code = system(cmd);
    if (exit_code != 0) {
        printf("    ERROR: -l returned non-zero: %d\n", exit_code);
        return 1;
    }
    system("gcov -i gcov-dump.cc > /dev/null 2>&1");
    
    // Test 4: -p flag with gcda file
    printf("  Test 4: -p flag\n");
    snprintf(cmd, sizeof(cmd), "%s -p %s", gcov_dump_path, gcda_file);
    exit_code = system(cmd);
    if (exit_code != 0) {
        printf("    ERROR: -p returned non-zero: %d\n", exit_code);
        return 1;
    }
    system("gcov -i gcov-dump.cc > /dev/null 2>&1");
    
    // Test 5: -r flag with gcda file
    printf("  Test 5: -r flag\n");
    snprintf(cmd, sizeof(cmd), "%s -r %s", gcov_dump_path, gcda_file);
    exit_code = system(cmd);
    if (exit_code != 0) {
        printf("    ERROR: -r returned non-zero: %d\n", exit_code);
        return 1;
    }
    system("gcov -i gcov-dump.cc > /dev/null 2>&1");
    
    // Test 6: -s flag with gcda file
    printf("  Test 6: -s flag\n");
    snprintf(cmd, sizeof(cmd), "%s -s %s", gcov_dump_path, gcda_file);
    exit_code = system(cmd);
    if (exit_code != 0) {
        printf("    ERROR: -s returned non-zero: %d\n", exit_code);
        return 1;
    }
    system("gcov -i gcov-dump.cc > /dev/null 2>&1");
    
    // Test 7: Combined flags (space-separated)
    printf("  Test 7: -l -p -r -s flags\n");
    snprintf(cmd, sizeof(cmd), "%s -l -p -r -s %s", gcov_dump_path, gcda_file);
    exit_code = system(cmd);
    if (exit_code != 0) {
        printf("    ERROR: Combined flags returned non-zero: %d\n", exit_code);
        return 1;
    }
    system("gcov -i gcov-dump.cc > /dev/null 2>&1");
    
    // Test 8: Concatenated flags
    printf("  Test 8: -lprs flags\n");
    snprintf(cmd, sizeof(cmd), "%s -lprs %s", gcov_dump_path, gcda_file);
    exit_code = system(cmd);
    if (exit_code != 0) {
        printf("    ERROR: Concatenated flags returned non-zero: %d\n", exit_code);
        return 1;
    }
    system("gcov -i gcov-dump.cc > /dev/null 2>&1");
    
    // Test 9: Invalid flag (to trigger default case)
    printf("  Test 9: Invalid -x flag (should fail)\n");
    snprintf(cmd, sizeof(cmd), "%s -x %s 2>&1", gcov_dump_path, gcda_file);
    exit_code = system(cmd);
    if (exit_code == 0) {
        printf("    ERROR: Invalid flag should have failed\n");
        return 1;
    }
    printf("    Invalid flag correctly returned non-zero: %d\n", exit_code);
    system("gcov -i gcov-dump.cc > /dev/null 2>&1");
    
    printf("\nAll test cases executed successfully\n");
    return 0;
}

int check_coverage() {
    printf("Generating coverage report...\n");
    
    // Generate coverage report
    int result = system("gcov -b gcov-dump.cc > coverage_report.txt 2>&1");
    if (result != 0) {
        fprintf(stderr, "Failed to generate coverage report\n");
        return 1;
    }
    
    // Display coverage summary
    printf("\nCoverage Report Summary:\n");
    system("grep -A5 -B5 'Lines executed:' coverage_report.txt");
    
    // Check specifically for our target lines
    printf("\nChecking target lines (111-130)...\n");
    
    // Create a simple check
    FILE *report = fopen("coverage_report.txt", "r");
    if (!report) {
        perror("Failed to open coverage report");
        return 1;
    }
    
    char line[1024];
    int target_lines_covered = 0;
    int total_target_lines = 0;
    
    while (fgets(line, sizeof(line), report)) {
        // Look for lines in the range 111-130
        if (strstr(line, ":") && !strstr(line, "function") && !strstr(line, "Creating")) {
            char *colon = strchr(line, ':');
            if (colon) {
                int line_num = atoi(line);
                if (line_num >= 111 && line_num <= 130) {
                    total_target_lines++;
                    // Check if line was executed (has a number before the colon)
                    if (line[0] >= '0' && line[0] <= '9') {
                        target_lines_covered++;
                        printf("  Line %d: COVERED\n", line_num);
                    } else {
                        printf("  Line %d: NOT COVERED\n", line_num);
                    }
                }
            }
        }
    }
    
    fclose(report);
    
    printf("\nTarget lines coverage: %d/%d lines\n", 
           target_lines_covered, total_target_lines);
    
    if (target_lines_covered > 0) {
        printf("SUCCESS: Target switch-case logic was exercised!\n");
        return 0;
    } else {
        printf("FAILURE: Target lines were not covered\n");
        return 1;
    }
}
