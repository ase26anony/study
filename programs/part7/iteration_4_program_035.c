/**
 * Test driver for gcov-dump command-line flag coverage
 * Targets specific switch cases in gcov-dump.cc lines 111-130
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define TEMP_FILENAME "test_gcov_dump_coverage"

/* Simple test program source code that will generate coverage data */
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
 * Execute a system command and return its exit status
 */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        printf("Command returned non-zero: %d\n", status);
    }
    return status;
}

/**
 * Execute command and capture stderr to verify error message
 */
int execute_and_check_error(const char *cmd, const char *expected_error) {
    printf("Executing (checking stderr): %s\n", cmd);
    
    /* Use popen to capture stderr (2>&1 redirects stderr to stdout) */
    char full_cmd[MAX_CMD_LEN];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    FILE *fp = popen(full_cmd, "r");
    if (!fp) {
        perror("popen failed");
        return -1;
    }
    
    char buffer[256];
    int found_error = 0;
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, expected_error) != NULL) {
            found_error = 1;
            printf("Found expected error message: %s", buffer);
        }
    }
    
    int status = pclose(fp);
    if (found_error) {
        printf("Successfully triggered default case with error message\n");
    } else {
        printf("Warning: Expected error message not found\n");
    }
    
    return status;
}

/**
 * Check if a file exists
 */
int file_exists(const char *filename) {
    struct stat st;
    return stat(filename, &st) == 0;
}

/**
 * Clean up temporary files
 */
void cleanup_files(void) {
    /* Remove all generated files */
    char cmd[MAX_CMD_LEN];
    
    /* Source and binary files */
    snprintf(cmd, sizeof(cmd), "rm -f %s.c %s", TEMP_FILENAME, TEMP_FILENAME);
    system(cmd);
    
    /* Coverage data files */
    snprintf(cmd, sizeof(cmd), "rm -f %s.gcda %s.gcno", TEMP_FILENAME, TEMP_FILENAME);
    system(cmd);
    
    /* gcov output files */
    snprintf(cmd, sizeof(cmd), "rm -f %s.c.gcov", TEMP_FILENAME);
    system(cmd);
}

int main(int argc, char *argv[]) {
    char cmd[MAX_CMD_LEN];
    int ret = 0;
    
    printf("=== Starting gcov-dump flag coverage test ===\n");
    
    /* Step 1: Create test source file */
    printf("\n1. Creating test source file...\n");
    FILE *src_file = fopen(TEMP_FILENAME ".c", "w");
    if (!src_file) {
        perror("Failed to create source file");
        return 1;
    }
    fprintf(src_file, "%s", test_source);
    fclose(src_file);
    
    /* Step 2: Compile with coverage instrumentation */
    printf("\n2. Compiling with coverage flags...\n");
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s.c -o %s -O0",
             TEMP_FILENAME, TEMP_FILENAME);
    ret = execute_command(cmd);
    if (ret != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup_files();
        return 1;
    }
    
    /* Step 3: Execute the program to generate .gcda file */
    printf("\n3. Executing test program to generate coverage data...\n");
    snprintf(cmd, sizeof(cmd), "./%s", TEMP_FILENAME);
    ret = execute_command(cmd);
    if (ret != 0) {
        fprintf(stderr, "Execution failed\n");
        cleanup_files();
        return 1;
    }
    
    /* Verify .gcda file was created */
    if (!file_exists(TEMP_FILENAME ".gcda")) {
        fprintf(stderr, "Coverage data file not created\n");
        cleanup_files();
        return 1;
    }
    printf("Coverage data file created: %s.gcda\n", TEMP_FILENAME);
    
    /* Step 4: Test gcov-dump with various flags */
    printf("\n4. Testing gcov-dump command-line flags...\n");
    
    /* 4a: Test -h flag (help) - triggers print_usage() */
    printf("\n4a. Testing -h flag (help)...\n");
    execute_command("gcov-dump -h");
    
    /* 4b: Test -v flag (version) - triggers print_version() */
    printf("\n4b. Testing -v flag (version)...\n");
    execute_command("gcov-dump -v");
    
    /* 4c: Test -l flag (dump contents) - sets flag_dump_contents = 1 */
    printf("\n4c. Testing -l flag (dump contents)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s.gcda", TEMP_FILENAME);
    execute_command(cmd);
    
    /* 4d: Test -p flag (dump positions) - sets flag_dump_positions = 1 */
    printf("\n4d. Testing -p flag (dump positions)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -p %s.gcda", TEMP_FILENAME);
    execute_command(cmd);
    
    /* 4e: Test -r flag (dump raw) - sets flag_dump_raw = 1 */
    printf("\n4e. Testing -r flag (dump raw)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -r %s.gcda", TEMP_FILENAME);
    execute_command(cmd);
    
    /* 4f: Test -s flag (dump stable) - sets flag_dump_stable = 1 */
    printf("\n4f. Testing -s flag (dump stable)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -s %s.gcda", TEMP_FILENAME);
    execute_command(cmd);
    
    /* 4g: Test combined flags */
    printf("\n4g. Testing combined flags (-l -p)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -p %s.gcda", TEMP_FILENAME);
    execute_command(cmd);
    
    /* 4h: Test invalid flag - triggers default case and fprintf */
    printf("\n4h. Testing invalid flag (should trigger 'unknown flag' error)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X %s.gcda", TEMP_FILENAME);
    execute_and_check_error(cmd, "unknown flag");
    
    /* Additional invalid flag tests to ensure coverage */
    printf("\n4i. Testing another invalid flag...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -z %s.gcda", TEMP_FILENAME);
    execute_and_check_error(cmd, "unknown flag");
    
    /* Test with no file argument for invalid flag */
    printf("\n4j. Testing invalid flag without file...\n");
    execute_and_check_error("gcov-dump -X", "unknown flag");
    
    /* Step 5: Clean up */
    printf("\n5. Cleaning up temporary files...\n");
    cleanup_files();
    
    printf("\n=== gcov-dump flag coverage test completed ===\n");
    printf("All target switch cases in gcov-dump.cc should have been executed:\n");
    printf("  - Case 'h': print_usage()\n");
    printf("  - Case 'v': print_version()\n");
    printf("  - Case 'l': flag_dump_contents = 1\n");
    printf("  - Case 'p': flag_dump_positions = 1\n");
    printf("  - Case 'r': flag_dump_raw = 1\n");
    printf("  - Case 's': flag_dump_stable = 1\n");
    printf("  - Default case: fprintf(stderr, \"unknown flag\")\n");
    
    return 0;
}
