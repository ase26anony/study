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

/* Create a second program to generate different profile data */
const char *test_program2 = 
"#include <stdio.h>\n"
"int main() {\n"
"    int i;\n"
"    for (i = 0; i < 5; i++) {\n"
"        printf(\"Count: %d\\n\", i);\n"
"    }\n"
"    return 0;\n"
"}\n";

/* Execute a command and return exit status */
int run_command(const char *cmd, int capture_output) {
    printf("Running: %s\n", cmd);
    
    if (capture_output) {
        char buffer[1024];
        FILE *fp = popen(cmd, "r");
        if (fp == NULL) {
            perror("popen failed");
            return -1;
        }
        
        printf("Output:\n");
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("  %s", buffer);
        }
        
        int status = pclose(fp);
        return WEXITSTATUS(status);
    } else {
        int status = system(cmd);
        return WEXITSTATUS(status);
    }
}

/* Create a temporary directory for test files */
char *create_temp_dir() {
    char *template = "/tmp/gcov_test_XXXXXX";
    char *dir = strdup(template);
    if (mkdtemp(dir) == NULL) {
        perror("Failed to create temp directory");
        free(dir);
        return NULL;
    }
    return dir;
}

/* Compile a test program with GCOV instrumentation */
int compile_with_gcov(const char *source, const char *output, const char *dir) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "cd %s && gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s",
             dir, output, source);
    return run_command(cmd, 0);
}

/* Run a program to generate .gcda files */
int run_program(const char *program, const char *dir) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "cd %s && ./%s > /dev/null 2>&1", dir, program);
    return run_command(cmd, 0);
}

