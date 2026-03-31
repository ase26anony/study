/**
 * test_gcov_dump_switch.c
 * 
 * Test driver to exercise the command-line switch cases in gcov-dump.cc
 * Specifically targets lines 111-130 for coverage.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define TEMP_FILENAME "test_gcov_dump_switch"

/**
 * Creates a simple C source file with coverage instrumentation potential
 */
int create_test_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return 0;
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
    return 1;
}

/**
 * Compiles the test source with coverage instrumentation
 */
int compile_with_coverage(const char *source_file, const char *output_file) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s 2>/dev/null",
             source_file, output_file);
    
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Compilation failed. Make sure gcc is installed.\n");
        return 0;
    }
    
    // Check if binary was created
    struct stat st;
    if (stat(output_file, &st) != 0) {
        fprintf(stderr, "Binary not created after compilation\n");
        return 0;
    }
    
    return 1;
}

/**
 * Executes the test program to generate .gcda file
 */
int generate_gcda(const char *program) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "./%s >/dev/null 2>&1", program);
    
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Execution of test program failed\n");
        return 0;
    }
    
    // Check if .gcda file was created
    char gcda_file[MAX_CMD_LEN];
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", TEMP_FILENAME);
    
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, ".gcda file not generated: %s\n", gcda_file);
        return 0;
    }
    
    return 1;
}

/**
 * Invokes gcov-dump with a specific flag and captures output
 * Returns 1 if command executed (regardless of gcov-dump exit status)
 */
int invoke_gcov_dump(const char *flag, const char *input_file, int capture_stderr) {
    char cmd[MAX_CMD_LEN];
    char output[1024];
    FILE *fp;
    int found_error = 0;
    
    if (input_file && input_file[0]) {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s 2>&1", flag, input_file);
    } else {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s 2>&1", flag);
    }
    
    printf("Executing: %s\n", cmd);
    
    if (capture_stderr) {
        fp = popen(cmd, "r");
        if (!fp) {
            perror("popen failed");
            return 0;
        }
        
        // Read output to check for "unknown flag" message
        while (fgets(output, sizeof(output), fp) != NULL) {
            if (strstr(output, "unknown flag")) {
                printf("  -> Successfully triggered 'default' case with message: %s", output);
                found_error = 1;
            }
        }
        
        pclose(fp);
        
        if (!found_error && capture_stderr) {
            printf("  -> Warning: 'unknown flag' message not found in output\n");
        }
    } else {
        // Just execute without capturing output
        int ret = system(cmd);
        if (ret != 0) {
            printf("  -> Command returned non-zero: %d\n", ret);
        }
    }
    
    return 1;
}

/**
 * Cleans up temporary files
 */
void cleanup_files(void) {
    char cmd[MAX_CMD_LEN];
    
    // Remove source file
    snprintf(cmd, sizeof(cmd), "rm -f %s.c", TEMP_FILENAME);
    system(cmd);
    
    // Remove binary
    snprintf(cmd, sizeof(cmd), "rm -f %s", TEMP_FILENAME);
    system(cmd);
    
    // Remove coverage files
    snprintf(cmd, sizeof(cmd), "rm -f %s.gcda %s.gcno %s.c.gcov", 
             TEMP_FILENAME, TEMP_FILENAME, TEMP_FILENAME);
    system(cmd);
}

int main(void) {
    char source_file[MAX_CMD_LEN];
    char gcda_file[MAX_CMD_LEN];
    
    printf("=== Test Driver for gcov-dump Switch Cases ===\n\n");
    
    // Create filenames
    snprintf(source_file, sizeof(source_file), "%s.c", TEMP_FILENAME);
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", TEMP_FILENAME);
    
    // Step 1: Create test source file
    printf("1. Creating test source file: %s\n", source_file);
    if (!create_test_source(source_file)) {
        fprintf(stderr, "Failed to create test source\n");
        return 1;
    }
    
    // Step 2: Compile with coverage
    printf("2. Compiling with coverage instrumentation\n");
    if (!compile_with_coverage(source_file, TEMP_FILENAME)) {
        cleanup_files();
        return 1;
    }
    
    // Step 3: Execute to generate .gcda file
    printf("3. Executing test program to generate .gcda file\n");
    if (!generate_gcda(TEMP_FILENAME)) {
        cleanup_files();
        return 1;
    }
    
    printf("4. .gcda file generated: %s\n\n", gcda_file);
    
    // Step 4: Invoke gcov-dump with various flags to trigger switch cases
    
    printf("=== Triggering gcov-dump switch cases ===\n\n");
    
    // Case 'h': print_usage()
    printf("A. Testing '-h' flag (should trigger print_usage()):\n");
    invoke_gcov_dump("-h", NULL, 0);
    printf("\n");
    
    // Case 'v': print_version()
    printf("B. Testing '-v' flag (should trigger print_version()):\n");
    invoke_gcov_dump("-v", NULL, 0);
    printf("\n");
    
    // Case 'l': flag_dump_contents = 1
    printf("C. Testing '-l' flag (should set flag_dump_contents):\n");
    invoke_gcov_dump("-l", gcda_file, 0);
    printf("\n");
    
    // Case 'p': flag_dump_positions = 1
    printf("D. Testing '-p' flag (should set flag_dump_positions):\n");
    invoke_gcov_dump("-p", gcda_file, 0);
    printf("\n");
    
    // Case 'r': flag_dump_raw = 1
    printf("E. Testing '-r' flag (should set flag_dump_raw):\n");
    invoke_gcov_dump("-r", gcda_file, 0);
    printf("\n");
    
    // Case 's': flag_dump_stable = 1
    printf("F. Testing '-s' flag (should set flag_dump_stable):\n");
    invoke_gcov_dump("-s", gcda_file, 0);
    printf("\n");
    
    // Combined flags: -l -p
    printf("G. Testing combined flags '-l -p':\n");
    invoke_gcov_dump("-l -p", gcda_file, 0);
    printf("\n");
    
    // Default case: invalid flag
    printf("H. Testing invalid flag '-X' (should trigger default case):\n");
    invoke_gcov_dump("-X", gcda_file, 1);
    printf("\n");
    
    // Additional invalid flag test without input file
    printf("I. Testing invalid flag '-z' without input file:\n");
    invoke_gcov_dump("-z", NULL, 1);
    printf("\n");
    
    // Cleanup
    printf("=== Cleaning up temporary files ===\n");
    cleanup_files();
    
    printf("\nTest completed. All target switch cases should have been executed.\n");
    printf("Check gcov-dump's coverage report to verify lines 111-130 were hit.\n");
    
    return 0;
}
