/**
 * Test driver for gcov-dump to trigger uncovered command-line parsing code.
 * Compile with: gcc -O0 -fprofile-arcs -ftest-coverage gcov_dump_test.c -o gcov_dump_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_XXXXXX.c"
#define TEMP_BINARY_FILE "test_coverage_XXXXXX"
#define TEMP_GCDA_FILE   "test_coverage_XXXXXX.gcda"
#define TEMP_GCNO_FILE   "test_coverage_XXXXXX.gcno"

/* Simple test program source code that will generate coverage data */
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
 * Create a temporary file with a given template and write content to it.
 * Returns the actual filename in dynamically allocated memory, or NULL on error.
 */
char *create_temp_file(const char *template, const char *content) {
    char *filename = strdup(template);
    if (!filename) {
        perror("strdup");
        return NULL;
    }
    
    int fd = mkstemps(filename, 0);
    if (fd < 0) {
        perror("mkstemps");
        free(filename);
        return NULL;
    }
    
    if (content) {
        if (write(fd, content, strlen(content)) != (ssize_t)strlen(content)) {
            perror("write");
            close(fd);
            free(filename);
            return NULL;
        }
    }
    
    close(fd);
    return filename;
}

/**
 * Execute a system command and return its exit status.
 */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status == -1) {
        perror("system");
        return -1;
    }
    return WEXITSTATUS(status);
}

/**
 * Execute a command and capture its stderr output.
 * Returns 1 if the expected error message is found, 0 otherwise.
 */
int check_error_output(const char *cmd, const char *expected_error) {
    printf("Executing (checking stderr): %s\n", cmd);
    
    /* Redirect stderr to stdout and capture via popen */
    char full_cmd[1024];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    FILE *fp = popen(full_cmd, "r");
    if (!fp) {
        perror("popen");
        return 0;
    }
    
    char buffer[1024];
    int found = 0;
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, expected_error) != NULL) {
            found = 1;
            printf("Found expected error: %s", buffer);
        }
    }
    
    pclose(fp);
    return found;
}

/**
 * Generate coverage data by compiling and running a test program.
 * Returns the base filename (without extension) in dynamically allocated memory.
 */
char *generate_coverage_data(void) {
    char *source_file = NULL;
    char *binary_file = NULL;
    char *gcda_file = NULL;
    char *gcno_file = NULL;
    char *base_name = NULL;
    
    /* Create source file */
    source_file = create_temp_file("/tmp/" TEMP_SOURCE_FILE, test_program_source);
    if (!source_file) {
        fprintf(stderr, "Failed to create source file\n");
        goto cleanup;
    }
    
    /* Extract base name (without .c extension) */
    base_name = strdup(source_file);
    if (!base_name) {
        perror("strdup");
        goto cleanup;
    }
    
    /* Remove .c extension if present */
    char *dot = strrchr(base_name, '.');
    if (dot && strcmp(dot, ".c") == 0) {
        *dot = '\0';
    }
    
    /* Create other filenames */
    binary_file = malloc(strlen(base_name) + 1);
    gcda_file = malloc(strlen(base_name) + 5);  /* +4 for .gcda + null */
    gcno_file = malloc(strlen(base_name) + 5);  /* +4 for .gcno + null */
    
    if (!binary_file || !gcda_file || !gcno_file) {
        perror("malloc");
        goto cleanup;
    }
    
    sprintf(binary_file, "%s", base_name);
    sprintf(gcda_file, "%s.gcda", base_name);
    sprintf(gcno_file, "%s.gcno", base_name);
    
    /* Compile with coverage flags */
    char compile_cmd[1024];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage %s -o %s",
             source_file, binary_file);
    
    if (execute_command(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        goto cleanup;
    }
    
    /* Run the program to generate .gcda file */
    char run_cmd[1024];
    snprintf(run_cmd, sizeof(run_cmd), "./%s", binary_file);
    
    if (execute_command(run_cmd) != 0) {
        fprintf(stderr, "Failed to run test program\n");
        goto cleanup;
    }
    
    /* Verify .gcda file was created */
    struct stat st;
    if (stat(gcda_file, &st) != 0 || !S_ISREG(st.st_mode)) {
        fprintf(stderr, "Failed to create .gcda file\n");
        goto cleanup;
    }
    
    printf("Generated coverage data: %s\n", gcda_file);
    
    /* Cleanup temporary files we don't need to pass to gcov-dump */
    unlink(source_file);
    unlink(binary_file);
    free(source_file);
    free(binary_file);
    
    /* Return the base name - caller will need to append .gcda */
    return base_name;

cleanup:
    free(source_file);
    free(binary_file);
    free(gcda_file);
    free(gcno_file);
    free(base_name);
    return NULL;
}

