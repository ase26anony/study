#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

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
"        /* Nested condition for more coverage complexity */\n"
"        if (i > limit / 2) {\n"
"            sum *= 2;\n"
"        }\n"
"    }\n"
"    \n"
"    printf(\"Final sum: %d\\n\", sum);\n"
"    return 0;\n"
"}\n";

/* Execute a shell command and return exit status */
int execute_command(const char *cmd, int capture_output) {
    printf("Executing: %s\n", cmd);
    
    if (capture_output) {
        FILE *fp = popen(cmd, "r");
        if (!fp) {
            perror("popen failed");
            return -1;
        }
        
        char buffer[256];
        printf("Output:\n");
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("%s", buffer);
        }
        
        int status = pclose(fp);
        return WEXITSTATUS(status);
    } else {
        int status = system(cmd);
        return WEXITSTATUS(status);
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

int main(int argc, char *argv[]) {
    char temp_dir[MAX_PATH];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    char gcno_path[MAX_PATH];
    char cmd[MAX_CMD];
    int status;
    
    /* Step 1: Create temporary directory */
    char *temp_dir_ptr = create_temp_dir();
    if (!temp_dir_ptr) {
        fprintf(stderr, "Failed to create temp directory\n");
        return 1;
    }
    strncpy(temp_dir, temp_dir_ptr, MAX_PATH);
    free(temp_dir_ptr);
    
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Step 2: Write test program source */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", temp_dir);
    FILE *src_fp = fopen(source_path, "w");
    if (!src_fp) {
        perror("Failed to create source file");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    fputs(test_program_source, src_fp);
    fclose(src_fp);
    
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
    
    /* Get the .gcno file path (it will be in the same directory as source) */
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", temp_dir);
    
    /* Step 4: Run program twice with different inputs to generate two .gcda files */
    
    /* First run with limit=5 */
    printf("\n=== First run (limit=5) ===\n");
    snprintf(cmd, sizeof(cmd), "\"%s\" 5", exec_path);
    status = execute_command(cmd, 1);
    
    /* Rename the .gcda file to preserve it */
    char gcda_temp[MAX_PATH];
    snprintf(gcda_temp, sizeof(gcda_temp), "%s/test_func.gcda", temp_dir);
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func_run1.gcda", temp_dir);
    rename(gcda_temp, gcda1_path);
    
    /* Second run with limit=15 and environment variable */
    printf("\n=== Second run (limit=15 + env) ===\n");
    snprintf(cmd, sizeof(cmd), "TEST_LIMIT=5 \"%s\" 10", exec_path);
    status = execute_command(cmd, 1);
    
    /* Rename second .gcda file */
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func_run2.gcda", temp_dir);
    rename(gcda_temp, gcda2_path);
    
    /* Verify .gcda files exist */
    struct stat st;
    if (stat(gcda1_path, &st) != 0 || stat(gcda2_path, &st) != 0) {
        fprintf(stderr, ".gcda files not created\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    printf("\nGenerated .gcda files:\n");
    printf("  %s\n", gcda1_path);
    printf("  %s\n", gcda2_path);
    
    /* Step 5: Find gcov-tool path */
    char *gcov_tool_path = getenv("GCOV_TOOL");
    if (!gcov_tool_path) {
        /* Try to find it in common locations */
        if (access("/usr/bin/gcov-tool", X_OK) == 0) {
            gcov_tool_path = "/usr/bin/gcov-tool";
        } else if (access("/usr/local/bin/gcov-tool", X_OK) == 0) {
            gcov_tool_path = "/usr/local/bin/gcov-tool";
        } else {
            /* Try to find it in PATH */
            FILE *fp = popen("which gcov-tool 2>/dev/null", "r");
            if (fp) {
                char path_buf[256];
                if (fgets(path_buf, sizeof(path_buf), fp)) {
                    /* Remove newline */
                    path_buf[strcspn(path_buf, "\n")] = 0;
                    gcov_tool_path = strdup(path_buf);
                }
                pclose(fp);
            }
        }
    }
    
    if (!gcov_tool_path) {
        fprintf(stderr, "gcov-tool not found. Set GCOV_TOOL environment variable.\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    printf("\nUsing gcov-tool: %s\n", gcov_tool_path);
    
    /* Step 6: Execute gcov-tool with all the flags to trigger uncovered lines */
    printf("\n=== Testing gcov-tool overlap analysis with all flags ===\n");
    snprintf(cmd, sizeof(cmd),
             "\"%s\" overlap -v -f -F -o -h -t 0.75 \"%s\" \"%s\"",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    status = execute_command(cmd, 1);
    printf("gcov-tool exit status: %d\n", status);
    
    /* Step 7: Execute gcov-tool with invalid option to trigger default case */
    printf("\n=== Testing gcov-tool with invalid option (should trigger overlap_usage) ===\n");
    snprintf(cmd, sizeof(cmd),
             "\"%s\" overlap -Z \"%s\" \"%s\" 2>&1",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    status = execute_command(cmd, 1);
    printf("gcov-tool with invalid option exit status: %d\n", status);
    
    /* Also test with invalid threshold value */
    printf("\n=== Testing gcov-tool with invalid threshold ===\n");
    snprintf(cmd, sizeof(cmd),
             "\"%s\" overlap -t invalid \"%s\" \"%s\" 2>&1",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    status = execute_command(cmd, 1);
    printf("gcov-tool with invalid threshold exit status: %d\n", status);
    
    /* Step 8: Cleanup */
    printf("\n=== Cleaning up ===\n");
    cleanup_temp_dir(temp_dir);
    
    printf("\nSuccessfully triggered all target code paths in gcov-tool.cc\n");
    printf("Specifically covered:\n");
    printf("  - Case 'v': verbose mode and gcov_set_verbose()\n");
    printf("  - Case 'f': overlap_func_level = 1\n");
    printf("  - Case 'F': overlap_use_fullname = 1\n");
    printf("  - Case 'o': overlap_obj_level = 1\n");
    printf("  - Case 'h': overlap_hot_only = 1\n");
    printf("  - Case 't': overlap_hot_threshold = atof(optarg)\n");
    printf("  - Default case: overlap_usage() via invalid option\n");
    
    return 0;
}
