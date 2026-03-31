#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024

// Function prototypes
int build_instrumented_gcov_dump();
int generate_test_gcov_data();
int run_coverage_tests();
int merge_coverage_data();
int check_coverage_results();

int main() {
    printf("=== GCOV-Dump Coverage Test ===\n\n");
    
    // Step 1: Build instrumented gcov-dump
    printf("1. Building instrumented gcov-dump...\n");
    if (build_instrumented_gcov_dump() != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    // Step 2: Generate test GCOV data
    printf("2. Generating test GCOV data...\n");
    if (generate_test_gcov_data() != 0) {
        fprintf(stderr, "Failed to generate test GCOV data\n");
        return 1;
    }
    
    // Step 3: Run coverage tests
    printf("3. Running coverage tests...\n");
    if (run_coverage_tests() != 0) {
        fprintf(stderr, "Failed to run coverage tests\n");
        return 1;
    }
    
    // Step 4: Merge coverage data
    printf("4. Merging coverage data...\n");
    if (merge_coverage_data() != 0) {
        fprintf(stderr, "Failed to merge coverage data\n");
        return 1;
    }
    
    // Step 5: Check coverage results
    printf("5. Checking coverage results...\n");
    if (check_coverage_results() != 0) {
        fprintf(stderr, "Coverage check failed\n");
        return 1;
    }
    
    printf("\n=== All tests completed successfully ===\n");
    return 0;
}

int build_instrumented_gcov_dump() {
    char cmd[MAX_PATH * 2];
    int status;
    
    // First check if gcov-dump source exists
    struct stat st;
    if (stat("gcov-dump.cc", &st) != 0) {
        fprintf(stderr, "gcov-dump.cc not found in current directory\n");
        return 1;
    }
    
    // Try to find libiberty
    char *libiberty_path = "../../libiberty/libiberty.a";
    if (stat(libiberty_path, &st) != 0) {
        // Try alternative path
        libiberty_path = "../libiberty/libiberty.a";
        if (stat(libiberty_path, &st) != 0) {
            fprintf(stderr, "libiberty.a not found. Trying to compile without it...\n");
            libiberty_path = "";
        }
    }
    
    // Build command
    if (strlen(libiberty_path) > 0) {
        snprintf(cmd, sizeof(cmd),
            "g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include -I../../libiberty "
            "gcov-dump.cc %s -o gcov-dump-instrumented",
            libiberty_path);
    } else {
        snprintf(cmd, sizeof(cmd),
            "g++ -O0 -fprofile-arcs -ftest-coverage -I. "
            "gcov-dump.cc -o gcov-dump-instrumented");
    }
    
    printf("Executing: %s\n", cmd);
    status = system(cmd);
    
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("Successfully built instrumented gcov-dump\n");
        return 0;
    } else {
        fprintf(stderr, "Build failed with status %d\n", status);
        return 1;
    }
}

int generate_test_gcov_data() {
    // Create dummy.c test program
    FILE *fp = fopen("dummy.c", "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return 1;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i, sum = 0;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        sum += i;\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    printf(\"Sum: %%d\\n\", sum);\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    fclose(fp);
    
    // Compile dummy.c with coverage
    int status = system("gcc -O0 -fprofile-arcs -ftest-coverage dummy.c -o dummy_prog");
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("Compiled dummy.c successfully\n");
    } else {
        fprintf(stderr, "Failed to compile dummy.c\n");
        return 1;
    }
    
    // Run dummy_prog to generate .gcda file
    status = system("./dummy_prog");
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("Generated dummy.gcda successfully\n");
        return 0;
    } else {
        fprintf(stderr, "Failed to run dummy_prog\n");
        return 1;
    }
}

