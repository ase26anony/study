#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_PATH 1024
#define MAX_CMD 2048

/* Simple helper program source code */
const char *helper_src = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Helper program executed\\n\");\n"
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
    static char result[8192];
    FILE *fp;
    
    result[0] = '\0';
    
    /* Use popen with stderr redirected to stdout */
    char full_cmd[MAX_CMD];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    fp = popen(full_cmd, "r");
    if (fp == NULL) {
        strcpy(result, "Failed to execute command");
        if (exit_status) *exit_status = -1;
        return result;
    }
    
    /* Read output */
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        strcat(result, buffer);
    }
    
    /* Get exit status */
    if (exit_status) {
        *exit_status = pclose(fp);
    } else {
        pclose(fp);
    }
    
    return result;
}

/* Clean up temporary files */
void cleanup(const char *helper_c, const char *helper_exe, 
             const char *gcda_file, const char *gcno_file) {
    if (helper_c && file_exists(helper_c)) {
        unlink(helper_c);
    }
    if (helper_exe && file_exists(helper_exe)) {
        unlink(helper_exe);
    }
    if (gcda_file && file_exists(gcda_file)) {
        unlink(gcda_file);
    }
    if (gcno_file && file_exists(gcno_file)) {
        unlink(gcno_file);
    }
}

