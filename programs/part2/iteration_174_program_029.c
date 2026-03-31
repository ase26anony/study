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
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024
#define MAX_CMD 4096

/**
 * Build an instrumented version of gcov-dump
 */
int build_instrumented_gcov_dump(const char *source_dir, const char *build_dir) {
    char cmd[MAX_CMD];
    int status;
    
    printf("Building instrumented gcov-dump...\n");
    
    // First, check if we can find gcov-dump source
    char gcov_dump_source[MAX_PATH];
    snprintf(gcov_dump_source, sizeof(gcov_dump_source), 
             "%s/gcov-dump.cc", source_dir);
    
    struct stat st;
    if (stat(gcov_dump_source, &st) != 0) {
        // Try alternative location
        snprintf(gcov_dump_source, sizeof(gcov_dump_source), 
                 "%s/../gcc/gcov-dump.cc", source_dir);
        if (stat(gcov_dump_source, &st) != 0) {
            fprintf(stderr, "Error: Cannot find gcov-dump.cc\n");
            return 0;
        }
    }
    
    // Build command to compile instrumented gcov-dump
    snprintf(cmd, sizeof(cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage "
             "-I%s -I%s/../include -I%s/../libiberty "
             "%s %s/../libiberty/libiberty.a "
             "-o %s/gcov-dump-instrumented",
             source_dir, source_dir, source_dir,
             gcov_dump_source, source_dir,
             build_dir);
    
    printf("Compile command: %s\n", cmd);
    status = system(cmd);
    
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("Successfully built instrumented gcov-dump\n");
        return 1;
    } else {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 0;
    }
}

/**
 * Create a simple test program to generate GCOV data
 */
int create_test_program(const char *build_dir) {
    char dummy_c_path[MAX_PATH];
    snprintf(dummy_c_path, sizeof(dummy_c_path), "%s/dummy.c", build_dir);
    
    FILE *fp = fopen(dummy_c_path, "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return 0;
    }
    
    // Write a simple C program with loops and branches
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i, sum = 0;\n");
    fprintf(fp, "    \n");
    fprintf(fp, "    // Simple loop to generate some coverage\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        sum += i;\n");
    fprintf(fp, "        if (i %% 2 == 0) {\n");
    fprintf(fp, "            printf(\"Even: %%d\\n\", i);\n");
    fprintf(fp, "        } else {\n");
    fprintf(fp, "            printf(\"Odd: %%d\\n\", i);\n");
    fprintf(fp, "        }\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    \n");
    fprintf(fp, "    printf(\"Sum: %%d\\n\", sum);\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    
    // Compile the dummy program with coverage
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd),
             "cd %s && gcc -O0 -fprofile-arcs -ftest-coverage dummy.c -o dummy_prog",
             build_dir);
    
    int status = system(cmd);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("Successfully compiled dummy program\n");
        
        // Run the dummy program to generate .gcda file
        snprintf(cmd, sizeof(cmd), "cd %s && ./dummy_prog > /dev/null", build_dir);
        status = system(cmd);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Generated dummy.gcda file\n");
            return 1;
        }
    }
    
    fprintf(stderr, "Failed to create test program\n");
    return 0;
}

/**
 * Run gcov-dump with specific arguments and merge coverage
 */
int run_gcov_dump_test(const char *build_dir, const char *gcov_dump_path, 
                       const char *args, int expect_success) {
    char cmd[MAX_CMD];
    char gcda_file[MAX_PATH];
    int status;
    
    snprintf(gcda_file, sizeof(gcda_file), "%s/dummy.gcda", build_dir);
    
    // Build the command
    if (strstr(args, "-h") || strstr(args, "-v") || strstr(args, "-x")) {
        // These don't need a gcda file
        snprintf(cmd, sizeof(cmd), "%s %s 2>&1", gcov_dump_path, args);
    } else {
        // These need a gcda file
        snprintf(cmd, sizeof(cmd), "%s %s %s 2>&1", gcov_dump_path, args, gcda_file);
    }
    
    printf("Running: %s\n", cmd);
    status = system(cmd);
    
    // Check exit status
    int exited = WIFEXITED(status);
    int exit_code = exited ? WEXITSTATUS(status) : -1;
    
    if (expect_success) {
        if (!exited || exit_code != 0) {
            fprintf(stderr, "Command failed unexpectedly (exit code: %d)\n", exit_code);
            return 0;
        }
    } else {
        // For invalid flag, we expect failure
        if (exited && exit_code == 0) {
            fprintf(stderr, "Invalid flag command succeeded unexpectedly\n");
            return 0;
        }
    }
    
    // Merge coverage data after each run
    // First, find the .gcda file for gcov-dump
    char merge_cmd[MAX_CMD];
    snprintf(merge_cmd, sizeof(merge_cmd),
             "cd %s && gcov -i gcov-dump-instrumented-gcov-dump.cc 2>/dev/null || true",
             build_dir);
    system(merge_cmd);
    
    return 1;
}

/**
 * Generate final coverage report and check target lines
 */
