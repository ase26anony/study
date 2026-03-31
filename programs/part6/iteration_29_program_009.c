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
    int expected_exit_code;  /* 0 for success, non-zero for failure */
} test_case_t;

/* Global variables for temporary directory */
char temp_dir[256];
char test_prog_path[256];
char gcda_files[MAX_FILES][256];
int num_gcda_files = 0;

/* Function prototypes */
int create_temp_directory(void);
int compile_test_program(void);
int run_test_program(void);
int generate_gcda_files(int count);
void cleanup(void);
int run_gcov_tool(const char *args, int *exit_code);
void run_test_suite(void);

/* Create a temporary directory for test files */
int create_temp_directory(void) {
    snprintf(temp_dir, sizeof(temp_dir), "/tmp/gcov_test_%d", getpid());
    
    if (mkdir(temp_dir, 0755) == -1) {
        perror("Failed to create temporary directory");
        return 0;
    }
    
    printf("Created temporary directory: %s\n", temp_dir);
    return 1;
}

/* Compile a simple test program with GCOV instrumentation */
int compile_test_program(void) {
    const char *test_prog = 
        "#include <stdio.h>\n"
        "int main() {\n"
        "    printf(\"Test program for gcov-tool\\n\");\n"
        "    for (int i = 0; i < 10; i++) {\n"
        "        if (i % 2 == 0) {\n"
        "            printf(\"Even: %d\\n\", i);\n"
        "        } else {\n"
        "            printf(\"Odd: %d\\n\", i);\n"
        "        }\n"
        "    }\n"
        "    return 0;\n"
        "}\n";
    
    /* Write test program to file */
    char src_path[256];
    snprintf(src_path, sizeof(src_path), "%s/test_prog.c", temp_dir);
    
    FILE *fp = fopen(src_path, "w");
    if (!fp) {
        perror("Failed to create test program source");
        return 0;
    }
    fputs(test_prog, fp);
    fclose(fp);
    
    /* Compile with GCOV instrumentation */
    char cmd[512];
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -o %s/test_prog %s/test_prog.c 2>&1",
             temp_dir, temp_dir);
    
    printf("Compiling test program...\n");
    int result = system(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        return 0;
    }
    
    snprintf(test_prog_path, sizeof(test_prog_path), "%s/test_prog", temp_dir);
    return 1;
}

/* Run the test program to generate .gcda files */
int run_test_program(void) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s/test_prog > /dev/null 2>&1", temp_dir);
    
    printf("Running test program to generate .gcda file...\n");
    return system(cmd) == 0;
}

/* Generate multiple .gcda files by running the test program multiple times */
int generate_gcda_files(int count) {
    if (count > MAX_FILES) {
        count = MAX_FILES;
    }
    
    /* First, clean any existing .gcda files */
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -f %s/*.gcda 2>/dev/null", temp_dir);
    system(cmd);
    
    /* Run the program multiple times to generate different profiles */
    for (int i = 0; i < count; i++) {
        printf("Generating gcda file %d/%d...\n", i + 1, count);
        
        /* Run the program */
        if (!run_test_program()) {
            fprintf(stderr, "Failed to run test program\n");
            return 0;
        }
        
        /* Copy the .gcda file to preserve it */
        char src_gcda[256];
        char dest_gcda[256];
        snprintf(src_gcda, sizeof(src_gcda), "%s/test_prog.gcda", temp_dir);
        snprintf(dest_gcda, sizeof(dest_gcda), "%s/test_prog_%d.gcda", temp_dir, i);
        
        /* Use system to copy the file */
        snprintf(cmd, sizeof(cmd), "cp %s %s 2>/dev/null", src_gcda, dest_gcda);
        system(cmd);
        
        /* Store the path for later use */
        snprintf(gcda_files[i], sizeof(gcda_files[i]), "%s/test_prog_%d.gcda", temp_dir, i);
    }
    
    num_gcda_files = count;
    return 1;
}

/* Run gcov-tool with specified arguments and capture exit code */
int run_gcov_tool(const char *args, int *exit_code) {
    char cmd[MAX_CMD_LEN];
    int result;
    
    /* Build the command string */
    snprintf(cmd, sizeof(cmd), "gcov-tool %s", args);
    
    printf("Executing: %s\n", cmd);
    
    /* Use system() to run the command */
    result = system(cmd);
    
    /* Extract exit code from system() return value */
    if (WIFEXITED(result)) {
        *exit_code = WEXITSTATUS(result);
    } else {
        *exit_code = -1;  /* Process didn't exit normally */
    }
    
    return 1;
}

