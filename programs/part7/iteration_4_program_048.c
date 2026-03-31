/**
 * Test driver for gcov-dump command-line flag coverage.
 * This program generates coverage data files and invokes gcov-dump
 * with various flags to exercise the switch cases in gcov-dump.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define TEMP_FILE_TEMPLATE "/tmp/gcov_test_XXXXXX"

/* Function prototypes */
int create_temp_file(char *template, char *suffix, char *fullpath, size_t len);
int compile_with_coverage(const char *source_file, const char *output_file);
int execute_program(const char *program);
int invoke_gcov_dump(const char *args, const char *gcda_file, int capture_stderr);
void cleanup_files(const char **files, int count);

int main(int argc, char *argv[]) {
    int ret = 0;
    char source_file[256];
    char binary_file[256];
    char gcda_file[256];
    char gcno_file[256];
    char cmd[MAX_CMD_LEN];
    const char *temp_files[4];
    int file_count = 0;
    
    printf("=== Starting gcov-dump flag coverage test ===\n\n");
    
    /* Step 1: Create a simple C source file with coverage instrumentation */
    printf("1. Creating test source file...\n");
    if (create_temp_file(TEMP_FILE_TEMPLATE, ".c", source_file, sizeof(source_file)) != 0) {
        fprintf(stderr, "Failed to create temporary source file\n");
        return 1;
    }
    temp_files[file_count++] = source_file;
    
    /* Write a simple C program that will generate coverage data */
    FILE *src = fopen(source_file, "w");
    if (!src) {
        perror("Failed to open source file");
        cleanup_files(temp_files, file_count);
        return 1;
    }
    
    fprintf(src, "#include <stdio.h>\n\n");
    fprintf(src, "int factorial(int n) {\n");
    fprintf(src, "    if (n <= 1) return 1;\n");
    fprintf(src, "    return n * factorial(n - 1);\n");
    fprintf(src, "}\n\n");
    fprintf(src, "int main() {\n");
    fprintf(src, "    int i, sum = 0;\n");
    fprintf(src, "    for (i = 0; i < 10; i++) {\n");
    fprintf(src, "        sum += i;\n");
    fprintf(src, "    }\n");
    fprintf(src, "    printf(\"Sum: %%d\\n\", sum);\n");
    fprintf(src, "    printf(\"Factorial 5: %%d\\n\", factorial(5));\n");
    fprintf(src, "    return 0;\n");
    fprintf(src, "}\n");
    fclose(src);
    
    printf("   Source file created: %s\n", source_file);
    
    /* Step 2: Compile with coverage flags */
    printf("\n2. Compiling with coverage instrumentation...\n");
    if (create_temp_file(TEMP_FILE_TEMPLATE, "", binary_file, sizeof(binary_file)) != 0) {
        fprintf(stderr, "Failed to create temporary binary file\n");
        cleanup_files(temp_files, file_count);
        return 1;
    }
    temp_files[file_count++] = binary_file;
    
    if (compile_with_coverage(source_file, binary_file) != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup_files(temp_files, file_count);
        return 1;
    }
    
    /* Set up gcda and gcno file names */
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", binary_file);
    snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", binary_file);
    temp_files[file_count++] = gcda_file;
    temp_files[file_count++] = gcno_file;
    
    printf("   Binary created: %s\n", binary_file);
    printf("   Expected gcda: %s\n", gcda_file);
    printf("   Expected gcno: %s\n", gcno_file);
    
    /* Step 3: Execute the program to generate coverage data */
    printf("\n3. Executing program to generate coverage data...\n");
    if (execute_program(binary_file) != 0) {
        fprintf(stderr, "Execution failed\n");
        cleanup_files(temp_files, file_count);
        return 1;
    }
    
    /* Verify gcda file was created */
    struct stat st;
    if (stat(gcda_file, &st) != 0 || st.st_size == 0) {
        fprintf(stderr, "Coverage data file not created or empty: %s\n", gcda_file);
        cleanup_files(temp_files, file_count);
        return 1;
    }
    printf("   Coverage data generated: %s (%ld bytes)\n", gcda_file, (long)st.st_size);
    
    /* Step 4: Invoke gcov-dump with various flags */
    printf("\n4. Testing gcov-dump command-line flags...\n");
    
    /* 4.1 Test -h flag (help) - triggers print_usage() */
    printf("\n   a) Testing -h flag (help)...\n");
    if (invoke_gcov_dump("-h", NULL, 0) != 0) {
        fprintf(stderr, "Warning: -h flag test may have failed\n");
    }
    
    /* 4.2 Test -v flag (version) - triggers print_version() */
    printf("\n   b) Testing -v flag (version)...\n");
    if (invoke_gcov_dump("-v", NULL, 0) != 0) {
        fprintf(stderr, "Warning: -v flag test may have failed\n");
    }
    
    /* 4.3 Test -l flag (dump contents) - sets flag_dump_contents = 1 */
    printf("\n   c) Testing -l flag (dump contents)...\n");
    if (invoke_gcov_dump("-l", gcda_file, 0) != 0) {
        fprintf(stderr, "Warning: -l flag test may have failed\n");
    }
    
    /* 4.4 Test -p flag (dump positions) - sets flag_dump_positions = 1 */
    printf("\n   d) Testing -p flag (dump positions)...\n");
    if (invoke_gcov_dump("-p", gcda_file, 0) != 0) {
        fprintf(stderr, "Warning: -p flag test may have failed\n");
    }
    
    /* 4.5 Test -r flag (dump raw) - sets flag_dump_raw = 1 */
    printf("\n   e) Testing -r flag (dump raw)...\n");
    if (invoke_gcov_dump("-r", gcda_file, 0) != 0) {
        fprintf(stderr, "Warning: -r flag test may have failed\n");
    }
    
    /* 4.6 Test -s flag (dump stable) - sets flag_dump_stable = 1 */
    printf("\n   f) Testing -s flag (dump stable)...\n");
    if (invoke_gcov_dump("-s", gcda_file, 0) != 0) {
        fprintf(stderr, "Warning: -s flag test may have failed\n");
    }
    
    /* 4.7 Test combined flags -l -p */
    printf("\n   g) Testing combined flags -l -p...\n");
    if (invoke_gcov_dump("-l -p", gcda_file, 0) != 0) {
        fprintf(stderr, "Warning: combined flags test may have failed\n");
    }
    
    /* 4.8 Test invalid flag -X - triggers default case and fprintf */
    printf("\n   h) Testing invalid flag -X (should trigger error)...\n");
    if (invoke_gcov_dump("-X", gcda_file, 1) == 0) {
        printf("      SUCCESS: Invalid flag triggered error message\n");
    } else {
        printf("      Note: Invalid flag test completed (may not have printed expected error)\n");
    }
    
    /* Step 5: Cleanup */
    printf("\n5. Cleaning up temporary files...\n");
    cleanup_files(temp_files, file_count);
    
    printf("\n=== gcov-dump flag coverage test completed ===\n");
    return ret;
}

