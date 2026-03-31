#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

#define TEMP_DIR_TEMPLATE "/tmp/gcov_test_XXXXXX"
#define MAX_PATH 1024
#define MAX_CMD 4096

/* Simple test program source code that will be compiled with coverage */
const char *test_program_source = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"\n"
"int main(int argc, char *argv[]) {\n"
"    int i, limit;\n"
"    \n"
"    /* Different runs will have different limits */\n"
"    if (argc > 1) {\n"
"        limit = atoi(argv[1]);\n"
"    } else {\n"
"        limit = 10;  /* Default */\n"
"    }\n"
"    \n"
"    /* Environment variable can also affect execution */\n"
"    char *env_limit = getenv(\"TEST_LIMIT\");\n"
"    if (env_limit) {\n"
"        limit += atoi(env_limit);\n"
"    }\n"
"    \n"
"    int sum = 0;\n"
"    for (i = 0; i < limit; i++) {\n"
"        if (i % 2 == 0) {\n"
"            sum += i;      /* Even path */\n"
"        } else {\n"
"            sum -= i;      /* Odd path */\n"
"        }\n"
"        \n"
"        /* Nested condition for more coverage complexity */\n"
"        if (i > limit / 2) {\n"
"            sum *= 2;\n"
"        }\n"
"    }\n"
"    \n"
"    printf(\"Result: %d\\n\", sum);\n"
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
        printf("Output:\n");
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("%s", buffer);
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
void cleanup_temp_dir(const char *temp_dir) {
    if (!temp_dir) return;
    
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", temp_dir);
    system(cmd);
}

/* Find gcov-tool path */
const char *find_gcov_tool() {
    const char *paths[] = {
        "/usr/bin/gcov-tool",
        "/usr/local/bin/gcov-tool",
        "/bin/gcov-tool",
        NULL
    };
    
    for (int i = 0; paths[i]; i++) {
        if (access(paths[i], X_OK) == 0) {
            return paths[i];
        }
    }
    
    /* Check PATH */
    char *path_env = getenv("PATH");
    if (path_env) {
        static char full_path[MAX_PATH];
        char *path_copy = strdup(path_env);
        char *dir = strtok(path_copy, ":");
        
        while (dir) {
            snprintf(full_path, sizeof(full_path), "%s/gcov-tool", dir);
            if (access(full_path, X_OK) == 0) {
                free(path_copy);
                return full_path;
            }
            dir = strtok(NULL, ":");
        }
        free(path_copy);
    }
    
    return "/usr/bin/gcov-tool";  /* Default, will fail if not found */
}

int main() {
    char temp_dir[MAX_PATH];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    char gcno_path[MAX_PATH];
    int ret = 0;
    
    /* Create temporary directory */
    strcpy(temp_dir, TEMP_DIR_TEMPLATE);
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
    FILE *src_fp = fopen(source_path, "w");
    if (!src_fp) {
        perror("Failed to create source file");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    fputs(test_program_source, src_fp);
    fclose(src_fp);
    
    /* Compile with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", temp_dir);
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", temp_dir);
    
    char compile_cmd[MAX_CMD];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -O0 -g \"%s\" -o \"%s\"",
             source_path, exec_path);
    
    if (execute_command(compile_cmd, 1) != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Run program twice with different parameters to generate different .gcda files */
    
    /* First run with limit=5 */
    setenv("TEST_LIMIT", "0", 1);  /* No extra from env */
    char run_cmd1[MAX_CMD];
    snprintf(run_cmd1, sizeof(run_cmd1), "\"%s\" 5", exec_path);
    execute_command(run_cmd1, 1);
    
    /* Rename the generated .gcda file */
    char gcda_temp[MAX_PATH];
    snprintf(gcda_temp, sizeof(gcda_temp), "%s/test_func.gcda", temp_dir);
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func_run1.gcda", temp_dir);
    rename(gcda_temp, gcda1_path);
    
    /* Second run with limit=15 and environment variable */
    setenv("TEST_LIMIT", "5", 1);  /* Add 5 from env */
    char run_cmd2[MAX_CMD];
    snprintf(run_cmd2, sizeof(run_cmd2), "\"%s\" 10", exec_path);  /* 10 + 5 from env = 15 */
    execute_command(run_cmd2, 1);
    
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func_run2.gcda", temp_dir);
    rename(gcda_temp, gcda2_path);
    
    /* Find gcov-tool */
    const char *gcov_tool = find_gcov_tool();
    printf("Using gcov-tool: %s\n", gcov_tool);
    
    /* Test 1: Trigger all the flag cases from uncovered lines */
    printf("\n=== Test 1: Triggering flag cases (lines 534-554) ===\n");
    
    char gcov_cmd[MAX_CMD];
    snprintf(gcov_cmd, sizeof(gcov_cmd),
             "%s overlap -v -f -F -o -h -t 0.75 \"%s\" \"%s\"",
             gcov_tool, gcda1_path, gcda2_path);
    
    int status1 = execute_command(gcov_cmd, 1);
    if (status1 != 0) {
        fprintf(stderr, "Warning: gcov-tool overlap returned %d\n", status1);
        /* Continue anyway - some versions might not support all flags */
    }
    
    /* Test 2: Trigger default case with invalid option */
    printf("\n=== Test 2: Triggering default case (overlap_usage) ===\n");
    
    snprintf(gcov_cmd, sizeof(gcov_cmd),
             "%s overlap -Z \"%s\" \"%s\"",
             gcov_tool, gcda1_path, gcda2_path);
    
    int status2 = execute_command(gcov_cmd, 1);
    if (status2 != 0) {
        printf("Expected error for invalid option -Z (triggered overlap_usage)\n");
    }
    
    /* Test 3: Additional test with just verbose flag */
    printf("\n=== Test 3: Testing verbose flag only ===\n");
    
    snprintf(gcov_cmd, sizeof(gcov_cmd),
             "%s overlap -v \"%s\" \"%s\"",
             gcov_tool, gcda1_path, gcda2_path);
    
    execute_command(gcov_cmd, 1);
    
    /* Test 4: Test with threshold only */
    printf("\n=== Test 4: Testing threshold flag only ===\n");
    
    snprintf(gcov_cmd, sizeof(gcov_cmd),
             "%s overlap -t 0.5 \"%s\" \"%s\"",
             gcov_tool, gcda1_path, gcda2_path);
    
    execute_command(gcov_cmd, 1);
    
    /* Cleanup */
    printf("\nCleaning up temp directory: %s\n", temp_dir);
    cleanup_temp_dir(temp_dir);
    
    if (status1 == 0 || status2 == 0) {
        printf("\nSuccessfully triggered target code paths in gcov-tool!\n");
        return 0;
    } else {
        printf("\nSome gcov-tool calls failed. Check if gcov-tool is installed.\n");
        return 1;
    }
}
