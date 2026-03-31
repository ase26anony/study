#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_PATH 1024
#define MAX_CMD 2048

/* Helper function to execute a command and capture its output */
int execute_and_check(const char *cmd, const char *expected_error) {
    char buffer[4096];
    FILE *fp;
    int found = 0;
    
    printf("Executing: %s\n", cmd);
    
    /* Execute command and capture both stdout and stderr */
    fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return 0;
    }
    
    /* Read output */
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("Output: %s", buffer);
        if (expected_error && strstr(buffer, expected_error)) {
            found = 1;
        }
    }
    
    /* Get exit status */
    int status = pclose(fp);
    int exit_code = WEXITSTATUS(status);
    
    printf("Exit code: %d\n\n", exit_code);
    
    if (expected_error) {
        /* For invalid flag commands, we expect error message and non-zero exit */
        return found && (exit_code != 0);
    } else {
        /* For valid commands, we just want successful execution */
        return exit_code == 0;
    }
}

/* Create a simple C program to generate GCOV data */
void create_helper_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create helper source");
        exit(1);
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    printf(\"Helper program executed\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
}

int main(int argc, char *argv[]) {
    char helper_c[MAX_PATH];
    char helper_exe[MAX_PATH];
    char gcda_file[MAX_PATH];
    char cmd[MAX_CMD];
    char cwd[MAX_PATH];
    int success = 1;
    
    /* Get current directory for absolute paths */
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed");
        return 1;
    }
    
    /* Create temporary filenames */
    snprintf(helper_c, sizeof(helper_c), "%s/helper_gcov_test.c", cwd);
    snprintf(helper_exe, sizeof(helper_exe), "%s/helper_gcov_test", cwd);
    snprintf(gcda_file, sizeof(gcda_file), "%s/helper_gcov_test.gcda", cwd);
    
    /* Step 1: Generate GCOV data */
    printf("=== Step 1: Generating GCOV data ===\n");
    
    /* Create helper source file */
    create_helper_source(helper_c);
    
    /* Compile with coverage flags */
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -o %s %s", 
             helper_exe, helper_c);
    
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile helper program\n");
        /* Cleanup and exit */
        unlink(helper_c);
        return 1;
    }
    
    /* Run helper to generate .gcda file */
    if (system(helper_exe) != 0) {
        fprintf(stderr, "Failed to run helper program\n");
        success = 0;
        goto cleanup;
    }
    
    /* Verify .gcda file was created */
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "Failed to create .gcda file\n");
        success = 0;
        goto cleanup;
    }
    
    printf("Generated GCOV data at: %s\n\n", gcda_file);
    
    /* Step 2: Test gcov-dump with various flags */
    printf("=== Step 2: Testing gcov-dump ===\n");
    
    /* First, test with a valid flag to ensure basic functionality */
    printf("--- Testing valid flag (-l) ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s 2>&1", gcda_file);
    if (!execute_and_check(cmd, NULL)) {
        fprintf(stderr, "Valid flag test failed\n");
        success = 0;
    }
    
    /* Test multiple invalid single-character flags */
    printf("--- Testing invalid flags ---\n");
    
    /* Test alphabetic characters not in {h,v,l,p,r,s} */
    const char *invalid_flags[] = {"-a", "-b", "-c", "-x", "-y", "-z", "-1", "-?", "-@"};
    int num_flags = sizeof(invalid_flags) / sizeof(invalid_flags[0]);
    
    for (int i = 0; i < num_flags; i++) {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s 2>&1", 
                 invalid_flags[i], gcda_file);
        
        if (!execute_and_check(cmd, "unknown flag")) {
            fprintf(stderr, "Invalid flag test failed for %s\n", invalid_flags[i]);
            success = 0;
        }
    }
    
    /* Test combination: valid flag followed by invalid flag */
    printf("--- Testing flag combination ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -x %s 2>&1", gcda_file);
    if (!execute_and_check(cmd, "unknown flag")) {
        fprintf(stderr, "Flag combination test failed\n");
        success = 0;
    }
    
    /* Test invalid flag without data file (different error path) */
    printf("--- Testing invalid flag without data file ---\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -x 2>&1");
    if (!execute_and_check(cmd, "unknown flag")) {
        fprintf(stderr, "Invalid flag without file test failed\n");
        success = 0;
    }
    
cleanup:
    /* Step 3: Cleanup */
    printf("=== Step 3: Cleaning up ===\n");
    
    /* Remove generated files */
    unlink(helper_c);
    unlink(helper_exe);
    unlink(gcda_file);
    
    /* Also remove other GCOV files that might have been created */
    char gcno_file[MAX_PATH];
    snprintf(gcno_file, sizeof(gcno_file), "%s/helper_gcov_test.gcno", cwd);
    unlink(gcno_file);
    
    printf("Cleanup completed\n\n");
    
    if (success) {
        printf("=== All tests passed ===\n");
        return 0;
    } else {
        printf("=== Some tests failed ===\n");
        return 1;
    }
}
