/**
 * Test program to cover the default case in gcov-dump's option parsing.
 * This program:
 * 1. Creates a simple C program and compiles it with coverage instrumentation
 * 2. Runs it to generate .gcda files
 * 3. Calls gcov-dump with various invalid single-character flags
 * 4. Verifies the error message "unknown flag" appears
 * 5. Cleans up temporary files
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_PATH 1024
#define MAX_CMD 2048

/**
 * Creates a simple C source file that will generate coverage data
 */
int create_helper_source(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create helper source");
        return 0;
    }
    
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main() {\n");
    fprintf(f, "    printf(\"Helper program running\\n\");\n");
    fprintf(f, "    int x = 0;\n");
    fprintf(f, "    for (int i = 0; i < 10; i++) {\n");
    fprintf(f, "        x += i;\n");
    fprintf(f, "    }\n");
    fprintf(f, "    return x > 0 ? 0 : 1;\n");
    fprintf(f, "}\n");
    
    fclose(f);
    return 1;
}

/**
 * Compiles the helper program with coverage instrumentation
 */
int compile_helper(const char *source, const char *executable) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s 2>&1",
             executable, source);
    
    printf("Compiling helper: %s\n", cmd);
    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        perror("Failed to compile helper");
        return 0;
    }
    
    // Read and discard compilation output
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        // Check for compilation errors
        if (strstr(buffer, "error:") || strstr(buffer, "Error:")) {
            printf("Compilation error: %s", buffer);
            pclose(pipe);
            return 0;
        }
    }
    
    int status = pclose(pipe);
    if (status != 0) {
        printf("Compilation failed with status %d\n", status);
        return 0;
    }
    
    return 1;
}

/**
 * Runs the helper program to generate .gcda file
 */
int run_helper(const char *executable) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "./%s", executable);
    
    printf("Running helper: %s\n", cmd);
    int status = system(cmd);
    
    if (status != 0) {
        printf("Helper program failed with status %d\n", status);
        return 0;
    }
    
    // Check if .gcda file was created
    struct stat st;
    if (stat("helper.gcda", &st) == 0) {
        printf("Successfully created helper.gcda (%ld bytes)\n", (long)st.st_size);
        return 1;
    } else {
        printf("Failed to create helper.gcda\n");
        return 0;
    }
}

/**
 * Tests gcov-dump with a specific flag and checks for error message
 * Returns 1 if "unknown flag" error was found, 0 otherwise
 */
int test_invalid_flag(const char *gcda_file, char invalid_flag) {
    char cmd[MAX_CMD];
    char output[4096];
    int found_error = 0;
    
    // Build command with stderr redirected to stdout
    snprintf(cmd, sizeof(cmd), 
             "gcov-dump -%c %s 2>&1", invalid_flag, gcda_file);
    
    printf("Testing invalid flag -%c: %s\n", invalid_flag, cmd);
    
    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        perror("Failed to run gcov-dump");
        return 0;
    }
    
    // Read output
    output[0] = '\0';
    while (fgets(output + strlen(output), 
                 sizeof(output) - strlen(output), pipe) != NULL) {
        // Check if we've found the error message
        if (strstr(output, "unknown flag")) {
            found_error = 1;
        }
    }
    
    int status = pclose(pipe);
    
    printf("  Exit status: %d\n", status);
    printf("  Output: %s", output);
    
    if (found_error) {
        printf("  ✓ Found 'unknown flag' error\n");
    } else {
        printf("  ✗ Did not find 'unknown flag' error\n");
    }
    
    return found_error;
}

/**
 * Tests gcov-dump with a valid flag to ensure basic functionality works
 */
int test_valid_flag(const char *gcda_file) {
    char cmd[MAX_CMD];
    char output[4096];
    
    snprintf(cmd, sizeof(cmd), "gcov-dump -l %s 2>&1", gcda_file);
    
    printf("Testing valid flag -l: %s\n", cmd);
    
    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        perror("Failed to run gcov-dump with valid flag");
        return 0;
    }
    
    // Read and discard output
    while (fgets(output, sizeof(output), pipe) != NULL) {
        // Just reading to consume output
    }
    
    int status = pclose(pipe);
    printf("  Valid command exit status: %d\n", status);
    
    return status == 0;  // Valid command should succeed
}

int main(int argc, char *argv[]) {
    printf("=== GCOV-Dump Invalid Flag Test ===\n\n");
    
    // Step 1: Create and compile helper program
    printf("1. Creating helper program...\n");
    if (!create_helper_source("helper.c")) {
        return 1;
    }
    
    if (!compile_helper("helper.c", "helper")) {
        unlink("helper.c");
        return 1;
    }
    
    // Step 2: Run helper to generate .gcda file
    printf("\n2. Generating coverage data...\n");
    if (!run_helper("helper")) {
        unlink("helper.c");
        unlink("helper");
        return 1;
    }
    
    // Step 3: Test with valid flag first
    printf("\n3. Testing valid flag (baseline)...\n");
    if (!test_valid_flag("helper.gcda")) {
        printf("Warning: Valid flag test failed - gcov-dump may not be installed\n");
    }
    
    // Step 4: Test various invalid flags
    printf("\n4. Testing invalid flags (targeting uncovered lines)...\n");
    
    // Array of invalid flags to test
    char invalid_flags[] = {
        'a',  // alphabetic, not in {h,v,l,p,r,s}
        'z',  // another alphabetic
        '1',  // numeric
        '?',  // punctuation
        'x',  // another alphabetic
        'A',  // uppercase (should also be invalid)
        '\0'  // null terminator
    };
    
    int tests_passed = 0;
    int total_tests = 0;
    
    for (int i = 0; invalid_flags[i] != '\0'; i++) {
        total_tests++;
        if (test_invalid_flag("helper.gcda", invalid_flags[i])) {
            tests_passed++;
        }
        printf("\n");
    }
    
    // Step 5: Test edge case - just a dash
    printf("5. Testing edge case (just dash)...\n");
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "gcov-dump - helper.gcda 2>&1");
    printf("Command: %s\n", cmd);
    
    FILE *pipe = popen(cmd, "r");
    if (pipe) {
        char output[4096];
        output[0] = '\0';
        while (fgets(output + strlen(output), 
                     sizeof(output) - strlen(output), pipe) != NULL) {
            // Read output
        }
        pclose(pipe);
        printf("Output: %s\n", output);
    }
    
    // Step 6: Cleanup
    printf("\n6. Cleaning up...\n");
    unlink("helper.c");
    unlink("helper");
    unlink("helper.gcda");
    unlink("helper.gcno");
    
    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    
    if (tests_passed > 0) {
        printf("✓ Successfully triggered the 'default' case in gcov-dump's option parser\n");
        return 0;
    } else {
        printf("✗ Failed to trigger the uncovered lines\n");
        return 1;
    }
}
