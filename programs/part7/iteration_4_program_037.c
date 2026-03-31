/**
 * test_gcov_dump_switches.c
 * 
 * Test driver to exercise the command-line switch cases in gcov-dump.cc
 * Specifically targets lines 111-130:
 * - Cases for flags: h, v, l, p, r, s
 * - Default case for invalid flags
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
 * Creates a temporary C source file with coverage instrumentation
 * Returns 0 on success, -1 on failure
 */
int create_coverage_test_file(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return -1;
    }
    
    fprintf(fp, "/* Test file for gcov-dump coverage */\n");
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
 * Compiles the test program with coverage instrumentation
 */
int compile_with_coverage(const char *source, const char *binary) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 %s -o %s 2>&1",
             source, binary);
    
    printf("Compiling: %s\n", cmd);
    int ret = system(cmd);
    
    if (ret != 0) {
        fprintf(stderr, "Compilation failed\n");
        return -1;
    }
    
    return 0;
}

/**
 * Executes the test program to generate .gcda file
 */
int run_coverage_program(const char *binary) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "./%s 2>&1", binary);
    
    printf("Running: %s\n", cmd);
    int ret = system(cmd);
    
    if (ret != 0) {
        fprintf(stderr, "Execution failed\n");
        return -1;
    }
    
    return 0;
}

/**
 * Checks if a file exists
 */
int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

/**
 * Runs gcov-dump with specified flags and captures output
 * Returns 1 if "unknown flag" message found, 0 otherwise
 */
int run_gcov_dump(const char *flags, const char *gcda_file, int capture_stderr) {
    char cmd[MAX_CMD];
    char result_path[MAX_PATH];
    
    if (gcda_file) {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s 2>&1", flags, gcda_file);
    } else {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s 2>&1", flags);
    }
    
    printf("\nExecuting: %s\n", cmd);
    
    if (capture_stderr) {
        // Use popen to capture stderr for invalid flag detection
        FILE *pipe = popen(cmd, "r");
        if (!pipe) {
            perror("popen failed");
            return 0;
        }
        
        char buffer[256];
        int found_unknown = 0;
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            printf("Output: %s", buffer);
            if (strstr(buffer, "unknown flag")) {
                found_unknown = 1;
            }
        }
        
        pclose(pipe);
        return found_unknown;
    } else {
        // Just execute without capturing
        int ret = system(cmd);
        printf("Return code: %d\n", ret);
        return 0;
    }
}

/**
 * Clean up temporary files
 */
void cleanup_files(const char *base_name) {
    char cmd[MAX_CMD];
    
    // Remove all generated files
    const char *extensions[] = {".c", "", ".gcda", ".gcno", ".o", NULL};
    
    for (int i = 0; extensions[i] != NULL; i++) {
        char path[MAX_PATH];
        if (extensions[i][0] == '\0') {
            snprintf(path, sizeof(path), "%s", base_name); // binary
        } else {
            snprintf(path, sizeof(path), "%s%s", base_name, extensions[i]);
        }
        
        if (file_exists(path)) {
            printf("Removing: %s\n", path);
            unlink(path);
        }
    }
}

