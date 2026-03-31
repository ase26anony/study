/**
 * Test driver for gcov-dump command-line flag coverage
 * Targets lines 111-130 in gcov-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_CMD_LEN 1024
#define TEMP_FILENAME "test_coverage_XXXXXX"

/* Simple test program source code to generate coverage data */
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
void create_temp_filename(char *template, const char *suffix) {
    int fd = mkstemp(template);
    if (fd != -1) {
        close(fd);
        unlink(template);  /* Remove the temporary file created by mkstemp */
        strcat(template, suffix);
    }
}

/* Execute a command and check return status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Execute command and capture stderr to verify error message */
int execute_and_check_error(const char *cmd, const char *expected_error) {
    char buffer[256];
    FILE *fp;
    int found = 0;
    
    printf("Executing (checking stderr): %s\n", cmd);
    
    /* Redirect stderr to stdout and capture */
    char full_cmd[MAX_CMD_LEN];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    fp = popen(full_cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return 0;
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, expected_error) != NULL) {
            found = 1;
            printf("Found expected error: %s", buffer);
        }
    }
    
    pclose(fp);
    return found;
}

/* Clean up temporary files */
void cleanup_files(const char *base_name) {
    char cmd[MAX_CMD_LEN];
    
    /* Remove all generated files */
    const char *extensions[] = {".c", ".gcda", ".gcno", "", NULL};
    
    for (int i = 0; extensions[i] != NULL; i++) {
        snprintf(cmd, sizeof(cmd), "rm -f %s%s", base_name, extensions[i]);
        system(cmd);
    }
}

int main(int argc, char *argv[]) {
    char source_file[64];
    char gcda_file[64];
    char gcno_file[64];
    char binary_file[64];
    char cmd[MAX_CMD_LEN];
    int ret;
    
    printf("=== Starting gcov-dump flag coverage test ===\n\n");
    
    /* Create unique temporary filenames */
    strcpy(source_file, TEMP_FILENAME);
    create_temp_filename(source_file, ".c");
    
    /* Use the same base name for all files */
    char base_name[64];
    strcpy(base_name, source_file);
    base_name[strlen(base_name) - 2] = '\0';  /* Remove ".c" */
    
    strcpy(gcda_file, base_name);
    strcat(gcda_file, ".gcda");
    
    strcpy(gcno_file, base_name);
    strcat(gcno_file, ".gcno");
    
    strcpy(binary_file, base_name);
    
    printf("Using temporary files with base: %s\n\n", base_name);
    
    /* Step 1: Create test source file */
    printf("1. Creating test source file: %s\n", source_file);
    FILE *fp = fopen(source_file, "w");
    if (!fp) {
        perror("Failed to create source file");
        return 1;
    }
    fprintf(fp, "%s", test_source);
    fclose(fp);
    
    /* Step 2: Compile with coverage instrumentation */
    printf("2. Compiling with coverage flags\n");
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s",
             source_file, binary_file);
    ret = execute_command(cmd);
    if (ret != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup_files(base_name);
        return 1;
    }
    
    /* Step 3: Execute to generate .gcda file */
    printf("3. Executing program to generate .gcda file\n");
    snprintf(cmd, sizeof(cmd), "./%s", binary_file);
    ret = execute_command(cmd);
    if (ret != 0) {
        fprintf(stderr, "Execution failed\n");
        cleanup_files(base_name);
        return 1;
    }
    
    /* Verify .gcda file was created */
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, ".gcda file not created: %s\n", gcda_file);
        cleanup_files(base_name);
        return 1;
    }
    printf("Generated: %s (size: %ld bytes)\n\n", gcda_file, st.st_size);
    
    /* Step 4: Test gcov-dump with various flags */
    printf("4. Testing gcov-dump command-line flags\n");
    
    /* 4a: Test -h flag (help) - triggers print_usage() */
    printf("\n4a. Testing -h flag (help):\n");
    execute_command("gcov-dump -h");
    
    /* 4b: Test -v flag (version) - triggers print_version() */
    printf("\n4b. Testing -v flag (version):\n");
    execute_command("gcov-dump -v");
    
    /* 4c: Test -l flag (dump contents) - sets flag_dump_contents = 1 */
    printf("\n4c. Testing -l flag (dump contents):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_file);
    execute_command(cmd);
    
    /* 4d: Test -p flag (dump positions) - sets flag_dump_positions = 1 */
    printf("\n4d. Testing -p flag (dump positions):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -p %s", gcda_file);
    execute_command(cmd);
    
    /* 4e: Test -r flag (dump raw) - sets flag_dump_raw = 1 */
    printf("\n4e. Testing -r flag (dump raw):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -r %s", gcda_file);
    execute_command(cmd);
    
    /* 4f: Test -s flag (dump stable) - sets flag_dump_stable = 1 */
    printf("\n4f. Testing -s flag (dump stable):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -s %s", gcda_file);
    execute_command(cmd);
    
    /* 4g: Test combined flags - both -l and -p */
    printf("\n4g. Testing combined flags (-l -p):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -p %s", gcda_file);
    execute_command(cmd);
    
    /* 4h: Test invalid flag - triggers default case and fprintf */
    printf("\n4h. Testing invalid flag (-X):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X %s", gcda_file);
    if (execute_and_check_error(cmd, "unknown flag")) {
        printf("SUCCESS: Default case triggered for invalid flag\n");
    } else {
        printf("WARNING: Expected error message not found\n");
    }
    
    /* 4i: Test another invalid flag variation */
    printf("\n4i. Testing another invalid flag (-z):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -z %s", gcda_file);
    if (execute_and_check_error(cmd, "unknown flag")) {
        printf("SUCCESS: Default case triggered for invalid flag\n");
    } else {
        printf("WARNING: Expected error message not found\n");
    }
    
    /* 4j: Test flag without required file argument */
    printf("\n4j. Testing flag without file argument:\n");
    execute_command("gcov-dump -l");
    
    /* Step 5: Cleanup */
    printf("\n5. Cleaning up temporary files\n");
    cleanup_files(base_name);
    
    printf("\n=== Test completed ===\n");
    printf("All gcov-dump command-line flags have been exercised.\n");
    printf("Target lines 111-130 in gcov-dump.cc should now be covered.\n");
    
    return 0;
}
