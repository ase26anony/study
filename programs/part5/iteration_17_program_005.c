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

/* Execute a shell command and return exit status */
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

/* Clean up temporary files */
void cleanup(const char *temp_dir, const char *source_path, 
             const char *exec_path, const char *gcno_path) {
    if (source_path) unlink(source_path);
    if (exec_path) unlink(exec_path);
    if (gcno_path) unlink(gcno_path);
    
    /* Remove .gcda files */
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -f %s/*.gcda", temp_dir);
    system(cmd);
    
    /* Remove temp directory if empty */
    rmdir(temp_dir);
}

int main() {
    char temp_dir[MAX_PATH];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    char gcno_path[MAX_PATH];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    char cmd[MAX_CMD];
    int ret = 0;
    
    /* Create temporary directory */
    strcpy(temp_dir, TEMPLATE);
    if (!mkdtemp(temp_dir)) {
        perror("Failed to create temp directory");
        return 1;
    }
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Set environment to write .gcda files to temp directory */
    setenv("GCOV_PREFIX", temp_dir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Write test program source */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", temp_dir);
    FILE *src = fopen(source_path, "w");
    if (!src) {
        perror("Failed to create source file");
        cleanup(temp_dir, NULL, NULL, NULL);
        return 1;
    }
    fputs(test_program_source, src);
    fclose(src);
    
    /* Compile with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", temp_dir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s",
             source_path, exec_path);
    
    if (execute_command(cmd, 1) != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup(temp_dir, source_path, NULL, NULL);
        return 1;
    }
    
    /* Get .gcno file path (created during compilation) */
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", temp_dir);
    
    /* First run with different inputs to generate distinct .gcda files */
    
    /* Run 1: Using command line argument */
    printf("\n=== First run (value=5) ===\n");
    snprintf(cmd, sizeof(cmd), "%s 5", exec_path);
    execute_command(cmd, 1);
    
    /* Rename the generated .gcda file */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func_run1.gcda", temp_dir);
    snprintf(cmd, sizeof(cmd), "mv %s/test_func.gcda %s", temp_dir, gcda1_path);
    system(cmd);
    
    /* Run 2: Using environment variable */
    printf("\n=== Second run (value=15 via env) ===\n");
    setenv("TEST_VALUE", "5", 1);  /* Adds 5 to default value of 10 */
    snprintf(cmd, sizeof(cmd), "%s", exec_path);
    execute_command(cmd, 1);
    unsetenv("TEST_VALUE");
    
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func_run2.gcda", temp_dir);
    snprintf(cmd, sizeof(cmd), "mv %s/test_func.gcda %s", temp_dir, gcda2_path);
    system(cmd);
    
    /* Find gcov-tool path */
    char *gcov_tool_path = getenv("GCOV_TOOL");
    if (!gcov_tool_path) {
        gcov_tool_path = "gcov-tool";  /* Rely on PATH */
    }
    
    /* Test 1: Trigger all the case statements in the uncovered block */
    printf("\n=== Test 1: Triggering case statements (-v -f -F -o -h -t) ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -v -f -F -o -h -t 0.75 %s %s",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    int status1 = execute_command(cmd, 1);
    if (status1 != 0) {
        fprintf(stderr, "Warning: gcov-tool overlap returned %d\n", status1);
        /* Continue anyway - some versions might not support all options */
    }
    
    /* Test 2: Trigger default case with invalid option */
    printf("\n=== Test 2: Triggering default case (invalid option -Z) ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -Z %s %s",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    int status2 = execute_command(cmd, 1);
    printf("Invalid option test returned: %d (expected non-zero)\n", status2);
    
    /* Test 3: Additional test with just verbose flag */
    printf("\n=== Test 3: Testing verbose flag alone ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -v %s %s",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    execute_command(cmd, 1);
    
    /* Test 4: Test with threshold value */
    printf("\n=== Test 4: Testing threshold option ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -t 0.5 %s %s",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    execute_command(cmd, 1);
    
    printf("\n=== All tests completed ===\n");
    printf("Generated files in: %s\n", temp_dir);
    printf("You may want to examine the directory before it's cleaned up.\n");
    printf("Press Enter to clean up and exit...\n");
    getchar();
    
    /* Cleanup */
    cleanup(temp_dir, source_path, exec_path, gcno_path);
    
    return 0;
}
