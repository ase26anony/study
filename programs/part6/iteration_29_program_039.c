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

/* Structure to hold test case information */
typedef struct {
    char *description;
    char *command;
    int expected_exit_code;
} test_case_t;

/* Global variables for cleanup */
char temp_dir[256] = "";
char *gcda_files[MAX_FILES] = {NULL};
int gcda_count = 0;

/* Function prototypes */
void create_temp_directory(void);
void cleanup_temp_directory(void);
void create_gcov_test_files(void);
int run_gcov_tool(const char *command);
void run_test_cases(void);

/* Create a temporary directory for test files */
void create_temp_directory(void) {
    char template[] = "/tmp/gcov_test_XXXXXX";
    char *dir = mkdtemp(template);
    if (!dir) {
        perror("Failed to create temporary directory");
        exit(EXIT_FAILURE);
    }
    strcpy(temp_dir, dir);
    printf("Created temporary directory: %s\n", temp_dir);
}

/* Clean up temporary directory and files */
void cleanup_temp_directory(void) {
    if (temp_dir[0]) {
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
        system(cmd);
        printf("Cleaned up temporary directory: %s\n", temp_dir);
    }
    
    for (int i = 0; i < gcda_count; i++) {
        free(gcda_files[i]);
    }
}

/* Create a simple C program, compile it with GCOV, and run it to generate .gcda files */
void create_gcov_test_files(void) {
    char c_file[256], exe_file[256], gcda_file[256];
    FILE *fp;
    
    /* Create a simple C program */
    snprintf(c_file, sizeof(c_file), "%s/test_prog.c", temp_dir);
    fp = fopen(c_file, "w");
    if (!fp) {
        perror("Failed to create test C file");
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    printf(\"Hello from test program!\\n\");\n");
    fprintf(fp, "    for (int i = 0; i < 10; i++) {\n");
    fprintf(fp, "        printf(\"Iteration %%d\\n\", i);\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    fclose(fp);
    
    /* Compile with GCOV instrumentation */
    snprintf(exe_file, sizeof(exe_file), "%s/test_prog", temp_dir);
    char compile_cmd[512];
    snprintf(compile_cmd, sizeof(compile_cmd), 
             "gcc -fprofile-arcs -ftest-coverage -o %s %s 2>/dev/null", 
             exe_file, c_file);
    
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile test program. Make sure gcc is installed.\n");
        exit(EXIT_FAILURE);
    }
    
    /* Run the program multiple times to generate different .gcda files */
    for (int i = 0; i < 3; i++) {
        char run_cmd[256];
        snprintf(run_cmd, sizeof(run_cmd), "cd %s && ./test_prog > /dev/null 2>&1", temp_dir);
        system(run_cmd);
        
        /* Copy the .gcda file with different names */
        snprintf(gcda_file, sizeof(gcda_file), "%s/test_prog_%d.gcda", temp_dir, i);
        char copy_cmd[512];
        snprintf(copy_cmd, sizeof(copy_cmd), 
                 "cp %s/test_prog.gcda %s 2>/dev/null", temp_dir, gcda_file);
        system(copy_cmd);
        
        /* Store the file path for later use */
        gcda_files[gcda_count] = strdup(gcda_file);
        gcda_count++;
        
        printf("Created GCOV data file: %s\n", gcda_file);
    }
}

/* Run gcov-tool with the given command and return exit code */
int run_gcov_tool(const char *command) {
    printf("Running: %s\n", command);
    
    int status = system(command);
    
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        printf("Exit code: %d\n\n", exit_code);
        return exit_code;
    } else {
        printf("Command terminated abnormally\n\n");
        return -1;
    }
}

