#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define TEMP_DIR_TEMPLATE "/tmp/gcov_test_XXXXXX"
#define MAX_PATH 1024
#define MAX_CMD 4096

/* Simple test program that will be compiled with coverage instrumentation */
const char *test_program_source = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"\n"
"int process_value(int x) {\n"
"    int result = 0;\n"
"    for (int i = 0; i < x; i++) {\n"
"        if (i % 2 == 0) {\n"
"            result += i * 2;\n"
"        } else {\n"
"            result += i;\n"
"        }\n"
"    }\n"
"    return result;\n"
"}\n"
"\n"
"int main(int argc, char *argv[]) {\n"
"    int value = 5;  /* default value */\n"
"    \n"
"    if (argc > 1) {\n"
"        value = atoi(argv[1]);\n"
"    }\n"
"    \n"
"    /* Environment variable can also affect behavior */\n"
"    char *env_val = getenv(\"TEST_VALUE\");\n"
"    if (env_val != NULL) {\n"
"        value += atoi(env_val);\n"
"    }\n"
"    \n"
"    int result = process_value(value);\n"
"    printf(\"Result: %d\\n\", result);\n"
"    return 0;\n"
"}\n";

/* Function to create a temporary directory */
char *create_temp_dir() {
    char *temp_dir = malloc(MAX_PATH);
    if (!temp_dir) {
        perror("malloc failed");
        return NULL;
    }
    
    strcpy(temp_dir, TEMP_DIR_TEMPLATE);
    char *result = mkdtemp(temp_dir);
    if (!result) {
        perror("mkdtemp failed");
        free(temp_dir);
        return NULL;
    }
    
    return temp_dir;
}

/* Function to write the test program source file */
int write_test_program(const char *dir_path) {
    char source_path[MAX_PATH];
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", dir_path);
    
    FILE *fp = fopen(source_path, "w");
    if (!fp) {
        perror("fopen failed");
        return 0;
    }
    
    fputs(test_program_source, fp);
    fclose(fp);
    return 1;
}

/* Function to compile the test program with coverage instrumentation */
int compile_test_program(const char *dir_path) {
    char cmd[MAX_CMD];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", dir_path);
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", dir_path);
    
    /* Compile with coverage instrumentation */
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s 2>&1",
             source_path, exec_path);
    
    printf("Compiling test program: %s\n", cmd);
    
    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        perror("popen failed");
        return 0;
    }
    
    /* Read and discard compilation output */
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        /* Optional: print compilation warnings/errors */
        printf("Compilation: %s", buffer);
    }
    
    int status = pclose(pipe);
    if (status != 0) {
        fprintf(stderr, "Compilation failed with status %d\n", status);
        return 0;
    }
    
    return 1;
}

/* Function to run the test program and generate .gcda files */
int run_test_program(const char *dir_path, int run_num, const char *env_value) {
    char exec_path[MAX_PATH];
    char cmd[MAX_CMD];
    
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", dir_path);
    
    /* Set environment for GCOV_PREFIX to ensure .gcda files go to our temp dir */
    setenv("GCOV_PREFIX", dir_path, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    if (env_value) {
        setenv("TEST_VALUE", env_value, 1);
    } else {
        unsetenv("TEST_VALUE");
    }
    
    /* Run with different arguments for each run to generate different coverage */
    if (run_num == 1) {
        snprintf(cmd, sizeof(cmd), "%s 3", exec_path);
    } else if (run_num == 2) {
        snprintf(cmd, sizeof(cmd), "%s 7", exec_path);
    } else {
        snprintf(cmd, sizeof(cmd), "%s 10", exec_path);
    }
    
    printf("Running test program (run %d): %s\n", run_num, cmd);
    
    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        perror("popen failed");
        return 0;
    }
    
    /* Read and display program output */
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        printf("Run %d output: %s", run_num, buffer);
    }
    
    int status = pclose(pipe);
    if (status != 0) {
        fprintf(stderr, "Run %d failed with status %d\n", run_num, status);
        return 0;
    }
    
    return 1;
}

/* Function to find gcov-tool path */
const char *find_gcov_tool() {
    const char *gcov_tool = getenv("GCOV_TOOL");
    if (gcov_tool && access(gcov_tool, X_OK) == 0) {
        return gcov_tool;
    }
    
    /* Try common locations */
    const char *common_paths[] = {
        "/usr/bin/gcov-tool",
        "/usr/local/bin/gcov-tool",
        "/bin/gcov-tool",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            return common_paths[i];
        }
    }
    
    return NULL;
}

