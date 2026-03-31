/**
 * Test program to cover the default case in gcov-dump's option parsing.
 * This program generates GCOV data files and tests gcov-dump with invalid flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024
#define MAX_CMD 2048

/**
 * Creates a simple C source file for generating GCOV data.
 */
void create_helper_source(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to create helper source");
        exit(1);
    }
    
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main() {\n");
    fprintf(f, "    printf(\"Generating GCOV data...\\n\");\n");
    fprintf(f, "    int x = 5;\n");
    fprintf(f, "    if (x > 0) {\n");
    fprintf(f, "        printf(\"x is positive\\n\");\n");
    fprintf(f, "    }\n");
    fprintf(f, "    for (int i = 0; i < 3; i++) {\n");
    fprintf(f, "        printf(\"Iteration %%d\\n\", i);\n");
    fprintf(f, "    }\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
}

/**
 * Compiles the helper program with GCOV instrumentation.
 */
void compile_helper(const char *source, const char *executable) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -o %s %s", 
             executable, source);
    
    printf("Compiling helper: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Failed to compile helper program\n");
        exit(1);
    }
}

/**
 * Runs a command and captures its stderr output.
 * Returns the command's exit status.
 */
int run_command(const char *cmd, char *output, size_t output_size) {
    char full_cmd[MAX_CMD];
    // Redirect stderr to stdout for capture
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    printf("Running: %s\n", cmd);
    
    FILE *pipe = popen(full_cmd, "r");
    if (!pipe) {
        perror("popen failed");
        return -1;
    }
    
    // Clear output buffer
    if (output && output_size > 0) {
        output[0] = '\0';
    }
    
    // Read output
    if (output) {
        size_t total_read = 0;
        while (!feof(pipe) && total_read < output_size - 1) {
            size_t bytes_read = fread(output + total_read, 1, 
                                     output_size - total_read - 1, pipe);
            total_read += bytes_read;
            output[total_read] = '\0';
        }
    }
    
    int status = pclose(pipe);
    return WEXITSTATUS(status);
}

/**
 * Tests gcov-dump with various flags.
 */
void test_gcov_dump(const char *gcov_dump_path, const char *gcda_file) {
    char cmd[MAX_CMD];
    char output[4096];
    int status;
    
    printf("\n=== Testing gcov-dump ===\n");
    
    // Test 1: Valid flag to ensure basic functionality works
    printf("\n--- Test 1: Valid flag (-l) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -l %s", gcov_dump_path, gcda_file);
    status = run_command(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status);
    printf("Output (first 200 chars):\n%.200s\n", output);
    
    // Test 2-7: Invalid single-character flags
    char invalid_flags[] = "a?xz19";  // Various invalid characters
    for (int i = 0; i < strlen(invalid_flags); i++) {
        printf("\n--- Test %d: Invalid flag (-%c) ---\n", 
               i + 2, invalid_flags[i]);
        snprintf(cmd, sizeof(cmd), "%s -%c %s", 
                 gcov_dump_path, invalid_flags[i], gcda_file);
        status = run_command(cmd, output, sizeof(output));
        printf("Exit status: %d\n", status);
        
        // Check for the expected error message
        if (strstr(output, "unknown flag") != NULL) {
            printf("✓ Found 'unknown flag' in output\n");
        } else {
            printf("✗ 'unknown flag' NOT found in output\n");
            printf("Output (first 300 chars):\n%.300s\n", output);
        }
        
        // Invalid flags should return non-zero exit status
        if (status != 0) {
            printf("✓ Non-zero exit status as expected\n");
        } else {
            printf("✗ Unexpected zero exit status\n");
        }
    }
    
    // Test 8: Multiple invalid flags in sequence
    printf("\n--- Test 8: Multiple invalid flags (-ab) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -ab %s", gcov_dump_path, gcda_file);
    status = run_command(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status);
    printf("Output (first 300 chars):\n%.300s\n", output);
    
    // Test 9: Valid flag followed by invalid flag
    printf("\n--- Test 9: Valid + invalid flag (-l -x) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -l -x %s", gcov_dump_path, gcda_file);
    status = run_command(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status);
    if (strstr(output, "unknown flag") != NULL) {
        printf("✓ Found 'unknown flag' in output\n");
    }
    
    // Test 10: Invalid flag with no argument (should also trigger error)
    printf("\n--- Test 10: Invalid flag only (-q) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -q", gcov_dump_path);
    status = run_command(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status);
    printf("Output (first 300 chars):\n%.300s\n", output);
}

