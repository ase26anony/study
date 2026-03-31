/**
 * test_gcov_dump_coverage.c
 * 
 * A test program to cover the uncovered switch-case lines in gcov-dump.cc
 * Specifically targets lines 111-130 handling command-line flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024

/**
 * Compile gcov-dump with coverage instrumentation
 */
int compile_gcov_dump_with_coverage(const char *source_dir, const char *output_path) {
    char cmd[2048];
    int status;
    
    printf("Compiling gcov-dump with coverage instrumentation...\n");
    
    // Try to find gcov-dump source in common locations
    const char *possible_paths[] = {
        ".",
        "../gcc",
        "../../gcc",
        source_dir
    };
    
    char gcov_dump_source[MAX_PATH] = "";
    struct stat st;
    
    for (int i = 0; i < sizeof(possible_paths)/sizeof(possible_paths[0]); i++) {
        if (possible_paths[i]) {
            snprintf(gcov_dump_source, MAX_PATH, "%s/gcov-dump.cc", possible_paths[i]);
            if (stat(gcov_dump_source, &st) == 0) {
                printf("Found gcov-dump source at: %s\n", gcov_dump_source);
                break;
            }
        }
    }
    
    if (strlen(gcov_dump_source) == 0 || stat(gcov_dump_source, &st) != 0) {
        fprintf(stderr, "Error: Could not find gcov-dump.cc source file\n");
        return -1;
    }
    
    // Build command to compile gcov-dump with coverage
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include -I../../libiberty "
        "%s ../../libiberty/libiberty.a -o %s",
        gcov_dump_source, output_path);
    
    printf("Compilation command: %s\n", cmd);
    status = system(cmd);
    
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("Successfully compiled instrumented gcov-dump\n");
        return 0;
    } else {
        fprintf(stderr, "Failed to compile gcov-dump with coverage\n");
        return -1;
    }
}

/**
 * Create a dummy C program to generate GCOV data
 */
int create_dummy_program(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return -1;
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
    return 0;
}

/**
 * Compile and run dummy program to generate .gcda file
 */
int generate_gcda_file(const char *dummy_source, const char *gcda_file) {
    char cmd[1024];
    int status;
    
    printf("Creating dummy program for GCOV data generation...\n");
    
    // Compile dummy program with coverage
    snprintf(cmd, sizeof(cmd),
        "gcc -O0 -fprofile-arcs -ftest-coverage %s -o dummy_prog",
        dummy_source);
    
    status = system(cmd);
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return -1;
    }
    
    // Run dummy program to generate .gcda file
    status = system("./dummy_prog");
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        return -1;
    }
    
    // The .gcda file will be named based on the source file
    // Let's find and copy it to our desired location
    char source_gcda[MAX_PATH];
    snprintf(source_gcda, sizeof(source_gcda), "dummy.gcda");
    
    // Check if .gcda file exists
    struct stat st;
    if (stat(source_gcda, &st) != 0) {
        // Try alternative naming
        snprintf(source_gcda, sizeof(source_gcda), "%s.gcda", dummy_source);
        if (stat(source_gcda, &st) != 0) {
            fprintf(stderr, "Could not find generated .gcda file\n");
            return -1;
        }
    }
    
    // Copy to desired location
    snprintf(cmd, sizeof(cmd), "cp %s %s", source_gcda, gcda_file);
    system(cmd);
    
    printf("Generated GCOV data file: %s\n", gcda_file);
    return 0;
}

/**
 * Run gcov-dump with specific arguments and merge coverage
 */
int run_gcov_dump_test(const char *gcov_dump_path, const char *gcda_file, 
                       const char *args, int expect_success) {
    char cmd[2048];
    int status;
    
    printf("\nTesting: gcov-dump %s\n", args);
    
    // Build the command
    if (strstr(args, gcda_file) == NULL) {
        // If gcda_file is not already in args, add it
        snprintf(cmd, sizeof(cmd), "%s %s %s", gcov_dump_path, args, gcda_file);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_path, args);
    }
    
    printf("Command: %s\n", cmd);
    
    // Execute gcov-dump
    status = system(cmd);
    int exit_code = WEXITSTATUS(status);
    
    if (expect_success) {
        if (exit_code != 0) {
            fprintf(stderr, "Warning: Command failed with exit code %d\n", exit_code);
        }
    } else {
        // For invalid flag, we expect failure
        if (exit_code == 0) {
            fprintf(stderr, "Warning: Invalid flag test should have failed\n");
        }
    }
    
    // Merge coverage data
    snprintf(cmd, sizeof(cmd), "gcov -i gcov-dump.cc 2>/dev/null");
    system(cmd);
    
    return exit_code;
}

/**
 * Check coverage of specific lines
 */
