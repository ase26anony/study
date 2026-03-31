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

/* Simple helper program that will generate .gcda file */
const char *helper_source = 
    "#include <stdio.h>\n"
    "int main() {\n"
    "    printf(\"Helper program executed\\n\");\n"
    "    return 0;\n"
    "}\n";

/* Execute a command and capture its stderr output */
int execute_and_check(const char *command, int expect_error) {
    char buffer[1024];
    int found_error = 0;
    FILE *fp;
    
    printf("Executing: %s\n", command);
    
    /* Execute command and capture both stdout and stderr */
    char cmd_with_stderr[2048];
    snprintf(cmd_with_stderr, sizeof(cmd_with_stderr), "%s 2>&1", command);
    
    fp = popen(cmd_with_stderr, "r");
    if (fp == NULL) {
        perror("popen failed");
        return -1;
    }
    
    /* Read output */
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("  Output: %s", buffer);
        if (strstr(buffer, "unknown flag") != NULL) {
            found_error = 1;
        }
    }
    
    int status = pclose(fp);
    int exit_code = WEXITSTATUS(status);
    
    printf("  Exit code: %d\n", exit_code);
    
    if (expect_error) {
        if (!found_error) {
            printf("  ERROR: Expected 'unknown flag' message not found!\n");
            return -1;
        }
        if (exit_code == 0) {
            printf("  ERROR: Expected non-zero exit code!\n");
            return -1;
        }
    } else {
        if (found_error) {
            printf("  ERROR: Unexpected 'unknown flag' message found!\n");
            return -1;
        }
    }
    
    return 0;
}

int main() {
    int ret = 0;
    char command[1024];
    
    /* Create temporary directory */
    mkdir(TEMP_DIR, 0755);
    
    /* Write helper source file */
    FILE *fp = fopen(HELPER_SRC, "w");
    if (!fp) {
        perror("Failed to create helper source");
        return 1;
    }
    fputs(helper_source, fp);
    fclose(fp);
    
    /* Compile helper with coverage instrumentation */
    printf("Compiling helper program...\n");
    snprintf(command, sizeof(command),
             "gcc -fprofile-arcs -ftest-coverage -o %s %s",
             HELPER_BIN, HELPER_SRC);
    
    if (system(command) != 0) {
        fprintf(stderr, "Failed to compile helper program\n");
        return 1;
    }
    
    /* Run helper to generate .gcda file */
    printf("Running helper program to generate .gcda file...\n");
    if (system(HELPER_BIN) != 0) {
        fprintf(stderr, "Failed to run helper program\n");
        return 1;
    }
    
    /* Check if .gcda file was created */
    struct stat st;
    if (stat(GCOV_DATA, &st) != 0) {
        fprintf(stderr, "No .gcda file generated at %s\n", GCOV_DATA);
        return 1;
    }
    
    printf("\n=== Testing gcov-dump with various flags ===\n\n");
    
    /* Test 1: Valid flag (should work) */
    printf("Test 1: Valid flag (-l)\n");
    snprintf(command, sizeof(command), "gcov-dump -l %s", GCOV_DATA);
    if (execute_and_check(command, 0) != 0) {
        printf("Test 1 FAILED\n");
        ret = 1;
    } else {
        printf("Test 1 PASSED\n");
    }
    printf("\n");
    
    /* Test 2: Invalid alphabetic flag */
    printf("Test 2: Invalid flag (-a)\n");
    snprintf(command, sizeof(command), "gcov-dump -a %s", GCOV_DATA);
    if (execute_and_check(command, 1) != 0) {
        printf("Test 2 FAILED\n");
        ret = 1;
    } else {
        printf("Test 2 PASSED\n");
    }
    printf("\n");
    
    /* Test 3: Another invalid alphabetic flag */
    printf("Test 3: Invalid flag (-z)\n");
    snprintf(command, sizeof(command), "gcov-dump -z %s", GCOV_DATA);
    if (execute_and_check(command, 1) != 0) {
        printf("Test 3 FAILED\n");
        ret = 1;
    } else {
        printf("Test 3 PASSED\n");
    }
    printf("\n");
    
    /* Test 4: Invalid numeric flag */
    printf("Test 4: Invalid flag (-1)\n");
    snprintf(command, sizeof(command), "gcov-dump -1 %s", GCOV_DATA);
    if (execute_and_check(command, 1) != 0) {
        printf("Test 4 FAILED\n");
        ret = 1;
    } else {
        printf("Test 4 PASSED\n");
    }
    printf("\n");
    
    /* Test 5: Invalid special character flag */
    printf("Test 5: Invalid flag (-?)\n");
    snprintf(command, sizeof(command), "gcov-dump -? %s", GCOV_DATA);
    if (execute_and_check(command, 1) != 0) {
        printf("Test 5 FAILED\n");
        ret = 1;
    } else {
        printf("Test 5 PASSED\n");
    }
    printf("\n");
    
    /* Test 6: Multiple invalid flags in sequence */
    printf("Test 6: Multiple invalid flags (-x -y)\n");
    snprintf(command, sizeof(command), "gcov-dump -x -y %s", GCOV_DATA);
    if (execute_and_check(command, 1) != 0) {
        printf("Test 6 FAILED\n");
        ret = 1;
    } else {
        printf("Test 6 PASSED\n");
    }
    printf("\n");
    
    /* Test 7: Valid flag combined with invalid flag */
    printf("Test 7: Valid + invalid flag (-l -q)\n");
    snprintf(command, sizeof(command), "gcov-dump -l -q %s", GCOV_DATA);
    if (execute_and_check(command, 1) != 0) {
        printf("Test 7 FAILED\n");
        ret = 1;
    } else {
        printf("Test 7 PASSED\n");
    }
    printf("\n");
    
    /* Test 8: Just dash without character (edge case) */
    printf("Test 8: Just dash (-)\n");
    snprintf(command, sizeof(command), "gcov-dump - %s", GCOV_DATA);
    if (execute_and_check(command, 1) != 0) {
        printf("Test 8 FAILED\n");
        ret = 1;
    } else {
        printf("Test 8 PASSED\n");
    }
    
    /* Cleanup */
    printf("\nCleaning up...\n");
    unlink(HELPER_SRC);
    unlink(HELPER_BIN);
    unlink(GCOV_DATA);
    unlink(TEMP_DIR "/helper.gcno");  /* Also remove .gcno file */
    rmdir(TEMP_DIR);
    
    if (ret == 0) {
        printf("\n=== All tests passed! ===\n");
    } else {
        printf("\n=== Some tests failed! ===\n");
    }
    
    return ret;
}
