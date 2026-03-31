#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_PATH 1024
#define MAX_CMD 2048

/* Simple helper program that will generate .gcda file */
const char *helper_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Helper program executed\\n\");\n"
"    return 0;\n"
"}\n";

/* Function to check if a file exists */
int file_exists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

/* Execute a command and capture its output */
char *execute_command(const char *cmd, int *exit_status) {
    char buffer[4096];
    static char result[8192];
    FILE *fp;
    
    result[0] = '\0';
    
    /* Use popen to capture both stdout and stderr */
    char full_cmd[MAX_CMD];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    fp = popen(full_cmd, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to run command: %s\n", cmd);
        return NULL;
    }
    
    /* Read the entire output */
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        strcat(result, buffer);
    }
    
    *exit_status = pclose(fp);
    return result;
}

int main(int argc, char *argv[]) {
    char helper_c_path[MAX_PATH];
    char helper_exe_path[MAX_PATH];
    char gcda_path[MAX_PATH];
    char cmd[MAX_CMD];
    char *output;
    int exit_status;
    int tests_passed = 0;
    int tests_total = 0;
    
    /* Create temporary directory for test files */
    char tmpdir[MAX_PATH];
    snprintf(tmpdir, sizeof(tmpdir), "/tmp/gcov_test_%d", getpid());
    mkdir(tmpdir, 0755);
    
    /* Create helper source file */
    snprintf(helper_c_path, sizeof(helper_c_path), "%s/helper.c", tmpdir);
    FILE *fp = fopen(helper_c_path, "w");
    if (!fp) {
        fprintf(stderr, "Failed to create helper.c\n");
        return 1;
    }
    fputs(helper_source, fp);
    fclose(fp);
    
    /* Compile helper with coverage flags */
    snprintf(helper_exe_path, sizeof(helper_exe_path), "%s/helper", tmpdir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -o %s %s 2>&1",
             helper_exe_path, helper_c_path);
    
    printf("Compiling helper program...\n");
    output = execute_command(cmd, &exit_status);
    if (exit_status != 0) {
        fprintf(stderr, "Failed to compile helper: %s\n", output);
        return 1;
    }
    
    /* Run helper to generate .gcda file */
    printf("Running helper to generate .gcda...\n");
    snprintf(cmd, sizeof(cmd), "cd %s && ./helper", tmpdir);
    output = execute_command(cmd, &exit_status);
    if (exit_status != 0) {
        fprintf(stderr, "Failed to run helper: %s\n", output);
        return 1;
    }
    
    /* Verify .gcda file was created */
    snprintf(gcda_path, sizeof(gcda_path), "%s/helper.gcda", tmpdir);
    if (!file_exists(gcda_path)) {
        fprintf(stderr, "No .gcda file generated at %s\n", gcda_path);
        return 1;
    }
    
    printf("\n=== Testing gcov-dump with invalid flags ===\n\n");
    
    /* Test 1: Valid flag to ensure gcov-dump works */
    printf("Test 1: Valid flag (-l)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_path);
    output = execute_command(cmd, &exit_status);
    tests_total++;
    if (exit_status == 0) {
        printf("✓ Valid flag test passed\n");
        tests_passed++;
    } else {
        printf("✗ Valid flag test failed: %s\n", output);
    }
    
    /* Test 2: Invalid flag -a */
    printf("\nTest 2: Invalid flag (-a)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -a %s", gcda_path);
    output = execute_command(cmd, &exit_status);
    tests_total++;
    if (strstr(output, "unknown flag") != NULL && exit_status != 0) {
        printf("✓ Invalid flag '-a' triggered error: %s", output);
        tests_passed++;
    } else {
        printf("✗ Invalid flag '-a' test failed. Output: %s\n", output);
    }
    
    /* Test 3: Invalid flag -z */
    printf("\nTest 3: Invalid flag (-z)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -z %s", gcda_path);
    output = execute_command(cmd, &exit_status);
    tests_total++;
    if (strstr(output, "unknown flag") != NULL && exit_status != 0) {
        printf("✓ Invalid flag '-z' triggered error: %s", output);
        tests_passed++;
    } else {
        printf("✗ Invalid flag '-z' test failed. Output: %s\n", output);
    }
    
    /* Test 4: Invalid flag -1 (non-alphabetic) */
    printf("\nTest 4: Invalid flag (-1)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -1 %s", gcda_path);
    output = execute_command(cmd, &exit_status);
    tests_total++;
    if (strstr(output, "unknown flag") != NULL && exit_status != 0) {
        printf("✓ Invalid flag '-1' triggered error: %s", output);
        tests_passed++;
    } else {
        printf("✗ Invalid flag '-1' test failed. Output: %s\n", output);
    }
    
    /* Test 5: Invalid flag -? (special character) */
    printf("\nTest 5: Invalid flag (-?)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump \\-? %s", gcda_path);
    output = execute_command(cmd, &exit_status);
    tests_total++;
    if (strstr(output, "unknown flag") != NULL && exit_status != 0) {
        printf("✓ Invalid flag '-?' triggered error: %s", output);
        tests_passed++;
    } else {
        printf("✗ Invalid flag '-?' test failed. Output: %s\n", output);
    }
    
    /* Test 6: Invalid flag -x */
    printf("\nTest 6: Invalid flag (-x)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -x %s", gcda_path);
    output = execute_command(cmd, &exit_status);
    tests_total++;
    if (strstr(output, "unknown flag") != NULL && exit_status != 0) {
        printf("✓ Invalid flag '-x' triggered error: %s", output);
        tests_passed++;
    } else {
        printf("✗ Invalid flag '-x' test failed. Output: %s\n", output);
    }
    
    /* Test 7: Multiple invalid flags in sequence */
    printf("\nTest 7: Multiple invalid flags (-b -c -d)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -b -c -d %s", gcda_path);
    output = execute_command(cmd, &exit_status);
    tests_total++;
    if (strstr(output, "unknown flag") != NULL && exit_status != 0) {
        printf("✓ Multiple invalid flags triggered error: %s", output);
        tests_passed++;
    } else {
        printf("✗ Multiple invalid flags test failed. Output: %s\n", output);
    }
    
    /* Test 8: Valid flag combined with invalid flag */
    printf("\nTest 8: Valid flag with invalid flag (-l -q)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -q %s", gcda_path);
    output = execute_command(cmd, &exit_status);
    tests_total++;
    if (strstr(output, "unknown flag") != NULL && exit_status != 0) {
        printf("✓ Valid+invalid flag combination triggered error: %s", output);
        tests_passed++;
    } else {
        printf("✗ Valid+invalid flag test failed. Output: %s\n", output);
    }
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    snprintf(cmd, sizeof(cmd), "rm -rf %s", tmpdir);
    execute_command(cmd, &exit_status);
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d tests\n", tests_passed, tests_total);
    
    return (tests_passed == tests_total) ? 0 : 1;
}
