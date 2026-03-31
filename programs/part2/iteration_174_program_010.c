#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_PATH 1024

// Simple dummy program to generate .gcda files
const char *dummy_source = 
"#include <stdio.h>\n"
"int main() {\n"
"    int i;\n"
"    for (i = 0; i < 10; i++) {\n"
"        printf(\"Value: %d\\n\", i);\n"
"    }\n"
"    return 0;\n"
"}";

// Function to check if a file exists
int file_exists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

// Function to compile gcov-dump with coverage instrumentation
int compile_gcov_dump_with_coverage() {
    printf("Compiling gcov-dump with coverage instrumentation...\n");
    
    // Try to find gcov-dump source in common locations
    const char *source_paths[] = {
        "gcov-dump.cc",
        "../gcc/gcov-dump.cc",
        "../../gcc/gcov-dump.cc",
        "/usr/src/gcc/gcc/gcov-dump.cc",
        NULL
    };
    
    const char *source_file = NULL;
    for (int i = 0; source_paths[i] != NULL; i++) {
        if (file_exists(source_paths[i])) {
            source_file = source_paths[i];
            break;
        }
    }
    
    if (!source_file) {
        fprintf(stderr, "Error: Could not find gcov-dump source file\n");
        return 0;
    }
    
    // Build command to compile gcov-dump with coverage
    char cmd[MAX_PATH * 4];
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include "
        "-I../../libiberty %s ../../libiberty/libiberty.a "
        "-o gcov-dump-instrumented",
        source_file);
    
    printf("Compiling with: %s\n", cmd);
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Compilation failed with code %d\n", result);
        return 0;
    }
    
    return file_exists("gcov-dump-instrumented");
}

// Function to create and compile dummy test program
int create_dummy_gcda() {
    printf("Creating dummy program for GCOV data generation...\n");
    
    // Write dummy source file
    FILE *fp = fopen("dummy.c", "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return 0;
    }
    fputs(dummy_source, fp);
    fclose(fp);
    
    // Compile dummy program with coverage
    int result = system("gcc -O0 -fprofile-arcs -ftest-coverage dummy.c -o dummy_prog");
    if (result != 0) {
        fprintf(stderr, "Failed to compile dummy program\n");
        return 0;
    }
    
    // Run dummy program to generate .gcda file
    result = system("./dummy_prog > /dev/null 2>&1");
    if (result != 0) {
        fprintf(stderr, "Failed to run dummy program\n");
        return 0;
    }
    
    return file_exists("dummy.gcda");
}

// Function to merge coverage data
void merge_coverage_data() {
    // Use gcov to merge coverage data
    system("gcov -i gcov-dump.cc > /dev/null 2>&1");
    
    // Alternative: copy .gcda files to preserve them
    static int run_count = 0;
    char cmd[MAX_PATH];
    snprintf(cmd, sizeof(cmd), "cp gcov-dump-instrumented.gcda gcov-dump-instrumented.gcda.%d 2>/dev/null", run_count++);
    system(cmd);
}

// Function to run gcov-dump with specific arguments
int run_gcov_dump(const char *args, int expect_success) {
    char cmd[MAX_PATH * 2];
    snprintf(cmd, sizeof(cmd), "./gcov-dump-instrumented %s", args);
    
    printf("Running: %s\n", cmd);
    
    // Use fork/exec to capture exit status
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        char *argv[20];
        int argc = 0;
        
        // Parse command line
        char *token = strtok(strdup(cmd), " ");
        while (token != NULL && argc < 19) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
        argv[argc] = NULL;
        
        execvp(argv[0], argv);
        perror("execvp failed");
        exit(1);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        int exit_code = WEXITSTATUS(status);
        printf("Exit code: %d\n", exit_code);
        
        // Merge coverage after each run
        merge_coverage_data();
        
        if (expect_success) {
            return (exit_code == 0);
        } else {
            return (exit_code != 0);
        }
    }
    
    return 0;
}

