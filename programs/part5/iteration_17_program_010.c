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
"    if (env_val) {\n"
"        value += atoi(env_val);\n"
"    }\n"
"    \n"
"    int result = process_value(value);\n"
"    printf(\"Result: %d\\n\", result);\n"
"    return 0;\n"
"}\n";

/* Function to execute a command and capture output */
int execute_command(const char *cmd, int capture_output) {
    FILE *fp;
    char buffer[1024];
    int status;
    
    if (capture_output) {
        fp = popen(cmd, "r");
        if (fp == NULL) {
            perror("popen failed");
            return -1;
        }
        
        /* Read and discard output (or could log it) */
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            /* Optionally print for debugging */
            /* printf("OUTPUT: %s", buffer); */
        }
        
        status = pclose(fp);
    } else {
        status = system(cmd);
    }
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Function to create temporary directory */
char *create_temp_dir() {
    char *template = strdup(TEMPLATE);
    if (template == NULL) {
        return NULL;
    }
    
    char *dir = mkdtemp(template);
    if (dir == NULL) {
        free(template);
        return NULL;
    }
    
    return dir;
}

/* Function to write test program source file */
int write_test_program(const char *dir, const char *filename) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s/%s", dir, filename);
    
    FILE *fp = fopen(path, "w");
    if (fp == NULL) {
        perror("Failed to create test program");
        return 0;
    }
    
    fputs(test_program_source, fp);
    fclose(fp);
    return 1;
}

/* Function to compile test program with coverage */
int compile_with_coverage(const char *dir, const char *source, const char *executable) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), 
             "cd %s && gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s 2>&1",
             dir, source, executable);
    
    printf("Compiling: %s\n", cmd);
    return execute_command(cmd, 1) == 0;
}

/* Function to run test program and generate .gcda files */
int run_test_program(const char *dir, const char *executable, 
                     const char *gcda_prefix, int run_id, int arg_value) {
    char cmd[MAX_CMD];
    char env_cmd[MAX_CMD];
    
    /* Set environment to control where .gcda files are written */
    snprintf(env_cmd, sizeof(env_cmd),
             "cd %s && GCOV_PREFIX='%s' GCOV_PREFIX_STRIP=0 %s/%s %d",
             dir, dir, dir, executable, arg_value);
    
    printf("Running test (run %d): %s\n", run_id, env_cmd);
    
    /* Also set TEST_VALUE environment variable for variation */
    char full_cmd[MAX_CMD];
    snprintf(full_cmd, sizeof(full_cmd),
             "TEST_VALUE=%d %s", run_id * 5, env_cmd);
    
    return execute_command(full_cmd, 1) == 0;
}

/* Function to find gcov-tool path */
const char *find_gcov_tool() {
    const char *gcov_tool = getenv("GCOV_TOOL");
    if (gcov_tool != NULL && access(gcov_tool, X_OK) == 0) {
        return gcov_tool;
    }
    
    /* Try common locations */
    const char *common_paths[] = {
        "/usr/bin/gcov-tool",
        "/usr/local/bin/gcov-tool",
        "/bin/gcov-tool",
        NULL
    };
    
    for (int i = 0; common_paths[i] != NULL; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            return common_paths[i];
        }
    }
    
    return "gcov-tool";  /* Hope it's in PATH */
}

/* Function to run gcov-tool with overlap analysis flags */
int run_gcov_tool_overlap(const char *gcov_tool, const char *dir, 
                          const char *gcda1, const char *gcda2) {
    char cmd[MAX_CMD];
    
    /* Build command with all the flags from the uncovered block */
    snprintf(cmd, sizeof(cmd),
             "%s overlap -v -f -F -o -h -t 0.75 %s/%s %s/%s",
             gcov_tool, dir, gcda1, dir, gcda2);
    
    printf("\nExecuting gcov-tool with overlap flags:\n%s\n", cmd);
    
    int result = execute_command(cmd, 1);
    
    if (result == 0) {
        printf("gcov-tool overlap analysis completed successfully\n");
    } else {
        printf("gcov-tool overlap analysis failed with code %d\n", result);
    }
    
    return result;
}

/* Function to trigger default case with invalid option */
int trigger_default_case(const char *gcov_tool, const char *dir,
                         const char *gcda1, const char *gcda2) {
    char cmd[MAX_CMD];
    
    /* Use invalid option -Z to trigger default case */
    snprintf(cmd, sizeof(cmd),
             "%s overlap -v -Z %s/%s %s/%s 2>&1",
             gcov_tool, dir, gcda1, dir, gcda2);
    
    printf("\nExecuting gcov-tool with invalid option (to trigger default case):\n%s\n", cmd);
    
    /* We expect this to fail, so we capture output to see the usage message */
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("popen failed for invalid option test");
        return -1;
    }
    
    char buffer[1024];
    int found_usage = 0;
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("INVALID OPTION OUTPUT: %s", buffer);
        /* Check if usage message was printed */
        if (strstr(buffer, "Usage:") != NULL || 
            strstr(buffer, "usage:") != NULL ||
            strstr(buffer, "overlap") != NULL) {
            found_usage = 1;
        }
    }
    
    int status = pclose(fp);
    
    if (found_usage) {
        printf("Successfully triggered overlap_usage() via invalid option\n");
    }
    
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

