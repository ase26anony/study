/**
 * test_gcov_dump_switches.c
 * 
 * Test driver to exercise the command-line parsing switch cases in gcov-dump.cc
 * Specifically targets lines 111-130 for coverage.
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
 * Create a temporary file with the given content
 * Returns dynamically allocated filename (caller must free) or NULL on error
 */
char *create_temp_file(const char *content, const char *suffix) {
    char *filename = malloc(MAX_PATH);
    if (!filename) return NULL;
    
    strcpy(filename, "/tmp/gcov_test_XXXXXX");
    if (suffix) {
        strcat(filename, suffix);
    }
    
    int fd = mkstemps(filename, suffix ? strlen(suffix) : 0);
    if (fd < 0) {
        free(filename);
        return NULL;
    }
    
    FILE *f = fdopen(fd, "w");
    if (!f) {
        close(fd);
        free(filename);
        return NULL;
    }
    
    fputs(content, f);
    fclose(f);
    return filename;
}

/**
 * Execute a command and capture stderr
 * Returns 1 if "unknown flag" message found in stderr, 0 otherwise
 */
int check_for_unknown_flag(const char *cmd) {
    char full_cmd[MAX_CMD];
    char buffer[256];
    int found = 0;
    
    /* Redirect stderr to stdout for capture */
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    FILE *pipe = popen(full_cmd, "r");
    if (!pipe) {
        fprintf(stderr, "Failed to execute: %s\n", cmd);
        return 0;
    }
    
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        if (strstr(buffer, "unknown flag")) {
            found = 1;
            printf("  ✓ Found expected 'unknown flag' message: %s", buffer);
        }
    }
    
    pclose(pipe);
    return found;
}

/**
 * Execute a command and check return status
 */
int execute_command(const char *cmd, const char *description) {
    printf("Executing: %s\n", description);
    printf("  Command: %s\n", cmd);
    
    int status = system(cmd);
    if (status != 0) {
        printf("  ⚠ Command returned non-zero: %d\n", WEXITSTATUS(status));
    } else {
        printf("  ✓ Command succeeded\n");
    }
    return status;
}

int main(int argc, char *argv[]) {
    char *source_file = NULL;
    char *binary_file = NULL;
    char *gcda_file = NULL;
    char cmd[MAX_CMD];
    int ret = 0;
    
    printf("=== Testing gcov-dump command-line switches ===\n\n");
    
    /* Step 1: Create test source file */
    printf("1. Creating test source file...\n");
    source_file = create_temp_file(test_source, ".c");
    if (!source_file) {
        fprintf(stderr, "Failed to create source file\n");
        return 1;
    }
    printf("   Source: %s\n", source_file);
    
    /* Step 2: Create binary filename */
    binary_file = malloc(MAX_PATH);
    snprintf(binary_file, MAX_PATH, "%s.bin", source_file);
    
    /* Step 3: Compile with coverage instrumentation */
    printf("\n2. Compiling with coverage flags...\n");
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s",
             source_file, binary_file);
    
    if (execute_command(cmd, "Compile test program") != 0) {
        fprintf(stderr, "Compilation failed\n");
        ret = 1;
        goto cleanup;
    }
    
    /* Step 4: Execute to generate .gcda file */
    printf("\n3. Running test program to generate coverage data...\n");
    if (execute_command(binary_file, "Run test program") != 0) {
        fprintf(stderr, "Execution failed\n");
        ret = 1;
        goto cleanup;
    }
    
    /* Step 5: Construct .gcda filename */
    gcda_file = malloc(MAX_PATH);
    snprintf(gcda_file, MAX_PATH, "%s.gcda", source_file);
    
    /* Verify .gcda file exists */
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "Coverage data file not found: %s\n", gcda_file);
        ret = 1;
        goto cleanup;
    }
    printf("   Coverage data: %s (size: %ld bytes)\n", gcda_file, st.st_size);
    
    /* Step 6: Test gcov-dump switches */
    printf("\n4. Testing gcov-dump command-line switches...\n");
    
    /* 6a: Test -h (help) - triggers print_usage() */
    printf("\n   a) Testing -h flag (help):\n");
    execute_command("gcov-dump -h", "Print usage information");
    
    /* 6b: Test -v (version) - triggers print_version() */
    printf("\n   b) Testing -v flag (version):\n");
    execute_command("gcov-dump -v", "Print version information");
    
    /* 6c: Test -l flag (dump contents) */
    printf("\n   c) Testing -l flag (dump contents):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_file);
    execute_command(cmd, "Dump contents of coverage data");
    
    /* 6d: Test -p flag (dump positions) */
    printf("\n   d) Testing -p flag (dump positions):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -p %s", gcda_file);
    execute_command(cmd, "Dump positions in coverage data");
    
    /* 6e: Test -r flag (dump raw) */
    printf("\n   e) Testing -r flag (dump raw):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -r %s", gcda_file);
    execute_command(cmd, "Dump raw coverage data");
    
    /* 6f: Test -s flag (dump stable) */
    printf("\n   f) Testing -s flag (dump stable):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -s %s", gcda_file);
    execute_command(cmd, "Dump stable coverage data");
    
    /* 6g: Test combined flags */
    printf("\n   g) Testing combined flags (-l -p):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -p %s", gcda_file);
    execute_command(cmd, "Dump contents and positions");
    
    /* 6h: Test invalid flag - triggers default case and fprintf */
    printf("\n   h) Testing invalid flag (should trigger 'unknown flag' error):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X %s", gcda_file);
    if (!check_for_unknown_flag(cmd)) {
        printf("  ✗ Did not find expected 'unknown flag' message\n");
        printf("  ⚠ Note: Some gcov-dump versions may handle invalid flags differently\n");
    }
    
    /* Test another invalid flag combination */
    printf("\n   i) Testing another invalid flag (-z):\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -z %s", gcda_file);
    check_for_unknown_flag(cmd);
    
    /* Test flag with argument (if supported) */
    printf("\n   j) Testing flag with invalid argument:\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -? %s", gcda_file);
    check_for_unknown_flag(cmd);
    
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
        /* Also remove .gcno file */
        char *gcno_file = malloc(MAX_PATH);
        if (gcno_file) {
            snprintf(gcno_file, MAX_PATH, "%s.gcno", source_file ? source_file : "");
            unlink(gcno_file);
            free(gcno_file);
        }
        free(gcda_file);
    }
    
    return ret;
}
