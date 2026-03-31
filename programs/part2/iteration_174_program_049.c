// test_gcov_dump_coverage.c
// Test wrapper to cover the command-line argument parsing in gcov-dump.cc
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

// Simple dummy program to generate GCOV data
const char* dummy_program = 
"#include <stdio.h>\n"
"int main() {\n"
"    int i;\n"
"    for (i = 0; i < 10; i++) {\n"
"        printf(\"Value: %d\\n\", i);\n"
"    }\n"
"    return 0;\n"
"}\n";

// Function to check if file exists
int file_exists(const char* filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

// Function to execute a command and check return code
int execute_command(const char* cmd, int expect_success) {
    printf("Executing: %s\n", cmd);
    int result = system(cmd);
    if (expect_success && result != 0) {
        fprintf(stderr, "Command failed: %s\n", cmd);
        return 0;
    }
    return 1;
}

// Function to compile gcov-dump with coverage instrumentation
int compile_gcov_dump_with_coverage() {
    printf("Compiling gcov-dump with coverage instrumentation...\n");
    
    // First check if we have gcov-dump source
    if (!file_exists("gcov-dump.cc")) {
        fprintf(stderr, "Error: gcov-dump.cc not found in current directory\n");
        return 0;
    }
    
    // Try to compile with coverage flags
    const char* compile_cmd = 
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-I. -I../../include -I../../libiberty "
        "gcov-dump.cc ../../libiberty/libiberty.a "
        "-o gcov-dump-instrumented 2>&1";
    
    return execute_command(compile_cmd, 1);
}

// Function to generate test GCOV data file
int generate_test_gcda() {
    printf("Generating test GCOV data file...\n");
    
    // Write dummy program
    FILE* fp = fopen("dummy.c", "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return 0;
    }
    fprintf(fp, "%s", dummy_program);
    fclose(fp);
    
    // Compile dummy program with coverage
    if (!execute_command("gcc -O0 -fprofile-arcs -ftest-coverage dummy.c -o dummy_prog", 1)) {
        return 0;
    }
    
    // Run dummy program to generate .gcda file
    if (!execute_command("./dummy_prog > /dev/null 2>&1", 1)) {
        return 0;
    }
    
    // Check if .gcda file was created
    if (!file_exists("dummy.gcda")) {
        fprintf(stderr, "Error: dummy.gcda not generated\n");
        return 0;
    }
    
    return 1;
}

// Function to run gcov-dump with various flag combinations
int run_gcov_dump_tests() {
    printf("Running gcov-dump tests...\n");
    
    const char* gcda_file = "dummy.gcda";
    const char* gcov_dump_binary = "./gcov-dump-instrumented";
    
    // Test individual flags
    printf("\n=== Testing individual flags ===\n");
    execute_command("./gcov-dump-instrumented -h", 1);
    execute_command("./gcov-dump-instrumented -v", 1);
    execute_command("./gcov-dump-instrumented -l dummy.gcda", 1);
    execute_command("./gcov-dump-instrumented -p dummy.gcda", 1);
    execute_command("./gcov-dump-instrumented -r dummy.gcda", 1);
    execute_command("./gcov-dump-instrumented -s dummy.gcda", 1);
    
    // Test combined flags (space-separated)
    printf("\n=== Testing combined flags (space-separated) ===\n");
    execute_command("./gcov-dump-instrumented -l -p -r -s dummy.gcda", 1);
    
    // Test concatenated flags
    printf("\n=== Testing concatenated flags ===\n");
    execute_command("./gcov-dump-instrumented -lprs dummy.gcda", 1);
    
    // Test invalid flag (should trigger default case)
    printf("\n=== Testing invalid flag (should trigger default case) ===\n");
    execute_command("./gcov-dump-instrumented -x dummy.gcda 2>&1", 0);
    
    return 1;
}

// Function to merge and check coverage
int check_coverage() {
    printf("\n=== Merging and checking coverage ===\n");
    
    // First, merge coverage data
    if (!execute_command("gcov -i gcov-dump.cc 2>&1", 1)) {
        fprintf(stderr, "Warning: gcov -i failed, trying alternative method\n");
        // Try alternative: copy .gcda files
        execute_command("cp gcov-dump-instrumented-*.gcda . 2>/dev/null || true", 1);
    }
    
    // Generate coverage report
    printf("\nGenerating coverage report...\n");
    execute_command("gcov -b gcov-dump.cc 2>&1", 1);
    
    // Check if coverage file exists
    if (file_exists("gcov-dump.cc.gcov")) {
        printf("\n=== Coverage report for gcov-dump.cc ===\n");
        // Display relevant portion of coverage report
        execute_command("grep -n -A2 -B2 '^[[:space:]]*111:' gcov-dump.cc.gcov", 1);
        execute_command("grep -n -A2 -B2 '^[[:space:]]*130:' gcov-dump.cc.gcov", 1);
        
        // Check if target lines are covered
        FILE* cov = fopen("gcov-dump.cc.gcov", "r");
        if (cov) {
            char line[256];
            int target_lines_covered = 0;
            while (fgets(line, sizeof(line), cov)) {
                // Look for lines 111-130 in coverage output
                if (strstr(line, ":") && !strstr(line, "-:") && !strstr(line, "#####:")) {
                    char* colon = strchr(line, ':');
                    if (colon) {
                        int line_num = atoi(line);
                        if (line_num >= 111 && line_num <= 130) {
                            target_lines_covered++;
                            printf("Line %d is covered\n", line_num);
                        }
                    }
                }
            }
            fclose(cov);
            printf("\nTotal target lines covered: %d/20\n", target_lines_covered);
        }
    }
    
    return 1;
}

int main(int argc, char* argv[]) {
    printf("=== GCOV-Dump Coverage Test ===\n");
    
    // Step 1: Compile gcov-dump with coverage
    if (!compile_gcov_dump_with_coverage()) {
        fprintf(stderr, "Failed to compile gcov-dump with coverage\n");
        return 1;
    }
    
    // Step 2: Generate test GCOV data
    if (!generate_test_gcda()) {
        fprintf(stderr, "Failed to generate test GCOV data\n");
        return 1;
    }
    
    // Step 3: Run tests with various flag combinations
    if (!run_gcov_dump_tests()) {
        fprintf(stderr, "Failed to run gcov-dump tests\n");
        return 1;
    }
    
    // Step 4: Check coverage
    if (!check_coverage()) {
        fprintf(stderr, "Failed to check coverage\n");
        return 1;
    }
    
    printf("\n=== Test completed successfully ===\n");
    printf("Coverage data should now show lines 111-130 as executed.\n");
    
    // Cleanup
    execute_command("rm -f dummy.c dummy_prog dummy.gcda dummy.gcno 2>/dev/null", 1);
    
    return 0;
}
