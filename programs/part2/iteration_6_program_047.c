#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define TEMP_DIR "/tmp/gcov_dump_test"
#define HELPER_SRC TEMP_DIR "/helper.c"
#define HELPER_BIN TEMP_DIR "/helper"
#define GCOV_DATA TEMP_DIR "/helper.gcda"

/* Simple helper program that will generate GCOV data */
const char *helper_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Helper program running\\n\");\n"
"    return 0;\n"
"}\n";

/* Execute a shell command and capture output */
char *execute_command(const char *cmd, int *exit_status) {
    char buffer[1024];
    char *result = malloc(1);
    result[0] = '\0';
    FILE *fp = popen(cmd, "r");
    
    if (fp == NULL) {
        free(result);
        return NULL;
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        size_t old_len = strlen(result);
        size_t new_len = old_len + strlen(buffer) + 1;
        result = realloc(result, new_len);
        strcat(result, buffer);
    }
    
    *exit_status = pclose(fp);
    return result;
}

/* Check if file exists */
int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

/* Clean up temporary files */
void cleanup() {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", TEMP_DIR);
    system(cmd);
}

int main() {
    FILE *fp;
    int exit_status;
    char *output;
    int tests_passed = 0;
    int total_tests = 0;
    
    /* Create temporary directory */
    mkdir(TEMP_DIR, 0755);
    
    /* Step 1: Generate GCOV data file */
    printf("=== Generating GCOV data file ===\n");
    
    /* Write helper source */
    fp = fopen(HELPER_SRC, "w");
    if (!fp) {
        perror("Failed to create helper source");
        cleanup();
        return 1;
    }
    fputs(helper_source, fp);
    fclose(fp);
    
    /* Compile helper with coverage flags */
    char compile_cmd[512];
    snprintf(compile_cmd, sizeof(compile_cmd), 
             "gcc -fprofile-arcs -ftest-coverage -o %s %s 2>&1", 
             HELPER_BIN, HELPER_SRC);
    
    output = execute_command(compile_cmd, &exit_status);
    if (output && strlen(output) > 0) {
        printf("Compile output: %s\n", output);
    }
    free(output);
    
    if (!file_exists(HELPER_BIN)) {
        printf("ERROR: Failed to compile helper program\n");
        cleanup();
        return 1;
    }
    
    /* Run helper to generate .gcda file */
    snprintf(compile_cmd, sizeof(compile_cmd), "%s", HELPER_BIN);
    output = execute_command(compile_cmd, &exit_status);
    free(output);
    
    if (!file_exists(GCOV_DATA)) {
        printf("ERROR: Failed to generate .gcda file\n");
        printf("Trying alternative location...\n");
        /* Try current directory */
        snprintf(compile_cmd, sizeof(compile_cmd), "cp %s . 2>/dev/null", HELPER_BIN);
        system(compile_cmd);
        system("./helper");
    }
    
    /* Step 2: Test gcov-dump with various flags */
    printf("\n=== Testing gcov-dump with invalid flags ===\n");
    
    /* First, test with a valid flag to ensure gcov-dump works */
    total_tests++;
    printf("\nTest %d: Valid flag (-l)\n", total_tests);
    char valid_cmd[512];
    snprintf(valid_cmd, sizeof(valid_cmd), "gcov-dump -l %s 2>&1", GCOV_DATA);
    output = execute_command(valid_cmd, &exit_status);
    if (output && exit_status == 0) {
        printf("✓ Valid flag test passed\n");
        tests_passed++;
    } else {
        printf("✗ Valid flag test failed\n");
        if (output) printf("Output: %s\n", output);
    }
    free(output);
    
    /* Test invalid single-character flags */
    char *invalid_flags[] = {"a", "z", "1", "?", "x", "!", "@", "9", "A", "Z"};
    int num_invalid_flags = sizeof(invalid_flags) / sizeof(invalid_flags[0]);
    
    for (int i = 0; i < num_invalid_flags; i++) {
        total_tests++;
        printf("\nTest %d: Invalid flag (-%s)\n", total_tests, invalid_flags[i]);
        
        char test_cmd[512];
        snprintf(test_cmd, sizeof(test_cmd), "gcov-dump -%s %s 2>&1", 
                 invalid_flags[i], GCOV_DATA);
        
        output = execute_command(test_cmd, &exit_status);
        
        int found_error = 0;
        if (output) {
            /* Check for the expected error message */
            if (strstr(output, "unknown flag") != NULL) {
                found_error = 1;
                printf("✓ Found 'unknown flag' error message\n");
            }
            
            /* Also check that exit status is non-zero */
            if (exit_status != 0) {
                printf("✓ Exit status is non-zero (%d)\n", exit_status);
                if (found_error) {
                    tests_passed++;
                }
            } else {
                printf("✗ Exit status should be non-zero but was %d\n", exit_status);
            }
            
            /* Print first line of output for debugging */
            char *first_line = strtok(output, "\n");
            if (first_line) {
                printf("Output: %s\n", first_line);
            }
        }
        free(output);
    }
    
    /* Test with multiple invalid flags in one command */
    total_tests++;
    printf("\nTest %d: Multiple invalid flags (-a -z)\n", total_tests);
    char multi_cmd[512];
    snprintf(multi_cmd, sizeof(multi_cmd), "gcov-dump -a -z %s 2>&1", GCOV_DATA);
    output = execute_command(multi_cmd, &exit_status);
    if (output && strstr(output, "unknown flag") != NULL && exit_status != 0) {
        printf("✓ Multiple invalid flags test passed\n");
        tests_passed++;
    } else {
        printf("✗ Multiple invalid flags test failed\n");
    }
    free(output);
    
    /* Test with flag that has correct prefix but invalid character */
    total_tests++;
    printf("\nTest %d: Invalid flag with dash (-q)\n", total_tests);
    snprintf(multi_cmd, sizeof(multi_cmd), "gcov-dump -q %s 2>&1", GCOV_DATA);
    output = execute_command(multi_cmd, &exit_status);
    if (output && strstr(output, "unknown flag") != NULL && exit_status != 0) {
        printf("✓ Invalid flag -q test passed\n");
        tests_passed++;
    } else {
        printf("✗ Invalid flag -q test failed\n");
    }
    free(output);
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    
    /* Cleanup */
    cleanup();
    
    return (tests_passed == total_tests) ? 0 : 1;
}
