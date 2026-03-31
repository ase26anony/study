/**
 * test_gcov_dump_coverage.c
 * 
 * This program tests the command-line argument parsing in gcov-dump.cc
 * to ensure coverage of lines 111-130 (the switch-case handling of flags).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024

/**
 * Creates a simple C program that will generate GCOV data when compiled
 * with coverage flags and executed.
 */
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
    fprintf(fp, "        printf(\"Value: %%d\\n\", i);\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    printf("Created dummy program: %s\n", filename);
}

/**
 * Compiles a program with coverage instrumentation.
 */
int compile_with_coverage(const char *source, const char *output, 
                         const char *compiler, const char *extra_flags) {
    char command[MAX_PATH * 4];
    
    snprintf(command, sizeof(command), 
             "%s -O0 -fprofile-arcs -ftest-coverage %s %s -o %s",
             compiler, extra_flags ? extra_flags : "", source, output);
    
    printf("Compiling: %s\n", command);
    return system(command);
}

/**
 * Executes a program and returns its exit status.
 */
int execute_program(const char *program, const char *args) {
    char command[MAX_PATH * 2];
    
    if (args) {
        snprintf(command, sizeof(command), "%s %s", program, args);
    } else {
        snprintf(command, sizeof(command), "%s", program);
    }
    
    printf("Executing: %s\n", command);
    return system(command);
}

/**
 * Runs gcov-dump with specific arguments and merges coverage data.
 */
void run_gcov_dump_test(const char *gcov_dump_path, const char *gcda_file, 
                       const char *args, const char *test_name) {
    char command[MAX_PATH * 4];
    int status;
    
    printf("\n=== Test: %s ===\n", test_name);
    
    // Build the full command
    if (gcda_file && strstr(args, gcda_file) == NULL) {
        // If gcda_file is provided but not already in args, append it
        snprintf(command, sizeof(command), "%s %s %s", 
                gcov_dump_path, args, gcda_file);
    } else {
        snprintf(command, sizeof(command), "%s %s", 
                gcov_dump_path, args);
    }
    
    printf("Command: %s\n", command);
    
    // Execute gcov-dump
    status = system(command);
    printf("Exit status: %d\n", status);
    
    // Merge coverage data (critical step)
    // First, find the .gcda file for gcov-dump itself
    char merge_cmd[MAX_PATH * 2];
    snprintf(merge_cmd, sizeof(merge_cmd), 
             "gcov -i gcov-dump.cc 2>/dev/null || true");
    system(merge_cmd);
}

/**
 * Main test driver
 */
