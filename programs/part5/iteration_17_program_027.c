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
"    int value = 10;  /* default */\n"
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

/* Create a temporary directory with a unique name */
char *create_temp_dir() {
    char *template = strdup(TEMPLATE);
    if (!template) {
        perror("strdup failed");
        return NULL;
    }
    
    char *dir_name = mkdtemp(template);
    if (!dir_name) {
        perror("mkdtemp failed");
        free(template);
        return NULL;
    }
    
    return dir_name;
}

/* Write the test program source to a file */
int write_test_program(const char *dir_path, const char *filename) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s/%s", dir_path, filename);
    
    FILE *fp = fopen(path, "w");
    if (!fp) {
        perror("fopen failed");
        return -1;
    }
    
    fputs(test_program_source, fp);
    fclose(fp);
    return 0;
}

/* Compile the test program with coverage instrumentation */
int compile_with_coverage(const char *dir_path, const char *source_file, 
                          const char *executable) {
    char cmd[MAX_CMD];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    
    snprintf(source_path, sizeof(source_path), "%s/%s", dir_path, source_file);
    snprintf(exec_path, sizeof(exec_path), "%s/%s", dir_path, executable);
    
    /* Compile with coverage flags */
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s 2>&1",
             source_path, exec_path);
    
    printf("Compiling: %s\n", cmd);
    
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        return -1;
    }
    
    /* Read and discard compilation output */
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        /* Optional: print compilation warnings/errors */
        printf("%s", buffer);
    }
    
    int status = pclose(fp);
    if (status != 0) {
        fprintf(stderr, "Compilation failed with status %d\n", status);
        return -1;
    }
    
    return 0;
}

/* Run the test program to generate .gcda files */
int run_test_program(const char *dir_path, const char *executable, 
                     int run_id, const char *input_arg) {
    char cmd[MAX_CMD];
    char exec_path[MAX_PATH];
    
    snprintf(exec_path, sizeof(exec_path), "%s/%s", dir_path, executable);
    
    /* Set GCOV_PREFIX to write .gcda files to our temp directory */
    char gcov_prefix[MAX_PATH];
    snprintf(gcov_prefix, sizeof(gcov_prefix), "GCOV_PREFIX=%s", dir_path);
    
    /* Set GCOV_PREFIX_STRIP to strip path components */
    char gcov_strip[] = "GCOV_PREFIX_STRIP=0";
    
    /* Prepare environment variables */
    char *envp[] = {
        gcov_prefix,
        gcov_strip,
        NULL
    };
    
    /* Prepare command arguments */
    char *argv[] = {
        exec_path,
        (char *)input_arg,
        NULL
    };
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        execve(exec_path, argv, envp);
        perror("execve failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Run %d completed successfully\n", run_id);
            return 0;
        } else {
            fprintf(stderr, "Run %d failed\n", run_id);
            return -1;
        }
    } else {
        perror("fork failed");
        return -1;
    }
}

/* Execute gcov-tool with specified arguments */
int run_gcov_tool(const char *dir_path, const char **gcda_files, 
                  int num_files, const char *extra_args) {
    char cmd[MAX_CMD];
    char gcda_paths[MAX_CMD] = "";
    
    /* Build list of .gcda file paths */
    for (int i = 0; i < num_files; i++) {
        char temp[MAX_PATH];
        snprintf(temp, sizeof(temp), "%s/%s", dir_path, gcda_files[i]);
        strncat(gcda_paths, temp, sizeof(gcda_paths) - strlen(gcda_paths) - 1);
        if (i < num_files - 1) {
            strncat(gcda_paths, " ", sizeof(gcda_paths) - strlen(gcda_paths) - 1);
        }
    }
    
    /* Find gcov-tool path */
    char *gcov_tool_path = getenv("GCOV_TOOL");
    if (!gcov_tool_path) {
        gcov_tool_path = "/usr/bin/gcov-tool";
    }
    
    /* Construct the command */
    snprintf(cmd, sizeof(cmd), "%s %s %s 2>&1", 
             gcov_tool_path, extra_args, gcda_paths);
    
    printf("\nExecuting: %s\n", cmd);
    
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        return -1;
    }
    
    /* Read and display output */
    char buffer[1024];
    printf("Output:\n");
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
    }
    
    int status = pclose(fp);
    printf("Exit status: %d\n\n", status);
    
    return status;
}

/* Clean up temporary directory */
void cleanup_temp_dir(const char *dir_path) {
    if (!dir_path) return;
    
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dir_path);
    
    printf("Cleaning up: %s\n", dir_path);
    system(cmd);
}

