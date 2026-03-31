/**
 * test_gcov_dump_coverage.c
 * 
 * Test program to cover the uncovered switch-case lines in gcov-dump.cc
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
 * Returns 0 on success, -1 on failure.
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
        "gcc/gcov-dump.cc",
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        NULL
    };
    
    char source_path[MAX_PATH] = "";
    for (int i = 0; possible_sources[i] != NULL; i++) {
        if (file_exists(possible_sources[i])) {
            strcpy(source_path, possible_sources[i]);
            break;
        }
    }
    
    if (strlen(source_path) == 0) {
        // Try to find it using find command
        printf("Searching for gcov-dump source...\n");
        FILE *fp = popen("find . -name 'gcov-dump.cc' -type f 2>/dev/null | head -1", "r");
        if (fp) {
            if (fgets(source_path, sizeof(source_path), fp)) {
                // Remove newline
                source_path[strcspn(source_path, "\n")] = 0;
            }
            pclose(fp);
        }
    }
    
    if (strlen(source_path) == 0) {
        fprintf(stderr, "ERROR: Could not find gcov-dump.cc source file\n");
        return -1;
    }
    
    printf("Found source at: %s\n", source_path);
    
    // Build command to compile instrumented gcov-dump
    // This is a simplified version - adjust based on your actual build environment
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include -I../../libiberty "
        "%s ../../libiberty/libiberty.a -o %s",
        source_path, output_path);
    
    printf("Compilation command: %s\n", cmd);
    
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "ERROR: Failed to build instrumented gcov-dump\n");
        
        // Try a simpler compilation if the above fails
        printf("Trying simpler compilation...\n");
        snprintf(cmd, sizeof(cmd),
            "g++ -O0 -fprofile-arcs -ftest-coverage %s -o %s",
            source_path, output_path);
        
        if (execute_command(cmd) != 0) {
            fprintf(stderr, "ERROR: Simple compilation also failed\n");
            return -1;
        }
    }
    
    printf("Successfully built instrumented gcov-dump at: %s\n", output_path);
    return 0;
}

/**
 * Create a dummy C program to generate GCOV data.
 */
int create_dummy_program(const char *dummy_source) {
    FILE *fp = fopen(dummy_source, "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return -1;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        printf(\"Value: %%d\\n\", i);\n");
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
        return -1;
    }
    
    // Run the program to generate .gcda file
    if (execute_command("./dummy_prog") != 0) {
        fprintf(stderr, "ERROR: Failed to run dummy program\n");
        return -1;
    }
    
    // The .gcda file will be named based on the source file
    // Let's find it and copy it to our desired location
    snprintf(cmd, sizeof(cmd),
        "find . -name '*.gcda' -type f 2>/dev/null | head -1");
    
    FILE *fp = popen(cmd, "r");
    if (fp) {
        char found_gcda[MAX_PATH] = "";
        if (fgets(found_gcda, sizeof(found_gcda), fp)) {
            found_gcda[strcspn(found_gcda, "\n")] = 0;
            // Copy to our desired location
            snprintf(cmd, sizeof(cmd), "cp \"%s\" \"%s\"", found_gcda, gcda_file);
            execute_command(cmd);
        }
        pclose(fp);
    }
    
    // Clean up
    execute_command("rm -f dummy_prog");
    
    printf("GCOV data file ready at: %s\n", gcda_file);
    return 0;
}

/**
 * Merge coverage data after each gcov-dump invocation.
 */
void merge_coverage_data(const char *gcov_dump_binary) {
    char cmd[MAX_CMD];
    
    // Method 1: Use gcov -i to merge intermediate files
    snprintf(cmd, sizeof(cmd), "gcov -i %s 2>/dev/null", gcov_dump_binary);
    execute_command(cmd);
    
    // Method 2: Copy .gcda files to a common location
    // This ensures we capture all coverage data
    execute_command("mkdir -p coverage_data");
    execute_command("cp *.gcda coverage_data/ 2>/dev/null || true");
}

/**
 * Run gcov-dump with specific arguments and merge coverage.
 */
int test_gcov_dump(const char *gcov_dump_binary, const char *gcda_file, 
                   const char *args, int expect_success) {
    char cmd[MAX_CMD];
    int status;
    
    // Build the command
    if (strstr(args, "-h") || strstr(args, "-v")) {
        // Help and version don't need a gcda file
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_binary, args);
    } else if (strstr(args, "-x")) {
        // Invalid flag test
        snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_binary, args);
    } else {
        // Normal case with gcda file
        snprintf(cmd, sizeof(cmd), "%s %s %s", gcov_dump_binary, args, gcda_file);
    }
    
    printf("\n=== Testing: %s ===\n", args);
    status = execute_command(cmd);
    
    // Merge coverage data after this run
    merge_coverage_data(gcov_dump_binary);
    
    // Check if result matches expectation
    if (expect_success) {
        if (status != 0) {
            fprintf(stderr, "WARNING: Command failed with status %d: %s\n", status, args);
        }
    } else {
        if (status == 0) {
            fprintf(stderr, "WARNING: Expected failure but command succeeded: %s\n", args);
        }
    }
    
    return status;
}

