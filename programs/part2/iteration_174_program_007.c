/**
 * test_gcov_dump_coverage.c
 * 
 * This program tests the gcov-dump utility to ensure coverage of the
 * command-line argument parsing switch-case block (lines 111-130).
 * It builds an instrumented version of gcov-dump, generates test GCOV
 * data files, and executes gcov-dump with various flag combinations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024
#define TEST_DIR "/tmp/gcov_dump_test"

/**
 * Creates a simple C program that will be compiled with coverage
 * instrumentation to generate GCOV data files.
 */
void create_dummy_program() {
    FILE *fp = fopen(TEST_DIR "/dummy.c", "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        exit(1);
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
    printf("Created dummy.c\n");
}

/**
 * Compiles a program with coverage instrumentation.
 * Returns 0 on success, non-zero on failure.
 */
int compile_with_coverage(const char *source, const char *output, const char *extra_flags) {
    char cmd[MAX_PATH * 4];
    snprintf(cmd, sizeof(cmd), "gcc -O0 -fprofile-arcs -ftest-coverage %s %s -o %s", 
             extra_flags ? extra_flags : "", source, output);
    printf("Compiling: %s\n", cmd);
    return system(cmd);
}

/**
 * Builds an instrumented version of gcov-dump.
 * Assumes gcov-dump.cc is in the current directory.
 */
int build_instrumented_gcov_dump() {
    struct stat st;
    
    // Check if gcov-dump.cc exists
    if (stat("gcov-dump.cc", &st) != 0) {
        fprintf(stderr, "Error: gcov-dump.cc not found in current directory\n");
        return -1;
    }
    
    // Try to find libiberty
    char *libiberty_paths[] = {
        "../../libiberty/libiberty.a",
        "../libiberty/libiberty.a",
        "libiberty.a",
        NULL
    };
    
    char *libiberty = NULL;
    for (int i = 0; libiberty_paths[i]; i++) {
        if (stat(libiberty_paths[i], &st) == 0) {
            libiberty = libiberty_paths[i];
            break;
        }
    }
    
    if (!libiberty) {
        fprintf(stderr, "Error: Could not find libiberty.a\n");
        return -1;
    }
    
    // Build command
    char cmd[MAX_PATH * 4];
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include -I../../libiberty "
             "gcov-dump.cc %s -o %s/gcov-dump-instrumented",
             libiberty, TEST_DIR);
    
    printf("Building instrumented gcov-dump: %s\n", cmd);
    return system(cmd);
}

/**
 * Executes a command and returns its exit status.
 */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/**
 * Merges coverage data for gcov-dump.cc
 */
void merge_coverage_data() {
    char cmd[MAX_PATH];
    
    // First, ensure we're in the test directory
    chdir(TEST_DIR);
    
    // Copy any .gcda files to the current directory
    snprintf(cmd, sizeof(cmd), "cp -f *.gcda . 2>/dev/null || true");
    system(cmd);
    
    // Generate coverage info for gcov-dump.cc
    snprintf(cmd, sizeof(cmd), "gcov -i gcov-dump.cc 2>&1");
    system(cmd);
    
    // Go back to original directory
    chdir("..");
}

/**
 * Checks if target lines are covered in the coverage report.
 */
void check_coverage() {
    char cmd[MAX_PATH];
    char line[1024];
    FILE *fp;
    
    chdir(TEST_DIR);
    
    // Generate human-readable coverage report
    snprintf(cmd, sizeof(cmd), "gcov -b gcov-dump.cc > coverage_report.txt 2>&1");
    system(cmd);
    
    // Look for coverage of our target lines
    printf("\n=== Coverage Report Summary ===\n");
    snprintf(cmd, sizeof(cmd), "grep -n -A5 -B5 '111\\|130' gcov-dump.cc.gcov");
    fp = popen(cmd, "r");
    if (fp) {
        while (fgets(line, sizeof(line), fp)) {
            printf("%s", line);
        }
        pclose(fp);
    }
    
    // Check specifically for the switch cases
    printf("\n=== Checking Target Switch Cases ===\n");
    const char *cases[] = {"case 'l':", "case 'p':", "case 'r':", "case 's':", "default:"};
    for (int i = 0; i < 5; i++) {
        snprintf(cmd, sizeof(cmd), "grep -n '%s' gcov-dump.cc.gcov", cases[i]);
        fp = popen(cmd, "r");
        if (fp) {
            if (fgets(line, sizeof(line), fp)) {
                printf("Found: %s", line);
            } else {
                printf("WARNING: Not found: %s\n", cases[i]);
            }
            pclose(fp);
        }
    }
    
    chdir("..");
}

