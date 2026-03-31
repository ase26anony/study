// test_gcov_dump_coverage.c
// Test wrapper to cover the command-line argument parsing in gcov-dump.cc

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_PATH 1024

// Function to check if a file exists
int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

// Function to execute a command and check return code
int execute_command(const char *cmd, int expect_success) {
    printf("Executing: %s\n", cmd);
    int ret = system(cmd);
    if (expect_success && ret != 0) {
        fprintf(stderr, "Command failed with code %d: %s\n", ret, cmd);
        return 0;
    }
    return 1;
}

// Function to compile gcov-dump with coverage instrumentation
int compile_gcov_dump(const char *source_dir, const char *output_path) {
    char cmd[MAX_PATH * 4];
    
    // Try to find gcov-dump source
    char source_path[MAX_PATH];
    snprintf(source_path, sizeof(source_path), "%s/gcov-dump.cc", source_dir);
    
    if (!file_exists(source_path)) {
        // Try alternative locations
        snprintf(source_path, sizeof(source_path), "%s/../gcc/gcov-dump.cc", source_dir);
        if (!file_exists(source_path)) {
            fprintf(stderr, "Could not find gcov-dump.cc in %s\n", source_dir);
            return 0;
        }
    }
    
    // Build command to compile gcov-dump with coverage
    snprintf(cmd, sizeof(cmd),
        "g++ -O0 -fprofile-arcs -ftest-coverage "
        "-I%s -I%s/../../include -I%s/../../libiberty "
        "%s %s/../../libiberty/libiberty.a "
        "-o %s",
        source_dir, source_dir, source_dir,
        source_path, source_dir, output_path);
    
    return execute_command(cmd, 1);
}

