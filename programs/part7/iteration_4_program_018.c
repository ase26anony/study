/**
 * test_gcov_dump_switches.c
 * 
 * Test driver to exercise specific command-line switch cases in gcov-dump
 * Targets lines 111-130 in gcov-dump.cc:
 * - Cases for flags: h, v, l, p, r, s
 * - Default case for invalid flags
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define TEMP_FILENAME_PREFIX "test_gcov_dump_"

/* Simple test program that will be compiled with coverage flags */
const char *test_program_source = 
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
 * Create a temporary file with given content
 * Returns dynamically allocated filename or NULL on error
 */
char *create_temp_file(const char *content, const char *suffix) {
    char *filename = malloc(256);
    if (!filename) return NULL;
    
    snprintf(filename, 256, "%s%s", TEMP_FILENAME_PREFIX, suffix);
    
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        free(filename);
        return NULL;
    }
    
    fputs(content, fp);
    fclose(fp);
    return filename;
}

/**
 * Check if a file exists
 */
int file_exists(const char *filename) {
    struct stat st;
    return stat(filename, &st) == 0;
}

/**
 * Execute a command and capture stderr
 * Returns 1 if "unknown flag" appears in output (for invalid flag test)
 */
int execute_and_check_stderr(const char *cmd) {
    char buffer[1024];
    int found_unknown_flag = 0;
    
    /* Use popen to capture stderr (2>&1 redirects stderr to stdout) */
    char full_cmd[MAX_CMD_LEN];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    FILE *fp = popen(full_cmd, "r");
    if (!fp) {
        fprintf(stderr, "Failed to execute: %s\n", cmd);
        return 0;
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, "unknown flag")) {
            found_unknown_flag = 1;
        }
    }
    
    pclose(fp);
    return found_unknown_flag;
}

/**
 * Clean up temporary files
 */
void cleanup_files(const char **filenames, int count) {
    for (int i = 0; i < count; i++) {
        if (filenames[i]) {
            unlink(filenames[i]);
        }
    }
}

int main() {
    char cmd[MAX_CMD_LEN];
    const char *temp_files[10] = {0};
    int file_count = 0;
    
    printf("=== Testing gcov-dump command-line switches ===\n\n");
    
    /* Step 1: Create test source file */
    char *source_file = create_temp_file(test_program_source, "source.c");
    if (!source_file) {
        fprintf(stderr, "Failed to create source file\n");
        return 1;
    }
    temp_files[file_count++] = source_file;
    
    /* Step 2: Compile with coverage flags */
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %sprogram",
             source_file, TEMP_FILENAME_PREFIX);
    
    printf("Compiling test program: %s\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup_files(temp_files, file_count);
        return 1;
    }
    
    char *binary_file = malloc(256);
    snprintf(binary_file, 256, "%sprogram", TEMP_FILENAME_PREFIX);
    temp_files[file_count++] = binary_file;
    
    /* Step 3: Execute to generate .gcda file */
    printf("Executing test program to generate coverage data...\n");
    if (system(binary_file) != 0) {
        fprintf(stderr, "Execution failed\n");
        cleanup_files(temp_files, file_count);
        return 1;
    }
    
    /* Construct .gcda filename (matches source filename) */
    char gcda_file[256];
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", source_file);
    
    /* Wait a moment for file system sync */
    sleep(1);
    
    if (!file_exists(gcda_file)) {
        fprintf(stderr, "Coverage data file not created: %s\n", gcda_file);
        /* Try alternative naming */
        snprintf(gcda_file, sizeof(gcda_file), "test_gcov_dump_source.gcda");
    }
    
    if (!file_exists(gcda_file)) {
        fprintf(stderr, "Cannot find .gcda file, trying current directory...\n");
        /* Last resort: look for any .gcda file */
        system("ls -la *.gcda 2>/dev/null");
        strcpy(gcda_file, "test_gcov_dump_source.gcda");
    }
    
    printf("Using coverage file: %s\n\n", gcda_file);
    
    /* Step 4: Test gcov-dump with various flags */
    
    /* Test 1: Help flag (-h) - triggers print_usage() */
    printf("1. Testing -h flag (help):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -h");
    system(cmd);
    printf("\n");
    
    /* Test 2: Version flag (-v) - triggers print_version() */
    printf("2. Testing -v flag (version):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -v");
    system(cmd);
    printf("\n");
    
    /* Test 3: Dump contents flag (-l) */
    printf("3. Testing -l flag (dump contents):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_file);
    system(cmd);
    printf("\n");
    
    /* Test 4: Dump positions flag (-p) */
    printf("4. Testing -p flag (dump positions):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -p %s", gcda_file);
    system(cmd);
    printf("\n");
    
    /* Test 5: Dump raw flag (-r) */
    printf("5. Testing -r flag (dump raw):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -r %s", gcda_file);
    system(cmd);
    printf("\n");
    
    /* Test 6: Dump stable flag (-s) */
    printf("6. Testing -s flag (dump stable):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -s %s", gcda_file);
    system(cmd);
    printf("\n");
    
    /* Test 7: Combined flags (-l -p) */
    printf("7. Testing combined flags (-l -p):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -p %s", gcda_file);
    system(cmd);
    printf("\n");
    
    /* Test 8: Invalid flag (-X) - triggers default case and fprintf */
    printf("8. Testing invalid flag (-X):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X %s", gcda_file);
    int found_error = execute_and_check_stderr(cmd);
    
    if (found_error) {
        printf("✓ Successfully triggered 'unknown flag' error message\n");
    } else {
        printf("✗ Did not find expected error message\n");
    }
    printf("\n");
    
    /* Additional test: Multiple invalid flags */
    printf("9. Testing another invalid flag (-z):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -z %s", gcda_file);
    found_error = execute_and_check_stderr(cmd);
    
    if (found_error) {
        printf("✓ Successfully triggered 'unknown flag' error message\n");
    } else {
        printf("✗ Did not find expected error message\n");
    }
    printf("\n");
    
    /* Test with no flags but with file (should also parse options) */
    printf("10. Testing with just filename (no flags):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump %s", gcda_file);
    system(cmd);
    printf("\n");
    
    /* Cleanup */
    printf("Cleaning up temporary files...\n");
    
    /* Add gcda and gcno files to cleanup list */
    temp_files[file_count++] = gcda_file;
    
    char gcno_file[256];
    snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", source_file);
    if (file_exists(gcno_file)) {
        temp_files[file_count++] = strdup(gcno_file);
    }
    
    cleanup_files(temp_files, file_count);
    
    /* Free allocated strings */
    free(source_file);
    free(binary_file);
    
    printf("\n=== All tests completed ===\n");
    printf("Targeted switch cases in gcov-dump.cc should have been executed.\n");
    
    return 0;
}
