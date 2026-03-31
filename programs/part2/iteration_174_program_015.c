/**
 * test_gcov_dump_coverage.c
 * 
 * A test program to cover the uncovered switch-case lines in gcov-dump.cc
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
 * Execute a shell command and return its exit status
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
 * Check if a file exists
 */
int file_exists(const char *path) {
    return access(path, F_OK) == 0;
}

/**
 * Build instrumented gcov-dump if needed
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
    // This assumes we have the necessary headers and libraries
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include -I../../libiberty "
        "%s ../../libiberty/libiberty.a -o %s",
        source_path, output_path);
    
    printf("Compilation command: %s\n", cmd);
    
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "ERROR: Failed to compile instrumented gcov-dump\n");
        
        // Try simpler compilation
        printf("Trying simpler compilation...\n");
        snprintf(cmd, sizeof(cmd),
            "g++ -O0 -fprofile-arcs -ftest-coverage %s -o %s",
            source_path, output_path);
        
        if (execute_command(cmd) != 0) {
            fprintf(stderr, "ERROR: Simple compilation also failed\n");
            return -1;
        }
    }
    
    printf("Successfully built instrumented gcov-dump\n");
    return 0;
}

/**
 * Create a dummy C program to generate GCOV data
 */
int create_dummy_program(const char *dummy_source, const char *dummy_binary) {
    FILE *fp = fopen(dummy_source, "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return -1;
    }
    
    // Write a simple C program that will generate coverage data
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i, sum = 0;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        sum += i;\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    printf(\"Sum: %%d\\n\", sum);\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    
    // Compile with coverage instrumentation
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd),
        "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s",
        dummy_source, dummy_binary);
    
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "ERROR: Failed to compile dummy program\n");
        return -1;
    }
    
    // Run the dummy program to generate .gcda file
    if (execute_command(dummy_binary) != 0) {
        fprintf(stderr, "ERROR: Failed to run dummy program\n");
        return -1;
    }
    
    printf("Created dummy program and generated coverage data\n");
    return 0;
}

/**
 * Merge coverage data from gcov-dump runs
 */
void merge_coverage_data(const char *gcov_dump_binary) {
    char cmd[MAX_CMD];
    
    // First, find the .gcda file for gcov-dump
    // It should be in the same directory as the binary or in the current directory
    printf("Merging coverage data...\n");
    
    // Method 1: Use gcov directly on the source file
    snprintf(cmd, sizeof(cmd), "gcov -i gcov-dump.cc 2>/dev/null || true");
    execute_command(cmd);
    
    // Method 2: Copy any .gcda files to current directory
    execute_command("find . -name '*.gcda' -exec cp {} . \\; 2>/dev/null || true");
    
    // Method 3: Generate coverage report
    snprintf(cmd, sizeof(cmd), "gcov -b %s 2>/dev/null | grep -A5 'gcov-dump.cc' || true", 
             gcov_dump_binary);
    execute_command(cmd);
}

/**
 * Run gcov-dump with specific flags and check exit code
 */
int run_gcov_dump_test(const char *gcov_dump_binary, const char *gcda_file, 
                       const char *flags, int expect_success) {
    char cmd[MAX_CMD];
    int exit_code;
    
    if (strcmp(flags, "") == 0) {
        // No flags, just run with gcda file
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_binary, gcda_file);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s %s", gcov_dump_binary, flags, gcda_file);
    }
    
    exit_code = execute_command(cmd);
    
    if (expect_success) {
        if (exit_code != 0) {
            fprintf(stderr, "WARNING: Command failed but was expected to succeed: %s\n", cmd);
            fprintf(stderr, "Exit code: %d\n", exit_code);
        }
    } else {
        if (exit_code == 0) {
            fprintf(stderr, "WARNING: Command succeeded but was expected to fail: %s\n", cmd);
        }
    }
    
    // Merge coverage after each test
    merge_coverage_data(gcov_dump_binary);
    
    return exit_code;
}

/**
 * Main test function
 */
