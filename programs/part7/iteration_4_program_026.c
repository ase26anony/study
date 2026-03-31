/**
 * Test driver for gcov-dump command-line parsing coverage
 * Targets specific uncovered lines in gcov-dump.cc (lines 111-130)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_XXXXXX.c"
#define TEMP_BINARY_FILE "test_coverage_XXXXXX"
#define TEMP_GCDA_FILE "test_coverage_XXXXXX.gcda"
#define TEMP_GCNO_FILE "test_coverage_XXXXXX.gcno"

/* Simple C program source code to generate coverage data */
static const char *test_program_source = 
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
void create_temp_filename(char *template, const char *pattern) {
    strcpy(template, pattern);
    int fd = mkstemp(template);
    if (fd != -1) {
        close(fd);
    }
}

/* Execute a system command and return success/failure */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Command failed with return code %d: %s\n", ret, cmd);
    }
    return ret == 0;
}

/* Execute command and capture stderr to check for specific output */
int execute_and_check_stderr(const char *cmd, const char *expected_error) {
    printf("Executing (checking stderr): %s\n", cmd);
    
    /* Redirect stderr to a pipe and capture it */
    char full_cmd[1024];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    FILE *fp = popen(full_cmd, "r");
    if (!fp) {
        perror("popen failed");
        return 0;
    }
    
    char buffer[256];
    int found = 0;
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, expected_error) != NULL) {
            printf("Found expected error message: %s", buffer);
            found = 1;
        }
    }
    
    int status = pclose(fp);
    return found && (WEXITSTATUS(status) != 0);
}

/* Clean up temporary files */
void cleanup_files(const char *source_file, const char *binary_file, 
                   const char *gcda_file, const char *gcno_file) {
    if (source_file) unlink(source_file);
    if (binary_file) unlink(binary_file);
    if (gcda_file) unlink(gcda_file);
    if (gcno_file) unlink(gcno_file);
}

int main(int argc, char *argv[]) {
    char source_file[256];
    char binary_file[256];
    char gcda_file[256];
    char gcno_file[256];
    
    /* Create unique temporary filenames */
    create_temp_filename(source_file, "test_cov_XXXXXX.c");
    create_temp_filename(binary_file, "test_cov_XXXXXX");
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", binary_file);
    snprintf(gcno_file, sizeof(gcno_file), "%s.gcno", binary_file);
    
    printf("=== Generating coverage data file ===\n");
    
    /* Step 1: Write test program source */
    FILE *fp = fopen(source_file, "w");
    if (!fp) {
        perror("Failed to create source file");
        return 1;
    }
    fputs(test_program_source, fp);
    fclose(fp);
    
    /* Step 2: Compile with coverage instrumentation */
    char compile_cmd[512];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage %s -o %s",
             source_file, binary_file);
    
    if (!execute_command(compile_cmd)) {
        fprintf(stderr, "Compilation failed\n");
        cleanup_files(source_file, binary_file, gcda_file, gcno_file);
        return 1;
    }
    
    /* Step 3: Execute to generate .gcda file */
    if (!execute_command(binary_file)) {
        fprintf(stderr, "Execution failed\n");
        cleanup_files(source_file, binary_file, gcda_file, gcno_file);
        return 1;
    }
    
    /* Verify .gcda file was created */
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "No .gcda file generated: %s\n", gcda_file);
        cleanup_files(source_file, binary_file, gcda_file, gcno_file);
        return 1;
    }
    
    printf("\n=== Testing gcov-dump command-line parsing ===\n");
    
    /* Test 1: -h flag (help) - triggers print_usage() */
    printf("\n--- Testing -h flag (help) ---\n");
    execute_command("gcov-dump -h");
    
    /* Test 2: -v flag (version) - triggers print_version() */
    printf("\n--- Testing -v flag (version) ---\n");
    execute_command("gcov-dump -v");
    
    /* Test 3: -l flag (dump contents) - sets flag_dump_contents = 1 */
    printf("\n--- Testing -l flag (dump contents) ---\n");
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_file);
    execute_command(cmd);
    
    /* Test 4: -p flag (dump positions) - sets flag_dump_positions = 1 */
    printf("\n--- Testing -p flag (dump positions) ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -p %s", gcda_file);
    execute_command(cmd);
    
    /* Test 5: -r flag (dump raw) - sets flag_dump_raw = 1 */
    printf("\n--- Testing -r flag (dump raw) ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -r %s", gcda_file);
    execute_command(cmd);
    
    /* Test 6: -s flag (dump stable) - sets flag_dump_stable = 1 */
    printf("\n--- Testing -s flag (dump stable) ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -s %s", gcda_file);
    execute_command(cmd);
    
    /* Test 7: Combined flags - tests multiple flag processing */
    printf("\n--- Testing combined flags (-l -p) ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -p %s", gcda_file);
    execute_command(cmd);
    
    /* Test 8: Invalid flag - triggers default case and fprintf */
    printf("\n--- Testing invalid flag (-X) ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X %s", gcda_file);
    if (execute_and_check_stderr(cmd, "unknown flag")) {
        printf("SUCCESS: Invalid flag triggered expected error message\n");
    } else {
        printf("WARNING: Invalid flag may not have triggered expected error\n");
    }
    
    /* Additional test: Multiple invalid flags */
    printf("\n--- Testing multiple invalid flags (-X -Y) ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X -Y %s", gcda_file);
    execute_and_check_stderr(cmd, "unknown flag");
    
    /* Test with .gcno file as well */
    printf("\n--- Testing with .gcno file ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcno_file);
    execute_command(cmd);
    
    /* Test with no file argument (should show usage or error) */
    printf("\n--- Testing with no file argument ---\n");
    execute_command("gcov-dump -l");
    
    printf("\n=== All tests completed ===\n");
    
    /* Cleanup */
    cleanup_files(source_file, binary_file, gcda_file, gcno_file);
    
    return 0;
}
