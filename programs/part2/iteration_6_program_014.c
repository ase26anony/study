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

/* Simple C program to generate GCOV data */
const char *helper_source = 
    "#include <stdio.h>\n"
    "int main() {\n"
    "    printf(\"Generating GCOV data...\\n\");\n"
    "    return 0;\n"
    "}\n";

/* Execute a shell command and capture stderr */
int execute_and_capture(const char *cmd, char *output, size_t output_size) {
    char buffer[1024];
    FILE *fp;
    int found = 0;
    
    /* Clear output buffer */
    if (output && output_size > 0) {
        output[0] = '\0';
    }
    
    /* Execute command and capture both stdout and stderr */
    char full_cmd[1024];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    fp = popen(full_cmd, "r");
    if (fp == NULL) {
        return -1;
    }
    
    /* Read output */
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (output && (strlen(output) + strlen(buffer) < output_size)) {
            strcat(output, buffer);
        }
        
        /* Check for "unknown flag" message */
        if (strstr(buffer, "unknown flag") != NULL) {
            found = 1;
        }
    }
    
    int status = pclose(fp);
    return found ? 1 : (WEXITSTATUS(status) == 0 ? 0 : -1);
}

/* Create temporary directory */
int create_temp_dir() {
    struct stat st = {0};
    
    if (stat(TEMP_DIR, &st) == -1) {
        if (mkdir(TEMP_DIR, 0700) != 0) {
            perror("mkdir");
            return 0;
        }
    }
    return 1;
}

/* Generate GCOV data file */
int generate_gcov_data() {
    FILE *fp;
    
    /* Write helper source file */
    fp = fopen(HELPER_SRC, "w");
    if (!fp) {
        perror("fopen helper.c");
        return 0;
    }
    fprintf(fp, "%s", helper_source);
    fclose(fp);
    
    /* Compile with coverage flags */
    char compile_cmd[1024];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -o %s %s",
             HELPER_BIN, HELPER_SRC);
    
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile helper program\n");
        return 0;
    }
    
    /* Run helper to generate .gcda file */
    if (system(HELPER_BIN) != 0) {
        fprintf(stderr, "Failed to run helper program\n");
        return 0;
    }
    
    /* Verify .gcda file was created */
    struct stat st;
    if (stat(GCOV_DATA, &st) != 0) {
        fprintf(stderr, "GCOV data file not created: %s\n", GCOV_DATA);
        return 0;
    }
    
    return 1;
}

/* Clean up temporary files */
void cleanup() {
    char cmd[1024];
    
    /* Remove generated files */
    snprintf(cmd, sizeof(cmd), "rm -f %s %s %s %s.gcno %s",
             HELPER_SRC, HELPER_BIN, GCOV_DATA, HELPER_BIN, TEMP_DIR "/*.gcov");
    system(cmd);
    
    /* Remove directory if empty */
    rmdir(TEMP_DIR);
}

int main(int argc, char *argv[]) {
    char output[4096];
    int tests_passed = 0;
    int total_tests = 0;
    
    printf("=== Testing gcov-dump unknown flag handling ===\n");
    
    /* Create temporary directory */
    if (!create_temp_dir()) {
        return 1;
    }
    
    /* Generate GCOV data */
    printf("1. Generating GCOV data...\n");
    if (!generate_gcov_data()) {
        cleanup();
        return 1;
    }
    
    /* Test 1: Valid flag (should work) */
    printf("\n2. Testing valid flag (-l)...\n");
    char valid_cmd[1024];
    snprintf(valid_cmd, sizeof(valid_cmd), "gcov-dump -l %s", GCOV_DATA);
    
    if (execute_and_capture(valid_cmd, output, sizeof(output)) >= 0) {
        printf("   ✓ Valid flag test passed\n");
        tests_passed++;
    } else {
        printf("   ✗ Valid flag test failed\n");
    }
    total_tests++;
    
    /* Test 2-7: Various invalid single-character flags */
    printf("\n3. Testing invalid single-character flags...\n");
    
    /* Array of invalid flags to test */
    char invalid_flags[] = {'a', 'z', 'x', '?', '1', 'A', '\0'};
    
    for (int i = 0; invalid_flags[i] != '\0'; i++) {
        char invalid_cmd[1024];
        snprintf(invalid_cmd, sizeof(invalid_cmd), 
                 "gcov-dump -%c %s", invalid_flags[i], GCOV_DATA);
        
        printf("   Testing flag -%c: ", invalid_flags[i]);
        
        if (execute_and_capture(invalid_cmd, output, sizeof(output)) == 1) {
            printf("✓ (got 'unknown flag' error)\n");
            tests_passed++;
        } else {
            printf("✗ (missing expected error)\n");
            printf("     Output: %s\n", output);
        }
        total_tests++;
    }
    
    /* Test 8: Multiple invalid flags in sequence */
    printf("\n4. Testing multiple invalid flags...\n");
    char multi_invalid_cmd[1024];
    snprintf(multi_invalid_cmd, sizeof(multi_invalid_cmd),
             "gcov-dump -a -b -c %s", GCOV_DATA);
    
    printf("   Testing -a -b -c: ");
    if (execute_and_capture(multi_invalid_cmd, output, sizeof(output)) == 1) {
        printf("✓ (got 'unknown flag' error)\n");
        tests_passed++;
    } else {
        printf("✗\n");
    }
    total_tests++;
    
    /* Test 9: Invalid flag with valid flag */
    printf("\n5. Testing invalid flag with valid flag...\n");
    char mixed_cmd[1024];
    snprintf(mixed_cmd, sizeof(mixed_cmd),
             "gcov-dump -l -x %s", GCOV_DATA);
    
    printf("   Testing -l -x: ");
    if (execute_and_capture(mixed_cmd, output, sizeof(output)) == 1) {
        printf("✓ (got 'unknown flag' error)\n");
        tests_passed++;
    } else {
        printf("✗\n");
    }
    total_tests++;
    
    /* Test 10: Just the invalid flag without data file */
    printf("\n6. Testing invalid flag without data file...\n");
    printf("   Testing -x (no file): ");
    
    if (execute_and_capture("gcov-dump -x", output, sizeof(output)) == 1) {
        printf("✓ (got 'unknown flag' error)\n");
        tests_passed++;
    } else {
        printf("✗\n");
    }
    total_tests++;
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    
    /* Cleanup */
    cleanup();
    
    return (tests_passed == total_tests) ? 0 : 1;
}
