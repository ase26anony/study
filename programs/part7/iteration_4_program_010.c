/**
 * test_gcov_dump_switches.c
 * 
 * Test driver to exercise specific command-line switch cases in gcov-dump
 * Targets lines 111-130 in gcov-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define TEMP_FILENAME "test_gcov_dump_coverage"

/* Simple test program that will be compiled with coverage flags */
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

/**
 * Execute a system command and return its exit status
 */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        printf("Command failed with status %d\n", status);
    }
    return status;
}

/**
 * Execute command and capture stderr to check for specific error message
 * Returns 1 if error message found, 0 otherwise
 */
int check_error_message(const char *cmd, const char *expected_error) {
    char full_cmd[MAX_CMD_LEN];
    char buffer[256];
    FILE *fp;
    int found = 0;
    
    /* Redirect stderr to stdout and capture */
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    printf("Executing (checking stderr): %s\n", cmd);
    
    fp = popen(full_cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return 0;
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, expected_error) != NULL) {
            printf("Found expected error: %s", buffer);
            found = 1;
        }
    }
    
    pclose(fp);
    return found;
}

/**
 * Create a temporary file with given content
 */
int create_temp_file(const char *filename, const char *content) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("Failed to create temp file");
        return 0;
    }
    fputs(content, fp);
    fclose(fp);
    return 1;
}

/**
 * Check if file exists
 */
int file_exists(const char *filename) {
    struct stat st;
    return (stat(filename, &st) == 0);
}

/**
 * Clean up temporary files
 */
void cleanup_files(void) {
    /* Remove all generated files */
    remove(TEMP_FILENAME ".c");
    remove(TEMP_FILENAME);
    remove(TEMP_FILENAME ".gcda");
    remove(TEMP_FILENAME ".gcno");
    remove(TEMP_FILENAME ".gcov");
}

int main(void) {
    char cmd[MAX_CMD_LEN];
    int ret;
    
    printf("=== Starting gcov-dump switch coverage test ===\n\n");
    
    /* Step 1: Create test source file */
    printf("1. Creating test source file...\n");
    if (!create_temp_file(TEMP_FILENAME ".c", test_source)) {
        fprintf(stderr, "Failed to create test source file\n");
        return 1;
    }
    
    /* Step 2: Compile with coverage instrumentation */
    printf("\n2. Compiling with coverage flags...\n");
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s.c -o %s -O0",
             TEMP_FILENAME, TEMP_FILENAME);
    ret = execute_command(cmd);
    if (ret != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup_files();
        return 1;
    }
    
    /* Step 3: Execute to generate .gcda file */
    printf("\n3. Executing test program to generate coverage data...\n");
    snprintf(cmd, sizeof(cmd), "./%s", TEMP_FILENAME);
    ret = execute_command(cmd);
    if (ret != 0) {
        fprintf(stderr, "Execution failed\n");
        cleanup_files();
        return 1;
    }
    
    /* Verify .gcda file was created */
    if (!file_exists(TEMP_FILENAME ".gcda")) {
        fprintf(stderr, "Coverage data file not created\n");
        cleanup_files();
        return 1;
    }
    printf("Generated %s.gcda successfully\n", TEMP_FILENAME);
    
    /* Step 4: Test gcov-dump with various switches */
    printf("\n4. Testing gcov-dump command-line switches...\n");
    
    /* 4a: Test -h (help) - triggers print_usage() */
    printf("\n4a. Testing -h flag (should trigger print_usage)...\n");
    execute_command("gcov-dump -h");
    
    /* 4b: Test -v (version) - triggers print_version() */
    printf("\n4b. Testing -v flag (should trigger print_version)...\n");
    execute_command("gcov-dump -v");
    
    /* 4c: Test -l (dump contents) with .gcda file */
    printf("\n4c. Testing -l flag (should set flag_dump_contents)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s.gcda", TEMP_FILENAME);
    execute_command(cmd);
    
    /* 4d: Test -p (dump positions) with .gcda file */
    printf("\n4d. Testing -p flag (should set flag_dump_positions)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -p %s.gcda", TEMP_FILENAME);
    execute_command(cmd);
    
    /* 4e: Test -r (dump raw) with .gcda file */
    printf("\n4e. Testing -r flag (should set flag_dump_raw)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -r %s.gcda", TEMP_FILENAME);
    execute_command(cmd);
    
    /* 4f: Test -s (dump stable) with .gcda file */
    printf("\n4f. Testing -s flag (should set flag_dump_stable)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -s %s.gcda", TEMP_FILENAME);
    execute_command(cmd);
    
    /* 4g: Test combination of flags */
    printf("\n4g. Testing combination -l -p flags...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -p %s.gcda", TEMP_FILENAME);
    execute_command(cmd);
    
    /* 4h: Test invalid flag - triggers default case with fprintf */
    printf("\n4h. Testing invalid flag -X (should trigger default case)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X %s.gcda", TEMP_FILENAME);
    if (check_error_message(cmd, "unknown flag")) {
        printf("SUCCESS: Invalid flag triggered expected error message\n");
    } else {
        printf("WARNING: Invalid flag might not have triggered expected error\n");
    }
    
    /* Additional test: Try with .gcno file as well */
    printf("\n4i. Testing with .gcno file...\n");
    if (file_exists(TEMP_FILENAME ".gcno")) {
        snprintf(cmd, sizeof(cmd), "gcov-dump -l %s.gcno", TEMP_FILENAME);
        execute_command(cmd);
    }
    
    /* Step 5: Test edge cases */
    printf("\n5. Testing edge cases...\n");
    
    /* Test with no arguments (should show usage or error) */
    printf("\n5a. Testing with no arguments...\n");
    execute_command("gcov-dump");
    
    /* Test with multiple invalid flags */
    printf("\n5b. Testing with multiple invalid flags...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X -Y -Z %s.gcda", TEMP_FILENAME);
    check_error_message(cmd, "unknown flag");
    
    /* Test flag in different position */
    printf("\n5c. Testing flag after filename...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump %s.gcda -l", TEMP_FILENAME);
    execute_command(cmd);
    
    printf("\n=== Test completed ===\n");
    
    /* Cleanup */
    cleanup_files();
    
    return 0;
}
