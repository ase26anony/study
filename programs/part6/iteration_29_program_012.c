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

/* Global variables for test configuration */
char *gcov_tool_path = "./gcov-tool";
char *temp_dir = "/tmp/gcov_test_XXXXXX";
char *test_prog = "test_prog.c";
char *test_binary = "test_prog";

/* Function prototypes */
int create_temp_directory(void);
int compile_test_program(void);
int generate_gcda_files(int count);
int run_gcov_tool(const char *command, int expected_exit);
void cleanup(void);
void print_test_result(const char *desc, int passed);

/* Test cases for the overlap subcommand */
test_case_t test_cases[] = {
    /* Basic flag combinations */
    {"All flags with valid threshold", 
     "overlap -v -f -F -o -h -t 0.75", 0},
    
    {"All flags in different order", 
     "overlap -h -t 0.5 -F -o -v -f", 0},
    
    {"Only verbose flag", 
     "overlap -v", 0},
    
    {"Function level only", 
     "overlap -f", 0},
    
    {"Full filename only", 
     "overlap -F", 0},
    
    {"Object level only", 
     "overlap -o", 0},
    
    {"Hot only flag", 
     "overlap -h", 0},
    
    {"Threshold only", 
     "overlap -t 1.0", 0},
    
    /* Edge cases for threshold */
    {"Threshold with decimal", 
     "overlap -t 0.12345", 0},
    
    {"Threshold with high value", 
     "overlap -t 999.999", 0},
    
    {"Threshold with zero", 
     "overlap -t 0.0", 0},
    
    /* Flag combinations */
    {"Verbose with function level", 
     "overlap -v -f", 0},
    
    {"Fullname with object level", 
     "overlap -F -o", 0},
    
    {"Hot only with threshold", 
     "overlap -h -t 0.8", 0},
    
    {"All except verbose", 
     "overlap -f -F -o -h -t 0.9", 0},
    
    /* Repeated flags */
    {"Multiple verbose flags", 
     "overlap -v -v -v", 0},
    
    {"Mixed repeated flags", 
     "overlap -f -F -f -o -o", 0},
    
    /* Error cases (should trigger usage or error) */
    {"Missing threshold argument", 
     "overlap -t", 1},  /* Should fail */
    
    {"Invalid threshold (non-numeric)", 
     "overlap -t not_a_number", 1},  /* Should fail */
    
    {"Unknown flag", 
     "overlap -x", 1},  /* Should trigger default case */
    
    {"Empty arguments", 
     "overlap", 0},  /* Should work with defaults */
    
    {NULL, NULL, 0}  /* Sentinel */
};

/* Create a temporary directory for test files */
int create_temp_directory(void) {
    char template[] = "/tmp/gcov_test_XXXXXX";
    temp_dir = mkdtemp(template);
    if (temp_dir == NULL) {
        perror("Failed to create temporary directory");
        return -1;
    }
    printf("Created temporary directory: %s\n", temp_dir);
    return 0;
}

/* Create and compile a simple test program with GCOV instrumentation */
int compile_test_program(void) {
    FILE *fp;
    char compile_cmd[MAX_CMD_LEN];
    int status;
    
    /* Create a simple C program */
    char *test_prog_path = malloc(strlen(temp_dir) + strlen(test_prog) + 2);
    sprintf(test_prog_path, "%s/%s", temp_dir, test_prog);
    
    fp = fopen(test_prog_path, "w");
    if (fp == NULL) {
        perror("Failed to create test program");
        free(test_prog_path);
        return -1;
    }
    
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    printf(\"Hello from test program\\n\");\n");
    fprintf(fp, "    for (int i = 0; i < 10; i++) {\n");
    fprintf(fp, "        if (i %% 2 == 0) {\n");
    fprintf(fp, "            printf(\"Even: %%d\\n\", i);\n");
    fprintf(fp, "        } else {\n");
    fprintf(fp, "            printf(\"Odd: %%d\\n\", i);\n");
    fprintf(fp, "        }\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    fclose(fp);
    
    /* Compile with GCOV instrumentation */
    char *test_binary_path = malloc(strlen(temp_dir) + strlen(test_binary) + 2);
    sprintf(test_binary_path, "%s/%s", temp_dir, test_binary);
    
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s %s",
             test_binary_path, test_prog_path);
    
    printf("Compiling test program: %s\n", compile_cmd);
    status = system(compile_cmd);
    
    free(test_prog_path);
    free(test_binary_path);
    
    if (status != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return -1;
    }
    
    return 0;
}

/* Generate multiple .gcda files by running the test program */
int generate_gcda_files(int count) {
    char run_cmd[MAX_CMD_LEN];
    char gcda_file[MAX_CMD_LEN];
    int i;
    
    if (count > MAX_FILES) {
        count = MAX_FILES;
    }
    
    for (i = 0; i < count; i++) {
        /* Run the test program to generate .gcda file */
        snprintf(run_cmd, sizeof(run_cmd),
                 "cd %s && ./%s > /dev/null 2>&1",
                 temp_dir, test_binary);
        
        printf("Generating gcda file %d/%d\n", i + 1, count);
        system(run_cmd);
        
        /* Rename the gcda file to create multiple versions */
        if (i > 0) {
            snprintf(gcda_file, sizeof(gcda_file),
                     "cd %s && cp test_prog.gcda test_prog%d.gcda 2>/dev/null",
                     temp_dir, i);
            system(gcda_file);
        }
        
        /* Sleep briefly to ensure different timestamps */
        usleep(100000);
    }
    
    return 0;
}

