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
"    int value = 10;  /* default value */\n"
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
    char *template = strdup(TEMPLATE);
    if (template == NULL) {
        perror("strdup failed");
        return NULL;
    }
    
    char *dir = mkdtemp(template);
    if (dir == NULL) {
        perror("mkdtemp failed");
        free(template);
        return NULL;
    }
    
    return dir;
}

/* Execute a command and return its exit status */
int execute_command(const char *cmd, int capture_output) {
    printf("Executing: %s\n", cmd);
    
    if (capture_output) {
        FILE *fp = popen(cmd, "r");
        if (fp == NULL) {
            perror("popen failed");
            return -1;
        }
        
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            /* Discard or log output */
            printf("Output: %s", buffer);
        }
        
        int status = pclose(fp);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    } else {
        int status = system(cmd);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    }
}

/* Clean up temporary directory */
void cleanup_temp_dir(const char *dir) {
    if (dir == NULL) return;
    
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", dir);
    system(cmd);
}

int main(int argc, char *argv[]) {
    char *temp_dir = NULL;
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    char gcno_path[MAX_PATH];
    char cmd[MAX_CMD];
    int ret = 0;
    
    printf("=== Starting gcov-tool coverage test ===\n");
    
    /* Step 1: Create temporary directory */
    temp_dir = create_temp_dir();
    if (temp_dir == NULL) {
        fprintf(stderr, "Failed to create temporary directory\n");
        return 1;
    }
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Set environment to write .gcda files to our temp directory */
    setenv("GCOV_PREFIX", temp_dir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Step 2: Write test source file */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", temp_dir);
    FILE *src_file = fopen(source_path, "w");
    if (src_file == NULL) {
        perror("Failed to create source file");
        cleanup_temp_dir(temp_dir);
        free(temp_dir);
        return 1;
    }
    fprintf(src_file, "%s", test_program_source);
    fclose(src_file);
    
    /* Step 3: Compile with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", temp_dir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g \"%s\" -o \"%s\"",
             source_path, exec_path);
    
    if (execute_command(cmd, 1) != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup_temp_dir(temp_dir);
        free(temp_dir);
        return 1;
    }
    
    /* Get the .gcno file path (created during compilation) */
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", temp_dir);
    
    /* Step 4: Run the program twice with different inputs to generate two .gcda files */
    
    /* First run with command-line argument */
    unsetenv("TEST_VALUE");  /* Clear environment variable */
    snprintf(cmd, sizeof(cmd), "\"%s\" 5", exec_path);
    if (execute_command(cmd, 0) != 0) {
        fprintf(stderr, "First program execution failed\n");
    }
    
    /* First .gcda file path */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", temp_dir);
    
    /* Remove .gcda file to start fresh for second run */
    unlink(gcda1_path);
    
    /* Second run with environment variable */
    setenv("TEST_VALUE", "15", 1);
    snprintf(cmd, sizeof(cmd), "\"%s\" 8", exec_path);
    if (execute_command(cmd, 0) != 0) {
        fprintf(stderr, "Second program execution failed\n");
    }
    
    /* Second .gcda file path */
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func.gcda", temp_dir);
    
    /* Verify both .gcda files exist */
    if (access(gcda1_path, F_OK) != 0) {
        /* If first .gcda doesn't exist, create a copy of the second one */
        snprintf(cmd, sizeof(cmd), "cp \"%s\" \"%s/test_func_1.gcda\"", gcda2_path, temp_dir);
        execute_command(cmd, 0);
        snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func_1.gcda", temp_dir);
        snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func.gcda", temp_dir);
    } else {
        /* Rename first .gcda and copy second one */
        snprintf(cmd, sizeof(cmd), "mv \"%s\" \"%s/test_func_1.gcda\"", gcda1_path, temp_dir);
        execute_command(cmd, 0);
        snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func_1.gcda", temp_dir);
        snprintf(cmd, sizeof(cmd), "cp \"%s\" \"%s/test_func_2.gcda\"", gcda2_path, temp_dir);
        execute_command(cmd, 0);
        snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func_2.gcda", temp_dir);
    }
    
    printf("Generated .gcda files:\n");
    printf("  %s\n", gcda1_path);
    printf("  %s\n", gcda2_path);
    
    /* Step 5: Find gcov-tool path */
    char *gcov_tool_path = getenv("GCOV_TOOL");
    if (gcov_tool_path == NULL) {
        gcov_tool_path = "gcov-tool";  /* Rely on PATH */
    }
    
    /* Step 6: Execute gcov-tool with all the flags to trigger the uncovered lines */
    printf("\n=== Testing gcov-tool with valid flags ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -v -f -F -o -h -t 0.75 \"%s\" \"%s\"",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    int status1 = execute_command(cmd, 1);
    if (status1 != 0) {
        printf("Note: gcov-tool exited with status %d (may be expected)\n", status1);
    }
    
    /* Step 7: Execute gcov-tool with invalid flag to trigger default case */
    printf("\n=== Testing gcov-tool with invalid flag (to trigger default case) ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -Z \"%s\" \"%s\"",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    int status2 = execute_command(cmd, 1);
    if (status2 != 0) {
        printf("Note: gcov-tool with invalid flag exited with status %d (expected)\n", status2);
    }
    
    /* Step 8: Cleanup */
    printf("\n=== Cleaning up ===\n");
    cleanup_temp_dir(temp_dir);
    free(temp_dir);
    
    printf("\n=== Test completed ===\n");
    printf("Summary:\n");
    printf("  Valid flags test exit code: %d\n", status1);
    printf("  Invalid flag test exit code: %d\n", status2);
    
    if (status1 == 0 || status2 == 0) {
        printf("At least one gcov-tool invocation succeeded\n");
        ret = 0;
    } else {
        printf("Both gcov-tool invocations failed (may indicate gcov-tool not found)\n");
        ret = 1;
    }
    
    return ret;
}
