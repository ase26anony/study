#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024

// Function to check if a file exists
int file_exists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

// Function to execute a command and check its exit status
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
    
    // Check if we're in a GCC source tree structure
    const char *libiberty_path = "../../libiberty/libiberty.a";
    if (!file_exists(libiberty_path)) {
        libiberty_path = "../libiberty/libiberty.a";
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
}

// Function to compile and run the dummy program to generate GCOV data
int generate_gcov_data(const char *dummy_source, const char *gcda_file) {
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
    
    // The .gcda file should be created with the same name as the source
    char expected_gcda[MAX_PATH];
    snprintf(expected_gcda, sizeof(expected_gcda), "%s.gcda", dummy_source);
    
    if (!file_exists(expected_gcda)) {
        fprintf(stderr, "Failed to generate .gcda file\n");
        return 0;
    }
    
    // Copy to our desired location
    snprintf(cmd, sizeof(cmd), "cp %s %s", expected_gcda, gcda_file);
    return execute_command(cmd, 1);
}

// Function to merge coverage data after each gcov-dump invocation
void merge_coverage_data(const char *gcov_dump_binary, const char *source_file) {
    char cmd[MAX_PATH * 4];
    
    // First, find the .gcda file for gcov-dump
    // It will be in the same directory as the binary with name gcov-dump.gcda
    char gcda_pattern[MAX_PATH];
    snprintf(gcda_pattern, sizeof(gcda_pattern), "%s*.gcda", gcov_dump_binary);
    
    // Use gcov to merge coverage data
    snprintf(cmd, sizeof(cmd), 
             "gcov -i %s > /dev/null 2>&1", 
             source_file);
    
    execute_command(cmd, 0);  // Don't check exit status - gcov might fail if no .gcda yet
}

// Function to run gcov-dump with various flag combinations
void run_flag_combinations(const char *gcov_dump_binary, const char *gcda_file) {
    char cmd[MAX_PATH * 4];
    const char *source_file = "gcov-dump.cc";
    
    printf("\n=== Testing individual flags ===\n");
    
    // Test help flag
    snprintf(cmd, sizeof(cmd), "%s -h", gcov_dump_binary);
    execute_command(cmd, 1);
    merge_coverage_data(gcov_dump_binary, source_file);
    
    // Test version flag
    snprintf(cmd, sizeof(cmd), "%s -v", gcov_dump_binary);
    execute_command(cmd, 1);
    merge_coverage_data(gcov_dump_binary, source_file);
    
    printf("\n=== Testing dump flags with GCOV data file ===\n");
    
    // Test individual dump flags with GCOV data file
    const char *flags[] = {"-l", "-p", "-r", "-s"};
    for (int i = 0; i < 4; i++) {
        snprintf(cmd, sizeof(cmd), "%s %s %s", gcov_dump_binary, flags[i], gcda_file);
        execute_command(cmd, 1);
        merge_coverage_data(gcov_dump_binary, source_file);
    }
    
    printf("\n=== Testing combined flags (space-separated) ===\n");
    
    // Test all flags combined with spaces
    snprintf(cmd, sizeof(cmd), "%s -l -p -r -s %s", gcov_dump_binary, gcda_file);
    execute_command(cmd, 1);
    merge_coverage_data(gcov_dump_binary, source_file);
    
    printf("\n=== Testing concatenated flags ===\n");
    
    // Test all flags concatenated
    snprintf(cmd, sizeof(cmd), "%s -lprs %s", gcov_dump_binary, gcda_file);
    execute_command(cmd, 1);
    merge_coverage_data(gcov_dump_binary, source_file);
    
    printf("\n=== Testing invalid flag (to trigger default case) ===\n");
    
    // Test invalid flag to trigger default case
    snprintf(cmd, sizeof(cmd), "%s -x %s 2>&1", gcov_dump_binary, gcda_file);
    // We expect this to fail, so don't check for success
    system(cmd);
    merge_coverage_data(gcov_dump_binary, source_file);
}

