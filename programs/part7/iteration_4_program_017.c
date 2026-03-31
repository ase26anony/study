/**
 * test_gcov_dump_switches.c
 * 
 * Test driver to exercise the command-line switch cases in gcov-dump.cc
 * Specifically targets lines 111-130 for coverage.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define TEMP_SOURCE "test_coverage_XXXXXX.c"
#define TEMP_BINARY "test_coverage_XXXXXX"
#define TEMP_GCDA   "test_coverage_XXXXXX.gcda"
#define TEMP_GCNO   "test_coverage_XXXXXX.gcno"

/* Simple test program source code that will generate coverage data */
const char *test_program_source = 
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
 * Create a temporary file with a given template and write content to it.
 * Returns the actual filename in dynamically allocated memory.
 */
char *create_temp_file(const char *template, const char *content) {
    char *filename = strdup(template);
    int fd = mkstemp(filename);
    if (fd < 0) {
        perror("mkstemp failed");
        free(filename);
        return NULL;
    }
    
    if (content) {
        write(fd, content, strlen(content));
    }
    
    close(fd);
    return filename;
}

/**
 * Check if a file exists
 */
int file_exists(const char *filename) {
    struct stat st;
    return stat(filename, &st) == 0;
}

/**
 * Execute a system command and return its exit status
 */
int execute_command(const char *command) {
    printf("Executing: %s\n", command);
    int status = system(command);
    if (status != 0) {
        printf("Command returned non-zero: %d\n", WEXITSTATUS(status));
    }
    return status;
}

/**
 * Execute a command and capture its stderr output
 * Returns 1 if the expected string is found in stderr, 0 otherwise
 */
int execute_and_check_stderr(const char *command, const char *expected_error) {
    printf("Executing (checking stderr): %s\n", command);
    
    /* Use popen to capture output (both stdout and stderr redirected) */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s 2>&1", command);
    
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        return 0;
    }
    
    char buffer[1024];
    int found = 0;
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, expected_error) != NULL) {
            found = 1;
            printf("Found expected error message: %s", buffer);
        }
    }
    
    pclose(fp);
    return found;
}

/**
 * Clean up temporary files
 */
void cleanup_files(char **filenames, int count) {
    for (int i = 0; i < count; i++) {
        if (filenames[i]) {
            unlink(filenames[i]);
            free(filenames[i]);
        }
    }
}

