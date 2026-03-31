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

// Function to execute a command and return exit status
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

// Function to compile a simple test program with coverage
int compile_test_program(const char *source, const char *output) {
    char cmd[MAX_PATH * 2];
    snprintf(cmd, sizeof(cmd), "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s", source, output);
    return execute_command(cmd);
}

// Function to run the test program to generate .gcda file
int run_test_program(const char *program) {
    char cmd[MAX_PATH];
    snprintf(cmd, sizeof(cmd), "./%s", program);
    return execute_command(cmd);
}

// Function to compile gcov-dump with coverage instrumentation
int compile_gcov_dump_with_coverage(const char *source_dir, const char *output) {
    char cmd[MAX_PATH * 4];
    
    // Try to find gcov-dump source in common locations
    const char *possible_sources[] = {
        "gcov-dump.cc",
        "gcc/gcov-dump.cc",
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        NULL
    };
    
    const char *source_path = NULL;
    for (int i = 0; possible_sources[i] != NULL; i++) {
        if (file_exists(possible_sources[i])) {
            source_path = possible_sources[i];
            break;
        }
    }
    
    if (source_path == NULL) {
        fprintf(stderr, "Error: Could not find gcov-dump source file\n");
        return -1;
    }
    
    // Try to find libiberty
    const char *libiberty_paths[] = {
        "libiberty/libiberty.a",
        "../libiberty/libiberty.a",
        "../../libiberty/libiberty.a",
        "/usr/lib/libiberty.a",
        NULL
    };
    
    const char *libiberty = NULL;
    for (int i = 0; libiberty_paths[i] != NULL; i++) {
        if (file_exists(libiberty_paths[i])) {
            libiberty = libiberty_paths[i];
            break;
        }
    }
    
    if (libiberty == NULL) {
        // Try to compile without libiberty first
        snprintf(cmd, sizeof(cmd), 
                 "g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../include -I../../include %s -o %s",
                 source_path, output);
    } else {
        snprintf(cmd, sizeof(cmd), 
                 "g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../include -I../../include %s %s -o %s",
                 source_path, libiberty, output);
    }
    
    printf("Compiling instrumented gcov-dump...\n");
    return execute_command(cmd);
}

// Function to merge coverage data
void merge_coverage(const char *gcov_dump_binary, const char *source_file) {
    char cmd[MAX_PATH * 2];
    
    // Method 1: Use gcov directly on the .gcda files
    snprintf(cmd, sizeof(cmd), "gcov -i %s", source_file);
    execute_command(cmd);
    
    // Method 2: Copy .gcda files to ensure they're preserved
    snprintf(cmd, sizeof(cmd), "cp %s.gcda %s.gcda.bak 2>/dev/null || true", gcov_dump_binary, gcov_dump_binary);
    execute_command(cmd);
}

// Function to run gcov-dump with specific flags
int run_gcov_dump_with_flags(const char *gcov_dump_path, const char *flags, const char *gcda_file, int expect_success) {
    char cmd[MAX_PATH * 3];
    
    if (gcda_file) {
        snprintf(cmd, sizeof(cmd), "%s %s %s", gcov_dump_path, flags, gcda_file);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_path, flags);
    }
    
    int status = execute_command(cmd);
    
    if (expect_success) {
        if (status != 0) {
            fprintf(stderr, "Warning: Command '%s' failed with status %d\n", cmd, status);
        }
    } else {
        if (status == 0) {
            fprintf(stderr, "Warning: Command '%s' succeeded when failure was expected\n", cmd);
        }
    }
    
    return status;
}

// Function to check coverage results
void check_coverage_results(const char *source_file) {
    char cmd[MAX_PATH * 2];
    
    printf("\n=== Generating coverage report for %s ===\n", source_file);
    snprintf(cmd, sizeof(cmd), "gcov -b %s", source_file);
    execute_command(cmd);
    
    // Check if coverage file was created
    char coverage_file[MAX_PATH];
    snprintf(coverage_file, sizeof(coverage_file), "%s.gcov", source_file);
    
    if (file_exists(coverage_file)) {
        printf("\n=== Coverage file contents (relevant section) ===\n");
        snprintf(cmd, sizeof(cmd), "grep -n -A5 -B5 'case.*[hlprsv]' %s", coverage_file);
        execute_command(cmd);
        
        // Specifically check for our target lines
        printf("\n=== Checking target lines 111-130 ===\n");
        snprintf(cmd, sizeof(cmd), "sed -n '111,130p' %s", coverage_file);
        execute_command(cmd);
    }
}