int main(int argc, char *argv[]) {
    char cwd[MAX_PATH];
    char helper_source[MAX_PATH];
    char helper_exec[MAX_PATH];
    char gcda_file[MAX_PATH];
    
    // Get current directory
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed");
        return 1;
    }
    
    // Create paths for helper files
    snprintf(helper_source, sizeof(helper_source), "%s/helper_gcov.c", cwd);
    snprintf(helper_exec, sizeof(helper_exec), "%s/helper_gcov", cwd);
    snprintf(gcda_file, sizeof(gcda_file), "%s/helper_gcov.gcda", cwd);
    
    printf("Working directory: %s\n", cwd);
    
    // Step 1: Create helper source file
    printf("\n=== Creating helper source ===\n");
    create_helper_source(helper_source);
    
    // Step 2: Compile helper with GCOV instrumentation
    printf("\n=== Compiling helper ===\n");
    compile_helper(helper_source, helper_exec);
    
    // Step 3: Run helper to generate .gcda file
    printf("\n=== Running helper to generate GCOV data ===\n");
    int status = system(helper_exec);
    if (status != 0) {
        fprintf(stderr, "Failed to run helper program\n");
        return 1;
    }
    
    // Verify .gcda file was created
    struct stat st;
    if (stat(gcda_file, &st) != 0) {
        fprintf(stderr, "GCOV data file not created: %s\n", gcda_file);
        return 1;
    }
    printf("GCOV data file created: %s (%ld bytes)\n", 
           gcda_file, (long)st.st_size);
    
    // Step 4: Test gcov-dump
    // Try to find gcov-dump in common locations
    const char *gcov_dump_candidates[] = {
        "./gcov-dump",
        "gcov-dump",
        "/usr/bin/gcov-dump",
        "/usr/local/bin/gcov-dump",
        NULL
    };
    
    const char *gcov_dump_path = NULL;
    for (int i = 0; gcov_dump_candidates[i]; i++) {
        if (access(gcov_dump_candidates[i], X_OK) == 0) {
            gcov_dump_path = gcov_dump_candidates[i];
            break;
        }
    }
    
    if (!gcov_dump_path) {
        fprintf(stderr, "Could not find gcov-dump executable\n");
        fprintf(stderr, "Please specify path as argument: %s <path-to-gcov-dump>\n", 
                argv[0]);
        
        // Try to build gcov-dump from source if available
        printf("\nAttempting to build gcov-dump from gcov-dump.cc...\n");
        char build_cmd[MAX_CMD];
        snprintf(build_cmd, sizeof(build_cmd), 
                 "g++ -fprofile-arcs -ftest-coverage -o gcov-dump gcov-dump.cc");
        if (system(build_cmd) == 0) {
            gcov_dump_path = "./gcov-dump";
            printf("Built gcov-dump successfully\n");
        } else {
            return 1;
        }
    }
    
    printf("Using gcov-dump: %s\n", gcov_dump_path);
    
    // Run the tests
    test_gcov_dump(gcov_dump_path, gcda_file);
    
    // Step 5: Cleanup
    printf("\n=== Cleaning up ===\n");
    remove(helper_source);
    remove(helper_exec);
    remove(gcda_file);
    
    // Also remove other GCOV files
    char gcno_file[MAX_PATH];
    snprintf(gcno_file, sizeof(gcno_file), "%s/helper_gcov.gcno", cwd);
    remove(gcno_file);
    
    printf("Test completed successfully\n");
    return 0;
}
