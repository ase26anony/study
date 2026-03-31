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

/* Global variables to prevent optimization */
volatile int test_counter = 0;
volatile int success_count = 0;
volatile int fail_count = 0;

/* Structure to hold test case information */
typedef struct {
    char *description;
    char *command;
    int expected_exit;  /* 0 for success, non-zero for failure */
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

/* Function to execute a command and return exit status */
int execute_command(const char *cmd) {
    int status;
    printf("Executing: %s\n", cmd);
    fflush(stdout);
    
    status = system(cmd);
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Function to create a temporary directory */
char *create_temp_dir() {
    char *template = "/tmp/gcov_test_XXXXXX";
    char *dir = strdup(template);
    if (mkdtemp(dir) == NULL) {
        perror("mkdtemp failed");
        free(dir);
        return NULL;
    }
    return dir;
}

/* Function to compile and run instrumented program */
int generate_gcda_files(const char *dir, int num_files) {
    char cmd[MAX_CMD_LEN];
    char src_path[MAX_CMD_LEN];
    char exe_path[MAX_CMD_LEN];
    int i;
    
    /* Create source file */
    snprintf(src_path, sizeof(src_path), "%s/test.c", dir);
    FILE *fp = fopen(src_path, "w");
    if (!fp) {
        perror("Failed to create source file");
        return 0;
    }
    fprintf(fp, "%s", test_program);
    fclose(fp);
    
    /* Compile with GCOV instrumentation */
    snprintf(exe_path, sizeof(exe_path), "%s/test.exe", dir);
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -o %s %s 2>/dev/null", 
             exe_path, src_path);
    
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Compilation failed\n");
        return 0;
    }
    
    /* Generate multiple .gcda files by running the program multiple times */
    for (i = 0; i < num_files; i++) {
        char gcda_dir[MAX_CMD_LEN];
        char run_cmd[MAX_CMD_LEN];
        
        /* Create separate directory for each run to avoid overwriting */
        snprintf(gcda_dir, sizeof(gcda_dir), "%s/run%d", dir, i);
        mkdir(gcda_dir, 0755);
        
        /* Copy executable and source to run directory */
        char cp_cmd[MAX_CMD_LEN];
        snprintf(cp_cmd, sizeof(cp_cmd), "cp %s %s/test.c %s 2>/dev/null", 
                 exe_path, src_path, gcda_dir);
        execute_command(cp_cmd);
        
        /* Run in the directory to generate .gcda */
        snprintf(run_cmd, sizeof(run_cmd), 
                 "cd %s && ./test.exe >/dev/null 2>&1", gcda_dir);
        execute_command(run_cmd);
    }
    
    return 1;
}

/* Run a single test case */
void run_test_case(const char *dir, test_case_t *test) {
    char full_cmd[MAX_CMD_LEN];
    int exit_code;
    
    test_counter++;
    
    /* Build the full command with directory path */
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", test->command);
    
    /* Replace placeholders with actual directory */
    char *pos;
    while ((pos = strstr(full_cmd, "$DIR")) != NULL) {
        char temp[MAX_CMD_LEN];
        int offset = pos - full_cmd;
        strncpy(temp, full_cmd, offset);
        temp[offset] = '\0';
        strcat(temp, dir);
        strcat(temp, pos + 4);
        strncpy(full_cmd, temp, sizeof(full_cmd));
    }
    
    exit_code = execute_command(full_cmd);
    
    printf("Test: %s\n", test->description);
    printf("Exit code: %d (expected: %d)\n", exit_code, test->expected_exit);
    
    /* Check if exit code matches expectation */
    if ((test->expected_exit == 0 && exit_code == 0) ||
        (test->expected_exit != 0 && exit_code != 0)) {
        printf("Result: PASS\n");
        success_count++;
    } else {
        printf("Result: FAIL\n");
        fail_count++;
    }
    printf("---\n");
}

