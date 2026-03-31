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
#define MAX_CMD 4096

/* Simple test program that will be compiled with coverage instrumentation */
const char *test_program_source = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"\n"
"int main(int argc, char **argv) {\n"
"    int i, limit = 10;\n"
"    int sum = 0;\n"
"    \n"
"    /* Different behavior based on environment variable */\n"
"    char *env_limit = getenv(\"TEST_LIMIT\");\n"
"    if (env_limit) {\n"
"        limit = atoi(env_limit);\n"
"    }\n"
"    \n"
"    /* Different behavior based on command line argument */\n"
"    if (argc > 1) {\n"
"        limit = atoi(argv[1]);\n"
"    }\n"
"    \n"
"    for (i = 0; i < limit; i++) {\n"
"        if (i % 2 == 0) {\n"
"            sum += i;\n"
"        } else {\n"
"            sum -= i;\n"
"        }\n"
"    }\n"
"    \n"
"    printf(\"Sum: %d\\n\", sum);\n"
"    return 0;\n"
"}\n";

/* Execute a shell command and return its exit status */
int execute_command(const char *cmd, int capture_output) {
    printf("Executing: %s\n", cmd);
    
    if (capture_output) {
        FILE *fp = popen(cmd, "r");
        if (!fp) {
            perror("popen failed");
            return -1;
        }
        
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            /* Discard or log output */
            printf("  Output: %s", buffer);
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

/* Create a temporary directory */
char *create_temp_dir() {
    char *template = strdup("/tmp/gcov_test_XXXXXX");
    char *dir = mkdtemp(template);
    if (!dir) {
        perror("Failed to create temp directory");
        free(template);
        return NULL;
    }
    return dir;
}

/* Clean up temporary directory */
void cleanup_temp_dir(const char *dir) {
    if (!dir) return;
    
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", dir);
    system(cmd);
}

int main(int argc, char **argv) {
    char temp_dir[MAX_PATH];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    char cmd[MAX_CMD];
    int status;
    
    /* Step 1: Create temporary directory */
    char *temp_dir_ptr = create_temp_dir();
    if (!temp_dir_ptr) {
        fprintf(stderr, "Failed to create temporary directory\n");
        return 1;
    }
    strncpy(temp_dir, temp_dir_ptr, sizeof(temp_dir));
    free(temp_dir_ptr);
    
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Set environment to write .gcda files to our temp directory */
    setenv("GCOV_PREFIX", temp_dir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Step 2: Write test program source */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", temp_dir);
    FILE *src = fopen(source_path, "w");
    if (!src) {
        perror("Failed to create source file");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    fputs(test_program_source, src);
    fclose(src);
    
    /* Step 3: Compile with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", temp_dir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g \"%s\" -o \"%s\"",
             source_path, exec_path);
    
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* The .gcno file should be in the same directory as the source */
    char gcno_path[MAX_PATH];
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", temp_dir);
    
    /* Step 4: Run program twice to generate different .gcda files */
    
    /* First run with limit=5 */
    setenv("TEST_LIMIT", "5", 1);
    snprintf(cmd, sizeof(cmd), "\"%s\"", exec_path);
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "First run failed\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Rename the .gcda file to preserve it */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", temp_dir);
    char gcda1_saved[MAX_PATH];
    snprintf(gcda1_saved, sizeof(gcda1_saved), "%s/test_func_run1.gcda", temp_dir);
    rename(gcda1_path, gcda1_saved);
    
    /* Second run with limit=15 */
    setenv("TEST_LIMIT", "15", 1);
    snprintf(cmd, sizeof(cmd), "\"%s\" 15", exec_path);  /* Also pass as argument */
    status = execute_command(cmd, 1);
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
    
    /* Step 5: Find gcov-tool path */
    char gcov_tool_path[MAX_PATH] = "/usr/bin/gcov-tool";
    char *env_gcov_tool = getenv("GCOV_TOOL");
    if (env_gcov_tool && strlen(env_gcov_tool) > 0) {
        strncpy(gcov_tool_path, env_gcov_tool, sizeof(gcov_tool_path));
    }
    
    /* Check if gcov-tool exists */
    if (access(gcov_tool_path, X_OK) != 0) {
        fprintf(stderr, "gcov-tool not found at %s\n", gcov_tool_path);
        fprintf(stderr, "Set GCOV_TOOL environment variable to correct path\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Step 6: Execute gcov-tool with all the flags to trigger uncovered lines */
    printf("\n=== Running gcov-tool with overlap analysis flags ===\n");
    snprintf(cmd, sizeof(cmd),
             "\"%s\" overlap -v -f -F -o -h -t 0.75 \"%s\" \"%s\"",
             gcov_tool_path, gcda1_saved, gcda2_saved);
    
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "gcov-tool overlap analysis failed with status %d\n", status);
        /* Continue anyway to try invalid option */
    }
    
    /* Step 7: Execute gcov-tool with invalid option to trigger default case */
    printf("\n=== Running gcov-tool with invalid option (to trigger default case) ===\n");
    snprintf(cmd, sizeof(cmd),
             "\"%s\" overlap -Z \"%s\" \"%s\"",
             gcov_tool_path, gcda1_saved, gcda2_saved);
    
    status = execute_command(cmd, 1);
    /* We expect this to fail due to invalid option */
    if (status == 0) {
        fprintf(stderr, "WARNING: Invalid option -Z did not cause failure\n");
    } else {
        printf("Successfully triggered invalid option handling (status: %d)\n", status);
    }
    
    /* Also test with completely invalid mode */
    printf("\n=== Running gcov-tool with invalid mode ===\n");
    snprintf(cmd, sizeof(cmd),
             "\"%s\" invalidmode -v",
             gcov_tool_path);
    
    status = execute_command(cmd, 1);
    
    /* Step 8: Cleanup */
    printf("\n=== Cleaning up ===\n");
    cleanup_temp_dir(temp_dir);
    
    printf("\nTest completed successfully!\n");
    printf("The following gcov-tool code paths should have been triggered:\n");
    printf("  - case 'v': verbose = true; gcov_set_verbose();\n");
    printf("  - case 'f': overlap_func_level = 1;\n");
    printf("  - case 'F': overlap_use_fullname = 1;\n");
    printf("  - case 'o': overlap_obj_level = 1;\n");
    printf("  - case 'h': overlap_hot_only = 1;\n");
    printf("  - case 't': overlap_hot_threshold = atof(optarg);\n");
    printf("  - default: overlap_usage();\n");
    
    return 0;
}