/* Generate test cases for gcov-tool overlap command */
void generate_test_cases(test_case_t *tests, int *num_tests, 
                        const char *dir, const char *gcda1, const char *gcda2) {
    int i = 0;
    
    /* Base test with all flags in different orders */
    const char *flag_orders[] = {
        "-v -f -F -o -h -t 0.5",
        "-f -F -o -h -t 0.75 -v",
        "-t 1.0 -h -o -F -f -v",
        "-v -t 0.25 -f -F -o -h",
        "-o -h -t 0.9 -v -f -F",
    };
    
    for (int j = 0; j < 5; j++) {
        tests[i].cmd = malloc(MAX_CMD_LEN);
        snprintf(tests[i].cmd, MAX_CMD_LEN, 
                "gcov-tool overlap %s %s/%s %s/%s",
                flag_orders[j], dir, gcda1, dir, gcda2);
        tests[i].expected_exit = 0;
        tests[i].description = malloc(100);
        snprintf(tests[i].description, 100, "All flags in order: %s", flag_orders[j]);
        i++;
    }
    
    /* Individual flag tests */
    const char *individual_flags[] = {"-v", "-f", "-F", "-o", "-h", "-t 0.5"};
    const char *flag_desc[] = {"verbose", "function-level", "fullname", 
                               "object-level", "hot-only", "threshold"};
    
    for (int j = 0; j < 6; j++) {
        tests[i].cmd = malloc(MAX_CMD_LEN);
        snprintf(tests[i].cmd, MAX_CMD_LEN, 
                "gcov-tool overlap %s %s/%s %s/%s",
                individual_flags[j], dir, gcda1, dir, gcda2);
        tests[i].expected_exit = 0;
        tests[i].description = malloc(100);
        snprintf(tests[i].description, 100, "Single flag: %s", flag_desc[j]);
        i++;
    }
    
    /* Test with multiple input files */
    tests[i].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[i].cmd, MAX_CMD_LEN, 
            "gcov-tool overlap -v -f -t 0.3 %s/%s %s/%s %s/%s.gcno",
            dir, gcda1, dir, gcda2, dir, "test1");
    tests[i].expected_exit = 0;
    tests[i].description = "Multiple input files with .gcno";
    i++;
    
    /* Test with absolute paths */
    char abs_path1[MAX_CMD_LEN], abs_path2[MAX_CMD_LEN];
    snprintf(abs_path1, sizeof(abs_path1), "%s/%s", dir, gcda1);
    snprintf(abs_path2, sizeof(abs_path2), "%s/%s", dir, gcda2);
    
    tests[i].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[i].cmd, MAX_CMD_LEN, 
            "gcov-tool overlap -v -F -t 0.6 %s %s",
            abs_path1, abs_path2);
    tests[i].expected_exit = 0;
    tests[i].description = "Absolute paths with -F flag";
    i++;
    
    /* Edge case: invalid threshold value */
    tests[i].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[i].cmd, MAX_CMD_LEN, 
            "gcov-tool overlap -t not_a_number %s/%s %s/%s",
            dir, gcda1, dir, gcda2);
    tests[i].expected_exit = 1;  /* Should fail */
    tests[i].description = "Invalid threshold value";
    i++;
    
    /* Edge case: missing threshold argument */
    tests[i].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[i].cmd, MAX_CMD_LEN, 
            "gcov-tool overlap -t %s/%s %s/%s",
            dir, gcda1, dir, gcda2);
    tests[i].expected_exit = 1;  /* Should fail */
    tests[i].description = "Missing threshold argument";
    i++;
    
    /* Edge case: unknown flag (should trigger default case) */
    tests[i].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[i].cmd, MAX_CMD_LEN, 
            "gcov-tool overlap -x %s/%s %s/%s",
            dir, gcda1, dir, gcda2);
    tests[i].expected_exit = 1;  /* Should fail */
    tests[i].description = "Unknown flag (triggers default case)";
    i++;
    
    /* Edge case: repeated flags */
    tests[i].cmd = malloc(MAX_CMD_LEN);
    snprintf(tests[i].cmd, MAX_CMD_LEN, 
            "gcov-tool overlap -v -v -v -f -f %s/%s %s/%s",
            dir, gcda1, dir, gcda2);
    tests[i].expected_exit = 0;
    tests[i].description = "Repeated flags";
    i++;
    
    /* Test with different threshold values */
    const char *thresholds[] = {"0.0", "0.1", "0.5", "0.9", "1.0", "1.5", "100.0"};
    for (int j = 0; j < 7; j++) {
        tests[i].cmd = malloc(MAX_CMD_LEN);
        snprintf(tests[i].cmd, MAX_CMD_LEN, 
                "gcov-tool overlap -t %s %s/%s %s/%s",
                thresholds[j], dir, gcda1, dir, gcda2);
        tests[i].expected_exit = 0;
        tests[i].description = malloc(100);
        snprintf(tests[i].description, 100, "Threshold value: %s", thresholds[j]);
        i++;
    }
    
    *num_tests = i;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    char *temp_dir = create_temp_dir();
    if (!temp_dir) {
        fprintf(stderr, "Failed to create temporary directory\n");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", temp_dir);
    
    /* Write test programs to files */
    char source1_path[MAX_CMD_LEN], source2_path[MAX_CMD_LEN];
    snprintf(source1_path, sizeof(source1_path), "%s/test1.c", temp_dir);
    snprintf(source2_path, sizeof(source2_path), "%s/test2.c", temp_dir);
    
    FILE *fp1 = fopen(source1_path, "w");
    FILE *fp2 = fopen(source2_path, "w");
    
    if (!fp1 || !fp2) {
        perror("Failed to create source files");
        free(temp_dir);
        return 1;
    }
    
    fputs(test_program, fp1);
    fputs(test_program2, fp2);
    fclose(fp1);
    fclose(fp2);
    
    /* Compile test programs with GCOV instrumentation */
    if (compile_with_gcov("test1.c", "test1", temp_dir) != 0) {
        fprintf(stderr, "Failed to compile test1.c\n");
        free(temp_dir);
        return 1;
    }
    
    if (compile_with_gcov("test2.c", "test2", temp_dir) != 0) {
        fprintf(stderr, "Failed to compile test2.c\n");
        free(temp_dir);
        return 1;
    }
    
    /* Run programs to generate .gcda files */
    if (run_program("test1", temp_dir) != 0) {
        fprintf(stderr, "Failed to run test1\n");
        free(temp_dir);
        return 1;
    }
    
    if (run_program("test2", temp_dir) != 0) {
        fprintf(stderr, "Failed to run test2\n");
        free(temp_dir);
        return 1;
    }
    
    /* Run test1 multiple times to generate different profile data */
    for (int i = 0; i < 3; i++) {
        run_program("test1", temp_dir);
    }
    
    /* Verify .gcda files exist */
    struct stat st;
    char gcda1_path[MAX_CMD_LEN], gcda2_path[MAX_CMD_LEN];
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test1.gcda", temp_dir);
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test2.gcda", temp_dir);
    
    if (stat(gcda1_path, &st) != 0 || stat(gcda2_path, &st) != 0) {
        fprintf(stderr, "GCOV data files not created\n");
        free(temp_dir);
        return 1;
    }
    
    printf("Generated GCOV data files:\n");
    printf("  %s\n", gcda1_path);
    printf("  %s\n", gcda2_path);
    
    /* Generate and run test cases */
    test_case_t tests[50];
    int num_tests = 0;
    
    generate_test_cases(tests, &num_tests, temp_dir, "test1.gcda", "test2.gcda");
    
    printf("\n=== Running %d test cases ===\n\n", num_tests);
    
    int passed = 0;
    int failed = 0;
    
    for (int i = 0; i < num_tests; i++) {
        printf("\nTest %d: %s\n", i + 1, tests[i].description);
        printf("Command: %s\n", tests[i].cmd);
        
        int exit_status = run_command(tests[i].cmd, 1);
        
        /* For edge cases that should fail, we accept any non-zero exit */
        if (tests[i].expected_exit == 0) {
            if (exit_status == 0) {
                printf("✓ PASSED\n");
                passed++;
            } else {
                printf("✗ FAILED (expected 0, got %d)\n", exit_status);
                failed++;
            }
        } else {
            if (exit_status != 0) {
                printf("✓ PASSED (expected failure, got %d)\n", exit_status);
                passed++;
            } else {
                printf("✗ FAILED (expected failure, got 0)\n");
                failed++;
            }
        }
        
        free(tests[i].cmd);
        free(tests[i].description);
    }
    
    /* Cleanup */
    char cleanup_cmd[MAX_CMD_LEN];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
    run_command(cleanup_cmd, 0);
    free(temp_dir);
    
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    printf("Total:  %d\n", num_tests);
    
    return failed > 0 ? 1 : 0;
}