int run_coverage_tests() {
    char cmd[MAX_PATH * 4];
    int status;
    int all_passed = 1;
    
    // Test 1: Help flag (-h)
    printf("  Testing -h flag...\n");
    snprintf(cmd, sizeof(cmd), "./gcov-dump-instrumented -h 2>&1");
    status = system(cmd);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "    -h test failed\n");
        all_passed = 0;
    }
    
    // Test 2: Version flag (-v)
    printf("  Testing -v flag...\n");
    snprintf(cmd, sizeof(cmd), "./gcov-dump-instrumented -v 2>&1");
    status = system(cmd);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "    -v test failed\n");
        all_passed = 0;
    }
    
    // Test 3: -l flag with data file
    printf("  Testing -l flag...\n");
    snprintf(cmd, sizeof(cmd), "./gcov-dump-instrumented -l dummy.gcda 2>&1");
    status = system(cmd);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "    -l test failed\n");
        all_passed = 0;
    }
    
    // Test 4: -p flag with data file
    printf("  Testing -p flag...\n");
    snprintf(cmd, sizeof(cmd), "./gcov-dump-instrumented -p dummy.gcda 2>&1");
    status = system(cmd);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "    -p test failed\n");
        all_passed = 0;
    }
    
    // Test 5: -r flag with data file
    printf("  Testing -r flag...\n");
    snprintf(cmd, sizeof(cmd), "./gcov-dump-instrumented -r dummy.gcda 2>&1");
    status = system(cmd);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "    -r test failed\n");
        all_passed = 0;
    }
    
    // Test 6: -s flag with data file
    printf("  Testing -s flag...\n");
    snprintf(cmd, sizeof(cmd), "./gcov-dump-instrumented -s dummy.gcda 2>&1");
    status = system(cmd);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "    -s test failed\n");
        all_passed = 0;
    }
    
    // Test 7: Combined flags (space-separated)
    printf("  Testing -l -p -r -s flags...\n");
    snprintf(cmd, sizeof(cmd), "./gcov-dump-instrumented -l -p -r -s dummy.gcda 2>&1");
    status = system(cmd);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "    Combined flags test failed\n");
        all_passed = 0;
    }
    
    // Test 8: Concatenated flags
    printf("  Testing -lprs flag...\n");
    snprintf(cmd, sizeof(cmd), "./gcov-dump-instrumented -lprs dummy.gcda 2>&1");
    status = system(cmd);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "    Concatenated flags test failed\n");
        all_passed = 0;
    }
    
    // Test 9: Invalid flag (to trigger default case)
    printf("  Testing invalid flag -x...\n");
    snprintf(cmd, sizeof(cmd), "./gcov-dump-instrumented -x dummy.gcda 2>&1 | grep -q 'unknown flag'");
    status = system(cmd);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "    Invalid flag test failed (should have printed 'unknown flag')\n");
        all_passed = 0;
    }
    
    return all_passed ? 0 : 1;
}

int merge_coverage_data() {
    // Create coverage directory
    mkdir("coverage_data", 0755);
    
    // Copy .gcda files to coverage directory
    system("cp gcov-dump-instrumented*.gcda coverage_data/ 2>/dev/null || true");
    
    // Change to coverage directory and merge
    chdir("coverage_data");
    
    // Generate coverage info
    int status = system("gcov -i ../gcov-dump.cc 2>&1");
    
    chdir("..");
    
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("Successfully merged coverage data\n");
        return 0;
    } else {
        fprintf(stderr, "Failed to merge coverage data\n");
        return 1;
    }
}

int check_coverage_results() {
    char cmd[MAX_PATH * 2];
    char line[1024];
    int target_lines_covered = 0;
    int in_target_section = 0;
    
    // Generate detailed coverage report
    printf("Generating coverage report...\n");
    snprintf(cmd, sizeof(cmd), "gcov -b gcov-dump.cc 2>&1 | tee coverage_report.txt");
    system(cmd);
    
    // Check if coverage report was generated
    FILE *report = fopen("coverage_report.txt", "r");
    if (!report) {
        fprintf(stderr, "Failed to open coverage report\n");
        return 1;
    }
    
    printf("\n=== Coverage Summary ===\n");
    while (fgets(line, sizeof(line), report)) {
        // Look for lines around our target (111-130)
        if (strstr(line, "111:") || strstr(line, "112:") || strstr(line, "113:")) {
            in_target_section = 1;
        }
        
        if (in_target_section) {
            printf("%s", line);
            
            // Check if line shows execution
            if (strstr(line, "#####") == NULL && 
                strstr(line, ":") != NULL &&
                strstr(line, "branch") == NULL) {
                char *colon = strchr(line, ':');
                if (colon) {
                    int line_num = atoi(line);
                    if (line_num >= 111 && line_num <= 130) {
                        target_lines_covered++;
                    }
                }
            }
            
            if (strstr(line, "130:") || strstr(line, "131:")) {
                in_target_section = 0;
            }
        }
        
        // Also show overall summary
        if (strstr(line, "Lines executed:") || strstr(line, "Branches executed:")) {
            printf("%s", line);
        }
    }
    fclose(report);
    
    printf("\nTarget lines covered (111-130): %d/20\n", target_lines_covered);
    
    if (target_lines_covered >= 10) { // Most lines should be covered
        printf("✓ Sufficient coverage achieved\n");
        return 0;
    } else {
        printf("✗ Insufficient coverage\n");
        return 1;
    }
}