/**
 * Test gcov-dump with various command-line flags.
 */
int test_gcov_dump(const char *gcda_file) {
    int all_tests_passed = 1;
    char cmd[1024];
    
    printf("\n=== Testing gcov-dump command-line parsing ===\n");
    
    /* Test 1: -h flag (help) - triggers print_usage() */
    printf("\n1. Testing -h flag (help):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -h");
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Warning: gcov-dump -h returned non-zero\n");
    }
    
    /* Test 2: -v flag (version) - triggers print_version() */
    printf("\n2. Testing -v flag (version):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -v");
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Warning: gcov-dump -v returned non-zero\n");
    }
    
    /* Test 3: -l flag (dump contents) */
    printf("\n3. Testing -l flag (dump contents):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_file);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Warning: gcov-dump -l returned non-zero\n");
    }
    
    /* Test 4: -p flag (dump positions) */
    printf("\n4. Testing -p flag (dump positions):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -p %s", gcda_file);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Warning: gcov-dump -p returned non-zero\n");
    }
    
    /* Test 5: -r flag (dump raw) */
    printf("\n5. Testing -r flag (dump raw):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -r %s", gcda_file);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Warning: gcov-dump -r returned non-zero\n");
    }
    
    /* Test 6: -s flag (dump stable) */
    printf("\n6. Testing -s flag (dump stable):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -s %s", gcda_file);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Warning: gcov-dump -s returned non-zero\n");
    }
    
    /* Test 7: Combined flags (-l -p) */
    printf("\n7. Testing combined flags (-l -p):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -p %s", gcda_file);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Warning: gcov-dump -l -p returned non-zero\n");
    }
    
    /* Test 8: Invalid flag (-X) - triggers default case and fprintf */
    printf("\n8. Testing invalid flag (-X) to trigger error message:\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X %s", gcda_file);
    if (check_error_output(cmd, "unknown flag")) {
        printf("✓ Successfully triggered 'unknown flag' error message\n");
    } else {
        printf("✗ Did not find expected error message\n");
        all_tests_passed = 0;
    }
    
    /* Test 9: Another invalid flag (-z) */
    printf("\n9. Testing another invalid flag (-z):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -z %s", gcda_file);
    if (check_error_output(cmd, "unknown flag")) {
        printf("✓ Successfully triggered 'unknown flag' error message\n");
    } else {
        printf("✗ Did not find expected error message\n");
        all_tests_passed = 0;
    }
    
    /* Test 10: No flags (should also trigger some code paths) */
    printf("\n10. Testing with no flags (just filename):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump %s", gcda_file);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Warning: gcov-dump with no flags returned non-zero\n");
    }
    
    return all_tests_passed;
}

/**
 * Clean up temporary files.
 */
void cleanup_files(const char *base_name) {
    if (!base_name) return;
    
    char gcda_file[1024];
    char gcno_file[1024];
    
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", base_name);
    snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", base_name);
    
    unlink(gcda_file);
    unlink(gcno_file);
    
    printf("Cleaned up temporary files: %s, %s\n", gcda_file, gcno_file);
}

int main(void) {
    char *base_name = NULL;
    char gcda_file[1024];
    int result = 0;
    
    printf("=== gcov-dump Test Driver ===\n");
    
    /* Step 1: Generate coverage data file */
    printf("\nGenerating coverage data file...\n");
    base_name = generate_coverage_data();
    if (!base_name) {
        fprintf(stderr, "Failed to generate coverage data\n");
        return 1;
    }
    
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", base_name);
    printf("Using coverage file: %s\n", gcda_file);
    
    /* Step 2: Test gcov-dump with various flags */
    result = test_gcov_dump(gcda_file);
    
    /* Step 3: Cleanup */
    printf("\n=== Cleaning up ===\n");
    cleanup_files(base_name);
    free(base_name);
    
    if (result) {
        printf("\n✓ All tests completed successfully\n");
        return 0;
    } else {
        printf("\n✗ Some tests failed\n");
        return 1;
    }
}