/* Main test driver */
int main(int argc, char *argv[]) {
    printf("=== GCOV-Tool Overlap Subcommand Test Driver ===\n\n");
    
    /* Set up environment */
    create_temp_directory();
    create_gcov_test_files();
    
    /* Test cases covering the uncovered lines */
    test_case_t test_cases[] = {
        /* Basic flag combinations - these should trigger all case statements */
        {
            "Test all flags together",
            "gcov-tool overlap -v -f -F -o -h -t 0.75 test_prog_0.gcda test_prog_1.gcda",
            0
        },
        {
            "Test all flags in different order",
            "gcov-tool overlap -t 0.5 -h -o -F -f -v test_prog_0.gcda test_prog_1.gcda",
            0
        },
        {
            "Test with high threshold value",
            "gcov-tool overlap -v -t 99.9 test_prog_0.gcda test_prog_1.gcda",
            0
        },
        {
            "Test with low threshold value",
            "gcov-tool overlap -v -t 0.001 test_prog_0.gcda test_prog_1.gcda",
            0
        },
        
        /* Individual flag tests */
        {
            "Test verbose flag only",
            "gcov-tool overlap -v test_prog_0.gcda test_prog_1.gcda",
            0
        },
        {
            "Test function level flag only",
            "gcov-tool overlap -f test_prog_0.gcda test_prog_1.gcda",
            0
        },
        {
            "Test fullname flag only",
            "gcov-tool overlap -F test_prog_0.gcda test_prog_1.gcda",
            0
        },
        {
            "Test object level flag only",
            "gcov-tool overlap -o test_prog_0.gcda test_prog_1.gcda",
            0
        },
        {
            "Test hot only flag only",
            "gcov-tool overlap -h test_prog_0.gcda test_prog_1.gcda",
            0
        },
        {
            "Test threshold flag only",
            "gcov-tool overlap -t 1.0 test_prog_0.gcda test_prog_1.gcda",
            0
        },
        
        /* Edge cases and error conditions */
        {
            "Test missing argument for -t flag (should fail)",
            "gcov-tool overlap -t",
            1  /* Non-zero exit code expected */
        },
        {
            "Test invalid argument for -t flag (should fail)",
            "gcov-tool overlap -t not_a_number test_prog_0.gcda",
            1  /* Non-zero exit code expected */
        },
        {
            "Test unknown flag (should trigger default case)",
            "gcov-tool overlap -x test_prog_0.gcda",
            1  /* Non-zero exit code expected */
        },
        {
            "Test repeated flags",
            "gcov-tool overlap -v -v -v test_prog_0.gcda test_prog_1.gcda",
            0
        },
        {
            "Test combination with repeated threshold",
            "gcov-tool overlap -t 0.3 -t 0.7 test_prog_0.gcda test_prog_1.gcda",
            0  /* Last value should be used */
        },
        
        /* Test with different file combinations */
        {
            "Test with three input files",
            "gcov-tool overlap -v -f test_prog_0.gcda test_prog_1.gcda test_prog_2.gcda",
            0
        },
        {
            "Test with absolute paths",
            "",
            0  /* Will be filled dynamically */
        },
        
        /* Test with no flags (baseline) */
        {
            "Test with no flags",
            "gcov-tool overlap test_prog_0.gcda test_prog_1.gcda",
            0
        }
    };
    
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int passed = 0;
    int failed = 0;
    
    /* Run all test cases */
    for (int i = 0; i < num_tests; i++) {
        printf("\n--- Test %d: %s ---\n", i + 1, test_cases[i].description);
        
        /* Handle dynamic test case for absolute paths */
        if (strcmp(test_cases[i].description, "Test with absolute paths") == 0) {
            char abs_cmd[MAX_CMD_LEN];
            char abs_path1[256], abs_path2[256];
            
            /* Get absolute paths */
            realpath(gcda_files[0], abs_path1);
            realpath(gcda_files[1], abs_path2);
            
            snprintf(abs_cmd, sizeof(abs_cmd),
                    "gcov-tool overlap -v -f %s %s",
                    abs_path1, abs_path2);
            
            test_cases[i].command = strdup(abs_cmd);
        }
        
        /* Change to temp directory for relative paths */
        char old_cwd[256];
        getcwd(old_cwd, sizeof(old_cwd));
        chdir(temp_dir);
        
        /* Run the command */
        int exit_code = run_gcov_tool(test_cases[i].command);
        
        /* Change back to original directory */
        chdir(old_cwd);
        
        /* Check result */
        if (test_cases[i].expected_exit_code == 0) {
            if (exit_code == 0) {
                printf("✓ PASSED\n");
                passed++;
            } else {
                printf("✗ FAILED: Expected exit code 0, got %d\n", exit_code);
                failed++;
            }
        } else {
            if (exit_code != 0) {
                printf("✓ PASSED (expected non-zero exit code)\n");
                passed++;
            } else {
                printf("✗ FAILED: Expected non-zero exit code, got 0\n");
                failed++;
            }
        }
        
        /* Free dynamically allocated command string */
        if (strcmp(test_cases[i].description, "Test with absolute paths") == 0) {
            free(test_cases[i].command);
        }
    }
    
    /* Additional permutation tests */
    printf("\n--- Testing Flag Permutations ---\n");
    
    /* Define all flags to test */
    char *flags[] = {"-v", "-f", "-F", "-o", "-h", "-t 0.5"};
    int num_flags = 6;
    
    /* Test different combinations */
    for (int mask = 1; mask < (1 << num_flags); mask++) {
        char cmd[MAX_CMD_LEN] = "gcov-tool overlap";
        
        /* Build command with selected flags */
        for (int j = 0; j < num_flags; j++) {
            if (mask & (1 << j)) {
                strcat(cmd, " ");
                strcat(cmd, flags[j]);
            }
        }
        
        /* Add input files */
        strcat(cmd, " test_prog_0.gcda test_prog_1.gcda");
        
        printf("\nTesting combination: ");
        for (int j = 0; j < num_flags; j++) {
            if (mask & (1 << j)) {
                printf("%s ", flags[j]);
            }
        }
        printf("\n");
        
        /* Run the command */
        char old_cwd[256];
        getcwd(old_cwd, sizeof(old_cwd));
        chdir(temp_dir);
        
        int exit_code = run_gcov_tool(cmd);
        
        chdir(old_cwd);
        
        if (exit_code == 0) {
            printf("✓ Success\n");
        } else {
            printf("✗ Failed with exit code %d\n", exit_code);
        }
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", num_tests);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    printf("Pass rate: %.1f%%\n", (float)passed / num_tests * 100);
    
    /* Clean up */
    cleanup_temp_directory();
    
    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
