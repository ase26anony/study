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
"    if (env_val != NULL) {\n"
"        value = atoi(env_val);\n"
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
        if (fp == NULL) {
            perror("popen failed");
            return -1;
        }
        
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            /* Discard or log output as needed */
            printf("Output: %s", buffer);
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
void cleanup_tempdir(const char *tempdir) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", tempdir);
    system(cmd);
}

int main(int argc, char *argv[]) {
    char tempdir[MAX_PATH];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    char gcno_path[MAX_PATH];
    char cmd[MAX_CMD];
    int status;
    
    /* Create temporary directory */
    strcpy(tempdir, TEMPLATE);
    if (mkdtemp(tempdir) == NULL) {
        perror("Failed to create temporary directory");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", tempdir);
    
    /* Set environment to write .gcda files to our temp directory */
    setenv("GCOV_PREFIX", tempdir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Write test program source */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", tempdir);
    FILE *src = fopen(source_path, "w");
    if (src == NULL) {
        perror("Failed to create source file");
        cleanup_tempdir(tempdir);
        return 1;
    }
    fputs(test_program_source, src);
    fclose(src);
    
    /* Compile test program with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", tempdir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g \"%s\" -o \"%s\"",
             source_path, exec_path);
    
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup_tempdir(tempdir);
        return 1;
    }
    
    /* Find the .gcno file (it will be in the temp directory due to GCOV_PREFIX) */
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", tempdir);
    
    /* First run - produce first .gcda file */
    printf("\n=== First run ===\n");
    snprintf(cmd, sizeof(cmd), "\"%s\" 5", exec_path);
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "First run failed\n");
        cleanup_tempdir(tempdir);
        return 1;
    }
    
    /* Rename the .gcda file to preserve it */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func_run1.gcda", tempdir);
    snprintf(cmd, sizeof(cmd), "mv \"%s/test_func.gcda\" \"%s\"", tempdir, gcda1_path);
    system(cmd);
    
    /* Second run - produce second .gcda file with different execution counts */
    printf("\n=== Second run ===\n");
    setenv("TEST_VALUE", "15", 1);
    snprintf(cmd, sizeof(cmd), "\"%s\"", exec_path);
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "Second run failed\n");
        cleanup_tempdir(tempdir);
        return 1;
    }
    
    /* Rename second .gcda file */
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func_run2.gcda", tempdir);
    snprintf(cmd, sizeof(cmd), "mv \"%s/test_func.gcda\" \"%s\"", tempdir, gcda2_path);
    system(cmd);
    
    /* Find gcov-tool path */
    char *gcov_tool_path = getenv("GCOV_TOOL");
    if (gcov_tool_path == NULL) {
        gcov_tool_path = "gcov-tool";  /* Rely on PATH */
    }
    
    /* Test 1: Trigger all the flag cases from the uncovered block */
    printf("\n=== Test 1: Triggering flag cases (-v, -f, -F, -o, -h, -t) ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -v -f -F -o -h -t 0.75 \"%s\" \"%s\"",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    status = execute_command(cmd, 1);
    if (status != 0) {
        printf("Note: gcov-tool returned non-zero status %d (may be expected)\n", status);
    }
    
    /* Test 2: Trigger default case with invalid option */
    printf("\n=== Test 2: Triggering default case with invalid option (-Z) ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -Z \"%s\" \"%s\"",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    status = execute_command(cmd, 1);
    if (status != 0) {
        printf("Note: gcov-tool returned non-zero status %d (expected for invalid option)\n", status);
    }
    
    /* Test 3: Also test with just -v flag to ensure verbose path is hit */
    printf("\n=== Test 3: Testing verbose flag alone ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -v \"%s\" \"%s\"",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    status = execute_command(cmd, 1);
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    cleanup_tempdir(tempdir);
    
    printf("\nTest program completed successfully!\n");
    printf("The following gcov-tool code paths should have been triggered:\n");
    printf("1. case 'v': verbose = true; gcov_set_verbose();\n");
    printf("2. case 'f': overlap_func_level = 1;\n");
    printf("3. case 'F': overlap_use_fullname = 1;\n");
    printf("4. case 'o': overlap_obj_level = 1;\n");
    printf("5. case 'h': overlap_hot_only = 1;\n");
    printf("6. case 't': overlap_hot_threshold = atof(optarg);\n");
    printf("7. default: overlap_usage();\n");
    
    return 0;
}
