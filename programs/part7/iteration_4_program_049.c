#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define TEMP_SOURCE_FILE "test_coverage_XXXXXX.c"
#define TEMP_BINARY_FILE "test_coverage_XXXXXX"
#define TEMP_GCDA_FILE "test_coverage_XXXXXX.gcda"
#define TEMP_GCNO_FILE "test_coverage_XXXXXX.gcno"

/* Function to create a unique temporary filename */
char* create_temp_filename(const char* pattern) {
    char* temp = strdup(pattern);
    if (!temp) {
        perror("strdup");
        exit(EXIT_FAILURE);
    }
    
    int fd;
    if (strstr(pattern, ".c")) {
        fd = mkstemps(temp, 2);  // .c is 2 chars
    } else if (strstr(pattern, ".gcda")) {
        fd = mkstemps(temp, 5);  // .gcda is 5 chars
    } else if (strstr(pattern, ".gcno")) {
        fd = mkstemps(temp, 5);  // .gcno is 5 chars
    } else {
        fd = mkstemp(temp);
    }
    
    if (fd < 0) {
        perror("mkstemp");
        free(temp);
        exit(EXIT_FAILURE);
    }
    close(fd);
    return temp;
}

/* Function to check if a file exists */
int file_exists(const char* filename) {
    struct stat st;
    return stat(filename, &st) == 0;
}

