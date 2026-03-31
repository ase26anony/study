/**
 * test_gcov_dump_coverage.c
 * 
 * This program tests the uncovered lines in gcov-dump.cc (lines 111-130)
 * by executing gcov-dump with various command-line flags to ensure
 * coverage of the switch-case handling for dump flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024

/**
 * Build an instrumented version of gcov-dump with coverage enabled
 */
int build_instrumented_gcov_dump(const char *source_dir, const char *output_path) {
    char cmd[2048];
    int status;
    
    printf("Building instrumented gcov-dump...\n");
    
    // Check if we're in a GCC source tree
    struct stat st;
    char gcov_dump_cc[MAX_PATH];
    snprintf(gcov_dump_cc, sizeof(gcov_dump_cc), "%s/gcov-dump.cc", source_dir);
    
    if (stat(gcov_dump_cc, &st) != 0) {
        // Try to find gcov-dump.cc in common locations
        const char *locations[] = {
            ".",
            "..",
            "../gcc",
            "../../gcc",
            "gcc",
            NULL
        };
        
        for (int i = 0; locations[i] != NULL; i++) {
            snprintf(gcov_dump_cc, sizeof(gcov_dump_cc), "%s/gcov-dump.cc", locations[i]);
            if (stat(gcov_dump_cc, &st) == 0) {
                source_dir = locations[i];
                break;
            }
        }
    }
    
    // Build command to compile instrumented gcov-dump
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-I%s -I%s/../../include -I%s/../../libiberty "
        "%s/gcov-dump.cc %s/../../libiberty/libiberty.a "
        "-o %s",
        source_dir, source_dir, source_dir,
        source_dir, source_dir, output_path);
    
    printf("Compile command: %s\n", cmd);
    status = system(cmd);
    
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("Successfully built instrumented gcov-dump at %s\n", output_path);
        return 1;
    } else {
        printf("Failed to build instrumented gcov-dump. Trying alternative approach...\n");
        
        // Try simpler approach - just compile the file directly if we can find it
        snprintf(cmd, sizeof(cmd),
            "g++ -O0 -fprofile-arcs -ftest-coverage gcov-dump.cc -o %s",
            output_path);
        
        status = system(cmd);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Built instrumented gcov-dump using simple approach\n");
            return 1;
        }
        
        // Check if gcov-dump already exists in PATH
        snprintf(cmd, sizeof(cmd), "which gcov-dump");
        FILE *fp = popen(cmd, "r");
        if (fp) {
            char path[MAX_PATH];
            if (fgets(path, sizeof(path), fp)) {
                path[strcspn(path, "\n")] = 0;
                printf("Found existing gcov-dump at %s\n", path);
                
                // Copy it to our instrumented location
                snprintf(cmd, sizeof(cmd), "cp %s %s", path, output_path);
                if (system(cmd) == 0) {
                    printf("Copied existing gcov-dump to %s\n", output_path);
                    pclose(fp);
                    return 1;
                }
            }
            pclose(fp);
        }
        
        printf("ERROR: Could not build or find gcov-dump\n");
        return 0;
    }
}

/**
 * Create a dummy C program to generate GCOV data
 */
int create_dummy_program(const char *dummy_c_path, const char *dummy_exe_path) {
    FILE *fp = fopen(dummy_c_path, "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return 0;
    }
    
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
    
    // Compile with coverage
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
        "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s",
        dummy_c_path, dummy_exe_path);
    
    int status = system(cmd);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("Created dummy program at %s\n", dummy_exe_path);
        return 1;
    }
    
    printf("Failed to compile dummy program\n");
    return 0;
}

/**
 * Run the dummy program to generate .gcda file
 */
int generate_gcda_file(const char *dummy_exe_path) {
    printf("Running dummy program to generate .gcda file...\n");
    
    int status = system(dummy_exe_path);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        // Check if .gcda file was created
        if (access("dummy.gcda", F_OK) == 0) {
            printf("Generated dummy.gcda file\n");
            return 1;
        } else {
            // Try to find it in the same directory as the executable
            char gcda_path[MAX_PATH];
            snprintf(gcda_path, sizeof(gcda_path), "%s.gcda", dummy_exe_path);
            if (access(gcda_path, F_OK) == 0) {
                printf("Found .gcda at %s\n", gcda_path);
                // Copy it to current directory
                char cmd[1024];
                snprintf(cmd, sizeof(cmd), "cp %s ./dummy.gcda", gcda_path);
                system(cmd);
                return 1;
            }
        }
    }
    
    printf("Failed to generate .gcda file\n");
    return 0;
}

