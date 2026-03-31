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

/* Function to create a temporary filename */
char *create_temp_filename(const char *prefix, const char *suffix) {
    static char filename[MAX_PATH];
    char template[MAX_PATH];
    
    snprintf(template, sizeof(template), "/tmp/%s_XXXXXX%s", prefix, suffix);
    int fd = mkstemps(template, strlen(suffix));
    if (fd < 0) {
        perror("mkstemps failed");
        return NULL;
    }
    close(fd);
    
    strncpy(filename, template, MAX_PATH);
    return filename;
}

/* Execute a command and return its exit status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        printf("Command failed with status %d\n", WEXITSTATUS(status));
    }
    return status;
}

/* Execute command and capture stderr to check for specific message */
int execute_and_check_stderr(const char *cmd, const char *expected_error) {
    char full_cmd[MAX_CMD];
    char buffer[256];
    int found = 0;
    
    /* Redirect stderr to stdout and pipe it */
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    printf("Executing (checking stderr): %s\n", cmd);
    
    FILE *pipe = popen(full_cmd, "r");
    if (!pipe) {
        perror("popen failed");
        return -1;
    }
    
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        if (strstr(buffer, expected_error) != NULL) {
            printf("Found expected error message: %s", buffer);
            found = 1;
        }
    }
    
    int status = pclose(pipe);
    if (found) {
        printf("Successfully triggered default case with error message\n");
    } else {
        printf("Warning: Expected error message not found\n");
    }
    
    return status;
}

/* Clean up temporary files */
void cleanup_files(const char *base_name) {
    char cmd[MAX_CMD];
    
    /* Remove all generated files */
    const char *extensions[] = {".c", "", ".gcda", ".gcno", ".gcov"};
    for (size_t i = 0; i < sizeof(extensions)/sizeof(extensions[0]); i++) {
        snprintf(cmd, sizeof(cmd), "rm -f %s%s", base_name, extensions[i]);
        system(cmd);
    }
}

int main(int argc, char *argv[]) {
    char source_file[MAX_PATH];
    char binary_file[MAX_PATH];
    char gcda_file[MAX_PATH];
    char cmd[MAX_CMD];
    int ret = 0;
    
    printf("=== Starting gcov-dump command-line parsing test ===\n");
    
    /* Create temporary filenames */
    char *temp_base = create_temp_filename("gcov_test", "");
    if (!temp_base) {
        fprintf(stderr, "Failed to create temporary filename\n");
        return 1;
    }
    
    snprintf(source_file, sizeof(source_file), "%s.c", temp_base);
    snprintf(binary_file, sizeof(binary_file), "%s", temp_base);
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", temp_base);
    
    /* Step 1: Create test source file */
    printf("\n1. Creating test source file: %s\n", source_file);
    FILE *fp = fopen(source_file, "w");
    if (!fp) {
        perror("Failed to create source file");
        free(temp_base);
        return 1;
    }
    fputs(test_source, fp);
    fclose(fp);
    
    /* Step 2: Compile with coverage instrumentation */
    printf("\n2. Compiling with coverage flags\n");
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage %s -o %s",
             source_file, binary_file);
    
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup_files(temp_base);
        free(temp_base);
        return 1;
    }
    
    /* Step 3: Execute to generate coverage data */
    printf("\n3. Executing program to generate .gcda file\n");
    snprintf(cmd, sizeof(cmd), "./%s", binary_file);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Execution failed\n");
        cleanup_files(temp_base);
        free(temp_base);
        return 1;
    }
    
    /* Verify .gcda file was created */
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, ".gcda file not created: %s\n", gcda_file);
        cleanup_files(temp_base);
        free(temp_base);
        return 1;
    }
    printf("Generated coverage file: %s (size: %ld bytes)\n", 
           gcda_file, (long)st.st_size);
    
    /* Step 4: Test gcov-dump with various flags */
    printf("\n4. Testing gcov-dump command-line parsing\n");
    
    /* 4a: Test -h flag (help) - triggers print_usage() */
    printf("\n4a. Testing -h flag (help)\n");
    execute_command("gcov-dump -h");
    
    /* 4b: Test -v flag (version) - triggers print_version() */
    printf("\n4b. Testing -v flag (version)\n");
    execute_command("gcov-dump -v");
    
    /* 4c: Test -l flag (dump contents) with .gcda file */
    printf("\n4c. Testing -l flag (dump contents)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_file);
    execute_command(cmd);
    
    /* 4d: Test -p flag (dump positions) with .gcda file */
    printf("\n4d. Testing -p flag (dump positions)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -p %s", gcda_file);
    execute_command(cmd);
    
    /* 4e: Test -r flag (dump raw) with .gcda file */
    printf("\n4e. Testing -r flag (dump raw)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -r %s", gcda_file);
    execute_command(cmd);
    
    /* 4f: Test -s flag (dump stable) with .gcda file */
    printf("\n4f. Testing -s flag (dump stable)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -s %s", gcda_file);
    execute_command(cmd);
    
    /* 4g: Test combined flags */
    printf("\n4g. Testing combined flags (-l -p)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -p %s", gcda_file);
    execute_command(cmd);
    
    /* 4h: Test invalid flag - triggers default case and fprintf */
    printf("\n4h. Testing invalid flag (should trigger default case)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -X %s", gcda_file);
    execute_and_check_stderr(cmd, "unknown flag");
    
    /* Additional invalid flag tests for robustness */
    printf("\n4i. Testing another invalid flag\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -z %s", gcda_file);
    execute_and_check_stderr(cmd, "unknown flag");
    
    /* Test with no file argument (should still parse flags) */
    printf("\n4j. Testing -h without file argument\n");
    execute_command("gcov-dump -h");
    
    /* Test flag ordering variations */
    printf("\n4k. Testing flag ordering (file first)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump %s -l", gcda_file);
    execute_command(cmd);
    
    /* Step 5: Cleanup */
    printf("\n5. Cleaning up temporary files\n");
    cleanup_files(temp_base);
    
    printf("\n=== Test completed ===\n");
    printf("All gcov-dump command-line parsing paths should have been exercised.\n");
    printf("Check coverage report to verify lines 111-130 in gcov-dump.cc are covered.\n");
    
    free(temp_base);
    return ret;
}