void check_coverage(const char *build_dir, const char *source_path) {
    char cmd[MAX_CMD];
    char report_file[MAX_PATH];
    
    printf("\n=== Generating Coverage Report ===\n");
    
    // Generate coverage report
    snprintf(cmd, sizeof(cmd), 
             "cd %s && gcov -b gcov-dump-instrumented-gcov-dump.cc 2>&1",
             build_dir);
    
    FILE *fp = popen(cmd, "r");
    if (fp) {
        char buffer[1024];
        int in_target_section = 0;
        int lines_covered = 0;
        
        while (fgets(buffer, sizeof(buffer), fp)) {
            printf("%s", buffer);
            
            // Look for our target lines (111-130)
            if (strstr(buffer, "111:") || strstr(buffer, "112:") || 
                strstr(buffer, "113:") || strstr(buffer, "#####:")) {
                // Check if these are our target lines
                if (strstr(buffer, "111:") || strstr(buffer, "112:")) {
                    // These should be executed (print_usage, print_version)
                    if (!strstr(buffer, "#####")) {
                        lines_covered++;
                    }
                } else if (strstr(buffer, "#####:")) {
                    // Check line numbers in the ##### lines
                    char *colon = strchr(buffer, ':');
                    if (colon) {
                        int line_num = atoi(buffer + 5); // Skip "#####:"
                        if (line_num >= 114 && line_num <= 130) {
                            printf("WARNING: Line %d not covered!\n", line_num);
                        }
                    }
                }
            }
        }
        pclose(fp);
    }
    
    // Also generate annotated source
    printf("\n=== Annotated Source (target lines 111-130) ===\n");
    snprintf(cmd, sizeof(cmd),
             "cd %s && gcov -a gcov-dump-instrumented-gcov-dump.cc 2>&1 | "
             "sed -n '111,130p'",
             build_dir);
    system(cmd);
}

int main(int argc, char *argv[]) {
    char build_dir[MAX_PATH];
    char source_dir[MAX_PATH];
    char cwd[MAX_PATH];
    
    // Get current directory
    if (!getcwd(cwd, sizeof(cwd))) {
        perror("getcwd failed");
        return 1;
    }
    
    // Determine source and build directories
    // Try to find gcov-dump source in common locations
    const char *possible_sources[] = {
        ".",
        "..",
        "../gcc",
        "../../gcc",
        "/usr/src/gcc",
        NULL
    };
    
    int found_source = 0;
    for (int i = 0; possible_sources[i]; i++) {
        char test_path[MAX_PATH];
        snprintf(test_path, sizeof(test_path), "%s/%s/gcov-dump.cc", 
                 cwd, possible_sources[i]);
        
        struct stat st;
        if (stat(test_path, &st) == 0) {
            snprintf(source_dir, sizeof(source_dir), "%s/%s", 
                    cwd, possible_sources[i]);
            found_source = 1;
            break;
        }
    }
    
    if (!found_source) {
        // Use current directory as fallback
        strcpy(source_dir, cwd);
    }
    
    // Create build directory
    snprintf(build_dir, sizeof(build_dir), "%s/gcov_dump_test_build", cwd);
    mkdir(build_dir, 0755);
    
    printf("Source directory: %s\n", source_dir);
    printf("Build directory: %s\n", build_dir);
    
    // Step 1: Build instrumented gcov-dump
    if (!build_instrumented_gcov_dump(source_dir, build_dir)) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    // Step 2: Create test program and generate GCOV data
    if (!create_test_program(build_dir)) {
        fprintf(stderr, "Failed to create test program\n");
        return 1;
    }
    
    char gcov_dump_path[MAX_PATH];
    snprintf(gcov_dump_path, sizeof(gcov_dump_path), 
             "%s/gcov-dump-instrumented", build_dir);
    
    // Step 3: Run comprehensive flag tests
    
    printf("\n=== Testing flag combinations ===\n");
    
    // Test help flag (-h)
    printf("\n1. Testing -h (help flag):\n");
    if (!run_gcov_dump_test(build_dir, gcov_dump_path, "-h", 1)) {
        fprintf(stderr, "Help flag test failed\n");
    }
    
    // Test version flag (-v)
    printf("\n2. Testing -v (version flag):\n");
    if (!run_gcov_dump_test(build_dir, gcov_dump_path, "-v", 1)) {
        fprintf(stderr, "Version flag test failed\n");
    }
    
    // Test individual flags with gcda file
    printf("\n3. Testing -l (dump contents):\n");
    if (!run_gcov_dump_test(build_dir, gcov_dump_path, "-l", 1)) {
        fprintf(stderr, "-l flag test failed\n");
    }
    
    printf("\n4. Testing -p (dump positions):\n");
    if (!run_gcov_dump_test(build_dir, gcov_dump_path, "-p", 1)) {
        fprintf(stderr, "-p flag test failed\n");
    }
    
    printf("\n5. Testing -r (dump raw):\n");
    if (!run_gcov_dump_test(build_dir, gcov_dump_path, "-r", 1)) {
        fprintf(stderr, "-r flag test failed\n");
    }
    
    printf("\n6. Testing -s (dump stable):\n");
    if (!run_gcov_dump_test(build_dir, gcov_dump_path, "-s", 1)) {
        fprintf(stderr, "-s flag test failed\n");
    }
    
    // Test combined flags (space-separated)
    printf("\n7. Testing -l -p -r -s (space separated):\n");
    if (!run_gcov_dump_test(build_dir, gcov_dump_path, "-l -p -r -s", 1)) {
        fprintf(stderr, "Combined flags test failed\n");
    }
    
    // Test concatenated flags
    printf("\n8. Testing -lprs (concatenated):\n");
    if (!run_gcov_dump_test(build_dir, gcov_dump_path, "-lprs", 1)) {
        fprintf(stderr, "Concatenated flags test failed\n");
    }
    
    // Test invalid flag (should trigger default case)
    printf("\n9. Testing -x (invalid flag):\n");
    if (!run_gcov_dump_test(build_dir, gcov_dump_path, "-x", 0)) {
        fprintf(stderr, "Invalid flag test failed\n");
    }
    
    // Step 4: Check coverage
    check_coverage(build_dir, source_dir);
    
    printf("\n=== Test Complete ===\n");
    printf("Coverage data has been generated for gcov-dump.cc\n");
    printf("Check %s/gcov-dump-instrumented-gcov-dump.cc.gcov for details\n", build_dir);
    
    return 0;
}
