/**
 * test_gcov_dump_switches.c
 * 
 * Test driver to exercise gcov-dump command-line parsing switch cases
 * Specifically targets lines 111-130 in gcov-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define TEMP_FILENAME "test_gcov_dump_switches"

/**
 * Creates a minimal C source file with coverage instrumentation
 * Returns 0 on success, -1 on failure
 */
static int create_test_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return -1;
    }
    
    fprintf(fp, "/* Minimal test program for gcov-dump coverage */\n");
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
 * Returns 0 on success, -1 on failure
 */
static int compile_with_coverage(const char *source_file, const char *binary_file) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s 2>&1",
             source_file, binary_file);
    
    printf("Compiling: %s\n", cmd);
    int status = system(cmd);
    
    if (status != 0) {
        fprintf(stderr, "Compilation failed with status %d\n", status);
        return -1;
    }
    
    return 0;
}

/**
 * Executes the test program to generate .gcda file
 * Returns 0 on success, -1 on failure
 */
static int run_test_program(const char *binary_file) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "./%s", binary_file);
    
    printf("Running test program: %s\n", cmd);
    int status = system(cmd);
    
    if (status != 0) {
        fprintf(stderr, "Test program execution failed with status %d\n", status);
        return -1;
    }
    
    return 0;
}

/**
 * Checks if a file exists
 * Returns 1 if exists, 0 otherwise
 */
static int file_exists(const char *filename) {
    struct stat st;
    return stat(filename, &st) == 0;
}

/**
 * Executes gcov-dump with given arguments and captures output
 * Returns exit status of gcov-dump
 */
static int run_gcov_dump(const char *args, int capture_stderr) {
    char cmd[MAX_CMD_LEN];
    FILE *fp;
    char buffer[256];
    int found_unknown_flag = 0;
    
    snprintf(cmd, sizeof(cmd), "gcov-dump %s 2>&1", args);
    
    printf("\nExecuting: gcov-dump %s\n", args);
    
    if (capture_stderr) {
        fp = popen(cmd, "r");
        if (!fp) {
            perror("popen failed");
            return -1;
        }
        
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("  Output: %s", buffer);
            if (strstr(buffer, "unknown flag")) {
                found_unknown_flag = 1;
            }
        }
        
        int status = pclose(fp);
        
        if (found_unknown_flag) {
            printf("  SUCCESS: Triggered 'unknown flag' error message\n");
        }
        
        return status;
    } else {
        return system(cmd);
    }
}

/**
 * Clean up temporary files
 */
static void cleanup_files(void) {
    char *files_to_remove[] = {
        TEMP_FILENAME ".c",
        TEMP_FILENAME,
        TEMP_FILENAME ".gcda",
        TEMP_FILENAME ".gcno",
        NULL
    };
    
    printf("\nCleaning up temporary files...\n");
    for (int i = 0; files_to_remove[i] != NULL; i++) {
        if (remove(files_to_remove[i]) == 0) {
            printf("  Removed: %s\n", files_to_remove[i]);
        }
    }
}