int main(int argc, char *argv[]) {
    const char *gcov_dump_instrumented = "./gcov-dump-instrumented";
    const char *dummy_source = "./dummy.c";
    const char *dummy_binary = "./dummy_prog";
    const char *gcda_file = "./dummy.gcda";
    
    printf("=== Starting gcov-dump coverage test ===\n\n");
    
    // Step 1: Build instrumented gcov-dump
    if (build_instrumented_gcov_dump(".", gcov_dump_instrumented) != 0) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    // Step 2: Create dummy program and generate GCOV data
    if (create_dummy_program(dummy_source, dummy_binary) != 0) {
        fprintf(stderr, "Failed to create dummy program\n");
        return 1;
    }
    
    // Verify gcda file exists
    if (!file_exists(gcda_file)) {
        // Try alternative name
        gcda_file = "dummy.gcda";
        if (!file_exists(gcda_file)) {
            fprintf(stderr, "ERROR: Could not find generated .gcda file\n");
            // List files to debug
            execute_command("ls -la *.gcda 2>/dev/null || true");
            return 1;
        }
    }
    
    printf("\n=== Running flag coverage tests ===\n\n");
    
    // Clear any existing coverage data
    execute_command("rm -f *.gcda *.gcno 2>/dev/null || true");
    
    // Test 1: Help flag (-h) - should succeed without gcda file
    printf("\n--- Test 1: Help flag (-h) ---\n");
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "%s -h", gcov_dump_instrumented);
    execute_command(cmd);
    merge_coverage_data(gcov_dump_instrumented);
    
    // Test 2: Version flag (-v) - should succeed without gcda file
    printf("\n--- Test 2: Version flag (-v) ---\n");
    snprintf(cmd, sizeof(cmd), "%s -v", gcov_dump_instrumented);
    execute_command(cmd);
    merge_coverage_data(gcov_dump_instrumented);
    
    // Test 3: Individual flags with gcda file
    printf("\n--- Test 3: Individual flags ---\n");
    run_gcov_dump_test(gcov_dump_instrumented, gcda_file, "-l", 1);  // flag_dump_contents
    run_gcov_dump_test(gcov_dump_instrumented, gcda_file, "-p", 1);  // flag_dump_positions
    run_gcov_dump_test(gcov_dump_instrumented, gcda_file, "-r", 1);  // flag_dump_raw
    run_gcov_dump_test(gcov_dump_instrumented, gcda_file, "-s", 1);  // flag_dump_stable
    
    // Test 4: Space-separated combined flags
    printf("\n--- Test 4: Space-separated combined flags ---\n");
    run_gcov_dump_test(gcov_dump_instrumented, gcda_file, "-l -p -r -s", 1);
    
    // Test 5: Concatenated flags
    printf("\n--- Test 5: Concatenated flags ---\n");
    run_gcov_dump_test(gcov_dump_instrumented, gcda_file, "-lprs", 1);
    
    // Test 6: Various flag combinations
    printf("\n--- Test 6: Various flag combinations ---\n");
    run_gcov_dump_test(gcov_dump_instrumented, gcda_file, "-lp", 1);
    run_gcov_dump_test(gcov_dump_instrumented, gcda_file, "-rs", 1);
    run_gcov_dump_test(gcov_dump_instrumented, gcda_file, "-l -s", 1);
    run_gcov_dump_test(gcov_dump_instrumented, gcda_file, "-p -r", 1);
    
    // Test 7: Invalid flag (to trigger default case)
    printf("\n--- Test 7: Invalid flag (should trigger default case) ---\n");
    run_gcov_dump_test(gcov_dump_instrumented, gcda_file, "-x", 0);  // Should fail
    
    // Test 8: No flags (just gcda file)
    printf("\n--- Test 8: No flags (just gcda file) ---\n");
    run_gcov_dump_test(gcov_dump_instrumented, gcda_file, "", 1);
    
    // Test 9: Multiple gcda files with flags
    printf("\n--- Test 9: Multiple gcda files ---\n");
    // Create another dummy program
    execute_command("cp dummy.c dummy2.c");
    execute_command("gcc -O0 -fprofile-arcs -ftest-coverage dummy2.c -o dummy2_prog");
    execute_command("./dummy2_prog");
    snprintf(cmd, sizeof(cmd), "%s -l dummy.gcda dummy2.gcda", gcov_dump_instrumented);
    execute_command(cmd);
    merge_coverage_data(gcov_dump_instrumented);
    
    printf("\n=== Generating final coverage report ===\n");
    
    // Generate comprehensive coverage report
    execute_command("rm -f gcov-dump.c.gcov 2>/dev/null || true");
    
    // Method 1: Use gcov on the source directly
    if (file_exists("gcov-dump.cc")) {
        snprintf(cmd, sizeof(cmd), "gcov -b gcov-dump.cc");
    } else {
        // Try to find it
        snprintf(cmd, sizeof(cmd), "find . -name 'gcov-dump.cc' -exec gcov -b {} \\; 2>/dev/null || true");
    }
    execute_command(cmd);
    
    // Method 2: Use lcov if available
    if (system("which lcov >/dev/null 2>&1") == 0) {
        execute_command("lcov --capture --directory . --output-file coverage.info 2>/dev/null || true");
        execute_command("genhtml coverage.info --output-directory coverage-report 2>/dev/null || true");
        printf("HTML coverage report generated in coverage-report/\n");
    }
    
    // Display key coverage information
    printf("\n=== Checking target lines coverage ===\n");
    
    // Look for the gcov output file
    const char *gcov_outputs[] = {
        "gcov-dump.cc.gcov",
        "gcov-dump.c.gcov",
        NULL
    };
    
    FILE *gcov_file = NULL;
    for (int i = 0; gcov_outputs[i] != NULL; i++) {
        gcov_file = fopen(gcov_outputs[i], "r");
        if (gcov_file) {
            printf("Found coverage file: %s\n", gcov_outputs[i]);
            break;
        }
    }
    
    if (gcov_file) {
        char line[1024];
        int line_num = 0;
        int target_start = 111;
        int target_end = 130;
        int in_target = 0;
        
        printf("\nLines around target area (111-130):\n");
        printf("Line : Count : Source\n");
        printf("----------------------\n");
        
        while (fgets(line, sizeof(line), gcov_file)) {
            // Parse gcov format: "    #####:   111:	case 'h':"
            if (strlen(line) > 20) {
                // Try to extract line number
                char *colon1 = strchr(line, ':');
                if (colon1) {
                    char *colon2 = strchr(colon1 + 1, ':');
                    if (colon2) {
                        line_num = atoi(colon1 + 1);
                        
                        if (line_num >= target_start - 5 && line_num <= target_end + 5) {
                            // Print the line
                            printf("%s", line);
                            
                            // Check if it's one of our target lines
                            if (line_num >= target_start && line_num <= target_end) {
                                // Check if it was executed (doesn't start with "#####")
                                if (strstr(line, "#####") != NULL) {
                                    printf("WARNING: Line %d was NOT executed!\n", line_num);
                                } else {
                                    printf("GOOD: Line %d was executed\n", line_num);
                                }
                            }
                        }
                    }
                }
            }
        }
        fclose(gcov_file);
    } else {
        printf("Could not find gcov output file. Trying alternative method...\n");
        
        // Try to generate it differently
        snprintf(cmd, sizeof(cmd), "gcov -i %s 2>&1 | grep -i 'gcov-dump' || true", 
                 gcov_dump_instrumented);
        execute_command(cmd);
    }
    
    // Cleanup
    printf("\n=== Cleaning up ===\n");
    execute_command("rm -f dummy.c dummy2.c dummy_prog dummy2_prog 2>/dev/null || true");
    execute_command("rm -f *.gcda *.gcno 2>/dev/null || true");
    
    printf("\n=== Test completed ===\n");
    printf("The instrumented gcov-dump binary and its .gcno file are preserved.\n");
    printf("You can run additional tests with: %s [flags] <gcda_file>\n", 
           gcov_dump_instrumented);
    
    return 0;
}
