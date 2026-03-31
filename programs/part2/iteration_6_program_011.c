#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024
#define MAX_CMD 2048

/* Simple helper program that will generate .gcda file */
static const char helper_source[] = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Helper program running\\n\");\n"
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
    int total_tests = 0;
    
    /* Create temporary directory for test files */
    char tmpdir_template[] = "/tmp/gcov_dump_test_XXXXXX";
    char *tmpdir = mkdtemp(tmpdir_template);
    if (tmpdir == NULL) {
        perror("Failed to create temp directory");
        return 1;
    }
    
    /* Create paths */
    snprintf(helper_c_path, sizeof(helper_c_path), "%s/helper.c", tmpdir);
    snprintf(helper_exe_path, sizeof(helper_exe_path), "%s/helper", tmpdir);
    snprintf(gcda_path, sizeof(gcda_path), "%s/helper.gcda", tmpdir);
    
    /* Write helper source file */
    FILE *fp = fopen(helper_c_path, "w");
    if (fp == NULL) {
        perror("Failed to create helper.c");
        return 1;
    }
    fwrite(helper_source, 1, sizeof(helper_source) - 1, fp);
    fclose(fp);
    
    /* Compile helper with coverage instrumentation */
    printf("Compiling helper program...\n");
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -o %s %s",
             helper_exe_path, helper_c_path);
    ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Failed to compile helper program\n");
        return 1;
    }
    
    /* Run helper to generate .gcda file */
    printf("Running helper program to generate .gcda...\n");
    ret = system(helper_exe_path);
    if (ret != 0) {
        fprintf(stderr, "Helper program failed\n");
        return 1;
    }
    
    /* Verify .gcda file was created */
    struct stat st;
    if (stat(gcda_path, &st) != 0) {
        fprintf(stderr, "No .gcda file generated at %s\n", gcda_path);
        return 1;
    }
    
    printf("\n=== Testing gcov-dump with invalid flags ===\n\n");
    
    /* Test 1: Valid command first (to ensure tool works) */
    total_tests++;
    printf("Test %d: Valid flag (-l)\n", total_tests);
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_path);
    ret = execute_and_capture(cmd, output, sizeof(output));
    if (ret == 0) {
        printf("  ✓ Valid flag accepted\n");
        tests_passed++;
    } else {
        printf("  ✗ Valid flag failed (exit code: %d)\n", ret);
    }
    
    /* Test 2: Invalid flag 'a' */
    total_tests++;
    printf("\nTest %d: Invalid flag '-a'\n", total_tests);
    snprintf(cmd, sizeof(cmd), "gcov-dump -a %s", gcda_path);
    ret = execute_and_capture(cmd, output, sizeof(output));
    if (ret != 0 && contains_unknown_flag(output)) {
        printf("  ✓ Correctly rejected invalid flag '-a'\n");
        printf("    Output: %s", output);
        tests_passed++;
    } else {
        printf("  ✗ Failed to detect invalid flag '-a'\n");
        printf("    Exit code: %d, Output: %s\n", ret, output);
    }
    
    /* Test 3: Invalid flag 'z' */
    total_tests++;
    printf("\nTest %d: Invalid flag '-z'\n", total_tests);
    snprintf(cmd, sizeof(cmd), "gcov-dump -z %s", gcda_path);
    ret = execute_and_capture(cmd, output, sizeof(output));
    if (ret != 0 && contains_unknown_flag(output)) {
        printf("  ✓ Correctly rejected invalid flag '-z'\n");
        printf("    Output: %s", output);
        tests_passed++;
    } else {
        printf("  ✗ Failed to detect invalid flag '-z'\n");
        printf("    Exit code: %d, Output: %s\n", ret, output);
    }
    
    /* Test 4: Invalid flag 'x' */
    total_tests++;
    printf("\nTest %d: Invalid flag '-x'\n", total_tests);
    snprintf(cmd, sizeof(cmd), "gcov-dump -x %s", gcda_path);
    ret = execute_and_capture(cmd, output, sizeof(output));
    if (ret != 0 && contains_unknown_flag(output)) {
        printf("  ✓ Correctly rejected invalid flag '-x'\n");
        printf("    Output: %s", output);
        tests_passed++;
    } else {
        printf("  ✗ Failed to detect invalid flag '-x'\n");
        printf("    Exit code: %d, Output: %s\n", ret, output);
    }
    
    /* Test 5: Invalid flag '1' (numeric) */
    total_tests++;
    printf("\nTest %d: Invalid flag '-1' (numeric)\n", total_tests);
    snprintf(cmd, sizeof(cmd), "gcov-dump -1 %s", gcda_path);
    ret = execute_and_capture(cmd, output, sizeof(output));
    if (ret != 0 && contains_unknown_flag(output)) {
        printf("  ✓ Correctly rejected invalid flag '-1'\n");
        printf("    Output: %s", output);
        tests_passed++;
    } else {
        printf("  ✗ Failed to detect invalid flag '-1'\n");
        printf("    Exit code: %d, Output: %s\n", ret, output);
    }
    
    /* Test 6: Invalid flag '?' (special character) */
    total_tests++;
    printf("\nTest %d: Invalid flag '-?' (special character)\n", total_tests);
    snprintf(cmd, sizeof(cmd), "gcov-dump -? %s", gcda_path);
    ret = execute_and_capture(cmd, output, sizeof(output));
    if (ret != 0 && contains_unknown_flag(output)) {
        printf("  ✓ Correctly rejected invalid flag '-?'\n");
        printf("    Output: %s", output);
        tests_passed++;
    } else {
        printf("  ✗ Failed to detect invalid flag '-?'\n");
        printf("    Exit code: %d, Output: %s\n", ret, output);
    }
    
    /* Test 7: Multiple invalid flags in sequence */
    total_tests++;
    printf("\nTest %d: Multiple invalid flags test\n", total_tests);
    const char *invalid_flags[] = {"-b", "-c", "-d", "-e", "-f"};
    int multi_passed = 0;
    for (int i = 0; i < 5; i++) {
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s", invalid_flags[i], gcda_path);
        ret = execute_and_capture(cmd, output, sizeof(output));
        if (ret != 0 && contains_unknown_flag(output)) {
            multi_passed++;
        }
    }
    if (multi_passed == 5) {
        printf("  ✓ All 5 invalid flags correctly rejected\n");
        tests_passed++;
    } else {
        printf("  ✗ Only %d/5 invalid flags were rejected\n", multi_passed);
    }
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    snprintf(cmd, sizeof(cmd), "rm -rf %s", tmpdir);
    system(cmd);
    
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d tests\n", tests_passed, total_tests);
    
    return (tests_passed == total_tests) ? 0 : 1;
}
