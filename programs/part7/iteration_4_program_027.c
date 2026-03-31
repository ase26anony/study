/**
 * Test driver for gcov-dump command-line flag coverage
 * Targets specific switch cases in gcov-dump.cc lines 111-130
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define TEMP_FILENAME "test_coverage_XXXXXX"

/* Simple test program source code that will generate coverage data */
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
 * Create a temporary file with given content
 * Returns dynamically allocated filename or NULL on error
 */
char *create_temp_file(const char *content, const char *suffix) {
    char *template = malloc(strlen(TEMP_FILENAME) + strlen(suffix) + 1);
    if (!template) return NULL;
    
    strcpy(template, TEMP_FILENAME);
    strcat(template, suffix);
    
    int fd = mkstemps(template, strlen(suffix));
    if (fd < 0) {
        free(template);
        return NULL;
    }
    
    FILE *f = fdopen(fd, "w");
    if (!f) {
        close(fd);
        free(template);
        return NULL;
    }
    
    fwrite(content, 1, strlen(content), f);
    fclose(f);
    
    return template;
}

/**
 * Execute a command and capture its stderr
 * Returns 1 if stderr contains "unknown flag", 0 otherwise
 */
int check_for_unknown_flag(const char *cmd) {
    char full_cmd[MAX_CMD_LEN];
    char buffer[256];
    int found = 0;
    
    /* Redirect stderr to stdout for capture */
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    FILE *pipe = popen(full_cmd, "r");
    if (!pipe) {
        fprintf(stderr, "Failed to execute command: %s\n", cmd);
        return 0;
    }
    
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        if (strstr(buffer, "unknown flag")) {
            found = 1;
            printf("  ✓ Successfully triggered 'unknown flag' error: %s", buffer);
        }
    }
    
    pclose(pipe);
    return found;
}

/**
 * Execute a command and check return status
 */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    
    if (status != 0) {
        printf("  Command returned status: %d\n", WEXITSTATUS(status));
    }
    
    return status;
}

int main(int argc, char *argv[]) {
    char *source_file = NULL;
    char *binary_file = NULL;
    char gcda_file[256];
    char gcno_file[256];
    char cmd[MAX_CMD_LEN];
    int ret = 0;
    
    printf("=== gcov-dump Command Line Flag Coverage Test ===\n\n");
    
    /* Step 1: Create test source file */
    printf("1. Creating test source file...\n");
    source_file = create_temp_file(test_source, ".c");
    if (!source_file) {
        fprintf(stderr, "Failed to create source file\n");
        return 1;
    }
    printf("   Created: %s\n", source_file);
    
    /* Step 2: Create binary filename */
    binary_file = malloc(strlen(source_file) + 1);
    if (!binary_file) {
        fprintf(stderr, "Memory allocation failed\n");
        ret = 1;
        goto cleanup;
    }
    strcpy(binary_file, source_file);
    char *dot = strrchr(binary_file, '.');
    if (dot) *dot = '\0';
    
    /* Step 3: Compile with coverage instrumentation */
    printf("\n2. Compiling with coverage flags...\n");
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s 2>/dev/null", 
             source_file, binary_file);
    
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Compilation failed\n");
        ret = 1;
        goto cleanup;
    }
    printf("   Compiled: %s\n", binary_file);
    
    /* Step 4: Execute to generate coverage data */
    printf("\n3. Executing program to generate .gcda file...\n");
    snprintf(cmd, sizeof(cmd), "./%s", binary_file);
    execute_command(cmd);
    
    /* Step 5: Verify .gcda file exists */
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", binary_file);
    snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", binary_file);
    
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "Failed to generate .gcda file: %s\n", gcda_file);
        ret = 1;
        goto cleanup;
    }
    printf("   Generated: %s (size: %ld bytes)\n", gcda_file, st.st_size);
    
    if (stat(gcno_file, &st) != 0) {
        fprintf(stderr, "Failed to generate .gcno file: %s\n", gcno_file);
        ret = 1;
        goto cleanup;
    }
    printf("   Generated: %s (size: %ld bytes)\n", gcno_file, st.st_size);
    
    /* Step 6: Test gcov-dump with various flags */
    printf("\n4. Testing gcov-dump command-line flags...\n");
    
    /* Test -h flag (help) - triggers print_usage() */
    printf("\n   Testing -h flag (help):\n");
    execute_command("gcov-dump -h");
    
    /* Test -v flag (version) - triggers print_version() */
    printf("\n   Testing -v flag (version):\n");
    execute_command("gcov-dump -v");
    
    /* Test -l flag (dump contents) - sets flag_dump_contents = 1 */
    printf("\n   Testing -l flag (dump contents):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_file);
    execute_command(cmd);
    
    /* Test -p flag (dump positions) - sets flag_dump_positions = 1 */
    printf("\n   Testing -p flag (dump positions):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -p %s", gcda_file);
    execute_command(cmd);
    
    /* Test -r flag (dump raw) - sets flag_dump_raw = 1 */
    printf("\n   Testing -r flag (dump raw):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -r %s", gcda_file);
    execute_command(cmd);
    
    /* Test -s flag (dump stable) - sets flag_dump_stable = 1 */
    printf("\n   Testing -s flag (dump stable):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -s %s", gcda_file);
    execute_command(cmd);
    
    /* Test combined flags -l -p */
    printf("\n   Testing combined flags -l -p:\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -p %s", gcda_file);
    execute_command(cmd);
    
    /* Test with .gcno file instead of .gcda */
    printf("\n   Testing with .gcno file:\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcno_file);
    execute_command(cmd);
    
    /* Test invalid flag - triggers default case and fprintf */
    printf("\n   Testing invalid flag -X (should trigger 'unknown flag' error):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X %s", gcda_file);
    if (!check_for_unknown_flag(cmd)) {
        printf("  ✗ Did not detect 'unknown flag' error message\n");
        printf("  Note: Some gcov-dump versions may handle invalid flags differently\n");
    }
    
    /* Test another invalid flag with popen to capture stderr */
    printf("\n   Testing invalid flag -z with stderr capture:\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -z %s", gcda_file);
    check_for_unknown_flag(cmd);
    
    /* Test flag without required file argument */
    printf("\n   Testing -l flag without file (should show usage or error):\n");
    execute_command("gcov-dump -l");
    
    printf("\n=== Test Complete ===\n");
    
cleanup:
    /* Cleanup temporary files */
    printf("\n5. Cleaning up temporary files...\n");
    if (source_file) {
        unlink(source_file);
        printf("   Removed: %s\n", source_file);
        free(source_file);
    }
    
    if (binary_file) {
        unlink(binary_file);
        printf("   Removed: %s\n", binary_file);
        
        snprintf(cmd, sizeof(cmd), "%s.gcda", binary_file);
        unlink(cmd);
        printf("   Removed: %s\n", cmd);
        
        snprintf(cmd, sizeof(cmd), "%s.gcno", binary_file);
        unlink(cmd);
        printf("   Removed: %s\n", cmd);
        
        free(binary_file);
    }
    
    return ret;
}