int main() {
    char helper_c_path[MAX_PATH];
    char helper_exe_path[MAX_PATH];
    char gcda_path[MAX_PATH];
    char gcno_path[MAX_PATH];
    char cmd[MAX_CMD];
    char *output;
    int exit_status;
    int tests_passed = 0;
    int tests_failed = 0;
    
    /* Create unique temporary filenames */
    snprintf(helper_c_path, sizeof(helper_c_path), "/tmp/helper_%d.c", getpid());
    snprintf(helper_exe_path, sizeof(helper_exe_path), "/tmp/helper_%d", getpid());
    snprintf(gcda_path, sizeof(gcda_path), "/tmp/helper_%d.gcda", getpid());
    snprintf(gcno_path, sizeof(gcno_path), "/tmp/helper_%d.gcno", getpid());
    
    printf("=== GCOV-Dump Test Program ===\n");
    printf("Temporary files:\n");
    printf("  Source: %s\n", helper_c_path);
    printf("  Executable: %s\n", helper_exe_path);
    printf("  GCOV data: %s\n", gcda_path);
    
    /* Step 1: Generate GCOV data file */
    printf("\n1. Generating GCOV data file...\n");
    
    /* Write helper source file */
    FILE *fp = fopen(helper_c_path, "w");
    if (!fp) {
        perror("Failed to create helper source file");
        return 1;
    }
    fputs(helper_src, fp);
    fclose(fp);
    
    /* Compile helper with coverage instrumentation */
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -o %s %s",
             helper_exe_path, helper_c_path);
    printf("Compiling: %s\n", cmd);
    
    output = execute_command(cmd, &exit_status);
    if (exit_status != 0) {
        printf("Compilation failed:\n%s\n", output);
        cleanup(helper_c_path, helper_exe_path, gcda_path, gcno_path);
        return 1;
    }
    
    /* Run helper to generate .gcda file */
    printf("Running helper program...\n");
    output = execute_command(helper_exe_path, &exit_status);
    printf("Output: %s", output);
    
    /* Verify .gcda file was created */
    if (!file_exists(gcda_path)) {
        /* Try alternative path (gcda might be in current directory) */
        snprintf(gcda_path, sizeof(gcda_path), "helper_%d.gcda", getpid());
        if (!file_exists(gcda_path)) {
            printf("ERROR: .gcda file not found!\n");
            printf("Tried: /tmp/helper_%d.gcda and helper_%d.gcda\n", getpid(), getpid());
            cleanup(helper_c_path, helper_exe_path, gcda_path, gcno_path);
            return 1;
        }
    }
    
    printf("GCOV data file created successfully.\n");
    
    /* Step 2: Test gcov-dump with various flags */
    printf("\n2. Testing gcov-dump with various flags...\n");
    
    /* Test cases: valid flag, invalid flags, edge cases */
    struct {
        const char *flag;
        const char *description;
        int expect_error;
        const char *error_substring;
    } test_cases[] = {
        {"-l", "Valid flag (dump contents)", 0, NULL},
        {"-a", "Invalid flag 'a'", 1, "unknown flag"},
        {"-z", "Invalid flag 'z'", 1, "unknown flag"},
        {"-1", "Invalid flag '1' (numeric)", 1, "unknown flag"},
        {"-?", "Invalid flag '?' (special char)", 1, "unknown flag"},
        {"-x", "Invalid flag 'x'", 1, "unknown flag"},
        {"-!", "Invalid flag '!' (special char)", 1, "unknown flag"},
        {"-@", "Invalid flag '@' (special char)", 1, "unknown flag"},
        {"-p", "Valid flag (dump positions)", 0, NULL},
        {"-r", "Valid flag (dump raw)", 0, NULL},
        {"-s", "Valid flag (dump stable)", 0, NULL},
        {"-v", "Valid flag (version)", 0, NULL},
        {"-h", "Valid flag (help)", 0, NULL},
    };
    
    for (size_t i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        printf("\nTest %zu: %s\n", i+1, test_cases[i].description);
        
        /* Build command - for -h and -v, no data file needed */
        if (strcmp(test_cases[i].flag, "-h") == 0 || 
            strcmp(test_cases[i].flag, "-v") == 0) {
            snprintf(cmd, sizeof(cmd), "gcov-dump %s", test_cases[i].flag);
        } else {
            snprintf(cmd, sizeof(cmd), "gcov-dump %s %s", 
                    test_cases[i].flag, gcda_path);
        }
        
        printf("Command: %s\n", cmd);
        
        output = execute_command(cmd, &exit_status);
        
        /* Check results */
        if (test_cases[i].expect_error) {
            /* Should find "unknown flag" in output */
            if (strstr(output, test_cases[i].error_substring) != NULL) {
                printf("✓ Correctly detected unknown flag\n");
                printf("  Output: %s", output);
                tests_passed++;
            } else {
                printf("✗ Failed to detect unknown flag\n");
                printf("  Expected substring: '%s'\n", test_cases[i].error_substring);
                printf("  Actual output: %s", output);
                tests_failed++;
            }
            
            /* Exit status should be non-zero for error */
            if (exit_status != 0) {
                printf("✓ Non-zero exit status as expected\n");
            } else {
                printf("✗ Expected non-zero exit status but got %d\n", exit_status);
            }
        } else {
            /* Valid flag - should execute without "unknown flag" error */
            if (strstr(output, "unknown flag") == NULL) {
                printf("✓ Valid flag accepted\n");
                tests_passed++;
            } else {
                printf("✗ Valid flag incorrectly rejected\n");
                printf("  Output: %s", output);
                tests_failed++;
            }
        }
    }
    
    /* Step 3: Additional test - multiple invalid flags in one call */
    printf("\n3. Testing with multiple invalid flags...\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -a -z -x %s", gcda_path);
    printf("Command: %s\n", cmd);
    
    output = execute_command(cmd, &exit_status);
    if (strstr(output, "unknown flag") != NULL) {
        printf("✓ Detected invalid flag in multi-flag call\n");
        tests_passed++;
    } else {
        printf("✗ Failed to detect invalid flag in multi-flag call\n");
        tests_failed++;
    }
    
    /* Step 4: Summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    
    /* Cleanup */
    printf("\nCleaning up temporary files...\n");
    cleanup(helper_c_path, helper_exe_path, gcda_path, gcno_path);
    
    return tests_failed > 0 ? 1 : 0;
}
