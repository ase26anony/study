/**
 * test_gcov_dump_switches.c
 * 
 * Test driver to exercise specific command-line switch cases in gcov-dump
 * Targets lines 111-130 of gcov-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

/* Simple test program to generate coverage data */
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

/* Function to create a temporary filename */
char *create_temp_filename(const char *prefix, const char *suffix) {
    char *template = malloc(strlen(prefix) + 10 + strlen(suffix) + 1);
    if (!template) return NULL;
    
    sprintf(template, "%sXXXXXX%s", prefix, suffix);
    int fd = mkstemps(template, strlen(suffix));
    if (fd < 0) {
        free(template);
        return NULL;
    }
    close(fd);
    return template;
}

/* Execute a command and capture stderr */
int execute_and_capture_stderr(const char *cmd, char *output, size_t output_size) {
    FILE *fp;
    char cmd_with_stderr[1024];
    
    /* Redirect stderr to stdout for capture */
    snprintf(cmd_with_stderr, sizeof(cmd_with_stderr), "%s 2>&1", cmd);
    
    fp = popen(cmd_with_stderr, "r");
    if (fp == NULL) {
        return -1;
    }
    
    /* Read output */
    size_t total_read = 0;
    while (fgets(output + total_read, output_size - total_read, fp) != NULL) {
        total_read = strlen(output);
        if (total_read >= output_size - 1) break;
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
}

/* Check if string contains substring */
int contains_string(const char *str, const char *substr) {
    return strstr(str, substr) != NULL;
}

int main(int argc, char *argv[]) {
    char *source_file = NULL;
    char *binary_file = NULL;
    char *gcda_file = NULL;
    char *gcno_file = NULL;
    char command[1024];
    char output[4096];
    int ret = 0;
    
    printf("=== Testing gcov-dump command-line switches ===\n\n");
    
    /* Create temporary filenames */
    source_file = create_temp_filename("test_gcov_", ".c");
    binary_file = create_temp_filename("test_gcov_", "");
    gcda_file = malloc(strlen(binary_file) + 6);
    gcno_file = malloc(strlen(binary_file) + 6);
    
    if (!source_file || !binary_file || !gcda_file || !gcno_file) {
        fprintf(stderr, "Failed to create temporary filenames\n");
        ret = 1;
        goto cleanup;
    }
    
    sprintf(gcda_file, "%s.gcda", binary_file);
    sprintf(gcno_file, "%s.gcno", binary_file);
    
    printf("Temporary files:\n");
    printf("  Source: %s\n", source_file);
    printf("  Binary: %s\n", binary_file);
    printf("  GCDA:   %s\n", gcda_file);
    printf("  GCNO:   %s\n", gcno_file);
    printf("\n");
    
    /* Step 1: Create test source file */
    printf("1. Creating test source file...\n");
    FILE *fp = fopen(source_file, "w");
    if (!fp) {
        fprintf(stderr, "Failed to create source file: %s\n", source_file);
        ret = 1;
        goto cleanup;
    }
    fputs(test_source, fp);
    fclose(fp);
    
    /* Step 2: Compile with coverage instrumentation */
    printf("2. Compiling with coverage flags...\n");
    snprintf(command, sizeof(command),
             "gcc -fprofile-arcs -ftest-coverage -O0 %s -o %s",
             source_file, binary_file);
    
    if (system(command) != 0) {
        fprintf(stderr, "Compilation failed: %s\n", command);
        ret = 1;
        goto cleanup;
    }
    
    /* Step 3: Execute to generate coverage data */
    printf("3. Executing to generate .gcda file...\n");
    if (system(binary_file) != 0) {
        fprintf(stderr, "Execution failed: %s\n", binary_file);
        ret = 1;
        goto cleanup;
    }
    
    /* Verify .gcda file exists */
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "Coverage data file not created: %s\n", gcda_file);
        ret = 1;
        goto cleanup;
    }
    
    printf("4. Testing gcov-dump switches...\n\n");
    
    /* Test 1: -h flag (help) - triggers print_usage() */
    printf("Test 1: -h flag (help)\n");
    printf("  Command: gcov-dump -h\n");
    snprintf(command, sizeof(command), "gcov-dump -h");
    if (execute_and_capture_stderr(command, output, sizeof(output)) != 0) {
        printf("  WARNING: Command returned non-zero\n");
    }
    printf("  Completed\n\n");
    
    /* Test 2: -v flag (version) - triggers print_version() */
    printf("Test 2: -v flag (version)\n");
    printf("  Command: gcov-dump -v\n");
    snprintf(command, sizeof(command), "gcov-dump -v");
    if (execute_and_capture_stderr(command, output, sizeof(output)) != 0) {
        printf("  WARNING: Command returned non-zero\n");
    }
    printf("  Completed\n\n");
    
    /* Test 3: -l flag (dump contents) */
    printf("Test 3: -l flag (dump contents)\n");
    printf("  Command: gcov-dump -l %s\n", gcda_file);
    snprintf(command, sizeof(command), "gcov-dump -l %s", gcda_file);
    if (execute_and_capture_stderr(command, output, sizeof(output)) != 0) {
        printf("  WARNING: Command returned non-zero\n");
    }
    printf("  Completed\n\n");
    
    /* Test 4: -p flag (dump positions) */
    printf("Test 4: -p flag (dump positions)\n");
    printf("  Command: gcov-dump -p %s\n", gcda_file);
    snprintf(command, sizeof(command), "gcov-dump -p %s", gcda_file);
    if (execute_and_capture_stderr(command, output, sizeof(output)) != 0) {
        printf("  WARNING: Command returned non-zero\n");
    }
    printf("  Completed\n\n");
    
    /* Test 5: -r flag (dump raw) */
    printf("Test 5: -r flag (dump raw)\n");
    printf("  Command: gcov-dump -r %s\n", gcda_file);
    snprintf(command, sizeof(command), "gcov-dump -r %s", gcda_file);
    if (execute_and_capture_stderr(command, output, sizeof(output)) != 0) {
        printf("  WARNING: Command returned non-zero\n");
    }
    printf("  Completed\n\n");
    
    /* Test 6: -s flag (dump stable) */
    printf("Test 6: -s flag (dump stable)\n");
    printf("  Command: gcov-dump -s %s\n", gcda_file);
    snprintf(command, sizeof(command), "gcov-dump -s %s", gcda_file);
    if (execute_and_capture_stderr(command, output, sizeof(output)) != 0) {
        printf("  WARNING: Command returned non-zero\n");
    }
    printf("  Completed\n\n");
    
    /* Test 7: Combined flags */
    printf("Test 7: Combined flags (-l -p)\n");
    printf("  Command: gcov-dump -l -p %s\n", gcda_file);
    snprintf(command, sizeof(command), "gcov-dump -l -p %s", gcda_file);
    if (execute_and_capture_stderr(command, output, sizeof(output)) != 0) {
        printf("  WARNING: Command returned non-zero\n");
    }
    printf("  Completed\n\n");
    
    /* Test 8: Invalid flag (triggers default case with fprintf) */
    printf("Test 8: Invalid flag (should trigger 'unknown flag' error)\n");
    printf("  Command: gcov-dump -X %s\n", gcda_file);
    snprintf(command, sizeof(command), "gcov-dump -X %s", gcda_file);
    
    memset(output, 0, sizeof(output));
    int exit_status = execute_and_capture_stderr(command, output, sizeof(output));
    
    /* Check for the expected error message */
    if (contains_string(output, "unknown flag") || contains_string(output, "unknown option")) {
        printf("  SUCCESS: Invalid flag detected! Output contains:\n");
        printf("    %s", output);
    } else {
        printf("  WARNING: Expected 'unknown flag' message not found\n");
        printf("  Exit status: %d\n", exit_status);
        printf("  Output: %s\n", output);
    }
    printf("  Completed\n\n");
    
    /* Test 9: Test with .gcno file as well */
    printf("Test 9: Testing with .gcno file\n");
    printf("  Command: gcov-dump -l %s\n", gcno_file);
    snprintf(command, sizeof(command), "gcov-dump -l %s", gcno_file);
    if (execute_and_capture_stderr(command, output, sizeof(output)) != 0) {
        printf("  WARNING: Command returned non-zero\n");
    }
    printf("  Completed\n\n");
    
    printf("=== All tests completed ===\n");

cleanup:
    /* Clean up temporary files */
    printf("\nCleaning up temporary files...\n");
    if (source_file) {
        unlink(source_file);
        free(source_file);
    }
    if (binary_file) {
        unlink(binary_file);
        free(binary_file);
    }
    if (gcda_file) {
        unlink(gcda_file);
        free(gcda_file);
    }
    if (gcno_file) {
        unlink(gcno_file);
        free(gcno_file);
    }
    
    return ret;
}
