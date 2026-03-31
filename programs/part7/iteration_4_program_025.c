/**
 * test_gcov_dump_switches.c
 * 
 * Test driver to exercise specific command-line switch cases in gcov-dump.
 * Targets lines 111-130 of gcov-dump.cc:
 * - Help flag (-h)
 * - Version flag (-v)
 * - Dump flags (-l, -p, -r, -s)
 * - Invalid flag (default case)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_PATH 1024
#define MAX_CMD 2048

/**
 * Creates a temporary C source file with coverage instrumentation.
 * Returns 0 on success, -1 on failure.
 */
static int create_coverage_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create source file");
        return -1;
    }
    
    fprintf(fp, "/* Test program for gcov-dump coverage */\n");
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
 * Compiles the source file with coverage instrumentation.
 * Returns 0 on success, -1 on failure.
 */
static int compile_with_coverage(const char *source_file, const char *binary_file) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s 2>&1",
             source_file, binary_file);
    
    printf("Compiling: %s\n", cmd);
    int ret = system(cmd);
    
    if (ret != 0) {
        fprintf(stderr, "Compilation failed\n");
        return -1;
    }
    
    return 0;
}

/**
 * Executes the binary to generate .gcda file.
 * Returns 0 on success, -1 on failure.
 */
static int run_coverage_binary(const char *binary_file) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "./%s", binary_file);
    
    printf("Running: %s\n", cmd);
    int ret = system(cmd);
    
    if (ret != 0) {
        fprintf(stderr, "Execution failed\n");
        return -1;
    }
    
    return 0;
}

/**
 * Checks if a file exists.
 * Returns 1 if exists, 0 otherwise.
 */
static int file_exists(const char *filename) {
    struct stat st;
    return stat(filename, &st) == 0;
}

/**
 * Runs gcov-dump with specified flags and captures stderr.
 * Returns 1 if "unknown flag" message found, 0 otherwise.
 */
static int run_gcov_dump_with_capture(const char *flags, const char *gcda_file) {
    char cmd[MAX_CMD];
    char buffer[256];
    int found_unknown = 0;
    
    if (gcda_file) {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s 2>&1", flags, gcda_file);
    } else {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s 2>&1", flags);
    }
    
    printf("\nExecuting: %s\n", cmd);
    
    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        perror("popen failed");
        return 0;
    }
    
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        printf("Output: %s", buffer);
        if (strstr(buffer, "unknown flag")) {
            found_unknown = 1;
        }
    }
    
    pclose(pipe);
    return found_unknown;
}

/**
 * Runs gcov-dump with system() call (no output capture).
 */
static void run_gcov_dump_simple(const char *flags, const char *gcda_file) {
    char cmd[MAX_CMD];
    
    if (gcda_file) {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s", flags, gcda_file);
    } else {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s", flags);
    }
    
    printf("\nExecuting: %s\n", cmd);
    system(cmd);
}

/**
 * Cleans up temporary files.
 */
static void cleanup_files(const char *base_name) {
    char cmd[MAX_CMD];
    
    // Remove all files with the base name
    const char *extensions[] = {".c", "", ".gcda", ".gcno", ".o", NULL};
    
    for (int i = 0; extensions[i] != NULL; i++) {
        char filename[MAX_PATH];
        if (strlen(extensions[i]) > 0) {
            snprintf(filename, sizeof(filename), "%s%s", base_name, extensions[i]);
        } else {
            snprintf(filename, sizeof(filename), "%s", base_name);
        }
        
        if (file_exists(filename)) {
            printf("Removing: %s\n", filename);
            remove(filename);
        }
    }
}

