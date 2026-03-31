/**
 * test_gcov_dump_switches.c
 * 
 * Test driver to exercise specific command-line switch cases in gcov-dump.
 * Targets lines 111-130 of gcov-dump.cc:
 * - Help (-h) -> print_usage()
 * - Version (-v) -> print_version()
 * - Dump flags (-l, -p, -r, -s) -> set internal flags
 * - Invalid flag -> default case with error message
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

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

/* Function to create a temporary filename */
char* create_temp_filename(const char *prefix, const char *suffix) {
    char *template = malloc(strlen(prefix) + 10 + strlen(suffix) + 1);
    if (!template) return NULL;
    
    sprintf(template, "%sXXXXXX%s", prefix, suffix);
    int fd = mkstemps(template, strlen(suffix));
    if (fd < 0) {
        free(template);
        return NULL;
    }
    close(fd);
    return template;
}

/* Check if a file exists */
int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

/* Execute a command and capture output */
int execute_and_capture(const char *cmd, char *output, size_t output_size) {
    FILE *fp = popen(cmd, "r");
    if (!fp) return -1;
    
    size_t total = 0;
    while (fgets(output + total, output_size - total, fp) != NULL) {
        total = strlen(output);
        if (total >= output_size - 1) break;
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
}

/* Execute a command and check stderr for specific message */
int check_stderr_for_message(const char *cmd, const char *expected_msg) {
    char full_cmd[512];
    /* Redirect stderr to stdout for capture */
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    char output[1024] = {0};
    execute_and_capture(full_cmd, output, sizeof(output));
    
    return strstr(output, expected_msg) != NULL;
}

int main(int argc, char *argv[]) {
    printf("=== Testing gcov-dump command-line switches ===\n\n");
    
    /* Create temporary filenames */
    char *source_file = create_temp_filename("test_gcov_", ".c");
    char *binary_file = create_temp_filename("test_gcov_", "");
    char *gcda_file = malloc(strlen(binary_file) + 6);
    char *gcno_file = malloc(strlen(binary_file) + 6);
    
    if (!source_file || !binary_file || !gcda_file || !gcno_file) {
        fprintf(stderr, "Failed to create temp filenames\n");
        return 1;
    }
    
    sprintf(gcda_file, "%s.gcda", binary_file);
    sprintf(gcno_file, "%s.gcno", binary_file);
    
    printf("Temporary files:\n");
    printf("  Source: %s\n", source_file);
    printf("  Binary: %s\n", binary_file);
    printf("  GCDA:   %s\n", gcda_file);
    printf("  GCNO:   %s\n", gcno_file);
    printf("\n");
    
    /* Step 1: Create test source file */
    printf("1. Creating test source file...\n");
    FILE *fp = fopen(source_file, "w");
    if (!fp) {
        perror("Failed to create source file");
        return 1;
    }
    fputs(test_source, fp);
    fclose(fp);
    
    /* Step 2: Compile with coverage instrumentation */
    printf("2. Compiling with coverage flags...\n");
    char compile_cmd[512];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage %s -o %s",
             source_file, binary_file);
    
    int ret = system(compile_cmd);
    if (ret != 0) {
        fprintf(stderr, "Compilation failed: %s\n", compile_cmd);
        return 1;
    }
    
    /* Step 3: Execute to generate coverage data */
    printf("3. Executing to generate .gcda file...\n");
    ret = system(binary_file);
    if (ret != 0) {
        fprintf(stderr, "Execution failed\n");
        return 1;
    }
    
    /* Verify .gcda file was created */
    if (!file_exists(gcda_file)) {
        fprintf(stderr, "Failed to generate .gcda file\n");
        return 1;
    }
    
    printf("4. Testing gcov-dump switches...\n\n");
    
    /* Test 1: Help flag (-h) - triggers print_usage() */
    printf("Test 1: -h (help flag)\n");
    system("gcov-dump -h 2>&1 | head -5");
    printf("... (help output truncated)\n\n");
    
    /* Test 2: Version flag (-v) - triggers print_version() */
    printf("Test 2: -v (version flag)\n");
    system("gcov-dump -v");
    printf("\n");
    
    /* Test 3-6: Individual dump flags with .gcda file */
    const char *flags[] = {"-l", "-p", "-r", "-s"};
    const char *descriptions[] = {
        "dump contents",
        "dump positions", 
        "dump raw",
        "dump stable"
    };
    
    for (int i = 0; i < 4; i++) {
        printf("Test %d: %s (%s)\n", i + 3, flags[i], descriptions[i]);
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s 2>&1 | head -3", 
                 flags[i], gcda_file);
        system(cmd);
        printf("... (output truncated)\n\n");
    }
    
    /* Test 7: Combined flags */
    printf("Test 7: Combined flags (-l -p)\n");
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -p %s 2>&1 | head -3", gcda_file);
    system(cmd);
    printf("... (output truncated)\n\n");
    
    /* Test 8: Invalid flag - triggers default case with error message */
    printf("Test 8: Invalid flag (-X) - should trigger 'unknown flag' error\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X %s", gcda_file);
    
    if (check_stderr_for_message(cmd, "unknown flag")) {
        printf("✓ Successfully triggered 'unknown flag' error message\n");
    } else {
        printf("✗ Did not see expected error message\n");
        /* Try again with direct stderr capture */
        printf("Direct output:\n");
        char full_cmd[512];
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
        system(full_cmd);
    }
    printf("\n");
    
    /* Additional test: Multiple invalid flags */
    printf("Test 9: Multiple invalid flags (-X -Y)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X -Y %s", gcda_file);
    if (check_stderr_for_message(cmd, "unknown flag")) {
        printf("✓ Triggered error for invalid flags\n");
    }
    printf("\n");
    
    /* Test with .gcno file as well */
    printf("Test 10: Testing with .gcno file\n");
    if (file_exists(gcno_file)) {
        for (int i = 0; i < 4; i++) {
            snprintf(cmd, sizeof(cmd), "gcov-dump %s %s >/dev/null 2>&1", 
                     flags[i], gcno_file);
            ret = system(cmd);
            if (ret == 0) {
                printf("  %s with .gcno: OK\n", flags[i]);
            }
        }
    }
    printf("\n");
    
    /* Cleanup */
    printf("Cleaning up temporary files...\n");
    unlink(source_file);
    unlink(binary_file);
    unlink(gcda_file);
    unlink(gcno_file);
    
    free(source_file);
    free(binary_file);
    free(gcda_file);
    free(gcno_file);
    
    printf("\n=== All tests completed ===\n");
    printf("Note: To verify coverage of gcov-dump.cc lines 111-130:\n");
    printf("1. Rebuild gcov-dump with coverage: g++ -fprofile-arcs -ftest-coverage gcov-dump.cc -o gcov-dump\n");
    printf("2. Run this test program\n");
    printf("3. Run: gcov gcov-dump.cc\n");
    printf("4. Check that lines 111-130 are marked as executed\n");
    
    return 0;
}
