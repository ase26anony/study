#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_XXXXXX.c"
#define TEMP_BINARY_FILE "test_coverage_XXXXXX"
#define TEMP_GCDA_FILE "test_coverage_XXXXXX.gcda"
#define TEMP_GCNO_FILE "test_coverage_XXXXXX.gcno"

/* Create a unique temporary filename */
char* create_temp_filename(const char* pattern) {
    char* temp = strdup(pattern);
    int fd = mkstemps(temp, strlen(pattern) - 6); /* 6 for XXXXXX */
    if (fd == -1) {
        perror("mkstemps failed");
        free(temp);
        return NULL;
    }
    close(fd);
    return temp;
}

/* Create a simple C source file for coverage testing */
int create_coverage_source(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create source file");
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

/* Execute a command and return its output */
char* execute_command(const char* cmd) {
    char buffer[1024];
    char* result = malloc(1);
    result[0] = '\0';
    
    FILE* fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        free(result);
        return NULL;
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        size_t old_len = strlen(result);
        size_t new_len = old_len + strlen(buffer) + 1;
        result = realloc(result, new_len);
        strcpy(result + old_len, buffer);
    }
    
    pclose(fp);
    return result;
}

/* Execute a command and capture stderr */
char* execute_command_stderr(const char* cmd) {
    char buffer[1024];
    char* result = malloc(1);
    result[0] = '\0';
    
    /* Redirect stderr to stdout for capture */
    char full_cmd[2048];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    FILE* fp = popen(full_cmd, "r");
    if (!fp) {
        perror("popen failed");
        free(result);
        return NULL;
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        size_t old_len = strlen(result);
        size_t new_len = old_len + strlen(buffer) + 1;
        result = realloc(result, new_len);
        strcpy(result + old_len, buffer);
    }
    
    pclose(fp);
    return result;
}

/* Check if file exists */
int file_exists(const char* filename) {
    struct stat st;
    return stat(filename, &st) == 0;
}

/* Clean up temporary files */
void cleanup_files(const char* source_file, const char* binary_file, 
                   const char* gcda_file, const char* gcno_file) {
    if (source_file && file_exists(source_file)) unlink(source_file);
    if (binary_file && file_exists(binary_file)) unlink(binary_file);
    if (gcda_file && file_exists(gcda_file)) unlink(gcda_file);
    if (gcno_file && file_exists(gcno_file)) unlink(gcno_file);
}