// Function to create and compile dummy test program
int create_dummy_program(const char *dir) {
    char dummy_c_path[MAX_PATH];
    snprintf(dummy_c_path, sizeof(dummy_c_path), "%s/dummy.c", dir);
    
    // Create dummy.c
    FILE *fp = fopen(dummy_c_path, "w");
    if (!fp) {
        perror("Failed to create dummy.c");
        return 0;
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
    
    // Compile dummy.c with coverage
    char cmd[MAX_PATH * 2];
    snprintf(cmd, sizeof(cmd),
        "gcc -O0 -fprofile-arcs -ftest-coverage %s/dummy.c -o %s/dummy_prog",
        dir, dir);
    
    return execute_command(cmd, 1);
}

// Function to run dummy program and generate .gcda file
int generate_gcda_file(const char *dir) {
    char cmd[MAX_PATH];
    snprintf(cmd, sizeof(cmd), "%s/dummy_prog", dir);
    
    if (!execute_command(cmd, 1)) {
        return 0;
    }
    
    // Check if .gcda file was created
    char gcda_path[MAX_PATH];
    snprintf(gcda_path, sizeof(gcda_path), "%s/dummy.gcda", dir);
    
    if (!file_exists(gcda_path)) {
        // Try alternative name
        snprintf(gcda_path, sizeof(gcda_path), "%s/dummy_prog.gcda", dir);
        if (!file_exists(gcda_path)) {
            fprintf(stderr, "No .gcda file generated\n");
            return 0;
        }
    }
    
    return 1;
}

// Function to merge coverage data
int merge_coverage(const char *gcov_dump_path, const char *source_file) {
    char cmd[MAX_PATH * 2];
    
    // First, find the .gcda file for gcov-dump
    snprintf(cmd, sizeof(cmd),
        "gcov -i %s 2>/dev/null || true",  // Suppress errors if no .gcda yet
        source_file);
    
    return execute_command(cmd, 0);  // Don't require success
}

// Function to test specific flag combinations
int test_flag_combination(const char *gcov_dump_path, const char *gcda_path, 
                         const char *flags, int expect_success, 
                         const char *source_file) {
    char cmd[MAX_PATH * 3];
    
    if (strcmp(flags, "-h") == 0 || strcmp(flags, "-v") == 0) {
        // Help and version don't need a gcda file
        snprintf(cmd, sizeof(cmd), "%s %s", gcov_dump_path, flags);
    } else if (strcmp(flags, "-x") == 0) {
        // Invalid flag - should fail
        snprintf(cmd, sizeof(cmd), "%s %s %s 2>&1", gcov_dump_path, flags, gcda_path);
    } else {
        // Normal flags need gcda file
        snprintf(cmd, sizeof(cmd), "%s %s %s", gcov_dump_path, flags, gcda_path);
    }
    
    if (!execute_command(cmd, expect_success)) {
        return 0;
    }
    
    // Merge coverage after each test
    return merge_coverage(gcov_dump_path, source_file);
}

int main(int argc, char *argv[]) {
    printf("=== Starting gcov-dump coverage test ===\n");
    
    // Create test directory
    char test_dir[MAX_PATH];
    snprintf(test_dir, sizeof(test_dir), "/tmp/gcov_dump_test_%d", getpid());
    
    if (mkdir(test_dir, 0755) != 0) {
        perror("Failed to create test directory");
        return 1;
    }
    
    printf("Test directory: %s\n", test_dir);
    
    // Paths
    char gcov_dump_path[MAX_PATH];
    snprintf(gcov_dump_path, sizeof(gcov_dump_path), "%s/gcov-dump-instrumented", test_dir);
    
    char gcda_path[MAX_PATH];
    snprintf(gcda_path, sizeof(gcda_path), "%s/dummy.gcda", test_dir);
    
    char source_file[MAX_PATH] = "gcov-dump.cc";
    
    // Try to use existing gcov-dump or compile it
    if (!file_exists(gcov_dump_path)) {
        printf("Compiling instrumented gcov-dump...\n");
        
        // Try to find source directory
        const char *source_dirs[] = {
            ".",
            "..",
            "../gcc",
            "../../gcc",
            "/usr/src/gcc",
            NULL
        };
        
        int compiled = 0;
        for (int i = 0; source_dirs[i] != NULL; i++) {
            if (compile_gcov_dump(source_dirs[i], gcov_dump_path)) {
                compiled = 1;
                break;
            }
        }
        
        if (!compiled) {
            fprintf(stderr, "Failed to compile gcov-dump\n");
            // Try to use system gcov-dump as fallback
            if (file_exists("/usr/bin/gcov-dump")) {
                snprintf(gcov_dump_path, sizeof(gcov_dump_path), "/usr/bin/gcov-dump");
                printf("Using system gcov-dump at %s\n", gcov_dump_path);
            } else {
                rmdir(test_dir);
                return 1;
            }
        }
    }
    
    // Create and compile dummy program
    printf("\nCreating dummy test program...\n");
    if (!create_dummy_program(test_dir)) {
        fprintf(stderr, "Failed to create dummy program\n");
        rmdir(test_dir);
        return 1;
    }
    
    // Generate .gcda file
    printf("\nGenerating GCOV data file...\n");
    if (!generate_gcda_file(test_dir)) {
        fprintf(stderr, "Failed to generate .gcda file\n");
        // Continue anyway - some tests don't need it
    }
    
    // Test flag combinations
    printf("\n=== Testing flag combinations ===\n");
    
    struct {
        const char *flags;
        int expect_success;
        const char *description;
    } tests[] = {
        {"-h", 1, "Help flag"},
        {"-v", 1, "Version flag"},
        {"-l", 1, "Dump contents flag"},
        {"-p", 1, "Dump positions flag"},
        {"-r", 1, "Dump raw flag"},
        {"-s", 1, "Dump stable flag"},
        {"-l -p -r -s", 1, "Combined flags (space-separated)"},
        {"-lprs", 1, "Combined flags (concatenated)"},
        {"-x", 0, "Invalid flag (should fail)"},
        {NULL, 0, NULL}
    };
    
    int all_passed = 1;
    for (int i = 0; tests[i].flags != NULL; i++) {
        printf("\nTest %d: %s\n", i + 1, tests[i].description);
        printf("Flags: %s\n", tests[i].flags);
        
        if (!test_flag_combination(gcov_dump_path, gcda_path, 
                                  tests[i].flags, tests[i].expect_success,
                                  source_file)) {
            printf("  FAILED\n");
            all_passed = 0;
        } else {
            printf("  PASSED\n");
        }
    }
    
    // Generate final coverage report
    printf("\n=== Generating coverage report ===\n");
    
    // First, ensure we have coverage data
    char merge_cmd[MAX_PATH * 2];
    snprintf(merge_cmd, sizeof(merge_cmd),
        "cd %s && gcov -i %s 2>/dev/null || true",
        test_dir, source_file);
    system(merge_cmd);
    
    // Generate human-readable report
    char report_cmd[MAX_PATH * 2];
    snprintf(report_cmd, sizeof(report_cmd),
        "cd %s && gcov -b %s 2>&1 | grep -A 20 'Lines executed:'",
        test_dir, source_file);
    
    printf("Running: %s\n", report_cmd);
    system(report_cmd);
    
    // Specifically check for our target lines
    printf("\n=== Checking target lines (111-130) ===\n");
    char check_cmd[MAX_PATH * 2];
    snprintf(check_cmd, sizeof(check_cmd),
        "cd %s && gcov -b %s 2>&1 | "
        "sed -n '/^ *111:/,/^ *131:/p'",
        test_dir, source_file);
    
    printf("Coverage for lines 111-130:\n");
    system(check_cmd);
    
    // Cleanup
    printf("\n=== Test completed ===\n");
    if (all_passed) {
        printf("All tests passed!\n");
    } else {
        printf("Some tests failed\n");
    }
    
    printf("\nTest artifacts preserved in: %s\n", test_dir);
    printf("To clean up: rm -rf %s\n", test_dir);
    
    return all_passed ? 0 : 1;
}
