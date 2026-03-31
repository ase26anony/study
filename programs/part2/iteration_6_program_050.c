#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define TEMP_DIR "/tmp/gcov_test_XXXXXX"
#define HELPER_SRC "helper.c"
#define HELPER_BIN "helper"
#define GCOV_DATA "helper.gcda"

void cleanup(const char *temp_dir, const char *helper_src, 
             const char *helper_bin, const char *gcov_data) {
    char path[512];
    
    if (helper_src) {
        snprintf(path, sizeof(path), "%s/%s", temp_dir, helper_src);
        unlink(path);
    }
    if (helper_bin) {
        snprintf(path, sizeof(path), "%s/%s", temp_dir, helper_bin);
        unlink(path);
    }
    if (gcov_data) {
        snprintf(path, sizeof(path), "%s/%s", temp_dir, gcov_data);
        unlink(path);
    }
    if (temp_dir) {
        rmdir(temp_dir);
    }
}

int execute_and_check(const char *command, int expect_error) {
    char buffer[1024];
    int found_error = 0;
    FILE *fp;
    
    printf("Executing: %s\n", command);
    
    fp = popen(command, "r");
    if (!fp) {
        perror("popen failed");
        return 0;
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("Output: %s", buffer);
        if (strstr(buffer, "unknown flag")) {
            found_error = 1;
        }
    }
    
    int status = pclose(fp);
    int exit_code = WEXITSTATUS(status);
    
    if (expect_error) {
        if (!found_error) {
            printf("ERROR: Expected 'unknown flag' message not found!\n");
            return 0;
        }
        if (exit_code == 0) {
            printf("ERROR: Expected non-zero exit code for invalid flag!\n");
            return 0;
        }
        printf("✓ Correctly detected invalid flag (exit code: %d)\n", exit_code);
    } else {
        if (exit_code != 0) {
            printf("ERROR: Valid command failed with exit code %d\n", exit_code);
            return 0;
        }
        printf("✓ Valid command executed successfully\n");
    }
    
    return 1;
}

int main() {
    char temp_dir[64];
    char helper_src_path[512];
    char helper_bin_path[512];
    char gcda_path[512];
    char command[1024];
    int success = 1;
    
    // Create temporary directory
    strcpy(temp_dir, TEMP_DIR);
    if (!mkdtemp(temp_dir)) {
        perror("Failed to create temp directory");
        return 1;
    }
    
    printf("Created temp directory: %s\n", temp_dir);
    
    // Create helper source file
    snprintf(helper_src_path, sizeof(helper_src_path), "%s/%s", temp_dir, HELPER_SRC);
    FILE *src = fopen(helper_src_path, "w");
    if (!src) {
        perror("Failed to create helper source");
        cleanup(temp_dir, NULL, NULL, NULL);
        return 1;
    }
    
    fprintf(src, "#include <stdio.h>\n");
    fprintf(src, "int main() {\n");
    fprintf(src, "    printf(\"Helper program executed\\n\");\n");
    fprintf(src, "    return 0;\n");
    fprintf(src, "}\n");
    fclose(src);
    
    // Compile helper with coverage
    snprintf(helper_bin_path, sizeof(helper_bin_path), "%s/%s", temp_dir, HELPER_BIN);
    snprintf(command, sizeof(command), 
             "gcc -fprofile-arcs -ftest-coverage -o %s %s 2>&1",
             helper_bin_path, helper_src_path);
    
    printf("Compiling helper program...\n");
    if (system(command) != 0) {
        printf("Failed to compile helper program\n");
        cleanup(temp_dir, HELPER_SRC, NULL, NULL);
        return 1;
    }
    
    // Run helper to generate gcda file
    printf("Running helper to generate GCOV data...\n");
    if (system(helper_bin_path) != 0) {
        printf("Failed to run helper program\n");
        cleanup(temp_dir, HELPER_SRC, HELPER_BIN, NULL);
        return 1;
    }
    
    // Construct path to gcda file
    snprintf(gcda_path, sizeof(gcda_path), "%s/%s", temp_dir, GCOV_DATA);
    
    // Test 1: Valid command (should succeed)
    snprintf(command, sizeof(command), "gcov-dump -l %s 2>&1", gcda_path);
    if (!execute_and_check(command, 0)) {
        success = 0;
    }
    
    printf("\n--- Testing invalid flags ---\n");
    
    // Test 2: Invalid alphabetic flag
    snprintf(command, sizeof(command), "gcov-dump -a %s 2>&1", gcda_path);
    if (!execute_and_check(command, 1)) {
        success = 0;
    }
    
    // Test 3: Another invalid alphabetic flag
    snprintf(command, sizeof(command), "gcov-dump -z %s 2>&1", gcda_path);
    if (!execute_and_check(command, 1)) {
        success = 0;
    }
    
    // Test 4: Invalid numeric flag
    snprintf(command, sizeof(command), "gcov-dump -1 %s 2>&1", gcda_path);
    if (!execute_and_check(command, 1)) {
        success = 0;
    }
    
    // Test 5: Invalid special character flag
    snprintf(command, sizeof(command), "gcov-dump -? %s 2>&1", gcda_path);
    if (!execute_and_check(command, 1)) {
        success = 0;
    }
    
    // Test 6: Invalid flag 'x'
    snprintf(command, sizeof(command), "gcov-dump -x %s 2>&1", gcda_path);
    if (!execute_and_check(command, 1)) {
        success = 0;
    }
    
    // Test 7: Multiple invalid flags (should trigger error on first invalid)
    snprintf(command, sizeof(command), "gcov-dump -a -l %s 2>&1", gcda_path);
    if (!execute_and_check(command, 1)) {
        success = 0;
    }
    
    // Test 8: Flag that's close to valid but not quite
    snprintf(command, sizeof(command), "gcov-dump -q %s 2>&1", gcda_path);
    if (!execute_and_check(command, 1)) {
        success = 0;
    }
    
    // Cleanup
    printf("\nCleaning up...\n");
    cleanup(temp_dir, HELPER_SRC, HELPER_BIN, GCOV_DATA);
    
    if (success) {
        printf("\n✅ All tests passed! The default case in switch statement was triggered.\n");
        return 0;
    } else {
        printf("\n❌ Some tests failed\n");
        return 1;
    }
}
