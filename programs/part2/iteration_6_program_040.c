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
    char helper_c_path[MAX_PATH];
    char helper_exe_path[MAX_PATH];
    char gcda_path[MAX_PATH];
    char cmd[MAX_CMD];
    char output[4096];
    int ret;
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
        perror("Failed to create helper.c");
        return 1;
    }
    fputs(helper_source, fp);
    fclose(fp);
    
    /* Compile helper with coverage instrumentation */
    snprintf(helper_exe_path, sizeof(helper_exe_path), "%s/helper", tmpdir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -o %s %s 2>&1",
             helper_exe_path, helper_c_path);
    
    ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Failed to compile helper program\n");
        /* Clean up and exit */
        snprintf(cmd, sizeof(cmd), "rm -rf %s", tmpdir);
        system(cmd);
        return 1;
    }
    
    /* Run helper to generate .gcda file */
    ret = system(helper_exe_path);
    if (ret != 0) {
        fprintf(stderr, "Failed to run helper program\n");
        snprintf(cmd, sizeof(cmd), "rm -rf %s", tmpdir);
        system(cmd);
        return 1;
    }
    
    /* Construct path to .gcda file */
    snprintf(gcda_path, sizeof(gcda_path), "%s/helper.gcda", tmpdir);
    
    /* Test 1: Valid flag to ensure gcov-dump works */
    printf("Test 1: Valid flag (-l)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_path);
    ret = execute_and_capture(cmd, output, sizeof(output));
    tests_total++;
    if (ret == 0) {
        printf("  PASS: gcov-dump executed successfully with valid flag\n");
        tests_passed++;
    } else {
        printf("  FAIL: gcov-dump failed with valid flag (exit code: %d)\n", ret);
        printf("  Output: %s\n", output);
    }
    
    /* Test 2: Invalid flag -a */
    printf("\nTest 2: Invalid flag (-a)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -a %s", gcda_path);
    ret = execute_and_capture(cmd, output, sizeof(output));
    tests_total++;
    if (ret != 0 && contains_unknown_flag(output)) {
        printf("  PASS: Got expected 'unknown flag' error (exit code: %d)\n", ret);
        printf("  Output: %s\n", output);
        tests_passed++;
    } else {
        printf("  FAIL: Did not get expected error (exit code: %d)\n", ret);
        printf("  Output: %s\n", output);
    }
    
    /* Test 3: Invalid flag -z */
    printf("\nTest 3: Invalid flag (-z)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -z %s", gcda_path);
    ret = execute_and_capture(cmd, output, sizeof(output));
    tests_total++;
    if (ret != 0 && contains_unknown_flag(output)) {
        printf("  PASS: Got expected 'unknown flag' error (exit code: %d)\n", ret);
        printf("  Output: %s\n", output);
        tests_passed++;
    } else {
        printf("  FAIL: Did not get expected error (exit code: %d)\n", ret);
        printf("  Output: %s\n", output);
    }
    
    /* Test 4: Invalid flag -x */
    printf("\nTest 4: Invalid flag (-x)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -x %s", gcda_path);
    ret = execute_and_capture(cmd, output, sizeof(output));
    tests_total++;
    if (ret != 0 && contains_unknown_flag(output)) {
        printf("  PASS: Got expected 'unknown flag' error (exit code: %d)\n", ret);
        printf("  Output: %s\n", output);
        tests_passed++;
    } else {
        printf("  FAIL: Did not get expected error (exit code: %d)\n", ret);
        printf("  Output: %s\n", output);
    }
    
    /* Test 5: Invalid flag -1 (non-alphabetic) */
    printf("\nTest 5: Invalid flag (-1)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -1 %s", gcda_path);
    ret = execute_and_capture(cmd, output, sizeof(output));
    tests_total++;
    if (ret != 0 && contains_unknown_flag(output)) {
        printf("  PASS: Got expected 'unknown flag' error (exit code: %d)\n", ret);
        printf("  Output: %s\n", output);
        tests_passed++;
    } else {
        printf("  FAIL: Did not get expected error (exit code: %d)\n", ret);
        printf("  Output: %s\n", output);
    }
    
    /* Test 6: Invalid flag -? (special character) */
    printf("\nTest 6: Invalid flag (-?)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -\\? %s", gcda_path);
    ret = execute_and_capture(cmd, output, sizeof(output));
    tests_total++;
    if (ret != 0 && contains_unknown_flag(output)) {
        printf("  PASS: Got expected 'unknown flag' error (exit code: %d)\n", ret);
        printf("  Output: %s\n", output);
        tests_passed++;
    } else {
        printf("  FAIL: Did not get expected error (exit code: %d)\n", ret);
        printf("  Output: %s\n", output);
    }
    
    /* Test 7: Multiple invalid flags in one call */
    printf("\nTest 7: Multiple invalid flags (-ab)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -ab %s", gcda_path);
    ret = execute_and_capture(cmd, output, sizeof(output));
    tests_total++;
    if (ret != 0 && contains_unknown_flag(output)) {
        printf("  PASS: Got expected 'unknown flag' error (exit code: %d)\n", ret);
        printf("  Output: %s\n", output);
        tests_passed++;
    } else {
        printf("  FAIL: Did not get expected error (exit code: %d)\n", ret);
        printf("  Output: %s\n", output);
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d tests\n", tests_passed, tests_total);
    
    /* Cleanup */
    snprintf(cmd, sizeof(cmd), "rm -rf %s", tmpdir);
    system(cmd);
    
    return (tests_passed == tests_total) ? 0 : 1;
}
