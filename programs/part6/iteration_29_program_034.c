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

/* Create a simple C program for GCOV instrumentation */
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

/* Create a second program with different execution pattern */
const char *test_program2 = 
"#include <stdio.h>\n"
"int main() {\n"
"    int i;\n"
"    for (i = 0; i < 5; i++) {\n"
"        printf(\"Count: %d\\n\", i);\n"
"    }\n"
"    return 0;\n"
"}\n";

/* Execute a shell command and return exit status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Create a temporary directory for test files */
char* create_temp_dir() {
    char *template = "/tmp/gcov_test_XXXXXX";
    char *dir = strdup(template);
    if (mkdtemp(dir) == NULL) {
        perror("mkdtemp failed");
        free(dir);
        return NULL;
    }
    printf("Created temp directory: %s\n", dir);
    return dir;
}

/* Clean up temporary directory */
void cleanup_temp_dir(const char *dir) {
    if (dir) {
        char cmd[MAX_CMD_LEN];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", dir);
        system(cmd);
        printf("Cleaned up: %s\n", dir);
    }
}

/* Compile a program with GCOV instrumentation */
int compile_with_gcov(const char *dir, const char *prog_name, const char *source) {
    char src_path[MAX_CMD_LEN];
    char exe_path[MAX_CMD_LEN];
    
    snprintf(src_path, sizeof(src_path), "%s/%s.c", dir, prog_name);
    snprintf(exe_path, sizeof(exe_path), "%s/%s", dir, prog_name);
    
    /* Write source file */
    FILE *fp = fopen(src_path, "w");
    if (!fp) {
        perror("Failed to create source file");
        return -1;
    }
    fprintf(fp, "%s", source);
    fclose(fp);
    
    /* Compile with GCOV instrumentation */
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s 2>/dev/null", 
             exe_path, src_path);
    
    return execute_command(cmd);
}

/* Run a program to generate .gcda files */
int run_program(const char *dir, const char *prog_name) {
    char exe_path[MAX_CMD_LEN];
    snprintf(exe_path, sizeof(exe_path), "%s/%s", dir, prog_name);
    
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "cd %s && ./%s >/dev/null 2>&1", dir, prog_name);
    
    return execute_command(cmd);
}