/**
 * Execute gcov-dump with given arguments and merge coverage
 */
int run_gcov_dump_test(const char *gcov_dump_path, const char *args, 
                       const char *gcda_file, int expect_success) {
    char cmd[2048];
    int status;
    
    // Build the command
    if (gcda_file && strlen(gcda_file) > 0) {
        snprintf(cmd, sizeof(cmd), "%s %s %s", gcov_dump_path, args, gcda_file);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_path, args);
    }
    
    printf("Running: %s\n", cmd);
    
    // Execute the command
    status = system(cmd);
    int exit_code = WEXITSTATUS(status);
    
    // Check if execution matched expectations
    if (expect_success) {
        if (exit_code != 0) {
            printf("WARNING: Command failed with exit code %d (expected success)\n", exit_code);
        }
    } else {
        if (exit_code == 0) {
            printf("WARNING: Command succeeded (expected failure)\n");
        }
    }
    
    // Merge coverage data
    printf("Merging coverage data...\n");
    
    // First, find the .gcda file for gcov-dump
    char gcda_pattern[MAX_PATH];
    snprintf(gcda_pattern, sizeof(gcda_pattern), "%s*.gcda", gcov_dump_path);
    
    // Use gcov to merge coverage
    snprintf(cmd, sizeof(cmd), "gcov -i gcov-dump.cc 2>/dev/null || true");
    system(cmd);
    
    // Alternative: just ensure .gcda files are in the right place
    snprintf(cmd, sizeof(cmd), "find . -name \"*.gcda\" -exec cp {} . \\; 2>/dev/null || true");
    system(cmd);
    
    return 1;
}

/**
 * Generate final coverage report and check target lines
 */
void check_coverage() {
    printf("\n=== Generating Coverage Report ===\n");
    
    // Generate coverage report
    system("gcov -b gcov-dump.cc 2>&1 | grep -A 20 -B 5 'Lines executed:'");
    
    // Specifically check for our target lines
    printf("\n=== Checking Target Lines (111-130) ===\n");
    
    // Read the .gcov file
    FILE *fp = fopen("gcov-dump.cc.gcov", "r");
    if (fp) {
        char line[1024];
        int in_target_range = 0;
        int target_lines_covered = 0;
        int target_lines_total = 0;
        
        while (fgets(line, sizeof(line), fp)) {
            // Parse gcov format: "    #####:   30:  case 'l':"
            int line_num, count;
            char tag[10];
            
            if (sscanf(line, "%s %d:", tag, &line_num) >= 2) {
                if (line_num >= 111 && line_num <= 130) {
                    target_lines_total++;
                    
                    // Check if line is executed (not starting with "#####")
                    if (strstr(tag, "#####") == NULL && strstr(tag, "-") == NULL) {
                        target_lines_covered++;
                        printf("Line %d: COVERED (%s)\n", line_num, tag);
                    } else {
                        printf("Line %d: NOT COVERED\n", line_num);
                    }
                }
            }
        }
        
        fclose(fp);
        
        printf("\nCoverage Summary for lines 111-130:\n");
        printf("  Covered: %d/%d lines (%.1f%%)\n", 
               target_lines_covered, target_lines_total,
               target_lines_total > 0 ? 
                 (100.0 * target_lines_covered / target_lines_total) : 0.0);
        
        if (target_lines_covered == target_lines_total) {
            printf("SUCCESS: All target lines covered!\n");
        } else {
            printf("PARTIAL: Some target lines not covered\n");
        }
    } else {
        printf("Could not open gcov-dump.cc.gcov\n");
        
        // Try alternative method
        printf("\nTrying alternative coverage check...\n");
        system("gcov -b gcov-dump.cc | tail -20");
    }
}

