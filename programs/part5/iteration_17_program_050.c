#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_PATH 1024
#define MAX_CMD 4096

/* Simple test program that will be compiled with coverage instrumentation */
const char *test_source = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"\n"
"int main(int argc, char *argv[]) {\n"
"    int i, limit = 10;\n"
"    int sum = 0;\n"
"    \n"
"    /* Different behavior based on environment variable */\n"
"    char *env_limit = getenv(\"TEST_LIMIT\");\n"
"    if (env_limit) {\n"
"        limit = atoi(env_limit);\n"
"    }\n"
"    \n"
"    /* Different behavior based on command line argument */\n"
"    if (argc > 1) {\n"
"        limit = atoi(argv[1]);\n"
"    }\n"
"    \n"
"    /* Loop with conditional */\n"
"    for (i = 0; i < limit; i++) {\n"
"        if (i % 2 == 0) {\n"
"            sum += i * 2;\n"
"        } else {\n"
"            sum += i;\n"
"        }\n"
"    }\n"
"    \n"
"    printf(\"Sum: %d\\n\", sum);\n"
"    return 0;\n"
"}\n";

/* Function to create a temporary directory */
char *create_temp_dir() {
    char *template = strdup("/tmp/gcov_test_XXXXXX");
    char *dir = mkdtemp(template);
    if (!dir) {
        perror("Failed to create temp directory");
        exit(EXIT_FAILURE);
    }
    return dir;
}

/* Function to write the test source file */
void write_test_file(const char *dir, const char *filename) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s/%s", dir, filename);
    
    FILE *f = fopen(path, "w");
    if (!f) {
        perror("Failed to create test source file");
        exit(EXIT_FAILURE);
    }
    
    fputs(test_source, f);
    fclose(f);
}

/* Function to compile the test program with coverage */
void compile_with_coverage(const char *dir, const char *source, const char *output) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), 
             "cd %s && gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s 2>&1",
             dir, source, output);
    
    printf("Compiling: %s\n", cmd);
    
    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        perror("Failed to compile");
        exit(EXIT_FAILURE);
    }
    
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        printf("%s", buffer);
    }
    
    int status = pclose(pipe);
    if (status != 0) {
        fprintf(stderr, "Compilation failed with status %d\n", status);
        exit(EXIT_FAILURE);
    }
}

/* Function to run the test program and generate .gcda files */
void run_test_program(const char *dir, const char *executable, 
                      const char *gcda_suffix, const char *arg) {
    char cmd[MAX_CMD];
    
    /* Set GCOV_PREFIX to ensure .gcda files go to our temp directory */
    setenv("GCOV_PREFIX", dir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    if (arg) {
        snprintf(cmd, sizeof(cmd), "cd %s && %s/%s %s > /dev/null 2>&1",
                 dir, dir, executable, arg);
    } else {
        snprintf(cmd, sizeof(cmd), "cd %s && %s/%s > /dev/null 2>&1",
                 dir, dir, executable);
    }
    
    printf("Running test: %s\n", cmd);
    
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Test execution failed with status %d\n", status);
    }
    
    /* Rename the .gcda file to have a unique name */
    char old_gcda[MAX_PATH];
    char new_gcda[MAX_PATH];
    snprintf(old_gcda, sizeof(old_gcda), "%s/%s.gcda", dir, executable);
    snprintf(new_gcda, sizeof(new_gcda), "%s/%s_%s.gcda", dir, executable, gcda_suffix);
    
    if (rename(old_gcda, new_gcda) != 0 && errno != ENOENT) {
        perror("Failed to rename .gcda file");
    }
}

