/**
 * Test driver for gcov-dump command-line flag coverage
 * Compile with: gcc -O0 -fprofile-arcs -ftest-coverage gcov_dump_test.c -o gcov_dump_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define MAX_OUTPUT_LEN 4096

/* Function prototypes */
int create_test_source(const char *filename);
int compile_with_coverage(const char *source_file, const char *binary_name);
int execute_test_program(const char *binary_name);
int invoke_gcov_dump(const char *command, int capture_stderr);
void cleanup_files(const char *base_name);
int check_invalid_flag_output(const char *output);

int main(int argc, char *argv[]) {
    char cmd[MAX_CMD_LEN];
    char output[MAX_OUTPUT_LEN];
    int ret;
    
    /* Use unique base name to avoid collisions */
    const char *base_name = "test_gcov_dump_coverage";
    char source_file[MAX_CMD_LEN];
    char binary_name[MAX_CMD_LEN];
    char gcda_file[MAX_CMD_LEN];
    
    snprintf(source_file, sizeof(source_file), "%s.c", base_name);
    snprintf(binary_name, sizeof(binary_name), "%s.exe", base_name);
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", base_name);
    
    printf("=== Starting gcov-dump flag coverage test ===\n\n");
    
    /* Step 1: Create test source file */
    printf("1. Creating test source file: %s\n", source_file);
    if (!create_test_source(source_file)) {
        fprintf(stderr, "Failed to create test source file\n");
        return 1;
    }
    
    /* Step 2: Compile with coverage instrumentation */
    printf("2. Compiling with coverage flags: %s\n", source_file);
    if (!compile_with_coverage(source_file, binary_name)) {
        fprintf(stderr, "Failed to compile test program\n");
        cleanup_files(base_name);
        return 1;
    }
    
    /* Step 3: Execute to generate .gcda file */
    printf("3. Executing test program to generate .gcda file\n");
    if (!execute_test_program(binary_name)) {
        fprintf(stderr, "Failed to execute test program\n");
        cleanup_files(base_name);
        return 1;
    }
    
    /* Verify .gcda file exists */
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, ".gcda file not created: %s\n", gcda_file);
        cleanup_files(base_name);
        return 1;
    }
    printf("   Generated: %s (size: %ld bytes)\n\n", gcda_file, st.st_size);
    
    /* Step 4: Invoke gcov-dump with various flags */
    printf("4. Testing gcov-dump command-line flags:\n");
    
    /* 4a: Test -h flag (help) - triggers print_usage() */
    printf("   a) Testing -h flag (help)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -h");
    ret = system(cmd);
    printf("     Return code: %d\n", ret >> 8);
    
    /* 4b: Test -v flag (version) - triggers print_version() */
    printf("   b) Testing -v flag (version)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -v");
    ret = system(cmd);
    printf("     Return code: %d\n", ret >> 8);
    
    /* 4c: Test -l flag (dump contents) - sets flag_dump_contents */
    printf("   c) Testing -l flag (dump contents)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_file);
    ret = system(cmd);
    printf("     Return code: %d\n", ret >> 8);
    
    /* 4d: Test -p flag (dump positions) - sets flag_dump_positions */
    printf("   d) Testing -p flag (dump positions)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -p %s", gcda_file);
    ret = system(cmd);
    printf("     Return code: %d\n", ret >> 8);
    
    /* 4e: Test -r flag (dump raw) - sets flag_dump_raw */
    printf("   e) Testing -r flag (dump raw)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -r %s", gcda_file);
    ret = system(cmd);
    printf("     Return code: %d\n", ret >> 8);
    
    /* 4f: Test -s flag (dump stable) - sets flag_dump_stable */
    printf("   f) Testing -s flag (dump stable)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -s %s", gcda_file);
    ret = system(cmd);
    printf("     Return code: %d\n", ret >> 8);
    
    /* 4g: Test combined flags -l -p */
    printf("   g) Testing combined flags -l -p...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -p %s", gcda_file);
    ret = system(cmd);
    printf("     Return code: %d\n", ret >> 8);
    
    /* 4h: Test invalid flag - triggers default case with fprintf */
    printf("   h) Testing invalid flag -X (should trigger 'unknown flag' error)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X %s 2>&1", gcda_file);
    
    /* Use popen to capture stderr output */
    FILE *fp = popen(cmd, "r");
    if (fp) {
        int found_error = 0;
        while (fgets(output, sizeof(output), fp) != NULL) {
            if (strstr(output, "unknown flag") != NULL) {
                printf("     SUCCESS: Found 'unknown flag' message: %s", output);
                found_error = 1;
            }
        }
        ret = pclose(fp);
        if (!found_error) {
            printf("     WARNING: 'unknown flag' message not found\n");
        }
        printf("     Return code: %d\n", ret >> 8);
    } else {
        fprintf(stderr, "Failed to execute command: %s\n", cmd);
    }
    
    /* 4i: Test another invalid flag with different letter */
    printf("   i) Testing invalid flag -z...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -z %s 2>&1", gcda_file);
    fp = popen(cmd, "r");
    if (fp) {
        int found_error = 0;
        while (fgets(output, sizeof(output), fp) != NULL) {
            if (strstr(output, "unknown flag") != NULL) {
                printf("     SUCCESS: Found 'unknown flag' message: %s", output);
                found_error = 1;
            }
        }
        ret = pclose(fp);
        if (!found_error) {
            printf("     WARNING: 'unknown flag' message not found\n");
        }
        printf("     Return code: %d\n", ret >> 8);
    }
    
    /* 4j: Test flag with no argument (should show usage) */
    printf("   j) Testing gcov-dump with no arguments...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump 2>&1");
    ret = system(cmd);
    printf("     Return code: %d\n", ret >> 8);
    
    printf("\n5. Cleanup...\n");
    cleanup_files(base_name);
    
    printf("\n=== Test completed ===\n");
    printf("All gcov-dump command-line flags have been exercised.\n");
    printf("Check gcov-dump's own coverage to verify the switch cases were hit.\n");
    
    return 0;
}

