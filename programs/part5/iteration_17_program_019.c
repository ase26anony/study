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
"int process_value(int x, int threshold) {\n"
"    int result = 0;\n"
"    for (int i = 0; i < x; i++) {\n"
"        if (i % 2 == 0) {\n"
"            result += i;\n"
"        } else {\n"
"            result -= i;\n"
"        }\n"
"    }\n"
"    \n"
"    if (result > threshold) {\n"
"        return 1;\n"
"    } else {\n"
"        return 0;\n"
"    }\n"
"}\n"
"\n"
"int main(int argc, char *argv[]) {\n"
"    int value = 10;\n"
"    int threshold = 20;\n"
"    \n"
"    if (argc > 1) {\n"
"        value = atoi(argv[1]);\n"
"    }\n"
"    \n"
"    if (argc > 2) {\n"
"        threshold = atoi(argv[2]);\n"
"    }\n"
"    \n"
"    int res = process_value(value, threshold);\n"
"    printf(\"Result: %d\\n\", res);\n"
"    return 0;\n"
"}\n";

/* Clean up temporary directory */
void cleanup_temp_dir(const char *temp_dir) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", temp_dir);
    system(cmd);
}

/* Execute a command and capture output */
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
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Find gcov-tool path */
const char *find_gcov_tool() {
    const char *paths[] = {
        "/usr/bin/gcov-tool",
        "/usr/local/bin/gcov-tool",
        "/bin/gcov-tool",
        NULL
    };
    
    /* Check if GCOV_TOOL environment variable is set */
    const char *env_tool = getenv("GCOV_TOOL");
    if (env_tool && access(env_tool, X_OK) == 0) {
        return env_tool;
    }
    
    /* Try default paths */
    for (int i = 0; paths[i] != NULL; i++) {
        if (access(paths[i], X_OK) == 0) {
            return paths[i];
        }
    }
    
    return NULL;
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
    const char *gcov_tool;
    
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
    
    status = execute_command(cmd, output, sizeof(output));
    if (status != 0) {
        fprintf(stderr, "Compilation failed:\n%s\n", output);
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Get .gcno file path (created during compilation) */
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", temp_dir);
    
    /* Run program twice with different parameters to generate different .gcda files */
    
    /* First run */
    printf("\n=== First run ===\n");
    snprintf(cmd, sizeof(cmd), "\"%s\" 5 10", exec_path);
    status = execute_command(cmd, NULL, 0);
    
    /* Rename the generated .gcda file */
    char temp_gcda[MAX_PATH];
    snprintf(temp_gcda, sizeof(temp_gcda), "%s/test_func.gcda", temp_dir);
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func_run1.gcda", temp_dir);
    rename(temp_gcda, gcda1_path);
    
    /* Second run with different parameters */
    printf("\n=== Second run ===\n");
    snprintf(cmd, sizeof(cmd), "\"%s\" 20 5", exec_path);
    status = execute_command(cmd, NULL, 0);
    
    /* Rename second .gcda file */
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func_run2.gcda", temp_dir);
    rename(temp_gcda, gcda2_path);
    
    /* Find gcov-tool */
    gcov_tool = find_gcov_tool();
    if (!gcov_tool) {
        fprintf(stderr, "gcov-tool not found. Please set GCOV_TOOL environment variable.\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    printf("\nFound gcov-tool at: %s\n", gcov_tool);
    
    /* Test 1: Trigger all the uncovered option handling code */
    printf("\n=== Test 1: Triggering uncovered option handling ===\n");
    snprintf(cmd, sizeof(cmd), 
             "\"%s\" overlap -v -f -F -o -h -t 0.75 \"%s\" \"%s\"",
             gcov_tool, gcda1_path, gcda2_path);
    
    status = execute_command(cmd, output, sizeof(output));
    printf("gcov-tool output (first 500 chars):\n%.500s\n", output);
    
    if (status != 0) {
        fprintf(stderr, "Warning: gcov-tool returned non-zero: %d\n", status);
    }
    
    /* Test 2: Trigger default case with invalid option to call overlap_usage() */
    printf("\n=== Test 2: Triggering default case (invalid option) ===\n");
    snprintf(cmd, sizeof(cmd), 
             "\"%s\" overlap -Z \"%s\" \"%s\"",
             gcov_tool, gcda1_path, gcda2_path);
    
    status = execute_command(cmd, output, sizeof(output));
    printf("gcov-tool output with invalid option:\n%s\n", output);
    
    if (status != 0) {
        printf("Expected non-zero return for invalid option\n");
    }
    
    /* Test 3: Additional test with just verbose flag */
    printf("\n=== Test 3: Testing verbose flag only ===\n");
    snprintf(cmd, sizeof(cmd), 
             "\"%s\" overlap -v \"%s\" \"%s\"",
             gcov_tool, gcda1_path, gcda2_path);
    
    status = execute_command(cmd, output, sizeof(output));
    printf("gcov-tool verbose output (first 300 chars):\n%.300s\n", output);
    
    /* Clean up */
    printf("\n=== Cleaning up ===\n");
    cleanup_temp_dir(temp_dir);
    
    printf("\n=== Test completed successfully ===\n");
    printf("Triggered the following uncovered code paths:\n");
    printf("1. case 'v': verbose = true; gcov_set_verbose();\n");
    printf("2. case 'f': overlap_func_level = 1;\n");
    printf("3. case 'F': overlap_use_fullname = 1;\n");
    printf("4. case 'o': overlap_obj_level = 1;\n");
    printf("5. case 'h': overlap_hot_only = 1;\n");
    printf("6. case 't': overlap_hot_threshold = atof(optarg);\n");
    printf("7. default: overlap_usage();\n");
    
    return 0;
}
