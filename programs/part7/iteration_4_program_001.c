#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_XXXXXX.c"
#define TEMP_BINARY_FILE "test_coverage_XXXXXX"
#define BUFFER_SIZE 1024

/* Create a unique temporary filename */
char* create_temp_filename(const char* template) {
    char* filename = strdup(template);
    if (!filename) {
        perror("strdup failed");
        return NULL;
    }
    
    int fd = mkstemps(filename, strlen(template) - 6); /* 6 for XXXXXX */
    if (fd < 0) {
        perror("mkstemps failed");
        free(filename);
        return NULL;
    }
    close(fd);
    
    return filename;
}

/* Write a simple C program for coverage testing */
int write_coverage_source(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to open source file");
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

/* Check if a file exists */
int file_exists(const char* filename) {
    struct stat st;
    return stat(filename, &st) == 0;
}

/* Execute a command and return its exit status */
int execute_command(const char* cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        printf("Command returned non-zero: %d\n", WEXITSTATUS(status));
    }
    return status;
}

/* Execute a command and capture stderr */
int execute_and_capture_stderr(const char* cmd, char* output, size_t output_size) {
    printf("Executing: %s\n", cmd);
    
    /* Redirect stderr to stdout and capture */
    char full_cmd[BUFFER_SIZE];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    FILE* fp = popen(full_cmd, "r");
    if (!fp) {
        perror("popen failed");
        return -1;
    }
    
    /* Read output */
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

/* Clean up temporary files */
void cleanup_files(const char* source_file, const char* binary_file) {
    if (source_file && file_exists(source_file)) {
        remove(source_file);
    }
    if (binary_file && file_exists(binary_file)) {
        remove(binary_file);
    }
    
    /* Remove coverage files */
    char gcda_file[BUFFER_SIZE];
    char gcno_file[BUFFER_SIZE];
    
    if (binary_file) {
        snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", binary_file);
        snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", binary_file);
        
        if (file_exists(gcda_file)) remove(gcda_file);
        if (file_exists(gcno_file)) remove(gcno_file);
    }
}

int main() {
    char *source_file = NULL;
    char *binary_file = NULL;
    char gcda_file[BUFFER_SIZE];
    char command[BUFFER_SIZE * 2];
    char output[BUFFER_SIZE];
    int success = 1;
    
    printf("=== Starting gcov-dump test driver ===\n\n");
    
    /* Step 1: Create temporary filenames */
    source_file = create_temp_filename(TEMP_SOURCE_FILE);
    if (!source_file) {
        fprintf(stderr, "Failed to create source filename\n");
        return 1;
    }
    
    binary_file = create_temp_filename(TEMP_BINARY_FILE);
    if (!binary_file) {
        fprintf(stderr, "Failed to create binary filename\n");
        free(source_file);
        return 1;
    }
    
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", binary_file);
    
    printf("Source file: %s\n", source_file);
    printf("Binary file: %s\n", binary_file);
    printf("Gcda file: %s\n\n", gcda_file);
    
    /* Step 2: Write and compile coverage test program */
    printf("--- Creating coverage test program ---\n");
    if (!write_coverage_source(source_file)) {
        cleanup_files(source_file, binary_file);
        free(source_file);
        free(binary_file);
        return 1;
    }
    
    /* Compile with coverage flags */
    snprintf(command, sizeof(command), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s",
             source_file, binary_file);
    
    if (execute_command(command) != 0) {
        fprintf(stderr, "Compilation failed\n");
        success = 0;
        goto cleanup;
    }
    
    /* Step 3: Execute to generate .gcda file */
    printf("\n--- Generating coverage data ---\n");
    if (execute_command(binary_file) != 0) {
        fprintf(stderr, "Execution failed\n");
        success = 0;
        goto cleanup;
    }
    
    /* Verify .gcda file was created */
    if (!file_exists(gcda_file)) {
        fprintf(stderr, "Coverage data file not created: %s\n", gcda_file);
        success = 0;
        goto cleanup;
    }
    
    printf("\n--- Testing gcov-dump with various flags ---\n");
    
    /* Test 1: Help flag (-h) - triggers print_usage() */
    printf("\n1. Testing -h flag (help):\n");
    execute_command("gcov-dump -h");
    
    /* Test 2: Version flag (-v) - triggers print_version() */
    printf("\n2. Testing -v flag (version):\n");
    execute_command("gcov-dump -v");
    
    /* Test 3: Dump contents flag (-l) */
    printf("\n3. Testing -l flag (dump contents):\n");
    snprintf(command, sizeof(command), "gcov-dump -l %s", gcda_file);
    execute_command(command);
    
    /* Test 4: Dump positions flag (-p) */
    printf("\n4. Testing -p flag (dump positions):\n");
    snprintf(command, sizeof(command), "gcov-dump -p %s", gcda_file);
    execute_command(command);
    
    /* Test 5: Dump raw flag (-r) */
    printf("\n5. Testing -r flag (dump raw):\n");
    snprintf(command, sizeof(command), "gcov-dump -r %s", gcda_file);
    execute_command(command);
    
    /* Test 6: Dump stable flag (-s) */
    printf("\n6. Testing -s flag (dump stable):\n");
    snprintf(command, sizeof(command), "gcov-dump -s %s", gcda_file);
    execute_command(command);
    
    /* Test 7: Combined flags (-l -p) */
    printf("\n7. Testing combined flags (-l -p):\n");
    snprintf(command, sizeof(command), "gcov-dump -l -p %s", gcda_file);
    execute_command(command);
    
    /* Test 8: Invalid flag (-X) - triggers default case and fprintf */
    printf("\n8. Testing invalid flag (-X) to trigger default case:\n");
    snprintf(command, sizeof(command), "gcov-dump -X %s", gcda_file);
    
    /* Capture stderr to verify the error message */
    int exit_status = execute_and_capture_stderr(command, output, sizeof(output));
    
    /* Check if the expected error message appears */
    if (strstr(output, "unknown flag") != NULL) {
        printf("SUCCESS: Invalid flag triggered expected error message:\n");
        printf("%s\n", output);
    } else {
        printf("WARNING: Invalid flag may not have triggered expected error.\n");
        printf("Output was:\n%s\n", output);
    }
    
    /* Test 9: Another invalid flag (-z) */
    printf("\n9. Testing another invalid flag (-z):\n");
    snprintf(command, sizeof(command), "gcov-dump -z %s", gcda_file);
    execute_and_capture_stderr(command, output, sizeof(output));
    if (strstr(output, "unknown flag") != NULL) {
        printf("SUCCESS: Invalid flag -z also triggered error.\n");
    }
    
    /* Test 10: No flags (should also trigger some path) */
    printf("\n10. Testing with no flags (just file):\n");
    snprintf(command, sizeof(command), "gcov-dump %s", gcda_file);
    execute_command(command);
    
cleanup:
    /* Clean up temporary files */
    printf("\n--- Cleaning up temporary files ---\n");
    cleanup_files(source_file, binary_file);
    
    if (source_file) free(source_file);
    if (binary_file) free(binary_file);
    
    printf("\n=== Test driver completed ===\n");
    return success ? 0 : 1;
}
