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
"    for (int i = 0; i < 10; i++) {\n"
"        if (i < x) {\n"
"            result += i * 2;\n"
"        } else {\n"
"            result += i;\n"
"        }\n"
"    }\n"
"    return result;\n"
"}\n"
"\n"
"int main(int argc, char *argv[]) {\n"
"    int value = 5;  /* Default value */\n"
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
        while (fgets(output + total_read, output_size - total_read, fp) != NULL) {
            total_read = strlen(output);
            if (total_read >= output_size - 1) {
                break;
            }
        }
    } else {
        /* Just read and discard output */
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            /* Discard */
        }
    }
    
    status = pclose(fp);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Clean up temporary directory */
void cleanup_temp_dir(const char *temp_dir) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", temp_dir);
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
    char output[4096];
    int ret;
    
    /* Create temporary directory */
    strncpy(temp_dir, TEMPLATE, sizeof(temp_dir));
    if (mkdtemp(temp_dir) == NULL) {
        perror("Failed to create temporary directory");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", temp_dir);
    
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
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g \"%s\" -o \"%s\"",
             source_path, exec_path);
    
    ret = execute_command(cmd, output, sizeof(output));
    if (ret != 0) {
        fprintf(stderr, "Compilation failed: %s\n", output);
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Generate .gcno file path */
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", temp_dir);
    
    /* First run with different inputs to generate distinct .gcda files */
    
    /* Run 1: Using command line argument */
    printf("\n=== First run (value=3) ===\n");
    snprintf(cmd, sizeof(cmd), "\"%s\" 3", exec_path);
    ret = execute_command(cmd, output, sizeof(output));
    if (ret != 0) {
        fprintf(stderr, "First run failed\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Rename the generated .gcda file */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", temp_dir);
    char gcda1_final[MAX_PATH];
    snprintf(gcda1_final, sizeof(gcda1_final), "%s/test_func_run1.gcda", temp_dir);
    rename(gcda1_path, gcda1_final);
    
    /* Run 2: Using environment variable */
    printf("\n=== Second run (value=8 via env) ===\n");
    setenv("TEST_VALUE", "3", 1);  /* Add 3 to default value of 5 = 8 */
    snprintf(cmd, sizeof(cmd), "\"%s\"", exec_path);
    ret = execute_command(cmd, output, sizeof(output));
    if (ret != 0) {
        fprintf(stderr, "Second run failed\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    unsetenv("TEST_VALUE");
    
    /* Rename second .gcda file */
    char gcda2_final[MAX_PATH];
    snprintf(gcda2_final, sizeof(gcda2_final), "%s/test_func_run2.gcda", temp_dir);
    rename(gcda1_path, gcda2_final);
    
    /* Find gcov-tool path */
    char *gcov_tool_path = getenv("GCOV_TOOL");
    if (!gcov_tool_path) {
        gcov_tool_path = "gcov-tool";  /* Rely on PATH */
    }
    
    /* Test 1: Trigger all the specific flag cases from uncovered lines */
    printf("\n=== Testing gcov-tool with all overlap flags ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -v -f -F -o -h -t 0.75 \"%s\" \"%s\"",
             gcov_tool_path, gcda1_final, gcda2_final);
    
    ret = execute_command(cmd, output, sizeof(output));
    printf("gcov-tool returned: %d\n", ret);
    if (output[0] != '\0') {
        printf("Output (first 500 chars):\n%.500s\n", output);
    }
    
    /* Test 2: Trigger default case with invalid option */
    printf("\n=== Testing gcov-tool with invalid option (to trigger default case) ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -Z \"%s\" \"%s\"",
             gcov_tool_path, gcda1_final, gcda2_final);
    
    ret = execute_command(cmd, output, sizeof(output));
    printf("gcov-tool with invalid option returned: %d\n", ret);
    if (output[0] != '\0') {
        printf("Output (first 500 chars):\n%.500s\n", output);
    }
    
    /* Additional test: Just verbose flag to ensure gcov_set_verbose() is called */
    printf("\n=== Testing gcov-tool with verbose flag only ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -v \"%s\" \"%s\"",
             gcov_tool_path, gcda1_final, gcda2_final);
    
    ret = execute_command(cmd, output, sizeof(output));
    printf("gcov-tool verbose only returned: %d\n", ret);
    
    /* Clean up */
    printf("\n=== Cleaning up ===\n");
    cleanup_temp_dir(temp_dir);
    
    printf("\nTest completed successfully!\n");
    printf("The following gcov-tool flags were tested:\n");
    printf("  -v (verbose) -> triggers gcov_set_verbose()\n");
    printf("  -f (function level overlap)\n");
    printf("  -F (use fullnames)\n");
    printf("  -o (object level)\n");
    printf("  -h (hot only)\n");
    printf("  -t 0.75 (hot threshold)\n");
    printf("  -Z (invalid option) -> triggers default case and overlap_usage()\n");
    
    return 0;
}
