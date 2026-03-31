/**
 * test_gcov_dump_switches.c
 * 
 * Test driver to exercise the command-line switch cases in gcov-dump.cc
 * Specifically targets lines 111-130 covering flags: h, v, l, p, r, s, and invalid flags
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define TEMP_SOURCE_FILENAME "test_gcov_dump_switches.c"
#define TEMP_BINARY_FILENAME "test_gcov_dump_switches_bin"
#define TEMP_GCDA_FILENAME "test_gcov_dump_switches_bin.gcda"
#define TEMP_GCNO_FILENAME "test_gcov_dump_switches_bin.gcno"

/**
 * Creates a minimal C source file with coverage instrumentation
 * to generate .gcda and .gcno files for gcov-dump to process
 */
int create_coverage_test_source(void) {
    FILE *fp = fopen(TEMP_SOURCE_FILENAME, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return 0;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    int i, sum = 0;\n");
    fprintf(fp, "    /* Loop to generate arc coverage */\n");
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
int compile_with_coverage(void) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s",
             TEMP_SOURCE_FILENAME, TEMP_BINARY_FILENAME);
    
    printf("Compiling: %s\n", cmd);
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Compilation failed with code %d\n", result);
        return 0;
    }
    
    return 1;
}

/**
 * Executes the compiled binary to generate .gcda file
 */
int generate_gcda_file(void) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "./%s", TEMP_BINARY_FILENAME);
    
    printf("Executing: %s\n", cmd);
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Execution failed with code %d\n", result);
        return 0;
    }
    
    // Verify .gcda file was created
    struct stat st;
    if (stat(TEMP_GCDA_FILENAME, &st) != 0) {
        fprintf(stderr, "Failed to create .gcda file\n");
        return 0;
    }
    
    printf("Generated: %s (%ld bytes)\n", TEMP_GCDA_FILENAME, (long)st.st_size);
    return 1;
}

/**
 * Executes gcov-dump with specified flags and captures output
 * Returns 1 if command executed (regardless of gcov-dump's exit code)
 */
int run_gcov_dump(const char *flags, const char *input_file, int capture_stderr) {
    char cmd[MAX_CMD_LEN];
    char buffer[1024];
    FILE *fp;
    int found_unknown_flag = 0;
    
    // Build command
    if (input_file) {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s 2>&1", flags, input_file);
    } else {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s 2>&1", flags);
    }
    
    printf("\nRunning: %s\n", cmd);
    
    if (capture_stderr) {
        // Use popen to capture stderr for checking error messages
        fp = popen(cmd, "r");
        if (!fp) {
            perror("popen failed");
            return 0;
        }
        
        // Read output to check for "unknown flag" message
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            if (strstr(buffer, "unknown flag")) {
                found_unknown_flag = 1;
            }
            // Print output for debugging
            printf("Output: %s", buffer);
        }
        
        int status = pclose(fp);
        printf("Exit code: %d\n", status);
        
        if (found_unknown_flag) {
            printf("SUCCESS: Triggered 'unknown flag' error message\n");
        }
        
        return 1;
    } else {
        // Simple system call without output capture
        int result = system(cmd);
        printf("Exit code: %d\n", result);
        return 1;
    }
}

/**
 * Cleans up temporary files
 */
void cleanup_temp_files(void) {
    char *files_to_remove[] = {
        TEMP_SOURCE_FILENAME,
        TEMP_BINARY_FILENAME,
        TEMP_GCDA_FILENAME,
        TEMP_GCNO_FILENAME,
        NULL
    };
    
    printf("\nCleaning up temporary files...\n");
    for (int i = 0; files_to_remove[i] != NULL; i++) {
        if (remove(files_to_remove[i]) == 0) {
            printf("Removed: %s\n", files_to_remove[i]);
        } else if (errno != ENOENT) {
            perror(files_to_remove[i]);
        }
    }
}

int main(void) {
    printf("=== Test Driver for gcov-dump Switch Cases ===\n");
    printf("Targeting lines 111-130 in gcov-dump.cc\n\n");
    
    // Step 1: Create coverage test source
    printf("1. Creating test source file...\n");
    if (!create_coverage_test_source()) {
        fprintf(stderr, "Failed to create test source\n");
        return EXIT_FAILURE;
    }
    
    // Step 2: Compile with coverage instrumentation
    printf("\n2. Compiling with coverage instrumentation...\n");
    if (!compile_with_coverage()) {
        cleanup_temp_files();
        return EXIT_FAILURE;
    }
    
    // Step 3: Execute to generate .gcda file
    printf("\n3. Generating .gcda file...\n");
    if (!generate_gcda_file()) {
        cleanup_temp_files();
        return EXIT_FAILURE;
    }
    
    // Step 4: Test gcov-dump with various flags
    printf("\n4. Testing gcov-dump command-line switches...\n");
    
    // Test case 1: -h (help) - triggers print_usage()
    printf("\n--- Test Case 1: -h (help) ---\n");
    run_gcov_dump("-h", NULL, 0);
    
    // Test case 2: -v (version) - triggers print_version()
    printf("\n--- Test Case 2: -v (version) ---\n");
    run_gcov_dump("-v", NULL, 0);
    
    // Test case 3: -l (dump contents) with .gcda file
    printf("\n--- Test Case 3: -l (dump contents) ---\n");
    run_gcov_dump("-l", TEMP_GCDA_FILENAME, 0);
    
    // Test case 4: -p (dump positions) with .gcda file
    printf("\n--- Test Case 4: -p (dump positions) ---\n");
    run_gcov_dump("-p", TEMP_GCDA_FILENAME, 0);
    
    // Test case 5: -r (dump raw) with .gcda file
    printf("\n--- Test Case 5: -r (dump raw) ---\n");
    run_gcov_dump("-r", TEMP_GCDA_FILENAME, 0);
    
    // Test case 6: -s (dump stable) with .gcda file
    printf("\n--- Test Case 6: -s (dump stable) ---\n");
    run_gcov_dump("-s", TEMP_GCDA_FILENAME, 0);
    
    // Test case 7: Combined flags -l -p
    printf("\n--- Test Case 7: -l -p (combined flags) ---\n");
    run_gcov_dump("-l -p", TEMP_GCDA_FILENAME, 0);
    
    // Test case 8: Invalid flag -X - triggers default case and fprintf
    printf("\n--- Test Case 8: -X (invalid flag) ---\n");
    run_gcov_dump("-X", TEMP_GCDA_FILENAME, 1);
    
    // Test case 9: Another invalid flag -z
    printf("\n--- Test Case 9: -z (invalid flag) ---\n");
    run_gcov_dump("-z", TEMP_GCDA_FILENAME, 1);
    
    // Test case 10: Multiple flags including invalid
    printf("\n--- Test Case 10: -l -X -p (mixed valid/invalid) ---\n");
    run_gcov_dump("-l -X -p", TEMP_GCDA_FILENAME, 1);
    
    // Cleanup
    cleanup_temp_files();
    
    printf("\n=== Test completed ===\n");
    printf("All gcov-dump switch cases should have been exercised.\n");
    
    return EXIT_SUCCESS;
}
