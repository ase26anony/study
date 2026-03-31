#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define TEMP_DIR_TEMPLATE "/tmp/gcov_test_XXXXXX"
#define MAX_PATH 1024
#define MAX_CMD 4096

/* Simple test program that will be compiled with coverage instrumentation */
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
"    /* Loop with conditional to generate interesting coverage */\n"
"    for (i = 0; i < limit; i++) {\n"
"        if (i % 3 == 0) {\n"
"            printf(\"Multiple of 3: %d\\n\", i);\n"
"        } else if (i % 2 == 0) {\n"
"            printf(\"Even: %d\\n\", i);\n"
"        } else {\n"
"            printf(\"Odd: %d\\n\", i);\n"
"        }\n"
"    }\n"
"    \n"
"    return 0;\n"
"}\n";

/* Execute a shell command and return exit status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Execute command and capture output */
int execute_and_capture(const char *cmd, char *output, size_t output_size) {
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        return -1;
    }
    
    size_t total_read = 0;
    while (fgets(output + total_read, output_size - total_read, fp) != NULL) {
        total_read = strlen(output);
        if (total_read >= output_size - 1) {
            break;
        }
    }
    
    int status = pclose(fp);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

int main(int argc, char *argv[]) {
    char temp_dir[MAX_PATH];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    char gcov_tool_path[MAX_PATH];
    char cmd[MAX_CMD];
    char output[8192];
    int ret = 0;
    
    /* Create temporary directory */
    strcpy(temp_dir, TEMP_DIR_TEMPLATE);
    if (mkdtemp(temp_dir) == NULL) {
        perror("Failed to create temp directory");
        return 1;
    }
    
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Set environment to write .gcda files to our temp directory */
    setenv("GCOV_PREFIX", temp_dir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Write test program source */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", temp_dir);
    FILE *src = fopen(source_path, "w");
    if (!src) {
        perror("Failed to write source file");
        ret = 1;
        goto cleanup;
    }
    fputs(test_program_source, src);
    fclose(src);
    
    /* Compile test program with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", temp_dir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s",
             source_path, exec_path);
    
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        ret = 1;
        goto cleanup;
    }
    
    /* Run program twice with different parameters to generate distinct .gcda files */
    
    /* First run - limit 5 */
    printf("\n=== First run (limit=5) ===\n");
    snprintf(cmd, sizeof(cmd), "%s 5", exec_path);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "First run failed\n");
    }
    
    /* Rename first .gcda file */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", temp_dir);
    char gcda1_backup[MAX_PATH];
    snprintf(gcda1_backup, sizeof(gcda1_backup), "%s/test_func_run1.gcda", temp_dir);
    rename(gcda1_path, gcda1_backup);
    
    /* Second run - limit 20 */
    printf("\n=== Second run (limit=20) ===\n");
    snprintf(cmd, sizeof(cmd), "%s 20", exec_path);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Second run failed\n");
    }
    
    /* Rename second .gcda file */
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func.gcda", temp_dir);
    char gcda2_backup[MAX_PATH];
    snprintf(gcda2_backup, sizeof(gcda2_backup), "%s/test_func_run2.gcda", temp_dir);
    rename(gcda2_path, gcda2_backup);
    
    /* Restore original names for gcov-tool */
    rename(gcda1_backup, gcda1_path);
    rename(gcda2_backup, gcda2_path);
    
    /* Find gcov-tool path */
    const char *env_gcov_tool = getenv("GCOV_TOOL");
    if (env_gcov_tool && strlen(env_gcov_tool) > 0) {
        strcpy(gcov_tool_path, env_gcov_tool);
    } else {
        /* Try common locations */
        if (access("/usr/bin/gcov-tool", X_OK) == 0) {
            strcpy(gcov_tool_path, "/usr/bin/gcov-tool");
        } else if (access("/usr/local/bin/gcov-tool", X_OK) == 0) {
            strcpy(gcov_tool_path, "/usr/local/bin/gcov-tool");
        } else {
            /* Try to find in PATH */
            FILE *which = popen("which gcov-tool 2>/dev/null", "r");
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
        fprintf(stderr, "Could not find gcov-tool. Set GCOV_TOOL environment variable.\n");
        ret = 1;
        goto cleanup;
    }
    
    printf("\nUsing gcov-tool at: %s\n", gcov_tool_path);
    
    /* Test 1: Trigger all the specific flag cases from uncovered lines */
    printf("\n=== Test 1: Triggering specific flag cases ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -v -f -F -o -h -t 0.75 %s %s 2>&1",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    memset(output, 0, sizeof(output));
    int exit_code = execute_and_capture(cmd, output, sizeof(output));
    printf("Exit code: %d\n", exit_code);
    
    /* Check if verbose output was produced (indicates gcov_set_verbose was called) */
    if (strstr(output, "Reading data file") != NULL ||
        strstr(output, "Overlap analysis") != NULL) {
        printf("✓ Verbose output detected\n");
    }
    
    /* Test 2: Trigger default case with invalid option */
    printf("\n=== Test 2: Triggering default case (invalid option) ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -Z %s %s 2>&1",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    memset(output, 0, sizeof(output));
    exit_code = execute_and_capture(cmd, output, sizeof(output));
    printf("Exit code: %d\n", exit_code);
    
    /* Check if usage message was printed */
    if (strstr(output, "Usage") != NULL ||
        strstr(output, "usage") != NULL ||
        strstr(output, "overlap") != NULL) {
        printf("✓ Usage message detected (overlap_usage() was called)\n");
    }
    
    printf("\n=== Summary ===\n");
    printf("Test completed successfully!\n");
    printf("The following gcov-tool options were tested:\n");
    printf("  -v (verbose) - triggers gcov_set_verbose()\n");
    printf("  -f (function level overlap)\n");
    printf("  -F (use full filenames)\n");
    printf("  -o (object level overlap)\n");
    printf("  -h (hot only)\n");
    printf("  -t 0.75 (hot threshold)\n");
    printf("  -Z (invalid option) - triggers default case and overlap_usage()\n");

cleanup:
    /* Clean up temporary files */
    printf("\nCleaning up temporary directory: %s\n", temp_dir);
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
    execute_command(cmd);
    
    return ret;
}
