#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024
#define MAX_CMD 2048

/* Simple helper program to generate GCOV data */
const char *helper_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Generating GCOV data...\\n\");\n"
"    return 0;\n"
"}\n";

/* Execute a command and capture its stderr output */
int execute_and_capture(const char *cmd, char *output, size_t output_size) {
    FILE *fp;
    char cmd_with_stderr[MAX_CMD];
    
    /* Redirect stderr to stdout for capture */
    snprintf(cmd_with_stderr, sizeof(cmd_with_stderr), "%s 2>&1", cmd);
    
    fp = popen(cmd_with_stderr, "r");
    if (fp == NULL) {
        return -1;
    }
    
    /* Read output */
    size_t total_read = 0;
    while (fgets(output + total_read, output_size - total_read, fp) != NULL) {
        total_read = strlen(output);
        if (total_read >= output_size - 1) {
            break;
        }
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
}

/* Check if output contains "unknown flag" */
int contains_unknown_flag(const char *output) {
    return strstr(output, "unknown flag") != NULL;
}

int main() {
    char temp_dir[MAX_PATH];
    char helper_c_path[MAX_PATH];
    char helper_exe_path[MAX_PATH];
    char gcda_path[MAX_PATH];
    char cmd[MAX_CMD];
    char output[4096];
    int status;
    int tests_passed = 0;
    int tests_total = 0;
    
    /* Create a temporary directory for test files */
    snprintf(temp_dir, sizeof(temp_dir), "/tmp/gcov_test_%d", getpid());
    mkdir(temp_dir, 0700);
    
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
             "gcc -fprofile-arcs -ftest-coverage -o %s %s 2>&1",
             helper_exe_path, helper_c_path);
    
    status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile helper program\n");
        return 1;
    }
    
    /* Run helper to generate .gcda file */
    snprintf(cmd, sizeof(cmd), "cd %s && ./helper 2>&1", temp_dir);
    status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to run helper program\n");
        return 1;
    }
    
    /* Get path to .gcda file */
    snprintf(gcda_path, sizeof(gcda_path), "%s/helper.gcda", temp_dir);
    
    /* Test 1: Valid flag to ensure gcov-dump works */
    tests_total++;
    printf("\n=== Test 1: Valid flag (-l) ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_path);
    status = execute_and_capture(cmd, output, sizeof(output));
    printf("Command: %s\n", cmd);
    printf("Exit status: %d\n", status);
    
    if (status == 0) {
        printf("✓ Valid flag test passed\n");
        tests_passed++;
    } else {
        printf("✗ Valid flag test failed\n");
        printf("Output:\n%s\n", output);
    }
    
    /* Test 2-7: Various invalid single-character flags */
    char invalid_flags[] = {'a', 'z', '1', '?', 'x', 'A'};
    char flag_str[3] = "- ";
    
    for (int i = 0; i < sizeof(invalid_flags); i++) {
        tests_total++;
        flag_str[1] = invalid_flags[i];
        
        printf("\n=== Test %d: Invalid flag (%s) ===\n", i+2, flag_str);
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s", flag_str, gcda_path);
        status = execute_and_capture(cmd, output, sizeof(output));
        printf("Command: %s\n", cmd);
        printf("Exit status: %d\n", status);
        printf("Output:\n%s\n", output);
        
        int has_unknown_flag = contains_unknown_flag(output);
        int has_nonzero_exit = (status != 0);
        
        if (has_unknown_flag && has_nonzero_exit) {
            printf("✓ Invalid flag '%c' correctly rejected\n", invalid_flags[i]);
            tests_passed++;
        } else {
            printf("✗ Invalid flag '%c' test failed\n", invalid_flags[i]);
            if (!has_unknown_flag) {
                printf("  Missing 'unknown flag' message\n");
            }
            if (!has_nonzero_exit) {
                printf("  Expected non-zero exit status\n");
            }
        }
    }
    
    /* Test 8: Multiple invalid flags in one call */
    tests_total++;
    printf("\n=== Test 8: Multiple invalid flags ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -a -b -c %s", gcda_path);
    status = execute_and_capture(cmd, output, sizeof(output));
    printf("Command: %s\n", cmd);
    printf("Exit status: %d\n", status);
    printf("Output:\n%s\n", output);
    
    if (contains_unknown_flag(output) && status != 0) {
        printf("✓ Multiple invalid flags correctly rejected\n");
        tests_passed++;
    } else {
        printf("✗ Multiple invalid flags test failed\n");
    }
    
    /* Test 9: Valid flag combined with invalid flag */
    tests_total++;
    printf("\n=== Test 9: Mixed valid and invalid flags ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -z %s", gcda_path);
    status = execute_and_capture(cmd, output, sizeof(output));
    printf("Command: %s\n", cmd);
    printf("Exit status: %d\n", status);
    printf("Output:\n%s\n", output);
    
    if (contains_unknown_flag(output) && status != 0) {
        printf("✓ Mixed flags correctly rejected invalid one\n");
        tests_passed++;
    } else {
        printf("✗ Mixed flags test failed\n");
    }
    
    /* Cleanup */
    printf("\n=== Cleanup ===\n");
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
    system(cmd);
    printf("Removed temporary directory: %s\n", temp_dir);
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d tests\n", tests_passed, tests_total);
    
    return (tests_passed == tests_total) ? 0 : 1;
}
