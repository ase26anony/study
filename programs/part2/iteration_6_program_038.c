#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define TEMP_DIR "/tmp/gcov_test_XXXXXX"
#define HELPER_SRC "helper.c"
#define HELPER_BIN "helper"
#define GCOV_DATA "helper.gcda"

/* Simple C program to generate GCOV data */
const char *helper_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Generating GCOV data...\\n\");\n"
"    return 0;\n"
"}\n";

/* Execute a command and capture its output */
char *execute_command(const char *cmd, int *exit_status) {
    char buffer[1024];
    char *result = malloc(4096);
    result[0] = '\0';
    
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        free(result);
        return NULL;
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        strcat(result, buffer);
    }
    
    *exit_status = pclose(fp);
    return result;
}

/* Check if output contains expected error message */
int contains_unknown_flag(const char *output) {
    return strstr(output, "unknown flag") != NULL;
}

int main() {
    char temp_dir[256];
    char helper_src_path[512];
    char helper_bin_path[512];
    char gcda_path[512];
    char cmd[1024];
    int status;
    char *output;
    
    /* Create temporary directory */
    strcpy(temp_dir, TEMP_DIR);
    if (mkdtemp(temp_dir) == NULL) {
        perror("Failed to create temp directory");
        return 1;
    }
    
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Create helper source file path */
    snprintf(helper_src_path, sizeof(helper_src_path), "%s/%s", temp_dir, HELPER_SRC);
    snprintf(helper_bin_path, sizeof(helper_bin_path), "%s/%s", temp_dir, HELPER_BIN);
    snprintf(gcda_path, sizeof(gcda_path), "%s/%s", temp_dir, GCOV_DATA);
    
    /* Write helper source file */
    FILE *fp = fopen(helper_src_path, "w");
    if (fp == NULL) {
        perror("Failed to create helper source");
        return 1;
    }
    fputs(helper_source, fp);
    fclose(fp);
    
    /* Compile helper with coverage flags */
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -o %s %s", 
             helper_bin_path, helper_src_path);
    printf("Compiling: %s\n", cmd);
    system(cmd);
    
    /* Run helper to generate .gcda file */
    snprintf(cmd, sizeof(cmd), "%s", helper_bin_path);
    printf("Running helper: %s\n", cmd);
    system(cmd);
    
    /* Verify .gcda file exists */
    struct stat st;
    if (stat(gcda_path, &st) != 0) {
        fprintf(stderr, "Failed to create .gcda file\n");
        return 1;
    }
    
    printf("\n=== Testing gcov-dump with various flags ===\n\n");
    
    /* Test 1: Valid flag (should work) */
    printf("Test 1: Valid flag (-l)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s 2>&1", gcda_path);
    output = execute_command(cmd, &status);
    if (output) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
        printf("Output (first 200 chars): %.200s\n", output);
        free(output);
    }
    
    /* Test 2-7: Invalid single-character flags */
    char *invalid_flags[] = {"-a", "-z", "-x", "-1", "-?", "-@"};
    int num_tests = sizeof(invalid_flags) / sizeof(invalid_flags[0]);
    
    for (int i = 0; i < num_tests; i++) {
        printf("\nTest %d: Invalid flag %s\n", i + 2, invalid_flags[i]);
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s 2>&1", invalid_flags[i], gcda_path);
        output = execute_command(cmd, &status);
        
        if (output) {
            int exit_code = WEXITSTATUS(status);
            printf("Exit status: %d\n", exit_code);
            
            if (contains_unknown_flag(output)) {
                printf("✓ Found 'unknown flag' message\n");
            } else {
                printf("✗ Did not find 'unknown flag' message\n");
                printf("Output: %s\n", output);
            }
            
            /* Verify non-zero exit status for invalid flags */
            if (exit_code == 0) {
                printf("Warning: Invalid flag returned exit code 0\n");
            }
            
            free(output);
        } else {
            printf("Failed to execute command\n");
        }
    }
    
    /* Test 8: Multiple invalid flags in one call */
    printf("\nTest 8: Multiple invalid flags (-a -z)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -a -z %s 2>&1", gcda_path);
    output = execute_command(cmd, &status);
    if (output) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
        if (contains_unknown_flag(output)) {
            printf("✓ Found 'unknown flag' message\n");
        }
        free(output);
    }
    
    /* Test 9: Valid flag combined with invalid flag */
    printf("\nTest 9: Valid + invalid flag (-l -a)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -a %s 2>&1", gcda_path);
    output = execute_command(cmd, &status);
    if (output) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
        if (contains_unknown_flag(output)) {
            printf("✓ Found 'unknown flag' message\n");
        }
        free(output);
    }
    
    /* Test 10: No flags (should work) */
    printf("\nTest 10: No flags\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump %s 2>&1", gcda_path);
    output = execute_command(cmd, &status);
    if (output) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
        free(output);
    }
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
    system(cmd);
    printf("Removed temp directory: %s\n", temp_dir);
    
    return 0;
}
