// test_gcov_dump_coverage.c
// Test wrapper to cover the switch-case argument parsing in gcov-dump.cc
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024

// Function to check if a file exists
int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

// Function to execute a command and check exit status
int execute_command(const char *cmd, int expect_success) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    
    if (expect_success) {
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Command failed: %s\n", cmd);
            return 0;
        }
    }
    return 1;
}

// Function to compile gcov-dump with coverage instrumentation
int compile_gcov_dump_with_coverage(const char *source_dir, const char *output_path) {
    char cmd[MAX_PATH * 4];
    
    // Check if we have the source file
    char source_path[MAX_PATH];
    snprintf(source_path, sizeof(source_path), "%s/gcov-dump.cc", source_dir);
    
    if (!file_exists(source_path)) {
        fprintf(stderr, "Source file not found: %s\n", source_path);
        return 0;
    }
    
    // Try to find libiberty
    char libiberty_path[MAX_PATH];
    snprintf(libiberty_path, sizeof(liberty_path), "%s/../../libiberty/libiberty.a", source_dir);
    
    if (!file_exists(liberty_path)) {
        // Try alternative path
        snprintf(libiberty_path, sizeof(liberty_path), "%s/libiberty.a", source_dir);
    }
    
    // Build command to compile gcov-dump with coverage
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-I%s -I%s/../../include -I%s/../../libiberty "
        "%s/gcov-dump.cc %s -o %s",
        source_dir, source_dir, source_dir,
        source_dir, libiberty_path, output_path);
    
    return execute_command(cmd, 1);
}

// Function to create a dummy C program for generating GCOV data
int create_dummy_program(const char *output_path) {
    FILE *fp = fopen(output_path, "w");
    if (!fp) {
        perror("Failed to create dummy program");
        return 0;
    }
    
    fprintf(fp, "/* dummy.c - Simple program to generate GCOV data */\n");
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        printf(\"Iteration %%d\\n\", i);\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 1;
}

// Function to compile and run dummy program to generate .gcda file
int generate_gcov_data(const char *dummy_source, const char *gcda_path) {
    char cmd[MAX_PATH * 4];
    
    // Compile dummy program with coverage
    snprintf(cmd, sizeof(cmd),
        "gcc -O0 -fprofile-arcs -ftest-coverage %s -o dummy_prog",
        dummy_source);
    
    if (!execute_command(cmd, 1)) {
        return 0;
    }
    
    // Run the dummy program to generate .gcda file
    if (!execute_command("./dummy_prog", 1)) {
        return 0;
    }
    
    // The .gcda file will be created in the current directory
    // Copy it to the desired location if needed
    if (strcmp(gcda_path, "dummy.gcda") != 0) {
        snprintf(cmd, sizeof(cmd), "cp dummy.gcda %s", gcda_path);
        execute_command(cmd, 1);
    }
    
    return 1;
}

// Function to merge coverage data after each gcov-dump invocation
void merge_coverage_data(const char *gcov_dump_source) {
    char cmd[MAX_PATH * 4];
    
    // Method 1: Use gcov -i to merge intermediate files
    snprintf(cmd, sizeof(cmd), "gcov -i %s 2>/dev/null", gcov_dump_source);
    system(cmd);
    
    // Method 2: Alternative approach - copy .gcda files to common location
    // This ensures coverage from multiple runs is accumulated
    system("mkdir -p coverage_data");
    system("cp gcov-dump-instrumented*.gcda coverage_data/ 2>/dev/null || true");
}

// Function to run gcov-dump with specific flags
void run_gcov_dump_test(const char *gcov_dump_bin, const char *gcda_file, 
                       const char *flags, int expect_success) {
    char cmd[MAX_PATH * 4];
    
    if (strcmp(flags, "-h") == 0 || strcmp(flags, "-v") == 0) {
        // Help and version don't need a gcda file
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_bin, flags);
    } else if (strstr(flags, "-x") != NULL) {
        // Invalid flag - include gcda file but expect failure
        snprintf(cmd, sizeof(cmd), "%s %s %s 2>&1", gcov_dump_bin, flags, gcda_file);
        expect_success = 0; // We expect this to fail
    } else {
        // Normal case with gcda file
        snprintf(cmd, sizeof(cmd), "%s %s %s", gcov_dump_bin, flags, gcda_file);
    }
    
    execute_command(cmd, expect_success);
    
    // Merge coverage data after each run
    merge_coverage_data("gcov-dump.cc");
}