int main(int argc, char *argv[]) {
    char *source_file = NULL;
    char *binary_file = NULL;
    char *gcda_file = NULL;
    char *gcno_file = NULL;
    char *files_to_clean[4] = {NULL, NULL, NULL, NULL};
    int file_count = 0;
    
    printf("=== Starting gcov-dump switch coverage test ===\n\n");
    
    /* Step 1: Create a simple C source file */
    printf("1. Creating test source file...\n");
    source_file = create_temp_file(TEMP_SOURCE, test_program_source);
    if (!source_file) {
        fprintf(stderr, "Failed to create source file\n");
        return 1;
    }
    files_to_clean[file_count++] = source_file;
    printf("   Created: %s\n", source_file);
    
    /* Step 2: Compile with coverage instrumentation */
    printf("\n2. Compiling with coverage flags...\n");
    char compile_cmd[1024];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage %s -o %s",
             source_file, source_file);
    
    /* Replace .c extension with binary name */
    binary_file = strdup(source_file);
    char *dot = strrchr(binary_file, '.');
    if (dot) *dot = '\0';
    
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage %s -o %s",
             source_file, binary_file);
    
    if (execute_command(compile_cmd) != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup_files(files_to_clean, file_count);
        return 1;
    }
    files_to_clean[file_count++] = binary_file;
    printf("   Compiled: %s\n", binary_file);
    
    /* Step 3: Execute the program to generate .gcda file */
    printf("\n3. Executing test program to generate coverage data...\n");
    if (execute_command(binary_file) != 0) {
        fprintf(stderr, "Execution failed\n");
        cleanup_files(files_to_clean, file_count);
        return 1;
    }
    
    /* Construct .gcda and .gcno filenames */
    gcda_file = malloc(strlen(binary_file) + 6);
    gcno_file = malloc(strlen(binary_file) + 6);
    sprintf(gcda_file, "%s.gcda", binary_file);
    sprintf(gcno_file, "%s.gcno", binary_file);
    
    if (!file_exists(gcda_file)) {
        fprintf(stderr, "No .gcda file generated: %s\n", gcda_file);
        cleanup_files(files_to_clean, file_count);
        free(gcda_file);
        free(gcno_file);
        return 1;
    }
    
    files_to_clean[file_count++] = gcda_file;
    files_to_clean[file_count++] = gcno_file;
    printf("   Generated: %s\n", gcda_file);
    printf("   Generated: %s\n", gcno_file);
    
    /* Step 4: Test gcov-dump with various switches */
    printf("\n4. Testing gcov-dump command-line switches...\n");
    
    /* 4.1 Test -h (help) - triggers print_usage() */
    printf("\n  4.1 Testing -h flag (help)...\n");
    execute_command("gcov-dump -h");
    
    /* 4.2 Test -v (version) - triggers print_version() */
    printf("\n  4.2 Testing -v flag (version)...\n");
    execute_command("gcov-dump -v");
    
    /* 4.3 Test -l flag (dump contents) */
    printf("\n  4.3 Testing -l flag (dump contents)...\n");
    char cmd_l[1024];
    snprintf(cmd_l, sizeof(cmd_l), "gcov-dump -l %s", gcda_file);
    execute_command(cmd_l);
    
    /* 4.4 Test -p flag (dump positions) */
    printf("\n  4.4 Testing -p flag (dump positions)...\n");
    char cmd_p[1024];
    snprintf(cmd_p, sizeof(cmd_p), "gcov-dump -p %s", gcda_file);
    execute_command(cmd_p);
    
    /* 4.5 Test -r flag (dump raw) */
    printf("\n  4.5 Testing -r flag (dump raw)...\n");
    char cmd_r[1024];
    snprintf(cmd_r, sizeof(cmd_r), "gcov-dump -r %s", gcda_file);
    execute_command(cmd_r);
    
    /* 4.6 Test -s flag (dump stable) */
    printf("\n  4.6 Testing -s flag (dump stable)...\n");
    char cmd_s[1024];
    snprintf(cmd_s, sizeof(cmd_s), "gcov-dump -s %s", gcda_file);
    execute_command(cmd_s);
    
    /* 4.7 Test combined flags */
    printf("\n  4.7 Testing combined flags (-l -p)...\n");
    char cmd_lp[1024];
    snprintf(cmd_lp, sizeof(cmd_lp), "gcov-dump -l -p %s", gcda_file);
    execute_command(cmd_lp);
    
    /* 4.8 Test invalid flag - triggers default case with fprintf */
    printf("\n  4.8 Testing invalid flag (should trigger 'unknown flag' error)...\n");
    char cmd_invalid[1024];
    snprintf(cmd_invalid, sizeof(cmd_invalid), "gcov-dump -X %s", gcda_file);
    
    int found_error = execute_and_check_stderr(cmd_invalid, "unknown flag");
    if (found_error) {
        printf("   SUCCESS: Invalid flag triggered the expected error message\n");
    } else {
        printf("   WARNING: Did not find expected error message for invalid flag\n");
    }
    
    /* 4.9 Test with .gcno file as well */
    printf("\n  4.9 Testing with .gcno file...\n");
    char cmd_gcno[1024];
    snprintf(cmd_gcno, sizeof(cmd_gcno), "gcov-dump -l %s", gcno_file);
    execute_command(cmd_gcno);
    
    /* 4.10 Test multiple invalid flags */
    printf("\n  4.10 Testing multiple invalid flags...\n");
    execute_and_check_stderr("gcov-dump -X -Y -Z", "unknown flag");
    
    /* Step 5: Cleanup */
    printf("\n5. Cleaning up temporary files...\n");
    cleanup_files(files_to_clean, file_count);
    
    printf("\n=== Test completed ===\n");
    printf("All gcov-dump switch cases should have been exercised.\n");
    printf("Check coverage report for gcov-dump.cc lines 111-130.\n");
    
    return 0;
}