/**
 * Create a simple C source file for coverage testing
 */
int create_test_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen");
        return 0;
    }
    
    fprintf(fp, "/* Test program for gcov-dump flag coverage */\n");
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int factorial(int n) {\n");
    fprintf(fp, "    if (n <= 1) return 1;\n");
    fprintf(fp, "    return n * factorial(n - 1);\n");
    fprintf(fp, "}\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i, sum = 0;\n");
    fprintf(fp, "    /* Loop to generate arc coverage */\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        sum += i;\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    printf(\"Sum: %%d\\n\", sum);\n");
    fprintf(fp, "    printf(\"Factorial 5: %%d\\n\", factorial(5));\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 1;
}

/**
 * Compile the test program with coverage flags
 */
int compile_with_coverage(const char *source_file, const char *binary_name) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s",
             source_file, binary_name);
    
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Compilation failed: %s\n", cmd);
        return 0;
    }
    
    /* Verify binary was created */
    struct stat st;
    if (stat(binary_name, &st) != 0) {
        fprintf(stderr, "Binary not created: %s\n", binary_name);
        return 0;
    }
    
    return 1;
}

/**
 * Execute the test program to generate .gcda file
 */
int execute_test_program(const char *binary_name) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "./%s", binary_name);
    
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Execution failed: %s\n", cmd);
        return 0;
    }
    
    return 1;
}

/**
 * Clean up temporary files
 */
void cleanup_files(const char *base_name) {
    char cmd[MAX_CMD_LEN];
    const char *extensions[] = {".c", ".exe", ".gcda", ".gcno", ".o"};
    int i;
    
    for (i = 0; i < sizeof(extensions)/sizeof(extensions[0]); i++) {
        snprintf(cmd, sizeof(cmd), "rm -f %s%s", base_name, extensions[i]);
        system(cmd);
    }
    
    printf("   Cleaned up temporary files with base: %s\n", base_name);
}