int main(int argc, char *argv[]) {
    int status;
    char cmd[MAX_PATH * 4];
    char dummy_gcda_path[MAX_PATH];
    
    // Create test directory
    printf("Creating test directory: %s\n", TEST_DIR);
    mkdir(TEST_DIR, 0755);
    
    // Create dummy program
    create_dummy_program();
    
    // Build instrumented gcov-dump
    status = build_instrumented_gcov_dump();
    if (status != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    // Compile dummy program with coverage
    snprintf(cmd, sizeof(cmd), "%s/dummy.c", TEST_DIR);
    status = compile_with_coverage(cmd, TEST_DIR "/dummy_prog", "");
    if (status != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return 1;
    }
    
    // Run dummy program to generate .gcda file
    snprintf(cmd, sizeof(cmd), "cd %s && ./dummy_prog > /dev/null", TEST_DIR);
    printf("Generating GCOV data: %s\n", cmd);
    system(cmd);
    
    snprintf(dummy_gcda_path, sizeof(dummy_gcda_path), "%s/dummy.gcda", TEST_DIR);
    
    // Clear any existing coverage data
    snprintf(cmd, sizeof(cmd), "rm -f %s/*.gcda %s/*.gcov", TEST_DIR, TEST_DIR);
    system(cmd);
    
    printf("\n=== Starting gcov-dump flag coverage tests ===\n");
    
    // Test 1: Help flag (-h)
    printf("\nTest 1: Help flag (-h)\n");
    snprintf(cmd, sizeof(cmd), "%s/gcov-dump-instrumented -h", TEST_DIR);
    status = execute_command(cmd);
    if (status != 0) {
        printf("WARNING: -h returned non-zero: %d\n", status);
    }
    merge_coverage_data();
    
    // Test 2: Version flag (-v)
    printf("\nTest 2: Version flag (-v)\n");
    snprintf(cmd, sizeof(cmd), "%s/gcov-dump-instrumented -v", TEST_DIR);
    status = execute_command(cmd);
    if (status != 0) {
        printf("WARNING: -v returned non-zero: %d\n", status);
    }
    merge_coverage_data();
    
    // Test 3: Individual flags with GCOV data file
    const char *flags[] = {"-l", "-p", "-r", "-s"};
    const char *flag_names[] = {"contents", "positions", "raw", "stable"};
    
    for (int i = 0; i < 4; i++) {
        printf("\nTest 3.%d: %s flag (%s)\n", i+1, flag_names[i], flags[i]);
        snprintf(cmd, sizeof(cmd), "%s/gcov-dump-instrumented %s %s", 
                 TEST_DIR, flags[i], dummy_gcda_path);
        status = execute_command(cmd);
        if (status != 0) {
            printf("WARNING: %s returned non-zero: %d\n", flags[i], status);
        }
        merge_coverage_data();
    }
    
    // Test 4: Combined flags (space-separated)
    printf("\nTest 4: Combined flags (space-separated)\n");
    snprintf(cmd, sizeof(cmd), "%s/gcov-dump-instrumented -l -p -r -s %s", 
             TEST_DIR, dummy_gcda_path);
    status = execute_command(cmd);
    if (status != 0) {
        printf("WARNING: Combined flags returned non-zero: %d\n", status);
    }
    merge_coverage_data();
    
    // Test 5: Concatenated flags
    printf("\nTest 5: Concatenated flags (-lprs)\n");
    snprintf(cmd, sizeof(cmd), "%s/gcov-dump-instrumented -lprs %s", 
             TEST_DIR, dummy_gcda_path);
    status = execute_command(cmd);
    if (status != 0) {
        printf("WARNING: Concatenated flags returned non-zero: %d\n", status);
    }
    merge_coverage_data();
    
    // Test 6: Invalid flag (to trigger default case)
    printf("\nTest 6: Invalid flag (-x) to trigger default case\n");
    snprintf(cmd, sizeof(cmd), "%s/gcov-dump-instrumented -x %s 2>&1", 
             TEST_DIR, dummy_gcda_path);
    printf("Executing: %s\n", cmd);
    
    FILE *fp = popen(cmd, "r");
    if (fp) {
        char output[1024];
        int found_error = 0;
        while (fgets(output, sizeof(output), fp)) {
            printf("%s", output);
            if (strstr(output, "unknown flag")) {
                found_error = 1;
            }
        }
        status = pclose(fp);
        
        if (WIFEXITED(status)) {
            status = WEXITSTATUS(status);
        }
        
        if (!found_error) {
            printf("WARNING: Expected 'unknown flag' message not found\n");
        }
        
        if (status == 0) {
            printf("WARNING: Invalid flag should return non-zero exit code\n");
        }
    }
    merge_coverage_data();
    
    // Test 7: Multiple data files with flags
    printf("\nTest 7: Multiple data files with flags\n");
    // Create another dummy.gcda by running dummy_prog again
    snprintf(cmd, sizeof(cmd), "cd %s && ./dummy_prog > /dev/null", TEST_DIR);
    system(cmd);
    
    snprintf(cmd, sizeof(cmd), "%s/gcov-dump-instrumented -lp %s %s/dummy.gcno", 
             TEST_DIR, dummy_gcda_path, TEST_DIR);
    status = execute_command(cmd);
    merge_coverage_data();
    
    // Final coverage check
    printf("\n=== Final Coverage Analysis ===\n");
    check_coverage();
    
    // Cleanup (optional)
    printf("\nTest completed. Test files are in: %s\n", TEST_DIR);
    printf("To clean up: rm -rf %s\n", TEST_DIR);
    
    return 0;
}