int main(int argc, char *argv[]) {
    char source_file[MAX_PATH];
    char binary_file[MAX_PATH];
    char gcda_file[MAX_PATH];
    
    // Use a unique base name for temporary files
    const char *base_name = "test_gcov_dump_coverage";
    
    snprintf(source_file, sizeof(source_file), "%s.c", base_name);
    snprintf(binary_file, sizeof(binary_file), "%s", base_name);
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", base_name);
    
    printf("=== Generating coverage data ===\n");
    
    // Step 1: Create test source file
    if (create_coverage_source(source_file) != 0) {
        fprintf(stderr, "Failed to create source file\n");
        return 1;
    }
    
    // Step 2: Compile with coverage
    if (compile_with_coverage(source_file, binary_file) != 0) {
        cleanup_files(base_name);
        return 1;
    }
    
    // Step 3: Run to generate .gcda file
    if (run_coverage_binary(binary_file) != 0) {
        cleanup_files(base_name);
        return 1;
    }
    
    // Verify .gcda file was created
    if (!file_exists(gcda_file)) {
        fprintf(stderr, "No .gcda file generated: %s\n", gcda_file);
        cleanup_files(base_name);
        return 1;
    }
    
    printf("\n=== Testing gcov-dump switches ===\n");
    
    // Test 1: Help flag (-h) - triggers print_usage()
    printf("\n--- Test 1: Help flag (-h) ---");
    run_gcov_dump_simple("-h", NULL);
    
    // Test 2: Version flag (-v) - triggers print_version()
    printf("\n--- Test 2: Version flag (-v) ---");
    run_gcov_dump_simple("-v", NULL);
    
    // Test 3: Dump contents flag (-l) - sets flag_dump_contents
    printf("\n--- Test 3: Dump contents flag (-l) ---");
    run_gcov_dump_simple("-l", gcda_file);
    
    // Test 4: Dump positions flag (-p) - sets flag_dump_positions
    printf("\n--- Test 4: Dump positions flag (-p) ---");
    run_gcov_dump_simple("-p", gcda_file);
    
    // Test 5: Dump raw flag (-r) - sets flag_dump_raw
    printf("\n--- Test 5: Dump raw flag (-r) ---");
    run_gcov_dump_simple("-r", gcda_file);
    
    // Test 6: Dump stable flag (-s) - sets flag_dump_stable
    printf("\n--- Test 6: Dump stable flag (-s) ---");
    run_gcov_dump_simple("-s", gcda_file);
    
    // Test 7: Combined flags (-l -p)
    printf("\n--- Test 7: Combined flags (-l -p) ---");
    run_gcov_dump_simple("-l -p", gcda_file);
    
    // Test 8: Invalid flag (-X) - triggers default case and fprintf
    printf("\n--- Test 8: Invalid flag (-X) ---");
    int found = run_gcov_dump_with_capture("-X", gcda_file);
    
    if (found) {
        printf("SUCCESS: 'unknown flag' message detected (default case triggered)\n");
    } else {
        printf("WARNING: 'unknown flag' message not detected\n");
    }
    
    // Test 9: Another invalid flag (-z)
    printf("\n--- Test 9: Another invalid flag (-z) ---");
    found = run_gcov_dump_with_capture("-z", gcda_file);
    
    if (found) {
        printf("SUCCESS: 'unknown flag' message detected (default case triggered)\n");
    } else {
        printf("WARNING: 'unknown flag' message not detected\n");
    }
    
    // Test 10: Flag with no argument (should show usage or error)
    printf("\n--- Test 10: gcov-dump with no arguments ---");
    run_gcov_dump_simple("", NULL);
    
    // Cleanup
    printf("\n=== Cleaning up ===\n");
    cleanup_files(base_name);
    
    printf("\n=== All tests completed ===\n");
    printf("The following gcov-dump switch cases should have been executed:\n");
    printf("  - case 'h': print_usage()\n");
    printf("  - case 'v': print_version()\n");
    printf("  - case 'l': flag_dump_contents = 1\n");
    printf("  - case 'p': flag_dump_positions = 1\n");
    printf("  - case 'r': flag_dump_raw = 1\n");
    printf("  - case 's': flag_dump_stable = 1\n");
    printf("  - default: fprintf(stderr, \"unknown flag\")\n");
    
    return 0;
}