/* Clean up temporary files */
void cleanup(void) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
    system(cmd);
    printf("Cleaned up temporary directory: %s\n", temp_dir);
}

/* Main test suite */
void run_test_suite(void) {
    int total_tests = 0;
    int passed_tests = 0;
    int exit_code;
    
    printf("\n=== Running gcov-tool overlap argument parsing tests ===\n\n");
    
    /* Test 1: Basic test with all uncovered flags */
    printf("Test 1: All uncovered flags (-v -f -F -o -h -t)\n");
    char cmd1[MAX_CMD_LEN];
    snprintf(cmd1, sizeof(cmd1), "overlap -v -f -F -o -h -t 0.75 %s %s", 
             gcda_files[0], gcda_files[1]);
    run_gcov_tool(cmd1, &exit_code);
    printf("Exit code: %d\n\n", exit_code);
    total_tests++;
    
    /* Test 2: Different order of flags */
    printf("Test 2: Different flag order\n");
    char cmd2[MAX_CMD_LEN];
    snprintf(cmd2, sizeof(cmd2), "overlap -t 1.0 -h -o -F -f -v %s %s", 
             gcda_files[0], gcda_files[1]);
    run_gcov_tool(cmd2, &exit_code);
    printf("Exit code: %d\n\n", exit_code);
    total_tests++;
    
    /* Test 3: Single flags individually (to ensure each case is hit) */
    printf("Test 3: Verbose flag only\n");
    char cmd3[MAX_CMD_LEN];
    snprintf(cmd3, sizeof(cmd3), "overlap -v %s %s", gcda_files[0], gcda_files[1]);
    run_gcov_tool(cmd3, &exit_code);
    printf("Exit code: %d\n\n", exit_code);
    total_tests++;
    
    printf("Test 4: Function level flag only\n");
    char cmd4[MAX_CMD_LEN];
    snprintf(cmd4, sizeof(cmd4), "overlap -f %s %s", gcda_files[0], gcda_files[1]);
    run_gcov_tool(cmd4, &exit_code);
    printf("Exit code: %d\n\n", exit_code);
    total_tests++;
    
    printf("Test 5: Fullname flag only\n");
    char cmd5[MAX_CMD_LEN];
    snprintf(cmd5, sizeof(cmd5), "overlap -F %s %s", gcda_files[0], gcda_files[1]);
    run_gcov_tool(cmd5, &exit_code);
    printf("Exit code: %d\n\n", exit_code);
    total_tests++;
    
    printf("Test 6: Object level flag only\n");
    char cmd6[MAX_CMD_LEN];
    snprintf(cmd6, sizeof(cmd6), "overlap -o %s %s", gcda_files[0], gcda_files[1]);
    run_gcov_tool(cmd6, &exit_code);
    printf("Exit code: %d\n\n", exit_code);
    total_tests++;
    
    printf("Test 7: Hot only flag only\n");
    char cmd7[MAX_CMD_LEN];
    snprintf(cmd7, sizeof(cmd7), "overlap -h %s %s", gcda_files[0], gcda_files[1]);
    run_gcov_tool(cmd7, &exit_code);
    printf("Exit code: %d\n\n", exit_code);
    total_tests++;
    
    printf("Test 8: Threshold flag only\n");
    char cmd8[MAX_CMD_LEN];
    snprintf(cmd8, sizeof(cmd8), "overlap -t 0.5 %s %s", gcda_files[0], gcda_files[1]);
    run_gcov_tool(cmd8, &exit_code);
    printf("Exit code: %d\n\n", exit_code);
    total_tests++;
    
    /* Test 9: Different threshold values */
    printf("Test 9: Different threshold values\n");
    float thresholds[] = {0.1, 0.25, 0.5, 0.75, 0.99, 1.0};
    for (int i = 0; i < 6; i++) {
        char cmd9[MAX_CMD_LEN];
        snprintf(cmd9, sizeof(cmd9), "overlap -t %.2f %s %s", 
                 thresholds[i], gcda_files[0], gcda_files[1]);
        printf("  Threshold %.2f: ", thresholds[i]);
        run_gcov_tool(cmd9, &exit_code);
        printf("  Exit code: %d\n", exit_code);
        total_tests++;
    }
    printf("\n");
    
    /* Test 10: Edge case - invalid argument for -t */
    printf("Test 10: Invalid argument for -t (should trigger error)\n");
    char cmd10[MAX_CMD_LEN];
    snprintf(cmd10, sizeof(cmd10), "overlap -t not_a_number %s %s", 
             gcda_files[0], gcda_files[1]);
    run_gcov_tool(cmd10, &exit_code);
    printf("Exit code: %d (non-zero expected for error)\n\n", exit_code);
    total_tests++;
    
    /* Test 11: Edge case - missing argument for -t */
    printf("Test 11: Missing argument for -t (should trigger error)\n");
    char cmd11[MAX_CMD_LEN];
    snprintf(cmd11, sizeof(cmd11), "overlap -t %s %s", gcda_files[0], gcda_files[1]);
    run_gcov_tool(cmd11, &exit_code);
    printf("Exit code: %d (non-zero expected for error)\n\n", exit_code);
    total_tests++;
    
    /* Test 12: Unknown flag (should trigger default case) */
    printf("Test 12: Unknown flag -x (should trigger default case)\n");
    char cmd12[MAX_CMD_LEN];
    snprintf(cmd12, sizeof(cmd12), "overlap -x %s %s", gcda_files[0], gcda_files[1]);
    run_gcov_tool(cmd12, &exit_code);
    printf("Exit code: %d (non-zero expected for error)\n\n", exit_code);
    total_tests++;
    
    /* Test 13: Repeated flags */
    printf("Test 13: Repeated flags (-v -v -f -f)\n");
    char cmd13[MAX_CMD_LEN];
    snprintf(cmd13, sizeof(cmd13), "overlap -v -v -f -f %s %s", 
             gcda_files[0], gcda_files[1]);
    run_gcov_tool(cmd13, &exit_code);
    printf("Exit code: %d\n\n", exit_code);
    total_tests++;
    
    /* Test 14: Combination with different file counts */
    printf("Test 14: Multiple input files (3 files)\n");
    char cmd14[MAX_CMD_LEN];
    snprintf(cmd14, sizeof(cmd14), "overlap -v -f -t 0.8 %s %s %s", 
             gcda_files[0], gcda_files[1], gcda_files[2]);
    run_gcov_tool(cmd14, &exit_code);
    printf("Exit code: %d\n\n", exit_code);
    total_tests++;
    
    /* Test 15: Using absolute paths */
    printf("Test 15: Using absolute paths\n");
    char cmd15[MAX_CMD_LEN];
    char abs_path1[256], abs_path2[256];
    realpath(gcda_files[0], abs_path1);
    realpath(gcda_files[1], abs_path2);
    snprintf(cmd15, sizeof(cmd15), "overlap -v -F -t 0.3 %s %s", abs_path1, abs_path2);
    run_gcov_tool(cmd15, &exit_code);
    printf("Exit code: %d\n\n", exit_code);
    total_tests++;
    
    /* Test 16: Minimal valid command */
    printf("Test 16: Minimal valid command (no flags)\n");
    char cmd16[MAX_CMD_LEN];
    snprintf(cmd16, sizeof(cmd16), "overlap %s %s", gcda_files[0], gcda_files[1]);
    run_gcov_tool(cmd16, &exit_code);
    printf("Exit code: %d\n\n", exit_code);
    total_tests++;
    
    printf("=== Test suite completed: %d tests executed ===\n", total_tests);
}

int main(int argc, char *argv[]) {
    printf("=== GCOV-Tool Overlap Argument Parser Test Driver ===\n");
    
    /* Set up signal handler for cleanup */
    atexit(cleanup);
    
    /* Create temporary directory */
    if (!create_temp_directory()) {
        fprintf(stderr, "Failed to set up test environment\n");
        return 1;
    }
    
    /* Compile test program */
    if (!compile_test_program()) {
        fprintf(stderr, "Failed to compile test program\n");
        cleanup();
        return 1;
    }
    
    /* Generate multiple .gcda files */
    if (!generate_gcda_files(3)) {
        fprintf(stderr, "Failed to generate .gcda files\n");
        cleanup();
        return 1;
    }
    
    /* Run the test suite */
    run_test_suite();
    
    /* Note: cleanup() will be called via atexit() */
    printf("\nNote: To get coverage data for gcov-tool itself, ensure it was built with:\n");
    printf("  --enable-coverage flag during GCC build\n");
    printf("Then run: gcov gcov-tool.cc\n");
    
    return 0;
}