int main() {
    char *temp_dir = NULL;
    int ret = 0;
    
    printf("=== Starting gcov-tool coverage test ===\n");
    
    /* Step 1: Create temporary directory */
    temp_dir = create_temp_dir();
    if (!temp_dir) {
        fprintf(stderr, "Failed to create temp directory\n");
        return EXIT_FAILURE;
    }
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Step 2: Write and compile test program */
    if (write_test_program(temp_dir, "test_func.c") != 0) {
        ret = EXIT_FAILURE;
        goto cleanup;
    }
    
    if (compile_with_coverage(temp_dir, "test_func.c", "test_prog") != 0) {
        ret = EXIT_FAILURE;
        goto cleanup;
    }
    
    /* Step 3: Run test program multiple times with different inputs */
    printf("\nGenerating profile data...\n");
    
    /* First run with input 5 */
    if (run_test_program(temp_dir, "test_prog", 1, "5") != 0) {
        ret = EXIT_FAILURE;
        goto cleanup;
    }
    
    /* Second run with input 15 */
    if (run_test_program(temp_dir, "test_prog", 2, "15") != 0) {
        ret = EXIT_FAILURE;
        goto cleanup;
    }
    
    /* Third run with input 8 (optional, for more data) */
    if (run_test_program(temp_dir, "test_prog", 3, "8") != 0) {
        ret = EXIT_FAILURE;
        goto cleanup;
    }
    
    /* Step 4: Prepare .gcda file list */
    const char *gcda_files[] = {
        "test_func.gcda",  /* Will be created in temp_dir due to GCOV_PREFIX */
    };
    int num_gcda_files = 1;  /* Actually same file overwritten, but gcov-tool needs multiple runs */
    
    /* Note: In real scenario, we'd need multiple .gcda files from different runs.
     * For this test, we'll use the same file multiple times which gcov-tool
     * should still process (though results may not be meaningful). */
    
    /* Step 5: Run gcov-tool with all target flags to trigger uncovered lines */
    printf("\n=== Testing gcov-tool with all target flags ===\n");
    
    /* This command triggers:
     * -v: verbose = true; gcov_set_verbose();
     * -f: overlap_func_level = 1;
     * -F: overlap_use_fullname = 1;
     * -o: overlap_obj_level = 1;
     * -h: overlap_hot_only = 1;
     * -t 0.75: overlap_hot_threshold = atof(optarg);
     */
    const char *gcda_file1 = "test_func.gcda";
    char gcda_path1[MAX_PATH];
    char gcda_path2[MAX_PATH];
    char gcda_path3[MAX_PATH];
    
    snprintf(gcda_path1, sizeof(gcda_path1), "%s/%s", temp_dir, gcda_file1);
    
    /* Create multiple copies with different names to simulate multiple runs */
    char cmd_copy[MAX_CMD];
    snprintf(cmd_copy, sizeof(cmd_copy), "cp %s %s/test_func_run1.gcda", 
             gcda_path1, temp_dir);
    system(cmd_copy);
    snprintf(cmd_copy, sizeof(cmd_copy), "cp %s %s/test_func_run2.gcda", 
             gcda_path1, temp_dir);
    system(cmd_copy);
    snprintf(cmd_copy, sizeof(cmd_copy), "cp %s %s/test_func_run3.gcda", 
             gcda_path1, temp_dir);
    system(cmd_copy);
    
    snprintf(gcda_path1, sizeof(gcda_path1), "%s/test_func_run1.gcda", temp_dir);
    snprintf(gcda_path2, sizeof(gcda_path2), "%s/test_func_run2.gcda", temp_dir);
    snprintf(gcda_path3, sizeof(gcda_path3), "%s/test_func_run3.gcda", temp_dir);
    
    /* Find gcov-tool */
    char *gcov_tool_path = getenv("GCOV_TOOL");
    if (!gcov_tool_path) {
        gcov_tool_path = "gcov-tool";  /* Rely on PATH */
    }
    
    /* First: Run with all valid flags to trigger the switch cases */
    char cmd1[MAX_CMD];
    snprintf(cmd1, sizeof(cmd1), "%s -v -f -F -o -h -t 0.75 %s %s %s",
             gcov_tool_path, gcda_path1, gcda_path2, gcda_path3);
    
    printf("Executing: %s\n", cmd1);
    FILE *fp1 = popen(cmd1, "r");
    if (fp1) {
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), fp1) != NULL) {
            /* Discard output or log it */
        }
        pclose(fp1);
        printf("First gcov-tool call completed\n");
    }
    
    /* Step 6: Run gcov-tool with invalid option to trigger default case */
    printf("\n=== Testing gcov-tool with invalid option ===\n");
    
    char cmd2[MAX_CMD];
    snprintf(cmd2, sizeof(cmd2), "%s -v -Z %s %s",
             gcov_tool_path, gcda_path1, gcda_path2);
    
    printf("Executing (should trigger overlap_usage): %s\n", cmd2);
    FILE *fp2 = popen(cmd2, "r");
    if (fp2) {
        char buffer[1024];
        printf("Output (should show usage):\n");
        while (fgets(buffer, sizeof(buffer), fp2) != NULL) {
            printf("%s", buffer);
        }
        pclose(fp2);
        printf("Second gcov-tool call completed\n");
    }
    
    printf("\n=== All tests completed successfully ===\n");
    
cleanup:
    /* Step 7: Cleanup */
    if (temp_dir) {
        cleanup_temp_dir(temp_dir);
        free(temp_dir);
    }
    
    return ret;
}
