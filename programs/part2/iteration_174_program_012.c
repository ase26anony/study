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
 * Build the instrumented gcov-dump binary
 */
int build_instrumented_gcov_dump(const char *source_dir, const char *output_path) {
    char cmd[MAX_CMD];
    
    // Check if we have the source file
    char source_path[MAX_PATH];
    snprintf(source_path, sizeof(source_path), "%s/gcov-dump.cc", source_dir);
    
    if (!file_exists(source_path)) {
        fprintf(stderr, "Error: gcov-dump.cc not found at %s\n", source_path);
        return 0;
    }
    
    // Try to find libiberty
    char libiberty_path[MAX_PATH] = "";
    const char *possible_paths[] = {
        "../../libiberty/libiberty.a",
        "../libiberty/libiberty.a",
        "libiberty/libiberty.a",
        "/usr/lib/libiberty.a",
        NULL
    };
    
    for (int i = 0; possible_paths[i] != NULL; i++) {
        if (file_exists(possible_paths[i])) {
            strncpy(liberty_path, possible_paths[i], sizeof(liberty_path)-1);
            break;
        }
    }
    
    if (strlen(libiberty_path) == 0) {
        fprintf(stderr, "Warning: libiberty.a not found, trying without it\n");
        snprintf(cmd, sizeof(cmd),
                 "g++ -O0 -fprofile-arcs -ftest-coverage -I%s -I%s/../../include "
                 "-I%s/../../libiberty %s -o %s",
                 source_dir, source_dir, source_dir, source_path, output_path);
    } else {
        snprintf(cmd, sizeof(cmd),
                 "g++ -O0 -fprofile-arcs -ftest-coverage -I%s -I%s/../../include "
                 "-I%s/../../libiberty %s %s -o %s",
                 source_dir, source_dir, source_dir, source_path, libiberty_path, output_path);
    }
    
    return execute_command(cmd) == 0;
}

/**
 * Create a dummy C program to generate GCOV data
 */
int create_dummy_program(const char *output_path) {
    FILE *f = fopen(output_path, "w");
    if (!f) {
        perror("Failed to create dummy.c");
        return 0;
    }
    
    fprintf(f, "/* dummy.c - Simple program to generate GCOV data */\n");
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main() {\n");
    fprintf(f, "    int i;\n");
    fprintf(f, "    for (i = 0; i < 10; i++) {\n");
    fprintf(f, "        printf(\"Iteration %%d\\n\", i);\n");
    fprintf(f, "    }\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
    return 1;
}

/**
 * Generate GCOV data file from dummy program
 */
int generate_gcov_data(const char *dummy_source, const char *gcda_file) {
    char cmd[MAX_CMD];
    
    // Compile dummy program with coverage
    snprintf(cmd, sizeof(cmd),
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o dummy_prog",
             dummy_source);
    
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return 0;
    }
    
    // Run the dummy program to generate gcda file
    if (execute_command("./dummy_prog") != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        return 0;
    }
    
    // The gcda file will be named based on the source file
    // Let's find and copy it to our desired location
    char source_gcda[MAX_PATH];
    char *base = strrchr(dummy_source, '/');
    if (base) {
        base++; // Skip '/'
    } else {
        base = (char *)dummy_source;
    }
    
    // Remove .c extension if present
    char base_name[MAX_PATH];
    strncpy(base_name, base, sizeof(base_name)-1);
    char *dot = strrchr(base_name, '.');
    if (dot && strcmp(dot, ".c") == 0) {
        *dot = '\0';
    }
    
    snprintf(source_gcda, sizeof(source_gcda), "%s.gcda", base_name);
    
    if (file_exists(source_gcda)) {
        snprintf(cmd, sizeof(cmd), "cp %s %s", source_gcda, gcda_file);
        return execute_command(cmd) == 0;
    }
    
    return 0;
}

/**
 * Merge coverage data after each gcov-dump invocation
 */
void merge_coverage_data(const char *gcov_dump_binary, const char *source_file) {
    char cmd[MAX_CMD];
    
    // First, find the gcda file for gcov-dump
    // It should be named gcov-dump.gcda or similar
    char *binary_name = strrchr(gcov_dump_binary, '/');
    if (binary_name) {
        binary_name++; // Skip '/'
    } else {
        binary_name = (char *)gcov_dump_binary;
    }
    
    char gcda_pattern[MAX_PATH];
    snprintf(gcda_pattern, sizeof(gcda_pattern), "%s*.gcda", binary_name);
    
    // Use gcov to merge coverage
    snprintf(cmd, sizeof(cmd), "gcov -i %s 2>/dev/null", source_file);
    execute_command(cmd);
}

/**
 * Run gcov-dump with specific flags and merge coverage
 */
void test_with_flags(const char *gcov_dump_binary, const char *gcda_file, 
                     const char *flags, int expect_success, 
                     const char *source_file) {
    char cmd[MAX_CMD];
    
    if (strcmp(flags, "-h") == 0 || strcmp(flags, "-v") == 0) {
        // Help and version don't need a gcda file
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_binary, flags);
    } else if (strcmp(flags, "-x") == 0) {
        // Invalid flag - should fail
        snprintf(cmd, sizeof(cmd), "%s %s %s 2>&1", gcov_dump_binary, flags, gcda_file);
    } else {
        // Regular flags need gcda file
        snprintf(cmd, sizeof(cmd), "%s %s %s", gcov_dump_binary, flags, gcda_file);
    }
    
    int result = execute_command(cmd);
    
    if (expect_success) {
        if (result != 0) {
            fprintf(stderr, "Warning: Command failed but was expected to succeed: %s\n", cmd);
        }
    } else {
        if (result == 0) {
            fprintf(stderr, "Warning: Command succeeded but was expected to fail: %s\n", cmd);
        }
    }
    
    // Merge coverage data
    merge_coverage_data(gcov_dump_binary, source_file);
}

