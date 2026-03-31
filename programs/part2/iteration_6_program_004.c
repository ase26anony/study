#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_PATH 1024
#define MAX_CMD 2048

/* Simple helper program that will generate .gcda file */
static const char helper_source[] = 
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
    if (mkdir(tmpdir, 0755) != 0 && errno != EEXIST) {
        perror("mkdir");
        return 1;
    }
    
    /* Write helper source file */
    snprintf(helper_c_path, sizeof(helper_c_path), "%s/helper.c", tmpdir);
    FILE *fp = fopen(helper_c_path, "w");
    if (!fp) {
        perror("fopen helper.c");
        return 1;
    }
    fwrite(helper_source, 1, sizeof(helper_source) - 1, fp);
    fclose(fp);
    
    /* Compile helper with coverage instrumentation */
    snprintf(helper_exe_path, sizeof(helper_exe_path), "%s/helper", tmpdir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -o %s %s 2>&1",
             helper_exe_path, helper_c_path);
    
    printf("Compiling helper program...\n");
    ret = system(cmd);
    if (ret != 0) {
        printf("Failed to compile helper program\n");
        /* Clean up and exit */
        unlink(helper_c_path);
        rmdir(tmpdir);
        return 1;
    }
    
    /* Run helper to generate .gcda file */
    printf("Running helper to generate .gcda file...\n");
    ret = system(helper_exe_path);
    if (ret != 0) {
        printf("Helper program execution failed\n");
    }
    
    /* Construct path to .gcda file */
    snprintf(gcda_path, sizeof(gcda_path), "%s/helper.gcda", tmpdir);
    
    /* Test 1: Valid flag (should work) */
    tests_total++;
    printf("\n=== Test 1: Valid flag (-l) ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s", gcda_path);
    printf("Executing: %s\n", cmd);
    
    ret = execute_and_capture(cmd, output, sizeof(output));
    printf("Exit code: %d\n", ret);
    
    if (ret == 0) {
        printf("✓ Valid flag test passed\n");
        tests_passed++;
    } else {
        printf("✗ Valid flag test failed\n");
    }
    
    /* Test 2-7: Invalid single-character flags */
    char invalid_flags[] = "a?x9!z";
    for (int i = 0; i < strlen(invalid_flags); i++) {
        tests_total++;
        printf("\n=== Test %d: Invalid flag (-%c) ===\n", 
               i + 2, invalid_flags[i]);
        
        snprintf(cmd, sizeof(cmd), "gcov-dump -%c %s", 
                 invalid_flags[i], gcda_path);
        printf("Executing: %s\n", cmd);
        
        ret = execute_and_capture(cmd, output, sizeof(output));
        printf("Exit code: %d\n", ret);
        printf("Output:\n%s\n", output);
        
        if (contains_unknown_flag(output) && ret != 0) {
            printf("✓ Invalid flag '-%c' triggered expected error\n", 
                   invalid_flags[i]);
            tests_passed++;
        } else {
            printf("✗ Invalid flag '-%c' did not trigger expected error\n", 
                   invalid_flags[i]);
        }
    }
    
    /* Test 8: Multiple invalid flags in sequence */
    tests_total++;
    printf("\n=== Test 8: Multiple invalid flags ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -a -b -c %s", gcda_path);
    printf("Executing: %s\n", cmd);
    
    ret = execute_and_capture(cmd, output, sizeof(output));
    printf("Exit code: %d\n", ret);
    printf("Output:\n%s\n", output);
    
    if (contains_unknown_flag(output) && ret != 0) {
        printf("✓ Multiple invalid flags triggered expected error\n");
        tests_passed++;
    } else {
        printf("✗ Multiple invalid flags did not trigger expected error\n");
    }
    
    /* Test 9: Mix valid and invalid flag */
    tests_total++;
    printf("\n=== Test 9: Mix valid and invalid flag ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -z %s", gcda_path);
    printf("Executing: %s\n", cmd);
    
    ret = execute_and_capture(cmd, output, sizeof(output));
    printf("Exit code: %d\n", ret);
    printf("Output:\n%s\n", output);
    
    if (contains_unknown_flag(output) && ret != 0) {
        printf("✓ Mixed flags triggered expected error\n");
        tests_passed++;
    } else {
        printf("✗ Mixed flags did not trigger expected error\n");
    }
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    unlink(helper_c_path);
    unlink(helper_exe_path);
    unlink(gcda_path);
    
    /* Also remove .gcno file if it exists */
    char gcov_files[MAX_PATH];
    snprintf(gcov_files, sizeof(gcov_files), "%s/*.gcno", tmpdir);
    char rm_cmd[MAX_CMD];
    snprintf(rm_cmd, sizeof(rm_cmd), "rm -f %s", gcov_files);
    system(rm_cmd);
    
    rmdir(tmpdir);
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, tests_total);
    
    if (tests_passed == tests_total) {
        printf("✓ All tests passed!\n");
        return 0;
    } else {
        printf("✗ Some tests failed\n");
        return 1;
    }
}
