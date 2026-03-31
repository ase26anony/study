/**
 * test_gcov_dump_switches.c
 * 
 * Test driver to exercise specific command-line switch cases in gcov-dump
 * Targets lines 111-130 of gcov-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define TEMP_FILENAME "test_gcov_dump_coverage"

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
 * Execute a command and capture its stderr output
 * Returns 1 if "unknown flag" message is found, 0 otherwise
 */
int check_for_unknown_flag(const char *cmd) {
    char buffer[256];
    FILE *fp;
    int found = 0;
    
    /* Redirect stderr to stdout and capture */
    char full_cmd[MAX_CMD_LEN];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    fp = popen(full_cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return 0;
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, "unknown flag") != NULL) {
            printf("  -> Found expected 'unknown flag' message: %s", buffer);
            found = 1;
        }
    }
    
    pclose(fp);
    return found;
}

/**
 * Execute a command and print its output
 */
void execute_and_print(const char *cmd, const char *description) {
    char buffer[256];
    FILE *fp;
    
    printf("\n=== Testing: %s ===\n", description);
    printf("Command: %s\n", cmd);
    
    fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return;
    }
    
    /* Read and print first few lines of output */
    int lines = 0;
    while (fgets(buffer, sizeof(buffer), fp) != NULL && lines < 5) {
        printf("  Output: %s", buffer);
        lines++;
    }
    
    if (lines == 5) {
        printf("  [Output truncated...]\n");
    }
    
    pclose(fp);
}

/**
 * Clean up temporary files
 */
void cleanup_files(void) {
    char cmd[MAX_CMD_LEN];
    
    /* Remove generated files */
    const char *files[] = {
        TEMP_FILENAME ".c",
        TEMP_FILENAME,
        TEMP_FILENAME ".gcno",
        TEMP_FILENAME ".gcda",
        NULL
    };
    
    for (int i = 0; files[i] != NULL; i++) {
        if (unlink(files[i]) == 0) {
            printf("Removed: %s\n", files[i]);
        } else if (errno != ENOENT) {
            perror(files[i]);
        }
    }
}

int main(void) {
    char cmd[MAX_CMD_LEN];
    FILE *fp;
    int ret;
    
    printf("=== Generating coverage data for gcov-dump testing ===\n");
    
    /* Step 1: Create test source file */
    fp = fopen(TEMP_FILENAME ".c", "w");
    if (fp == NULL) {
        perror("Failed to create source file");
        return 1;
    }
    fputs(test_source, fp);
    fclose(fp);
    printf("Created source file: %s.c\n", TEMP_FILENAME);
    
    /* Step 2: Compile with coverage instrumentation */
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s.c -o %s",
             TEMP_FILENAME, TEMP_FILENAME);
    printf("Compiling: %s\n", cmd);
    ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup_files();
        return 1;
    }
    
    /* Step 3: Execute to generate .gcda file */
    printf("Executing program to generate coverage data...\n");
    snprintf(cmd, sizeof(cmd), "./%s", TEMP_FILENAME);
    ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Execution failed\n");
        cleanup_files();
        return 1;
    }
    
    /* Verify .gcda file exists */
    struct stat st;
    if (stat(TEMP_FILENAME ".gcda", &st) != 0) {
        fprintf(stderr, "Coverage data file not created\n");
        cleanup_files();
        return 1;
    }
    printf("Coverage data file created: %s.gcda\n", TEMP_FILENAME);
    
    printf("\n=== Testing gcov-dump command-line switches ===\n");
    
    /* Test 1: -h flag (help) - triggers print_usage() */
    execute_and_print("gcov-dump -h", "Help flag (-h)");
    
    /* Test 2: -v flag (version) - triggers print_version() */
    execute_and_print("gcov-dump -v", "Version flag (-v)");
    
    /* Test 3: -l flag (dump contents) - sets flag_dump_contents = 1 */
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s.gcda", TEMP_FILENAME);
    execute_and_print(cmd, "Dump contents flag (-l)");
    
    /* Test 4: -p flag (dump positions) - sets flag_dump_positions = 1 */
    snprintf(cmd, sizeof(cmd), "gcov-dump -p %s.gcda", TEMP_FILENAME);
    execute_and_print(cmd, "Dump positions flag (-p)");
    
    /* Test 5: -r flag (dump raw) - sets flag_dump_raw = 1 */
    snprintf(cmd, sizeof(cmd), "gcov-dump -r %s.gcda", TEMP_FILENAME);
    execute_and_print(cmd, "Dump raw flag (-r)");
    
    /* Test 6: -s flag (dump stable) - sets flag_dump_stable = 1 */
    snprintf(cmd, sizeof(cmd), "gcov-dump -s %s.gcda", TEMP_FILENAME);
    execute_and_print(cmd, "Dump stable flag (-s)");
    
    /* Test 7: Combined flags - both -l and -p */
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -p %s.gcda", TEMP_FILENAME);
    execute_and_print(cmd, "Combined flags (-l -p)");
    
    /* Test 8: Invalid flag - triggers default case with fprintf */
    printf("\n=== Testing invalid flag (should trigger 'unknown flag' error) ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X %s.gcda", TEMP_FILENAME);
    printf("Command: %s\n", cmd);
    
    if (check_for_unknown_flag(cmd)) {
        printf("SUCCESS: Invalid flag triggered expected error message\n");
    } else {
        printf("WARNING: 'unknown flag' message not found (may be handled differently)\n");
    }
    
    /* Test 9: Another invalid flag for good measure */
    printf("\n=== Testing another invalid flag ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -z %s.gcda", TEMP_FILENAME);
    printf("Command: %s\n", cmd);
    
    if (check_for_unknown_flag(cmd)) {
        printf("SUCCESS: Invalid flag 'z' triggered expected error message\n");
    }
    
    /* Test 10: No flags but with file (tests default behavior) */
    printf("\n=== Testing with no flags (default behavior) ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump %s.gcda", TEMP_FILENAME);
    execute_and_print(cmd, "No flags, just file");
    
    printf("\n=== All tests completed ===\n");
    
    /* Cleanup */
    cleanup_files();
    
    return 0;
}