/**
 * Generate final coverage report and check target lines
 */
void check_coverage(const char *source_file) {
    char cmd[MAX_CMD];
    
    printf("\n=== Generating Coverage Report ===\n");
    
    // Generate human-readable coverage report
    snprintf(cmd, sizeof(cmd), "gcov -b %s", source_file);
    execute_command(cmd);
    
    // The coverage data will be in gcov-dump.cc.gcov
    char gcov_file[MAX_PATH];
    snprintf(gcov_file, sizeof(gcov_file), "%s.gcov", source_file);
    
    if (file_exists(gcov_file)) {
        printf("\n=== Checking Coverage for Target Lines (111-130) ===\n");
        
        FILE *f = fopen(gcov_file, "r");
        if (f) {
            char line[1024];
            int in_target_range = 0;
            int target_lines_covered = 0;
            int target_lines_total = 0;
            
            while (fgets(line, sizeof(line), f)) {
                // Parse gcov format: "    #####:   11:  case 'h':"
                // or "        1:   11:  case 'h':"
                if (strlen(line) > 10) {
                    int line_num;
                    char coverage[10];
                    
                    if (sscanf(line, "%9[^:]:%d:", coverage, &line_num) == 2) {
                        if (line_num >= 111 && line_num <= 130) {
                            target_lines_total++;
                            
                            // Check if line is covered (not starting with "#####")
                            if (strstr(coverage, "#####") == NULL) {
                                target_lines_covered++;
                                printf("Line %d: COVERED - %s", line_num, line + 20);
                            } else {
                                printf("Line %d: NOT COVERED - %s", line_num, line + 20);
                            }
                        }
                    }
                }
            }
            
            fclose(f);
            
            printf("\n=== Coverage Summary for Lines 111-130 ===\n");
            printf("Covered: %d/%d lines (%.1f%%)\n", 
                   target_lines_covered, target_lines_total,
                   target_lines_total > 0 ? 
                   (100.0 * target_lines_covered / target_lines_total) : 0.0);
            
            if (target_lines_covered == target_lines_total) {
                printf("SUCCESS: All target lines are covered!\n");
            } else {
                printf("WARNING: Not all target lines are covered\n");
            }
        }
    }
}

