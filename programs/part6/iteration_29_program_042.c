#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define MAX_FILES 10

typedef struct {
    char *cmd;
    int expected_exit;
    char *description;
} test_case_t;

// Create a simple C program for GCOV instrumentation
const char *test_program = 
"#include <stdio.h>\n"
"int main() {\n"
"    int i;\n"
"    for (i = 0; i < 10; i++) {\n"
"        if (i % 2 == 0) {\n"
"            printf(\"Even: %d\\n\", i);\n"
"        } else {\n"
"            printf(\"Odd: %d\\n\", i);\n"
"        }\n"
"    }\n"
"    return 0;\n"
"}\n";

// Create a second test program for multiple file comparison
const char *test_program2 = 
"#include <stdio.h>\n"
"int helper(int x) {\n"
"    return x * 2;\n"
"}\n"
"int main() {\n"
"    int i;\n"
"    for (i = 0; i < 5; i++) {\n"
"        printf(\"Value: %d\\n\", helper(i));\n"
"    }\n"
"    return 0;\n"
"}\n";

int compile_with_gcov(const char *source, const char *output) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s", 
             output, source);
    return system(cmd);
}

int run_program(const char *program) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "./%s > /dev/null 2>&1", program);
    return system(cmd);
}

int execute_test(const char *cmd, int expected_exit, const char *description) {
    printf("Test: %s\n", description);
    printf("Command: %s\n", cmd);
    
    int status = system(cmd);
    int exit_code = WEXITSTATUS(status);
    
    if (exit_code == expected_exit) {
        printf("✓ PASSED (exit code: %d)\n\n", exit_code);
        return 1;
    } else {
        printf("✗ FAILED (expected: %d, got: %d)\n\n", expected_exit, exit_code);
        return 0;
    }
}

void create_temp_dir(char *path, size_t size) {
    snprintf(path, size, "/tmp/gcov_test_%d", getpid());
    mkdir(path, 0755);
}

void cleanup_temp_dir(const char *path) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", path);
    system(cmd);
}