/* Generate test cases for the uncovered switch cases */
void generate_test_cases(test_case_t *tests, const char *dir, 
                         const char *gcda1, const char *gcda2, int *count) {
    int idx = 0;
    
    /* Base command with all flags from uncovered lines */
    char base_cmd[MAX_CMD_LEN];
    snprintf(base_cmd, sizeof(base_cmd), 
             "gcov-tool overlap -v -f -F -o -h -t 0.75 %s/%s %s/%s",
             dir, gcda1, dir, gcda2);
    
    /* Test 1: All flags together (covers all case statements) */
    tests[idx].cmd = strdup(base_cmd);
    tests[idx].expected_exit = 0;
    tests[idx].description = "All flags: -v -f -F -o -h -t 0.75";
    idx++;
    
    /* Test 2: Different order of flags */
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -t 0.5 -h -o -F -f -v %s/%s %s/%s",
             dir, gcda1, dir, gcda2);
    tests[idx].cmd = strdup(cmd);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Flags in reverse order";
    idx++;
    
    /* Test 3: Only verbose flag */
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -v %s/%s %s/%s",
             dir, gcda1, dir, gcda2);
    tests[idx].cmd = strdup(cmd);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Only -v flag";
    idx++;
    
    /* Test 4: Function level and fullname flags */
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -f -F %s/%s %s/%s",
             dir, gcda1, dir, gcda2);
    tests[idx].cmd = strdup(cmd);
    tests[idx].expected_exit = 0;
    tests[idx].description = "-f and -F flags";
    idx++;
    
    /* Test 5: Object level and hot only flags */
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -o -h %s/%s %s/%s",
             dir, gcda1, dir, gcda2);
    tests[idx].cmd = strdup(cmd);
    tests[idx].expected_exit = 0;
    tests[idx].description = "-o and -h flags";
    idx++;
    
    /* Test 6: Different threshold values */
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -t 0.1 %s/%s %s/%s",
             dir, gcda1, dir, gcda2);
    tests[idx].cmd = strdup(cmd);
    tests[idx].expected_exit = 0;
    tests[idx].description = "-t 0.1 threshold";
    idx++;
    
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -t 1.0 %s/%s %s/%s",
             dir, gcda1, dir, gcda2);
    tests[idx].cmd = strdup(cmd);
    tests[idx].expected_exit = 0;
    tests[idx].description = "-t 1.0 threshold";
    idx++;
    
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -t 99.9 %s/%s %s/%s",
             dir, gcda1, dir, gcda2);
    tests[idx].cmd = strdup(cmd);
    tests[idx].expected_exit = 0;
    tests[idx].description = "-t 99.9 threshold";
    idx++;
    
    /* Test 7: Repeated flags */
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -v -v -f -f %s/%s %s/%s",
             dir, gcda1, dir, gcda2);
    tests[idx].cmd = strdup(cmd);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Repeated -v and -f flags";
    idx++;
    
    /* Test 8: Edge case - invalid argument for -t */
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -t not_a_number %s/%s %s/%s",
             dir, gcda1, dir, gcda2);
    tests[idx].cmd = strdup(cmd);
    tests[idx].expected_exit = 1;  /* Expected to fail */
    tests[idx].description = "Invalid -t argument (not_a_number)";
    idx++;
    
    /* Test 9: Edge case - missing argument for -t */
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -t %s/%s %s/%s",
             dir, gcda1, dir, gcda2);
    tests[idx].cmd = strdup(cmd);
    tests[idx].expected_exit = 1;  /* Expected to fail */
    tests[idx].description = "Missing argument for -t flag";
    idx++;
    
    /* Test 10: Unknown flag (should trigger default case) */
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -x %s/%s %s/%s",
             dir, gcda1, dir, gcda2);
    tests[idx].cmd = strdup(cmd);
    tests[idx].expected_exit = 1;  /* Expected to fail */
    tests[idx].description = "Unknown flag -x (triggers default case)";
    idx++;
    
    /* Test 11: Combination with unknown flag */
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -v -x -f %s/%s %s/%s",
             dir, gcda1, dir, gcda2);
    tests[idx].cmd = strdup(cmd);
    tests[idx].expected_exit = 1;  /* Expected to fail */
    tests[idx].description = "Valid flags with unknown -x";
    idx++;
    
    /* Test 12: No flags at all (just overlap command) */
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap %s/%s %s/%s",
             dir, gcda1, dir, gcda2);
    tests[idx].cmd = strdup(cmd);
    tests[idx].expected_exit = 0;
    tests[idx].description = "No flags, just overlap command";
    idx++;
    
    /* Test 13: Using absolute paths */
    char abs_path1[MAX_CMD_LEN], abs_path2[MAX_CMD_LEN];
    realpath(dir, abs_path1);
    snprintf(abs_path2, sizeof(abs_path2), "%s/%s", abs_path1, gcda1);
    char abs_path3[MAX_CMD_LEN];
    snprintf(abs_path3, sizeof(abs_path3), "%s/%s", abs_path1, gcda2);
    
    snprintf(cmd, sizeof(cmd), 
             "gcov-tool overlap -v -f -F %s %s",
             abs_path2, abs_path3);
    tests[idx].cmd = strdup(cmd);
    tests[idx].expected_exit = 0;
    tests[idx].description = "Absolute paths with flags";
    idx++;
    
    *count = idx;
}

