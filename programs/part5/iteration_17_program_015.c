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

/* Simple test program source code that will be compiled with coverage */
const char *test_program_source = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"\n"
"int process_value(int x) {\n"
"    int result = 0;\n"
"    for (int i = 0; i < x; i++) {\n"
"        if (i % 2 == 0) {\n"
"            result += i;\n"
"        } else {\n"
"            result -= i;\n"
"        }\n"
"    }\n"
"    return result;\n"
"}\n"
"\n"
"int main(int argc, char *argv[]) {\n"
"    int value = 10;  /* default value */\n"
"    \n"
"    if (argc > 1) {\n"
"        value = atoi(argv[1]);\n"
"    }\n"
"    \n"
"    /* Environment variable can also affect execution */\n"
"    char *env_val = getenv(\"TEST_VALUE\");\n"
"    if (env_val) {\n"
"        value += atoi(env_val);\n"
"    }\n"
"    \n"
"    int result = process_value(value);\n"
"    printf(\"Result: %d\\n\", result);\n"
"    return 0;\n"
"}\n";

/* Function to execute a command and capture output */
int execute_command(const char *cmd, char *output, size_t output_size) {
    FILE *fp;
    int status;
    
    if (output && output_size > 0) {
        output[0] = '\0';
    }
    
    printf("Executing: %s\n", cmd);
    
    fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return -1;
    }
    
    if (output && output_size > 0) {
        size_t total_read = 0;
        while (!feof(fp) && total_read < output_size - 1) {
            size_t n = fread(output + total_read, 1, output_size - total_read - 1, fp);
            total_read += n;
        }
        output[total_read] = '\0';
    } else {
        /* Just drain the output */
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            /* Discard output */
        }
    }
    
    status = pclose(fp);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Clean up temporary directory */
void cleanup(const char *temp_dir) {
    if (temp_dir) {
        char cmd[MAX_CMD];
        snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", temp_dir);
        system(cmd);
    }
}

