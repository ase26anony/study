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
    const char *description;
    const char *args;
    int expected_exit_code;
} test_case_t;

/* Global variables for test configuration */
static char temp_dir[256];
static char test_prog_path[256];
static char gcda_files[2][256];

/* Function prototypes */
static void setup_test_environment(void);
static void cleanup_test_environment(void);
static int compile_test_program(void);
static int run_test_program(void);
static int execute_gcov_tool(const char *args, int *exit_code);
static void run_test_suite(void);

int main(void) {
    printf("=== GCOV-Tool Overlap Parser Test Suite ===\n\n");
    
    /* Setup test environment */
    setup_test_environment();
    
    /* Compile and run test program to generate .gcda files */
    if (compile_test_program() != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        cleanup_test_environment();
        return 1;
    }
    
    if (run_test_program() != 0) {
        fprintf(stderr, "Failed to run test program\n");
        cleanup_test_environment();
        return 1;
    }
    
    /* Run the test suite */
    run_test_suite();
    
    /* Cleanup */
    cleanup_test_environment();
    
    printf("\n=== Test Suite Complete ===\n");
    return 0;
}

static void setup_test_environment(void) {
    /* Create a temporary directory for test files */
    snprintf(temp_dir, sizeof(temp_dir), "/tmp/gcov_test_%d", getpid());
    mkdir(temp_dir, 0755);
    
    /* Set paths for test files */
    snprintf(test_prog_path, sizeof(test_prog_path), "%s/test_prog", temp_dir);
    snprintf(gcda_files[0], sizeof(gcda_files[0]), "%s/test_prog.gcda", temp_dir);
    snprintf(gcda_files[1], sizeof(gcda_files[1]), "%s/test_prog2.gcda", temp_dir);
    
    printf("Test directory: %s\n", temp_dir);
}

static void cleanup_test_environment(void) {
    /* Remove all temporary files */
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
    system(cmd);
}

static int compile_test_program(void) {
    /* Create a simple C program for testing */
    char source_path[256];
    snprintf(source_path, sizeof(source_path), "%s/test.c", temp_dir);
    
    FILE *fp = fopen(source_path, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return -1;
    }
    
    /* Write a simple test program */
    fprintf(fp, "#include <stdio.h>\n");
    fprintf(fp, "int main() {\n");
    fprintf(fp, "    printf(\"Test program running\\n\");\n");
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
    char cmd[512];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -o %s %s 2>&1",
             test_prog_path, source_path);
    
    printf("Compiling test program...\n");
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Compilation failed\n");
        return -1;
    }
    
    /* Create a second .gcda file by copying and modifying if needed */
    snprintf(cmd, sizeof(cmd), "cp %s %s/test.c2", source_path, temp_dir);
    system(cmd);
    
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -o %s/test_prog2 %s/test.c2 2>&1",
             temp_dir, temp_dir);
    
    printf("Compiling second test program...\n");
    result = system(cmd);
    
    return (result == 0) ? 0 : -1;
}

static int run_test_program(void) {
    /* Run the test program to generate .gcda files */
    printf("Running test programs to generate .gcda files...\n");
    
    /* Run first program */
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s > /dev/null 2>&1", test_prog_path);
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Failed to run first test program\n");
        return -1;
    }
    
    /* Run second program */
    snprintf(cmd, sizeof(cmd), "%s/test_prog2 > /dev/null 2>&1", temp_dir);
    result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Failed to run second test program\n");
        return -1;
    }
    
    /* Verify .gcda files were created */
    struct stat st;
    if (stat(gcda_files[0], &st) != 0 || stat(gcda_files[1], &st) != 0) {
        fprintf(stderr, ".gcda files not created\n");
        return -1;
    }
    
    printf("Generated .gcda files: %s, %s\n", gcda_files[0], gcda_files[1]);
    return 0;
}

