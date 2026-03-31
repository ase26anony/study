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

/* Simple test program source code that will be compiled with coverage */
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
"    int iterations = 10;  /* default */\n"
"    \n"
"    if (argc > 1) {\n"
"        iterations = atoi(argv[1]);\n"
"    }\n"
"    \n"
"    /* Environment variable can also affect behavior */\n"
"    char *env_iter = getenv(\"TEST_ITERATIONS\");\n"
"    if (env_iter != NULL) {\n"
"        iterations += atoi(env_iter);\n"
"    }\n"
"    \n"
"    int result = process_value(iterations);\n"
"    printf(\"Result: %d\\n\", result);\n"
"    return 0;\n"
"}\n";

/* Execute a command and return its exit status */
int execute_command(const char *cmd, int capture_output) {
    printf("Executing: %s\n", cmd);
    
    if (capture_output) {
        FILE *fp = popen(cmd, "r");
        if (fp == NULL) {
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

/* Clean up temporary files */
void cleanup(const char *temp_dir, const char *source_path, 
             const char *exec_path, const char **gcda_files, int gcda_count) {
    if (source_path && access(source_path, F_OK) == 0) {
        unlink(source_path);
    }
    
    if (exec_path && access(exec_path, F_OK) == 0) {
        unlink(exec_path);
    }
    
    /* Remove .gcno and .gcda files */
    char path[MAX_PATH];
    
    /* Remove .gcno file */
    snprintf(path, sizeof(path), "%s/test_program.gcno", temp_dir);
    if (access(path, F_OK) == 0) {
        unlink(path);
    }
    
    /* Remove .gcda files */
    for (int i = 0; i < gcda_count; i++) {
        if (gcda_files[i] && access(gcda_files[i], F_OK) == 0) {
            unlink(gcda_files[i]);
        }
    }
    
    /* Remove temp directory if empty */
    rmdir(temp_dir);
}

int main(int argc, char *argv[]) {
    char temp_dir[MAX_PATH];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    const char *gcda_files[2] = {NULL, NULL};
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
    snprintf(source_path, sizeof(source_path), "%s/test_program.c", temp_dir);
    FILE *src_fp = fopen(source_path, "w");
    if (src_fp == NULL) {
        perror("Failed to create source file");
        cleanup(temp_dir, NULL, NULL, gcda_files, 0);
        return 1;
    }
    fputs(test_program_source, src_fp);
    fclose(src_fp);
    
    /* Compile test program with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_program", temp_dir);
    char compile_cmd[MAX_CMD];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s",
             source_path, exec_path);
    
    if (execute_command(compile_cmd, 1) != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup(temp_dir, source_path, NULL, gcda_files, 0);
        return 1;
    }
    
    /* Run program twice with different inputs to generate distinct .gcda files */
    
    /* First run with 5 iterations */
    setenv("TEST_ITERATIONS", "0", 1);
    char run1_cmd[MAX_CMD];
    snprintf(run1_cmd, sizeof(run1_cmd), "%s 5", exec_path);
    execute_command(run1_cmd, 1);
    
    /* Rename first .gcda file */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_program.gcda", temp_dir);
    char gcda1_renamed[MAX_PATH];
    snprintf(gcda1_renamed, sizeof(gcda1_renamed), "%s/test_program_run1.gcda", temp_dir);
    rename(gcda1_path, gcda1_renamed);
    strcpy(gcda1_path, gcda1_renamed);
    gcda_files[0] = gcda1_path;
    
    /* Second run with 20 iterations */
    setenv("TEST_ITERATIONS", "5", 1);
    char run2_cmd[MAX_CMD];
    snprintf(run2_cmd, sizeof(run2_cmd), "%s 15", exec_path);
    execute_command(run2_cmd, 1);
    
    /* Rename second .gcda file */
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_program.gcda", temp_dir);
    char gcda2_renamed[MAX_PATH];
    snprintf(gcda2_renamed, sizeof(gcda2_renamed), "%s/test_program_run2.gcda", temp_dir);
    rename(gcda2_path, gcda2_renamed);
    strcpy(gcda2_path, gcda2_renamed);
    gcda_files[1] = gcda2_path;
    
    /* Find gcov-tool path */
    char gcov_tool_path[MAX_PATH] = "/usr/bin/gcov-tool";
    char *env_gcov_tool = getenv("GCOV_TOOL");
    if (env_gcov_tool != NULL && access(env_gcov_tool, X_OK) == 0) {
        strcpy(gcov_tool_path, env_gcov_tool);
    } else if (access(gcov_tool_path, X_OK) != 0) {
        /* Try to find it in PATH */
        FILE *fp = popen("which gcov-tool 2>/dev/null", "r");
        if (fp != NULL) {
            if (fgets(gcov_tool_path, sizeof(gcov_tool_path), fp) != NULL) {
                /* Remove newline */
                gcov_tool_path[strcspn(gcov_tool_path, "\n")] = '\0';
            }
            pclose(fp);
        }
    }
    
    printf("Using gcov-tool at: %s\n", gcov_tool_path);
    
    /* Test 1: Trigger all the case statements in the uncovered block */
    char gcov_cmd1[MAX_CMD];
    snprintf(gcov_cmd1, sizeof(gcov_cmd1),
             "%s overlap -v -f -F -o -h -t 0.75 %s %s",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    printf("\n=== Test 1: Triggering case statements ===\n");
    int result1 = execute_command(gcov_cmd1, 1);
    printf("gcov-tool overlap returned: %d\n", result1);
    
    /* Test 2: Trigger default case with invalid option */
    char gcov_cmd2[MAX_CMD];
    snprintf(gcov_cmd2, sizeof(gcov_cmd2),
             "%s overlap -Z %s %s",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    printf("\n=== Test 2: Triggering default case (invalid option) ===\n");
    int result2 = execute_command(gcov_cmd2, 1);
    printf("gcov-tool with invalid option returned: %d\n", result2);
    
    /* Also test with just the invalid option to ensure overlap_usage() is called */
    char gcov_cmd3[MAX_CMD];
    snprintf(gcov_cmd3, sizeof(gcov_cmd3), "%s overlap -Z", gcov_tool_path);
    
    printf("\n=== Test 3: Triggering default case (invalid option only) ===\n");
    int result3 = execute_command(gcov_cmd3, 1);
    printf("gcov-tool with invalid option only returned: %d\n", result3);
    
    /* Summary */
    printf("\n=== Summary ===\n");
    printf("Test 1 (case statements): %s\n", result1 == 0 ? "PASS" : "FAIL");
    printf("Test 2 (default case): %s\n", result2 != 0 ? "PASS (expected error)" : "CHECK");
    printf("Test 3 (default case only): %s\n", result3 != 0 ? "PASS (expected error)" : "CHECK");
    
    /* Cleanup */
    cleanup(temp_dir, source_path, exec_path, gcda_files, 2);
    
    printf("\nTemporary files cleaned up\n");
    
    return 0;
}
