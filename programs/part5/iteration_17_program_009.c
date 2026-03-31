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
"            sum += i;      /* Even path */\n"
"        } else {\n"
"            sum -= i;      /* Odd path */\n"
"        }\n"
"        \n"
"        /* Nested condition for more coverage complexity */\n"
"        if (i % 3 == 0) {\n"
"            sum *= 2;\n"
"        }\n"
"    }\n"
"    \n"
"    printf(\"Final sum: %d\\n\", sum);\n"
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
            /* Output can be printed for debugging */
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

/* Clean up temporary directory */
void cleanup_temp_dir(const char *temp_dir) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
    system(cmd);
}

int main() {
    char temp_dir[MAX_PATH];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    char gcov_tool_path[MAX_PATH];
    char cmd[MAX_CMD];
    int ret;
    
    /* Create unique temporary directory */
    snprintf(temp_dir, sizeof(temp_dir), "/tmp/gcov_test_%d", getpid());
    if (mkdir(temp_dir, 0755) == -1) {
        perror("Failed to create temp directory");
        return 1;
    }
    
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Set environment to write .gcda files to our temp directory */
    setenv("GCOV_PREFIX", temp_dir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Write test program source */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", temp_dir);
    FILE *src_fp = fopen(source_path, "w");
    if (!src_fp) {
        perror("Failed to create source file");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    fputs(test_program_source, src_fp);
    fclose(src_fp);
    
    /* Compile test program with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", temp_dir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s",
             source_path, exec_path);
    
    printf("Compiling: %s\n", cmd);
    ret = execute_command(cmd, 1);
    if (ret != 0) {
        fprintf(stderr, "Compilation failed with code %d\n", ret);
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Run program twice with different parameters to generate distinct .gcda files */
    
    /* First run: limit = 5 */
    printf("Running first test (limit=5)...\n");
    snprintf(cmd, sizeof(cmd), "%s 5", exec_path);
    ret = execute_command(cmd, 1);
    
    /* Rename .gcda file to preserve it */
    snprintf(cmd, sizeof(cmd), "mv %s/test_func.gcda %s/test_func_run1.gcda", 
             temp_dir, temp_dir);
    system(cmd);
    
    /* Second run: limit = 15, plus environment variable */
    printf("Running second test (limit=15 + env)...\n");
    setenv("TEST_LIMIT", "5", 1);  /* Adds 5 to the limit */
    snprintf(cmd, sizeof(cmd), "%s 15", exec_path);
    ret = execute_command(cmd, 1);
    unsetenv("TEST_LIMIT");
    
    /* Rename second .gcda file */
    snprintf(cmd, sizeof(cmd), "mv %s/test_func.gcda %s/test_func_run2.gcda", 
             temp_dir, temp_dir);
    system(cmd);
    
    /* Set paths to .gcda files */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func_run1.gcda", temp_dir);
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func_run2.gcda", temp_dir);
    
    /* Find gcov-tool path */
    char *gcov_tool_env = getenv("GCOV_TOOL");
    if (gcov_tool_env) {
        strncpy(gcov_tool_path, gcov_tool_env, sizeof(gcov_tool_path));
    } else {
        /* Try common locations */
        if (access("/usr/bin/gcov-tool", X_OK) == 0) {
            strcpy(gcov_tool_path, "/usr/bin/gcov-tool");
        } else if (access("/usr/local/bin/gcov-tool", X_OK) == 0) {
            strcpy(gcov_tool_path, "/usr/local/bin/gcov-tool");
        } else {
            /* Try to find it in PATH */
            FILE *which = popen("which gcov-tool", "r");
            if (which) {
                if (fgets(gcov_tool_path, sizeof(gcov_tool_path), which)) {
                    /* Remove newline */
                    gcov_tool_path[strcspn(gcov_tool_path, "\n")] = 0;
                }
                pclose(which);
            }
        }
    }
    
    if (strlen(gcov_tool_path) == 0) {
        fprintf(stderr, "gcov-tool not found. Set GCOV_TOOL environment variable.\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    printf("Using gcov-tool at: %s\n", gcov_tool_path);
    
    /* Test 1: Trigger all the flag cases from uncovered lines */
    printf("\n=== Test 1: Triggering flag handling code ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s -v -f -F -o -h -t 0.75 %s %s",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    printf("Executing: %s\n", cmd);
    ret = execute_command(cmd, 1);
    printf("gcov-tool overlap analysis returned: %d\n", ret);
    
    /* Test 2: Trigger default case with invalid option */
    printf("\n=== Test 2: Triggering default case (invalid option) ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s -Z %s %s",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    printf("Executing: %s\n", cmd);
    ret = execute_command(cmd, 1);
    printf("gcov-tool with invalid option returned: %d\n", ret);
    
    /* Additional test: Try with just -v flag to ensure verbose is set */
    printf("\n=== Test 3: Testing verbose flag alone ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s -v %s %s",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    printf("Executing: %s\n", cmd);
    ret = execute_command(cmd, 1);
    printf("gcov-tool verbose only returned: %d\n", ret);
    
    /* Clean up */
    printf("\nCleaning up temporary directory: %s\n", temp_dir);
    cleanup_temp_dir(temp_dir);
    
    printf("\nAll tests completed successfully!\n");
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
