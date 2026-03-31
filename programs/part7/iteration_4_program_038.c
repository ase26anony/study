#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define TEMP_SOURCE_TEMPLATE "/tmp/test_coverage_XXXXXX.c"
#define TEMP_BINARY_TEMPLATE "/tmp/test_coverage_XXXXXX"
#define MAX_CMD_LEN 1024

/* Create a simple test program that will generate coverage data */
const char *test_program = 
"#include <stdio.h>\n"
"int main() {\n"
"    int i, sum = 0;\n"
"    for (i = 0; i < 10; i++) {\n"
"        sum += i;\n"
"    }\n"
"    printf(\"Sum: %d\\n\", sum);\n"
"    return 0;\n"
"}\n";

/* Function to create a temporary file with given template */
char *create_temp_file(const char *template) {
    char *filename = strdup(template);
    if (!filename) {
        perror("strdup failed");
        return NULL;
    }
    
    int fd = mkstemps(filename, 0);
    if (fd < 0) {
        perror("mkstemps failed");
        free(filename);
        return NULL;
    }
    close(fd);
    return filename;
}

/* Function to check if a file exists */
int file_exists(const char *filename) {
    struct stat st;
    return stat(filename, &st) == 0;
}

/* Function to execute a command and capture stderr */
int execute_and_capture_stderr(const char *cmd, char *output, size_t output_size) {
    char full_cmd[MAX_CMD_LEN];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    FILE *fp = popen(full_cmd, "r");
    if (!fp) {
        perror("popen failed");
        return -1;
    }
    
    if (output && output_size > 0) {
        output[0] = '\0';
        size_t total_read = 0;
        while (fgets(output + total_read, output_size - total_read, fp) != NULL) {
            total_read = strlen(output);
            if (total_read >= output_size - 1) {
                break;
            }
        }
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
}

/* Function to clean up temporary files */
void cleanup_files(const char *source_file, const char *binary_file) {
    if (source_file && file_exists(source_file)) {
        unlink(source_file);
    }
    if (binary_file && file_exists(binary_file)) {
        unlink(binary_file);
    }
    
    /* Also clean up coverage files */
    char gcda_file[MAX_CMD_LEN];
    char gcno_file[MAX_CMD_LEN];
    
    if (binary_file) {
        snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", binary_file);
        snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", binary_file);
        
        if (file_exists(gcda_file)) unlink(gcda_file);
        if (file_exists(gcno_file)) unlink(gcno_file);
    }
}

int main() {
    char *source_file = NULL;
    char *binary_file = NULL;
    char gcda_file[MAX_CMD_LEN];
    char cmd[MAX_CMD_LEN];
    char output[4096];
    int ret;
    
    printf("=== Starting gcov-dump test driver ===\n");
    
    /* Step 1: Create temporary source file */
    source_file = create_temp_file(TEMP_SOURCE_TEMPLATE);
    if (!source_file) {
        fprintf(stderr, "Failed to create source file\n");
        return 1;
    }
    printf("Created source file: %s\n", source_file);
    
    /* Write test program to source file */
    FILE *fp = fopen(source_file, "w");
    if (!fp) {
        perror("Failed to open source file for writing");
        cleanup_files(source_file, NULL);
        free(source_file);
        return 1;
    }
    fprintf(fp, "%s", test_program);
    fclose(fp);
    
    /* Step 2: Create temporary binary name */
    binary_file = create_temp_file(TEMP_BINARY_TEMPLATE);
    if (!binary_file) {
        fprintf(stderr, "Failed to create binary file name\n");
        cleanup_files(source_file, NULL);
        free(source_file);
        return 1;
    }
    printf("Binary will be: %s\n", binary_file);
    
    /* Step 3: Compile with coverage flags */
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage %s -o %s",
             source_file, binary_file);
    printf("Compiling: %s\n", cmd);
    
    ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup_files(source_file, binary_file);
        free(source_file);
        free(binary_file);
        return 1;
    }
    
    /* Step 4: Execute the program to generate .gcda file */
    printf("Executing program to generate coverage data...\n");
    ret = system(binary_file);
    if (ret != 0) {
        fprintf(stderr, "Execution failed\n");
        cleanup_files(source_file, binary_file);
        free(source_file);
        free(binary_file);
        return 1;
    }
    
    /* Construct .gcda filename */
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", binary_file);
    
    /* Verify .gcda file was created */
    if (!file_exists(gcda_file)) {
        fprintf(stderr, "Coverage data file not found: %s\n", gcda_file);
        cleanup_files(source_file, binary_file);
        free(source_file);
        free(binary_file);
        return 1;
    }
    printf("Coverage data file created: %s\n", gcda_file);
    
    printf("\n=== Testing gcov-dump with various flags ===\n");
    
    /* Test 1: -h flag (help) - triggers print_usage() */
    printf("\n1. Testing -h flag (help):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -h");
    printf("Command: %s\n", cmd);
    ret = system(cmd);
    printf("Exit code: %d\n", ret);
    
    /* Test 2: -v flag (version) - triggers print_version() */
    printf("\n2. Testing -v flag (version):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -v");
    printf("Command: %s\n", cmd);
    ret = system(cmd);
    printf("Exit code: %d\n", ret);
    
    /* Test 3: -l flag (dump contents) - sets flag_dump_contents = 1 */
    printf("\n3. Testing -l flag (dump contents):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_file);
    printf("Command: %s\n", cmd);
    ret = system(cmd);
    printf("Exit code: %d\n", ret);
    
    /* Test 4: -p flag (dump positions) - sets flag_dump_positions = 1 */
    printf("\n4. Testing -p flag (dump positions):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -p %s", gcda_file);
    printf("Command: %s\n", cmd);
    ret = system(cmd);
    printf("Exit code: %d\n", ret);
    
    /* Test 5: -r flag (dump raw) - sets flag_dump_raw = 1 */
    printf("\n5. Testing -r flag (dump raw):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -r %s", gcda_file);
    printf("Command: %s\n", cmd);
    ret = system(cmd);
    printf("Exit code: %d\n", ret);
    
    /* Test 6: -s flag (dump stable) - sets flag_dump_stable = 1 */
    printf("\n6. Testing -s flag (dump stable):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -s %s", gcda_file);
    printf("Command: %s\n", cmd);
    ret = system(cmd);
    printf("Exit code: %d\n", ret);
    
    /* Test 7: Combined flags (-l -p) */
    printf("\n7. Testing combined flags (-l -p):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -p %s", gcda_file);
    printf("Command: %s\n", cmd);
    ret = system(cmd);
    printf("Exit code: %d\n", ret);
    
    /* Test 8: Invalid flag (-X) - triggers default case and fprintf */
    printf("\n8. Testing invalid flag (-X) to trigger default case:\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X %s", gcda_file);
    printf("Command: %s\n", cmd);
    
    /* Capture stderr to verify the error message */
    memset(output, 0, sizeof(output));
    ret = execute_and_capture_stderr(cmd, output, sizeof(output));
    
    /* Check if the expected error message appears */
    if (strstr(output, "unknown flag") != NULL) {
        printf("SUCCESS: Default case triggered with message: %s", output);
    } else {
        printf("Output did not contain 'unknown flag'. Full output:\n%s", output);
    }
    printf("Exit code: %d\n", ret);
    
    /* Additional test: Multiple invalid flags */
    printf("\n9. Testing multiple invalid flags (-X -Y):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X -Y %s", gcda_file);
    printf("Command: %s\n", cmd);
    
    memset(output, 0, sizeof(output));
    ret = execute_and_capture_stderr(cmd, output, sizeof(output));
    
    if (strstr(output, "unknown flag") != NULL) {
        printf("SUCCESS: Default case triggered with message: %s", output);
    } else {
        printf("Output:\n%s", output);
    }
    printf("Exit code: %d\n", ret);
    
    /* Cleanup */
    printf("\n=== Cleaning up temporary files ===\n");
    cleanup_files(source_file, binary_file);
    free(source_file);
    free(binary_file);
    
    printf("\n=== Test completed ===\n");
    printf("All gcov-dump command-line parsing paths should have been exercised.\n");
    
    return 0;
}