int main(int argc, char *argv[]) {
    const char *gcov_dump_source_dir = ".";
    const char *instrumented_gcov_dump = "./gcov-dump-instrumented";
    const char *dummy_c = "./dummy.c";
    const char *dummy_exe = "./dummy_prog";
    const char *gcda_file = "./dummy.gcda";
    
    printf("=== GCOV-Dump Coverage Test ===\n\n");
    
    // Step 1: Build instrumented gcov-dump
    if (!build_instrumented_gcov_dump(gcov_dump_source_dir, instrumented_gcov_dump)) {
        printf("Trying to use system gcov-dump...\n");
        instrumented_gcov_dump = "gcov-dump";
    }
    
    // Step 2: Create and compile dummy program
    if (!create_dummy_program(dummy_c, dummy_exe)) {
        return 1;
    }
    
    // Step 3: Generate .gcda file
    if (!generate_gcda_file(dummy_exe)) {
        // Try to use any existing .gcda file
        printf("Looking for existing .gcda files...\n");
        if (access("dummy.gcda", F_OK) != 0) {
            printf("No .gcda file found. Creating empty one...\n");
            // Create minimal valid .gcda file
            FILE *fp = fopen("dummy.gcda", "wb");
            if (fp) {
                // Write minimal GCOV data (magic number + version + empty)
                unsigned int magic = 0x67636461; // 'gcda'
                unsigned int version = 0x3430392a; // '409*'
                fwrite(&magic, 4, 1, fp);
                fwrite(&version, 4, 1, fp);
                fclose(fp);
                printf("Created minimal dummy.gcda\n");
            }
        }
    }
    
    // Clean up any existing coverage data
    system("rm -f *.gcda *.gcno gcov-dump.cc.gcov 2>/dev/null");
    
    printf("\n=== Running Test Cases ===\n\n");
    
    // Step 4: Execute test cases to cover all switch branches
    
    // Test help flag (covers case 'h')
    printf("1. Testing -h (help flag)...\n");
    run_gcov_dump_test(instrumented_gcov_dump, "-h", NULL, 1);
    
    // Test version flag (covers case 'v')
    printf("\n2. Testing -v (version flag)...\n");
    run_gcov_dump_test(instrumented_gcov_dump, "-v", NULL, 1);
    
    // Test individual flags with gcda file
    printf("\n3. Testing -l (dump contents)...\n");
    run_gcov_dump_test(instrumented_gcov_dump, "-l", gcda_file, 1);
    
    printf("\n4. Testing -p (dump positions)...\n");
    run_gcov_dump_test(instrumented_gcov_dump, "-p", gcda_file, 1);
    
    printf("\n5. Testing -r (dump raw)...\n");
    run_gcov_dump_test(instrumented_gcov_dump, "-r", gcda_file, 1);
    
    printf("\n6. Testing -s (dump stable)...\n");
    run_gcov_dump_test(instrumented_gcov_dump, "-s", gcda_file, 1);
    
    // Test combined flags (space-separated)
    printf("\n7. Testing -l -p -r -s (combined flags)...\n");
    run_gcov_dump_test(instrumented_gcov_dump, "-l -p -r -s", gcda_file, 1);
    
    // Test concatenated flags
    printf("\n8. Testing -lprs (concatenated flags)...\n");
    run_gcov_dump_test(instrumented_gcov_dump, "-lprs", gcda_file, 1);
    
    // Test invalid flag (covers default case)
    printf("\n9. Testing -x (invalid flag - should trigger default case)...\n");
    run_gcov_dump_test(instrumented_gcov_dump, "-x", gcda_file, 0);
    
    // Test multiple invalid flags
    printf("\n10. Testing -xyz (multiple invalid flags)...\n");
    run_gcov_dump_test(instrumented_gcov_dump, "-xyz", gcda_file, 0);
    
    // Additional test: flags in different order
    printf("\n11. Testing -s -r -p -l (reverse order)...\n");
    run_gcov_dump_test(instrumented_gcov_dump, "-s -r -p -l", gcda_file, 1);
    
    // Test with no flags (just gcda file)
    printf("\n12. Testing with no flags (just gcda file)...\n");
    run_gcov_dump_test(instrumented_gcov_dump, "", gcda_file, 1);
    
    // Step 5: Check coverage
    printf("\n=== Final Coverage Analysis ===\n");
    check_coverage();
    
    // Cleanup
    printf("\n=== Cleaning Up ===\n");
    system("rm -f dummy.c dummy_prog dummy.gcda dummy.gcno 2>/dev/null");
    
    printf("\nTest completed. Check gcov-dump.cc.gcov for detailed coverage information.\n");
    
    return 0;
}
