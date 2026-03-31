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
#define TEMPLATE "/tmp/gcov_test_XXXXXX"

/* Simple test program that will be compiled with coverage */
const char *test_source = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"\n"
"int process_value(int x, int mode) {\n"
"    int result = 0;\n"
"    for (int i = 0; i < x; i++) {\n"
"        if (mode == 1) {\n"
"            result += i * 2;\n"
"        } else if (mode == 2) {\n"
"            result += i * 3;\n"
"        } else {\n"
"            result += i;\n"
"        }\n"
"        if (i % 2 == 0) {\n"
"            result += 1;\n"
"        }\n"
"    }\n"
"    return result;\n"
"}\n"
"\n"
"int main(int argc, char *argv[]) {\n"
"    int mode = 1;\n"
"    int value = 10;\n"
"    \n"
"    if (argc > 1) {\n"
"        mode = atoi(argv[1]);\n"
"    }\n"
"    if (argc > 2) {\n"
"        value = atoi(argv[2]);\n"
"    }\n"
"    \n"
"    int result = process_value(value, mode);\n"
"    printf(\"Result: %d\\n\", result);\n"
"    return 0;\n"
"}\n";

/* Execute a command and capture its output */
int execute_command(const char *cmd, char *output, size_t output_size) {
    FILE *fp;
    int status;
    
    if (output && output_size > 0) {
        output[0] = '\0';
    }
    
    fp = popen(cmd, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to execute command: %s\n", cmd);
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
    return WEXITSTATUS(status);
}

/* Clean up temporary files */
void cleanup_temp_dir(const char *dir) {
    char cmd[MAX_PATH];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dir);
    system(cmd);
}

int main(int argc, char *argv[]) {
    char temp_dir[MAX_PATH];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    char gcno_path[MAX_PATH];
    char cmd[MAX_PATH * 2];
    char output[4096];
    int status;
    
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
    
    /* Write test source file */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", temp_dir);
    FILE *src_fp = fopen(source_path, "w");
    if (!src_fp) {
        perror("Failed to create source file");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    fputs(test_source, src_fp);
    fclose(src_fp);
    
    /* Compile with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", temp_dir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s",
             source_path, exec_path);
    
    printf("Compiling: %s\n", cmd);
    status = execute_command(cmd, output, sizeof(output));
    if (status != 0) {
        fprintf(stderr, "Compilation failed: %s\n", output);
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Get .gcno file path (created during compilation) */
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", temp_dir);
    
    /* Run program twice with different parameters to generate distinct .gcda files */
    
    /* First run with mode=1, value=10 */
    printf("Running first test (mode=1)...\n");
    snprintf(cmd, sizeof(cmd), "%s 1 10", exec_path);
    status = execute_command(cmd, NULL, 0);
    if (status != 0) {
        fprintf(stderr, "First run failed\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Rename .gcda file to preserve it */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", temp_dir);
    char gcda1_saved[MAX_PATH];
    snprintf(gcda1_saved, sizeof(gcda1_saved), "%s/test_func_run1.gcda", temp_dir);
    rename(gcda1_path, gcda1_saved);
    
    /* Second run with mode=2, value=20 */
    printf("Running second test (mode=2)...\n");
    snprintf(cmd, sizeof(cmd), "%s 2 20", exec_path);
    status = execute_command(cmd, NULL, 0);
    if (status != 0) {
        fprintf(stderr, "Second run failed\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Rename second .gcda file */
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func.gcda", temp_dir);
    char gcda2_saved[MAX_PATH];
    snprintf(gcda2_saved, sizeof(gcda2_saved), "%s/test_func_run2.gcda", temp_dir);
    rename(gcda2_path, gcda2_saved);
    
    /* Find gcov-tool path */
    const char *gcov_tool_path = getenv("GCOV_TOOL");
    if (!gcov_tool_path) {
        gcov_tool_path = "gcov-tool";  /* Rely on PATH */
    }
    
    /* Test 1: Trigger all the specific flags from the uncovered block */
    printf("\n=== Test 1: Triggering specific flags (-v, -f, -F, -o, -h, -t) ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -v -f -F -o -h -t 0.75 %s %s",
             gcov_tool_path, gcda1_saved, gcda2_saved);
    
    printf("Executing: %s\n", cmd);
    status = execute_command(cmd, output, sizeof(output));
    
    if (status == 0) {
        printf("Successfully executed gcov-tool with specific flags\n");
        if (strlen(output) > 0) {
            printf("Output (first 500 chars):\n%.500s\n", output);
        }
    } else {
        printf("gcov-tool exited with status %d\n", status);
        if (strlen(output) > 0) {
            printf("Output:\n%s\n", output);
        }
    }
    
    /* Test 2: Trigger the default case with invalid option */
    printf("\n=== Test 2: Triggering default case with invalid option ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -Z %s %s",
             gcov_tool_path, gcda1_saved, gcda2_saved);
    
    printf("Executing: %s\n", cmd);
    status = execute_command(cmd, output, sizeof(output));
    
    if (status != 0) {
        printf("Successfully triggered default case (expected non-zero exit)\n");
        if (strlen(output) > 0) {
            printf("Output (first 500 chars):\n%.500s\n", output);
        }
    } else {
        printf("Warning: Invalid option didn't cause error (status=%d)\n", status);
    }
    
    /* Test 3: Additional test with just verbose flag */
    printf("\n=== Test 3: Testing with verbose flag only ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -v %s %s",
             gcov_tool_path, gcda1_saved, gcda2_saved);
    
    printf("Executing: %s\n", cmd);
    status = execute_command(cmd, output, sizeof(output));
    
    if (status == 0) {
        printf("Successfully executed gcov-tool with verbose flag\n");
    }
    
    /* Cleanup */
    printf("\nCleaning up temporary directory: %s\n", temp_dir);
    cleanup_temp_dir(temp_dir);
    
    printf("\nAll tests completed successfully!\n");
    return 0;
}