int main() {
    char* source_file = NULL;
    char* binary_file = NULL;
    char* gcda_file = NULL;
    char* gcno_file = NULL;
    char cmd[1024];
    int success = 1;
    
    printf("=== Starting gcov-dump test driver ===\n\n");
    
    /* Step 1: Create unique temporary filenames */
    source_file = create_temp_filename(TEMP_SOURCE_FILE);
    binary_file = create_temp_filename(TEMP_BINARY_FILE);
    gcda_file = create_temp_filename(TEMP_GCDA_FILE);
    gcno_file = create_temp_filename(TEMP_GCNO_FILE);
    
    if (!source_file || !binary_file || !gcda_file || !gcno_file) {
        fprintf(stderr, "Failed to create temporary filenames\n");
        success = 0;
        goto cleanup;
    }
    
    printf("Created temporary files:\n");
    printf("  Source: %s\n", source_file);
    printf("  Binary: %s\n", binary_file);
    printf("  GCDA:   %s\n", gcda_file);
    printf("  GCNO:   %s\n", gcno_file);
    printf("\n");
    
    /* Step 2: Create a simple C source file */
    printf("Creating test coverage source file...\n");
    if (!create_coverage_source(source_file)) {
        success = 0;
        goto cleanup;
    }
    
    /* Step 3: Compile with coverage instrumentation */
    printf("Compiling with coverage instrumentation...\n");
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s",
             source_file, binary_file);
    
    if (system(cmd) != 0) {
        fprintf(stderr, "Compilation failed: %s\n", cmd);
        success = 0;
        goto cleanup;
    }
    
    /* Step 4: Execute the program to generate .gcda file */
    printf("Executing test program to generate coverage data...\n");
    if (system(binary_file) != 0) {
        fprintf(stderr, "Execution of %s failed\n", binary_file);
        success = 0;
        goto cleanup;
    }
    
    /* Rename the generated .gcda file to our unique name */
    if (file_exists("test_coverage.gcda")) {
        rename("test_coverage.gcda", gcda_file);
    }
    if (file_exists("test_coverage.gcno")) {
        rename("test_coverage.gcno", gcno_file);
    }
    
    /* Verify .gcda file exists */
    if (!file_exists(gcda_file)) {
        fprintf(stderr, "Failed to generate .gcda file\n");
        success = 0;
        goto cleanup;
    }
    
    printf("Coverage data file created: %s\n\n", gcda_file);
    
    /* Step 5: Test gcov-dump with various flags */
    printf("=== Testing gcov-dump flags ===\n\n");
    
    /* Test 1: -h flag (help) - triggers print_usage() */
    printf("Test 1: -h flag (help)\n");
    printf("Command: gcov-dump -h\n");
    system("gcov-dump -h");
    printf("\n");
    
    /* Test 2: -v flag (version) - triggers print_version() */
    printf("Test 2: -v flag (version)\n");
    printf("Command: gcov-dump -v\n");
    system("gcov-dump -v");
    printf("\n");
    
    /* Test 3: -l flag (dump contents) - sets flag_dump_contents = 1 */
    printf("Test 3: -l flag (dump contents)\n");
    printf("Command: gcov-dump -l %s\n", gcda_file);
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_file);
    system(cmd);
    printf("\n");
    
    /* Test 4: -p flag (dump positions) - sets flag_dump_positions = 1 */
    printf("Test 4: -p flag (dump positions)\n");
    printf("Command: gcov-dump -p %s\n", gcda_file);
    snprintf(cmd, sizeof(cmd), "gcov-dump -p %s", gcda_file);
    system(cmd);
    printf("\n");
    
    /* Test 5: -r flag (dump raw) - sets flag_dump_raw = 1 */
    printf("Test 5: -r flag (dump raw)\n");
    printf("Command: gcov-dump -r %s\n", gcda_file);
    snprintf(cmd, sizeof(cmd), "gcov-dump -r %s", gcda_file);
    system(cmd);
    printf("\n");
    
    /* Test 6: -s flag (dump stable) - sets flag_dump_stable = 1 */
    printf("Test 6: -s flag (dump stable)\n");
    printf("Command: gcov-dump -s %s\n", gcda_file);
    snprintf(cmd, sizeof(cmd), "gcov-dump -s %s", gcda_file);
    system(cmd);
    printf("\n");
    
    /* Test 7: Combined flags (-l -p) */
    printf("Test 7: Combined flags (-l -p)\n");
    printf("Command: gcov-dump -l -p %s\n", gcda_file);
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -p %s", gcda_file);
    system(cmd);
    printf("\n");
    
    /* Test 8: Invalid flag (-X) - triggers default case and fprintf */
    printf("Test 8: Invalid flag (-X) - should trigger 'unknown flag' error\n");
    printf("Command: gcov-dump -X %s\n", gcda_file);
    snprintf(cmd, sizeof(cmd), "gcov-dump -X %s", gcda_file);
    
    /* Capture stderr to verify the error message */
    char* output = execute_command_stderr(cmd);
    if (output) {
        if (strstr(output, "unknown flag") != NULL) {
            printf("SUCCESS: Got expected 'unknown flag' error\n");
            printf("Error message: %s", output);
        } else {
            printf("WARNING: 'unknown flag' message not found in output\n");
            printf("Output: %s", output);
        }
        free(output);
    } else {
        printf("ERROR: Failed to capture command output\n");
    }
    printf("\n");
    
    /* Additional test: Multiple invalid flags */
    printf("Test 9: Multiple invalid flags (-X -Y)\n");
    printf("Command: gcov-dump -X -Y %s\n", gcda_file);
    snprintf(cmd, sizeof(cmd), "gcov-dump -X -Y %s", gcda_file);
    output = execute_command_stderr(cmd);
    if (output) {
        if (strstr(output, "unknown flag") != NULL) {
            printf("SUCCESS: Got expected 'unknown flag' error\n");
        }
        free(output);
    }
    printf("\n");
    
    printf("=== Test complete ===\n");
    
cleanup:
    /* Clean up temporary files */
    printf("\nCleaning up temporary files...\n");
    cleanup_files(source_file, binary_file, gcda_file, gcno_file);
    
    /* Also clean up any default named files */
    if (file_exists("test_coverage.gcda")) unlink("test_coverage.gcda");
    if (file_exists("test_coverage.gcno")) unlink("test_coverage.gcno");
    if (file_exists("test_coverage")) unlink("test_coverage");
    
    free(source_file);
    free(binary_file);
    free(gcda_file);
    free(gcno_file);
    
    if (success) {
        printf("\nAll tests executed successfully!\n");
        printf("The following gcov-dump switch cases should have been triggered:\n");
        printf("  - 'h' case: print_usage()\n");
        printf("  - 'v' case: print_version()\n");
        printf("  - 'l' case: flag_dump_contents = 1\n");
        printf("  - 'p' case: flag_dump_positions = 1\n");
        printf("  - 'r' case: flag_dump_raw = 1\n");
        printf("  - 's' case: flag_dump_stable = 1\n");
        printf("  - default case: fprintf(stderr, \"unknown flag\")\n");
        return 0;
    } else {
        fprintf(stderr, "\nTest failed!\n");
        return 1;
    }
}
