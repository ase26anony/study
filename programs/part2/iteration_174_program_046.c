/**
 * test_gcov_dump_coverage.c
 * 
 * A test program to exercise the uncovered switch-case lines in gcov-dump.cc
 * Specifically targets lines 111-130 handling command-line flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH 1024
#define MAX_CMD 4096

/**
 * Execute a shell command and return its exit status.
 */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/**
 * Check if a file exists.
 */
int file_exists(const char *path) {
    return access(path, F_OK) == 0;
}

/**
 * Build the instrumented gcov-dump binary if it doesn't exist.
 * Returns 0 on success, non-zero on failure.
 */
int build_instrumented_gcov_dump(const char *source_dir, const char *output_path) {
    char cmd[MAX_CMD];
    
    // Check if we already have the instrumented binary
    if (file_exists(output_path)) {
        printf("Instrumented gcov-dump already exists at: %s\n", output_path);
        return 0;
    }
    
    printf("Building instrumented gcov-dump...\n");
    
    // Try to find gcov-dump source in common locations
    const char *possible_sources[] = {
        "gcov-dump.cc",
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "gcc/gcov-dump.cc",
        NULL
    };
    
    const char *source_path = NULL;
    for (int i = 0; possible_sources[i] != NULL; i++) {
        if (file_exists(possible_sources[i])) {
            source_path = possible_sources[i];
            break;
        }
    }
    
    if (source_path == NULL && source_dir != NULL) {
        char full_path[MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s/gcov-dump.cc", source_dir);
        if (file_exists(full_path)) {
            source_path = full_path;
        }
    }
    
    if (source_path == NULL) {
        fprintf(stderr, "ERROR: Could not find gcov-dump.cc source file\n");
        fprintf(stderr, "Please specify source directory or place gcov-dump.cc in current directory\n");
        return 1;
    }
    
    // Build command to compile instrumented gcov-dump
    // Using minimal flags - adjust as needed for your environment
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../include -I../../include "
             "-I../libiberty -I../../libiberty %s "
             "../libiberty/libiberty.a -o %s",
             source_path, output_path);
    
    // Try alternative if libiberty.a not found
    if (execute_command(cmd) != 0) {
        printf("First compilation attempt failed, trying alternative...\n");
        
        // Try without libiberty (some systems have it as a shared library)
        snprintf(cmd, sizeof(cmd),
                 "g++ -O0 -fprofile-arcs -ftest-coverage %s "
                 "-liberty -o %s",
                 source_path, output_path);
        
        if (execute_command(cmd) != 0) {
            fprintf(stderr, "ERROR: Failed to compile instrumented gcov-dump\n");
            fprintf(stderr, "You may need to adjust library paths\n");
            return 1;
        }
    }
    
    printf("Successfully built instrumented gcov-dump at: %s\n", output_path);
    return 0;
}

/**
 * Create a dummy C program to generate GCOV data files.
 */
int create_dummy_program(const char *dummy_source) {
    FILE *fp = fopen(dummy_source, "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return 1;
    }
    
    fprintf(fp, "/* dummy.c - Simple program to generate GCOV data */\n");
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i;\n");
    fprintf(fp, "    printf(\"Hello from dummy program\\n\");\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        printf(\"Iteration %%d\\n\", i);\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/**
 * Generate GCOV data file by compiling and running dummy program.
 */
int generate_gcov_data(const char *dummy_source, const char *gcda_file) {
    char cmd[MAX_CMD];
    
    printf("Generating GCOV data file...\n");
    
    // Compile dummy program with coverage
    snprintf(cmd, sizeof(cmd),
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o dummy_prog",
             dummy_source);
    
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "ERROR: Failed to compile dummy program\n");
        return 1;
    }
    
    // Run the dummy program to generate .gcda file
    if (execute_command("./dummy_prog") != 0) {
        fprintf(stderr, "ERROR: Failed to run dummy program\n");
        return 1;
    }
    
    // Check if .gcda file was created
    if (!file_exists(gcda_file)) {
        fprintf(stderr, "ERROR: GCOV data file not created: %s\n", gcda_file);
        fprintf(stderr, "Looking for alternative .gcda files...\n");
        
        // Try to find any .gcda file
        if (execute_command("find . -name \"*.gcda\" -type f | head -5") != 0) {
            return 1;
        }
        return 1;
    }
    
    printf("Generated GCOV data file: %s\n", gcda_file);
    return 0;
}

/**
 * Merge coverage data from gcov-dump execution.
 */
int merge_coverage_data(const char *gcov_dump_binary, const char *source_file) {
    char cmd[MAX_CMD];
    
    printf("Merging coverage data...\n");
    
    // Method 1: Use gcov directly on the source file
    snprintf(cmd, sizeof(cmd), "gcov -i %s", source_file);
    int result = execute_command(cmd);
    
    if (result != 0) {
        // Method 2: Copy .gcda files to appropriate location
        printf("Alternative merge method...\n");
        
        // Find .gcda files for gcov-dump
        snprintf(cmd, sizeof(cmd),
                 "find . -name \"*.gcda\" | xargs -I {} cp {} . 2>/dev/null || true");
        execute_command(cmd);
        
        // Run gcov again
        snprintf(cmd, sizeof(cmd), "gcov %s", source_file);
        result = execute_command(cmd);
    }
    
    return result;
}

/**
 * Run gcov-dump with specific arguments and merge coverage.
 */
