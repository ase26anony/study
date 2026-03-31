#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define TEMP_DIR "/tmp/gcov_test_XXXXXX"
#define HELPER_SRC "helper.c"
#define HELPER_BIN "helper"
#define MAX_CMD_LEN 1024

/* Simple C program to generate GCOV data */
const char *helper_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Generating GCOV data...\\n\");\n"
"    return 0;\n"
"}\n";

/* Execute a shell command and capture output */
int execute_command(const char *cmd, char *output, size_t output_size) {
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        return -1;
    }
    
    if (output && output_size > 0) {
        output[0] = '\0';
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            strncat(output, buffer, output_size - strlen(output) - 1);
        }
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
}

/* Check if file exists */
int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

int main() {
    char temp_dir[64];
    char cmd[MAX_CMD_LEN];
    char output[4096];
    int status;
    
    /* Create temporary directory */
    strcpy(temp_dir, TEMP_DIR);
    if (mkdtemp(temp_dir) == NULL) {
        perror("Failed to create temp directory");
        return 1;
    }
    
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Change to temp directory */
    if (chdir(temp_dir) != 0) {
        perror("Failed to change directory");
        return 1;
    }
    
    /* Write helper source file */
    FILE *fp = fopen(HELPER_SRC, "w");
    if (!fp) {
        perror("Failed to create helper.c");
        return 1;
    }
    fputs(helper_source, fp);
    fclose(fp);
    
    /* Compile helper with coverage instrumentation */
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -o %s %s 2>&1", 
             HELPER_BIN, HELPER_SRC);
    printf("Compiling helper: %s\n", cmd);
    status = execute_command(cmd, output, sizeof(output));
    if (status != 0) {
        fprintf(stderr, "Compilation failed:\n%s\n", output);
        return 1;
    }
    
    /* Run helper to generate .gcda file */
    snprintf(cmd, sizeof(cmd), "./%s", HELPER_BIN);
    printf("Running helper to generate GCOV data\n");
    status = execute_command(cmd, NULL, 0);
    if (status != 0) {
        fprintf(stderr, "Helper execution failed\n");
        return 1;
    }
    
    /* Check that .gcda file was created */
    char gcda_file[MAX_CMD_LEN];
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", HELPER_SRC);
    if (!file_exists(gcda_file)) {
        fprintf(stderr, "GCOV data file not created: %s\n", gcda_file);
        return 1;
    }
    
    printf("GCOV data file created: %s\n", gcda_file);
    
    /* Test 1: Valid flag (to ensure tool works) */
    printf("\n=== Test 1: Valid flag (-l) ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s 2>&1", gcda_file);
    status = execute_command(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status);
    printf("Output (first 200 chars):\n%.200s\n", output);
    
    /* Test 2-7: Invalid single-character flags */
    char invalid_flags[] = "a?x9!z";
    int tests_passed = 0;
    int tests_failed = 0;
    
    for (int i = 0; i < strlen(invalid_flags); i++) {
        printf("\n=== Test %d: Invalid flag (-%c) ===\n", 
               i + 2, invalid_flags[i]);
        
        snprintf(cmd, sizeof(cmd), "gcov-dump -%c %s 2>&1", 
                 invalid_flags[i], gcda_file);
        status = execute_command(cmd, output, sizeof(output));
        
        printf("Exit status: %d\n", status);
        printf("Output: %s", output);
        
        /* Check for expected error message */
        if (strstr(output, "unknown flag") != NULL) {
            printf("✓ Found 'unknown flag' in output\n");
            tests_passed++;
        } else {
            printf("✗ 'unknown flag' NOT found in output\n");
            tests_failed++;
        }
        
        /* Check for non-zero exit status */
        if (status != 0) {
            printf("✓ Non-zero exit status (expected)\n");
        } else {
            printf("✗ Zero exit status (unexpected)\n");
            tests_failed++;
        }
    }
    
    /* Test 8: Multiple invalid flags in one call */
    printf("\n=== Test 8: Multiple invalid flags (-ab) ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -ab %s 2>&1", gcda_file);
    status = execute_command(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status);
    printf("Output: %s", output);
    
    /* Test 9: Flag with no argument */
    printf("\n=== Test 9: Flag with no GCOV file ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -x 2>&1");
    status = execute_command(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status);
    printf("Output: %s", output);
    
    /* Test 10: Valid flag combined with invalid flag */
    printf("\n=== Test 10: Valid + invalid flag (-l -x) ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -x %s 2>&1", gcda_file);
    status = execute_command(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status);
    printf("Output: %s", output);
    
    /* Summary */
    printf("\n=== Summary ===\n");
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    
    /* Cleanup */
    printf("\nCleaning up temp directory: %s\n", temp_dir);
    chdir("..");
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
    system(cmd);
    
    return (tests_failed > 0) ? 1 : 0;
}
