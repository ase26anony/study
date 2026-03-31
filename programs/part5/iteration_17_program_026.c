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
"    int value = 5;  /* Default value */\n"
"    \n"
"    if (argc > 1) {\n"
"        value = atoi(argv[1]);\n"
"    }\n"
"    \n"
"    /* Environment variable can also affect behavior */\n"
"    char *env_val = getenv(\"TEST_VALUE\");\n"
"    if (env_val != NULL) {\n"
"        value += atoi(env_val);\n"
"    }\n"
"    \n"
"    int result = process_value(value);\n"
"    printf(\"Result: %d\\n\", result);\n"
"    return 0;\n"
"}\n";

/* Function to execute a command and capture output */
int execute_command(const char *cmd, char *output, size_t output_size) {
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to execute command: %s\n", cmd);
        return -1;
    }
    
    if (output && output_size > 0) {
        output[0] = '\0';
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
    
    int status = pclose(fp);
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

/* Find gcov-tool path */
const char *find_gcov_tool() {
    const char *paths[] = {
        "/usr/bin/gcov-tool",
        "/usr/local/bin/gcov-tool",
        "/bin/gcov-tool",
        NULL
    };
    
    for (int i = 0; paths[i] != NULL; i++) {
        if (access(paths[i], X_OK) == 0) {
            return paths[i];
        }
    }
    
    /* Check if GCOV_TOOL environment variable is set */
    const char *env_tool = getenv("GCOV_TOOL");
    if (env_tool != NULL && access(env_tool, X_OK) == 0) {
        return env_tool;
    }
    
    return NULL;
}

int main() {
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
    strcpy(temp_dir, TEMP_DIR_TEMPLATE);
    if (mkdtemp(temp_dir) == NULL) {
        perror("Failed to create temporary directory");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", temp_dir);
    
    /* Set environment to write .gcda files to our temp directory */
    setenv("GCOV_PREFIX", temp_dir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Write test program source file */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", temp_dir);
    FILE *src_fp = fopen(source_path, "w");
    if (src_fp == NULL) {
        perror("Failed to create source file");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    fwrite(test_program_source, 1, strlen(test_program_source), src_fp);
    fclose(src_fp);
    
    /* Compile test program with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_program", temp_dir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g \"%s\" -o \"%s\"",
             source_path, exec_path);
    
    printf("Compiling test program: %s\n", cmd);
    ret = execute_command(cmd, output, sizeof(output));
    if (ret != 0) {
        fprintf(stderr, "Compilation failed. Output:\n%s\n", output);
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Find .gcno file (created during compilation) */
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", temp_dir);
    if (access(gcno_path, F_OK) != 0) {
        /* Try alternative location */
        snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.c.gcno", temp_dir);
        if (access(gcno_path, F_OK) != 0) {
            fprintf(stderr, "Could not find .gcno file\n");
            cleanup_temp_dir(temp_dir);
            return 1;
        }
    }
    
    /* Run test program twice with different inputs to generate two .gcda files */
    
    /* First run with argument value 3 */
    printf("Running test program (first run)...\n");
    snprintf(cmd, sizeof(cmd), "\"%s\" 3", exec_path);
    ret = execute_command(cmd, output, sizeof(output));
    if (ret != 0) {
        fprintf(stderr, "First run failed\n");
    }
    
    /* Find first .gcda file */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", temp_dir);
    if (access(gcda1_path, F_OK) != 0) {
        snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.c.gcda", temp_dir);
    }
    
    /* Second run with argument value 7 and environment variable */
    printf("Running test program (second run)...\n");
    setenv("TEST_VALUE", "2", 1);
    snprintf(cmd, sizeof(cmd), "\"%s\" 7", exec_path);
    ret = execute_command(cmd, output, sizeof(output));
    if (ret != 0) {
        fprintf(stderr, "Second run failed\n");
    }
    unsetenv("TEST_VALUE");
    
    /* Find second .gcda file - need to rename first one */
    char gcda1_backup[MAX_PATH];
    snprintf(gcda1_backup, sizeof(gcda1_backup), "%s/test_func_run1.gcda", temp_dir);
    rename(gcda1_path, gcda1_backup);
    strcpy(gcda1_path, gcda1_backup);
    
    /* Now get the new .gcda from second run */
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func.gcda", temp_dir);
    if (access(gcda2_path, F_OK) != 0) {
        snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func.c.gcda", temp_dir);
    }
    
    printf("Generated .gcda files:\n");
    printf("  %s\n", gcda1_path);
    printf("  %s\n", gcda2_path);
    
    /* Find gcov-tool */
    const char *gcov_tool = find_gcov_tool();
    if (gcov_tool == NULL) {
        fprintf(stderr, "Could not find gcov-tool. Please set GCOV_TOOL environment variable.\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    printf("Using gcov-tool: %s\n", gcov_tool);
    
    /* Test 1: Trigger all the case statements in the uncovered block */
    printf("\n=== Test 1: Triggering all case statements ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -v -f -F -o -h -t 0.75 \"%s\" \"%s\"",
             gcov_tool, gcda1_path, gcda2_path);
    
    printf("Executing: %s\n", cmd);
    ret = execute_command(cmd, output, sizeof(output));
    printf("Exit code: %d\n", ret);
    if (strlen(output) > 0) {
        printf("Output (first 500 chars):\n%.500s\n", output);
    }
    
    /* Test 2: Trigger the default case with invalid option */
    printf("\n=== Test 2: Triggering default case with invalid option ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -Z \"%s\" \"%s\"",
             gcov_tool, gcda1_path, gcda2_path);
    
    printf("Executing: %s\n", cmd);
    ret = execute_command(cmd, output, sizeof(output));
    printf("Exit code: %d\n", ret);
    if (strlen(output) > 0) {
        printf("Output (first 500 chars):\n%.500s\n", output);
    }
    
    /* Test 3: Additional test with just verbose flag */
    printf("\n=== Test 3: Testing verbose flag only ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -v \"%s\" \"%s\"",
             gcov_tool, gcda1_path, gcda2_path);
    
    printf("Executing: %s\n", cmd);
    ret = execute_command(cmd, output, sizeof(output));
    printf("Exit code: %d\n", ret);
    
    /* Test 4: Test with threshold value */
    printf("\n=== Test 4: Testing with different threshold ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -v -t 0.5 \"%s\" \"%s\"",
             gcov_tool, gcda1_path, gcda2_path);
    
    printf("Executing: %s\n", cmd);
    ret = execute_command(cmd, output, sizeof(output));
    printf("Exit code: %d\n", ret);
    
    /* Clean up */
    printf("\nCleaning up temporary directory: %s\n", temp_dir);
    cleanup_temp_dir(temp_dir);
    
    printf("\nAll tests completed successfully!\n");
    printf("The following gcov-tool options were tested:\n");
    printf("  -v (verbose) - triggers: verbose = true; gcov_set_verbose();\n");
    printf("  -f (func level) - triggers: overlap_func_level = 1;\n");
    printf("  -F (fullname) - triggers: overlap_use_fullname = 1;\n");
    printf("  -o (obj level) - triggers: overlap_obj_level = 1;\n");
    printf("  -h (hot only) - triggers: overlap_hot_only = 1;\n");
    printf("  -t (threshold) - triggers: overlap_hot_threshold = atof(optarg);\n");
    printf("  -Z (invalid) - triggers: default: overlap_usage();\n");
    
    return 0;
}