// Function to check if target lines are covered
void check_coverage() {
    printf("\n=== Checking Coverage ===\n");
    
    // Generate coverage report
    system("gcov -b gcov-dump.cc > coverage_report.txt 2>&1");
    
    // Display relevant parts of coverage report
    system("grep -A5 -B5 'lines 111-130' coverage_report.txt 2>/dev/null || "
           "echo 'Could not find specific line range in report'");
    
    // Check for the specific switch cases
    printf("\nLooking for coverage of specific flags:\n");
    system("grep -n 'flag_dump_contents = 1' gcov-dump.cc");
    system("grep -n 'flag_dump_positions = 1' gcov-dump.cc");
    system("grep -n 'flag_dump_raw = 1' gcov-dump.cc");
    system("grep -n 'flag_dump_stable = 1' gcov-dump.cc");
    system("grep -n 'unknown flag' gcov-dump.cc");
    
    // Show summary
    printf("\n=== Coverage Summary ===\n");
    system("tail -20 coverage_report.txt 2>/dev/null");
}

int main() {
    printf("=== GCOV-Dump Coverage Test ===\n\n");
    
    // Step 1: Build instrumented gcov-dump
    if (!compile_gcov_dump_with_coverage()) {
        fprintf(stderr, "Failed to build instrumented gcov-dump\n");
        return 1;
    }
    
    // Step 2: Generate test GCOV data
    if (!create_dummy_gcda()) {
        fprintf(stderr, "Failed to create dummy .gcda file\n");
        return 1;
    }
    
    // Step 3: Execute comprehensive flag coverage tests
    printf("\n=== Running Flag Coverage Tests ===\n");
    
    int all_tests_passed = 1;
    
    // Test help flag (should succeed)
    printf("\n1. Testing -h flag:\n");
    if (!run_gcov_dump("-h", 1)) {
        printf("FAIL: -h flag test\n");
        all_tests_passed = 0;
    }
    
    // Test version flag (should succeed)
    printf("\n2. Testing -v flag:\n");
    if (!run_gcov_dump("-v", 1)) {
        printf("FAIL: -v flag test\n");
        all_tests_passed = 0;
    }
    
    // Test individual flags with data file
    printf("\n3. Testing -l flag with data file:\n");
    if (!run_gcov_dump("-l dummy.gcda", 1)) {
        printf("FAIL: -l flag test\n");
        all_tests_passed = 0;
    }
    
    printf("\n4. Testing -p flag with data file:\n");
    if (!run_gcov_dump("-p dummy.gcda", 1)) {
        printf("FAIL: -p flag test\n");
        all_tests_passed = 0;
    }
    
    printf("\n5. Testing -r flag with data file:\n");
    if (!run_gcov_dump("-r dummy.gcda", 1)) {
        printf("FAIL: -r flag test\n");
        all_tests_passed = 0;
    }
    
    printf("\n6. Testing -s flag with data file:\n");
    if (!run_gcov_dump("-s dummy.gcda", 1)) {
        printf("FAIL: -s flag test\n");
        all_tests_passed = 0;
    }
    
    // Test combined flags (space-separated)
    printf("\n7. Testing combined flags -l -p -r -s:\n");
    if (!run_gcov_dump("-l -p -r -s dummy.gcda", 1)) {
        printf("FAIL: Combined flags test\n");
        all_tests_passed = 0;
    }
    
    // Test concatenated flags
    printf("\n8. Testing concatenated flags -lprs:\n");
    if (!run_gcov_dump("-lprs dummy.gcda", 1)) {
        printf("FAIL: Concatenated flags test\n");
        all_tests_passed = 0;
    }
    
    // Test invalid flag (should fail)
    printf("\n9. Testing invalid flag -x:\n");
    if (!run_gcov_dump("-x dummy.gcda", 0)) {
        printf("FAIL: Invalid flag test (should have failed but didn't)\n");
        all_tests_passed = 0;
    }
    
    // Additional test: multiple data files
    printf("\n10. Testing with multiple data files:\n");
    system("cp dummy.gcda dummy2.gcda");
    if (!run_gcov_dump("-l dummy.gcda dummy2.gcda", 1)) {
        printf("FAIL: Multiple files test\n");
        all_tests_passed = 0;
    }
    
    // Step 4: Check coverage
    check_coverage();
    
    // Step 5: Cleanup (optional)
    printf("\n=== Cleanup ===\n");
    char *keep_files = getenv("KEEP_TEST_FILES");
    if (!keep_files || strcmp(keep_files, "1") != 0) {
        system("rm -f dummy.c dummy_prog dummy.gcda dummy2.gcda "
               "gcov-dump-instrumented.gcda.* coverage_report.txt "
               "*.gcov 2>/dev/null");
    }
    
    printf("\n=== Test %s ===\n", all_tests_passed ? "PASSED" : "FAILED");
    return all_tests_passed ? 0 : 1;
}
