#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_PATH 1024
#define TEMP_DIR "/tmp/gcov_dump_test"

// Helper function to execute a command and capture output
int execute_and_check(const char *command, const char *expected_error) {
    char buffer[1024];
    int found_error = 0;
    
    printf("Executing: %s\n", command);
    
    // Use popen to capture both stdout and stderr
    char full_command[MAX_PATH * 2];
    snprintf(full_command, sizeof(full_command), "%s 2>&1", command);
    
    FILE *fp = popen(full_command, "r");
    if (fp == NULL) {
        perror("popen failed");
        return 0;
    }
    
    // Read output
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("  Output: %s", buffer);
        if (expected_error && strstr(buffer, expected_error)) {
            found_error = 1;
        }
    }
    
    int status = pclose(fp);
    printf("  Exit status: %d\n\n", WEXITSTATUS(status));
    
    return found_error;
}

// Create a simple C program for generating GCOV data
void create_helper_source(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create helper source");
        exit(1);
    }
    
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main() {\n");
    fprintf(f, "    printf(\"Helper program running\\n\");\n");
    fprintf(f, "    int x = 5;\n");
    fprintf(f, "    if (x > 0) {\n");
    fprintf(f, "        printf(\"x is positive\\n\");\n");
    fprintf(f, "    }\n");
    fprintf(f, "    for (int i = 0; i < 3; i++) {\n");
    fprintf(f, "        printf(\"Loop iteration %%d\\n\", i);\n");
    fprintf(f, "    }\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
}

int main() {
    char helper_c[MAX_PATH];
    char helper_exe[MAX_PATH];
    char gcda_file[MAX_PATH];
    char command[MAX_PATH * 3];
    
    // Create temporary directory
    mkdir(TEMP_DIR, 0755);
    
    // Set up file paths
    snprintf(helper_c, sizeof(helper_c), "%s/helper.c", TEMP_DIR);
    snprintf(helper_exe, sizeof(helper_exe), "%s/helper", TEMP_DIR);
    snprintf(gcda_file, sizeof(gcda_file), "%s/helper.gcda", TEMP_DIR);
    
    printf("=== Generating GCOV test data ===\n");
    
    // Create helper source
    create_helper_source(helper_c);
    
    // Compile helper with coverage instrumentation
    snprintf(command, sizeof(command), 
             "gcc -fprofile-arcs -ftest-coverage -o %s %s", 
             helper_exe, helper_c);
    
    if (system(command) != 0) {
        fprintf(stderr, "Failed to compile helper program\n");
        return 1;
    }
    
    // Run helper to generate .gcda file
    if (system(helper_exe) != 0) {
        fprintf(stderr, "Failed to run helper program\n");
        return 1;
    }
    
    printf("\n=== Testing gcov-dump with various flags ===\n\n");
    
    // Test 1: Valid flag (should work)
    printf("Test 1: Valid flag (-l)\n");
    snprintf(command, sizeof(command), "gcov-dump -l %s", gcda_file);
    execute_and_check(command, NULL);
    
    // Test 2-7: Invalid single-character flags
    char invalid_flags[] = {'a', 'z', 'x', '?', '1', '9'};
    int tests_passed = 0;
    
    for (int i = 0; i < sizeof(invalid_flags); i++) {
        printf("Test %d: Invalid flag (-%c)\n", i + 2, invalid_flags[i]);
        snprintf(command, sizeof(command), "gcov-dump -%c %s", 
                 invalid_flags[i], gcda_file);
        
        if (execute_and_check(command, "unknown flag")) {
            tests_passed++;
        }
    }
    
    // Test 8: Multiple invalid flags in sequence
    printf("Test 8: Multiple invalid flags (-ab)\n");
    snprintf(command, sizeof(command), "gcov-dump -ab %s", gcda_file);
    execute_and_check(command, NULL);  // This might trigger different error
    
    // Test 9: Valid flag followed by invalid flag
    printf("Test 9: Mixed valid and invalid (-l -x)\n");
    snprintf(command, sizeof(command), "gcov-dump -l -x %s", gcda_file);
    if (execute_and_check(command, "unknown flag")) {
        tests_passed++;
    }
    
    // Test 10: Just a dash (edge case)
    printf("Test 10: Just a dash (-)\n");
    snprintf(command, sizeof(command), "gcov-dump - %s", gcda_file);
    execute_and_check(command, NULL);
    
    printf("\n=== Summary ===\n");
    printf("Invalid flag tests passed: %d/%d\n", tests_passed, 
           (int)sizeof(invalid_flags) + 1);  // +1 for test 9
    
    // Cleanup
    printf("\nCleaning up temporary files...\n");
    snprintf(command, sizeof(command), "rm -rf %s", TEMP_DIR);
    system(command);
    
    if (tests_passed >= sizeof(invalid_flags)) {
        printf("\nSUCCESS: All invalid flag tests triggered the uncovered code!\n");
        return 0;
    } else {
        printf("\nWARNING: Some tests didn't trigger the expected error\n");
        return 1;
    }
}