int test_gcov_dump(const char *gcov_dump_binary, const char *gcda_file, 
                   const char *args, int expect_success) {
    char cmd[MAX_CMD];
    int status;
    
    // Build the command
    if (strstr(args, "-h") != NULL || strstr(args, "-v") != NULL) {
        // Help and version don't need a gcda file
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_binary, args);
    } else {
        // Other flags need the gcda file
        snprintf(cmd, sizeof(cmd), "%s %s %s", gcov_dump_binary, args, gcda_file);
    }
    
    printf("\n=== Testing: %s ===\n", cmd);
    
    // Execute the command
    status = execute_command(cmd);
    
    // Check if exit status matches expectation
    if (expect_success) {
        if (status != 0) {
            fprintf(stderr, "WARNING: Command failed with status %d (expected success)\n", status);
        }
    } else {
        if (status == 0) {
            fprintf(stderr, "WARNING: Command succeeded (expected failure)\n");
        }
    }
    
    // Merge coverage data after each run
    merge_coverage_data(gcov_dump_binary, "gcov-dump.cc");
    
    return status;
}

/**
 * Display coverage information for the target lines.
 */
void check_coverage() {
    char cmd[MAX_CMD];
    
    printf("\n=== Checking Coverage ===\n");
    
    // Generate human-readable coverage report
    snprintf(cmd, sizeof(cmd), "gcov -b gcov-dump.cc");
    execute_command(cmd);
    
    // Look for coverage of specific lines
    printf("\nChecking for coverage of target lines (111-130)...\n");
    
    // Try to extract coverage information
    snprintf(cmd, sizeof(cmd), 
             "if [ -f gcov-dump.cc.gcov ]; then "
             "echo 'Lines 111-130 coverage:'; "
             "sed -n '111,130p' gcov-dump.cc.gcov | grep -E '^[[:space:]]*[0-9]+:|^[[:space:]]*#####'; "
             "fi");
    execute_command(cmd);
    
    // Also check the .gcov file directly
    if (file_exists("gcov-dump.cc.gcov")) {
        printf("\nFirst few lines of coverage report:\n");
        snprintf(cmd, sizeof(cmd), "head -50 gcov-dump.cc.gcov");
        execute_command(cmd);
    }
}

int main(int argc, char *argv[]) {
    const char *gcov_dump_source_dir = NULL;
    const char *instrumented_binary = "./gcov-dump-instrumented";
    const char *dummy_source = "dummy.c";
    const char *gcda_file = "dummy.gcda";
    
    printf("=== GCOV-Dump Coverage Test ===\n");
    
    // Parse command line arguments
    if (argc > 1) {
        gcov_dump_source_dir = argv[1];
    }
    
    // Step 1: Build or locate instrumented gcov-dump
    if (build_instrumented_gcov_dump(gcov_dump_source_dir, instrumented_binary) != 0) {
        return 1;
    }
    
    // Step 2: Create dummy program and generate GCOV data
    if (create_dummy_program(dummy_source) != 0) {
        return 1;
    }
    
    if (generate_gcov_data(dummy_source, gcda_file) != 0) {
        return 1;
    }
    
    // Clean up any existing coverage data
    execute_command("rm -f *.gcda *.gcno gcov-dump.cc.gcov 2>/dev/null || true");
    
    // Step 3: Execute comprehensive flag testing
    
    // Test help flag (doesn't need gcda file)
    test_gcov_dump(instrumented_binary, gcda_file, "-h", 1);
    
    // Test version flag (doesn't need gcda file)
    test_gcov_dump(instrumented_binary, gcda_file, "-v", 1);
    
    // Test individual flags
    test_gcov_dump(instrumented_binary, gcda_file, "-l", 1);
    test_gcov_dump(instrumented_binary, gcda_file, "-p", 1);
    test_gcov_dump(instrumented_binary, gcda_file, "-r", 1);
    test_gcov_dump(instrumented_binary, gcda_file, "-s", 1);
    
    // Test combined flags (space-separated)
    test_gcov_dump(instrumented_binary, gcda_file, "-l -p -r -s", 1);
    
    // Test concatenated flags
    test_gcov_dump(instrumented_binary, gcda_file, "-lprs", 1);
    
    // Test various combinations
    test_gcov_dump(instrumented_binary, gcda_file, "-lp", 1);
    test_gcov_dump(instrumented_binary, gcda_file, "-rs", 1);
    test_gcov_dump(instrumented_binary, gcda_file, "-l -s", 1);
    
    // Test with multiple gcda files (if available)
    test_gcov_dump(instrumented_binary, gcda_file, "-l dummy.gcda dummy.gcda", 1);
    
    // Test invalid flag (should trigger default case)
    test_gcov_dump(instrumented_binary, gcda_file, "-x", 0);
    
    // Test another invalid flag combination
    test_gcov_dump(instrumented_binary, gcda_file, "-l -x", 0);
    
    // Step 4: Final coverage check
    check_coverage();
    
    // Step 5: Cleanup (optional)
    printf("\n=== Test Complete ===\n");
    printf("Generated files:\n");
    execute_command("ls -la *.gcda *.gcno *.gcov dummy* gcov-dump-instrumented 2>/dev/null || true");
    
    printf("\nTo view detailed coverage:\n");
    printf("  gcov -b gcov-dump.cc\n");
    printf("  cat gcov-dump.cc.gcov | grep -n -A2 -B2 '111\\|112\\|113\\|114\\|115\\|116\\|117\\|118\\|119\\|120'\n");
    printf("  cat gcov-dump.cc.gcov | grep -n -A2 -B2 '121\\|122\\|123\\|124\\|125\\|126\\|127\\|128\\|129\\|130'\n");
    
    return 0;
}