/**
 * Generate final coverage report and check target lines.
 */
void generate_coverage_report(const char *source_file) {
    char cmd[MAX_CMD];
    
    printf("\n=== Generating Coverage Report ===\n");
    
    // Generate human-readable coverage report
    snprintf(cmd, sizeof(cmd), "gcov -b %s", source_file);
    execute_command(cmd);
    
    // Look for the gcov output file
    char gcov_file[MAX_PATH];
    snprintf(gcov_file, sizeof(gcov_file), "%s.gcov", source_file);
    
    if (file_exists(gcov_file)) {
        printf("\n=== Coverage Summary for target lines ===\n");
        
        // Extract and display coverage for lines 111-130
        snprintf(cmd, sizeof(cmd), 
            "sed -n '111,130p' %s | grep -E '^[[:space:]]*[0-9]+|[[:space:]]*#####'",
            gcov_file);
        execute_command(cmd);
        
        printf("\n=== Full coverage file created: %s ===\n", gcov_file);
    } else {
        // Try to find it
        execute_command("find . -name '*.gcov' -type f 2>/dev/null");
    }
}

int main(int argc, char *argv[]) {
    const char *gcov_dump_binary = "./gcov-dump-instrumented";
    const char *dummy_source = "dummy.c";
    const char *gcda_file = "test.gcda";
    const char *gcov_dump_source = "gcov-dump.cc";
    
    printf("=== Starting gcov-dump coverage test ===\n");
    
    // Step 1: Build instrumented gcov-dump
    if (build_instrumented_gcov_dump(".", gcov_dump_binary) != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    // Step 2: Create dummy program and generate GCOV data
    if (create_dummy_program(dummy_source) != 0) {
        return 1;
    }
    
    if (generate_gcov_data(dummy_source, gcda_file) != 0) {
        fprintf(stderr, "Failed to generate GCOV data\n");
        return 1;
    }
    
    // Clean up any existing coverage data
    execute_command("rm -f *.gcda *.gcno coverage_data/* 2>/dev/null || true");
    
    // Step 3: Execute comprehensive flag testing
    // Each test will merge coverage data after execution
    
    // Test individual flags (lines 111-130)
    test_gcov_dump(gcov_dump_binary, gcda_file, "-h", 1);      // help
    test_gcov_dump(gcov_dump_binary, gcda_file, "-v", 1);      // version
    
    // Individual dump flags
    test_gcov_dump(gcov_dump_binary, gcda_file, "-l", 1);      // flag_dump_contents
    test_gcov_dump(gcov_dump_binary, gcda_file, "-p", 1);      // flag_dump_positions
    test_gcov_dump(gcov_dump_binary, gcda_file, "-r", 1);      // flag_dump_raw
    test_gcov_dump(gcov_dump_binary, gcda_file, "-s", 1);      // flag_dump_stable
    
    // Combined flags (space-separated)
    test_gcov_dump(gcov_dump_binary, gcda_file, "-l -p -r -s", 1);
    
    // Concatenated flags
    test_gcov_dump(gcov_dump_binary, gcda_file, "-lprs", 1);
    
    // Invalid flag (to trigger default case)
    test_gcov_dump(gcov_dump_binary, gcda_file, "-x", 0);
    
    // Additional combinations
    test_gcov_dump(gcov_dump_binary, gcda_file, "-lp", 1);
    test_gcov_dump(gcov_dump_binary, gcda_file, "-rs", 1);
    test_gcov_dump(gcov_dump_binary, gcda_file, "-l -s", 1);
    
    // Step 4: Generate final coverage report
    generate_coverage_report(gcov_dump_source);
    
    // Step 5: Verify coverage
    printf("\n=== Verification ===\n");
    printf("All flag combinations tested:\n");
    printf("  -h (help)              - Should execute print_usage()\n");
    printf("  -v (version)           - Should execute print_version()\n");
    printf("  -l                     - Should set flag_dump_contents\n");
    printf("  -p                     - Should set flag_dump_positions\n");
    printf("  -r                     - Should set flag_dump_raw\n");
    printf("  -s                     - Should set flag_dump_stable\n");
    printf("  -l -p -r -s            - Should set all flags\n");
    printf("  -lprs                  - Should set all flags (concatenated)\n");
    printf("  -x                     - Should trigger default case\n");
    
    printf("\nCheck the generated .gcov file to confirm lines 111-130 are covered.\n");
    printf("Look for execution counts (numbers) instead of '#####' for uncovered lines.\n");
    
    // Cleanup
    execute_command("rm -f dummy.c dummy_prog 2>/dev/null || true");
    
    printf("\n=== Test complete ===\n");
    return 0;
}
