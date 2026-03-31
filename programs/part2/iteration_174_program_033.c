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

// Function to execute a command and check its exit status
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

// Function to compile gcov-dump with coverage instrumentation
int compile_gcov_dump_with_coverage(const char *source_dir, const char *output_path) {
    char cmd[MAX_PATH * 4];
    
    // Try to find gcov-dump source in common locations
    const char *possible_sources[] = {
        "gcov-dump.cc",
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "gcc/gcov-dump.cc",
        NULL
    };
    
    char source_path[MAX_PATH] = "";
    for (int i = 0; possible_sources[i]; i++) {
        snprintf(cmd, sizeof(cmd), "find %s -name gcov-dump.cc 2>/dev/null | head -1", source_dir);
        FILE *fp = popen(cmd, "r");
        if (fp) {
            if (fgets(source_path, sizeof(source_path), fp)) {
                // Remove trailing newline
                source_path[strcspn(source_path, "\n")] = 0;
                pclose(fp);
                break;
            }
            pclose(fp);
        }
    }
    
    if (strlen(source_path) == 0) {
        // Try direct paths
        for (int i = 0; possible_sources[i]; i++) {
            if (file_exists(possible_sources[i])) {
                strncpy(source_path, possible_sources[i], sizeof(source_path));
                break;
            }
        }
    }
    
    if (strlen(source_path) == 0) {
        fprintf(stderr, "Error: Could not find gcov-dump.cc source file\n");
        return 0;
    }
    
    printf("Found gcov-dump source at: %s\n", source_path);
    
    // Extract directory for include paths
    char source_dir_path[MAX_PATH];
    strncpy(source_dir_path, source_path, sizeof(source_dir_path));
    char *last_slash = strrchr(source_dir_path, '/');
    if (last_slash) *last_slash = '\0';
    
    // Compile with coverage instrumentation
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-I%s -I%s/../../include -I%s/../../libiberty "
        "%s %s/../../libiberty/libiberty.a "
        "-o %s",
        source_dir_path, source_dir_path, source_dir_path,
        source_path, source_dir_path,
        output_path);
    
    printf("Compiling gcov-dump with: %s\n", cmd);
    return execute_command(cmd) == 0;
}

