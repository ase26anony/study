#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define TEMP_DIR "/tmp/gcov_dump_test"
#define HELPER_SRC TEMP_DIR "/helper.c"
#define HELPER_BIN TEMP_DIR "/helper"
#define GCOV_DATA TEMP_DIR "/helper.gcda"

/* Simple helper program to generate GCOV data */
const char *helper_source = 
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
    char full_cmd[2048];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    fp = popen(full_cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return 0;
    }
    
    /* Read output */
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("  Output: %s", buffer);
        if (expected_error && strstr(buffer, expected_error)) {
            found = 1;
        }
    }
    
    int status = pclose(fp);
    int exit_code = WEXITSTATUS(status);
    
    printf("  Exit code: %d\n", exit_code);
    
    if (expected_error) {
        if (found) {
            printf("  ✓ Found expected error: '%s'\n", expected_error);
        } else {
            printf("  ✗ Did not find expected error: '%s'\n", expected_error);
        }
        return found && (exit_code != 0);
    }
    
    return exit_code == 0;
}

/* Create temporary directory and files */
void setup_test_environment() {
    struct stat st = {0};
    
    /* Create temp directory */
    if (stat(TEMP_DIR, &st) == -1) {
        mkdir(TEMP_DIR, 0700);
    }
    
    /* Write helper source file */
    FILE *fp = fopen(HELPER_SRC, "w");
    if (fp) {
        fputs(helper_source, fp);
        fclose(fp);
    }
    
    /* Compile helper with coverage instrumentation */
    char compile_cmd[512];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -o %s %s",
             HELPER_BIN, HELPER_SRC);
    
    printf("Compiling helper program...\n");
    system(compile_cmd);
    
    /* Run helper to generate GCOV data */
    printf("Running helper to generate GCOV data...\n");
    system(HELPER_BIN);
}

/* Clean up temporary files */
void cleanup_test_environment() {
    char cmd[512];
    
    printf("\nCleaning up...\n");
    
    /* Remove generated files */
    snprintf(cmd, sizeof(cmd), "rm -f %s %s %s %s.gcno %s.gcda",
             HELPER_SRC, HELPER_BIN, GCOV_DATA, HELPER_BIN, HELPER_BIN);
    system(cmd);
    
    /* Remove temp directory if empty */
    rmdir(TEMP_DIR);
}

int main() {
    int tests_passed = 0;
    int total_tests = 0;
    
    printf("=== GCOV-Dump Invalid Flag Test ===\n\n");
    
    /* Setup test environment */
    setup_test_environment();
    
    /* Test 1: Valid command to ensure gcov-dump works */
    printf("\n--- Test 1: Valid command ---\n");
    char valid_cmd[512];
    snprintf(valid_cmd, sizeof(valid_cmd), "gcov-dump -l %s", GCOV_DATA);
    
    if (execute_and_check(valid_cmd, NULL)) {
        printf("✓ Valid command executed successfully\n");
        tests_passed++;
    } else {
        printf("✗ Valid command failed\n");
    }
    total_tests++;
    
    /* Test 2-7: Invalid single-character flags */
    printf("\n--- Test 2-7: Invalid single-character flags ---\n");
    char invalid_flags[] = "a?x9Z$";  /* Various invalid characters */
    
    for (int i = 0; i < strlen(invalid_flags); i++) {
        char invalid_cmd[512];
        snprintf(invalid_cmd, sizeof(invalid_cmd), 
                 "gcov-dump -%c %s", invalid_flags[i], GCOV_DATA);
        
        if (execute_and_check(invalid_cmd, "unknown flag")) {
            tests_passed++;
        }
        total_tests++;
    }
    
    /* Test 8: Multiple invalid flags in one call */
    printf("\n--- Test 8: Multiple invalid flags ---\n");
    char multi_invalid_cmd[512];
    snprintf(multi_invalid_cmd, sizeof(multi_invalid_cmd),
             "gcov-dump -a -b -c %s", GCOV_DATA);
    
    if (execute_and_check(multi_invalid_cmd, "unknown flag")) {
        tests_passed++;
    }
    total_tests++;
    
    /* Test 9: Mix of valid and invalid flags */
    printf("\n--- Test 9: Mix of valid and invalid flags ---\n");
    char mixed_cmd[512];
    snprintf(mixed_cmd, sizeof(mixed_cmd),
             "gcov-dump -l -x -p %s", GCOV_DATA);
    
    if (execute_and_check(mixed_cmd, "unknown flag")) {
        tests_passed++;
    }
    total_tests++;
    
    /* Test 10: Just a dash (edge case) */
    printf("\n--- Test 10: Just a dash ---\n");
    char dash_only_cmd[512];
    snprintf(dash_only_cmd, sizeof(dash_only_cmd),
             "gcov-dump - %s", GCOV_DATA);
    
    if (execute_and_check(dash_only_cmd, "unknown flag")) {
        tests_passed++;
    }
    total_tests++;
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    
    /* Cleanup */
    cleanup_test_environment();
    
    return tests_passed == total_tests ? 0 : 1;
}