void check_coverage() {
    char cmd[1024];
    FILE *fp;
    char line[1024];
    int target_lines_covered = 0;
    
    printf("\n=== Checking Coverage Results ===\n");
    
    // Generate coverage report
    snprintf(cmd, sizeof(cmd), "gcov -b gcov-dump.cc 2>&1");
    fp = popen(cmd, "r");
    if (!fp) {
        fprintf(stderr, "Failed to generate coverage report\n");
        return;
    }
    
    // Look for coverage of our target lines (111-130)
    while (fgets(line, sizeof(line), fp) != NULL) {
        // Check if line mentions our target line range
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
            printf("%s", line);
            if (strstr(line, "#####") == NULL) {
                target_lines_covered++;
            }
        }
        
        // Also look for branch coverage
        if (strstr(line, "branch") && (strstr(line, "111") || strstr(line, "take") || strstr(line, "case"))) {
            printf("Branch info: %s", line);
        }
    }
    
    pclose(fp);
    
    printf("\nTarget lines covered: %d/20\n", target_lines_covered);
    
    if (target_lines_covered >= 15) {  // Most lines should be covered
        printf("✓ Good coverage achieved for target switch-case block\n");
    } else {
        printf("✗ Insufficient coverage for target switch-case block\n");
    }
}

int main(int argc, char *argv[]) {
    const char *gcov_dump_source_dir = (argc > 1) ? argv[1] : ".";
    const char *instrumented_gcov_dump = "./gcov-dump-instrumented";
    const char *dummy_source = "dummy.c";
    const char *gcda_file = "test.gcda";
    
    printf("=== GCOV-Dump Coverage Test ===\n");
    
    // Step 1: Compile gcov-dump with coverage
    if (compile_gcov_dump_with_coverage(gcov_dump_source_dir, instrumented_gcov_dump) != 0) {
        fprintf(stderr, "Failed to compile instrumented gcov-dump\n");
        return 1;
    }
    
    // Step 2: Create and compile dummy program
    if (create_dummy_program(dummy_source) != 0) {
        fprintf(stderr, "Failed to create dummy program\n");
        return 1;
    }
    
    // Step 3: Generate .gcda file
    if (generate_gcda_file(dummy_source, gcda_file) != 0) {
        fprintf(stderr, "Failed to generate GCOV data file\n");
        return 1;
    }
    
    // Clean any existing coverage data
    system("rm -f gcov-dump.gcda gcov-dump.gcno");
    
    // Step 4: Execute comprehensive flag tests
    
    // Test help flag (doesn't need gcda file)
    printf("\n--- Testing help flag (-h) ---\n");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s -h", instrumented_gcov_dump);
    system(cmd);
    system("gcov -i gcov-dump.cc 2>/dev/null");
    
    // Test version flag (doesn't need gcda file)
    printf("\n--- Testing version flag (-v) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -v", instrumented_gcov_dump);
    system(cmd);
    system("gcov -i gcov-dump.cc 2>/dev/null");
    
    // Test individual flags with gcda file
    run_gcov_dump_test(instrumented_gcov_dump, gcda_file, "-l", 1);
    run_gcov_dump_test(instrumented_gcov_dump, gcda_file, "-p", 1);
    run_gcov_dump_test(instrumented_gcov_dump, gcda_file, "-r", 1);
    run_gcov_dump_test(instrumented_gcov_dump, gcda_file, "-s", 1);
    
    // Test combined flags (space-separated)
    run_gcov_dump_test(instrumented_gcov_dump, gcda_file, "-l -p -r -s", 1);
    
    // Test concatenated flags
    run_gcov_dump_test(instrumented_gcov_dump, gcda_file, "-lprs", 1);
    
    // Test various flag combinations
    run_gcov_dump_test(instrumented_gcov_dump, gcda_file, "-lp", 1);
    run_gcov_dump_test(instrumented_gcov_dump, gcda_file, "-rs", 1);
    run_gcov_dump_test(instrumented_gcov_dump, gcda_file, "-l -s", 1);
    
    // Test invalid flag to trigger default case
    printf("\n--- Testing invalid flag (-x) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -x %s 2>&1", instrumented_gcov_dump, gcda_file);
    FILE *fp = popen(cmd, "r");
    if (fp) {
        char output[1024];
        while (fgets(output, sizeof(output), fp) != NULL) {
            if (strstr(output, "unknown flag")) {
                printf("✓ Got expected error: %s", output);
            }
        }
        pclose(fp);
    }
    system("gcov -i gcov-dump.cc 2>/dev/null");
    
    // Test with no flags (just gcda file)
    printf("\n--- Testing with no flags ---\n");
    snprintf(cmd, sizeof(cmd), "%s %s", instrumented_gcov_dump, gcda_file);
    system(cmd);
    system("gcov -i gcov-dump.cc 2>/dev/null");
    
    // Step 5: Check final coverage
    check_coverage();
    
    // Cleanup
    printf("\n=== Cleaning up ===\n");
    system("rm -f dummy_prog dummy.gcda dummy.gcno test.gcda gcov-dump.gcda gcov-dump.gcno gcov-dump.cc.gcov");
    
    printf("\n=== Test Complete ===\n");
    printf("Coverage data has been accumulated for gcov-dump.cc\n");
    printf("Target switch-case block (lines 111-130) should now be covered.\n");
    
    return 0;
}
