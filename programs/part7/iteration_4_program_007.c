#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_XXXXXX.c"
#define TEMP_BINARY_FILE "test_coverage_XXXXXX"
#define TEMP_GCDA_FILE "test_coverage_XXXXXX.gcda"
#define TEMP_GCNO_FILE "test_coverage_XXXXXX.gcno"

/* Create a temporary filename template */
void create_temp_template(char *buffer, const char *pattern) {
    strcpy(buffer, pattern);
    int fd = mkstemp(buffer);
    if (fd != -1) {
        close(fd);
        unlink(buffer);  /* Remove the file, we just want the name */
    }
}

/* Write a simple C program for coverage testing */
void write_coverage_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create source file");
        exit(1);
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
}

/* Execute a system command and check return value */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int ret = system(cmd);
    if (ret != 0) {
        printf("Command failed with return code: %d\n", ret);
    }
    return ret;
}

/* Execute command and capture stderr to check for specific message */
int execute_and_check_stderr(const char *cmd, const char *expected_error) {
    printf("Executing: %s\n", cmd);
    
    /* Redirect stderr to stdout and capture */
    char full_cmd[1024];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    FILE *fp = popen(full_cmd, "r");
    if (!fp) {
        perror("popen failed");
        return -1;
    }
    
    char buffer[1024];
    int found_error = 0;
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, expected_error) != NULL) {
            found_error = 1;
            printf("Found expected error message: %s", buffer);
        }
    }
    
    int ret = pclose(fp);
    if (found_error) {
        printf("Successfully triggered default case with invalid flag\n");
    } else {
        printf("Warning: Expected error message not found\n");
    }
    
    return ret;
}

/* Check if file exists */
int file_exists(const char *filename) {
    struct stat st;
    return stat(filename, &st) == 0;
}

/* Clean up temporary files */
void cleanup_files(const char *source_file, const char *binary_file, 
                   const char *gcda_file, const char *gcno_file) {
    if (file_exists(source_file)) unlink(source_file);
    if (file_exists(binary_file)) unlink(binary_file);
    if (file_exists(gcda_file)) unlink(gcda_file);
    if (file_exists(gcno_file)) unlink(gcno_file);
}

int main() {
    char source_file[256];
    char binary_file[256];
    char gcda_file[256];
    char gcno_file[256];
    
    /* Create unique temporary filenames */
    create_temp_template(source_file, "test_coverage_XXXXXX.c");
    create_temp_template(binary_file, "test_coverage_XXXXXX");
    create_temp_template(gcda_file, "test_coverage_XXXXXX.gcda");
    create_temp_template(gcno_file, "test_coverage_XXXXXX.gcno");
    
    printf("=== Generating coverage data files ===\n");
    
    /* Write simple C source file */
    write_coverage_source(source_file);
    printf("Created source file: %s\n", source_file);
    
    /* Compile with coverage instrumentation */
    char compile_cmd[1024];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage %s -o %s",
             source_file, binary_file);
    
    if (execute_command(compile_cmd) != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup_files(source_file, binary_file, gcda_file, gcno_file);
        return 1;
    }
    
    /* Execute the program to generate .gcda file */
    if (execute_command(binary_file) != 0) {
        fprintf(stderr, "Execution failed\n");
        cleanup_files(source_file, binary_file, gcda_file, gcno_file);
        return 1;
    }
    
    /* Rename the generated .gcda file to our temp name */
    char generated_gcda[256];
    snprintf(generated_gcda, sizeof(generated_gcda), "%s.gcda", binary_file);
    if (file_exists(generated_gcda)) {
        rename(generated_gcda, gcda_file);
    }
    
    /* Rename the generated .gcno file to our temp name */
    char generated_gcno[256];
    snprintf(generated_gcno, sizeof(generated_gcno), "%s.gcno", binary_file);
    if (file_exists(generated_gcno)) {
        rename(generated_gcno, gcno_file);
    }
    
    /* Verify .gcda file exists */
    if (!file_exists(gcda_file)) {
        fprintf(stderr, "Failed to generate .gcda file\n");
        cleanup_files(source_file, binary_file, gcda_file, gcno_file);
        return 1;
    }
    
    printf("\n=== Testing gcov-dump with various flags ===\n");
    
    /* 1. Test -h flag (help) - triggers print_usage() */
    printf("\n--- Testing -h flag (help) ---\n");
    execute_command("gcov-dump -h");
    
    /* 2. Test -v flag (version) - triggers print_version() */
    printf("\n--- Testing -v flag (version) ---\n");
    execute_command("gcov-dump -v");
    
    /* 3. Test -l flag (dump contents) with .gcda file */
    printf("\n--- Testing -l flag (dump contents) ---\n");
    char cmd_l[1024];
    snprintf(cmd_l, sizeof(cmd_l), "gcov-dump -l %s", gcda_file);
    execute_command(cmd_l);
    
    /* 4. Test -p flag (dump positions) with .gcda file */
    printf("\n--- Testing -p flag (dump positions) ---\n");
    char cmd_p[1024];
    snprintf(cmd_p, sizeof(cmd_p), "gcov-dump -p %s", gcda_file);
    execute_command(cmd_p);
    
    /* 5. Test -r flag (dump raw) with .gcda file */
    printf("\n--- Testing -r flag (dump raw) ---\n");
    char cmd_r[1024];
    snprintf(cmd_r, sizeof(cmd_r), "gcov-dump -r %s", gcda_file);
    execute_command(cmd_r);
    
    /* 6. Test -s flag (dump stable) with .gcda file */
    printf("\n--- Testing -s flag (dump stable) ---\n");
    char cmd_s[1024];
    snprintf(cmd_s, sizeof(cmd_s), "gcov-dump -s %s", gcda_file);
    execute_command(cmd_s);
    
    /* 7. Test combined flags (-l -p) with .gcda file */
    printf("\n--- Testing combined flags (-l -p) ---\n");
    char cmd_lp[1024];
    snprintf(cmd_lp, sizeof(cmd_lp), "gcov-dump -l -p %s", gcda_file);
    execute_command(cmd_lp);
    
    /* 8. Test invalid flag (-X) to trigger default case */
    printf("\n--- Testing invalid flag (-X) to trigger default case ---\n");
    char cmd_invalid[1024];
    snprintf(cmd_invalid, sizeof(cmd_invalid), "gcov-dump -X %s", gcda_file);
    execute_and_check_stderr(cmd_invalid, "unknown flag");
    
    /* 9. Additional test: Try with .gcno file as well */
    printf("\n--- Testing with .gcno file ---\n");
    if (file_exists(gcno_file)) {
        char cmd_gcno[1024];
        snprintf(cmd_gcno, sizeof(cmd_gcno), "gcov-dump -l %s", gcno_file);
        execute_command(cmd_gcno);
    }
    
    /* 10. Test with no arguments (should show usage or error) */
    printf("\n--- Testing with no arguments ---\n");
    execute_command("gcov-dump");
    
    /* Cleanup */
    printf("\n=== Cleaning up temporary files ===\n");
    cleanup_files(source_file, binary_file, gcda_file, gcno_file);
    
    printf("\n=== Test completed ===\n");
    printf("All gcov-dump command-line parsing paths should have been exercised.\n");
    
    return 0;
}