int main(int argc, char *argv[]) {
    char temp_dir[256];
    char source1[256], source2[256];
    char exec1[256], exec2[256];
    char gcda1[256], gcda2[256];
    char gcno1[256], gcno2[256];
    int passed = 0, total = 0;
    
    // Create temporary directory
    create_temp_dir(temp_dir, sizeof(temp_dir));
    printf("Created temp directory: %s\n", temp_dir);
    
    // Create source files
    snprintf(source1, sizeof(source1), "%s/test1.c", temp_dir);
    snprintf(source2, sizeof(source2), "%s/test2.c", temp_dir);
    
    FILE *fp = fopen(source1, "w");
    if (fp) {
        fputs(test_program, fp);
        fclose(fp);
    }
    
    fp = fopen(source2, "w");
    if (fp) {
        fputs(test_program2, fp);
        fclose(fp);
    }
    
    // Compile with GCOV instrumentation
    snprintf(exec1, sizeof(exec1), "%s/test1", temp_dir);
    snprintf(exec2, sizeof(exec2), "%s/test2", temp_dir);
    
    if (compile_with_gcov(source1, exec1) != 0) {
        fprintf(stderr, "Failed to compile test1.c\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    if (compile_with_gcov(source2, exec2) != 0) {
        fprintf(stderr, "Failed to compile test2.c\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    // Run programs to generate .gcda files
    chdir(temp_dir);
    run_program("test1");
    run_program("test2");
    
    // Run test1 again to get different coverage data
    system("rm -f test1.gcda");
    run_program("test1");
    
    // Get absolute paths for files
    char abs_gcda1[256], abs_gcda2[256];
    realpath("test1.gcda", abs_gcda1);
    realpath("test2.gcda", abs_gcda2);
    
    // Define test cases targeting the uncovered switch cases
    test_case_t tests[] = {
        // Basic flag combinations (all should succeed)
        {"gcov-tool overlap -v test1.gcda test2.gcda", 0, "Verbose flag (-v)"},
        {"gcov-tool overlap -f test1.gcda test2.gcda", 0, "Function level flag (-f)"},
        {"gcov-tool overlap -F test1.gcda test2.gcda", 0, "Full filename flag (-F)"},
        {"gcov-tool overlap -o test1.gcda test2.gcda", 0, "Object level flag (-o)"},
        {"gcov-tool overlap -h test1.gcda test2.gcda", 0, "Hot only flag (-h)"},
        {"gcov-tool overlap -t 0.5 test1.gcda test2.gcda", 0, "Threshold flag with value (-t 0.5)"},
        
        // Combined flags (testing all uncovered cases in one command)
        {"gcov-tool overlap -v -f -F -o -h -t 0.75 test1.gcda test2.gcda", 0, 
         "All flags combined: -v -f -F -o -h -t 0.75"},
        
        // Different threshold values
        {"gcov-tool overlap -t 0.1 test1.gcda test2.gcda", 0, "Low threshold (-t 0.1)"},
        {"gcov-tool overlap -t 1.0 test1.gcda test2.gcda", 0, "High threshold (-t 1.0)"},
        {"gcov-tool overlap -t 0.0 test1.gcda test2.gcda", 0, "Zero threshold (-t 0.0)"},
        {"gcov-tool overlap -t 0.999 test1.gcda test2.gcda", 0, "Fractional threshold (-t 0.999)"},
        
        // Flag permutations (different orders)
        {"gcov-tool overlap -f -v -o test1.gcda test2.gcda", 0, "Flag permutation: -f -v -o"},
        {"gcov-tool overlap -t 0.5 -h -F test1.gcda test2.gcda", 0, "Flag permutation: -t 0.5 -h -F"},
        {"gcov-tool overlap -o -F -f -v test1.gcda test2.gcda", 0, "Flag permutation: -o -F -f -v"},
        
        // With absolute paths
        {"gcov-tool overlap -v -f test1.gcda test2.gcda", 0, "With relative paths"},
        
        // Edge cases and error conditions
        {"gcov-tool overlap -t not_a_number test1.gcda test2.gcda", 1, 
         "Invalid threshold (should fail)"},
        {"gcov-tool overlap -x test1.gcda test2.gcda", 1, 
         "Unknown flag -x (should trigger default case)"},
        {"gcov-tool overlap -v -v test1.gcda test2.gcda", 0, 
         "Repeated verbose flag (-v -v)"},
        {"gcov-tool overlap -f -f -f test1.gcda test2.gcda", 0, 
         "Multiple repeated flags (-f -f -f)"},
        
        // Missing required argument for -t (last argument)
        {"gcov-tool overlap -t", 1, "Missing argument for -t (should fail)"},
        
        // Only flags, no input files (should fail)
        {"gcov-tool overlap -v -f", 1, "No input files provided"},
        
        // Single input file (minimum valid case)
        {"gcov-tool overlap -v test1.gcda", 0, "Single input file with verbose flag"},
        
        // Mixed with other valid gcov-tool options if any
        {"gcov-tool overlap --help", 0, "Help flag (--help)"},
    };
    
    // Execute all test cases
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    
    for (int i = 0; i < num_tests; i++) {
        total++;
        if (execute_test(tests[i].cmd, tests[i].expected_exit, tests[i].description)) {
            passed++;
        }
        
        // Add some delay to avoid overwhelming the system
        usleep(10000);
    }
    
    // Additional test: Generate commands dynamically with all permutations of flags
    printf("\n=== Testing flag permutations ===\n");
    
    // Base flags to permute
    const char *flags[] = {"-v", "-f", "-F", "-o", "-h", "-t 0.5"};
    int num_flags = 6;
    
    // Test different combinations (not all 2^6, but a representative sample)
    int combinations[] = {
        0b111111,  // All flags
        0b101010,  // Alternate flags
        0b010101,  // Other alternate
        0b100001,  // First and last
        0b011110,  // Middle four
    };
    
    for (int comb_idx = 0; comb_idx < 5; comb_idx++) {
        char cmd[MAX_CMD_LEN] = "gcov-tool overlap";
        int mask = combinations[comb_idx];
        
        for (int i = 0; i < num_flags; i++) {
            if (mask & (1 << i)) {
                strcat(cmd, " ");
                strcat(cmd, flags[i]);
            }
        }
        
        strcat(cmd, " test1.gcda test2.gcda");
        
        char desc[100];
        snprintf(desc, sizeof(desc), "Flag combination mask: 0x%02x", mask);
        
        total++;
        if (execute_test(cmd, 0, desc)) {
            passed++;
        }
    }
    
    // Test with environment variable to prevent optimization
    printf("\n=== Testing with environment variables ===\n");
    
    // Use environment variable to construct command (prevents compile-time optimization)
    char *gcov_tool_path = getenv("GCOV_TOOL_PATH");
    if (!gcov_tool_path) {
        gcov_tool_path = "gcov-tool";
    }
    
    char env_cmd[MAX_CMD_LEN];
    snprintf(env_cmd, sizeof(env_cmd), "%s overlap -v -f -t %f test1.gcda test2.gcda", 
             gcov_tool_path, 0.25);
    
    total++;
    if (execute_test(env_cmd, 0, "Command from environment variable")) {
        passed++;
    }
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);
    
    // Cleanup
    chdir("..");
    cleanup_temp_dir(temp_dir);
    
    return (passed == total) ? 0 : 1;
}