/**
 * Create a temporary file with the given suffix
 */
int create_temp_file(char *template, char *suffix, char *fullpath, size_t len) {
    char base[256];
    int fd;
    
    strncpy(base, template, sizeof(base) - 1);
    base[sizeof(base) - 1] = '\0';
    
    fd = mkstemp(base);
    if (fd < 0) {
        return -1;
    }
    close(fd);
    
    if (suffix && suffix[0] != '\0') {
        snprintf(fullpath, len, "%s%s", base, suffix);
        /* Rename to add suffix */
        if (rename(base, fullpath) != 0) {
            unlink(base);
            return -1;
        }
    } else {
        strncpy(fullpath, base, len - 1);
        fullpath[len - 1] = '\0';
    }
    
    return 0;
}

/**
 * Compile a source file with coverage instrumentation
 */
int compile_with_coverage(const char *source_file, const char *output_file) {
    char cmd[MAX_CMD_LEN];
    int ret;
    
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s 2>&1",
             source_file, output_file);
    
    ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Compilation command failed: %s\n", cmd);
        /* Try to capture error output */
        FILE *pipe = popen(cmd, "r");
        if (pipe) {
            char buffer[128];
            while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
                fputs(buffer, stderr);
            }
            pclose(pipe);
        }
        return -1;
    }
    
    return 0;
}

/**
 * Execute a program
 */
int execute_program(const char *program) {
    char cmd[MAX_CMD_LEN];
    int ret;
    
    snprintf(cmd, sizeof(cmd), "%s 2>&1", program);
    ret = system(cmd);
    
    if (ret != 0) {
        fprintf(stderr, "Execution failed for: %s\n", program);
        return -1;
    }
    
    return 0;
}

/**
 * Invoke gcov-dump with the given arguments
 * If capture_stderr is non-zero, capture and check for error messages
 */
int invoke_gcov_dump(const char *args, const char *gcda_file, int capture_stderr) {
    char cmd[MAX_CMD_LEN];
    FILE *pipe;
    char buffer[256];
    int found_error = 0;
    
    /* Build the command */
    if (gcda_file) {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s 2>&1", args, gcda_file);
    } else {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s 2>&1", args);
    }
    
    printf("      Command: %s\n", cmd);
    
    if (capture_stderr) {
        /* Use popen to capture output and check for error message */
        pipe = popen(cmd, "r");
        if (!pipe) {
            perror("popen failed");
            return -1;
        }
        
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            /* Check for the expected error message from default case */
            if (strstr(buffer, "unknown flag") != NULL) {
                printf("      Found expected error: %s", buffer);
                found_error = 1;
            }
        }
        
        pclose(pipe);
        return found_error ? 0 : -1;
    } else {
        /* Just execute the command */
        int ret = system(cmd);
        if (ret != 0) {
            /* For help and version, non-zero exit might be expected */
            if (strstr(args, "-h") || strstr(args, "-v")) {
                return 0;  /* These often exit with non-zero */
            }
            fprintf(stderr, "      Command returned non-zero: %d\n", ret);
            return -1;
        }
        return 0;
    }
}

/**
 * Clean up temporary files
 */
void cleanup_files(const char **files, int count) {
    int i;
    for (i = 0; i < count; i++) {
        if (files[i] && files[i][0]) {
            if (unlink(files[i]) == 0) {
                printf("   Removed: %s\n", files[i]);
            } else if (errno != ENOENT) {
                fprintf(stderr, "   Failed to remove %s: %s\n", 
                        files[i], strerror(errno));
            }
        }
    }
}
