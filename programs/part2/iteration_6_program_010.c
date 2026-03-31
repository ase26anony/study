#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024
#define MAX_CMD 2048

/* Simple helper program that will generate GCOV data */
const char *helper_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Helper program executed\\n\");\n"
"    return 0;\n"
"}\n";

/* Execute a command and capture its output */
char *execute_command(const char *cmd, int *exit_status) {
    char buffer[4096];
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

/* Check if output contains "unknown flag" message */
int contains_unknown_flag(const char *output) {
    return strstr(output, "unknown flag") != NULL;
}

int main() {
    char cwd[MAX_PATH];
    char helper_c_path[MAX_PATH];
    char helper_exe_path[MAX_PATH];
    char gcda_path[MAX_PATH];
    char cmd[MAX_CMD];
    int exit_status;
    char *output;
    int tests_passed = 0;
    int tests_total = 0;
    
    /* Get current working directory */
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed");
        return 1;
    }
    
    /* Create paths for helper files */
    snprintf(helper_c_path, sizeof(helper_c_path), "%s/helper_gcov.c", cwd);
    snprintf(helper_exe_path, sizeof(helper_exe_path), "%s/helper_gcov", cwd);
    snprintf(gcda_path, sizeof(gcda_path), "%s/helper_gcov.gcda", cwd);
    
    /* Step 1: Generate GCOV data file */
    printf("=== Step 1: Generating GCOV data ===\n");
    
    /* Write helper source file */
    FILE *fp = fopen(helper_c_path, "w");
    if (fp == NULL) {
        perror("Failed to create helper source file");
        return 1;
    }
    fprintf(fp, "%s", helper_source);
    fclose(fp);
    
    /* Compile helper with coverage instrumentation */
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -o %s %s 2>&1", 
             helper_exe_path, helper_c_path);
    printf("Compiling helper: %s\n", cmd);
    output = execute_command(cmd, &exit_status);
    free(output);
    
    if (exit_status != 0) {
        printf("Failed to compile helper program\n");
        return 1;
    }
    
    /* Run helper to generate .gcda file */
    printf("Running helper to generate .gcda file\n");
    snprintf(cmd, sizeof(cmd), "%s", helper_exe_path);
    output = execute_command(cmd, &exit_status);
    free(output);
    
    /* Verify .gcda file was created */
    struct stat st;
    if (stat(gcda_path, &st) != 0) {
        printf("ERROR: .gcda file not created at %s\n", gcda_path);
        return 1;
    }
    printf("GCOV data file created: %s\n", gcda_path);
    
    /* Step 2: Test gcov-dump with various flags */
    printf("\n=== Step 2: Testing gcov-dump with invalid flags ===\n");
    
    /* First, test with a valid flag to ensure basic functionality */
    printf("\n--- Test 1: Valid flag (should succeed) ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s 2>&1", gcda_path);
    printf("Command: %s\n", cmd);
    output = execute_command(cmd, &exit_status);
    if (output) {
        printf("Output (first 200 chars): %.200s\n", output);
        if (exit_status == 0) {
            printf("✓ Valid flag test passed\n");
            tests_passed++;
        } else {
            printf("✗ Valid flag test failed (exit status: %d)\n", exit_status);
        }
        free(output);
    }
    tests_total++;
    
    /* Test with various invalid single-character flags */
    const char *invalid_flags[] = {"a", "z", "1", "?", "x", "!", "@", "9", "q", "w"};
    int num_invalid_flags = sizeof(invalid_flags) / sizeof(invalid_flags[0]);
    
    for (int i = 0; i < num_invalid_flags; i++) {
        printf("\n--- Test %d: Invalid flag -%s ---\n", i + 2, invalid_flags[i]);
        snprintf(cmd, sizeof(cmd), "gcov-dump -%s %s 2>&1", invalid_flags[i], gcda_path);
        printf("Command: %s\n", cmd);
        output = execute_command(cmd, &exit_status);
        
        if (output) {
            /* Check for the expected error message */
            if (contains_unknown_flag(output)) {
                printf("✓ Found 'unknown flag' message\n");
                tests_passed++;
            } else {
                printf("✗ Missing 'unknown flag' message\n");
                printf("Output: %s\n", output);
            }
            
            /* Check for non-zero exit status */
            if (exit_status != 0) {
                printf("✓ Non-zero exit status: %d\n", exit_status);
            } else {
                printf("✗ Unexpected zero exit status\n");
            }
            free(output);
        } else {
            printf("✗ Command execution failed\n");
        }
        tests_total++;
    }
    
    /* Test edge case: just a dash with no character */
    printf("\n--- Edge case: Just '-' ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump - %s 2>&1", gcda_path);
    printf("Command: %s\n", cmd);
    output = execute_command(cmd, &exit_status);
    if (output) {
        printf("Output: %s\n", output);
        free(output);
    }
    
    /* Test combination: valid flag followed by invalid flag */
    printf("\n--- Test: Combination -l -a (valid + invalid) ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -a %s 2>&1", gcda_path);
    printf("Command: %s\n", cmd);
    output = execute_command(cmd, &exit_status);
    if (output) {
        if (contains_unknown_flag(output)) {
            printf("✓ Found 'unknown flag' message in combination\n");
            tests_passed++;
        } else {
            printf("Output: %s\n", output);
        }
        free(output);
    }
    tests_total++;
    
    /* Step 3: Cleanup */
    printf("\n=== Step 3: Cleanup ===\n");
    unlink(helper_c_path);
    unlink(helper_exe_path);
    unlink(gcda_path);
    
    /* Also remove other GCOV files that might have been created */
    char gcno_path[MAX_PATH];
    snprintf(gcno_path, sizeof(gcno_path), "%s/helper_gcov.gcno", cwd);
    unlink(gcno_path);
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, tests_total);
    printf("Success rate: %.1f%%\n", (tests_passed * 100.0) / tests_total);
    
    return (tests_passed == tests_total) ? 0 : 1;
}
