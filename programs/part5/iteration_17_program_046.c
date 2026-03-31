#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_PATH 1024
#define MAX_CMD 4096

/* Simple test program source code that will be compiled with coverage */
const char *test_program_source = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"\n"
"int main(int argc, char *argv[]) {\n"
"    int i, limit = 10;\n"
"    int sum = 0;\n"
"    \n"
"    /* Different behavior based on environment variable or argument */\n"
"    char *env_limit = getenv(\"TEST_LIMIT\");\n"
"    if (env_limit != NULL) {\n"
"        limit = atoi(env_limit);\n"
"    } else if (argc > 1) {\n"
"        limit = atoi(argv[1]);\n"
"    }\n"
"    \n"
"    /* Loop with conditional to generate interesting coverage */\n"
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

/* Execute a shell command and return its exit status */
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
            /* Discard or log output as needed */
            /* For debugging: fputs(buffer, stderr); */
        }
        
        int status = pclose(fp);
        if (status == -1) {
            perror("pclose failed");
            return -1;
        }
        
        return WEXITSTATUS(status);
    } else {
        int status = system(cmd);
        if (status == -1) {
            perror("system failed");
            return -1;
        }
        return WEXITSTATUS(status);
    }
}

/* Clean up temporary directory */
void cleanup_temp_dir(const char *temp_dir) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
    system(cmd);
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
    
    /* Create unique temporary directory */
    snprintf(temp_dir, sizeof(temp_dir), "/tmp/gcov_test_%d", getpid());
    if (mkdir(temp_dir, 0755) == -1) {
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
    if (src_fp == NULL) {
        perror("Failed to create source file");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    fputs(test_program_source, src_fp);
    fclose(src_fp);
    
    /* Compile test program with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", temp_dir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s",
             source_path, exec_path);
    
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* The .gcno file should be in the same directory as source */
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", temp_dir);
    
    /* First run with different limits to generate distinct .gcda files */
    
    /* Run 1: limit = 5 */
    unsetenv("TEST_LIMIT");  /* Clear any existing */
    setenv("TEST_LIMIT", "5", 1);
    snprintf(cmd, sizeof(cmd), "%s", exec_path);
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "First run failed\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Rename the .gcda file to preserve it */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func_1.gcda", temp_dir);
    snprintf(cmd, sizeof(cmd), "mv %s/test_func.gcda %s", temp_dir, gcda1_path);
    execute_command(cmd, 0);
    
    /* Run 2: limit = 15 */
    setenv("TEST_LIMIT", "15", 1);
    snprintf(cmd, sizeof(cmd), "%s", exec_path);
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "Second run failed\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Rename second .gcda file */
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func_2.gcda", temp_dir);
    snprintf(cmd, sizeof(cmd), "mv %s/test_func.gcda %s", temp_dir, gcda2_path);
    execute_command(cmd, 0);
    
    /* Find gcov-tool path */
    char *gcov_tool_path = getenv("GCOV_TOOL");
    if (gcov_tool_path == NULL) {
        gcov_tool_path = "gcov-tool";  /* Rely on PATH */
    }
    
    /* Test 1: Trigger all the case statements in the uncovered block */
    printf("\n=== Test 1: Triggering case statements with valid options ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -v -f -F -o -h -t 0.75 %s %s",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "gcov-tool overlap analysis failed (this might be expected)\n");
    }
    
    /* Test 2: Trigger default case with invalid option */
    printf("\n=== Test 2: Triggering default case with invalid option ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -Z %s %s",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "gcov-tool with invalid option failed as expected\n");
    }
    
    /* Optional: Also test with just the invalid option to ensure overlap_usage() is called */
    printf("\n=== Test 3: Testing invalid option alone ===\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -Z", gcov_tool_path);
    status = execute_command(cmd, 1);
    
    printf("\n=== All tests completed ===\n");
    
    /* Clean up */
    cleanup_temp_dir(temp_dir);
    
    return 0;
}
