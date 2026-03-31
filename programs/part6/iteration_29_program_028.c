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

/* Global variables to track test results */
int tests_run = 0;
int tests_passed = 0;
int tests_failed = 0;

/* Function to create a simple C program for GCOV instrumentation */
void create_test_program(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create test program");
        exit(1);
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        printf(\"Hello, World!\\n\");\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
}

/* Function to compile and run a test program with GCOV instrumentation */
int generate_gcda_file(const char *source_file, const char *output_prefix) {
    char compile_cmd[512];
    char run_cmd[512];
    
    /* Compile with GCOV instrumentation */
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s",
             output_prefix, source_file);
    
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return 0;
    }
    
    /* Run the program to generate .gcda file */
    snprintf(run_cmd, sizeof(run_cmd), "./%s > /dev/null 2>&1", output_prefix);
    if (system(run_cmd) != 0) {
        fprintf(stderr, "Failed to run test program\n");
        return 0;
    }
    
    return 1;
}

/* Function to execute gcov-tool and check exit status */
int run_gcov_tool_test(const char *description, const char *command, int expect_success) {
    tests_run++;
    
    printf("Test %d: %s\n", tests_run, description);
    printf("  Command: %s\n", command);
    
    int status = system(command);
    int exit_code = WEXITSTATUS(status);
    
    int success;
    if (expect_success) {
        success = (exit_code == 0);
    } else {
        success = (exit_code != 0);
    }
    
    if (success) {
        printf("  Result: PASSED\n\n");
        tests_passed++;
        return 1;
    } else {
        printf("  Result: FAILED (exit code: %d)\n\n", exit_code);
        tests_failed++;
        return 0;
    }
}

/* Function to generate permutations of flags */
void generate_flag_permutations(char **commands, int *count) {
    const char *flag_sets[][7] = {
        {"-v", "-f", "-F", "-o", "-h", "-t", "0.5"},
        {"-f", "-v", "-F", "-o", "-h", "-t", "0.75"},
        {"-F", "-f", "-v", "-o", "-h", "-t", "1.0"},
        {"-o", "-f", "-F", "-v", "-h", "-t", "0.25"},
        {"-h", "-f", "-F", "-o", "-v", "-t", "0.9"},
        {"-t", "0.3", "-v", "-f", "-F", "-o", "-h"}
    };
    
    for (int i = 0; i < 6; i++) {
        char cmd[MAX_CMD_LEN];
        snprintf(cmd, sizeof(cmd), "gcov-tool overlap ");
        
        for (int j = 0; j < 6; j++) {
            if (strcmp(flag_sets[i][j], "-t") == 0) {
                strcat(cmd, "-t ");
                strcat(cmd, flag_sets[i][j+1]);
                strcat(cmd, " ");
                j++; /* Skip the argument in the next iteration */
            } else {
                strcat(cmd, flag_sets[i][j]);
                strcat(cmd, " ");
            }
        }
        
        strcat(cmd, "test1.gcda test2.gcda");
        commands[*count] = strdup(cmd);
        (*count)++;
    }
}