/* Function to execute a command and capture stderr */
int execute_and_capture_stderr(const char* cmd, char* output, size_t output_size) {
    char full_cmd[1024];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    FILE* fp = popen(full_cmd, "r");
    if (!fp) {
        return -1;
    }
    
    output[0] = '\0';
    size_t total_read = 0;
    while (fgets(output + total_read, output_size - total_read, fp) != NULL) {
        total_read = strlen(output);
        if (total_read >= output_size - 1) {
            break;
        }
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
}

/* Function to execute a command and ignore output */
int execute_command(const char* cmd) {
    int status = system(cmd);
    if (status == -1) {
        perror("system");
        return -1;
    }
    return WEXITSTATUS(status);
}

int main() {
    char *source_file = NULL;
    char *binary_file = NULL;
    char *gcda_file = NULL;
    char *gcno_file = NULL;
    char cmd[1024];
    char output[4096];
    int ret;
    
    printf("=== Starting gcov-dump test driver ===\n");
    
    /* Step 1: Create unique temporary filenames */
    source_file = create_temp_filename(TEMP_SOURCE_FILE);
    binary_file = create_temp_filename(TEMP_BINARY_FILE);
    gcda_file = create_temp_filename(TEMP_GCDA_FILE);
    gcno_file = create_temp_filename(TEMP_GCNO_FILE);
    
    /* Remove the extensions from binary_file for gcc output */
    char* dot = strrchr(binary_file, '.');
    if (dot) *dot = '\0';
    
    printf("Temporary files:\n");
    printf("  Source: %s\n", source_file);
    printf("  Binary: %s\n", binary_file);
    printf("  GCDA:   %s\n", gcda_file);
    printf("  GCNO:   %s\n", gcno_file);
    
    /* Step 2: Create a simple C source file with coverage instrumentation */
    FILE* fp = fopen(source_file, "w");
    if (!fp) {
        perror("fopen source");
        goto cleanup;
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
    
    /* Step 3: Compile with coverage flags */
    printf("\n=== Compiling test program with coverage ===\n");
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s",
             source_file, binary_file);
    printf("Command: %s\n", cmd);
    
    ret = execute_command(cmd);
    if (ret != 0) {
        fprintf(stderr, "Compilation failed with exit code %d\n", ret);
        goto cleanup;
    }
    
    /* Step 4: Execute the program to generate .gcda file */
    printf("\n=== Executing test program to generate coverage data ===\n");
    snprintf(cmd, sizeof(cmd), "./%s", binary_file);
    printf("Command: %s\n", cmd);
    
    ret = execute_command(cmd);
    if (ret != 0) {
        fprintf(stderr, "Execution failed with exit code %d\n", ret);
        goto cleanup;
    }
    
    /* The .gcda and .gcno files should now exist with the binary name */
    /* Rename them to our temporary names */
    char expected_gcda[256], expected_gcno[256];
    snprintf(expected_gcda, sizeof(expected_gcda), "%s.gcda", binary_file);
    snprintf(expected_gcno, sizeof(expected_gcno), "%s.gcno", binary_file);
    
    if (file_exists(expected_gcda)) {
        rename(expected_gcda, gcda_file);
    }
    if (file_exists(expected_gcno)) {
        rename(expected_gcno, gcno_file);
    }
    
    if (!file_exists(gcda_file)) {
        fprintf(stderr, "Error: .gcda file not created: %s\n", gcda_file);
        goto cleanup;
    }
    
    printf("Coverage data file created: %s\n", gcda_file);
    
    /* Step 5: Test gcov-dump with various flags */
    printf("\n=== Testing gcov-dump with different flags ===\n");
    
    /* 5.1 Test -h flag (help) - triggers print_usage() */
    printf("\n1. Testing -h flag (help):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -h");
    printf("Command: %s\n", cmd);
    execute_command(cmd);
    
    /* 5.2 Test -v flag (version) - triggers print_version() */
    printf("\n2. Testing -v flag (version):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -v");
    printf("Command: %s\n", cmd);
    execute_command(cmd);
    
    /* 5.3 Test -l flag (dump contents) - sets flag_dump_contents = 1 */
    printf("\n3. Testing -l flag (dump contents):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_file);
    printf("Command: %s\n", cmd);
    execute_command(cmd);
    
    /* 5.4 Test -p flag (dump positions) - sets flag_dump_positions = 1 */
    printf("\n4. Testing -p flag (dump positions):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -p %s", gcda_file);
    printf("Command: %s\n", cmd);
    execute_command(cmd);
    
    /* 5.5 Test -r flag (dump raw) - sets flag_dump_raw = 1 */
    printf("\n5. Testing -r flag (dump raw):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -r %s", gcda_file);
    printf("Command: %s\n", cmd);
    execute_command(cmd);
    
    /* 5.6 Test -s flag (dump stable) - sets flag_dump_stable = 1 */
    printf("\n6. Testing -s flag (dump stable):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -s %s", gcda_file);
    printf("Command: %s\n", cmd);
    execute_command(cmd);
    
    /* 5.7 Test combined flags -l -p */
    printf("\n7. Testing combined flags -l -p:\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -p %s", gcda_file);
    printf("Command: %s\n", cmd);
    execute_command(cmd);
    
    /* 5.8 Test invalid flag -X - triggers default case and fprintf */
    printf("\n8. Testing invalid flag -X (should trigger 'unknown flag' error):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X %s", gcda_file);
    printf("Command: %s\n", cmd);
    
    /* Capture stderr to verify the error message */
    ret = execute_and_capture_stderr(cmd, output, sizeof(output));
    printf("Exit code: %d\n", ret);
    
    /* Check if the expected error message is in the output */
    if (strstr(output, "unknown flag") != NULL) {
        printf("SUCCESS: 'unknown flag' error message detected!\n");
        printf("Output snippet: ");
        char* unknown_flag_msg = strstr(output, "unknown flag");
        if (unknown_flag_msg) {
            /* Print just the line containing the error */
            char* newline = strchr(unknown_flag_msg, '\n');
            if (newline) {
                *newline = '\0';
            }
            printf("%s\n", unknown_flag_msg);
        }
    } else {
        printf("Output did not contain 'unknown flag' message:\n%s\n", output);
    }
    
    /* 5.9 Test another invalid flag -z */
    printf("\n9. Testing another invalid flag -z:\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -z %s", gcda_file);
    printf("Command: %s\n", cmd);
    
    ret = execute_and_capture_stderr(cmd, output, sizeof(output));
    if (strstr(output, "unknown flag") != NULL) {
        printf("SUCCESS: 'unknown flag' error message detected for -z flag!\n");
    }
    
    /* 5.10 Test with .gcno file as well */
    printf("\n10. Testing with .gcno file:\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcno_file);
    printf("Command: %s\n", cmd);
    execute_command(cmd);
    
    printf("\n=== All tests completed ===\n");

cleanup:
    /* Step 6: Clean up temporary files */
    printf("\n=== Cleaning up temporary files ===\n");
    
    if (source_file && file_exists(source_file)) {
        unlink(source_file);
        printf("Removed: %s\n", source_file);
    }
    
    if (binary_file && file_exists(binary_file)) {
        unlink(binary_file);
        printf("Removed: %s\n", binary_file);
    }
    
    /* Also remove the binary with .gcda/.gcno extensions if they exist */
    char temp[256];
    snprintf(temp, sizeof(temp), "%s.gcda", binary_file);
    if (file_exists(temp)) {
        unlink(temp);
        printf("Removed: %s\n", temp);
    }
    
    snprintf(temp, sizeof(temp), "%s.gcno", binary_file);
    if (file_exists(temp)) {
        unlink(temp);
        printf("Removed: %s\n", temp);
    }
    
    if (gcda_file && file_exists(gcda_file)) {
        unlink(gcda_file);
        printf("Removed: %s\n", gcda_file);
    }
    
    if (gcno_file && file_exists(gcno_file)) {
        unlink(gcno_file);
        printf("Removed: %s\n", gcno_file);
    }
    
    /* Free allocated memory */
    free(source_file);
    free(binary_file);
    free(gcda_file);
    free(gcno_file);
    
    printf("\n=== Test driver finished ===\n");
    return 0;
}
