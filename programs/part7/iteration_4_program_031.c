/**
 * test_gcov_dump_switches.c
 * 
 * Test driver to exercise the command-line switch cases in gcov-dump.cc
 * Specifically targets lines 111-130 covering flags: h, v, l, p, r, s, and invalid flags
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define MAX_PATH_LEN 256

/* Simple test program to generate coverage data */
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

/**
 * Create a temporary file with given content
 * Returns dynamically allocated filename or NULL on error
 */
char *create_temp_file(const char *content, const char *suffix) {
    char template[] = "/tmp/gcov_test_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }
    
    /* Write content */
    if (content) {
        write(fd, content, strlen(content));
    }
    
    close(fd);
    
    /* Add suffix if provided */
    if (suffix) {
        char *new_name = malloc(strlen(template) + strlen(suffix) + 1);
        if (!new_name) {
            unlink(template);
            return NULL;
        }
        strcpy(new_name, template);
        strcat(new_name, suffix);
        unlink(template);  /* Remove original */
        return new_name;
    }
    
    char *name = strdup(template);
    return name;
}

/**
 * Execute a command and capture stderr
 * Returns 1 if "unknown flag" found in stderr, 0 otherwise
 */
int execute_and_check_stderr(const char *cmd) {
    char buf[1024];
    int found_unknown = 0;
    
    /* Use popen to capture stderr (2>&1 redirects stderr to stdout) */
    char full_cmd[MAX_CMD_LEN];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    FILE *fp = popen(full_cmd, "r");
    if (!fp) {
        perror("popen failed");
        return 0;
    }
    
    printf("Executing: %s\n", cmd);
    
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        /* Check for "unknown flag" message */
        if (strstr(buf, "unknown flag")) {
            found_unknown = 1;
            printf("  -> Got expected 'unknown flag' message: %s", buf);
        }
    }
    
    pclose(fp);
    return found_unknown;
}

/**
 * Execute a command without capturing output
 */
void execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int ret = system(cmd);
    if (ret != 0) {
        printf("  -> Command returned %d\n", ret);
    }
}

int main(int argc, char *argv[]) {
    char *source_file = NULL;
    char *binary_file = NULL;
    char *gcda_file = NULL;
    char *gcno_file = NULL;
    char cmd[MAX_CMD_LEN];
    int ret = 0;
    
    printf("=== Test Driver for gcov-dump Switch Cases ===\n\n");
    
    /* Step 1: Create test source file */
    printf("1. Creating test source file...\n");
    source_file = create_temp_file(test_program, ".c");
    if (!source_file) {
        fprintf(stderr, "Failed to create source file\n");
        return 1;
    }
    printf("   Source: %s\n", source_file);
    
    /* Step 2: Create binary file name */
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
             "gcc -fprofile-arcs -ftest-coverage %s -o %s",
             source_file, binary_file);
    execute_command(cmd);
    
    /* Step 4: Execute to generate .gcda file */
    printf("\n3. Executing test program to generate coverage data...\n");
    execute_command(binary_file);
    
    /* Step 5: Verify .gcda and .gcno files exist */
    gcda_file = malloc(strlen(binary_file) + 6);
    gcno_file = malloc(strlen(binary_file) + 6);
    if (!gcda_file || !gcno_file) {
        fprintf(stderr, "Memory allocation failed\n");
        ret = 1;
        goto cleanup;
    }
    
    sprintf(gcda_file, "%s.gcda", binary_file);
    sprintf(gcno_file, "%s.gcno", binary_file);
    
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "Failed to generate .gcda file: %s\n", gcda_file);
        ret = 1;
        goto cleanup;
    }
    printf("   Generated: %s\n", gcda_file);
    
    if (stat(gcno_file, &st) != 0) {
        fprintf(stderr, "Failed to generate .gcno file: %s\n", gcno_file);
        ret = 1;
        goto cleanup;
    }
    printf("   Generated: %s\n", gcno_file);
    
    /* ============================================================
     * Step 6: Test gcov-dump with various flags to trigger switch cases
     * ============================================================ */
    
    printf("\n4. Testing gcov-dump command-line switches...\n");
    
    /* Test 1: -h flag (help) - triggers print_usage() */
    printf("\n   Test 1: -h flag (help)\n");
    execute_command("gcov-dump -h");
    
    /* Test 2: -v flag (version) - triggers print_version() */
    printf("\n   Test 2: -v flag (version)\n");
    execute_command("gcov-dump -v");
    
    /* Test 3: -l flag (dump contents) - sets flag_dump_contents = 1 */
    printf("\n   Test 3: -l flag (dump contents)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_file);
    execute_command(cmd);
    
    /* Test 4: -p flag (dump positions) - sets flag_dump_positions = 1 */
    printf("\n   Test 4: -p flag (dump positions)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -p %s", gcda_file);
    execute_command(cmd);
    
    /* Test 5: -r flag (dump raw) - sets flag_dump_raw = 1 */
    printf("\n   Test 5: -r flag (dump raw)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -r %s", gcda_file);
    execute_command(cmd);
    
    /* Test 6: -s flag (dump stable) - sets flag_dump_stable = 1 */
    printf("\n   Test 6: -s flag (dump stable)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -s %s", gcda_file);
    execute_command(cmd);
    
    /* Test 7: Combined flags - tests multiple flag processing */
    printf("\n   Test 7: Combined flags (-l -p)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -p %s", gcda_file);
    execute_command(cmd);
    
    /* Test 8: Invalid flag - triggers default case and fprintf */
    printf("\n   Test 8: Invalid flag (-X)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X %s", gcda_file);
    int found = execute_and_check_stderr(cmd);
    if (!found) {
        printf("  WARNING: Did not see 'unknown flag' message\n");
    } else {
        printf("  SUCCESS: Triggered default case with 'unknown flag' message\n");
    }
    
    /* Test 9: Test with .gcno file as well */
    printf("\n   Test 9: Testing with .gcno file (-l flag)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcno_file);
    execute_command(cmd);
    
    /* Test 10: Another invalid flag test with different letter */
    printf("\n   Test 10: Another invalid flag (-z)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -z %s", gcda_file);
    found = execute_and_check_stderr(cmd);
    if (!found) {
        printf("  WARNING: Did not see 'unknown flag' message\n");
    }
    
    printf("\n=== All tests completed ===\n");
    
cleanup:
    /* Cleanup temporary files */
    printf("\n5. Cleaning up temporary files...\n");
    if (source_file) {
        unlink(source_file);
        free(source_file);
    }
    if (binary_file) {
        unlink(binary_file);
        free(binary_file);
    }
    if (gcda_file) {
        unlink(gcda_file);
        free(gcda_file);
    }
    if (gcno_file) {
        unlink(gcno_file);
        free(gcno_file);
    }
    
    return ret;
}