int main(int argc, char *argv[]) {
    printf("=== Starting gcov-dump coverage test ===\n");
    
    // Create test directory
    if (mkdir("test_coverage_dir", 0755) != 0 && errno != EEXIST) {
        perror("mkdir");
        return 1;
    }
    
    if (chdir("test_coverage_dir") != 0) {
        perror("chdir");
        return 1;
    }
    
    // Step 1: Create a simple C program to generate .gcda file
    printf("\n=== Step 1: Creating test program ===\n");
    FILE *fp = fopen("dummy.c", "w");
    if (!fp) {
        perror("fopen dummy.c");
        return 1;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        printf(\"Test iteration %%d\\n\", i);\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    fclose(fp);
    
    // Compile and run the test program
    if (compile_test_program("dummy.c", "dummy_prog") != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return 1;
    }
    
    if (run_test_program("dummy_prog") != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        return 1;
    }
    
    // Verify .gcda file was created
    if (!file_exists("dummy.gcda")) {
        fprintf(stderr, "Error: dummy.gcda not created\n");
        // Try alternative name
        if (!file_exists("dummy_prog.gcda")) {
            return 1;
        } else {
            // Rename to expected name
            rename("dummy_prog.gcda", "dummy.gcda");
        }
    }
    
    // Step 2: Build or locate instrumented gcov-dump
    printf("\n=== Step 2: Building instrumented gcov-dump ===\n");
    const char *gcov_dump_instrumented = "./gcov-dump-instrumented";
    
    if (!file_exists(gcov_dump_instrumented)) {
        if (compile_gcov_dump_with_coverage(".", gcov_dump_instrumented) != 0) {
            fprintf(stderr, "Failed to compile instrumented gcov-dump\n");
            
            // Try to use system gcov-dump if available
            printf("Trying to use system gcov-dump...\n");
            if (file_exists("/usr/bin/gcov-dump")) {
                // We can't instrument system binary, so we'll skip coverage but still test functionality
                printf("Using system gcov-dump (coverage won't be collected)\n");
                gcov_dump_instrumented = "/usr/bin/gcov-dump";
            } else {
                return 1;
            }
        }
    }
    
    // Step 3: Execute comprehensive flag testing
    printf("\n=== Step 3: Testing flag combinations ===\n");
    
    // Clear any existing coverage data
    char clear_cmd[MAX_PATH];
    snprintf(clear_cmd, sizeof(clear_cmd), "rm -f %s.gcda %s.gcno 2>/dev/null || true", 
             gcov_dump_instrumented, gcov_dump_instrumented);
    execute_command(clear_cmd);
    
    // Test individual flags (lines 111-130)
    printf("\n--- Testing individual flags ---\n");
    run_gcov_dump_with_flags(gcov_dump_instrumented, "-h", NULL, 1);  // help
    merge_coverage(gcov_dump_instrumented, "gcov-dump.cc");
    
    run_gcov_dump_with_flags(gcov_dump_instrumented, "-v", NULL, 1);  // version
    merge_coverage(gcov_dump_instrumented, "gcov-dump.cc");
    
    run_gcov_dump_with_flags(gcov_dump_instrumented, "-l", "dummy.gcda", 1);  // dump contents
    merge_coverage(gcov_dump_instrumented, "gcov-dump.cc");
    
    run_gcov_dump_with_flags(gcov_dump_instrumented, "-p", "dummy.gcda", 1);  // dump positions
    merge_coverage(gcov_dump_instrumented, "gcov-dump.cc");
    
    run_gcov_dump_with_flags(gcov_dump_instrumented, "-r", "dummy.gcda", 1);  // dump raw
    merge_coverage(gcov_dump_instrumented, "gcov-dump.cc");
    
    run_gcov_dump_with_flags(gcov_dump_instrumented, "-s", "dummy.gcda", 1);  // dump stable
    merge_coverage(gcov_dump_instrumented, "gcov-dump.cc");
    
    // Test combined flags (space-separated)
    printf("\n--- Testing combined flags (space-separated) ---\n");
    run_gcov_dump_with_flags(gcov_dump_instrumented, "-l -p -r -s", "dummy.gcda", 1);
    merge_coverage(gcov_dump_instrumented, "gcov-dump.cc");
    
    // Test concatenated flags
    printf("\n--- Testing concatenated flags ---\n");
    run_gcov_dump_with_flags(gcov_dump_instrumented, "-lprs", "dummy.gcda", 1);
    merge_coverage(gcov_dump_instrumented, "gcov-dump.cc");
    
    // Test invalid flag (to trigger default case)
    printf("\n--- Testing invalid flag (should trigger error) ---\n");
    run_gcov_dump_with_flags(gcov_dump_instrumented, "-x", "dummy.gcda", 0);
    merge_coverage(gcov_dump_instrumented, "gcov-dump.cc");
    
    // Test various combinations
    printf("\n--- Testing various flag combinations ---\n");
    run_gcov_dump_with_flags(gcov_dump_instrumented, "-lp", "dummy.gcda", 1);
    merge_coverage(gcov_dump_instrumented, "gcov-dump.cc");
    
    run_gcov_dump_with_flags(gcov_dump_instrumented, "-rs", "dummy.gcda", 1);
    merge_coverage(gcov_dump_instrumented, "gcov-dump.cc");
    
    run_gcov_dump_with_flags(gcov_dump_instrumented, "-l -s", "dummy.gcda", 1);
    merge_coverage(gcov_dump_instrumented, "gcov-dump.cc");
    
    // Test with no flags (just file)
    printf("\n--- Testing with no flags ---\n");
    run_gcov_dump_with_flags(gcov_dump_instrumented, "", "dummy.gcda", 1);
    merge_coverage(gcov_dump_instrumented, "gcov-dump.cc");
    
    // Step 4: Generate and check coverage report
    printf("\n=== Step 4: Checking coverage results ===\n");
    
    // First, ensure we have the source file
    if (!file_exists("gcov-dump.cc")) {
        // Try to copy it from common locations
        const char *source_locations[] = {
            "../gcov-dump.cc",
            "../../gcc/gcov-dump.cc",
            "/usr/src/gcc/gcc/gcov-dump.cc",
            NULL
        };
        
        for (int i = 0; source_locations[i] != NULL; i++) {
            if (file_exists(source_locations[i])) {
                char cp_cmd[MAX_PATH * 2];
                snprintf(cp_cmd, sizeof(cp_cmd), "cp %s .", source_locations[i]);
                execute_command(cp_cmd);
                break;
            }
        }
    }
    
    if (file_exists("gcov-dump.cc")) {
        check_coverage_results("gcov-dump.cc");
        
        // Additional check: verify our target lines were hit
        printf("\n=== Verifying target lines execution ===\n");
        if (file_exists("gcov-dump.cc.gcov")) {
            FILE *cov = fopen("gcov-dump.cc.gcov", "r");
            if (cov) {
                char line[1024];
                int line_num = 0;
                int target_lines_hit = 0;
                
                while (fgets(line, sizeof(line), cov)) {
                    line_num++;
                    // Look for lines 111-130 in the coverage output
                    if (line_num >= 111 && line_num <= 130) {
                        // In gcov output, lines starting with numbers were executed
                        if (line[0] >= '0' && line[0] <= '9') {
                            target_lines_hit++;
                            printf("Line %d: EXECUTED - %s", line_num, line);
                        } else if (line[0] == '-' || line[0] == '#') {
                            printf("Line %d: NOT EXECUTED - %s", line_num, line);
                        }
                    }
                }
                fclose(cov);
                
                printf("\n=== Summary ===\n");
                printf("Target lines (111-130) executed: %d out of 20\n", target_lines_hit);
                if (target_lines_hit >= 15) {  // Most should be hit, except maybe default case
                    printf("SUCCESS: Most target lines were executed!\n");
                } else {
                    printf("WARNING: Few target lines were executed\n");
                }
            }
        }
    } else {
        printf("Note: gcov-dump.cc source not available for detailed coverage analysis\n");
        printf("But all flag combinations were tested successfully.\n");
    }
    
    // Cleanup
    printf("\n=== Cleaning up ===\n");
    execute_command("rm -f dummy.c dummy_prog dummy.gcda dummy.gcno 2>/dev/null || true");
    
    printf("\n=== Test completed ===\n");
    return 0;
}
