#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define TEMP_DIR_TEMPLATE "/tmp/gcov_test_XXXXXX"
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
"    /* Use environment variable to change behavior for different runs */\n"
"    char *env_limit = getenv(\"TEST_LIMIT\");\n"
"    if (env_limit) {\n"
"        limit = atoi(env_limit);\n"
"    }\n"
"    \n"
"    /* Different behavior based on command line argument */\n"
"    if (argc > 1) {\n"
"        limit = atoi(argv[1]);\n"
"    }\n"
"    \n"
"    /* Loop with conditional to generate interesting coverage data */\n"
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
            /* Do nothing, just consume output */
        }
    }
    
    status = pclose(fp);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Clean up temporary files */
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
    strcpy(temp_dir, TEMP_DIR_TEMPLATE);
    if (mkdtemp(temp_dir) == NULL) {
        perror("Failed to create temporary directory");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", temp_dir);
    
    /* Set environment to write .gcda files to our temp directory */
    setenv("GCOV_PREFIX", temp_dir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Create source file */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", temp_dir);
    FILE *src_fp = fopen(source_path, "w");
    if (!src_fp) {
        perror("Failed to create source file");
        cleanup(temp_dir);
        return 1;
    }
    fputs(test_program_source, src_fp);
    fclose(src_fp);
    
    /* Create executable path */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", temp_dir);
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", temp_dir);
    
    /* Compile with coverage instrumentation */
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g \"%s\" -o \"%s\"",
             source_path, exec_path);
    
    if (execute_command(cmd, output, sizeof(output)) != 0) {
        fprintf(stderr, "Compilation failed:\n%s\n", output);
        cleanup(temp_dir);
        return 1;
    }
    
    /* First run with different behavior */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", temp_dir);
    setenv("TEST_LIMIT", "5", 1);
    snprintf(cmd, sizeof(cmd), "\"%s\" 5", exec_path);
    execute_command(cmd, NULL, 0);
    
    /* Rename first .gcda file */
    char gcda1_renamed[MAX_PATH];
    snprintf(gcda1_renamed, sizeof(gcda1_renamed), "%s/test_func_run1.gcda", temp_dir);
    rename(gcda1_path, gcda1_renamed);
    
    /* Second run with different behavior */
    setenv("TEST_LIMIT", "15", 1);
    snprintf(cmd, sizeof(cmd), "\"%s\" 15", exec_path);
    execute_command(cmd, NULL, 0);
    
    /* Rename second .gcda file */
    char gcda2_renamed[MAX_PATH];
    snprintf(gcda2_renamed, sizeof(gcda2_renamed), "%s/test_func_run2.gcda", temp_dir);
    rename(gcda1_path, gcda2_renamed);
    
    /* Find gcov-tool path */
    const char *gcov_tool_path = getenv("GCOV_TOOL");
    if (!gcov_tool_path) {
        gcov_tool_path = "gcov-tool";  /* Rely on PATH */
    }
    
    /* Test 1: Trigger all the flag cases (lines 534-554) */
    printf("\n=== Test 1: Triggering flag handling code ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -v -f -F -o -h -t 0.75 \"%s\" \"%s\"",
             gcov_tool_path, gcda1_renamed, gcda2_renamed);
    
    int status1 = execute_command(cmd, output, sizeof(output));
    printf("gcov-tool output (flags test):\n%s\n", output);
    
    /* Test 2: Trigger default case with invalid option */
    printf("\n=== Test 2: Triggering default case (invalid option) ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -Z \"%s\" \"%s\"",
             gcov_tool_path, gcda1_renamed, gcda2_renamed);
    
    int status2 = execute_command(cmd, output, sizeof(output));
    printf("gcov-tool output (invalid option):\n%s\n", output);
    
    /* Check results */
    if (status1 == 0) {
        printf("\n✓ Successfully triggered flag handling code\n");
    } else {
        printf("\n✗ Flag handling test returned status: %d\n", status1);
        ret = 1;
    }
    
    if (status2 != 0) {
        printf("✓ Successfully triggered default case (expected failure)\n");
    } else {
        printf("✗ Invalid option test unexpectedly succeeded\n");
        ret = 1;
    }
    
    /* Clean up */
    printf("\nCleaning up temporary directory: %s\n", temp_dir);
    cleanup(temp_dir);
    
    return ret;
}