// Main test function
int main(int argc, char *argv[]) {
    printf("=== Starting gcov-dump coverage test ===\n");
    
    // Configuration
    const char *gcov_dump_source_dir = ".";  // Current directory
    const char *gcov_dump_binary = "./gcov-dump-instrumented";
    const char *dummy_source = "dummy.c";
    const char *gcda_file = "dummy.gcda";
    
    // Step 1: Compile gcov-dump with coverage instrumentation
    printf("\n1. Compiling gcov-dump with coverage instrumentation...\n");
    if (!compile_gcov_dump_with_coverage(gcov_dump_source_dir, gcov_dump_binary)) {
        fprintf(stderr, "Failed to compile gcov-dump with coverage\n");
        
        // Try a simpler approach - maybe gcov-dump already exists
        if (!file_exists(gcov_dump_binary)) {
            fprintf(stderr, "gcov-dump binary not found and compilation failed\n");
            return 1;
        }
        printf("Using existing gcov-dump binary\n");
    }
    
    // Step 2: Create and compile dummy program for GCOV data
    printf("\n2. Generating test GCOV data file...\n");
    if (!create_dummy_program(dummy_source)) {
        fprintf(stderr, "Failed to create dummy program\n");
        return 1;
    }
    
    if (!generate_gcov_data(dummy_source, gcda_file)) {
        fprintf(stderr, "Failed to generate GCOV data\n");
        return 1;
    }
    
    if (!file_exists(gcda_file)) {
        fprintf(stderr, "GCOV data file not created: %s\n", gcda_file);
        return 1;
    }
    
    // Clean up any existing coverage data
    system("rm -f gcov-dump-instrumented*.gcda gcov-dump-instrumented*.gcno");
    system("rm -f *.gcov");
    
    // Step 3: Run comprehensive flag tests
    printf("\n3. Running comprehensive flag tests...\n");
    
    // Test individual flags (lines 111-130)
    printf("\nTesting individual flags:\n");
    run_gcov_dump_test(gcov_dump_binary, gcda_file, "-h", 1);      // help
    run_gcov_dump_test(gcov_dump_binary, gcda_file, "-v", 1);      // version
    run_gcov_dump_test(gcov_dump_binary, gcda_file, "-l", 1);      // dump contents
    run_gcov_dump_test(gcov_dump_binary, gcda_file, "-p", 1);      // dump positions
    run_gcov_dump_test(gcov_dump_binary, gcda_file, "-r", 1);      // dump raw
    run_gcov_dump_test(gcov_dump_binary, gcda_file, "-s", 1);      // dump stable
    
    // Test combined flags (space-separated)
    printf("\nTesting combined flags (space-separated):\n");
    run_gcov_dump_test(gcov_dump_binary, gcda_file, "-l -p -r -s", 1);
    
    // Test concatenated flags
    printf("\nTesting concatenated flags:\n");
    run_gcov_dump_test(gcov_dump_binary, gcda_file, "-lprs", 1);
    
    // Test invalid flag (to trigger default case)
    printf("\nTesting invalid flag (should trigger default case):\n");
    run_gcov_dump_test(gcov_dump_binary, gcda_file, "-x", 0);      // invalid flag
    
    // Test various combinations
    printf("\nTesting additional combinations:\n");
    run_gcov_dump_test(gcov_dump_binary, gcda_file, "-lp", 1);
    run_gcov_dump_test(gcov_dump_binary, gcda_file, "-rs", 1);
    run_gcov_dump_test(gcov_dump_binary, gcda_file, "-l -s", 1);
    
    // Step 4: Generate final coverage report
    printf("\n4. Generating final coverage report...\n");
    
    // First, ensure all coverage data is merged
    system("gcov -i gcov-dump.cc 2>/dev/null || true");
    
    // Generate human-readable coverage report
    char cmd[MAX_PATH * 4];
    snprintf(cmd, sizeof(cmd), "gcov -b %s/gcov-dump.cc", gcov_dump_source_dir);
    execute_command(cmd, 1);
    
    // Check if gcov-dump.cc.gcov exists and display relevant lines
    printf("\n5. Checking coverage of target lines (111-130)...\n");
    
    FILE *gcov_report = fopen("gcov-dump.cc.gcov", "r");
    if (gcov_report) {
        char line[1024];
        int line_num = 0;
        int target_lines_covered = 0;
        int target_lines_total = 0;
        
        while (fgets(line, sizeof(line), gcov_report)) {
            // Parse gcov format: "    #####:   11:case 'h':"
            // or "        1:   11:case 'h':"
            if (strlen(line) > 10) {
                int count = 0;
                char *colon1 = strchr(line, ':');
                if (colon1) {
                    char *colon2 = strchr(colon1 + 1, ':');
                    if (colon2) {
                        // Extract line number
                        line_num = atoi(colon1 + 1);
                        
                        // Check if this is one of our target lines
                        if (line_num >= 111 && line_num <= 130) {
                            target_lines_total++;
                            
                            // Check if line was executed
                            if (strstr(line, "#####") == NULL) {
                                // Line was executed at least once
                                target_lines_covered++;
                                
                                // Print covered line
                                printf("Covered: %s", colon2 + 1);
                            } else {
                                // Line not covered
                                printf("NOT covered: %s", colon2 + 1);
                            }
                        }
                    }
                }
            }
        }
        fclose(gcov_report);
        
        printf("\nCoverage Summary for lines 111-130:\n");
        printf("  Lines covered: %d/%d\n", target_lines_covered, target_lines_total);
        printf("  Coverage: %.1f%%\n", 
               target_lines_total > 0 ? (100.0 * target_lines_covered / target_lines_total) : 0.0);
        
        if (target_lines_covered == target_lines_total) {
            printf("\nSUCCESS: All target lines covered!\n");
        } else {
            printf("\nPARTIAL: Some target lines not covered\n");
        }
    } else {
        printf("Could not open coverage report\n");
    }
    
    // Step 5: Cleanup
    printf("\n6. Cleaning up...\n");
    system("rm -f dummy_prog dummy.gcda dummy.gcno dummy.c");
    system("rm -rf coverage_data");
    
    printf("\n=== Test completed ===\n");
    return 0;
}
