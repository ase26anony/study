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
#define TEMP_DIR_PATTERN "/tmp/gcov_test_XXXXXX"

typedef struct {
    char *command;
    int expected_exit;
    char *description;
} test_case_t;

/* Create a simple C program for GCOV instrumentation */
const char *test_program = 
    "#include <stdio.h>\n"
    "int main() {\n"
    "    int i;\n"
    "    for (i = 0; i < 10; i++) {\n"
    "        printf(\"Test %d\\n\", i);\n"
    "    }\n"
    "    return 0;\n"
    "}\n";

/* Create a second program for overlap comparison */
const char *test_program2 = 
    "#include <stdio.h>\n"
    "int main() {\n"
    "    int i;\n"
    "    for (i = 0; i < 5; i++) {\n"
    "        printf(\"Test2 %d\\n\", i);\n"
    "    }\n"
    "    return 0;\n"
    "}\n";

/* Execute a shell command and return exit status */
int run_command(const char *cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Create a temporary directory */
char *create_temp_dir() {
    char *template = strdup(TEMP_DIR_PATTERN);
    if (mkdtemp(template) == NULL) {
        perror("mkdtemp failed");
        free(template);
        return NULL;
    }
    return template;
}

/* Clean up temporary directory */
void cleanup_temp_dir(const char *dir) {
    if (dir) {
        char cmd[MAX_CMD_LEN];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", dir);
        system(cmd);
    }
}

/* Generate GCOV data files */
int generate_gcov_data(const char *dir, int file_count) {
    char path[MAX_CMD_LEN];
    int i;
    
    /* Write test programs */
    snprintf(path, sizeof(path), "%s/test1.c", dir);
    FILE *f = fopen(path, "w");
    if (!f) return -1;
    fputs(test_program, f);
    fclose(f);
    
    snprintf(path, sizeof(path), "%s/test2.c", dir);
    f = fopen(path, "w");
    if (!f) return -1;
    fputs(test_program2, f);
    fclose(f);
    
    /* Compile with GCOV instrumentation */
    snprintf(path, sizeof(path), 
             "cd %s && gcc -fprofile-arcs -ftest-coverage -O0 test1.c -o test1", dir);
    if (run_command(path) != 0) {
        fprintf(stderr, "Failed to compile test1.c\n");
        return -1;
    }
    
    snprintf(path, sizeof(path), 
             "cd %s && gcc -fprofile-arcs -ftest-coverage -O0 test2.c -o test2", dir);
    if (run_command(path) != 0) {
        fprintf(stderr, "Failed to compile test2.c\n");
        return -1;
    }
    
    /* Run programs to generate .gcda files */
    for (i = 0; i < file_count; i++) {
        snprintf(path, sizeof(path), "cd %s && ./test%d > /dev/null", dir, i % 2 + 1);
        if (run_command(path) != 0) {
            fprintf(stderr, "Failed to run test%d\n", i % 2 + 1);
            return -1;
        }
    }
    
    return 0;
}

/* Run a single test case */
int run_test_case(const char *dir, const test_case_t *test) {
    char cmd[MAX_CMD_LEN];
    char gcda_files[MAX_CMD_LEN];
    
    /* Build the list of .gcda files */
    snprintf(gcda_files, sizeof(gcda_files), 
             "%s/test1.gcda %s/test2.gcda", dir, dir);
    
    /* Construct the full command */
    snprintf(cmd, sizeof(cmd), "gcov-tool %s %s", test->command, gcda_files);
    
    printf("\n=== Test: %s ===\n", test->description);
    printf("Command: %s\n", cmd);
    
    int exit_code = run_command(cmd);
    
    printf("Exit code: %d (expected: %d)\n", exit_code, test->expected_exit);
    
    /* For invalid commands, we accept any non-zero exit */
    if (test->expected_exit == 0) {
        return (exit_code == 0) ? 1 : 0;
    } else {
        return (exit_code != 0) ? 1 : 0;
    }
}

int main(int argc, char *argv[]) {
    char *temp_dir = NULL;
    int passed = 0;
    int total = 0;
    
    printf("=== GCOV-Tool Overlap Parser Test ===\n");
    
    /* Create temporary directory */
    temp_dir = create_temp_dir();
    if (!temp_dir) {
        fprintf(stderr, "Failed to create temp directory\n");
        return 1;
    }
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Generate GCOV data files */
    if (generate_gcov_data(temp_dir, 3) != 0) {
        fprintf(stderr, "Failed to generate GCOV data\n");
        cleanup_temp_dir(temp_dir);
        free(temp_dir);
        return 1;
    }
    
    /* Define test cases covering all uncovered lines */
    test_case_t tests[] = {
        /* Basic flag combinations - all should succeed */
        {"overlap -v", 0, "Verbose flag"},
        {"overlap -f", 0, "Function level flag"},
        {"overlap -F", 0, "Full filename flag"},
        {"overlap -o", 0, "Object level flag"},
        {"overlap -h", 0, "Hot only flag"},
        {"overlap -t 0.5", 0, "Hot threshold 0.5"},
        {"overlap -t 1.0", 0, "Hot threshold 1.0"},
        {"overlap -t 0.75", 0, "Hot threshold 0.75"},
        
        /* Combined flags - test all uncovered cases in one command */
        {"overlap -v -f -F -o -h -t 0.8", 0, "All flags combined"},
        
        /* Permutations of flag order */
        {"overlap -f -v -F -o -h -t 0.6", 0, "Flags in different order 1"},
        {"overlap -t 0.9 -h -o -F -f -v", 0, "Flags in different order 2"},
        {"overlap -h -t 0.3 -v -f -F -o", 0, "Flags in different order 3"},
        
        /* Repeated flags */
        {"overlap -v -v -v", 0, "Repeated verbose flag"},
        {"overlap -f -f -t 0.5 -f", 0, "Repeated function flag"},
        
        /* Edge cases for -t flag */
        {"overlap -t 0", 0, "Zero threshold"},
        {"overlap -t 100.5", 0, "Large threshold"},
        {"overlap -t .5", 0, "Threshold without leading zero"},
        {"overlap -t 1e-3", 0, "Scientific notation threshold"},
        
        /* Invalid cases - should fail */
        {"overlap -t", 1, "Missing threshold value (should fail)"},
        {"overlap -t not_a_number", 1, "Invalid threshold (should fail)"},
        {"overlap -x", 1, "Unknown flag (should trigger default case)"},
        {"overlap -t 0.5 -x", 1, "Valid then invalid flag"},
        
        /* Mixed valid files and flags */
        {"overlap -v -f", 0, "Verbose + function level"},
        {"overlap -F -o -t 0.7", 0, "Fullname + object + threshold"},
        {"overlap -h -t 0.1 -v", 0, "Hot only + low threshold + verbose"},
        
        /* Empty flags (just overlap command) */
        {"overlap", 0, "No flags, just overlap subcommand"},
    };
    
    /* Run all test cases */
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    
    for (int i = 0; i < num_tests; i++) {
        total++;
        if (run_test_case(temp_dir, &tests[i])) {
            printf("✓ PASSED\n");
            passed++;
        } else {
            printf("✗ FAILED\n");
        }
        printf("\n");
    }
    
    /* Additional test: Create a test with environment variable to prevent optimization */
    char *env_test = getenv("GCOV_TEST_EXTRA");
    if (env_test) {
        char dynamic_cmd[MAX_CMD_LEN];
        snprintf(dynamic_cmd, sizeof(dynamic_cmd), "overlap %s", env_test);
        test_case_t dynamic_test = {dynamic_cmd, 0, "Dynamic from environment"};
        total++;
        if (run_test_case(temp_dir, &dynamic_test)) {
            printf("✓ DYNAMIC TEST PASSED\n");
            passed++;
        } else {
            printf("✗ DYNAMIC TEST FAILED\n");
        }
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d\n", passed, total);
    printf("Failed: %d\n", total - passed);
    
    /* Cleanup */
    cleanup_temp_dir(temp_dir);
    free(temp_dir);
    
    return (passed == total) ? 0 : 1;
}
