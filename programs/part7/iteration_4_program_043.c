/**
 * test_gcov_dump_switches.c
 * 
 * Test driver to exercise the command-line parsing switch cases in gcov-dump.cc
 * Specifically targets lines 111-130 covering flags: h, v, l, p, r, s, and invalid flags
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define TEMP_SOURCE_FILENAME "test_coverage_tmp.c"
#define TEMP_BINARY_FILENAME "test_coverage_tmp"
#define TEMP_GCDA_FILENAME "test_coverage_tmp.gcda"
#define TEMP_GCNO_FILENAME "test_coverage_tmp.gcno"

/* Simple test program source code to generate coverage data */
const char *test_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    int i, sum = 0;\n"
"    for (i = 0; i < 10; i++) {\n"
"        sum += i;\n"
"    }\n"
"    printf(\"Sum: %d\\n\", sum);\n"
"    return 0;\n"
"}\n";

/**
 * Create a temporary C source file for coverage testing
 */
int create_test_source() {
    FILE *fp = fopen(TEMP_SOURCE_FILENAME, "w");
    if (!fp) {
        fprintf(stderr, "Failed to create source file: %s\n", strerror(errno));
        return 0;
    }
    fputs(test_source, fp);
    fclose(fp);
    return 1;
}

/**
 * Compile the test program with coverage instrumentation
 */
int compile_with_coverage() {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s 2>/dev/null",
             TEMP_SOURCE_FILENAME, TEMP_BINARY_FILENAME);
    
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Compilation failed (command: %s)\n", cmd);
        return 0;
    }
    
    /* Check if .gcno file was created */
    struct stat st;
    if (stat(TEMP_GCNO_FILENAME, &st) != 0) {
        fprintf(stderr, "No .gcno file generated\n");
        return 0;
    }
    
    return 1;
}

/**
 * Execute the test program to generate .gcda file
 */
int generate_gcda() {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "./%s >/dev/null 2>&1", TEMP_BINARY_FILENAME);
    
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Execution failed (command: %s)\n", cmd);
        return 0;
    }
    
    /* Check if .gcda file was created */
    struct stat st;
    if (stat(TEMP_GCDA_FILENAME, &st) != 0) {
        fprintf(stderr, "No .gcda file generated\n");
        return 0;
    }
    
    return 1;
}

/**
 * Execute gcov-dump with given arguments and capture output if needed
 * Returns 1 if command executed (regardless of gcov-dump exit status)
 */
int run_gcov_dump(const char *args, int capture_stderr) {
    char cmd[MAX_CMD_LEN];
    FILE *fp;
    char buffer[256];
    int found_error = 0;
    
    if (capture_stderr) {
        /* Capture stderr to check for error messages */
        snprintf(cmd, sizeof(cmd), "gcov-dump %s 2>&1", args);
        fp = popen(cmd, "r");
        if (!fp) {
            fprintf(stderr, "Failed to run command: %s\n", cmd);
            return 0;
        }
        
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            /* Check for the specific error message from the default case */
            if (strstr(buffer, "unknown flag")) {
                found_error = 1;
            }
        }
        
        pclose(fp);
        
        if (found_error) {
            printf("✓ Successfully triggered 'unknown flag' error for args: %s\n", args);
        }
    } else {
        /* Just execute without capturing output */
        snprintf(cmd, sizeof(cmd), "gcov-dump %s >/dev/null 2>&1", args);
        int ret = system(cmd);
        if (ret != 0) {
            /* Don't fail for invalid flag case - that's expected */
            if (strstr(args, "-X") == NULL) {
                fprintf(stderr, "Command failed (exit %d): %s\n", ret, cmd);
            }
        }
    }
    
    return 1;
}

/**
 * Clean up temporary files
 */
void cleanup() {
    char *files[] = {
        TEMP_SOURCE_FILENAME,
        TEMP_BINARY_FILENAME,
        TEMP_GCDA_FILENAME,
        TEMP_GCNO_FILENAME,
        NULL
    };
    
    for (int i = 0; files[i] != NULL; i++) {
        if (unlink(files[i]) == 0) {
            printf("Cleaned up: %s\n", files[i]);
        }
    }
}

int main() {
    printf("=== Testing gcov-dump command-line switches ===\n\n");
    
    /* Step 1: Create coverage data file */
    printf("1. Creating test coverage data...\n");
    if (!create_test_source()) {
        fprintf(stderr, "Failed to create test source\n");
        return 1;
    }
    
    if (!compile_with_coverage()) {
        fprintf(stderr, "Failed to compile with coverage\n");
        cleanup();
        return 1;
    }
    
    if (!generate_gcda()) {
        fprintf(stderr, "Failed to generate .gcda file\n");
        cleanup();
        return 1;
    }
    
    printf("✓ Coverage data created: %s\n\n", TEMP_GCDA_FILENAME);
    
    /* Step 2: Test individual flags */
    printf("2. Testing individual command-line flags:\n");
    
    /* Test -h flag (help) - triggers print_usage() */
    printf("  Testing -h flag (help)...\n");
    run_gcov_dump("-h", 0);
    
    /* Test -v flag (version) - triggers print_version() */
    printf("  Testing -v flag (version)...\n");
    run_gcov_dump("-v", 0);
    
    /* Test -l flag (dump contents) - sets flag_dump_contents = 1 */
    printf("  Testing -l flag (dump contents)...\n");
    run_gcov_dump("-l " TEMP_GCDA_FILENAME, 0);
    
    /* Test -p flag (dump positions) - sets flag_dump_positions = 1 */
    printf("  Testing -p flag (dump positions)...\n");
    run_gcov_dump("-p " TEMP_GCDA_FILENAME, 0);
    
    /* Test -r flag (dump raw) - sets flag_dump_raw = 1 */
    printf("  Testing -r flag (dump raw)...\n");
    run_gcov_dump("-r " TEMP_GCDA_FILENAME, 0);
    
    /* Test -s flag (dump stable) - sets flag_dump_stable = 1 */
    printf("  Testing -s flag (dump stable)...\n");
    run_gcov_dump("-s " TEMP_GCDA_FILENAME, 0);
    
    /* Step 3: Test combined flags */
    printf("\n3. Testing combined flags:\n");
    printf("  Testing -l -p (combined dump)...\n");
    run_gcov_dump("-l -p " TEMP_GCDA_FILENAME, 0);
    
    /* Step 4: Test invalid flag - triggers default case and fprintf */
    printf("\n4. Testing invalid flag (should trigger 'unknown flag' error):\n");
    run_gcov_dump("-X " TEMP_GCDA_FILENAME, 1);
    
    /* Step 5: Additional edge cases */
    printf("\n5. Testing additional cases:\n");
    printf("  Testing multiple invalid flags...\n");
    run_gcov_dump("-X -Y -Z " TEMP_GCDA_FILENAME, 1);
    
    printf("  Testing valid flag after invalid flag...\n");
    run_gcov_dump("-X -l " TEMP_GCDA_FILENAME, 1);
    
    printf("  Testing invalid flag without file argument...\n");
    run_gcov_dump("-X", 1);
    
    /* Cleanup */
    printf("\n6. Cleaning up temporary files...\n");
    cleanup();
    
    printf("\n=== Test complete ===\n");
    printf("All gcov-dump command-line switch cases should have been executed.\n");
    
    return 0;
}
