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
    
    // Execute command and capture stderr (2>&1 redirects stderr to stdout)
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
void create_helper_program(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create helper program");
        exit(1);
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    printf(\"Helper program executed\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
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
    
    // Step 1: Generate GCOV data
    printf("=== Step 1: Generating GCOV data ===\n");
    
    // Create helper source file
    create_helper_program(helper_c);
    
    // Compile with coverage instrumentation
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
    
    // Verify .gcda file was created
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "Failed to create .gcda file\n");
        // Try alternative location (gcda might be in current directory)
        snprintf(gcda_file, sizeof(gcda_file), "helper.gcda");
    }
    
    printf("GCOV data file: %s\n\n", gcda_file);
    
    // Step 2: Test gcov-dump with various flags
    printf("=== Step 2: Testing gcov-dump with various flags ===\n\n");
    
    // Test 1: Valid flag (to ensure tool works correctly)
    printf("Test 1: Valid flag (-l)\n");
    snprintf(command, sizeof(command), "gcov-dump -l %s", gcda_file);
    execute_and_check(command, NULL);
    
    // Test 2-7: Invalid single-character flags
    // These should trigger the default case in the switch statement
    char invalid_flags[] = "a?x9z!";
    int tests_passed = 0;
    int total_invalid_tests = 0;
    
    for (int i = 0; i < strlen(invalid_flags); i++) {
        printf("Test %d: Invalid flag (-%c)\n", i + 2, invalid_flags[i]);
        snprintf(command, sizeof(command), "gcov-dump -%c %s", 
                 invalid_flags[i], gcda_file);
        
        if (execute_and_check(command, "unknown flag")) {
            tests_passed++;
        }
        total_invalid_tests++;
    }
    
    // Test with multiple invalid flags in one call
    printf("Test 8: Multiple invalid flags (-ab)\n");
    snprintf(command, sizeof(command), "gcov-dump -ab %s", gcda_file);
    execute_and_check(command, "unknown flag");
    
    // Test with dash only (no character)
    printf("Test 9: Dash only\n");
    snprintf(command, sizeof(command), "gcov-dump - %s", gcda_file);
    execute_and_check(command, NULL);  // This might trigger different error
    
    // Test with uppercase invalid flag (case sensitivity test)
    printf("Test 10: Uppercase invalid flag (-A)\n");
    snprintf(command, sizeof(command), "gcov-dump -A %s", gcda_file);
    if (execute_and_check(command, "unknown flag")) {
        tests_passed++;
    }
    total_invalid_tests++;
    
    // Summary
    printf("=== Test Summary ===\n");
    printf("Invalid flag tests passed: %d/%d\n", tests_passed, total_invalid_tests);
    
    if (tests_passed > 0) {
        printf("SUCCESS: Successfully triggered the default case for unknown flags!\n");
    } else {
        printf("FAILURE: Did not trigger the default case\n");
    }
    
    // Cleanup
    printf("\n=== Cleaning up ===\n");
    snprintf(command, sizeof(command), "rm -rf %s", TEMP_DIR);
    system(command);
    system("rm -f helper.gcda helper.gcno helper");
    
    return (tests_passed > 0) ? 0 : 1;
}
