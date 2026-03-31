#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_PATH 1024
#define MAX_CMD 2048

/* Simple helper C source to generate GCOV data */
const char *helper_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Generating GCOV data...\\n\");\n"
"    return 0;\n"
"}\n";

/* Execute a command and capture its stderr output */
char *execute_command(const char *cmd, int *exit_status) {
    char buffer[4096];
    static char result[8192];
    FILE *fp;
    
    result[0] = '\0';
    
    /* Use popen with stderr redirected to stdout */
    char full_cmd[MAX_CMD];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    fp = popen(full_cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return NULL;
    }
    
    /* Read the output */
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
    char temp_dir[MAX_PATH];
    char helper_c_path[MAX_PATH];
    char helper_exe_path[MAX_PATH];
    char gcda_path[MAX_PATH];
    char cmd[MAX_CMD];
    char *output;
    int exit_status;
    int tests_passed = 0;
    int tests_total = 0;
    
    /* Create a temporary directory for test files */
    snprintf(temp_dir, sizeof(temp_dir), "/tmp/gcov_test_%d", getpid());
    mkdir(temp_dir, 0755);
    
    /* Write helper source file */
    snprintf(helper_c_path, sizeof(helper_c_path), "%s/helper.c", temp_dir);
    FILE *fp = fopen(helper_c_path, "w");
    if (!fp) {
        perror("Failed to create helper.c");
        return 1;
    }
    fputs(helper_source, fp);
    fclose(fp);
    
    /* Compile helper with coverage instrumentation */
    snprintf(helper_exe_path, sizeof(helper_exe_path), "%s/helper", temp_dir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -o %s %s",
             helper_exe_path, helper_c_path);
    
    printf("Compiling helper program...\n");
    system(cmd);
    
    /* Run helper to generate .gcda file */
    printf("Running helper to generate GCOV data...\n");
    system(helper_exe_path);
    
    /* Find the .gcda file path */
    snprintf(gcda_path, sizeof(gcda_path), "%s/helper.gcda", temp_dir);
    
    /* Test 1: Valid command to ensure gcov-dump works */
    printf("\n=== Test 1: Valid command (gcov-dump -l) ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_path);
    output = execute_command(cmd, &exit_status);
    tests_total++;
    
    if (output && exit_status == 0) {
        printf("✓ Valid command succeeded\n");
        tests_passed++;
    } else {
        printf("✗ Valid command failed\n");
    }
    
    /* Test 2-7: Invalid single-character flags */
    char invalid_flags[] = {'a', 'z', 'x', '?', '1', '9'};
    char flag_str[3] = "- ";
    
    for (int i = 0; i < sizeof(invalid_flags); i++) {
        flag_str[1] = invalid_flags[i];
        
        printf("\n=== Test %d: Invalid flag %s ===\n", 
               i + 2, flag_str);
        
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s", flag_str, gcda_path);
        output = execute_command(cmd, &exit_status);
        tests_total++;
        
        if (output && contains_unknown_flag(output) && exit_status != 0) {
            printf("✓ Correctly detected unknown flag '%c'\n", invalid_flags[i]);
            printf("  Output: %s", output);
            tests_passed++;
        } else {
            printf("✗ Failed to detect unknown flag '%c'\n", invalid_flags[i]);
            if (output) printf("  Output: %s", output);
        }
    }
    
    /* Test 8: Multiple invalid flags in one call */
    printf("\n=== Test 8: Multiple invalid flags ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -a -z -x %s", gcda_path);
    output = execute_command(cmd, &exit_status);
    tests_total++;
    
    if (output && contains_unknown_flag(output) && exit_status != 0) {
        printf("✓ Correctly detected first unknown flag\n");
        printf("  Output: %s", output);
        tests_passed++;
    } else {
        printf("✗ Failed to detect unknown flag in multiple flags\n");
        if (output) printf("  Output: %s", output);
    }
    
    /* Test 9: Valid flag combined with invalid flag */
    printf("\n=== Test 9: Valid + invalid flag ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -q %s", gcda_path);
    output = execute_command(cmd, &exit_status);
    tests_total++;
    
    if (output && contains_unknown_flag(output) && exit_status != 0) {
        printf("✓ Correctly detected unknown flag 'q' after valid flag\n");
        printf("  Output: %s", output);
        tests_passed++;
    } else {
        printf("✗ Failed to detect unknown flag after valid flag\n");
        if (output) printf("  Output: %s", output);
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d tests\n", tests_passed, tests_total);
    
    /* Cleanup */
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
    system(cmd);
    
    return (tests_passed == tests_total) ? 0 : 1;
}
