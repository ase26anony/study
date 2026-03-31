/**
 * test_gcov_dump_switches.c
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
#define TEMP_FILENAME "test_gcov_dump_coverage"

/**
 * Creates a minimal C source file with coverage instrumentation potential
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
 * Executes a system command and returns success/failure
 */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int result = system(cmd);
    if (result != 0) {
        printf("Command returned non-zero: %d\n", result);
    }
    return (result == 0);
}

/**
 * Executes a command and captures stderr to check for specific output
 * Returns 1 if the expected string is found in stderr
 */
int execute_and_check_stderr(const char *cmd, const char *expected_str) {
    char full_cmd[MAX_CMD_LEN];
    char buffer[256];
    int found = 0;
    
    // Redirect stderr to stdout and pipe it
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    printf("Executing (checking stderr): %s\n", cmd);
    
    FILE *pipe = popen(full_cmd, "r");
    if (!pipe) {
        perror("popen failed");
        return 0;
    }
    
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        if (strstr(buffer, expected_str) != NULL) {
            found = 1;
            printf("Found expected string in stderr: %s", buffer);
        }
    }
    
    pclose(pipe);
    return found;
}

/**
 * Checks if a file exists
 */
int file_exists(const char *filename) {
    struct stat st;
    return (stat(filename, &st) == 0);
}

/**
 * Cleans up temporary files
 */
void cleanup_files(void) {
    char cmd[MAX_CMD_LEN];
    
    // Remove all generated files
    const char *files[] = {
        TEMP_FILENAME ".c",
        TEMP_FILENAME,
        TEMP_FILENAME ".gcno",
        TEMP_FILENAME ".gcda",
        TEMP_FILENAME ".gcov",
        NULL
    };
    
    for (int i = 0; files[i] != NULL; i++) {
        if (file_exists(files[i])) {
            snprintf(cmd, sizeof(cmd), "rm -f %s", files[i]);
            system(cmd);
        }
    }
}

int main(int argc, char *argv[]) {
    char cmd[MAX_CMD_LEN];
    char source_file[MAX_CMD_LEN];
    char gcda_file[MAX_CMD_LEN];
    int success = 1;
    
    // Set up filenames
    snprintf(source_file, sizeof(source_file), "%s.c", TEMP_FILENAME);
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", TEMP_FILENAME);
    
    printf("=== Starting gcov-dump switch coverage test ===\n\n");
    
    // Clean up any existing files first
    cleanup_files();
    
    // Step 1: Create test source file
    printf("1. Creating test source file: %s\n", source_file);
    if (!create_test_source(source_file)) {
        fprintf(stderr, "Failed to create test source file\n");
        return 1;
    }
    
    // Step 2: Compile with coverage instrumentation
    printf("\n2. Compiling with coverage flags\n");
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s",
             source_file, TEMP_FILENAME);
    
    if (!execute_command(cmd)) {
        fprintf(stderr, "Compilation failed\n");
        cleanup_files();
        return 1;
    }
    
    // Verify .gcno file was created
    char gcno_file[MAX_CMD_LEN];
    snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", TEMP_FILENAME);
    if (!file_exists(gcno_file)) {
        fprintf(stderr, ".gcno file not created\n");
        cleanup_files();
        return 1;
    }
    
    // Step 3: Execute the test program to generate .gcda file
    printf("\n3. Executing test program to generate .gcda file\n");
    snprintf(cmd, sizeof(cmd), "./%s", TEMP_FILENAME);
    if (!execute_command(cmd)) {
        fprintf(stderr, "Test program execution failed\n");
        cleanup_files();
        return 1;
    }
    
    // Verify .gcda file was created
    if (!file_exists(gcda_file)) {
        fprintf(stderr, ".gcda file not created\n");
        cleanup_files();
        return 1;
    }
    
    printf("\n4. Testing gcov-dump command-line switches\n");
    printf("===========================================\n");
    
    // Test 1: -h flag (help) - triggers print_usage()
    printf("\nTest 1: -h flag (help)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -h");
    execute_command(cmd);
    
    // Test 2: -v flag (version) - triggers print_version()
    printf("\nTest 2: -v flag (version)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -v");
    execute_command(cmd);
    
    // Test 3: -l flag with .gcda file - sets flag_dump_contents
    printf("\nTest 3: -l flag (dump contents)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_file);
    execute_command(cmd);
    
    // Test 4: -p flag with .gcda file - sets flag_dump_positions
    printf("\nTest 4: -p flag (dump positions)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -p %s", gcda_file);
    execute_command(cmd);
    
    // Test 5: -r flag with .gcda file - sets flag_dump_raw
    printf("\nTest 5: -r flag (dump raw)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -r %s", gcda_file);
    execute_command(cmd);
    
    // Test 6: -s flag with .gcda file - sets flag_dump_stable
    printf("\nTest 6: -s flag (dump stable)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -s %s", gcda_file);
    execute_command(cmd);
    
    // Test 7: Combined flags - tests multiple flag processing
    printf("\nTest 7: Combined flags (-l -p)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -p %s", gcda_file);
    execute_command(cmd);
    
    // Test 8: Invalid flag - triggers default case and fprintf
    printf("\nTest 8: Invalid flag (should trigger 'unknown flag' error)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X %s", gcda_file);
    int found_error = execute_and_check_stderr(cmd, "unknown flag");
    
    if (found_error) {
        printf("✓ Successfully triggered 'unknown flag' error (default case)\n");
    } else {
        printf("✗ Did not find expected 'unknown flag' error\n");
        success = 0;
    }
    
    // Additional test: Try with .gcno file as well
    printf("\nTest 9: Testing with .gcno file\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcno_file);
    execute_command(cmd);
    
    // Additional test: Try invalid flag without file argument
    printf("\nTest 10: Invalid flag without file argument\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -Z");
    found_error = execute_and_check_stderr(cmd, "unknown flag");
    
    if (found_error) {
        printf("✓ Successfully triggered 'unknown flag' error\n");
    }
    
    // Cleanup
    printf("\n5. Cleaning up temporary files\n");
    cleanup_files();
    
    printf("\n=== Test completed ===\n");
    printf("All target switch cases in gcov-dump.cc should have been executed.\n");
    
    return success ? 0 : 1;
}