/* Run all test cases */
void run_tests(test_case_t *tests, int count) {
    int passed = 0;
    int failed = 0;
    
    printf("\n=== Running %d test cases ===\n\n", count);
    
    for (int i = 0; i < count; i++) {
        printf("Test %d: %s\n", i + 1, tests[i].description);
        printf("Command: %s\n", tests[i].cmd);
        
        int exit_code = execute_command(tests[i].cmd);
        
        if (tests[i].expected_exit == 0) {
            if (exit_code == 0) {
                printf("✓ PASSED (exit code: %d)\n\n", exit_code);
                passed++;
            } else {
                printf("✗ FAILED - Expected 0, got %d\n\n", exit_code);
                failed++;
            }
        } else {
            if (exit_code != 0) {
                printf("✓ PASSED - Expected non-zero, got %d\n\n", exit_code);
                passed++;
            } else {
                printf("✗ FAILED - Expected non-zero, got 0\n\n", exit_code);
                failed++;
            }
        }
        
        /* Small delay to avoid overwhelming the system */
        usleep(100000);
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total:  %d\n", count);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    
    if (failed == 0) {
        printf("\n✓ All tests passed!\n");
    } else {
        printf("\n✗ Some tests failed\n");
    }
}

/* Free allocated memory */
void cleanup_tests(test_case_t *tests, int count) {
    for (int i = 0; i < count; i++) {
        free(tests[i].cmd);
    }
}

int main(int argc, char *argv[]) {
    char *temp_dir = NULL;
    test_case_t tests[50];
    int test_count = 0;
    
    printf("=== GCOV-Tool Overlap Parser Test ===\n");
    
    /* Create temporary directory */
    temp_dir = create_temp_dir();
    if (!temp_dir) {
        fprintf(stderr, "Failed to create temp directory\n");
        return 1;
    }
    
    /* Compile test programs with GCOV instrumentation */
    printf("\nCompiling test programs with GCOV instrumentation...\n");
    
    if (compile_with_gcov(temp_dir, "test1", test_program) != 0) {
        fprintf(stderr, "Failed to compile test1\n");
        cleanup_temp_dir(temp_dir);
        free(temp_dir);
        return 1;
    }
    
    if (compile_with_gcov(temp_dir, "test2", test_program2) != 0) {
        fprintf(stderr, "Failed to compile test2\n");
        cleanup_temp_dir(temp_dir);
        free(temp_dir);
        return 1;
    }
    
    /* Run programs to generate .gcda files */
    printf("Running programs to generate .gcda files...\n");
    
    if (run_program(temp_dir, "test1") != 0) {
        fprintf(stderr, "Failed to run test1\n");
        cleanup_temp_dir(temp_dir);
        free(temp_dir);
        return 1;
    }
    
    if (run_program(temp_dir, "test2") != 0) {
        fprintf(stderr, "Failed to run test2\n");
        cleanup_temp_dir(temp_dir);
        free(temp_dir);
        return 1;
    }
    
    /* Run test1 multiple times to get different profile data */
    printf("Running test1 again for different profile...\n");
    if (run_program(temp_dir, "test1") != 0) {
        fprintf(stderr, "Failed to run test1 again\n");
    }
    
    /* Find .gcda files */
    char find_cmd[MAX_CMD_LEN];
    snprintf(find_cmd, sizeof(find_cmd), "find %s -name '*.gcda' 2>/dev/null", temp_dir);
    
    FILE *fp = popen(find_cmd, "r");
    if (!fp) {
        fprintf(stderr, "Failed to find .gcda files\n");
        cleanup_temp_dir(temp_dir);
        free(temp_dir);
        return 1;
    }
    
    char gcda_files[MAX_FILES][MAX_CMD_LEN];
    int file_count = 0;
    
    while (fgets(gcda_files[file_count], MAX_CMD_LEN, fp) && file_count < MAX_FILES) {
        /* Remove newline */
        gcda_files[file_count][strcspn(gcda_files[file_count], "\n")] = 0;
        printf("Found GCOV data: %s\n", gcda_files[file_count]);
        file_count++;
    }
    pclose(fp);
    
    if (file_count < 2) {
        fprintf(stderr, "Need at least 2 .gcda files for overlap analysis\n");
        cleanup_temp_dir(temp_dir);
        free(temp_dir);
        return 1;
    }
    
    /* Extract just the filenames from paths */
    char *gcda1 = strrchr(gcda_files[0], '/');
    char *gcda2 = strrchr(gcda_files[1], '/');
    if (!gcda1 || !gcda2) {
        gcda1 = gcda_files[0];
        gcda2 = gcda_files[1];
    } else {
        gcda1++;  /* Skip the '/' */
        gcda2++;
    }
    
    /* Generate test cases */
    printf("\nGenerating test cases...\n");
    generate_test_cases(tests, temp_dir, gcda1, gcda2, &test_count);
    
    /* Run tests */
    run_tests(tests, test_count);
    
    /* Cleanup */
    cleanup_tests(tests, test_count);
    cleanup_temp_dir(temp_dir);
    free(temp_dir);
    
    printf("\n=== Test Complete ===\n");
    printf("To collect coverage data for gcov-tool.cc:\n");
    printf("1. Ensure gcov-tool was built with --enable-coverage\n");
    printf("2. Run: gcov gcov-tool.cc\n");
    printf("3. Check gcov-tool.cc.gcov for line coverage\n");
    
    return 0;
}
