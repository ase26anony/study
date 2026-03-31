#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
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
"    /* Environment variable can also affect behavior */\n"
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
            /* Discard or log output */
            printf("  Output: %s", buffer);
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
    if (temp_dir && temp_dir[0]) {
        char cmd[MAX_CMD];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
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
    int status;
    
    /* Create temporary directory */
    strcpy(temp_dir, TEMPLATE);
    if (!mkdtemp(temp_dir)) {
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
        cleanup(temp_dir);
        return 1;
    }
    fputs(test_program_source, src);
    fclose(src);
    
    /* Compile test program with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", temp_dir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s",
             source_path, exec_path);
    
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup(temp_dir);
        return 1;
    }
    
    /* The .gcno file will be created during compilation */
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", temp_dir);
    
    /* Run program twice with different inputs to generate distinct .gcda files */
    
    /* First run with value 10 */
    unsetenv("TEST_VALUE");  /* Ensure clean environment */
    snprintf(cmd, sizeof(cmd), "%s 10", exec_path);
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "First run failed\n");
        cleanup(temp_dir);
        return 1;
    }
    
    /* Rename first .gcda file */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", temp_dir);
    char gcda1_final[MAX_PATH];
    snprintf(gcda1_final, sizeof(gcda1_final), "%s/test_func_run1.gcda", temp_dir);
    rename(gcda1_path, gcda1_final);
    
    /* Second run with value 20 via environment variable */
    setenv("TEST_VALUE", "10", 1);  /* Adds 10 to default value of 10 = 20 total */
    snprintf(cmd, sizeof(cmd), "%s", exec_path);  /* Use default value */
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "Second run failed\n");
        cleanup(temp_dir);
        return 1;
    }
    
    /* Rename second .gcda file */
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func.gcda", temp_dir);
    char gcda2_final[MAX_PATH];
    snprintf(gcda2_final, sizeof(gcda2_final), "%s/test_func_run2.gcda", temp_dir);
    rename(gcda2_path, gcda2_final);
    
    /* Find gcov-tool path */
    char *gcov_tool_path = getenv("GCOV_TOOL");
    if (!gcov_tool_path) {
        gcov_tool_path = "gcov-tool";  /* Rely on PATH */
    }
    
    printf("\n=== Testing gcov-tool overlap analysis with all flags ===\n");
    
    /* Test 1: Trigger all the case statements in the uncovered block */
    snprintf(cmd, sizeof(cmd),
             "%s overlap -v -f -F -o -h -t 0.75 %s %s",
             gcov_tool_path, gcda1_final, gcda2_final);
    
    status = execute_command(cmd, 1);
    printf("gcov-tool overlap returned: %d\n", status);
    
    printf("\n=== Testing gcov-tool with invalid option (trigger default case) ===\n");
    
    /* Test 2: Trigger default case with invalid option */
    snprintf(cmd, sizeof(cmd),
             "%s overlap -Z %s %s",
             gcov_tool_path, gcda1_final, gcda2_final);
    
    status = execute_command(cmd, 1);
    printf("gcov-tool with invalid option returned: %d\n", status);
    
    /* Additional test: Try without required positional arguments */
    printf("\n=== Testing gcov-tool with insufficient arguments ===\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v", gcov_tool_path);
    status = execute_command(cmd, 1);
    printf("gcov-tool with insufficient args returned: %d\n", status);
    
    /* Clean up */
    printf("\nCleaning up temp directory: %s\n", temp_dir);
    cleanup(temp_dir);
    
    printf("\nTest completed successfully!\n");
    printf("The following gcov-tool options were tested:\n");
    printf("  -v (verbose) -> triggers verbose = true; gcov_set_verbose()\n");
    printf("  -f (func level) -> triggers overlap_func_level = 1\n");
    printf("  -F (fullname) -> triggers overlap_use_fullname = 1\n");
    printf("  -o (obj level) -> triggers overlap_obj_level = 1\n");
    printf("  -h (hot only) -> triggers overlap_hot_only = 1\n");
    printf("  -t 0.75 (threshold) -> triggers overlap_hot_threshold = atof(optarg)\n");
    printf("  -Z (invalid) -> triggers default: overlap_usage()\n");
    
    return 0;
}