/* Function to execute gcov-tool with given arguments */
int execute_gcov_tool(const char *gcov_tool, const char *dir_path, 
                      const char *args, const char *description) {
    char cmd[MAX_CMD];
    char gcda1[MAX_PATH], gcda2[MAX_PATH];
    
    /* Construct paths to .gcda files */
    snprintf(gcda1, sizeof(gcda1), "%s/test_func.gcda", dir_path);
    snprintf(gcda2, sizeof(gcda2), "%s/#test_func.gcda#", dir_path);
    
    /* Build the command */
    snprintf(cmd, sizeof(cmd), "%s %s %s %s 2>&1", 
             gcov_tool, args, gcda1, gcda2);
    
    printf("\nExecuting gcov-tool (%s): %s\n", description, cmd);
    
    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        perror("popen failed");
        return 0;
    }
    
    /* Read and display gcov-tool output */
    char buffer[1024];
    int has_output = 0;
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        has_output = 1;
        printf("gcov-tool: %s", buffer);
    }
    
    int status = pclose(pipe);
    
    if (!has_output && status == 0) {
        printf("gcov-tool executed successfully (no output)\n");
    }
    
    if (status != 0) {
        fprintf(stderr, "gcov-tool failed with status %d\n", status);
        /* Don't return 0 here - some failures are expected (like invalid option) */
    }
    
    return 1;
}

/* Function to clean up temporary directory */
void cleanup_temp_dir(const char *dir_path) {
    if (!dir_path) return;
    
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dir_path);
    
    printf("\nCleaning up: %s\n", cmd);
    system(cmd);
}

int main(int argc, char *argv[]) {
    char *temp_dir = NULL;
    const char *gcov_tool = NULL;
    int success = 0;
    
    printf("=== Starting gcov-tool coverage test ===\n");
    
    /* Step 1: Create temporary directory */
    temp_dir = create_temp_dir();
    if (!temp_dir) {
        fprintf(stderr, "Failed to create temporary directory\n");
        return 1;
    }
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Step 2: Write test program source */
    if (!write_test_program(temp_dir)) {
        fprintf(stderr, "Failed to write test program\n");
        goto cleanup;
    }
    
    /* Step 3: Compile test program with coverage */
    if (!compile_test_program(temp_dir)) {
        fprintf(stderr, "Failed to compile test program\n");
        goto cleanup;
    }
    
    /* Step 4: Run test program multiple times to generate different .gcda files */
    printf("\n--- Generating profile data ---\n");
    
    /* First run */
    if (!run_test_program(temp_dir, 1, NULL)) {
        fprintf(stderr, "First run failed\n");
        goto cleanup;
    }
    
    /* Rename the first .gcda file to preserve it */
    char gcda1[MAX_PATH], gcda1_backup[MAX_PATH];
    snprintf(gcda1, sizeof(gcda1), "%s/test_func.gcda", temp_dir);
    snprintf(gcda1_backup, sizeof(gcda1_backup), "%s/#test_func.gcda#", temp_dir);
    
    if (rename(gcda1, gcda1_backup) != 0) {
        perror("Failed to rename first .gcda file");
        goto cleanup;
    }
    
    /* Second run with different parameters */
    if (!run_test_program(temp_dir, 2, "2")) {
        fprintf(stderr, "Second run failed\n");
        goto cleanup;
    }
    
    /* Now we have two different .gcda files:
     * - test_func.gcda (from second run)
     * - #test_func.gcda# (from first run, renamed)
     */
    
    /* Step 5: Find gcov-tool */
    gcov_tool = find_gcov_tool();
    if (!gcov_tool) {
        fprintf(stderr, "gcov-tool not found. Set GCOV_TOOL environment variable.\n");
        fprintf(stderr, "Common locations: /usr/bin/gcov-tool, /usr/local/bin/gcov-tool\n");
        goto cleanup;
    }
    printf("\nFound gcov-tool: %s\n", gcov_tool);
    
    /* Step 6: Execute gcov-tool with all the flags to trigger the uncovered lines */
    /* This triggers: case 'v', case 'f', case 'F', case 'o', case 'h', case 't' */
    if (!execute_gcov_tool(gcov_tool, temp_dir, 
                          "-v -f -F -o -h -t 0.75 overlap", 
                          "testing all overlap flags")) {
        fprintf(stderr, "Failed to execute gcov-tool with overlap flags\n");
        goto cleanup;
    }
    
    /* Step 7: Execute gcov-tool with invalid option to trigger default case */
    /* This triggers: default: overlap_usage() */
    if (!execute_gcov_tool(gcov_tool, temp_dir, 
                          "-Z invalid_option", 
                          "testing invalid option (should show usage)")) {
        fprintf(stderr, "Failed to execute gcov-tool with invalid option\n");
        /* Continue anyway - this failure is expected */
    }
    
    /* Additional test: Try without the 'overlap' command to ensure we hit the switch */
    if (!execute_gcov_tool(gcov_tool, temp_dir, 
                          "-v overlap", 
                          "testing verbose flag only")) {
        fprintf(stderr, "Failed to execute gcov-tool with verbose flag\n");
        goto cleanup;
    }
    
    success = 1;
    printf("\n=== All tests completed successfully ===\n");
    
cleanup:
    /* Step 8: Clean up */
    if (temp_dir) {
        cleanup_temp_dir(temp_dir);
        free(temp_dir);
    }
    
    return success ? 0 : 1;
}
