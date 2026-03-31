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
"int main(int argc, char *argv[]) {\n"
"    int i, limit = 10;\n"
"    int sum = 0;\n"
"    \n"
"    /* Use environment variable to control execution path */\n"
"    char *env_limit = getenv(\"TEST_LIMIT\");\n"
"    if (env_limit) {\n"
"        limit = atoi(env_limit);\n"
"    }\n"
"    \n"
"    /* Different execution based on command line argument */\n"
"    if (argc > 1) {\n"
"        limit = atoi(argv[1]);\n"
"    }\n"
"    \n"
"    for (i = 0; i < limit; i++) {\n"
"        if (i % 2 == 0) {\n"
"            sum += i;  /* Even path */\n"
"        } else {\n"
"            sum -= i;  /* Odd path */\n"
"        }\n"
"    }\n"
"    \n"
"    printf(\"Final sum: %d\\n\", sum);\n"
"    return 0;\n"
"}\n";

/* Execute a shell command and return its exit status */
int execute_command(const char *cmd, int capture_output) {
    printf("Executing: %s\n", cmd);
    
    if (capture_output) {
        FILE *fp = popen(cmd, "r");
        if (!fp) {
            perror("popen failed");
            return -1;
        }
        
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            /* Discard or log output as needed */
            /* Uncomment to see output: printf("Output: %s", buffer); */
        }
        
        int status = pclose(fp);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    } else {
        return system(cmd);
    }
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
    char gcov_tool_path[MAX_PATH] = "/usr/bin/gcov-tool";
    char cmd[MAX_CMD];
    int ret = 0;
    
    /* Create temporary directory */
    strcpy(temp_dir, TEMP_DIR_TEMPLATE);
    if (!mkdtemp(temp_dir)) {
        perror("Failed to create temporary directory");
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
        cleanup(temp_dir);
        return 1;
    }
    fputs(test_program_source, src);
    fclose(src);
    
    /* Compile test program with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", temp_dir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g \"%s\" -o \"%s\"",
             source_path, exec_path);
    
    if (execute_command(cmd, 0) != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup(temp_dir);
        return 1;
    }
    
    /* First run with different limits to generate distinct profile data */
    printf("\n=== First run (limit=15) ===\n");
    setenv("TEST_LIMIT", "15", 1);
    snprintf(cmd, sizeof(cmd), "\"%s\" 15", exec_path);
    execute_command(cmd, 1);
    
    /* Rename first .gcda file */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", temp_dir);
    char gcda1_backup[MAX_PATH];
    snprintf(gcda1_backup, sizeof(gcda1_backup), "%s/test_func_run1.gcda", temp_dir);
    rename(gcda1_path, gcda1_backup);
    
    /* Second run with different parameters */
    printf("\n=== Second run (limit=5) ===\n");
    setenv("TEST_LIMIT", "5", 1);
    snprintf(cmd, sizeof(cmd), "\"%s\" 5", exec_path);
    execute_command(cmd, 1);
    
    /* Rename second .gcda file */
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func.gcda", temp_dir);
    char gcda2_backup[MAX_PATH];
    snprintf(gcda2_backup, sizeof(gcda2_backup), "%s/test_func_run2.gcda", temp_dir);
    rename(gcda2_path, gcda2_backup);
    
    /* Check if gcov-tool exists at alternative location */
    char *env_gcov_tool = getenv("GCOV_TOOL");
    if (env_gcov_tool) {
        strncpy(gcov_tool_path, env_gcov_tool, sizeof(gcov_tool_path) - 1);
    } else {
        /* Try to find gcov-tool in PATH */
        FILE *which = popen("which gcov-tool 2>/dev/null", "r");
        if (which) {
            if (fgets(gcov_tool_path, sizeof(gcov_tool_path), which)) {
                /* Remove newline */
                gcov_tool_path[strcspn(gcov_tool_path, "\n")] = 0;
            }
            pclose(which);
        }
    }
    
    printf("\n=== Testing gcov-tool with all overlap options ===\n");
    
    /* Test 1: Trigger all the case statements in the uncovered block */
    snprintf(cmd, sizeof(cmd),
             "\"%s\" overlap -v -f -F -o -h -t 0.75 \"%s\" \"%s\"",
             gcov_tool_path, gcda1_backup, gcda2_backup);
    
    int status1 = execute_command(cmd, 1);
    printf("gcov-tool overlap returned: %d\n", status1);
    
    /* Test 2: Trigger the default case with invalid option */
    printf("\n=== Testing gcov-tool with invalid option (to trigger default case) ===\n");
    snprintf(cmd, sizeof(cmd),
             "\"%s\" overlap -Z \"%s\" \"%s\"",
             gcov_tool_path, gcda1_backup, gcda2_backup);
    
    int status2 = execute_command(cmd, 1);
    printf("gcov-tool with invalid option returned: %d\n", status2);
    
    /* Additional test with just verbose flag */
    printf("\n=== Testing gcov-tool with verbose flag only ===\n");
    snprintf(cmd, sizeof(cmd),
             "\"%s\" overlap -v \"%s\" \"%s\"",
             gcov_tool_path, gcda1_backup, gcda2_backup);
    
    int status3 = execute_command(cmd, 1);
    printf("gcov-tool verbose only returned: %d\n", status3);
    
    /* Test with threshold value only */
    printf("\n=== Testing gcov-tool with threshold flag ===\n");
    snprintf(cmd, sizeof(cmd),
             "\"%s\" overlap -t 0.5 \"%s\" \"%s\"",
             gcov_tool_path, gcda1_backup, gcda2_backup);
    
    int status4 = execute_command(cmd, 1);
    printf("gcov-tool threshold only returned: %d\n", status4);
    
    /* Summary */
    printf("\n=== Summary ===\n");
    printf("All tests completed. Temporary files in: %s\n", temp_dir);
    
    /* Clean up */
    cleanup(temp_dir);
    
    return 0;
}
