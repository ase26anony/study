#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define TEMPLATE "/tmp/gcov_test_XXXXXX"
#define MAX_PATH 1024
#define MAX_CMD 4096

/* Simple test program that will be compiled with coverage instrumentation */
const char *test_program_source = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"\n"
"int main(int argc, char *argv[]) {\n"
"    int i, limit = 10;\n"
"    int sum = 0;\n"
"    \n"
"    /* Use environment variable or argument to change behavior */\n"
"    if (argc > 1) {\n"
"        limit = atoi(argv[1]);\n"
"    }\n"
"    \n"
"    /* Different behavior based on RUN_ID env var */\n"
"    char *run_id = getenv(\"RUN_ID\");\n"
"    if (run_id && run_id[0] == '2') {\n"
"        limit = 5;  /* Second run has different limit */\n"
"    }\n"
"    \n"
"    for (i = 0; i < limit; i++) {\n"
"        if (i % 2 == 0) {\n"
"            sum += i;\n"
"        } else {\n"
"            sum -= i;\n"
"        }\n"
"    }\n"
"    \n"
"    printf(\"Sum: %d\\n\", sum);\n"
"    return 0;\n"
"}\n";

/* Execute a shell command and return exit status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status == -1) {
        perror("system() failed");
        return -1;
    }
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Run gcov-tool with specified arguments */
int run_gcov_tool(const char *gcov_tool_path, const char *arg1, const char *arg2, 
                  const char *extra_args, int expect_failure) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "%s %s %s %s 2>&1", 
             gcov_tool_path, extra_args, arg1, arg2);
    
    printf("\n=== Running gcov-tool ===\n");
    printf("Command: %s\n", cmd);
    
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        return -1;
    }
    
    char buffer[256];
    printf("Output:\n");
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
    }
    
    int status = pclose(fp);
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        printf("Exit code: %d\n", exit_code);
        
        if (expect_failure) {
            /* For invalid option, we expect non-zero exit */
            return (exit_code != 0) ? 0 : -1;
        } else {
            return (exit_code == 0) ? 0 : -1;
        }
    }
    
    return -1;
}

int main(int argc, char *argv[]) {
    char temp_dir[MAX_PATH];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    char gcno_path[MAX_PATH];
    char cmd[MAX_CMD];
    int ret = 0;
    
    /* Create temporary directory */
    strcpy(temp_dir, TEMPLATE);
    if (mkdtemp(temp_dir) == NULL) {
        perror("Failed to create temp directory");
        return 1;
    }
    
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Set environment to write .gcda files to our temp directory */
    setenv("GCOV_PREFIX", temp_dir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Write test program source */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", temp_dir);
    FILE *src = fopen(source_path, "w");
    if (!src) {
        perror("Failed to create source file");
        ret = 1;
        goto cleanup;
    }
    fputs(test_program_source, src);
    fclose(src);
    
    /* Compile with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", temp_dir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s 2>&1",
             source_path, exec_path);
    
    printf("\n=== Compiling test program ===\n");
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Compilation failed\n");
        ret = 1;
        goto cleanup;
    }
    
    /* Get .gcno file path (created during compilation) */
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", temp_dir);
    
    /* Run program twice with different conditions to generate two .gcda files */
    
    /* First run */
    printf("\n=== First run ===\n");
    setenv("RUN_ID", "1", 1);
    snprintf(cmd, sizeof(cmd), "%s 10", exec_path);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "First run failed\n");
        ret = 1;
        goto cleanup;
    }
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", temp_dir);
    
    /* Rename first .gcda file */
    char gcda1_renamed[MAX_PATH];
    snprintf(gcda1_renamed, sizeof(gcda1_renamed), "%s/test_func_run1.gcda", temp_dir);
    rename(gcda1_path, gcda1_renamed);
    
    /* Second run with different behavior */
    printf("\n=== Second run ===\n");
    setenv("RUN_ID", "2", 1);
    snprintf(cmd, sizeof(cmd), "%s 5", exec_path);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Second run failed\n");
        ret = 1;
        goto cleanup;
    }
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func.gcda", temp_dir);
    
    /* Rename second .gcda file */
    char gcda2_renamed[MAX_PATH];
    snprintf(gcda2_renamed, sizeof(gcda2_renamed), "%s/test_func_run2.gcda", temp_dir);
    rename(gcda2_path, gcda2_renamed);
    
    /* Find gcov-tool path */
    char gcov_tool_path[MAX_PATH] = "/usr/bin/gcov-tool";
    char *env_gcov_tool = getenv("GCOV_TOOL");
    if (env_gcov_tool && strlen(env_gcov_tool) > 0) {
        strncpy(gcov_tool_path, env_gcov_tool, sizeof(gcov_tool_path) - 1);
    }
    
    /* Check if gcov-tool exists */
    if (access(gcov_tool_path, X_OK) != 0) {
        fprintf(stderr, "gcov-tool not found at %s\n", gcov_tool_path);
        fprintf(stderr, "Set GCOV_TOOL environment variable to correct path\n");
        ret = 1;
        goto cleanup;
    }
    
    /* Run gcov-tool with all the flags to trigger the uncovered switch cases */
    printf("\n=== Triggering overlap analysis with all flags ===\n");
    if (run_gcov_tool(gcov_tool_path, gcda1_renamed, gcda2_renamed,
                     "-v -f -F -o -h -t 0.75", 0) != 0) {
        fprintf(stderr, "gcov-tool overlap analysis failed\n");
        ret = 1;
        goto cleanup;
    }
    
    /* Run gcov-tool with invalid option to trigger default case and overlap_usage() */
    printf("\n=== Triggering default case with invalid option ===\n");
    if (run_gcov_tool(gcov_tool_path, gcda1_renamed, gcda2_renamed,
                     "-Z", 1) != 0) {
        fprintf(stderr, "Failed to trigger default case\n");
        ret = 1;
        goto cleanup;
    }
    
    printf("\n=== All tests completed successfully ===\n");
    
cleanup:
    /* Clean up temporary directory */
    printf("\n=== Cleaning up ===\n");
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
    execute_command(cmd);
    
    return ret;
}