int main(int argc, char *argv[]) {
    char cwd[MAX_PATH];
    char dummy_c[MAX_PATH];
    char dummy_exe[MAX_PATH];
    char dummy_gcda[MAX_PATH];
    char gcov_dump_instrumented[MAX_PATH];
    
    // Get current directory
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed");
        return 1;
    }
    
    // Create paths
    snprintf(dummy_c, sizeof(dummy_c), "%s/dummy.c", cwd);
    snprintf(dummy_exe, sizeof(dummy_exe), "%s/dummy_prog", cwd);
    snprintf(dummy_gcda, sizeof(dummy_gcda), "%s/dummy.gcda", cwd);
    
    // Try to find or specify the instrumented gcov-dump
    // First check if it's in the current directory
    snprintf(gcov_dump_instrumented, sizeof(gcov_dump_instrumented),
             "%s/gcov-dump-instrumented", cwd);
    
    struct stat st;
    if (stat(gcov_dump_instrumented, &st) != 0) {
        // Try to find it in PATH
        printf("Instrumented gcov-dump not found at %s\n", gcov_dump_instrumented);
        printf("Looking for gcov-dump in PATH...\n");
        
        // Try to use system gcov-dump (may not be instrumented)
        // For a proper test, we should compile our own instrumented version
        printf("\nTo properly test, you need to compile gcov-dump with coverage:\n");
        printf("g++ -O0 -fprofile-arcs -ftest-coverage \\\n");
        printf("    -I. -I../../include -I../../libiberty \\\n");
        printf("    gcov-dump.cc ../../libiberty/libiberty.a \\\n");
        printf("    -o gcov-dump-instrumented\n\n");
        
        // For this example, we'll assume it exists or use a fallback
        strcpy(gcov_dump_instrumented, "gcov-dump");
    } else {
        printf("Found instrumented gcov-dump at: %s\n", gcov_dump_instrumented);
    }
    
    // Step 1: Create and compile dummy program to generate GCOV data
    printf("\n--- Step 1: Generating test GCOV data ---\n");
    create_dummy_program(dummy_c);
    
    // Compile dummy program with coverage
    if (compile_with_coverage(dummy_c, dummy_exe, "gcc", NULL) != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return 1;
    }
    
    // Execute dummy program to generate .gcda file
    if (execute_program(dummy_exe, NULL) != 0) {
        fprintf(stderr, "Failed to execute dummy program\n");
        return 1;
    }
    
    // Verify .gcda file was created
    if (stat(dummy_gcda, &st) != 0) {
        fprintf(stderr, "Failed to create dummy.gcda file\n");
        return 1;
    }
    printf("Generated GCOV data file: %s\n", dummy_gcda);
    
    // Step 2: Clear any existing coverage data for gcov-dump
    printf("\n--- Step 2: Clearing existing coverage data ---\n");
    system("rm -f gcov-dump.gcda gcov-dump.gcno 2>/dev/null || true");
    system("rm -f *.gcda *.gcno 2>/dev/null || true");
    
    // Step 3: Execute comprehensive flag tests
    printf("\n--- Step 3: Testing gcov-dump flag parsing ---\n");
    
    // Test individual flags (lines 111-130)
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-h", "Help flag");
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-v", "Version flag");
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-l", "Dump contents flag");
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-p", "Dump positions flag");
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-r", "Dump raw flag");
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-s", "Dump stable flag");
    
    // Test combined flags (space-separated)
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-l -p -r -s", 
                      "All flags space-separated");
    
    // Test concatenated flags
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-lprs", 
                      "All flags concatenated");
    
    // Test invalid flag (should trigger default case)
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-x", 
                      "Invalid flag (should show error)");
    
    // Test mixed valid and invalid
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "-l -x -p", 
                      "Mixed valid/invalid flags");
    
    // Test with no flags (just the data file)
    run_gcov_dump_test(gcov_dump_instrumented, dummy_gcda, "", 
                      "No flags, just data file");
    
    // Step 4: Generate final coverage report
    printf("\n--- Step 4: Generating coverage report ---\n");
    
    // First, ensure we have the .gcno file (created during compilation)
    // If not, we need to compile gcov-dump with coverage
    if (stat("gcov-dump.gcno", &st) != 0) {
        printf("Note: gcov-dump.gcno not found. Coverage report may be limited.\n");
        printf("To get full coverage data, compile gcov-dump with:\n");
        printf("  -fprofile-arcs -ftest-coverage flags\n");
    }
    
    // Generate human-readable coverage report
    printf("\nGenerating coverage report for gcov-dump.cc...\n");
    system("gcov -b gcov-dump.cc 2>&1 | grep -A 20 'Lines executed:'");
    
    // Specifically check for our target lines
    printf("\n--- Checking target lines (111-130) ---\n");
    system("gcov -b gcov-dump.cc 2>&1 | grep -n '^[ ]*[0-9]' | "
           "awk '$1 >= 111 && $1 <= 130'");
    
    // Alternative: create a simple coverage summary
    printf("\n--- Creating detailed coverage output ---\n");
    char coverage_cmd[MAX_PATH * 4];
    snprintf(coverage_cmd, sizeof(coverage_cmd),
             "gcov -b gcov-dump.cc 2>&1 > gcov_output.txt && "
             "echo 'Coverage summary:' && "
             "grep -E '(Lines executed:|Branches executed:|Taken at least once:)' gcov_output.txt");
    system(coverage_cmd);
    
    // Clean up intermediate files
    printf("\n--- Cleaning up ---\n");
    remove(dummy_c);
    remove(dummy_exe);
    remove(dummy_gcda);
    remove("dummy.gcno");
    
    printf("\n=== Test completed ===\n");
    printf("Check gcov-dump.c.gcov for line-by-line coverage details.\n");
    printf("Lines 111-130 should now show execution counts > 0.\n");
    
    return 0;
}