int main(int argc, char *argv[]) {
    char *temp_dir = NULL;
    char gcda_files[MAX_CMD_LEN];
    int i;
    
    /* Test cases covering all uncovered lines and edge cases */
    test_case_t test_cases[] = {
        /* Basic tests for each flag individually */
        {
            "Test -v flag (verbose)",
            "gcov-tool overlap -v $DIR/run0/test.gcda $DIR/run1/test.gcda",
            0
        },
        {
            "Test -f flag (function level)",
            "gcov-tool overlap -f $DIR/run0/test.gcda $DIR/run1/test.gcda",
            0
        },
        {
            "Test -F flag (full filename)",
            "gcov-tool overlap -F $DIR/run0/test.gcda $DIR/run1/test.gcda",
            0
        },
        {
            "Test -o flag (object level)",
            "gcov-tool overlap -o $DIR/run0/test.gcda $DIR/run1/test.gcda",
            0
        },
        {
            "Test -h flag (hot only)",
            "gcov-tool overlap -h $DIR/run0/test.gcda $DIR/run1/test.gcda",
            0
        },
        {
            "Test -t flag with value 0.5",
            "gcov-tool overlap -t 0.5 $DIR/run0/test.gcda $DIR/run1/test.gcda",
            0
        },
        
        /* Combined flags - testing all uncovered cases in one command */
        {
            "Test all flags combined (all uncovered cases)",
            "gcov-tool overlap -v -f -F -o -h -t 0.75 $DIR/run0/test.gcda $DIR/run1/test.gcda",
            0
        },
        
        /* Permutations of flag order */
        {
            "Test flags in different order 1",
            "gcov-tool overlap -t 1.0 -h -o -F -f -v $DIR/run0/test.gcda $DIR/run1/test.gcda",
            0
        },
        {
            "Test flags in different order 2",
            "gcov-tool overlap -F -v -t 0.25 -f -o -h $DIR/run0/test.gcda $DIR/run1/test.gcda",
            0
        },
        
        /* Edge cases for -t flag */
        {
            "Test -t with high threshold",
            "gcov-tool overlap -t 99.9 $DIR/run0/test.gcda $DIR/run1/test.gcda",
            0
        },
        {
            "Test -t with zero threshold",
            "gcov-tool overlap -t 0.0 $DIR/run0/test.gcda $DIR/run1/test.gcda",
            0
        },
        {
            "Test -t with scientific notation",
            "gcov-tool overlap -t 1e-3 $DIR/run0/test.gcda $DIR/run1/test.gcda",
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
            "gcov-tool overlap -t not_a_number $DIR/run0/test.gcda $DIR/run1/test.gcda",
            1  /* Expected to fail */
        },
        {
            "Test unknown flag -x (should trigger default case)",
            "gcov-tool overlap -x $DIR/run0/test.gcda $DIR/run1/test.gcda",
            1  /* Expected to fail */
        },
        
        /* Multiple files and path variations */
        {
            "Test with three .gcda files",
            "gcov-tool overlap -v -f $DIR/run0/test.gcda $DIR/run1/test.gcda $DIR/run2/test.gcda",
            0
        },
        {
            "Test with absolute paths",
            "gcov-tool overlap -v `pwd`/$DIR/run0/test.gcda `pwd`/$DIR/run1/test.gcda",
            0
        },
        
        /* Flag repetition */
        {
            "Test repeated -v flags",
            "gcov-tool overlap -v -v -v $DIR/run0/test.gcda $DIR/run1/test.gcda",
            0
        },
        
        /* Mixed valid and invalid to test parsing */
        {
            "Test mixed flags with invalid at end (should fail)",
            "gcov-tool overlap -v -f -x $DIR/run0/test.gcda $DIR/run1/test.gcda",
            1  /* Expected to fail */
        }
    };
    
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    
    printf("=== GCOV-TOOL Overlap Parser Test Suite ===\n\n");
    
    /* Create temporary directory for test files */
    temp_dir = create_temp_dir();
    if (!temp_dir) {
        fprintf(stderr, "Failed to create temporary directory\n");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", temp_dir);
    
    /* Generate GCOV data files */
    printf("Generating GCOV data files...\n");
    if (!generate_gcda_files(temp_dir, 3)) {
        fprintf(stderr, "Failed to generate GCOV data files\n");
        free(temp_dir);
        return 1;
    }
    
    printf("Generated 3 sets of .gcda files\n\n");
    
    /* Run all test cases */
    for (i = 0; i < num_tests; i++) {
        run_test_case(temp_dir, &test_cases[i]);
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Total tests run: %d\n", test_counter);
    printf("Passed: %d\n", success_count);
    printf("Failed: %d\n", fail_count);
    printf("Success rate: %.1f%%\n", 
           test_counter > 0 ? (100.0 * success_count / test_counter) : 0.0);
    
    /* Cleanup */
    printf("\nCleaning up temporary directory: %s\n", temp_dir);
    char cleanup_cmd[MAX_CMD_LEN];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
    execute_command(cleanup_cmd);
    free(temp_dir);
    
    /* Make exit code observable to prevent optimization */
    if (fail_count > 0) {
        printf("\nSome tests failed. Check if gcov-tool is built with coverage.\n");
        printf("To build gcov-tool with coverage, use: --enable-coverage flag during GCC build.\n");
    }
    
    return fail_count > 0 ? 1 : 0;
}
