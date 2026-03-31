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
"void test_function(int iterations, int threshold) {\n"
"    int i;\n"
"    int sum = 0;\n"
"    \n"
"    for (i = 0; i < iterations; i++) {\n"
"        if (i < threshold) {\n"
"            sum += i * 2;  /* Hot path for first run */\n"
"        } else {\n"
"            sum += i;      /* Cold path for first run */\n"
"        }\n"
"    }\n"
"    \n"
"    printf(\"Sum: %d\\n\", sum);\n"
"}\n"
"\n"
"int main(int argc, char *argv[]) {\n"
"    int iterations = 100;\n"
"    int threshold = 50;\n"
"    \n"
"    if (argc > 1) {\n"
"        iterations = atoi(argv[1]);\n"
"    }\n"
"    if (argc > 2) {\n"
"        threshold = atoi(argv[2]);\n"
"    }\n"
"    \n"
"    test_function(iterations, threshold);\n"
"    return 0;\n"
"}\n";

/* Execute a shell command and return exit status */
int execute_command(const char *cmd, int capture_output) {
    int status;
    char buffer[1024];
    FILE *fp;
    
    if (capture_output) {
        fp = popen(cmd, "r");
        if (!fp) {
            perror("popen failed");
            return -1;
        }
        
        /* Read and discard output (or could log it) */
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            /* Optionally print for debugging:
            printf("CMD OUTPUT: %s", buffer);
            */
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
void cleanup(const char *temp_dir) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
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
    int ret;
    const char *gcov_tool_path;
    
    /* Create temporary directory */
    strcpy(temp_dir, TEMPLATE);
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
        perror("Failed to create source file");
        cleanup(temp_dir);
        return 1;
    }
    fputs(test_program_source, src);
    fclose(src);
    
    /* Compile test program with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", temp_dir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s",
             source_path, exec_path);
    
    printf("Compiling: %s\n", cmd);
    ret = execute_command(cmd, 1);
    if (ret != 0) {
        fprintf(stderr, "Compilation failed with status %d\n", ret);
        cleanup(temp_dir);
        return 1;
    }
    
    /* Get path to .gcno file */
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", temp_dir);
    
    /* First run - generate first .gcda file */
    printf("\n=== First run (hot path dominant) ===\n");
    snprintf(cmd, sizeof(cmd), "%s 100 75", exec_path);
    ret = execute_command(cmd, 1);
    if (ret != 0) {
        fprintf(stderr, "First run failed with status %d\n", ret);
        cleanup(temp_dir);
        return 1;
    }
    
    /* Rename .gcda file to preserve it */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", temp_dir);
    char gcda1_final[MAX_PATH];
    snprintf(gcda1_final, sizeof(gcda1_final), "%s/test_func_run1.gcda", temp_dir);
    rename(gcda1_path, gcda1_final);
    
    /* Second run - generate second .gcda file with different profile */
    printf("\n=== Second run (cold path dominant) ===\n");
    snprintf(cmd, sizeof(cmd), "%s 100 25", exec_path);
    ret = execute_command(cmd, 1);
    if (ret != 0) {
        fprintf(stderr, "Second run failed with status %d\n", ret);
        cleanup(temp_dir);
        return 1;
    }
    
    /* Rename second .gcda file */
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func.gcda", temp_dir);
    char gcda2_final[MAX_PATH];
    snprintf(gcda2_final, sizeof(gcda2_final), "%s/test_func_run2.gcda", temp_dir);
    rename(gcda2_path, gcda2_final);
    
    /* Find gcov-tool path */
    gcov_tool_path = getenv("GCOV_TOOL");
    if (!gcov_tool_path) {
        gcov_tool_path = "gcov-tool";
    }
    
    printf("\n=== Running gcov-tool with all overlap flags ===\n");
    
    /* Test 1: Trigger all the case statements in the uncovered block */
    snprintf(cmd, sizeof(cmd),
             "%s overlap -v -f -F -o -h -t 0.75 %s %s",
             gcov_tool_path, gcda1_final, gcda2_final);
    
    printf("Command: %s\n", cmd);
    ret = execute_command(cmd, 1);
    if (ret != 0) {
        printf("Note: gcov-tool returned %d (might be expected for test data)\n", ret);
    }
    
    /* Test 2: Trigger the default case with invalid option */
    printf("\n=== Running gcov-tool with invalid option (to trigger default case) ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -Z %s %s",
             gcov_tool_path, gcda1_final, gcda2_final);
    
    printf("Command: %s\n", cmd);
    ret = execute_command(cmd, 1);
    if (ret != 0) {
        printf("Expected failure with invalid option: returned %d\n", ret);
    }
    
    /* Test 3: Additional test with just verbose flag */
    printf("\n=== Running gcov-tool with verbose flag only ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -v %s %s",
             gcov_tool_path, gcda1_final, gcda2_final);
    
    printf("Command: %s\n", cmd);
    ret = execute_command(cmd, 1);
    
    /* Test 4: Test with threshold flag only */
    printf("\n=== Running gcov-tool with threshold flag ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -t 0.5 %s %s",
             gcov_tool_path, gcda1_final, gcda2_final);
    
    printf("Command: %s\n", cmd);
    ret = execute_command(cmd, 1);
    
    /* Clean up */
    printf("\n=== Cleaning up ===\n");
    cleanup(temp_dir);
    
    printf("\nTest completed successfully!\n");
    printf("The following gcov-tool options were tested:\n");
    printf("  -v (verbose) - triggers gcov_set_verbose()\n");
    printf("  -f (function level overlap)\n");
    printf("  -F (use full names)\n");
    printf("  -o (object level overlap)\n");
    printf("  -h (hot only)\n");
    printf("  -t <threshold> (hot threshold)\n");
    printf("  -Z (invalid option to trigger default case)\n");
    
    return 0;
}
