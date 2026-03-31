/**
 * test_gcov_dump_switch.c
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

#define MAX_PATH 1024
#define MAX_CMD 2048

/* Simple test program to generate coverage data */
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
    char *filename = malloc(MAX_PATH);
    if (!filename) return NULL;
    
    /* Create a unique filename */
    snprintf(filename, MAX_PATH, "/tmp/gcov_test_XXXXXX%s", suffix);
    int fd = mkstemps(filename, strlen(suffix));
    if (fd < 0) {
        free(filename);
        return NULL;
    }
    
    /* Write content */
    if (content) {
        write(fd, content, strlen(content));
    }
    
    close(fd);
    return filename;
}

/**
 * Execute a command and capture stderr
 * Returns 1 if "unknown flag" found in stderr, 0 otherwise
 */
int check_for_unknown_flag(const char *cmd) {
    char full_cmd[MAX_CMD];
    char buffer[256];
    int found = 0;
    
    /* Redirect stderr to stdout for capture */
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    FILE *fp = popen(full_cmd, "r");
    if (!fp) {
        fprintf(stderr, "Failed to execute: %s\n", cmd);
        return 0;
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, "unknown flag")) {
            found = 1;
            printf("  ✓ Successfully triggered 'unknown flag' message: %s", buffer);
        }
    }
    
    pclose(fp);
    return found;
}

/**
 * Execute a command and print status
 */
void execute_command(const char *description, const char *cmd, int check_error) {
    printf("\n%s\n", description);
    printf("Command: %s\n", cmd);
    
    if (check_error) {
        if (check_for_unknown_flag(cmd)) {
            printf("  ✓ Successfully triggered default case (invalid flag)\n");
        } else {
            printf("  ✗ Did not find expected 'unknown flag' message\n");
        }
    } else {
        int result = system(cmd);
        if (result != 0) {
            printf("  Command exited with code: %d\n", result);
        }
    }
}

int main(int argc, char *argv[]) {
    char *source_file = NULL;
    char *binary_file = NULL;
    char *gcda_file = NULL;
    char *gcno_file = NULL;
    char cmd[MAX_CMD];
    int ret = 0;
    
    printf("=== Testing gcov-dump switch cases (lines 111-130) ===\n");
    
    /* Step 1: Create test source file */
    printf("\n1. Creating test source file...\n");
    source_file = create_temp_file(test_source, ".c");
    if (!source_file) {
        fprintf(stderr, "Failed to create source file\n");
        ret = 1;
        goto cleanup;
    }
    printf("  Created: %s\n", source_file);
    
    /* Step 2: Compile with coverage instrumentation */
    printf("\n2. Compiling with coverage flags...\n");
    binary_file = malloc(MAX_PATH);
    snprintf(binary_file, MAX_PATH, "%s.bin", source_file);
    
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s 2>&1",
             source_file, binary_file);
    
    printf("  Compile command: %s\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Compilation failed\n");
        ret = 1;
        goto cleanup;
    }
    printf("  Compiled: %s\n", binary_file);
    
    /* Step 3: Execute to generate .gcda file */
    printf("\n3. Executing test program to generate coverage data...\n");
    if (system(binary_file) != 0) {
        fprintf(stderr, "Execution failed\n");
        ret = 1;
        goto cleanup;
    }
    
    /* Step 4: Locate generated coverage files */
    gcda_file = malloc(MAX_PATH);
    gcno_file = malloc(MAX_PATH);
    
    /* .gcno is created during compilation */
    snprintf(gcno_file, MAX_PATH, "%s.gcno", source_file);
    
    /* .gcda is created during execution (same basename as source) */
    char *base = strrchr(source_file, '/');
    if (!base) base = source_file;
    else base++;
    
    char base_name[MAX_PATH];
    strncpy(base_name, base, MAX_PATH);
    char *dot = strrchr(base_name, '.');
    if (dot) *dot = '\0';
    
    snprintf(gcda_file, MAX_PATH, "%s.gcda", base_name);
    
    printf("  Expected .gcda: %s\n", gcda_file);
    printf("  Expected .gcno: %s\n", gcno_file);
    
    /* Verify files exist */
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, ".gcda file not found: %s\n", gcda_file);
        /* Try alternative location */
        snprintf(gcda_file, MAX_PATH, "%s.gcda", source_file);
        if (stat(gcda_file, &st) != 0) {
            fprintf(stderr, "Could not find .gcda file anywhere\n");
            ret = 1;
            goto cleanup;
        }
    }
    
    if (stat(gcno_file, &st) != 0) {
        fprintf(stderr, ".gcno file not found: %s\n", gcno_file);
        ret = 1;
        goto cleanup;
    }
    
    printf("  Coverage files verified\n");
    
    /* Step 5: Test gcov-dump with various flags */
    printf("\n4. Testing gcov-dump command-line switches...\n");
    
    /* Test case 1: -h (help) - triggers print_usage() */
    execute_command("Testing -h flag (help):", "gcov-dump -h", 0);
    
    /* Test case 2: -v (version) - triggers print_version() */
    execute_command("Testing -v flag (version):", "gcov-dump -v", 0);
    
    /* Test case 3: -l (dump contents) with .gcda file */
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_file);
    execute_command("Testing -l flag (dump contents):", cmd, 0);
    
    /* Test case 4: -p (dump positions) with .gcda file */
    snprintf(cmd, sizeof(cmd), "gcov-dump -p %s", gcda_file);
    execute_command("Testing -p flag (dump positions):", cmd, 0);
    
    /* Test case 5: -r (dump raw) with .gcda file */
    snprintf(cmd, sizeof(cmd), "gcov-dump -r %s", gcda_file);
    execute_command("Testing -r flag (dump raw):", cmd, 0);
    
    /* Test case 6: -s (dump stable) with .gcda file */
    snprintf(cmd, sizeof(cmd), "gcov-dump -s %s", gcda_file);
    execute_command("Testing -s flag (dump stable):", cmd, 0);
    
    /* Test case 7: Combined flags -l -p */
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -p %s", gcda_file);
    execute_command("Testing combined -l -p flags:", cmd, 0);
    
    /* Test case 8: Invalid flag - triggers default case and fprintf */
    snprintf(cmd, sizeof(cmd), "gcov-dump -X %s", gcda_file);
    execute_command("Testing invalid -X flag (should trigger 'unknown flag'):", 
                   cmd, 1);
    
    /* Additional test: Try with .gcno file as well */
    printf("\n5. Additional tests with .gcno file...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcno_file);
    execute_command("Testing -l flag with .gcno file:", cmd, 0);
    
    /* Test another invalid flag */
    snprintf(cmd, sizeof(cmd), "gcov-dump -Z %s", gcda_file);
    execute_command("Testing another invalid flag -Z:", cmd, 1);
    
    printf("\n=== All tests completed ===\n");

cleanup:
    /* Cleanup temporary files */
    printf("\n6. Cleaning up temporary files...\n");
    if (source_file) {
        unlink(source_file);
        printf("  Removed: %s\n", source_file);
        free(source_file);
    }
    if (binary_file) {
        unlink(binary_file);
        printf("  Removed: %s\n", binary_file);
        free(binary_file);
    }
    if (gcda_file) {
        unlink(gcda_file);
        printf("  Removed: %s\n", gcda_file);
        free(gcda_file);
    }
    if (gcno_file) {
        unlink(gcno_file);
        printf("  Removed: %s\n", gcno_file);
        free(gcno_file);
    }
    
    /* Also clean up any default .gcda file */
    unlink("a.out.gcda");
    
    return ret;
}
