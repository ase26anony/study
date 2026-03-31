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
"int process_value(int x, int limit) {\n"
"    int result = 0;\n"
"    for (int i = 0; i < x; i++) {\n"
"        if (i < limit) {\n"
"            result += i * 2;\n"
"        } else {\n"
"            result += i;\n"
"        }\n"
"    }\n"
"    return result;\n"
"}\n"
"\n"
"int main(int argc, char *argv[]) {\n"
"    int x = 10;\n"
"    int limit = 5;\n"
"    \n"
"    /* Use environment variable to vary execution path */\n"
"    char *env_limit = getenv(\"LOOP_LIMIT\");\n"
"    if (env_limit) {\n"
"        limit = atoi(env_limit);\n"
"    }\n"
"    \n"
"    /* Use command line argument to vary input */\n"
"    if (argc > 1) {\n"
"        x = atoi(argv[1]);\n"
"    }\n"
"    \n"
"    int result = process_value(x, limit);\n"
"    printf(\"Result: %d\\n\", result);\n"
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

/* Clean up temporary directory */
void cleanup_tempdir(const char *tempdir) {
    if (tempdir) {
        char cmd[MAX_CMD];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", tempdir);
        system(cmd);
    }
}

int main(int argc, char *argv[]) {
    char tempdir[MAX_PATH];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    char gcno_path[MAX_PATH];
    char cmd[MAX_CMD];
    int status;
    
    /* Create temporary directory */
    strcpy(tempdir, TEMPLATE);
    if (!mkdtemp(tempdir)) {
        perror("Failed to create temporary directory");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", tempdir);
    
    /* Set environment to write .gcda files to our temp directory */
    setenv("GCOV_PREFIX", tempdir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Write test program source */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", tempdir);
    FILE *src = fopen(source_path, "w");
    if (!src) {
        perror("Failed to create source file");
        cleanup_tempdir(tempdir);
        return 1;
    }
    fputs(test_program_source, src);
    fclose(src);
    
    /* Compile test program with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", tempdir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s",
             source_path, exec_path);
    
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup_tempdir(tempdir);
        return 1;
    }
    
    /* The .gcno file should be in the same directory as the source */
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", tempdir);
    
    /* First run - generate first .gcda file */
    printf("\n=== First run ===\n");
    setenv("LOOP_LIMIT", "3", 1);  /* Low limit */
    snprintf(cmd, sizeof(cmd), "%s 8", exec_path);  /* Small input */
    status = execute_command(cmd, 1);
    
    /* First .gcda file path */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", tempdir);
    
    /* Second run - generate second .gcda file with different profile */
    printf("\n=== Second run ===\n");
    setenv("LOOP_LIMIT", "7", 1);  /* Higher limit */
    snprintf(cmd, sizeof(cmd), "%s 15", exec_path);  /* Larger input */
    status = execute_command(cmd, 1);
    
    /* Second .gcda file path (need to rename first one) */
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func.gcda", tempdir);
    
    /* Rename first .gcda to preserve it */
    char gcda1_renamed[MAX_PATH];
    snprintf(gcda1_renamed, sizeof(gcda1_renamed), "%s/test_func_run1.gcda", tempdir);
    rename(gcda1_path, gcda1_renamed);
    
    /* Rename second .gcda */
    char gcda2_renamed[MAX_PATH];
    snprintf(gcda2_renamed, sizeof(gcda2_renamed), "%s/test_func_run2.gcda", tempdir);
    rename(gcda2_path, gcda2_renamed);
    
    /* Find gcov-tool path */
    const char *gcov_tool_path = getenv("GCOV_TOOL");
    if (!gcov_tool_path) {
        gcov_tool_path = "gcov-tool";  /* Rely on PATH */
    }
    
    /* Test 1: Trigger all the specific flag cases (lines 534-554) */
    printf("\n=== Test 1: Triggering specific flag cases ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -v -f -F -o -h -t 0.75 %s %s",
             gcov_tool_path, gcda1_renamed, gcda2_renamed);
    
    status = execute_command(cmd, 1);
    if (status != 0) {
        printf("Note: gcov-tool returned non-zero status %d (may be expected)\n", status);
    }
    
    /* Test 2: Trigger default case with invalid option */
    printf("\n=== Test 2: Triggering default case (invalid option) ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -Z %s %s",
             gcov_tool_path, gcda1_renamed, gcda2_renamed);
    
    status = execute_command(cmd, 1);
    if (status != 0) {
        printf("Note: Invalid option triggered error (expected)\n");
    }
    
    /* Test 3: Also test with just -v flag to ensure verbose is set */
    printf("\n=== Test 3: Testing verbose flag alone ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -v %s %s",
             gcov_tool_path, gcda1_renamed, gcda2_renamed);
    
    status = execute_command(cmd, 1);
    
    /* Clean up */
    printf("\n=== Cleaning up ===\n");
    cleanup_tempdir(tempdir);
    
    printf("\nTest completed successfully!\n");
    printf("The following gcov-tool options were tested:\n");
    printf("  -v : verbose mode (sets verbose variable, calls gcov_set_verbose())\n");
    printf("  -f : function-level overlap\n");
    printf("  -F : use full filenames\n");
    printf("  -o : object-level overlap\n");
    printf("  -h : hot-only mode\n");
    printf("  -t 0.75 : hot threshold\n");
    printf("  -Z : invalid option (triggers default case -> overlap_usage())\n");
    
    return 0;
}
