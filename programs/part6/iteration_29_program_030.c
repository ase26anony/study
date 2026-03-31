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
int tests_passed = 0;
int tests_failed = 0;
int tests_total = 0;

/* Structure to hold a test case */
typedef struct {
    char *description;
    char *command;
    int expected_exit_code;
} test_case_t;

/* Function to create a temporary directory */
char *create_temp_dir() {
    char *template = "/tmp/gcov_test_XXXXXX";
    char *dir_name = strdup(template);
    if (mkdtemp(dir_name) == NULL) {
        perror("Failed to create temporary directory");
        exit(EXIT_FAILURE);
    }
    return dir_name;
}

/* Function to remove a directory and its contents */
void remove_temp_dir(const char *dir_name) {
    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dir_name);
    system(cmd);
}

/* Function to create a simple C program for GCOV testing */
void create_test_program(const char *dir_name, const char *prog_name) {
    char source_path[MAX_CMD_LEN];
    char exec_path[MAX_CMD_LEN];
    FILE *fp;
    
    /* Create source file */
    snprintf(source_path, sizeof(source_path), "%s/%s.c", dir_name, prog_name);
    fp = fopen(source_path, "w");
    if (!fp) {
        perror("Failed to create source file");
        exit(EXIT_FAILURE);
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    int i;\n");
    fprintf(fp, "    for (i = 0; i < 10; i++) {\n");
    fprintf(fp, "        printf(\"Value: %%d\\n\", i);\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    fclose(fp);
    
    /* Compile with GCOV instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/%s", dir_name, prog_name);
    char compile_cmd[MAX_CMD_LEN];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -o %s %s",
             exec_path, source_path);
    
    if (system(compile_cmd) != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        exit(EXIT_FAILURE);
    }
}

/* Function to run a program and generate GCOV data */
void generate_gcov_data(const char *dir_name, const char *prog_name, 
                        const char *data_suffix) {
    char prog_path[MAX_CMD_LEN];
    char rename_cmd[MAX_CMD_LEN];
    
    snprintf(prog_path, sizeof(prog_path), "%s/%s", dir_name, prog_name);
    
    /* Run the program to generate .gcda file */
    if (system(prog_path) != 0) {
        fprintf(stderr, "Failed to run test program\n");
        exit(EXIT_FAILURE);
    }
    
    /* Rename .gcda file to create multiple versions */
    if (data_suffix) {
        char gcda_file[MAX_CMD_LEN];
        char new_gcda[MAX_CMD_LEN];
        
        snprintf(gcda_file, sizeof(gcda_file), "%s/%s.gcda", dir_name, prog_name);
        snprintf(new_gcda, sizeof(new_gcda), "%s/%s_%s.gcda", 
                dir_name, prog_name, data_suffix);
        
        /* Copy the file */
        snprintf(rename_cmd, sizeof(rename_cmd), "cp %s %s", gcda_file, new_gcda);
        system(rename_cmd);
    }
}

/* Function to run a test case and check the result */
void run_test_case(const char *dir_name, test_case_t *test) {
    char full_cmd[MAX_CMD_LEN];
    int status;
    
    tests_total++;
    
    printf("Test %d: %s\n", tests_total, test->description);
    printf("  Command: %s\n", test->command);
    
    /* Construct the full command with directory path */
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", test->command);
    
    /* Execute the command */
    status = system(full_cmd);
    
    /* Check the exit status */
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        
        if (exit_code == test->expected_exit_code) {
            printf("  ✓ PASSED (exit code: %d)\n", exit_code);
            tests_passed++;
        } else {
            printf("  ✗ FAILED (expected %d, got %d)\n", 
                   test->expected_exit_code, exit_code);
            tests_failed++;
        }
    } else {
        printf("  ✗ FAILED (abnormal termination)\n");
        tests_failed++;
    }
    
    printf("\n");
}

/* Main test driver */
int main(int argc, char *argv[]) {
    char *temp_dir = NULL;
    char prog1_gcda[MAX_CMD_LEN];
    char prog2_gcda[MAX_CMD_LEN];
    char prog3_gcda[MAX_CMD_LEN];
    
    /* Create temporary directory for test files */
    temp_dir = create_temp_dir();
    printf("Created temporary directory: %s\n\n", temp_dir);
    
    /* Create and compile test programs */
    create_test_program(temp_dir, "test_prog1");
    create_test_program(temp_dir, "test_prog2");
    
    /* Generate GCOV data files */
    generate_gcov_data(temp_dir, "test_prog1", "v1");
    generate_gcov_data(temp_dir, "test_prog1", "v2");
    generate_gcov_data(temp_dir, "test_prog2", "v1");
    
    /* Construct paths to GCOV data files */
    snprintf(prog1_gcda, sizeof(prog1_gcda), 
             "%s/test_prog1_v1.gcda", temp_dir);
    snprintf(prog2_gcda, sizeof(prog2_gcda), 
             "%s/test_prog1_v2.gcda", temp_dir);
    snprintf(prog3_gcda, sizeof(prog3_gcda), 
             "%s/test_prog2_v1.gcda", temp_dir);
    
    /* Define test cases */
    test_case_t test_cases[] = {
        /* Basic flag tests - each flag individually */
        {
            "Test -v flag (verbose)",
            "gcov-tool overlap -v test_prog1_v1.gcda test_prog1_v2.gcda",
            0
        },
        {
            "Test -f flag (function level)",
            "gcov-tool overlap -f test_prog1_v1.gcda test_prog1_v2.gcda",
            0
        },
        {
            "Test -F flag (full filename)",
            "gcov-tool overlap -F test_prog1_v1.gcda test_prog1_v2.gcda",
            0
        },
        {
            "Test -o flag (object level)",
            "gcov-tool overlap -o test_prog1_v1.gcda test_prog1_v2.gcda",
            0
        },
        {
            "Test -h flag (hot only)",
            "gcov-tool overlap -h test_prog1_v1.gcda test_prog1_v2.gcda",
            0
        },
        {
            "Test -t flag with value 0.5",
            "gcov-tool overlap -t 0.5 test_prog1_v1.gcda test_prog1_v2.gcda",
            0
        },
        
        /* Combined flag tests - permutations */
        {
            "Test -v -f -F -o -h -t 0.75 (all flags)",
            "gcov-tool overlap -v -f -F -o -h -t 0.75 test_prog1_v1.gcda test_prog1_v2.gcda",
            0
        },
        {
            "Test -f -F -v -o -h -t 1.0 (different order)",
            "gcov-tool overlap -f -F -v -o -h -t 1.0 test_prog1_v1.gcda test_prog1_v2.gcda",
            0
        },
        {
            "Test -t 0.25 -h -o -F -f -v (reverse order)",
            "gcov-tool overlap -t 0.25 -h -o -F -f -v test_prog1_v1.gcda test_prog1_v2.gcda",
            0
        },
        
        /* Multiple input files */
        {
            "Test with three input files",
            "gcov-tool overlap -v -f test_prog1_v1.gcda test_prog1_v2.gcda test_prog2_v1.gcda",
            0
        },
        
        /* Edge cases for -t flag */
        {
            "Test -t with value 0.0 (minimum)",
            "gcov-tool overlap -t 0.0 test_prog1_v1.gcda test_prog1_v2.gcda",
            0
        },
        {
            "Test -t with value 100.0 (large value)",
            "gcov-tool overlap -t 100.0 test_prog1_v1.gcda test_prog1_v2.gcda",
            0
        },
        {
            "Test -t with scientific notation",
            "gcov-tool overlap -t 1.5e-1 test_prog1_v1.gcda test_prog1_v2.gcda",
            0
        },
        
        /* Error cases */
        {
            "Test -t without argument (should fail)",
            "gcov-tool overlap -t",
            1  /* Expected to fail */
        },
        {
            "Test invalid argument for -t (should fail)",
            "gcov-tool overlap -t not_a_number test_prog1_v1.gcda test_prog1_v2.gcda",
            1  /* Expected to fail */
        },
        {
            "Test unknown flag -x (should trigger default case)",
            "gcov-tool overlap -x test_prog1_v1.gcda test_prog1_v2.gcda",
            1  /* Expected to fail */
        },
        
        /* Repeated flags */
        {
            "Test repeated -v flags",
            "gcov-tool overlap -v -v -v test_prog1_v1.gcda test_prog1_v2.gcda",
            0
        },
        
        /* Mixed valid and invalid */
        {
            "Test valid flags with no input files (should fail)",
            "gcov-tool overlap -v -f -F",
            1  /* Expected to fail */
        },
        
        /* Using absolute paths */
        {
            "Test with absolute paths",
            "",
            0
        },
        
        /* End marker */
        {NULL, NULL, 0}
    };
    
    /* Update the absolute path test case */
    char abs_path_test[MAX_CMD_LEN];
    snprintf(abs_path_test, sizeof(abs_path_test),
             "gcov-tool overlap -v -f %s %s",
             prog1_gcda, prog2_gcda);
    test_cases[16].command = strdup(abs_path_test);
    
    /* Run all test cases */
    printf("Running gcov-tool overlap tests...\n");
    printf("========================================\n\n");
    
    for (int i = 0; test_cases[i].description != NULL; i++) {
        /* Change to temp directory for relative path tests */
        if (chdir(temp_dir) != 0) {
            perror("Failed to change directory");
            continue;
        }
        
        run_test_case(temp_dir, &test_cases[i]);
    }
    
    /* Print summary */
    printf("\n========================================\n");
    printf("Test Summary:\n");
    printf("  Total tests:  %d\n", tests_total);
    printf("  Passed:       %d\n", tests_passed);
    printf("  Failed:       %d\n", tests_failed);
    printf("  Success rate: %.1f%%\n", 
           (tests_total > 0) ? (100.0 * tests_passed / tests_total) : 0.0);
    
    /* Clean up */
    if (chdir("/") != 0) {
        perror("Failed to change to root directory");
    }
    
    remove_temp_dir(temp_dir);
    free(temp_dir);
    free(test_cases[16].command);
    
    printf("\nTemporary directory cleaned up.\n");
    
    return (tests_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
