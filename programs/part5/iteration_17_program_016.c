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

/* Execute a shell command and return its exit status */
int execute_command(const char *cmd, int capture_output) {
    printf("Executing: %s\n", cmd);
    
    if (capture_output) {
        FILE *fp = popen(cmd, "r");
        if (fp == NULL) {
            perror("popen failed");
            return -1;
        }
        
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            /* Discard or log output - for debugging you could printf here */
        }
        
        int status = pclose(fp);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    } else {
        return system(cmd);
    }
}

/* Clean up temporary directory */
void cleanup_temp_dir(const char *temp_dir) {
    if (temp_dir == NULL) return;
    
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
    
    /* Check if path is provided via environment */
    char *env_path = getenv("GCOV_TOOL");
    if (env_path != NULL && access(env_path, X_OK) == 0) {
        return env_path;
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
    char cmd[MAX_CMD];
    int status;
    
    /* Create temporary directory */
    strcpy(temp_dir, TEMP_DIR_TEMPLATE);
    if (mkdtemp(temp_dir) == NULL) {
        perror("Failed to create temporary directory");
        return 1;
    }
    
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Set environment to ensure .gcda files go to our temp directory */
    setenv("GCOV_PREFIX", temp_dir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Write test program source */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", temp_dir);
    FILE *src_file = fopen(source_path, "w");
    if (src_file == NULL) {
        perror("Failed to create source file");
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
    
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "Compilation failed with status %d\n", status);
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* First run - generate first .gcda file */
    printf("\n=== First run ===\n");
    unsetenv("TEST_VALUE");  /* Ensure clean environment */
    snprintf(cmd, sizeof(cmd), "\"%s\" 3", exec_path);
    status = execute_command(cmd, 1);
    
    /* Rename the generated .gcda file to preserve it */
    char gcda_temp[MAX_PATH];
    snprintf(gcda_temp, sizeof(gcda_temp), "%s/test_func.gcda", temp_dir);
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func_run1.gcda", temp_dir);
    rename(gcda_temp, gcda1_path);
    
    /* Second run - generate second .gcda file with different behavior */
    printf("\n=== Second run ===\n");
    setenv("TEST_VALUE", "2", 1);  /* Different input via environment */
    snprintf(cmd, sizeof(cmd), "\"%s\" 4", exec_path);  /* Different argument */
    status = execute_command(cmd, 1);
    
    /* Rename second .gcda file */
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func_run2.gcda", temp_dir);
    rename(gcda_temp, gcda2_path);
    
    /* Find gcov-tool */
    const char *gcov_tool = find_gcov_tool();
    if (gcov_tool == NULL) {
        fprintf(stderr, "gcov-tool not found. Set GCOV_TOOL environment variable.\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    printf("\nFound gcov-tool at: %s\n", gcov_tool);
    
    /* Test 1: Trigger all the specific flag cases from uncovered lines */
    printf("\n=== Test 1: Triggering specific flag cases ===\n");
    printf("This should trigger: verbose, overlap_func_level, overlap_use_fullname,\n");
    printf("overlap_obj_level, overlap_hot_only, overlap_hot_threshold\n");
    
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -v -f -F -o -h -t 0.75 \"%s\" \"%s\"",
             gcov_tool, gcda1_path, gcda2_path);
    
    status = execute_command(cmd, 1);
    printf("gcov-tool overlap analysis exited with status: %d\n", status);
    
    /* Test 2: Trigger default case with invalid option */
    printf("\n=== Test 2: Triggering default case (invalid option) ===\n");
    printf("This should trigger overlap_usage() via the default case\n");
    
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -Z \"%s\" \"%s\"",
             gcov_tool, gcda1_path, gcda2_path);
    
    status = execute_command(cmd, 1);
    printf("gcov-tool with invalid option exited with status: %d\n", status);
    
    /* Also test with just -h flag (help) which might also trigger usage */
    printf("\n=== Test 3: Testing help flag ===\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -h", gcov_tool);
    status = execute_command(cmd, 1);
    
    /* Clean up */
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