static int execute_gcov_tool(const char *args, int *exit_code) {
    char cmd[MAX_CMD_LEN];
    int result;
    
    /* Construct the full command */
    snprintf(cmd, sizeof(cmd), "gcov-tool %s", args);
    
    printf("Executing: %s\n", cmd);
    
    /* Use system() to execute the command */
    result = system(cmd);
    
    if (WIFEXITED(result)) {
        *exit_code = WEXITSTATUS(result);
    } else {
        *exit_code = -1;
    }
    
    return (result == -1) ? -1 : 0;
}

static void run_test_suite(void) {
    test_case_t test_cases[] = {
        /* Basic flag tests - each flag individually */
        {"Single -v flag", "overlap -v %s", 0},
        {"Single -f flag", "overlap -f %s", 0},
        {"Single -F flag", "overlap -F %s", 0},
        {"Single -o flag", "overlap -o %s", 0},
        {"Single -h flag", "overlap -h %s", 0},
        {"Single -t flag with value", "overlap -t 0.5 %s", 0},
        
        /* All flags combined in one command */
        {"All flags combined", "overlap -v -f -F -o -h -t 0.75 %s %s", 0},
        
        /* Different orders of flags */
        {"Flags in reverse order", "overlap -t 1.0 -h -o -F -f -v %s %s", 0},
        {"Mixed order 1", "overlap -f -v -t 0.25 -F -o -h %s %s", 0},
        {"Mixed order 2", "overlap -h -t 0.9 -o -v -F -f %s %s", 0},
        
        /* Multiple -v flags */
        {"Multiple -v flags", "overlap -v -v -v %s", 0},
        
        /* Edge cases for -t flag */
        {"-t with high threshold", "overlap -t 99.9 %s", 0},
        {"-t with zero threshold", "overlap -t 0.0 %s", 0},
        {"-t with very small threshold", "overlap -t 0.001 %s", 0},
        
        /* Invalid cases that should trigger errors */
        {"Missing argument for -t", "overlap -t", 1},
        {"Invalid argument for -t", "overlap -t not_a_number %s", 1},
        
        /* Unknown flag to trigger default case */
        {"Unknown flag -x", "overlap -x %s", 1},
        
        /* Combination with unknown flag */
        {"Valid flags with unknown", "overlap -v -x -f %s", 1},
        
        /* No flags at all */
        {"No flags", "overlap %s %s", 0},
        
        /* Only input files */
        {"Only input files", "overlap %s %s", 0},
        
        /* Single input file */
        {"Single input file with flags", "overlap -v -f %s", 0},
        
        /* Flags after files */
        {"Flags after files", "overlap %s -v -f", 0},
        
        /* Multiple -t flags (last one wins) */
        {"Multiple -t flags", "overlap -t 0.1 -t 0.2 -t 0.3 %s", 0},
        
        /* Combination with function level and fullname */
        {"Function level and fullname", "overlap -f -F %s %s", 0},
        
        /* Hot only with threshold */
        {"Hot only with specific threshold", "overlap -h -t 50.0 %s %s", 0},
        
        /* Object level with verbosity */
        {"Object level with verbosity", "overlap -o -v %s %s", 0},
        
        /* All boolean flags without -t */
        {"All boolean flags", "overlap -v -f -F -o -h %s %s", 0},
        
        /* End marker */
        {NULL, NULL, 0}
    };
    
    int passed = 0;
    int failed = 0;
    int total = 0;
    
    printf("\n=== Running Test Cases ===\n\n");
    
    for (int i = 0; test_cases[i].description != NULL; i++) {
        total++;
        printf("Test %d: %s\n", total, test_cases[i].description);
        
        /* Format the command arguments with actual file paths */
        char formatted_args[MAX_CMD_LEN];
        if (strstr(test_cases[i].args, "%s %s")) {
            snprintf(formatted_args, sizeof(formatted_args), 
                    test_cases[i].args, gcda_files[0], gcda_files[1]);
        } else if (strstr(test_cases[i].args, "%s")) {
            snprintf(formatted_args, sizeof(formatted_args), 
                    test_cases[i].args, gcda_files[0]);
        } else {
            snprintf(formatted_args, sizeof(formatted_args), "%s", test_cases[i].args);
        }
        
        /* Execute the command */
        int exit_code;
        int exec_result = execute_gcov_tool(formatted_args, &exit_code);
        
        if (exec_result == -1) {
            printf("  ❌ FAILED: Could not execute command\n");
            failed++;
        } else if ((test_cases[i].expected_exit_code == 0 && exit_code != 0) ||
                   (test_cases[i].expected_exit_code != 0 && exit_code == 0)) {
            printf("  ❌ FAILED: Expected exit code %d, got %d\n", 
                   test_cases[i].expected_exit_code, exit_code);
            failed++;
        } else {
            printf("  ✅ PASSED\n");
            passed++;
        }
        
        printf("\n");
    }
    
    /* Additional permutation tests */
    printf("=== Running Flag Permutation Tests ===\n\n");
    
    /* Generate permutations of the 6 main flags */
    const char *flags[] = {"-v", "-f", "-F", "-o", "-h", "-t 0.5"};
    const char *flag_names[] = {"verbose", "func_level", "fullname", "obj_level", "hot_only", "threshold"};
    int num_flags = 6;
    
    /* Test all individual flags */
    for (int i = 0; i < num_flags; i++) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "overlap %s %s %s", 
                flags[i], gcda_files[0], gcda_files[1]);
        
        printf("Testing single flag %s (%s)... ", flags[i], flag_names[i]);
        
        int exit_code;
        if (execute_gcov_tool(cmd, &exit_code) == 0 && exit_code == 0) {
            printf("✅\n");
            passed++;
        } else {
            printf("❌\n");
            failed++;
        }
        total++;
    }
    
    /* Test all pairs of flags */
    for (int i = 0; i < num_flags; i++) {
        for (int j = i + 1; j < num_flags; j++) {
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "overlap %s %s %s %s", 
                    flags[i], flags[j], gcda_files[0], gcda_files[1]);
            
            printf("Testing flags %s + %s... ", flag_names[i], flag_names[j]);
            
            int exit_code;
            if (execute_gcov_tool(cmd, &exit_code) == 0 && exit_code == 0) {
                printf("✅\n");
                passed++;
            } else {
                printf("❌\n");
                failed++;
            }
            total++;
        }
    }
    
    /* Test a selection of triple flag combinations */
    int triple_combinations[][3] = {
        {0, 1, 2},  /* v, f, F */
        {1, 2, 3},  /* f, F, o */
        {2, 3, 4},  /* F, o, h */
        {3, 4, 5},  /* o, h, t */
        {0, 4, 5},  /* v, h, t */
        {0, 2, 4},  /* v, F, h */
    };
    
    for (size_t idx = 0; idx < sizeof(triple_combinations)/sizeof(triple_combinations[0]); idx++) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "overlap %s %s %s %s %s", 
                flags[triple_combinations[idx][0]],
                flags[triple_combinations[idx][1]],
                flags[triple_combinations[idx][2]],
                gcda_files[0], gcda_files[1]);
        
        printf("Testing triple combination %d... ", (int)idx + 1);
        
        int exit_code;
        if (execute_gcov_tool(cmd, &exit_code) == 0 && exit_code == 0) {
            printf("✅\n");
            passed++;
        } else {
            printf("❌\n");
            failed++;
        }
        total++;
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    printf("Success rate: %.1f%%\n", (passed * 100.0) / total);
}

/* Alternative version using fork/exec for better control */
static int execute_gcov_tool_detailed(const char *args) {
    pid_t pid = fork();
    
    if (pid == -1) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        char *argv[64];
        int argc = 0;
        char *token;
        char args_copy[MAX_CMD_LEN];
        
        /* Copy args so we can tokenize it */
        strncpy(args_copy, args, sizeof(args_copy) - 1);
        args_copy[sizeof(args_copy) - 1] = '\0';
        
        /* First token is "gcov-tool" */
        argv[argc++] = "gcov-tool";
        
        /* Tokenize the rest of the arguments */
        token = strtok(args_copy, " ");
        while (token != NULL && argc < 62) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
        argv[argc] = NULL;
        
        /* Execute gcov-tool */
        execvp("gcov-tool", argv);
        
        /* If we get here, exec failed */
        perror("execvp");
        exit(127);
    } else {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return -1;
        }
    }
}