// Function to create a dummy C program for generating .gcda files
void create_dummy_program(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        exit(1);
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        printf(\"Iteration %%d\\n\", i);\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    printf("Created dummy program: %s\n", filename);
}

int main(int argc, char *argv[]) {
    char cwd[MAX_PATH];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed");
        return 1;
    }
    
    printf("Current directory: %s\n", cwd);
    
    // Paths for our files
    char gcov_dump_path[MAX_PATH];
    char dummy_c_path[MAX_PATH];
    char dummy_exe_path[MAX_PATH];
    
    snprintf(gcov_dump_path, sizeof(gcov_dump_path), "%s/gcov-dump-instrumented", cwd);
    snprintf(dummy_c_path, sizeof(dummy_c_path), "%s/dummy.c", cwd);
    snprintf(dummy_exe_path, sizeof(dummy_exe_path), "%s/dummy_prog", cwd);
    
    // Step 1: Build/Locate Instrumented gcov-dump
    if (!file_exists(gcov_dump_path)) {
        printf("Instrumented gcov-dump not found, compiling...\n");
        if (!compile_gcov_dump_with_coverage(cwd, gcov_dump_path)) {
            fprintf(stderr, "Failed to compile gcov-dump with coverage\n");
            return 1;
        }
    } else {
        printf("Using existing instrumented gcov-dump: %s\n", gcov_dump_path);
    }
    
    // Step 2: Generate Test GCOV Data
    printf("\n=== Generating test GCOV data ===\n");
    
    // Create dummy.c if it doesn't exist
    if (!file_exists(dummy_c_path)) {
        create_dummy_program(dummy_c_path);
    }
    
    // Compile dummy.c with coverage
    char cmd[MAX_PATH * 4];
    snprintf(cmd, sizeof(cmd), 
             "gcc -O0 -fprofile-arcs -ftest-coverage %s -o %s",
             dummy_c_path, dummy_exe_path);
    
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return 1;
    }
    
    // Run dummy program to generate .gcda file
    snprintf(cmd, sizeof(cmd), "%s", dummy_exe_path);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        return 1;
    }
    
    // Find the .gcda file (it will be in the same directory as the executable)
    char gcda_file[MAX_PATH];
    snprintf(gcda_file, sizeof(gcda_file), "%s.gcda", dummy_exe_path);
    
    if (!file_exists(gcda_file)) {
        // Try alternative location
        snprintf(gcda_file, sizeof(gcda_file), "dummy.gcda");
        if (!file_exists(gcda_file)) {
            fprintf(stderr, "Failed to find generated .gcda file\n");
            return 1;
        }
    }
    
    printf("Using GCOV data file: %s\n", gcda_file);
    
    // Step 3: Execute Flag Coverage Series
    printf("\n=== Testing gcov-dump flag combinations ===\n");
    
    // Array of test cases: {description, arguments, expect_success}
    struct {
        const char *desc;
        const char *args;
        int expect_success;
    } test_cases[] = {
        {"Help flag", "-h", 1},
        {"Version flag", "-v", 1},
        {"List contents flag", "-l", 1},
        {"Dump positions flag", "-p", 1},
        {"Raw dump flag", "-r", 1},
        {"Stable dump flag", "-s", 1},
        {"Combined flags (space-separated)", "-l -p -r -s", 1},
        {"Combined flags (concatenated)", "-lprs", 1},
        {"Invalid flag (should fail)", "-x", 0},
        {"List with data file", "-l", 1},
        {"Positions with data file", "-p", 1},
        {"Raw with data file", "-r", 1},
        {"Stable with data file", "-s", 1},
        {"All flags with data file", "-lprs", 1},
        {NULL, NULL, 0}
    };
    
    int total_tests = 0;
    int passed_tests = 0;
    
    for (int i = 0; test_cases[i].desc; i++) {
        printf("\nTest %d: %s\n", i + 1, test_cases[i].desc);
        
        // Build command
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_path, test_cases[i].args);
        
        // For tests that need a data file (not help/version), append gcda file
        if (strcmp(test_cases[i].args, "-h") != 0 && 
            strcmp(test_cases[i].args, "-v") != 0 &&
            strcmp(test_cases[i].args, "-x") != 0) {
            // Check if this is a flag-only test or flag+file test
            if (strstr(test_cases[i].desc, "with data file")) {
                // Already has the flag, need to add file
                strncat(cmd, " ", MAX_PATH - strlen(cmd) - 1);
                strncat(cmd, gcda_file, MAX_PATH - strlen(cmd) - 1);
            } else if (strcmp(test_cases[i].args, "-l") == 0 ||
                      strcmp(test_cases[i].args, "-p") == 0 ||
                      strcmp(test_cases[i].args, "-r") == 0 ||
                      strcmp(test_cases[i].args, "-s") == 0 ||
                      strcmp(test_cases[i].args, "-l -p -r -s") == 0 ||
                      strcmp(test_cases[i].args, "-lprs") == 0) {
                // These flags need a file argument but test description doesn't say so
                // They'll fail without a file, but that's OK - we still exercise the flag parsing
                printf("  Note: This test may fail due to missing required file argument\n");
            }
        }
        
        // Execute the command
        int exit_code = execute_command(cmd);
        
        // Check result
        int success = (exit_code == 0);
        if (test_cases[i].expect_success) {
            if (success) {
                printf("  PASS: Command succeeded as expected\n");
                passed_tests++;
            } else {
                printf("  FAIL: Command failed but expected success (exit code: %d)\n", exit_code);
            }
        } else {
            if (!success) {
                printf("  PASS: Command failed as expected (exit code: %d)\n", exit_code);
                passed_tests++;
            } else {
                printf("  FAIL: Command succeeded but expected failure\n");
            }
        }
        
        total_tests++;
        
        // Step 4: Merge coverage after each run
        printf("  Merging coverage data...\n");
        
        // First, find the gcov-dump.gcda file
        char gcda_pattern[MAX_PATH];
        snprintf(gcda_pattern, sizeof(gcda_pattern), "find . -name \"*gcov-dump*.gcda\" 2>/dev/null | head -1");
        
        FILE *fp = popen(gcda_pattern, "r");
        if (fp) {
            char found_gcda[MAX_PATH];
            if (fgets(found_gcda, sizeof(found_gcda), fp)) {
                found_gcda[strcspn(found_gcda, "\n")] = 0;
                pclose(fp);
                
                // Use gcov to process the coverage data
                snprintf(cmd, sizeof(cmd), "gcov -i %s 2>/dev/null", found_gcda);
                system(cmd);
            } else {
                pclose(fp);
            }
        }
        
        // Also try to merge using lcov if available
        system("lcov --capture --directory . --output-file coverage.info 2>/dev/null");
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    
    // Step 5: Final Coverage Check
    printf("\n=== Generating final coverage report ===\n");
    
    // Generate coverage report for gcov-dump.cc
    snprintf(cmd, sizeof(cmd), "gcov -b gcov-dump.cc 2>&1");
    printf("Running: %s\n", cmd);
    
    FILE *gcov_output = popen(cmd, "r");
    if (gcov_output) {
        char line[1024];
        int target_lines_covered = 0;
        int in_target_section = 0;
        
        printf("\nCoverage report for gcov-dump.cc:\n");
        printf("-----------------------------------\n");
        
        while (fgets(line, sizeof(line), gcov_output)) {
            printf("%s", line);
            
            // Check for lines 111-130 in the output
            if (strstr(line, ":") && !strstr(line, "creating")) {
                int line_num;
                if (sscanf(line, "%d:", &line_num) == 1) {
                    if (line_num >= 111 && line_num <= 130) {
                        // Check if line was executed
                        if (strstr(line, "#####") == NULL && 
                            strstr(line, "-:") == NULL) {
                            target_lines_covered++;
                        }
                    }
                }
            }
        }
        pclose(gcov_output);
        
        printf("\n=== Coverage Analysis ===\n");
        printf("Lines 111-130 in gcov-dump.cc: 20 total lines\n");
        printf("Lines covered in target block: %d\n", target_lines_covered);
        
        if (target_lines_covered > 0) {
            printf("SUCCESS: Target switch-case logic was executed!\n");
            
            // Generate more detailed report
            printf("\nDetailed coverage of target lines:\n");
            snprintf(cmd, sizeof(cmd), "gcov -a gcov-dump.cc 2>&1 | grep -A 30 'Lines 111-130'");
            system(cmd);
            
            return 0;
        } else {
            printf("FAILURE: Target switch-case logic was not executed\n");
            return 1;
        }
    } else {
        fprintf(stderr, "Failed to generate coverage report\n");
        return 1;
    }
    
    return 0;
}
