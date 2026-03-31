/**
 * Test driver for gcov-dump command-line parsing
 * Specifically targets uncovered lines 111-130 in gcov-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH 1024
#define MAX_CMD 2048

/* Simple test program that will generate coverage data */
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

/* Function to check if a file exists */
int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

/* Execute a command and return its exit status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status == -1) {
        perror("system() failed");
        return -1;
    }
    return WEXITSTATUS(status);
}

/* Execute command and capture stderr to check for specific output */
int execute_and_check_stderr(const char *cmd, const char *expected_error) {
    char buffer[1024];
    FILE *fp;
    int found = 0;
    
    printf("Executing (checking stderr): %s\n", cmd);
    
    /* Redirect stderr to stdout and capture */
    char full_cmd[MAX_CMD];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    fp = popen(full_cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return -1;
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, expected_error) != NULL) {
            found = 1;
            printf("Found expected error message: %s", buffer);
        }
    }
    
    int status = pclose(fp);
    if (status == -1) {
        perror("pclose failed");
        return -1;
    }
    
    return found ? 0 : -1;
}

/* Clean up temporary files */
void cleanup_files(const char *base_name) {
    char cmd[MAX_CMD];
    
    /* Remove all generated files */
    const char *extensions[] = {".c", "", ".gcda", ".gcno", ".gcov", NULL};
    
    for (int i = 0; extensions[i] != NULL; i++) {
        char path[MAX_PATH];
        if (extensions[i][0] == '\0') {
            snprintf(path, sizeof(path), "%s", base_name);  /* The executable */
        } else {
            snprintf(path, sizeof(path), "%s%s", base_name, extensions[i]);
        }
        
        if (file_exists(path)) {
            snprintf(cmd, sizeof(cmd), "rm -f \"%s\"", path);
            system(cmd);
        }
    }
}

int main(int argc, char *argv[]) {
    char source_file[MAX_PATH];
    char gcda_file[MAX_PATH];
    char gcno_file[MAX_PATH];
    char exe_file[MAX_PATH];
    char cmd[MAX_CMD];
    int ret;
    
    /* Create unique temporary filenames */
    const char *temp_base = "test_gcov_dump_XXXXXX";
    char temp_dir[] = "/tmp/test_gcov_dump_XXXXXX";
    
    /* Create a temporary directory for our test files */
    if (mkdtemp(temp_dir) == NULL) {
        perror("mkdtemp failed");
        return 1;
    }
    
    /* Set up file paths */
    snprintf(source_file, sizeof(source_file), "%s/test_coverage.c", temp_dir);
    snprintf(exe_file, sizeof(exe_file), "%s/test_coverage", temp_dir);
    snprintf(gcda_file, sizeof(gcda_file), "%s/test_coverage.gcda", temp_dir);
    snprintf(gcno_file, sizeof(gcno_file), "%s/test_coverage.gcno", temp_dir);
    
    printf("Test directory: %s\n", temp_dir);
    
    /* Step 1: Create test source file */
    FILE *fp = fopen(source_file, "w");
    if (fp == NULL) {
        perror("Failed to create source file");
        cleanup_files(source_file);
        return 1;
    }
    fputs(test_source, fp);
    fclose(fp);
    
    /* Step 2: Compile with coverage instrumentation */
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 \"%s\" -o \"%s\"",
             source_file, exe_file);
    ret = execute_command(cmd);
    if (ret != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup_files(source_file);
        return 1;
    }
    
    /* Step 3: Execute to generate .gcda file */
    snprintf(cmd, sizeof(cmd), "\"%s\"", exe_file);
    ret = execute_command(cmd);
    if (ret != 0) {
        fprintf(stderr, "Execution failed\n");
        cleanup_files(source_file);
        return 1;
    }
    
    /* Verify .gcda file was created */
    if (!file_exists(gcda_file)) {
        fprintf(stderr, "Failed to generate .gcda file\n");
        cleanup_files(source_file);
        return 1;
    }
    
    printf("\n=== Starting gcov-dump tests ===\n\n");
    
    /* Test 1: -h flag (help) - triggers print_usage() */
    printf("Test 1: -h flag (help)\n");
    execute_command("gcov-dump -h");
    printf("\n");
    
    /* Test 2: -v flag (version) - triggers print_version() */
    printf("Test 2: -v flag (version)\n");
    execute_command("gcov-dump -v");
    printf("\n");
    
    /* Test 3: -l flag (dump contents) - sets flag_dump_contents = 1 */
    printf("Test 3: -l flag (dump contents)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l \"%s\"", gcda_file);
    execute_command(cmd);
    printf("\n");
    
    /* Test 4: -p flag (dump positions) - sets flag_dump_positions = 1 */
    printf("Test 4: -p flag (dump positions)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -p \"%s\"", gcda_file);
    execute_command(cmd);
    printf("\n");
    
    /* Test 5: -r flag (dump raw) - sets flag_dump_raw = 1 */
    printf("Test 5: -r flag (dump raw)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -r \"%s\"", gcda_file);
    execute_command(cmd);
    printf("\n");
    
    /* Test 6: -s flag (dump stable) - sets flag_dump_stable = 1 */
    printf("Test 6: -s flag (dump stable)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -s \"%s\"", gcda_file);
    execute_command(cmd);
    printf("\n");
    
    /* Test 7: Combined flags -l -p */
    printf("Test 7: Combined flags -l -p\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -p \"%s\"", gcda_file);
    execute_command(cmd);
    printf("\n");
    
    /* Test 8: Invalid flag - triggers default case and fprintf */
    printf("Test 8: Invalid flag -X (should trigger 'unknown flag' error)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X \"%s\"", gcda_file);
    ret = execute_and_check_stderr(cmd, "unknown flag");
    if (ret == 0) {
        printf("SUCCESS: Invalid flag correctly triggered error message\n");
    } else {
        printf("WARNING: Did not see expected error message for invalid flag\n");
    }
    printf("\n");
    
    /* Test 9: Test with .gcno file instead of .gcda */
    printf("Test 9: Testing with .gcno file\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l \"%s\"", gcno_file);
    execute_command(cmd);
    printf("\n");
    
    /* Test 10: Multiple invalid flags */
    printf("Test 10: Multiple invalid flags\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X -Y -Z \"%s\"", gcda_file);
    execute_and_check_stderr(cmd, "unknown flag");
    printf("\n");
    
    /* Test 11: Valid flag with invalid file */
    printf("Test 11: Valid flag with non-existent file\n");
    execute_command("gcov-dump -l nonexistent.gcda");
    printf("\n");
    
    /* Test 12: No flags, just file (tests default behavior) */
    printf("Test 12: No flags, just file argument\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump \"%s\"", gcda_file);
    execute_command(cmd);
    printf("\n");
    
    /* Cleanup */
    printf("=== Cleaning up test files ===\n");
    cleanup_files(source_file);
    
    /* Remove the temporary directory */
    snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", temp_dir);
    system(cmd);
    
    printf("\nAll tests completed. Check coverage of gcov-dump.cc lines 111-130.\n");
    
    return 0;
}
