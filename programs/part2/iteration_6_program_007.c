#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024
#define MAX_CMD 2048

/* Simple helper program to generate gcda file */
const char *helper_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Generating coverage data...\\n\");\n"
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

/* Check if output contains the expected error message */
int contains_unknown_flag(const char *output) {
    return strstr(output, "unknown flag") != NULL;
}

int main() {
    char tmpdir[MAX_PATH];
    char helper_c_path[MAX_PATH];
    char helper_exe_path[MAX_PATH];
    char gcda_path[MAX_PATH];
    char cmd[MAX_CMD];
    char output[4096];
    int status;
    int tests_passed = 0;
    int tests_failed = 0;
    
    /* Create a temporary directory for test files */
    snprintf(tmpdir, sizeof(tmpdir), "/tmp/gcov_test_%d", getpid());
    mkdir(tmpdir, 0755);
    
    /* Write helper source file */
    snprintf(helper_c_path, sizeof(helper_c_path), "%s/helper.c", tmpdir);
    FILE *fp = fopen(helper_c_path, "w");
    if (!fp) {
        perror("Failed to create helper.c");
        return 1;
    }
    fputs(helper_source, fp);
    fclose(fp);
    
    /* Compile helper with coverage instrumentation */
    snprintf(helper_exe_path, sizeof(helper_exe_path), "%s/helper", tmpdir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -o %s %s",
             helper_exe_path, helper_c_path);
    
    status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile helper program\n");
        return 1;
    }
    
    /* Run helper to generate gcda file */
    snprintf(gcda_path, sizeof(gcda_path), "%s/helper.gcda", tmpdir);
    snprintf(cmd, sizeof(cmd), "cd %s && ./helper", tmpdir);
    status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to run helper program\n");
        return 1;
    }
    
    /* Verify gcda file was created */
    struct stat st;
    if (stat(gcda_path, &st) != 0) {
        fprintf(stderr, "gcda file not created: %s\n", gcda_path);
        return 1;
    }
    
    printf("Generated gcda file: %s\n", gcda_path);
    
    /* Test 1: Valid flag to ensure gcov-dump works */
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
        tests_failed++;
    }
    
    /* Test 2-7: Various invalid single-character flags */
    const char *invalid_flags[] = {"-a", "-z", "-x", "-1", "-?", "-@"};
    int num_invalid_tests = sizeof(invalid_flags) / sizeof(invalid_flags[0]);
    
    for (int i = 0; i < num_invalid_tests; i++) {
        printf("\n=== Test %d: Invalid flag %s ===\n", i + 2, invalid_flags[i]);
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s", invalid_flags[i], gcda_path);
        status = execute_and_capture(cmd, output, sizeof(output));
        
        printf("Command: %s\n", cmd);
        printf("Exit status: %d\n", status);
        printf("Output: %s", output);
        
        if (contains_unknown_flag(output) && status != 0) {
            printf("✓ Invalid flag test passed (found 'unknown flag' error)\n");
            tests_passed++;
        } else {
            printf("✗ Invalid flag test failed\n");
            tests_failed++;
        }
    }
    
    /* Test 8: Multiple invalid flags in sequence */
    printf("\n=== Test 8: Multiple invalid flags ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -a -b -c %s", gcda_path);
    status = execute_and_capture(cmd, output, sizeof(output));
    
    printf("Command: %s\n", cmd);
    printf("Exit status: %d\n", status);
    printf("Output: %s", output);
    
    if (contains_unknown_flag(output) && status != 0) {
        printf("✓ Multiple invalid flags test passed\n");
        tests_passed++;
    } else {
        printf("✗ Multiple invalid flags test failed\n");
        tests_failed++;
    }
    
    /* Test 9: Mix valid and invalid flags */
    printf("\n=== Test 9: Mix valid and invalid flags ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -x -p %s", gcda_path);
    status = execute_and_capture(cmd, output, sizeof(output));
    
    printf("Command: %s\n", cmd);
    printf("Exit status: %d\n", status);
    printf("Output: %s", output);
    
    if (contains_unknown_flag(output) && status != 0) {
        printf("✓ Mixed flags test passed\n");
        tests_passed++;
    } else {
        printf("✗ Mixed flags test failed\n");
        tests_failed++;
    }
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    snprintf(cmd, sizeof(cmd), "rm -rf %s", tmpdir);
    system(cmd);
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", tests_passed + tests_failed);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    
    return tests_failed > 0 ? 1 : 0;
}
