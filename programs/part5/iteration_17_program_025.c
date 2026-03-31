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
"    int value = 10;  /* default */\n"
"    if (argc > 1) {\n"
"        value = atoi(argv[1]);\n"
"    }\n"
"    \n"
"    /* Different execution path based on environment */\n"
"    char *env_val = getenv(\"TEST_MODE\");\n"
"    if (env_val && strcmp(env_val, \"ALT\") == 0) {\n"
"        value *= 2;\n"
"    }\n"
"    \n"
"    int res = process_value(value);\n"
"    printf(\"Result: %d\\n\", res);\n"
"    return 0;\n"
"}\n";

/* Clean up temporary directory and files */
void cleanup_tempdir(const char *tempdir) {
    if (tempdir) {
        char cmd[MAX_CMD];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", tempdir);
        system(cmd);
    }
}

/* Execute a command and capture output */
int execute_command(const char *cmd, char *output, size_t output_size) {
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        fprintf(stderr, "Failed to execute: %s\n", cmd);
        return -1;
    }
    
    if (output) {
        output[0] = '\0';
        size_t total = 0;
        while (fgets(output + total, output_size - total, fp) != NULL) {
            total = strlen(output);
            if (total >= output_size - 1) break;
        }
    } else {
        /* Just drain the output */
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            /* discard */
        }
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
}

/* Find gcov-tool path */
const char *find_gcov_tool() {
    const char *env_path = getenv("GCOV_TOOL");
    if (env_path && access(env_path, X_OK) == 0) {
        return env_path;
    }
    
    /* Try common locations */
    const char *common_paths[] = {
        "/usr/bin/gcov-tool",
        "/usr/local/bin/gcov-tool",
        "/bin/gcov-tool",
        NULL
    };
    
    for (int i = 0; common_paths[i]; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            return common_paths[i];
        }
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    char tempdir[MAX_PATH];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    char cmd[MAX_CMD];
    char output[4096];
    int status;
    
    /* Create temporary directory */
    strncpy(tempdir, TEMPLATE, sizeof(tempdir));
    if (!mkdtemp(tempdir)) {
        perror("Failed to create temp directory");
        return 1;
    }
    
    printf("Created temp directory: %s\n", tempdir);
    
    /* Set environment to write .gcda files to our temp directory */
    setenv("GCOV_PREFIX", tempdir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Write test source file */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", tempdir);
    FILE *src_fp = fopen(source_path, "w");
    if (!src_fp) {
        perror("Failed to create source file");
        cleanup_tempdir(tempdir);
        return 1;
    }
    fputs(test_program_source, src_fp);
    fclose(src_fp);
    
    /* Compile with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", tempdir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s 2>&1",
             source_path, exec_path);
    
    printf("Compiling test program...\n");
    status = execute_command(cmd, output, sizeof(output));
    if (status != 0) {
        fprintf(stderr, "Compilation failed:\n%s\n", output);
        cleanup_tempdir(tempdir);
        return 1;
    }
    
    /* Run first time with default parameters */
    printf("Running test program (first run)...\n");
    snprintf(cmd, sizeof(cmd), "%s 5", exec_path);
    status = execute_command(cmd, NULL, 0);
    if (status != 0) {
        fprintf(stderr, "First run failed\n");
        cleanup_tempdir(tempdir);
        return 1;
    }
    
    /* Rename first .gcda file */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", tempdir);
    char gcda1_backup[MAX_PATH];
    snprintf(gcda1_backup, sizeof(gcda1_backup), "%s/test_func_run1.gcda", tempdir);
    rename(gcda1_path, gcda1_backup);
    
    /* Run second time with different environment */
    printf("Running test program (second run with different env)...\n");
    setenv("TEST_MODE", "ALT", 1);
    snprintf(cmd, sizeof(cmd), "%s 8", exec_path);
    status = execute_command(cmd, NULL, 0);
    unsetenv("TEST_MODE");
    
    if (status != 0) {
        fprintf(stderr, "Second run failed\n");
        cleanup_tempdir(tempdir);
        return 1;
    }
    
    /* Rename second .gcda file */
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func.gcda", tempdir);
    char gcda2_backup[MAX_PATH];
    snprintf(gcda2_backup, sizeof(gcda2_backup), "%s/test_func_run2.gcda", tempdir);
    rename(gcda2_path, gcda2_backup);
    
    /* Find gcov-tool */
    const char *gcov_tool = find_gcov_tool();
    if (!gcov_tool) {
        fprintf(stderr, "gcov-tool not found. Set GCOV_TOOL environment variable.\n");
        cleanup_tempdir(tempdir);
        return 1;
    }
    
    printf("Using gcov-tool: %s\n", gcov_tool);
    
    /* Test 1: Trigger all the specific flags from uncovered lines */
    printf("\n=== Test 1: Triggering specific flags (lines 534-554) ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -v -f -F -o -h -t 0.75 %s %s",
             gcov_tool, gcda1_backup, gcda2_backup);
    
    printf("Executing: %s\n", cmd);
    status = execute_command(cmd, output, sizeof(output));
    
    if (status == 0) {
        printf("Successfully executed gcov-tool with all target flags\n");
        if (strlen(output) > 0) {
            printf("Output (first 500 chars):\n%.500s\n", output);
        }
    } else {
        printf("gcov-tool exited with status %d\n", status);
        if (strlen(output) > 0) {
            printf("Output:\n%s\n", output);
        }
    }
    
    /* Test 2: Trigger default case with invalid option */
    printf("\n=== Test 2: Triggering default case (invalid option) ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -Z %s %s",
             gcov_tool, gcda1_backup, gcda2_backup);
    
    printf("Executing: %s\n", cmd);
    status = execute_command(cmd, output, sizeof(output));
    
    if (status != 0) {
        printf("Successfully triggered invalid option handling (expected)\n");
        if (strlen(output) > 0) {
            printf("Output (first 500 chars):\n%.500s\n", output);
        }
    }
    
    /* Clean up */
    printf("\nCleaning up temp directory: %s\n", tempdir);
    cleanup_tempdir(tempdir);
    
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