int main(int argc, char *argv[]) {
    char cwd[MAX_PATH];
    char gcov_dump_binary[MAX_PATH];
    char dummy_source[MAX_PATH];
    char gcda_file[MAX_PATH];
    char source_file[MAX_PATH];
    
    // Get current directory
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed");
        return 1;
    }
    
    // Set paths
    snprintf(gcov_dump_binary, sizeof(gcov_dump_binary), "%s/gcov-dump-instrumented", cwd);
    snprintf(dummy_source, sizeof(dummy_source), "%s/dummy.c", cwd);
    snprintf(gcda_file, sizeof(gcda_file), "%s/test.gcda", cwd);
    
    // Try to find gcov-dump source
    const char *possible_source_dirs[] = {
        ".",
        "..",
        "../gcc",
        "../../gcc",
        NULL
    };
    
    int found_source = 0;
    for (int i = 0; possible_source_dirs[i] != NULL; i++) {
        snprintf(source_file, sizeof(source_file), "%s/gcov-dump.cc", possible_source_dirs[i]);
        if (file_exists(source_file)) {
            found_source = 1;
            break;
        }
    }
    
    if (!found_source) {
        fprintf(stderr, "Error: Could not find gcov-dump.cc\n");
        fprintf(stderr, "Please specify source directory as argument or place gcov-dump.cc in current directory\n");
        if (argc > 1) {
            snprintf(source_file, sizeof(source_file), "%s/gcov-dump.cc", argv[1]);
        } else {
            return 1;
        }
    }
    
    printf("=== Building Instrumented gcov-dump ===\n");
    
    // Extract directory from source_file path
    char source_dir[MAX_PATH];
    strncpy(source_dir, source_file, sizeof(source_dir)-1);
    char *last_slash = strrchr(source_dir, '/');
    if (last_slash) {
        *last_slash = '\0';
    } else {
        strcpy(source_dir, ".");
    }
    
    if (!build_instrumented_gcov_dump(source_dir, gcov_dump_binary)) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        
        // Try a simpler approach - maybe gcov-dump already exists
        if (file_exists("gcov-dump")) {
            printf("Using existing gcov-dump binary\n");
            strcpy(gcov_dump_binary, "gcov-dump");
            
            // Recompile it with coverage
            execute_command("g++ -O0 -fprofile-arcs -ftest-coverage gcov-dump.cc -o gcov-dump-instrumented 2>/dev/null");
            if (file_exists("gcov-dump-instrumented")) {
                strcpy(gcov_dump_binary, "gcov-dump-instrumented");
            }
        } else {
            return 1;
        }
    }
    
    printf("=== Creating Dummy Program ===\n");
    if (!create_dummy_program(dummy_source)) {
        return 1;
    }
    
    printf("=== Generating GCOV Test Data ===\n");
    if (!generate_gcov_data(dummy_source, gcda_file)) {
        fprintf(stderr, "Failed to generate GCOV data\n");
        return 1;
    }
    
    printf("=== Testing gcov-dump Flag Combinations ===\n");
    
    // Clear any existing coverage data
    execute_command("rm -f *.gcda *.gcno 2>/dev/null");
    
    // Test individual flags
    printf("\n--- Testing Help Flag (-h) ---\n");
    test_with_flags(gcov_dump_binary, gcda_file, "-h", 1, source_file);
    
    printf("\n--- Testing Version Flag (-v) ---\n");
    test_with_flags(gcov_dump_binary, gcda_file, "-v", 1, source_file);
    
    printf("\n--- Testing Contents Flag (-l) ---\n");
    test_with_flags(gcov_dump_binary, gcda_file, "-l", 1, source_file);
    
    printf("\n--- Testing Positions Flag (-p) ---\n");
    test_with_flags(gcov_dump_binary, gcda_file, "-p", 1, source_file);
    
    printf("\n--- Testing Raw Flag (-r) ---\n");
    test_with_flags(gcov_dump_binary, gcda_file, "-r", 1, source_file);
    
    printf("\n--- Testing Stable Flag (-s) ---\n");
    test_with_flags(gcov_dump_binary, gcda_file, "-s", 1, source_file);
    
    printf("\n--- Testing Combined Flags (space-separated) ---\n");
    test_with_flags(gcov_dump_binary, gcda_file, "-l -p -r -s", 1, source_file);
    
    printf("\n--- Testing Combined Flags (concatenated) ---\n");
    test_with_flags(gcov_dump_binary, gcda_file, "-lprs", 1, source_file);
    
    printf("\n--- Testing Invalid Flag (-x) ---\n");
    test_with_flags(gcov_dump_binary, gcda_file, "-x", 0, source_file);
    
    // Check final coverage
    check_coverage(source_file);
    
    // Cleanup
    printf("\n=== Cleaning Up ===\n");
    execute_command("rm -f dummy.c dummy_prog test.gcda 2>/dev/null");
    execute_command("rm -f *.gcov 2>/dev/null");
    
    printf("\n=== Test Complete ===\n");
    
    return 0;
}