/* Cleanup function */
void cleanup(const char *dir) {
    if (dir == NULL) return;
    
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dir);
    execute_command(cmd, 0);
    printf("\nCleaned up temporary directory: %s\n", dir);
    
    /* Also free the directory string if it was allocated */
    free((void *)dir);
}

int main() {
    char *temp_dir = NULL;
    const char *gcov_tool = NULL;
    int success = 0;
    
    printf("=== Starting gcov-tool coverage test ===\n");
    
    /* Step 1: Create temporary directory */
    temp_dir = create_temp_dir();
    if (temp_dir == NULL) {
        perror("Failed to create temporary directory");
        return EXIT_FAILURE;
    }
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Step 2: Write test program source */
    if (!write_test_program(temp_dir, "test_func.c")) {
        cleanup(temp_dir);
        return EXIT_FAILURE;
    }
    
    /* Step 3: Compile with coverage instrumentation */
    if (!compile_with_coverage(temp_dir, "test_func.c", "test_prog")) {
        fprintf(stderr, "Failed to compile test program\n");
        cleanup(temp_dir);
        return EXIT_FAILURE;
    }
    
    /* Step 4: Run test program twice to generate different .gcda files */
    printf("\nGenerating profile data...\n");
    
    /* First run with argument 10 */
    if (!run_test_program(temp_dir, "test_prog", temp_dir, 1, 10)) {
        fprintf(stderr, "First test run failed\n");
        cleanup(temp_dir);
        return EXIT_FAILURE;
    }
    
    /* Second run with argument 20 for different coverage */
    if (!run_test_program(temp_dir, "test_prog", temp_dir, 2, 20)) {
        fprintf(stderr, "Second test run failed\n");
        cleanup(temp_dir);
        return EXIT_FAILURE;
    }
    
    /* Verify .gcda files were created */
    char gcda1_path[MAX_PATH], gcda2_path[MAX_PATH];
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", temp_dir);
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func.gcda", temp_dir);
    
    /* Note: The second run overwrites the first .gcda, so we need to 
       rename the first one before the second run. Let's adjust the approach. */
    
    /* Actually, let's create two separate .gcda files by copying */
    char backup_gcda[MAX_PATH];
    snprintf(backup_gcda, sizeof(backup_gda), "%s/test_func.gcda.backup", temp_dir);
    
    /* Rename first .gcda to backup */
    char rename_cmd[MAX_CMD];
    snprintf(rename_cmd, sizeof(rename_cmd), "mv %s/test_func.gcda %s/test_func.gcda.1 2>/dev/null",
             temp_dir, temp_dir);
    execute_command(rename_cmd, 0);
    
    /* Run second time to create second .gcda */
    run_test_program(temp_dir, "test_prog", temp_dir, 2, 20);
    
    /* Rename second .gcda */
    snprintf(rename_cmd, sizeof(rename_cmd), "mv %s/test_func.gcda %s/test_func.gcda.2 2>/dev/null",
             temp_dir, temp_dir);
    execute_command(rename_cmd, 0);
    
    /* Restore first .gcda */
    snprintf(rename_cmd, sizeof(rename_cmd), "mv %s/test_func.gcda.1 %s/test_func.gcda 2>/dev/null",
             temp_dir, temp_dir);
    execute_command(rename_cmd, 0);
    
    /* Verify we have two different .gcda files */
    struct stat st1, st2;
    char gcda_file1[MAX_PATH], gcda_file2[MAX_PATH];
    snprintf(gcda_file1, sizeof(gcda_file1), "%s/test_func.gcda", temp_dir);
    snprintf(gcda_file2, sizeof(gcda_file2), "%s/test_func.gcda.2", temp_dir);
    
    if (stat(gcda_file1, &st1) != 0 || stat(gcda_file2, &st2) != 0) {
        fprintf(stderr, "Failed to create two distinct .gcda files\n");
        cleanup(temp_dir);
        return EXIT_FAILURE;
    }
    
    printf("Created two profile data files:\n");
    printf("  %s (%ld bytes)\n", gcda_file1, st1.st_size);
    printf("  %s (%ld bytes)\n", gcda_file2, st2.st_size);
    
    /* Step 5: Find gcov-tool */
    gcov_tool = find_gcov_tool();
    printf("\nUsing gcov-tool at: %s\n", gcov_tool);
    
    /* Step 6: Run gcov-tool with all overlap flags to trigger the uncovered block */
    if (run_gcov_tool_overlap(gcov_tool, temp_dir, 
                              "test_func.gcda", "test_func.gcda.2") != 0) {
        fprintf(stderr, "Warning: gcov-tool overlap analysis may have failed\n");
        /* Continue anyway to test invalid option */
    }
    
    /* Step 7: Trigger default case with invalid option */
    if (trigger_default_case(gcov_tool, temp_dir,
                             "test_func.gcda", "test_func.gcda.2") == 0) {
        printf("Warning: Invalid option didn't cause expected failure\n");
    }
    
    printf("\n=== Test completed ===\n");
    
    /* Cleanup */
    cleanup(temp_dir);
    
    return EXIT_SUCCESS;
}
