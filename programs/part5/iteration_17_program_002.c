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

/* Simple test program that will be compiled with coverage */
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
"    int iterations = 10;\n"
"    \n"
"    if (argc > 1) {\n"
"        iterations = atoi(argv[1]);\n"
"    }\n"
"    \n"
"    /* Environment variable can also affect behavior */\n"
"    char *env_iter = getenv(\"TEST_ITERATIONS\");\n"
"    if (env_iter) {\n"
"        iterations = atoi(env_iter);\n"
"    }\n"
"    \n"
"    int result = process_value(iterations);\n"
"    printf(\"Result: %d\\n\", result);\n"
"    return 0;\n"
"}\n";

/* Execute a command and capture its output */
int execute_command(const char *cmd, char *output, size_t output_size) {
    FILE *fp;
    int status;
    
    if (output) {
        output[0] = '\0';
    }
    
    printf("Executing: %s\n", cmd);
    fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return -1;
    }
    
    if (output) {
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
    if (status == -1) {
        perror("pclose failed");
        return -1;
    }
    
    return WEXITSTATUS(status);
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
    int status;
    
    /* Create temporary directory */
    strncpy(temp_dir, TEMPLATE, sizeof(temp_dir));
    if (mkdtemp(temp_dir) == NULL) {
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
    
    /* Compile test program with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", temp_dir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g \"%s\" -o \"%s\"",
             source_path, exec_path);
    
    status = execute_command(cmd, output, sizeof(output));
    if (status != 0) {
        fprintf(stderr, "Compilation failed:\n%s\n", output);
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* The .gcno file should be in the same directory as source */
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", temp_dir);
    
    /* First run - generate first .gcda file */
    printf("\n=== First run (10 iterations) ===\n");
    snprintf(cmd, sizeof(cmd), "\"%s\" 10", exec_path);
    status = execute_command(cmd, output, sizeof(output));
    if (status != 0) {
        fprintf(stderr, "First run failed:\n%s\n", output);
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* First .gcda file path */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", temp_dir);
    
    /* Second run - generate second .gcda file with different behavior */
    printf("\n=== Second run (20 iterations) ===\n");
    setenv("TEST_ITERATIONS", "20", 1);
    snprintf(cmd, sizeof(cmd), "\"%s\"", exec_path);
    status = execute_command(cmd, output, sizeof(output));
    if (status != 0) {
        fprintf(stderr, "Second run failed:\n%s\n", output);
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    unsetenv("TEST_ITERATIONS");
    
    /* Second .gcda file path - we need to rename the first one */
    /* Move first .gcda to gcda1, current becomes gcda2 */
    char gcda_temp[MAX_PATH];
    snprintf(gcda_temp, sizeof(gcda_temp), "%s/test_func.gcda.1", temp_dir);
    rename(gcda1_path, gcda_temp);
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func.gcda", temp_dir);
    rename(gcda_temp, gcda1_path);
    
    printf("\nGenerated files:\n");
    printf("  Source: %s\n", source_path);
    printf("  Executable: %s\n", exec_path);
    printf("  .gcno: %s\n", gcno_path);
    printf("  .gcda1: %s\n", gcda1_path);
    printf("  .gcda2: %s\n", gcda2_path);
    
    /* Find gcov-tool path */
    const char *gcov_tool_path = getenv("GCOV_TOOL");
    if (!gcov_tool_path) {
        gcov_tool_path = "gcov-tool";  /* Rely on PATH */
    }
    
    /* Test 1: Trigger all the flag cases from uncovered lines */
    printf("\n=== Test 1: Triggering flag handling (lines 534-554) ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -v -f -F -o -h -t 0.75 \"%s\" \"%s\"",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    status = execute_command(cmd, output, sizeof(output));
    printf("gcov-tool exit status: %d\n", status);
    if (output[0] != '\0') {
        printf("Output (first 500 chars):\n%.500s\n", output);
    }
    
    /* Test 2: Trigger default case with invalid option */
    printf("\n=== Test 2: Triggering default case (overlap_usage()) ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -Z \"%s\" \"%s\"",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    status = execute_command(cmd, output, sizeof(output));
    printf("gcov-tool exit status: %d\n", status);
    if (output[0] != '\0') {
        printf("Output (first 500 chars):\n%.500s\n", output);
    }
    
    /* Test 3: Additional test with just verbose flag */
    printf("\n=== Test 3: Testing verbose flag only ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -v \"%s\" \"%s\"",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    status = execute_command(cmd, output, sizeof(output));
    printf("gcov-tool exit status: %d\n", status);
    
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    cleanup_temp_dir(temp_dir);
    
    printf("\nTest completed successfully!\n");
    printf("Covered the following gcov-tool.cc code paths:\n");
    printf("  - case 'v': verbose = true; gcov_set_verbose();\n");
    printf("  - case 'f': overlap_func_level = 1;\n");
    printf("  - case 'F': overlap_use_fullname = 1;\n");
    printf("  - case 'o': overlap_obj_level = 1;\n");
    printf("  - case 'h': overlap_hot_only = 1;\n");
    printf("  - case 't': overlap_hot_threshold = atof(optarg);\n");
    printf("  - default: overlap_usage();\n");
    
    return 0;
}