int main(int argc, char *argv[]) {
    int ret = 0;
    char source_file[MAX_PATH];
    char binary_file[MAX_PATH];
    char gcda_file[MAX_PATH];
    
    // Use unique base name to avoid collisions
    const char *base_name = "test_gcov_dump_coverage";
    
    snprintf(source_file, sizeof(source_file), "%s.c", base_name);
    snprintf(binary_file, sizeof(binary_file), "%s", base_name);
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", base_name);
    
    printf("=== Starting gcov-dump switch coverage test ===\n");
    
    // Step 1: Create test source file
    printf("\n1. Creating test source file: %s\n", source_file);
    if (create_coverage_test_file(source_file) != 0) {
        fprintf(stderr, "Failed to create source file\n");
        return 1;
    }
    
    // Step 2: Compile with coverage
    printf("\n2. Compiling with coverage instrumentation\n");
    if (compile_with_coverage(source_file, binary_file) != 0) {
        cleanup_files(base_name);
        return 1;
    }
    
    // Step 3: Run to generate .gcda file
    printf("\n3. Running program to generate .gcda file\n");
    if (run_coverage_program(binary_file) != 0) {
        cleanup_files(base_name);
        return 1;
    }
    
    // Verify .gcda file was created
    if (!file_exists(gcda_file)) {
        fprintf(stderr, "Failed to generate .gcda file\n");
        cleanup_files(base_name);
        return 1;
    }
    printf("Generated: %s\n", gcda_file);
    
    // Step 4: Test gcov-dump with various flags
    printf("\n4. Testing gcov-dump command-line switches\n");
    
    // 4.1 Test -h flag (help) - triggers print_usage()
    printf("\n--- Testing -h flag (help) ---\n");
    run_gcov_dump("-h", NULL, 0);
    
    // 4.2 Test -v flag (version) - triggers print_version()
    printf("\n--- Testing -v flag (version) ---\n");
    run_gcov_dump("-v", NULL, 0);
    
    // 4.3 Test -l flag (dump contents) - sets flag_dump_contents = 1
    printf("\n--- Testing -l flag (dump contents) ---\n");
    run_gcov_dump("-l", gcda_file, 0);
    
    // 4.4 Test -p flag (dump positions) - sets flag_dump_positions = 1
    printf("\n--- Testing -p flag (dump positions) ---\n");
    run_gcov_dump("-p", gcda_file, 0);
    
    // 4.5 Test -r flag (dump raw) - sets flag_dump_raw = 1
    printf("\n--- Testing -r flag (dump raw) ---\n");
    run_gcov_dump("-r", gcda_file, 0);
    
    // 4.6 Test -s flag (dump stable) - sets flag_dump_stable = 1
    printf("\n--- Testing -s flag (dump stable) ---\n");
    run_gcov_dump("-s", gcda_file, 0);
    
    // 4.7 Test combined flags
    printf("\n--- Testing combined flags (-l -p) ---\n");
    run_gcov_dump("-l -p", gcda_file, 0);
    
    // 4.8 Test invalid flag - triggers default case and fprintf
    printf("\n--- Testing invalid flag (-X) ---\n");
    int found_unknown = run_gcov_dump("-X", gcda_file, 1);
    
    if (found_unknown) {
        printf("SUCCESS: 'unknown flag' message detected (default case triggered)\n");
    } else {
        printf("WARNING: 'unknown flag' message not found\n");
        ret = 1;
    }
    
    // Additional invalid flag tests
    printf("\n--- Testing another invalid flag (-z) ---\n");
    found_unknown = run_gcov_dump("-z", gcda_file, 1);
    
    if (found_unknown) {
        printf("SUCCESS: 'unknown flag' message detected for -z\n");
    }
    
    // Test with no flags but with file (should also parse)
    printf("\n--- Testing with just filename (no flags) ---\n");
    run_gcov_dump("", gcda_file, 0);
    
    // Step 5: Cleanup
    printf("\n5. Cleaning up temporary files\n");
    cleanup_files(base_name);
    
    printf("\n=== Test completed ===\n");
    printf("All target switch cases in gcov-dump.cc should have been executed:\n");
    printf("  - Case 'h': print_usage()\n");
    printf("  - Case 'v': print_version()\n");
    printf("  - Case 'l': flag_dump_contents = 1\n");
    printf("  - Case 'p': flag_dump_positions = 1\n");
    printf("  - Case 'r': flag_dump_raw = 1\n");
    printf("  - Case 's': flag_dump_stable = 1\n");
    printf("  - Default case: fprintf(stderr, \"unknown flag\")\n");
    
    return ret;
}