int main(void) {
    char source_file[256];
    char binary_file[256];
    char gcda_file[256];
    int ret = 0;
    
    printf("=== Test Driver for gcov-dump Switch Cases ===\n");
    
    // Set up filenames
    snprintf(source_file, sizeof(source_file), "%s.c", TEMP_FILENAME);
    snprintf(binary_file, sizeof(binary_file), "%s", TEMP_FILENAME);
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", TEMP_FILENAME);
    
    // Step 1: Create test source file
    printf("\n1. Creating test source file: %s\n", source_file);
    if (create_test_source(source_file) != 0) {
        fprintf(stderr, "Failed to create test source\n");
        return EXIT_FAILURE;
    }
    
    // Step 2: Compile with coverage
    printf("\n2. Compiling with coverage instrumentation\n");
    if (compile_with_coverage(source_file, binary_file) != 0) {
        cleanup_files();
        return EXIT_FAILURE;
    }
    
    // Step 3: Run test program to generate .gcda
    printf("\n3. Running test program to generate .gcda file\n");
    if (run_test_program(binary_file) != 0) {
        cleanup_files();
        return EXIT_FAILURE;
    }
    
    // Verify .gcda file was created
    if (!file_exists(gcda_file)) {
        fprintf(stderr, "ERROR: .gcda file not created: %s\n", gcda_file);
        cleanup_files();
        return EXIT_FAILURE;
    }
    printf("Generated coverage file: %s\n", gcda_file);
    
    // Step 4: Execute gcov-dump with various flags to trigger switch cases
    printf("\n4. Testing gcov-dump command-line parsing\n");
    
    // Case 1: -h flag (help) - triggers print_usage()
    printf("\n--- Testing -h flag (help) ---\n");
    if (run_gcov_dump("-h", 0) != 0) {
        fprintf(stderr, "Warning: gcov-dump -h returned non-zero\n");
    }
    
    // Case 2: -v flag (version) - triggers print_version()
    printf("\n--- Testing -v flag (version) ---\n");
    if (run_gcov_dump("-v", 0) != 0) {
        fprintf(stderr, "Warning: gcov-dump -v returned non-zero\n");
    }
    
    // Case 3: -l flag (dump contents) - sets flag_dump_contents = 1
    printf("\n--- Testing -l flag (dump contents) ---\n");
    char cmd_l[MAX_CMD_LEN];
    snprintf(cmd_l, sizeof(cmd_l), "-l %s", gcda_file);
    if (run_gcov_dump(cmd_l, 0) != 0) {
        fprintf(stderr, "Warning: gcov-dump -l returned non-zero\n");
    }
    
    // Case 4: -p flag (dump positions) - sets flag_dump_positions = 1
    printf("\n--- Testing -p flag (dump positions) ---\n");
    char cmd_p[MAX_CMD_LEN];
    snprintf(cmd_p, sizeof(cmd_p), "-p %s", gcda_file);
    if (run_gcov_dump(cmd_p, 0) != 0) {
        fprintf(stderr, "Warning: gcov-dump -p returned non-zero\n");
    }
    
    // Case 5: -r flag (dump raw) - sets flag_dump_raw = 1
    printf("\n--- Testing -r flag (dump raw) ---\n");
    char cmd_r[MAX_CMD_LEN];
    snprintf(cmd_r, sizeof(cmd_r), "-r %s", gcda_file);
    if (run_gcov_dump(cmd_r, 0) != 0) {
        fprintf(stderr, "Warning: gcov-dump -r returned non-zero\n");
    }
    
    // Case 6: -s flag (dump stable) - sets flag_dump_stable = 1
    printf("\n--- Testing -s flag (dump stable) ---\n");
    char cmd_s[MAX_CMD_LEN];
    snprintf(cmd_s, sizeof(cmd_s), "-s %s", gcda_file);
    if (run_gcov_dump(cmd_s, 0) != 0) {
        fprintf(stderr, "Warning: gcov-dump -s returned non-zero\n");
    }
    
    // Case 7: Combined flags (-l -p) - tests multiple flag parsing
    printf("\n--- Testing combined flags (-l -p) ---\n");
    char cmd_lp[MAX_CMD_LEN];
    snprintf(cmd_lp, sizeof(cmd_lp), "-l -p %s", gcda_file);
    if (run_gcov_dump(cmd_lp, 0) != 0) {
        fprintf(stderr, "Warning: gcov-dump -l -p returned non-zero\n");
    }
    
    // Case 8: Invalid flag (-X) - triggers default case and fprintf
    printf("\n--- Testing invalid flag (-X) ---\n");
    char cmd_invalid[MAX_CMD_LEN];
    snprintf(cmd_invalid, sizeof(cmd_invalid), "-X %s", gcda_file);
    // Capture stderr to verify the "unknown flag" message
    run_gcov_dump(cmd_invalid, 1);
    
    // Additional test: Flag with no argument (should trigger error)
    printf("\n--- Testing flag without required file argument ---\n");
    run_gcov_dump("-l", 1);
    
    // Step 5: Cleanup
    cleanup_files();
    
    printf("\n=== Test completed ===\n");
    printf("All gcov-dump switch cases should have been executed:\n");
    printf("  - Case 'h': print_usage()\n");
    printf("  - Case 'v': print_version()\n");
    printf("  - Case 'l': flag_dump_contents = 1\n");
    printf("  - Case 'p': flag_dump_positions = 1\n");
    printf("  - Case 'r': flag_dump_raw = 1\n");
    printf("  - Case 's': flag_dump_stable = 1\n");
    printf("  - Default case: 'unknown flag' error\n");
    
    return EXIT_SUCCESS;
}
