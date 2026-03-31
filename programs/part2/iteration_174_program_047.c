// test_gcov_dump_flags.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_PATH 1024

// Function to compile gcov-dump with coverage instrumentation
int compile_gcov_dump_with_coverage() {
    printf("Compiling gcov-dump with coverage instrumentation...\n");
    
    // Try to find gcov-dump source in common locations
    const char* source_paths[] = {
        "gcov-dump.cc",
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "/usr/src/gcc/gcc/gcov-dump.cc",
        NULL
    };
    
    const char* source_file = NULL;
    for (int i = 0; source_paths[i] != NULL; i++) {
        if (access(source_paths[i], R_OK) == 0) {
            source_file = source_paths[i];
            break;
        }
    }
    
    if (!source_file) {
        fprintf(stderr, "Error: Could not find gcov-dump.cc source file\n");
        return 0;
    }
    
    // Compile command
    char compile_cmd[2048];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include "
             "-I../../libiberty %s ../../libiberty/libiberty.a "
             "-o gcov-dump-instrumented",
             source_file);
    
    printf("Running: %s\n", compile_cmd);
    int result = system(compile_cmd);
    
    return (result == 0);
}

// Function to create a dummy program for generating .gcda files
int create_dummy_program() {
    printf("Creating dummy program for test data...\n");
    
    // Create dummy.c
    FILE* fp = fopen("dummy.c", "w");
    if (!fp) {
        perror("Error creating dummy.c");
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
    
    // Compile dummy program with coverage
    int result = system("gcc -O0 -fprofile-arcs -ftest-coverage dummy.c -o dummy_prog");
    if (result != 0) {
        fprintf(stderr, "Error compiling dummy program\n");
        return 0;
    }
    
    // Run dummy program to generate .gcda file
    result = system("./dummy_prog > /dev/null 2>&1");
    if (result != 0) {
        fprintf(stderr, "Error running dummy program\n");
        return 0;
    }
    
    return 1;
}

// Function to run gcov-dump with specific flags and merge coverage
int run_gcov_dump_test(const char* flags, const char* data_file, int expect_success) {
    char cmd[1024];
    int exit_status;
    
    // Build the command
    if (data_file) {
        snprintf(cmd, sizeof(cmd), "./gcov-dump-instrumented %s %s", flags, data_file);
    } else {
        snprintf(cmd, sizeof(cmd), "./gcov-dump-instrumented %s", flags);
    }
    
    printf("Running: %s\n", cmd);
    
    // Execute the command
    exit_status = system(cmd);
    
    // Check exit status
    if (expect_success) {
        if (exit_status != 0) {
            fprintf(stderr, "Error: Command failed with exit status %d\n", exit_status);
            return 0;
        }
    } else {
        if (exit_status == 0) {
            fprintf(stderr, "Error: Expected command to fail but it succeeded\n");
            return 0;
        }
    }
    
    // Merge coverage data
    printf("Merging coverage data...\n");
    system("gcov -i gcov-dump.cc > /dev/null 2>&1");
    
    return 1;
}

int main() {
    printf("=== Starting gcov-dump flag coverage test ===\n\n");
    
    // Step 1: Compile gcov-dump with coverage
    if (!compile_gcov_dump_with_coverage()) {
        fprintf(stderr, "Failed to compile gcov-dump with coverage\n");
        return 1;
    }
    
    // Step 2: Create dummy program and generate .gcda file
    if (!create_dummy_program()) {
        fprintf(stderr, "Failed to create dummy program\n");
        return 1;
    }
    
    // Step 3: Clean up any existing coverage data
    printf("Cleaning up existing coverage data...\n");
    system("rm -f *.gcda *.gcno gcov-dump-instrumented.gcda");
    
    // Step 4: Run tests for each flag combination
    
    // Test help flag (-h)
    printf("\n--- Testing help flag (-h) ---\n");
    if (!run_gcov_dump_test("-h", NULL, 1)) {
        fprintf(stderr, "Help flag test failed\n");
    }
    
    // Test version flag (-v)
    printf("\n--- Testing version flag (-v) ---\n");
    if (!run_gcov_dump_test("-v", NULL, 1)) {
        fprintf(stderr, "Version flag test failed\n");
    }
    
    // Test individual flags with data file
    printf("\n--- Testing individual flags ---\n");
    
    printf("\nTesting -l flag...\n");
    if (!run_gcov_dump_test("-l", "dummy.gcda", 1)) {
        fprintf(stderr, "-l flag test failed\n");
    }
    
    printf("\nTesting -p flag...\n");
    if (!run_gcov_dump_test("-p", "dummy.gcda", 1)) {
        fprintf(stderr, "-p flag test failed\n");
    }
    
    printf("\nTesting -r flag...\n");
    if (!run_gcov_dump_test("-r", "dummy.gcda", 1)) {
        fprintf(stderr, "-r flag test failed\n");
    }
    
    printf("\nTesting -s flag...\n");
    if (!run_gcov_dump_test("-s", "dummy.gcda", 1)) {
        fprintf(stderr, "-s flag test failed\n");
    }
    
    // Test combined flags (space-separated)
    printf("\n--- Testing combined flags (space-separated) ---\n");
    if (!run_gcov_dump_test("-l -p -r -s", "dummy.gcda", 1)) {
        fprintf(stderr, "Combined flags test failed\n");
    }
    
    // Test concatenated flags
    printf("\n--- Testing concatenated flags ---\n");
    if (!run_gcov_dump_test("-lprs", "dummy.gcda", 1)) {
        fprintf(stderr, "Concatenated flags test failed\n");
    }
    
    // Test invalid flag (should trigger default case)
    printf("\n--- Testing invalid flag (should trigger default case) ---\n");
    if (!run_gcov_dump_test("-x", "dummy.gcda", 0)) {
        fprintf(stderr, "Invalid flag test failed\n");
    }
    
    // Step 5: Generate final coverage report
    printf("\n=== Generating final coverage report ===\n");
    system("gcov -b gcov-dump.cc");
    
    // Step 6: Check if target lines were covered
    printf("\n=== Checking coverage of target lines (111-130) ===\n");
    
    // Read the coverage file
    FILE* cov_file = fopen("gcov-dump.cc.gcov", "r");
    if (cov_file) {
        char line[1024];
        int line_num;
        int hits;
        char source_line[1024];
        int target_lines_covered = 0;
        
        while (fgets(line, sizeof(line), cov_file)) {
            if (sscanf(line, "%d:%d:%99[^\n]", &hits, &line_num, source_line) == 3) {
                if (line_num >= 111 && line_num <= 130) {
                    printf("Line %d: %s (executed %d times)\n", line_num, source_line, hits);
                    if (hits > 0) {
                        target_lines_covered++;
                    }
                }
            }
        }
        fclose(cov_file);
        
        printf("\nCoverage summary for lines 111-130: %d/20 lines executed\n", 
               target_lines_covered);
        
        if (target_lines_covered == 20) {
            printf("SUCCESS: All target lines covered!\n");
        } else {
            printf("WARNING: Not all target lines were covered\n");
        }
    } else {
        fprintf(stderr, "Could not open coverage file\n");
    }
    
    // Cleanup
    printf("\n=== Cleaning up ===\n");
    system("rm -f dummy.c dummy_prog dummy.gcda dummy.gcno");
    
    return 0;
}