int main(int argc, char *argv[]) {
    char *temp_dir = "/tmp/gcov_test";
    char source_file[256];
    char exec1[256], exec2[256];
    char gcda1[256], gcda2[256];
    
    printf("=== GCOV-TOOL Overlap Parser Test ===\n\n");
    
    /* Create temporary directory */
    if (mkdir(temp_dir, 0755) != 0 && errno != EEXIST) {
        perror("Failed to create temp directory");
        return 1;
    }
    
    if (chdir(temp_dir) != 0) {
        perror("Failed to change to temp directory");
        return 1;
    }
    
    /* Create and compile test programs */
    printf("Setting up test environment in %s\n", temp_dir);
    
    snprintf(source_file, sizeof(source_file), "%s/test_prog.c", temp_dir);
    snprintf(exec1, sizeof(exec1), "%s/test1", temp_dir);
    snprintf(exec2, sizeof(exec2), "%s/test2", temp_dir);
    snprintf(gcda1, sizeof(gcda1), "%s/test_prog.gcda", temp_dir);
    snprintf(gcda2, sizeof(gcda2), "%s/test_prog.gcda", temp_dir);
    
    create_test_program("test_prog.c");
    
    /* Generate two different .gcda files by running with different inputs */
    if (!generate_gcda_file("test_prog.c", "test1")) {
        fprintf(stderr, "Failed to generate first gcda file\n");
        return 1;
    }
    
    /* Rename the first gcda file */
    rename("test_prog.gcda", "test1.gcda");
    
    /* Create a modified version to get different coverage */
    system("sed 's/for (i = 0; i < 10; i++)/for (i = 0; i < 5; i++)/' test_prog.c > test_prog2.c");
    
    if (!generate_gcda_file("test_prog2.c", "test2")) {
        fprintf(stderr, "Failed to generate second gcda file\n");
        return 1;
    }
    
    rename("test_prog.gcda", "test2.gcda");
    
    printf("Generated test .gcda files successfully\n\n");
    
    /* Array to store test commands */
    char *test_commands[100];
    int test_count = 0;
    
    /* Test 1: Basic test with all flags */
    test_commands[test_count++] = strdup("gcov-tool overlap -v -f -F -o -h -t 0.5 test1.gcda test2.gcda");
    
    /* Test 2: Different order of flags */
    test_commands[test_count++] = strdup("gcov-tool overlap -t 0.75 -h -o -F -f -v test1.gcda test2.gcda");
    
    /* Test 3: Single flag tests (each uncovered case individually) */
    test_commands[test_count++] = strdup("gcov-tool overlap -v test1.gcda test2.gcda");
    test_commands[test_count++] = strdup("gcov-tool overlap -f test1.gcda test2.gcda");
    test_commands[test_count++] = strdup("gcov-tool overlap -F test1.gcda test2.gcda");
    test_commands[test_count++] = strdup("gcov-tool overlap -o test1.gcda test2.gcda");
    test_commands[test_count++] = strdup("gcov-tool overlap -h test1.gcda test2.gcda");
    test_commands[test_count++] = strdup("gcov-tool overlap -t 0.3 test1.gcda test2.gcda");
    
    /* Test 4: Flag combinations */
    test_commands[test_count++] = strdup("gcov-tool overlap -v -f test1.gcda test2.gcda");
    test_commands[test_count++] = strdup("gcov-tool overlap -F -o test1.gcda test2.gcda");
    test_commands[test_count++] = strdup("gcov-tool overlap -h -t 0.8 test1.gcda test2.gcda");
    test_commands[test_count++] = strdup("gcov-tool overlap -v -f -F test1.gcda test2.gcda");
    
    /* Test 5: Repeated flags */
    test_commands[test_count++] = strdup("gcov-tool overlap -v -v -v test1.gcda test2.gcda");
    test_commands[test_count++] = strdup("gcov-tool overlap -f -f -t 0.5 -t 0.6 test1.gcda test2.gcda");
    
    /* Test 6: Edge cases for -t flag */
    test_commands[test_count++] = strdup("gcov-tool overlap -t 0.0 test1.gcda test2.gcda");
    test_commands[test_count++] = strdup("gcov-tool overlap -t 1.0 test1.gcda test2.gcda");
    test_commands[test_count++] = strdup("gcov-tool overlap -t 100.5 test1.gcda test2.gcda");
    
    /* Test 7: Invalid cases (should fail) */
    test_commands[test_count++] = strdup("gcov-tool overlap -t not_a_number test1.gcda test2.gcda");
    test_commands[test_count++] = strdup("gcov-tool overlap -t test1.gcda test2.gcda");  /* Missing argument */
    test_commands[test_count++] = strdup("gcov-tool overlap -x test1.gcda test2.gcda");  /* Unknown flag */
    
    /* Test 8: With different file paths */
    test_commands[test_count++] = strdup("gcov-tool overlap -v -f ./test1.gcda ./test2.gcda");
    
    /* Test 9: Generate permutations programmatically */
    generate_flag_permutations(test_commands, &test_count);
    
    /* Run all tests */
    printf("Running %d test cases...\n\n", test_count);
    
    for (int i = 0; i < test_count; i++) {
        char description[256];
        int expect_success = 1;
        
        /* Determine if we expect success or failure */
        if (strstr(test_commands[i], "not_a_number") != NULL ||
            strstr(test_commands[i], "overlap -t test1") != NULL ||  /* Missing argument */
            strstr(test_commands[i], "-x") != NULL) {
            expect_success = 0;
        }
        
        snprintf(description, sizeof(description), "Test case %d", i + 1);
        run_gcov_tool_test(description, test_commands[i], expect_success);
        
        free(test_commands[i]);
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Total tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", (float)tests_passed / tests_run * 100);
    
    /* Cleanup */
    printf("\nCleaning up temporary files...\n");
    chdir("..");
    char cleanup_cmd[256];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
    system(cleanup_cmd);
    
    return (tests_failed == 0) ? 0 : 1;
}
