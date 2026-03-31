#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_PATH 1024
#define MAX_CMD 2048

/* Simple helper program that will generate GCOV data */
static const char helper_source[] = 
"#include <stdio.h>\n"
"int main() {\n"
"    printf(\"Generating GCOV data...\\n\");\n"
"    return 0;\n"
"}\n";

/* Execute a command and capture its stderr output */
int execute_and_check(const char *cmd, const char *expected_error) {
    char buffer[1024];
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
    
    int status = pclose(fp);
    printf("Exit status: %d\n\n", WEXITSTATUS(status));
    
    return found;
}

int main(int argc, char *argv[]) {
    char helper_c_path[MAX_PATH];
    char helper_exe_path[MAX_PATH];
    char gcda_path[MAX_PATH];
    char cmd[MAX_CMD];
    FILE *fp;
    int i;
    
    /* Create temporary directory for test files */
    char tmpdir[] = "/tmp/gcov_test_XXXXXX";
    if (mkdtemp(tmpdir) == NULL) {
        perror("Failed to create temp directory");
        return 1;
    }
    
    printf("Created temp directory: %s\n", tmpdir);
    
    /* Create helper source file */
    snprintf(helper_c_path, sizeof(helper_c_path), "%s/helper.c", tmpdir);
    fp = fopen(helper_c_path, "w");
    if (!fp) {
        perror("Failed to create helper.c");
        return 1;
    }
    fwrite(helper_source, 1, strlen(helper_source), fp);
    fclose(fp);
    
    /* Compile helper with coverage instrumentation */
    snprintf(helper_exe_path, sizeof(helper_exe_path), "%s/helper", tmpdir);
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -o %s %s", 
             helper_exe_path, helper_c_path);
    
    printf("Compiling helper: %s\n", cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to compile helper program\n");
        return 1;
    }
    
    /* Run helper to generate .gcda file */
    printf("Running helper to generate GCOV data...\n");
    if (system(helper_exe_path) != 0) {
        fprintf(stderr, "Failed to run helper program\n");
        return 1;
    }
    
    /* Get path to .gcda file */
    snprintf(gcda_path, sizeof(gcda_path), "%s/helper.gcda", tmpdir);
    
    /* Test 1: Valid flag to ensure gcov-dump works */
    printf("\n=== Test 1: Valid flag (-l) ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s 2>&1", gcda_path);
    execute_and_check(cmd, NULL);
    
    /* Test 2-7: Invalid single-character flags to trigger default case */
    printf("\n=== Test 2-7: Invalid flags (should trigger 'unknown flag') ===\n");
    
    /* Test various invalid flags including edge cases */
    const char *invalid_flags[] = {"-a", "-z", "-x", "-?", "-1", "-@"};
    int tests_passed = 0;
    
    for (i = 0; i < sizeof(invalid_flags)/sizeof(invalid_flags[0]); i++) {
        printf("\n--- Testing invalid flag %s ---\n", invalid_flags[i]);
        snprintf(cmd, sizeof(cmd), "gcov-dump %s %s 2>&1", invalid_flags[i], gcda_path);
        
        if (execute_and_check(cmd, "unknown flag")) {
            tests_passed++;
            printf("✓ Correctly detected invalid flag %s\n", invalid_flags[i]);
        } else {
            printf("✗ Failed to detect invalid flag %s\n", invalid_flags[i]);
        }
    }
    
    /* Test 8: Multiple invalid flags in sequence */
    printf("\n=== Test 8: Multiple invalid flags ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -a -b -c %s 2>&1", gcda_path);
    if (execute_and_check(cmd, "unknown flag")) {
        tests_passed++;
        printf("✓ Correctly detected multiple invalid flags\n");
    }
    
    /* Test 9: Valid flag combined with invalid flag */
    printf("\n=== Test 9: Valid + invalid flag ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump -l -x %s 2>&1", gcda_path);
    if (execute_and_check(cmd, "unknown flag")) {
        tests_passed++;
        printf("✓ Correctly detected invalid flag among valid ones\n");
    }
    
    /* Test 10: Just the dash without character (should be treated as file) */
    printf("\n=== Test 10: Bare dash ===\n");
    snprintf(cmd, sizeof(cmd), "gcov-dump - %s 2>&1", gcda_path);
    execute_and_check(cmd, NULL);
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    snprintf(cmd, sizeof(cmd), "rm -rf %s", tmpdir);
    system(cmd);
    
    printf("\n=== Summary ===\n");
    printf("Invalid flag tests passed: %d/%d\n", tests_passed, 
           (int)(sizeof(invalid_flags)/sizeof(invalid_flags[0])) + 2);
    
    return (tests_passed > 0) ? 0 : 1;
}