/* Function to execute gcov-tool with given arguments */
void run_gcov_tool(const char *dir, const char *gcda1, const char *gcda2, 
                   const char *extra_args, int expect_failure) {
    char gcov_tool_path[MAX_PATH] = "/usr/bin/gcov-tool";
    
    /* Try to get gcov-tool path from environment */
    char *env_path = getenv("GCOV_TOOL");
    if (env_path && strlen(env_path) > 0) {
        strncpy(gcov_tool_path, env_path, sizeof(gcov_tool_path) - 1);
    }
    
    /* Check if gcov-tool exists */
    if (access(gcov_tool_path, X_OK) != 0) {
        fprintf(stderr, "gcov-tool not found at %s\n", gcov_tool_path);
        fprintf(stderr, "Set GCOV_TOOL environment variable to correct path\n");
        exit(EXIT_FAILURE);
    }
    
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "%s %s %s/%s %s/%s 2>&1",
             gcov_tool_path, extra_args, dir, gcda1, dir, gcda2);
    
    printf("\nExecuting gcov-tool: %s\n", cmd);
    
    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        perror("Failed to execute gcov-tool");
        return;
    }
    
    char buffer[1024];
    printf("Output:\n");
    while (fgets(buffer, sizeof(buffer), pipe)) {
        printf("%s", buffer);
    }
    
    int status = pclose(pipe);
    printf("Exit status: %d\n", status);
    
    if (expect_failure) {
        if (status == 0) {
            printf("WARNING: Expected failure but command succeeded\n");
        }
    } else {
        if (status != 0) {
            printf("WARNING: Command failed with status %d\n", status);
        }
    }
}

/* Clean up temporary directory */
void cleanup(const char *dir) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dir);
    
    printf("\nCleaning up: %s\n", dir);
    system(cmd);
}

int main(int argc, char *argv[]) {
    printf("=== Starting gcov-tool coverage test ===\n");
    
    /* Step 1: Create temporary directory */
    char *temp_dir = create_temp_dir();
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Step 2: Write and compile test program */
    const char *source_file = "test_func.c";
    const char *executable = "test_prog";
    
    write_test_file(temp_dir, source_file);
    compile_with_coverage(temp_dir, source_file, executable);
    
    /* Step 3: Run test program multiple times to generate different .gcda files */
    
    /* First run with default behavior */
    printf("\n--- First run (default) ---\n");
    unsetenv("TEST_LIMIT");
    run_test_program(temp_dir, executable, "run1", NULL);
    
    /* Second run with environment variable */
    printf("\n--- Second run (env var TEST_LIMIT=5) ---\n");
    setenv("TEST_LIMIT", "5", 1);
    run_test_program(temp_dir, executable, "run2", NULL);
    
    /* Third run with command line argument */
    printf("\n--- Third run (arg 7) ---\n");
    unsetenv("TEST_LIMIT");
    run_test_program(temp_dir, executable, "run3", "7");
    
    /* Step 4: Run gcov-tool with all the flags to trigger the uncovered code */
    printf("\n=== Testing gcov-tool overlap analysis flags ===\n");
    
    /* Construct gcda file paths */
    char gcda1[MAX_PATH], gcda2[MAX_PATH], gcda3[MAX_PATH];
    snprintf(gcda1, sizeof(gcda1), "%s_gcda", executable);
    snprintf(gcda2, sizeof(gcda2), "%s_run2.gcda", executable);
    snprintf(gcda3, sizeof(gcda3), "%s_run3.gcda", executable);
    
    /* Test 1: All valid flags to trigger case statements */
    printf("\n--- Test 1: Valid flags (triggering case 'v', 'f', 'F', 'o', 'h', 't') ---\n");
    run_gcov_tool(temp_dir, gcda2, gcda3, 
                  "-v -f -F -o -h -t 0.75 overlap", 0);
    
    /* Test 2: Invalid flag to trigger default case and overlap_usage() */
    printf("\n--- Test 2: Invalid flag (triggering default case) ---\n");
    run_gcov_tool(temp_dir, gcda2, gcda3, 
                  "-v -f -Z -o overlap", 1);
    
    /* Test 3: Just invalid flag */
    printf("\n--- Test 3: Only invalid flag ---\n");
    run_gcov_tool(temp_dir, gcda2, gcda3, 
                  "-Z overlap", 1);
    
    /* Test 4: Different threshold value */
    printf("\n--- Test 4: Different threshold ---\n");
    run_gcov_tool(temp_dir, gcda2, gcda3, 
                  "-v -t 0.5 overlap", 0);
    
    /* Step 5: Cleanup */
    cleanup(temp_dir);
    free(temp_dir);
    
    printf("\n=== Test completed ===\n");
    return 0;
}
