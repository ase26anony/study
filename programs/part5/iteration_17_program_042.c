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
"            result += i;\n"
"        } else {\n"
"            result -= i;\n"
"        }\n"
"    }\n"
"    return result;\n"
"}\n"
"\n"
"int main(int argc, char *argv[]) {\n"
"    int value = 5;  /* default value */\n"
"    \n"
"    if (argc > 1) {\n"
"        value = atoi(argv[1]);\n"
"    }\n"
"    \n"
"    /* Different execution path based on environment variable */\n"
"    char *env_val = getenv(\"TEST_MODE\");\n"
"    if (env_val && strcmp(env_val, \"ALT\") == 0) {\n"
"        value *= 2;\n"
"    }\n"
"    \n"
"    int result = process_value(value);\n"
"    printf(\"Result: %d\\n\", result);\n"
"    return 0;\n"
"}\n";

/* Clean up temporary directory and files */
void cleanup_temp_dir(const char *temp_dir) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", temp_dir);
    system(cmd);
}

/* Execute a command and capture its output */
int execute_command(const char *cmd, char *output, size_t output_size) {
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        return -1;
    }
    
    if (output) {
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
            /* discard */
        }
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
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
    char *env_tool = getenv("GCOV_TOOL");
    if (env_tool && access(env_tool, X_OK) == 0) {
        return env_tool;
    }
    
    /* Check standard paths */
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
    char cmd[MAX_CMD];
    char output[4096];
    int status;
    const char *gcov_tool;
    
    printf("=== Starting gcov-tool coverage test ===\n");
    
    /* Find gcov-tool */
    gcov_tool = find_gcov_tool();
    if (!gcov_tool) {
        fprintf(stderr, "Error: gcov-tool not found. Please set GCOV_TOOL environment variable.\n");
        return 1;
    }
    printf("Using gcov-tool: %s\n", gcov_tool);
    
    /* Create temporary directory */
    snprintf(temp_dir, sizeof(temp_dir), "/tmp/gcov_test_%d_%ld", getpid(), time(NULL));
    if (mkdir(temp_dir, 0755) != 0) {
        perror("Error creating temp directory");
        return 1;
    }
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Set environment to write .gcda files to our temp directory */
    setenv("GCOV_PREFIX", temp_dir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Write test program source */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", temp_dir);
    FILE *src_file = fopen(source_path, "w");
    if (!src_file) {
        perror("Error creating source file");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    fputs(test_program_source, src_file);
    fclose(src_file);
    
    /* Compile test program with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", temp_dir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g \"%s\" -o \"%s\"",
             source_path, exec_path);
    
    printf("Compiling test program...\n");
    status = execute_command(cmd, output, sizeof(output));
    if (status != 0) {
        fprintf(stderr, "Compilation failed:\n%s\n", output);
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* First run - generate first .gcda file */
    printf("Running test program (first run)...\n");
    unsetenv("TEST_MODE");  /* Ensure TEST_MODE is not set */
    snprintf(cmd, sizeof(cmd), "\"%s\" 3", exec_path);
    status = execute_command(cmd, NULL, 0);
    if (status != 0) {
        fprintf(stderr, "First run failed\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Find the generated .gcda file */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", temp_dir);
    
    /* Second run - with different conditions to generate different profile */
    printf("Running test program (second run with different conditions)...\n");
    setenv("TEST_MODE", "ALT", 1);  /* Set environment variable for different path */
    snprintf(cmd, sizeof(cmd), "\"%s\" 4", exec_path);
    status = execute_command(cmd, NULL, 0);
    if (status != 0) {
        fprintf(stderr, "Second run failed\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Find the second .gcda file (it will overwrite the first, so we need to copy) */
    /* Actually, we need to run in different directories or rename between runs */
    /* Let's use a different approach: run with different GCOV_PREFIX */
    
    /* Clean up and use a better approach */
    unlink(gcda1_path);
    
    /* Run first program instance */
    printf("Running with first configuration...\n");
    char temp_dir1[MAX_PATH];
    snprintf(temp_dir1, sizeof(temp_dir1), "%s/run1", temp_dir);
    mkdir(temp_dir1, 0755);
    setenv("GCOV_PREFIX", temp_dir1, 1);
    unsetenv("TEST_MODE");
    snprintf(cmd, sizeof(cmd), "\"%s\" 3", exec_path);
    execute_command(cmd, NULL, 0);
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", temp_dir1);
    
    /* Run second program instance */
    printf("Running with second configuration...\n");
    char temp_dir2[MAX_PATH];
    snprintf(temp_dir2, sizeof(temp_dir2), "%s/run2", temp_dir);
    mkdir(temp_dir2, 0755);
    setenv("GCOV_PREFIX", temp_dir2, 1);
    setenv("TEST_MODE", "ALT", 1);
    snprintf(cmd, sizeof(cmd), "\"%s\" 4", exec_path);
    execute_command(cmd, NULL, 0);
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func.gcda", temp_dir2);
    
    /* Verify .gcda files exist */
    if (access(gcda1_path, R_OK) != 0 || access(gcda2_path, R_OK) != 0) {
        fprintf(stderr, "Error: .gcda files not created\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    printf("Generated .gcda files:\n  %s\n  %s\n", gcda1_path, gcda2_path);
    
    /* ============================================== */
    /* TEST 1: Trigger all the flag cases in gcov-tool */
    /* ============================================== */
    printf("\n=== Test 1: Testing gcov-tool with all overlap flags ===\n");
    snprintf(cmd, sizeof(cmd),
             "\"%s\" overlap -v -f -F -o -h -t 0.75 \"%s\" \"%s\"",
             gcov_tool, gcda1_path, gcda2_path);
    
    printf("Executing: %s\n", cmd);
    status = execute_command(cmd, output, sizeof(output));
    
    if (status == 0) {
        printf("gcov-tool executed successfully with flags\n");
        if (strlen(output) > 0) {
            printf("Output (first 500 chars):\n%.500s\n", output);
        }
    } else {
        printf("gcov-tool exited with status %d\n", status);
        if (strlen(output) > 0) {
            printf("Output:\n%s\n", output);
        }
    }
    
    /* ============================================== */
    /* TEST 2: Trigger default case with invalid option */
    /* ============================================== */
    printf("\n=== Test 2: Testing gcov-tool with invalid option (to trigger default case) ===\n");
    snprintf(cmd, sizeof(cmd),
             "\"%s\" overlap -Z \"%s\" \"%s\" 2>&1",
             gcov_tool, gcda1_path, gcda2_path);
    
    printf("Executing: %s\n", cmd);
    status = execute_command(cmd, output, sizeof(output));
    
    printf("Exit status: %d\n", status);
    if (strlen(output) > 0) {
        printf("Output (should show usage):\n%s\n", output);
        
        /* Check if usage message was displayed */
        if (strstr(output, "Usage") != NULL || strstr(output, "usage") != NULL ||
            strstr(output, "option") != NULL) {
            printf("SUCCESS: overlap_usage() was called (usage message displayed)\n");
        }
    }
    
    /* ============================================== */
    /* Cleanup and exit */
    /* ============================================== */
    printf("\n=== Cleaning up ===\n");
    cleanup_temp_dir(temp_dir);
    
    printf("\n=== Test completed ===\n");
    printf("The following gcov-tool code paths should have been triggered:\n");
    printf("  - case 'v': verbose = true; gcov_set_verbose();\n");
    printf("  - case 'f': overlap_func_level = 1;\n");
    printf("  - case 'F': overlap_use_fullname = 1;\n");
    printf("  - case 'o': overlap_obj_level = 1;\n");
    printf("  - case 'h': overlap_hot_only = 1;\n");
    printf("  - case 't': overlap_hot_threshold = atof(optarg);\n");
    printf("  - default: overlap_usage(); (via invalid option -Z)\n");
    
    return 0;
}
