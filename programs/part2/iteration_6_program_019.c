#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_PATH 1024
#define MAX_CMD 2048

/* Simple helper C source that will generate GCOV data */
const char *helper_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Generating GCOV data...\\n\");\n"
"    return 0;\n"
"}\n";

/* Function to check if a file exists */
int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

/* Execute a command and capture its output */
char *execute_command(const char *cmd, int *exit_status) {
    char buffer[4096];
    static char output[8192];
    FILE *fp;
    
    output[0] = '\0';
    
    /* Use popen with stderr redirected to stdout */
    char full_cmd[MAX_CMD];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    fp = popen(full_cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return NULL;
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        strcat(output, buffer);
    }
    
    *exit_status = pclose(fp);
    
    return output;
}

int main(int argc, char *argv[]) {
    char helper_c_path[MAX_PATH];
    char helper_exe_path[MAX_PATH];
    char gcda_path[MAX_PATH];
    char cmd[MAX_CMD];
    char *output;
    int exit_status;
    int tests_passed = 0;
    int tests_failed = 0;
    
    printf("=== GCOV-Dump Invalid Flag Test ===\n");
    
    /* Create temporary directory for test files */
    char tmp_dir[MAX_PATH];
    snprintf(tmp_dir, sizeof(tmp_dir), "/tmp/gcov_test_%d", getpid());
    mkdir(tmp_dir, 0755);
    
    /* Create helper source file */
    snprintf(helper_c_path, sizeof(helper_c_path), "%s/helper.c", tmp_dir);
    FILE *fp = fopen(helper_c_path, "w");
    if (!fp) {
        perror("Failed to create helper.c");
        return 1;
    }
    fprintf(fp, "%s", helper_source);
    fclose(fp);
    
    /* Compile helper with coverage instrumentation */
    snprintf(helper_exe_path, sizeof(helper_exe_path), "%s/helper", tmp_dir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -o %s %s",
             helper_exe_path, helper_c_path);
    
    printf("Compiling helper program...\n");
    system(cmd);
    
    if (!file_exists(helper_exe_path)) {
        printf("ERROR: Failed to compile helper program\n");
        return 1;
    }
    
    /* Run helper to generate .gcda file */
    printf("Running helper to generate GCOV data...\n");
    system(helper_exe_path);
    
    /* Check for generated .gcda file */
    snprintf(gcda_path, sizeof(gcda_path), "%s/helper.gcda", tmp_dir);
    if (!file_exists(gcda_path)) {
        /* Try alternative location */
        snprintf(gcda_path, sizeof(gcda_path), "helper.gcda");
        if (!file_exists(gcda_path)) {
            printf("ERROR: No .gcda file generated\n");
            return 1;
        }
    }
    
    printf("GCOV data file: %s\n", gcda_path);
    printf("\n");
    
    /* Test 1: Valid flag (to ensure gcov-dump works) */
    printf("Test 1: Valid flag (-l)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_path);
    output = execute_command(cmd, &exit_status);
    if (output && exit_status == 0) {
        printf("  PASS: gcov-dump executed successfully with valid flag\n");
        tests_passed++;
    } else {
        printf("  FAIL: gcov-dump failed with valid flag\n");
        tests_failed++;
    }
    printf("\n");
    
    /* Test 2-7: Invalid single-character flags */
    char *invalid_flags[] = {"-a", "-z", "-x", "-1", "-?", "-@"};
    char *flag_descriptions[] = {
        "alphabetic (a)", 
        "alphabetic (z)", 
        "alphabetic (x)", 
        "numeric (1)", 
        "symbol (?)", 
        "symbol (@)"
    };
    
    for (int i = 0; i < 6; i++) {
        printf("Test %d: Invalid flag %s\n", i+2, flag_descriptions[i]);
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s", invalid_flags[i], gcda_path);
        output = execute_command(cmd, &exit_status);
        
        if (output) {
            /* Check for error message in output */
            if (strstr(output, "unknown flag") != NULL) {
                printf("  PASS: Got 'unknown flag' error message\n");
                tests_passed++;
                
                /* Print the actual error message */
                char *error_start = strstr(output, "unknown flag");
                if (error_start) {
                    char *line_end = strchr(error_start, '\n');
                    if (line_end) {
                        *line_end = '\0';
                    }
                    printf("  Message: %s\n", error_start);
                }
            } else {
                printf("  FAIL: No 'unknown flag' message found\n");
                printf("  Output: %s\n", output);
                tests_failed++;
            }
            
            /* Check exit status (should be non-zero for error) */
            if (exit_status != 0) {
                printf("  PASS: Non-zero exit status (%d)\n", exit_status);
                tests_passed++;
            } else {
                printf("  FAIL: Zero exit status (should be non-zero)\n");
                tests_failed++;
            }
        } else {
            printf("  FAIL: No output from command\n");
            tests_failed++;
        }
        printf("\n");
    }
    
    /* Test 8: Multiple invalid flags */
    printf("Test 8: Multiple invalid flags (-a -z)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -a -z %s", gcda_path);
    output = execute_command(cmd, &exit_status);
    
    if (output && strstr(output, "unknown flag") != NULL) {
        printf("  PASS: Got 'unknown flag' error message\n");
        tests_passed++;
    } else {
        printf("  FAIL: Expected error message not found\n");
        tests_failed++;
    }
    printf("\n");
    
    /* Test 9: Valid flag followed by invalid flag */
    printf("Test 9: Valid flag followed by invalid flag (-l -a)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -a %s", gcda_path);
    output = execute_command(cmd, &exit_status);
    
    if (output && strstr(output, "unknown flag") != NULL) {
        printf("  PASS: Got 'unknown flag' error message\n");
        tests_passed++;
    } else {
        printf("  FAIL: Expected error message not found\n");
        tests_failed++;
    }
    printf("\n");
    
    /* Test 10: Just the dash without character */
    printf("Test 10: Just dash without character (-)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump - %s", gcda_path);
    output = execute_command(cmd, &exit_status);
    
    if (output) {
        /* This might trigger different error, but should still fail */
        printf("  Output: %s\n", output);
        if (exit_status != 0) {
            printf("  PASS: Non-zero exit status as expected\n");
            tests_passed++;
        } else {
            printf("  FAIL: Zero exit status\n");
            tests_failed++;
        }
    }
    printf("\n");
    
    /* Summary */
    printf("=== Test Summary ===\n");
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    printf("Total tests:  %d\n", tests_passed + tests_failed);
    
    /* Cleanup */
    printf("\nCleaning up temporary files...\n");
    snprintf(cmd, sizeof(cmd), "rm -rf %s", tmp_dir);
    system(cmd);
    
    if (file_exists("helper.gcda")) {
        system("rm -f helper helper.gcda helper.gcno helper.c");
    }
    
    return tests_failed > 0 ? 1 : 0;
}