/* Run gcov-tool with the given command and check exit code */
int run_gcov_tool(const char *command, int expected_exit) {
    char full_cmd[MAX_CMD_LEN];
    char gcda_files[MAX_CMD_LEN];
    int status, result;
    pid_t pid;
    
    /* Build the list of gcda files */
    snprintf(gcda_files, sizeof(gcda_files),
             "%s/test_prog.gcda %s/test_prog1.gcda %s/test_prog2.gcda",
             temp_dir, temp_dir, temp_dir);
    
    /* Construct the full command */
    snprintf(full_cmd, sizeof(full_cmd), "%s %s %s",
             gcov_tool_path, command, gcda_files);
    
    printf("Running: %s\n", full_cmd);
    
    /* Use fork/exec to capture exit status */
    pid = fork();
    if (pid == 0) {
        /* Child process */
        char *argv[64];
        int argc = 0;
        char cmd_copy[MAX_CMD_LEN];
        char *token;
        
        /* Parse the command */
        strcpy(cmd_copy, full_cmd);
        token = strtok(cmd_copy, " ");
        
        while (token != NULL && argc < 63) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
        argv[argc] = NULL;
        
        /* Execute gcov-tool */
        execvp(argv[0], argv);
        
        /* If we get here, exec failed */
        perror("execvp failed");
        exit(127);
    } else if (pid > 0) {
        /* Parent process */
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            result = WEXITSTATUS(status);
            printf("Exit code: %d (expected: %d)\n", result, expected_exit);
            
            /* Check if exit code matches expectation */
            if ((expected_exit == 0 && result == 0) ||
                (expected_exit != 0 && result != 0)) {
                return 1;  /* Test passed */
            } else {
                return 0;  /* Test failed */
            }
        } else {
            printf("Process terminated abnormally\n");
            return 0;
        }
    } else {
        perror("fork failed");
        return 0;
    }
}

/* Clean up temporary files */
void cleanup(void) {
    char cleanup_cmd[MAX_CMD_LEN];
    
    if (temp_dir != NULL && strncmp(temp_dir, "/tmp/gcov_test_", 15) == 0) {
        snprintf(cleanup_cmd, sizeof(cleanup_cmd),
                 "rm -rf %s", temp_dir);
        system(cleanup_cmd);
        printf("Cleaned up temporary directory: %s\n", temp_dir);
    }
}

/* Print test result */
void print_test_result(const char *desc, int passed) {
    printf("  %-50s [%s]\n", desc, passed ? "PASS" : "FAIL");
}

int main(int argc, char *argv[]) {
    int i, passed, total, result;
    
    /* Override gcov-tool path if provided */
    if (argc > 1) {
        gcov_tool_path = argv[1];
    }
    
    printf("=== GCOV-Tool Overlap Subcommand Test ===\n");
    printf("Using gcov-tool: %s\n", gcov_tool_path);
    
    /* Set up test environment */
    if (create_temp_directory() < 0) {
        return 1;
    }
    
    /* Register cleanup handler */
    atexit(cleanup);
    
    /* Create and compile test program */
    if (compile_test_program() < 0) {
        return 1;
    }
    
    /* Generate gcda files */
    if (generate_gcda_files(3) < 0) {
        return 1;
    }
    
    /* Run test cases */
    printf("\n=== Running Test Cases ===\n");
    passed = 0;
    total = 0;
    
    for (i = 0; test_cases[i].description != NULL; i++) {
        result = run_gcov_tool(test_cases[i].command, test_cases[i].expected_exit_code);
        print_test_result(test_cases[i].description, result);
        
        if (result) {
            passed++;
        }
        total++;
        
        printf("\n");
    }
    
    /* Additional permutation tests */
    printf("\n=== Additional Permutation Tests ===\n");
    
    /* Test different threshold values */
    float thresholds[] = {0.1, 0.25, 0.5, 0.75, 0.99, 1.0, 5.0, 10.0};
    for (i = 0; i < sizeof(thresholds)/sizeof(thresholds[0]); i++) {
        char cmd[100];
        snprintf(cmd, sizeof(cmd), "overlap -t %f", thresholds[i]);
        
        printf("Testing threshold: %f\n", thresholds[i]);
        result = run_gcov_tool(cmd, 0);
        print_test_result(cmd, result);
        
        if (result) passed++;
        total++;
    }
    
    /* Test flag permutations */
    printf("\n=== Flag Permutation Tests ===\n");
    char *flags[] = {"-v", "-f", "-F", "-o", "-h"};
    int num_flags = sizeof(flags)/sizeof(flags[0]);
    
    /* Generate all combinations of 2 flags */
    for (i = 0; i < num_flags; i++) {
        int j;
        for (j = i + 1; j < num_flags; j++) {
            char cmd[100];
            snprintf(cmd, sizeof(cmd), "overlap %s %s", flags[i], flags[j]);
            
            printf("Testing combination: %s %s\n", flags[i], flags[j]);
            result = run_gcov_tool(cmd, 0);
            print_test_result(cmd, result);
            
            if (result) passed++;
            total++;
        }
    }
    
    /* Test with threshold combined with other flags */
    for (i = 0; i < num_flags; i++) {
        char cmd[100];
        snprintf(cmd, sizeof(cmd), "overlap %s -t 0.5", flags[i]);
        
        printf("Testing: %s with threshold\n", flags[i]);
        result = run_gcov_tool(cmd, 0);
        print_test_result(cmd, result);
        
        if (result) passed++;
        total++;
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", total - passed);
    printf("Success rate: %.1f%%\n", (float)passed / total * 100);
    
    if (passed == total) {
        printf("\nAll tests passed! The uncovered lines in parse_overlap_options should be exercised.\n");
        printf("To verify coverage, run:\n");
        printf("  gcov gcov-tool.cc\n");
        printf("And check that lines 534-554 are marked as executed.\n");
    } else {
        printf("\nSome tests failed. Check the output above for details.\n");
    }
    
    return (passed == total) ? 0 : 1;
}
