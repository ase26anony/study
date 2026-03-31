#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024
#define MAX_CMD 2048

/* Simple helper program that will generate GCOV data */
static const char helper_c[] = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Helper program executed\\n\");\n"
"    return 0;\n"
"}\n";

/* Execute a command and capture its output */
int execute_and_capture(const char *cmd, char *output, size_t output_size) {
    FILE *fp;
    int ret = 0;
    
    if (output_size > 0) output[0] = '\0';
    
    /* Use popen to capture both stdout and stderr */
    char cmd_with_stderr[MAX_CMD];
    snprintf(cmd_with_stderr, sizeof(cmd_with_stderr), "%s 2>&1", cmd);
    
    fp = popen(cmd_with_stderr, "r");
    if (fp == NULL) {
        return -1;
    }
    
    if (output && output_size > 0) {
        size_t total_read = 0;
        while (fgets(output + total_read, output_size - total_read, fp) != NULL) {
            total_read = strlen(output);
            if (total_read >= output_size - 1) break;
        }
    }
    
    ret = pclose(fp);
    return WEXITSTATUS(ret);
}

/* Check if output contains expected error message */
int contains_unknown_flag(const char *output) {
    return strstr(output, "unknown flag") != NULL;
}

int main(int argc, char *argv[]) {
    char tmpdir[MAX_PATH];
    char helper_c_path[MAX_PATH];
    char helper_exe_path[MAX_PATH];
    char gcda_path[MAX_PATH];
    char cmd[MAX_CMD];
    char output[4096];
    int ret;
    int tests_passed = 0;
    int tests_total = 0;
    
    /* Create a temporary directory for test files */
    snprintf(tmpdir, sizeof(tmpdir), "/tmp/gcov_dump_test_%d", getpid());
    mkdir(tmpdir, 0755);
    
    /* Create helper source file */
    snprintf(helper_c_path, sizeof(helper_c_path), "%s/helper.c", tmpdir);
    FILE *fp = fopen(helper_c_path, "w");
    if (!fp) {
        perror("Failed to create helper.c");
        return 1;
    }
    fwrite(helper_c, 1, strlen(helper_c), fp);
    fclose(fp);
    
    /* Compile helper with coverage instrumentation */
    snprintf(helper_exe_path, sizeof(helper_exe_path), "%s/helper", tmpdir);
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -o %s %s", 
             helper_exe_path, helper_c_path);
    
    ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Failed to compile helper program\n");
        return 1;
    }
    
    /* Run helper to generate .gcda file */
    ret = system(helper_exe_path);
    if (ret != 0) {
        fprintf(stderr, "Failed to run helper program\n");
        return 1;
    }
    
    /* Find the .gcda file path */
    snprintf(gcda_path, sizeof(gcda_path), "%s/helper.gcda", tmpdir);
    
    /* Test 1: Valid flag - should work */
    tests_total++;
    printf("Test 1: Valid flag (-l)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_path);
    ret = execute_and_capture(cmd, output, sizeof(output));
    if (ret == 0) {
        printf("  PASS: gcov-dump with valid flag executed successfully\n");
        tests_passed++;
    } else {
        printf("  FAIL: gcov-dump with valid flag failed (exit code: %d)\n", ret);
        printf("  Output: %s\n", output);
    }
    
    /* Test 2: Invalid flag 'a' - should trigger default case */
    tests_total++;
    printf("\nTest 2: Invalid flag (-a)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -a %s", gcda_path);
    ret = execute_and_capture(cmd, output, sizeof(output));
    if (ret != 0 && contains_unknown_flag(output)) {
        printf("  PASS: Got expected 'unknown flag' error\n");
        printf("  Output: %s", output);
        tests_passed++;
    } else {
        printf("  FAIL: Did not get expected error (exit code: %d)\n", ret);
        printf("  Output: %s\n", output);
    }
    
    /* Test 3: Invalid flag 'z' - another invalid character */
    tests_total++;
    printf("\nTest 3: Invalid flag (-z)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -z %s", gcda_path);
    ret = execute_and_capture(cmd, output, sizeof(output));
    if (ret != 0 && contains_unknown_flag(output)) {
        printf("  PASS: Got expected 'unknown flag' error\n");
        printf("  Output: %s", output);
        tests_passed++;
    } else {
        printf("  FAIL: Did not get expected error (exit code: %d)\n", ret);
        printf("  Output: %s\n", output);
    }
    
    /* Test 4: Invalid flag '1' - non-alphabetic character */
    tests_total++;
    printf("\nTest 4: Invalid flag (-1)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -1 %s", gcda_path);
    ret = execute_and_capture(cmd, output, sizeof(output));
    if (ret != 0 && contains_unknown_flag(output)) {
        printf("  PASS: Got expected 'unknown flag' error\n");
        printf("  Output: %s", output);
        tests_passed++;
    } else {
        printf("  FAIL: Did not get expected error (exit code: %d)\n", ret);
        printf("  Output: %s\n", output);
    }
    
    /* Test 5: Invalid flag '?' - special character */
    tests_total++;
    printf("\nTest 5: Invalid flag (-?)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -? %s", gcda_path);
    ret = execute_and_capture(cmd, output, sizeof(output));
    if (ret != 0 && contains_unknown_flag(output)) {
        printf("  PASS: Got expected 'unknown flag' error\n");
        printf("  Output: %s", output);
        tests_passed++;
    } else {
        printf("  FAIL: Did not get expected error (exit code: %d)\n", ret);
        printf("  Output: %s\n", output);
    }
    
    /* Test 6: Multiple invalid flags in sequence */
    tests_total++;
    printf("\nTest 6: Multiple invalid flags (-x -y)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -x -y %s", gcda_path);
    ret = execute_and_capture(cmd, output, sizeof(output));
    if (ret != 0 && contains_unknown_flag(output)) {
        printf("  PASS: Got expected 'unknown flag' error\n");
        printf("  Output: %s", output);
        tests_passed++;
    } else {
        printf("  FAIL: Did not get expected error (exit code: %d)\n", ret);
        printf("  Output: %s\n", output);
    }
    
    /* Test 7: Valid flag combined with invalid flag */
    tests_total++;
    printf("\nTest 7: Valid flag with invalid flag (-l -q)\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -q %s", gcda_path);
    ret = execute_and_capture(cmd, output, sizeof(output));
    if (ret != 0 && contains_unknown_flag(output)) {
        printf("  PASS: Got expected 'unknown flag' error\n");
        printf("  Output: %s", output);
        tests_passed++;
    } else {
        printf("  FAIL: Did not get expected error (exit code: %d)\n", ret);
        printf("  Output: %s\n", output);
    }
    
    /* Cleanup */
    snprintf(cmd, sizeof(cmd), "rm -rf %s", tmpdir);
    system(cmd);
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d tests\n", tests_passed, tests_total);
    
    return (tests_passed == tests_total) ? 0 : 1;
}
