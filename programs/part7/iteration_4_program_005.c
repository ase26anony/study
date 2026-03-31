/**
 * test_gcov_dump_switch.c
 * 
 * Test driver to exercise the command-line switch cases in gcov-dump.cc
 * Specifically targets lines 111-130 covering flags: -h, -v, -l, -p, -r, -s, and invalid flags
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define TEMP_FILE_TEMPLATE "/tmp/test_gcov_XXXXXX"

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
 * Create a temporary file with the given content
 * Returns dynamically allocated filename or NULL on error
 */
char *create_temp_file(const char *content, const char *suffix) {
    char *template = strdup(TEMP_FILE_TEMPLATE);
    if (!template) return NULL;
    
    int fd = mkstemp(template);
    if (fd < 0) {
        free(template);
        return NULL;
    }
    
    if (content) {
        write(fd, content, strlen(content));
    }
    
    close(fd);
    
    if (suffix) {
        char *new_name = malloc(strlen(template) + strlen(suffix) + 1);
        if (!new_name) {
            free(template);
            return NULL;
        }
        sprintf(new_name, "%s%s", template, suffix);
        remove(template);  /* Remove the original */
        free(template);
        return new_name;
    }
    
    return template;
}

/**
 * Execute a command and return its exit status
 */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        printf("Command returned status: %d\n", status);
    }
    return status;
}

/**
 * Execute a command and capture its stderr output
 * Returns 1 if "unknown flag" message is found, 0 otherwise
 */
int check_unknown_flag(const char *cmd) {
    char full_cmd[MAX_CMD_LEN];
    /* Redirect stderr to stdout for capture */
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    printf("Checking for unknown flag: %s\n", cmd);
    
    FILE *fp = popen(full_cmd, "r");
    if (!fp) {
        perror("popen failed");
        return 0;
    }
    
    char buffer[256];
    int found = 0;
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (strstr(buffer, "unknown flag")) {
            printf("Found expected error: %s", buffer);
            found = 1;
        }
    }
    
    pclose(fp);
    return found;
}

/**
 * Clean up temporary files
 */
void cleanup_files(const char **files, int count) {
    for (int i = 0; i < count; i++) {
        if (files[i]) {
            remove(files[i]);
        }
    }
}

int main(int argc, char *argv[]) {
    char *source_file = NULL;
    char *binary_file = NULL;
    char *gcda_file = NULL;
    char *gcno_file = NULL;
    char cmd[MAX_CMD_LEN];
    int ret = 0;
    
    printf("=== Starting gcov-dump switch case test ===\n\n");
    
    /* Step 1: Create test source file */
    printf("1. Creating test source file...\n");
    source_file = create_temp_file(test_source, ".c");
    if (!source_file) {
        fprintf(stderr, "Failed to create source file\n");
        return 1;
    }
    printf("   Source file: %s\n", source_file);
    
    /* Step 2: Compile with coverage instrumentation */
    printf("\n2. Compiling with coverage flags...\n");
    binary_file = create_temp_file(NULL, "");
    if (!binary_file) {
        fprintf(stderr, "Failed to create binary filename\n");
        ret = 1;
        goto cleanup;
    }
    
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s",
             source_file, binary_file);
    
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Compilation failed\n");
        ret = 1;
        goto cleanup;
    }
    
    /* Step 3: Execute to generate .gcda file */
    printf("\n3. Executing test program to generate coverage data...\n");
    if (execute_command(binary_file) != 0) {
        fprintf(stderr, "Execution failed\n");
        ret = 1;
        goto cleanup;
    }
    
    /* Construct .gcda and .gcno filenames */
    gcda_file = malloc(strlen(source_file) + 5);
    gcno_file = malloc(strlen(source_file) + 5);
    if (!gcda_file || !gcno_file) {
        fprintf(stderr, "Memory allocation failed\n");
        ret = 1;
        goto cleanup;
    }
    
    sprintf(gcda_file, "%s.gcda", source_file);
    sprintf(gcno_file, "%s.gcno", source_file);
    
    /* Verify files exist */
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, ".gcda file not found: %s\n", gcda_file);
        ret = 1;
        goto cleanup;
    }
    printf("   Generated: %s\n", gcda_file);
    
    if (stat(gcno_file, &st) != 0) {
        fprintf(stderr, ".gcno file not found: %s\n", gcno_file);
        ret = 1;
        goto cleanup;
    }
    printf("   Generated: %s\n", gcno_file);
    
    /* Step 4: Test gcov-dump with various flags */
    printf("\n4. Testing gcov-dump command-line switches...\n");
    
    /* Test -h flag (help) - triggers print_usage() */
    printf("\n   Testing -h flag (help)...\n");
    execute_command("gcov-dump -h");
    
    /* Test -v flag (version) - triggers print_version() */
    printf("\n   Testing -v flag (version)...\n");
    execute_command("gcov-dump -v");
    
    /* Test -l flag (dump contents) */
    printf("\n   Testing -l flag (dump contents)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_file);
    execute_command(cmd);
    
    /* Test -p flag (dump positions) */
    printf("\n   Testing -p flag (dump positions)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -p %s", gcda_file);
    execute_command(cmd);
    
    /* Test -r flag (dump raw) */
    printf("\n   Testing -r flag (dump raw)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -r %s", gcda_file);
    execute_command(cmd);
    
    /* Test -s flag (dump stable) */
    printf("\n   Testing -s flag (dump stable)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -s %s", gcda_file);
    execute_command(cmd);
    
    /* Test combination of flags */
    printf("\n   Testing combination -l -p flags...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -p %s", gcda_file);
    execute_command(cmd);
    
    /* Test with .gcno file instead of .gcda */
    printf("\n   Testing with .gcno file...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcno_file);
    execute_command(cmd);
    
    /* Test invalid flag - triggers default case with fprintf */
    printf("\n   Testing invalid flag (should trigger 'unknown flag' error)...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X %s", gcda_file);
    if (!check_unknown_flag(cmd)) {
        printf("WARNING: 'unknown flag' message not found. "
               "This might indicate the default case wasn't triggered.\n");
    }
    
    /* Test another invalid flag */
    printf("\n   Testing another invalid flag...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -z %s", gcda_file);
    if (!check_unknown_flag(cmd)) {
        printf("WARNING: 'unknown flag' message not found.\n");
    }
    
    /* Test flag without required file argument */
    printf("\n   Testing flag without file argument...\n");
    execute_command("gcov-dump -l");
    
    printf("\n=== Test completed ===\n");

cleanup:
    /* Clean up temporary files */
    printf("\nCleaning up temporary files...\n");
    if (source_file) {
        remove(source_file);
        free(source_file);
    }
    if (binary_file) {
        remove(binary_file);
        free(binary_file);
    }
    if (gcda_file) {
        remove(gcda_file);
        free(gcda_file);
    }
    if (gcno_file) {
        remove(gcno_file);
        free(gcno_file);
    }
    
    return ret;
}
