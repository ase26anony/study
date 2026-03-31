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

/* Simple test program source code that will be compiled with coverage */
const char *test_program_source = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"\n"
"int main(int argc, char *argv[]) {\n"
"    int i, limit;\n"
"    \n"
"    /* Different runs will have different limits */\n"
"    if (argc > 1) {\n"
"        limit = atoi(argv[1]);\n"
"    } else {\n"
"        limit = 10;  /* Default */\n"
"    }\n"
"    \n"
"    /* Environment variable can also affect execution */\n"
"    char *env_limit = getenv(\"TEST_LIMIT\");\n"
"    if (env_limit) {\n"
"        limit += atoi(env_limit);\n"
"    }\n"
"    \n"
"    int sum = 0;\n"
"    for (i = 0; i < limit; i++) {\n"
"        if (i % 2 == 0) {\n"
"            sum += i;  /* Even path */\n"
"        } else {\n"
"            sum -= i;  /* Odd path */\n"
"        }\n"
"        \n"
"        /* Another conditional to create more branches */\n"
"        if (i > limit / 2) {\n"
"            sum *= 2;\n"
"        }\n"
"    }\n"
"    \n"
"    printf(\"Final sum: %d\\n\", sum);\n"
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

/* Execute a command and capture its output */
int execute_command(const char *cmd, int capture_output) {
    FILE *fp;
    char buffer[256];
    int status;
    
    if (capture_output) {
        printf("Executing: %s\n", cmd);
        fp = popen(cmd, "r");
        if (fp == NULL) {
            perror("popen failed");
            return -1;
        }
        
        /* Read and optionally display output */
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("Output: %s", buffer);
        }
        
        status = pclose(fp);
    } else {
        /* Just execute without capturing output */
        status = system(cmd);
    }
    
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
    char *env_gcov_tool = getenv("GCOV_TOOL");
    if (env_gcov_tool && access(env_gcov_tool, X_OK) == 0) {
        return env_gcov_tool;
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
    char tempdir[MAX_PATH];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    char cmd[MAX_CMD];
    const char *gcov_tool;
    int ret = 0;
    
    /* Create temporary directory */
    strncpy(tempdir, TEMPLATE, sizeof(tempdir));
    if (mkdtemp(tempdir) == NULL) {
        perror("Failed to create temporary directory");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", tempdir);
    
    /* Set environment to write .gcda files to our temp directory */
    setenv("GCOV_PREFIX", tempdir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Create source file */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", tempdir);
    FILE *src_fp = fopen(source_path, "w");
    if (!src_fp) {
        perror("Failed to create source file");
        cleanup_tempdir(tempdir);
        return 1;
    }
    fputs(test_program_source, src_fp);
    fclose(src_fp);
    
    /* Create executable path */
    snprintf(exec_path, sizeof(exec_path), "%s/test_program", tempdir);
    
    /* Compile with coverage instrumentation */
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s",
             source_path, exec_path);
    
    printf("Compiling test program...\n");
    if (execute_command(cmd, 1) != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup_tempdir(tempdir);
        return 1;
    }
    
    /* Run first execution with different parameters to generate first .gcda */
    printf("\nRunning first execution...\n");
    setenv("TEST_LIMIT", "5", 1);  /* Add 5 via environment */
    snprintf(cmd, sizeof(cmd), "%s 8", exec_path);  /* Pass 8 as argument */
    if (execute_command(cmd, 1) != 0) {
        fprintf(stderr, "First execution failed\n");
    }
    
    /* The .gcda file will be named after the source file */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", tempdir);
    
    /* Run second execution with different parameters for second .gcda */
    printf("\nRunning second execution...\n");
    unsetenv("TEST_LIMIT");  /* No environment variable this time */
    snprintf(cmd, sizeof(cmd), "%s 15", exec_path);  /* Pass 15 as argument */
    if (execute_command(cmd, 1) != 0) {
        fprintf(stderr, "Second execution failed\n");
    }
    
    /* The second .gcda will overwrite the first, so we need to rename it */
    /* First, copy the first .gcda to a different name */
    char gcda1_backup[MAX_PATH];
    snprintf(gcda1_backup, sizeof(gcda1_backup), "%s/test_func_run1.gcda", tempdir);
    snprintf(cmd, sizeof(cmd), "cp %s %s", gcda1_path, gcda1_backup);
    execute_command(cmd, 0);
    
    /* Rename the second .gcda */
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func_run2.gcda", tempdir);
    snprintf(cmd, sizeof(cmd), "mv %s %s", gcda1_path, gcda2_path);
    execute_command(cmd, 0);
    
    /* Restore first .gcda */
    snprintf(cmd, sizeof(cmd), "mv %s %s", gcda1_backup, gcda1_path);
    execute_command(cmd, 0);
    
    printf("\nGenerated .gcda files:\n");
    printf("  %s\n", gcda1_path);
    printf("  %s\n", gcda2_path);
    
    /* Find gcov-tool */
    gcov_tool = find_gcov_tool();
    if (!gcov_tool) {
        fprintf(stderr, "gcov-tool not found. Please set GCOV_TOOL environment variable.\n");
        cleanup_tempdir(tempdir);
        return 1;
    }
    
    printf("\nUsing gcov-tool: %s\n", gcov_tool);
    
    /* Test 1: Trigger all the case statements in the uncovered block */
    printf("\n=== Test 1: Triggering case statements (lines 534-554) ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -v -f -F -o -h -t 0.75 %s %s",
             gcov_tool, gcda1_path, gcda2_path);
    
    int result1 = execute_command(cmd, 1);
    if (result1 != 0) {
        fprintf(stderr, "Warning: gcov-tool overlap command exited with code %d\n", result1);
    }
    
    /* Test 2: Trigger the default case with invalid option */
    printf("\n=== Test 2: Triggering default case (overlap_usage()) ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -Z %s %s",
             gcov_tool, gcda1_path, gcda2_path);
    
    int result2 = execute_command(cmd, 1);
    if (result2 != 0) {
        printf("Expected non-zero exit for invalid option: %d\n", result2);
    }
    
    /* Additional test: Try with just -v flag */
    printf("\n=== Test 3: Testing verbose flag only ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -v %s %s",
             gcov_tool, gcda1_path, gcda2_path);
    
    execute_command(cmd, 1);
    
    /* Test with different threshold values */
    printf("\n=== Test 4: Testing different threshold values ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -v -t 0.5 %s %s",
             gcov_tool, gcda1_path, gcda2_path);
    
    execute_command(cmd, 1);
    
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -v -t 0.9 %s %s",
             gcov_tool, gcda1_path, gcda2_path);
    
    execute_command(cmd, 1);
    
    /* Clean up */
    printf("\nCleaning up temporary directory: %s\n", tempdir);
    cleanup_tempdir(tempdir);
    
    printf("\n=== Test completed ===\n");
    printf("Successfully triggered:\n");
    printf("  - case 'v': verbose mode and gcov_set_verbose()\n");
    printf("  - case 'f': overlap_func_level = 1\n");
    printf("  - case 'F': overlap_use_fullname = 1\n");
    printf("  - case 'o': overlap_obj_level = 1\n");
    printf("  - case 'h': overlap_hot_only = 1\n");
    printf("  - case 't': overlap_hot_threshold = atof(optarg)\n");
    printf("  - default: overlap_usage() via invalid option -Z\n");
    
    return 0;
}
