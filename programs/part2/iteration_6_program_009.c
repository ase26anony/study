/**
 * Test program to cover the default case in gcov-dump's switch statement
 * for handling unknown single-character command-line flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_CMD_LEN 1024
#define MAX_OUTPUT_LEN 4096

/**
 * Creates a temporary C source file, compiles it with coverage flags,
 * runs it to generate .gcda file, and returns the path to the .gcda file.
 * Caller must free the returned string.
 */
char* create_gcda_file() {
    // Create a unique temporary directory
    char tmpdir_template[] = "/tmp/gcov_test_XXXXXX";
    char* tmpdir = mkdtemp(tmpdir_template);
    if (!tmpdir) {
        perror("Failed to create temp directory");
        return NULL;
    }
    
    // Create helper.c source file
    char helper_c_path[512];
    snprintf(helper_c_path, sizeof(helper_c_path), "%s/helper.c", tmpdir);
    
    FILE* f = fopen(helper_c_path, "w");
    if (!f) {
        perror("Failed to create helper.c");
        return NULL;
    }
    
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main() {\n");
    fprintf(f, "    printf(\"Generating coverage data...\\n\");\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    fclose(f);
    
    // Compile with coverage flags
    char compile_cmd[512];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -o %s/helper %s/helper.c 2>&1",
             tmpdir, tmpdir);
    
    int compile_status = system(compile_cmd);
    if (compile_status != 0) {
        fprintf(stderr, "Failed to compile helper program\n");
        return NULL;
    }
    
    // Run the helper to generate .gcda file
    char run_cmd[512];
    snprintf(run_cmd, sizeof(run_cmd), "%s/helper", tmpdir);
    system(run_cmd);
    
    // Construct path to .gcda file
    char* gcda_path = malloc(512);
    snprintf(gcda_path, 512, "%s/helper.gcda", tmpdir);
    
    // Verify .gcda file was created
    struct stat st;
    if (stat(gcda_path, &st) != 0) {
        fprintf(stderr, "Failed to create .gcda file\n");
        free(gcda_path);
        return NULL;
    }
    
    return gcda_path;
}

/**
 * Runs gcov-dump with given arguments and captures stderr output.
 * Returns 1 if "unknown flag" message is found, 0 otherwise.
 */
int run_gcov_dump_and_check(const char* gcda_path, const char* flag, int expect_error) {
    char cmd[MAX_CMD_LEN];
    char output[MAX_OUTPUT_LEN];
    int found_unknown_flag = 0;
    
    // Build command: gcov-dump <flag> <gcda_file> 2>&1
    snprintf(cmd, sizeof(cmd), "gcov-dump %s \"%s\" 2>&1", flag, gcda_path);
    
    // Execute command and capture output
    FILE* fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        return 0;
    }
    
    // Read output
    output[0] = '\0';
    while (fgets(output + strlen(output), 
                 sizeof(output) - strlen(output), fp) != NULL) {
        // Check if we found the target error message
        if (strstr(output, "unknown flag") != NULL) {
            found_unknown_flag = 1;
        }
    }
    
    int status = pclose(fp);
    
    if (expect_error) {
        printf("Testing flag '%s':\n", flag);
        printf("  Command: %s\n", cmd);
        printf("  Output: %s", output);
        printf("  Found 'unknown flag': %s\n", found_unknown_flag ? "YES" : "NO");
        printf("  Exit status: %d\n\n", WEXITSTATUS(status));
    }
    
    return found_unknown_flag;
}

/**
 * Clean up temporary files
 */
void cleanup(const char* gcda_path) {
    if (!gcda_path) return;
    
    // Extract directory from gcda_path
    char* last_slash = strrchr(gcda_path, '/');
    if (last_slash) {
        char dir_path[512];
        size_t dir_len = last_slash - gcda_path;
        strncpy(dir_path, gcda_path, dir_len);
        dir_path[dir_len] = '\0';
        
        char rm_cmd[512];
        snprintf(rm_cmd, sizeof(rm_cmd), "rm -rf %s", dir_path);
        system(rm_cmd);
    }
}

int main() {
    printf("=== GCOV-Dump Unknown Flag Test ===\n\n");
    
    // Step 1: Generate GCOV data file
    printf("1. Generating GCOV data file...\n");
    char* gcda_path = create_gcda_file();
    if (!gcda_path) {
        fprintf(stderr, "Failed to create GCOV data file\n");
        return 1;
    }
    printf("   Created: %s\n\n", gcda_path);
    
    // Step 2: Test valid flag first (to ensure tool works)
    printf("2. Testing valid flag (-l) to verify gcov-dump works...\n");
    int valid_test = run_gcov_dump_and_check(gcda_path, "-l", 0);
    printf("   Valid flag test completed (no error expected)\n\n");
    
    // Step 3: Test various invalid single-character flags
    printf("3. Testing invalid single-character flags (targeting default case)...\n");
    
    // Array of invalid flags to test
    const char* invalid_flags[] = {
        "-a",  // alphabetic, not in {h,v,l,p,r,s}
        "-z",  // another alphabetic
        "-x",  // another alphabetic
        "-1",  // numeric
        "-?",  // special character
        "-@",  // special character
        "-",   // just a dash (might trigger different error)
        NULL
    };
    
    int tests_passed = 0;
    int total_tests = 0;
    
    for (int i = 0; invalid_flags[i] != NULL; i++) {
        total_tests++;
        int found = run_gcov_dump_and_check(gcda_path, invalid_flags[i], 1);
        if (found) {
            tests_passed++;
        }
    }
    
    // Step 4: Summary
    printf("4. Test Summary:\n");
    printf("   Total invalid flag tests: %d\n", total_tests);
    printf("   Tests that triggered 'unknown flag' error: %d\n", tests_passed);
    
    if (tests_passed > 0) {
        printf("   SUCCESS: Default case in switch statement was triggered!\n");
    } else {
        printf("   FAILURE: Default case was not triggered\n");
    }
    
    // Step 5: Cleanup
    printf("\n5. Cleaning up temporary files...\n");
    cleanup(gcda_path);
    free(gcda_path);
    
    return (tests_passed > 0) ? 0 : 1;
}
