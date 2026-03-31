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

/* Execute a shell command and return exit status */
int run_command(const char *cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Create a temporary directory for test files */
char *create_temp_dir() {
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

/* Generate GCOV data files */
int generate_gcov_data(const char *dir, int num_files) {
    char cmd[MAX_CMD_LEN];
    char src_path[MAX_CMD_LEN];
    char exe_path[MAX_CMD_LEN];
    
    /* Write test program to file */
    snprintf(src_path, sizeof(src_path), "%s/test.c", dir);
    FILE *fp = fopen(src_path, "w");
    if (!fp) {
        perror("Failed to create test.c");
        return 0;
    }
    fputs(test_program, fp);
    fclose(fp);
    
    /* Compile with GCOV instrumentation */
    snprintf(exe_path, sizeof(exe_path), "%s/test", dir);
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -o %s %s", 
             exe_path, src_path);
    if (run_command(cmd) != 0) {
        fprintf(stderr, "Compilation failed\n");
        return 0;
    }
    
    /* Run multiple times to generate different .gcda files */
    for (int i = 0; i < num_files; i++) {
        /* Set GCOV_PREFIX to separate .gcda files */
        snprintf(cmd, sizeof(cmd), "cd %s && GCOV_PREFIX=. GCOV_PREFIX_STRIP=0 ./test", dir);
        if (run_command(cmd) != 0) {
            fprintf(stderr, "Execution %d failed\n", i);
            return 0;
        }
        
        /* Rename .gcda file to preserve it */
        if (i > 0) {
            snprintf(cmd, sizeof(cmd), "mv %s/test.gcda %s/test%d.gcda", dir, dir, i);
            run_command(cmd);
        }
    }
    
    return 1;
}

/* Run a single test case */
int run_test_case(const char *dir, const test_case_t *test) {
    char full_cmd[MAX_CMD_LEN];
    char gcda_files[MAX_CMD_LEN * 2];
    
    /* Build list of .gcda files */
    snprintf(gcda_files, sizeof(gcda_files), "%s/test.gcda", dir);
    for (int i = 1; i < 3; i++) {
        char temp[MAX_CMD_LEN];
        snprintf(temp, sizeof(temp), " %s/test%d.gcda", dir, i);
        strcat(gcda_files, temp);
    }
    
    /* Construct full command */
    snprintf(full_cmd, sizeof(full_cmd), "%s %s", test->cmd, gcda_files);
    
    printf("\n=== Test: %s ===\n", test->description);
    printf("Command: %s\n", full_cmd);
    
    int exit_code = run_command(full_cmd);
    printf("Exit code: %d (expected: %d)\n", exit_code, test->expected_exit);
    
    /* For invalid commands, we accept any non-zero exit code */
    if (test->expected_exit == 0) {
        return exit_code == 0;
    } else {
        return exit_code != 0;
    }
}

int main(int argc, char *argv[]) {
    char *temp_dir = NULL;
    int passed = 0, total = 0;
    
    /* Get path to gcov-tool from environment or use default */
    char *gcov_tool_path = getenv("GCOV_TOOL_PATH");
    if (!gcov_tool_path) {
        gcov_tool_path = "./gcov-tool";
    }
    
    printf("Using gcov-tool at: %s\n", gcov_tool_path);
    
    /* Create temporary directory */
    temp_dir = create_temp_dir();
    if (!temp_dir) {
        return 1;
    }
    
    /* Generate GCOV data files */
    if (!generate_gcov_data(temp_dir, 3)) {
        fprintf(stderr, "Failed to generate GCOV data\n");
        free(temp_dir);
        return 1;
    }
    
    /* Define test cases */
    test_case_t tests[] = {
        /* Basic flag combinations covering all uncovered cases */
        {.cmd = "gcov-tool overlap -v -f -F -o -h -t 0.5", 
         .expected_exit = 0,
         .description = "All flags together"},
        
        {.cmd = "gcov-tool overlap -t 1.0 -h -o -F -f -v", 
         .expected_exit = 0,
         .description = "All flags reversed order"},
        
        {.cmd = "gcov-tool overlap -v", 
         .expected_exit = 0,
         .description = "Verbose flag only"},
        
        {.cmd = "gcov-tool overlap -f", 
         .expected_exit = 0,
         .description = "Function level flag only"},
        
        {.cmd = "gcov-tool overlap -F", 
         .expected_exit = 0,
         .description = "Full filename flag only"},
        
        {.cmd = "gcov-tool overlap -o", 
         .expected_exit = 0,
         .description = "Object level flag only"},
        
        {.cmd = "gcov-tool overlap -h", 
         .expected_exit = 0,
         .description = "Hot only flag only"},
        
        {.cmd = "gcov-tool overlap -t 0.75", 
         .expected_exit = 0,
         .description = "Threshold flag only"},
        
        /* Various permutations */
        {.cmd = "gcov-tool overlap -v -f -t 0.3", 
         .expected_exit = 0,
         .description = "Verbose + function + threshold"},
        
        {.cmd = "gcov-tool overlap -F -o -h", 
         .expected_exit = 0,
         .description = "Fullname + object + hot only"},
        
        {.cmd = "gcov-tool overlap -t 0.9 -v -f", 
         .expected_exit = 0,
         .description = "Threshold + verbose + function"},
        
        /* Edge cases for threshold */
        {.cmd = "gcov-tool overlap -t 0.0", 
         .expected_exit = 0,
         .description = "Zero threshold"},
        
        {.cmd = "gcov-tool overlap -t 1.0", 
         .expected_exit = 0,
         .description = "Max threshold"},
        
        {.cmd = "gcov-tool overlap -t 0.123456", 
         .expected_exit = 0,
         .description = "Precise threshold"},
        
        /* Repeated flags */
        {.cmd = "gcov-tool overlap -v -v -v", 
         .expected_exit = 0,
         .description = "Repeated verbose flag"},
        
        {.cmd = "gcov-tool overlap -f -f -t 0.5 -f", 
         .expected_exit = 0,
         .description = "Multiple function flags"},
        
        /* Invalid cases to test error handling */
        {.cmd = "gcov-tool overlap -t not_a_number", 
         .expected_exit = 1,
         .description = "Invalid threshold (non-numeric)"},
        
        {.cmd = "gcov-tool overlap -t", 
         .expected_exit = 1,
         .description = "Missing threshold argument"},
        
        {.cmd = "gcov-tool overlap -x", 
         .expected_exit = 1,
         .description = "Unknown flag (triggers default case)"},
        
        {.cmd = "gcov-tool overlap -t 0.5 -x", 
         .expected_exit = 1,
         .description = "Valid flag followed by unknown flag"},
        
        {NULL, 0, NULL} /* Sentinel */
    };
    
    /* Run all test cases */
    for (int i = 0; tests[i].cmd != NULL; i++) {
        total++;
        
        /* Replace "gcov-tool" with actual path */
        char actual_cmd[MAX_CMD_LEN];
        char *placeholder = strstr(tests[i].cmd, "gcov-tool");
        if (placeholder) {
            int prefix_len = placeholder - tests[i].cmd;
            strncpy(actual_cmd, tests[i].cmd, prefix_len);
            actual_cmd[prefix_len] = '\0';
            strcat(actual_cmd, gcov_tool_path);
            strcat(actual_cmd, tests[i].cmd + prefix_len + strlen("gcov-tool"));
            
            test_case_t modified_test = tests[i];
            modified_test.cmd = actual_cmd;
            
            if (run_test_case(temp_dir, &modified_test)) {
                passed++;
                printf("Result: PASS\n");
            } else {
                printf("Result: FAIL\n");
            }
        }
        
        printf("\n");
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d\n", passed, total);
    printf("Failed: %d\n", total - passed);
    
    /* Cleanup */
    if (temp_dir) {
        char cmd[MAX_CMD_LEN];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
        run_command(cmd);
        free(temp_dir);
    }
    
    return (passed == total) ? 0 : 1;
}