int main(int argc, char *argv[]) {
    char temp_dir[MAX_PATH];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    char gcno_path[MAX_PATH];
    char cmd[MAX_CMD];
    char output[4096];
    int ret = 0;
    
    /* Create temporary directory */
    strncpy(temp_dir, TEMPLATE, sizeof(temp_dir));
    if (mkdtemp(temp_dir) == NULL) {
        perror("Failed to create temporary directory");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", temp_dir);
    
    /* Set up paths */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", temp_dir);
    snprintf(exec_path, sizeof(exec_path), "%s/test_program", temp_dir);
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", temp_dir);
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", temp_dir);
    
    /* Write test program source */
    FILE *src_file = fopen(source_path, "w");
    if (!src_file) {
        perror("Failed to create source file");
        cleanup(temp_dir);
        return 1;
    }
    fputs(test_program_source, src_file);
    fclose(src_file);
    
    /* Compile with coverage instrumentation */
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g \"%s\" -o \"%s\"",
             source_path, exec_path);
    
    if (execute_command(cmd, output, sizeof(output)) != 0) {
        fprintf(stderr, "Compilation failed:\n%s\n", output);
        cleanup(temp_dir);
        return 1;
    }
    
    /* Ensure the .gcno file exists */
    if (access(gcno_path, F_OK) != 0) {
        fprintf(stderr, ".gcno file not created: %s\n", gcno_path);
        cleanup(temp_dir);
        return 1;
    }
    
    /* First run - generate first .gcda file */
    printf("\n=== First run ===\n");
    snprintf(cmd, sizeof(cmd), "\"%s\" 5", exec_path);
    if (execute_command(cmd, output, sizeof(output)) != 0) {
        fprintf(stderr, "First run failed\n");
    }
    
    /* Rename first .gcda file */
    char gcda1_renamed[MAX_PATH];
    snprintf(gcda1_renamed, sizeof(gcda1_renamed), "%s/test_func_run1.gcda", temp_dir);
    rename(gcda1_path, gcda1_renamed);
    
    /* Second run - with different input to generate different coverage */
    printf("\n=== Second run ===\n");
    snprintf(cmd, sizeof(cmd), "TEST_VALUE=7 \"%s\" 3", exec_path);
    if (execute_command(cmd, output, sizeof(output)) != 0) {
        fprintf(stderr, "Second run failed\n");
    }
    
    /* Rename second .gcda file */
    char gcda2_renamed[MAX_PATH];
    snprintf(gcda2_renamed, sizeof(gcda2_renamed), "%s/test_func_run2.gcda", temp_dir);
    rename(gcda1_path, gcda2_renamed);
    
    /* Find gcov-tool path */
    char gcov_tool_path[MAX_PATH] = "/usr/bin/gcov-tool";
    char *env_gcov_tool = getenv("GCOV_TOOL");
    if (env_gcov_tool && access(env_gcov_tool, X_OK) == 0) {
        strncpy(gcov_tool_path, env_gcov_tool, sizeof(gcov_tool_path));
    } else if (access(gcov_tool_path, X_OK) != 0) {
        /* Try to find it in PATH */
        FILE *which = popen("which gcov-tool 2>/dev/null", "r");
        if (which) {
            if (fgets(gcov_tool_path, sizeof(gcov_tool_path), which)) {
                /* Remove newline */
                size_t len = strlen(gcov_tool_path);
                if (len > 0 && gcov_tool_path[len-1] == '\n') {
                    gcov_tool_path[len-1] = '\0';
                }
            }
            pclose(which);
        }
    }
    
    printf("\nUsing gcov-tool at: %s\n", gcov_tool_path);
    
    /* Test 1: Trigger all the flag cases from uncovered lines */
    printf("\n=== Test 1: Triggering flag cases (-v, -f, -F, -o, -h, -t) ===\n");
    snprintf(cmd, sizeof(cmd),
             "\"%s\" overlap -v -f -F -o -h -t 0.75 \"%s\" \"%s\"",
             gcov_tool_path, gcda1_renamed, gcda2_renamed);
    
    int status1 = execute_command(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status1);
    if (output[0] != '\0') {
        printf("Output:\n%s\n", output);
    }
    
    /* Test 2: Trigger default case with invalid option */
    printf("\n=== Test 2: Triggering default case with invalid option (-Z) ===\n");
    snprintf(cmd, sizeof(cmd),
             "\"%s\" overlap -Z \"%s\" \"%s\" 2>&1",
             gcov_tool_path, gcda1_renamed, gcda2_renamed);
    
    int status2 = execute_command(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status2);
    if (output[0] != '\0') {
        printf("Output (should show usage):\n%s\n", output);
    }
    
    /* Additional test: Test with just verbose flag */
    printf("\n=== Test 3: Testing verbose flag only ===\n");
    snprintf(cmd, sizeof(cmd),
             "\"%s\" overlap -v \"%s\" \"%s\"",
             gcov_tool_path, gcda1_renamed, gcda2_renamed);
    
    int status3 = execute_command(cmd, NULL, 0);
    printf("Exit status: %d\n", status3);
    
    /* Check if we successfully triggered the code paths */
    printf("\n=== Summary ===\n");
    printf("Test 1 (all flags): %s\n", status1 == 0 ? "PASS" : "FAIL");
    printf("Test 2 (invalid option): %s\n", status2 != 0 ? "PASS (expected failure)" : "CHECK");
    printf("Test 3 (verbose only): %s\n", status3 == 0 ? "PASS" : "FAIL");
    
    if (status1 == 0 && status3 == 0) {
        printf("\nSuccessfully triggered the uncovered flag handling code!\n");
        ret = 0;
    } else {
        printf("\nSome tests failed. Check gcov-tool availability and permissions.\n");
        ret = 1;
    }
    
    /* Cleanup */
    printf("\nCleaning up temporary directory: %s\n", temp_dir);
    cleanup(temp_dir);
    
    return ret;
}