// Function to generate final coverage report
void generate_coverage_report(const char *source_file) {
    char cmd[MAX_PATH * 4];
    
    printf("\n=== Generating final coverage report ===\n");
    
    // Generate human-readable coverage report
    snprintf(cmd, sizeof(cmd), "gcov -b %s", source_file);
    execute_command(cmd, 0);
    
    // Check if the coverage file was created
    char coverage_file[MAX_PATH];
    snprintf(coverage_file, sizeof(coverage_file), "%s.gcov", source_file);
    
    if (file_exists(coverage_file)) {
        printf("\nCoverage report generated: %s\n", coverage_file);
        
        // Display relevant lines from coverage report
        printf("\n=== Checking target lines (111-130) in coverage report ===\n");
        snprintf(cmd, sizeof(cmd), 
                 "grep -n -A 20 '^ *111:' %s | head -30", 
                 coverage_file);
        system(cmd);
    }
}

int main(int argc, char *argv[]) {
    char cwd[MAX_PATH];
    char gcov_dump_binary[MAX_PATH];
    char dummy_source[MAX_PATH];
    char gcda_file[MAX_PATH];
    
    // Get current working directory
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed");
        return 1;
    }
    
    // Set paths
    snprintf(gcov_dump_binary, sizeof(gcov_dump_binary), "%s/gcov-dump-instrumented", cwd);
    snprintf(dummy_source, sizeof(dummy_source), "%s/dummy.c", cwd);
    snprintf(gcda_file, sizeof(gcda_file), "%s/test.gcda", cwd);
    
    printf("Current directory: %s\n", cwd);
    printf("Instrumented gcov-dump will be at: %s\n", gcov_dump_binary);
    
    // Step 1: Compile gcov-dump with coverage instrumentation
    printf("\n=== Step 1: Compiling gcov-dump with coverage ===\n");
    
    // Try to find gcov-dump source in common locations
    const char *possible_sources[] = {
        ".",
        "..",
        "../gcc",
        "../../gcc",
        NULL
    };
    
    const char *gcov_dump_source = NULL;
    for (int i = 0; possible_sources[i] != NULL; i++) {
        char test_path[MAX_PATH];
        snprintf(test_path, sizeof(test_path), "%s/%s/gcov-dump.cc", 
                 cwd, possible_sources[i]);
        
        if (file_exists(test_path)) {
            gcov_dump_source = possible_sources[i];
            printf("Found gcov-dump.cc in: %s\n", gcov_dump_source);
            break;
        }
    }
    
    if (gcov_dump_source == NULL) {
        fprintf(stderr, "Could not find gcov-dump.cc source file\n");
        fprintf(stderr, "Please run this test from GCC source directory or specify path\n");
        return 1;
    }
    
    if (!compile_gcov_dump_with_coverage(gcov_dump_source, gcov_dump_binary)) {
        fprintf(stderr, "Failed to compile gcov-dump with coverage\n");
        return 1;
    }
    
    if (!file_exists(gcov_dump_binary)) {
        fprintf(stderr, "Instrumented gcov-dump binary not created\n");
        return 1;
    }
    
    printf("Successfully compiled instrumented gcov-dump\n");
    
    // Step 2: Create and compile dummy program for GCOV data
    printf("\n=== Step 2: Generating test GCOV data ===\n");
    
    create_dummy_program(dummy_source);
    printf("Created dummy.c at: %s\n", dummy_source);
    
    if (!generate_gcov_data(dummy_source, gcda_file)) {
        fprintf(stderr, "Failed to generate GCOV data file\n");
        return 1;
    }
    
    if (!file_exists(gcda_file)) {
        fprintf(stderr, "GCOV data file not created: %s\n", gcda_file);
        return 1;
    }
    
    printf("Generated GCOV data file: %s\n", gcda_file);
    
    // Step 3: Run flag combinations to exercise the switch statement
    printf("\n=== Step 3: Testing flag combinations ===\n");
    
    run_flag_combinations(gcov_dump_binary, gcda_file);
    
    // Step 4: Generate final coverage report
    generate_coverage_report("gcov-dump.cc");
    
    // Cleanup
    printf("\n=== Cleaning up temporary files ===\n");
    char cleanup_cmd[MAX_PATH * 4];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd),
             "rm -f dummy.c dummy_prog dummy.gcda test.gcda *.gcov 2>/dev/null");
    system(cleanup_cmd);
    
    printf("\n=== Test completed ===\n");
    printf("Check gcov-dump.cc.gcov for coverage details of lines 111-130\n");
    
    return 0;
}
