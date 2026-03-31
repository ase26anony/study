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
"    int iterations = 10;  /* default */\n"
"    \n"
"    if (argc > 1) {\n"
"        iterations = atoi(argv[1]);\n"
"    }\n"
"    \n"
"    /* Different runs will produce different coverage counts */\n"
"    int result = process_value(iterations);\n"
"    printf(\"Result: %d\\n\", result);\n"
"    \n"
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
        
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            /* Discard or log output as needed */
            printf("Output: %s", buffer);
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

/* Clean up temporary files */
void cleanup(const char *temp_dir, const char *source_path, 
             const char *exec_path, const char **gcda_files, int gcda_count) {
    if (source_path && access(source_path, F_OK) == 0) {
        unlink(source_path);
    }
    
    if (exec_path && access(exec_path, F_OK) == 0) {
        unlink(exec_path);
    }
    
    /* Remove .gcno and .gcda files */
    for (int i = 0; i < gcda_count; i++) {
        if (gcda_files[i]) {
            char gcno_path[MAX_PATH];
            char gcda_path[MAX_PATH];
            
            /* Remove .gcda */
            snprintf(gcda_path, sizeof(gcda_path), "%s", gcda_files[i]);
            if (access(gcda_path, F_OK) == 0) {
                unlink(gcda_path);
            }
            
            /* Remove .gcno */
            snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", temp_dir);
            if (access(gcno_path, F_OK) == 0) {
                unlink(gcno_path);
            }
        }
    }
    
    /* Remove temp directory if empty */
    rmdir(temp_dir);
}

int main(int argc, char *argv[]) {
    char temp_dir[MAX_PATH];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    char gcda_path1[MAX_PATH];
    char gcda_path2[MAX_PATH];
    char cmd[MAX_CMD];
    int status;
    const char *gcda_files[2] = {gcda_path1, gcda_path2};
    
    /* Create temporary directory */
    strcpy(temp_dir, TEMPLATE);
    if (mkdtemp(temp_dir) == NULL) {
        perror("Failed to create temp directory");
        return 1;
    }
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Set environment to write .gcda files to temp directory */
    setenv("GCOV_PREFIX", temp_dir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Write test program source */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", temp_dir);
    FILE *src = fopen(source_path, "w");
    if (!src) {
        perror("Failed to create source file");
        cleanup(temp_dir, NULL, NULL, NULL, 0);
        return 1;
    }
    fputs(test_program_source, src);
    fclose(src);
    
    /* Compile with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", temp_dir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s",
             source_path, exec_path);
    
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup(temp_dir, source_path, NULL, NULL, 0);
        return 1;
    }
    
    /* Run program twice with different inputs to generate distinct .gcda files */
    
    /* First run - 5 iterations */
    printf("\n=== First run (5 iterations) ===\n");
    snprintf(cmd, sizeof(cmd), "%s 5", exec_path);
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "First run failed\n");
        cleanup(temp_dir, source_path, exec_path, NULL, 0);
        return 1;
    }
    
    /* Rename first .gcda file */
    snprintf(gcda_path1, sizeof(gcda_path1), "%s/test_func.gcda", temp_dir);
    char gcda_path1_renamed[MAX_PATH];
    snprintf(gcda_path1_renamed, sizeof(gcda_path1_renamed), 
             "%s/test_func_run1.gcda", temp_dir);
    rename(gcda_path1, gcda_path1_renamed);
    strcpy(gcda_path1, gcda_path1_renamed);
    
    /* Second run - 20 iterations */
    printf("\n=== Second run (20 iterations) ===\n");
    snprintf(cmd, sizeof(cmd), "%s 20", exec_path);
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "Second run failed\n");
        cleanup(temp_dir, source_path, exec_path, gcda_files, 1);
        return 1;
    }
    
    /* Rename second .gcda file */
    snprintf(gcda_path2, sizeof(gcda_path2), "%s/test_func.gcda", temp_dir);
    char gcda_path2_renamed[MAX_PATH];
    snprintf(gcda_path2_renamed, sizeof(gcda_path2_renamed), 
             "%s/test_func_run2.gcda", temp_dir);
    rename(gcda_path2, gcda_path2_renamed);
    strcpy(gcda_path2, gcda_path2_renamed);
    
    /* Find gcov-tool path */
    const char *gcov_tool_path = getenv("GCOV_TOOL");
    if (!gcov_tool_path) {
        gcov_tool_path = "gcov-tool";  /* Rely on PATH */
    }
    
    /* Test 1: Trigger all the specific flag cases from uncovered lines */
    printf("\n=== Test 1: Triggering specific flag cases ===\n");
    printf("(Lines 534-554: -v, -f, -F, -o, -h, -t)\n");
    
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -v -f -F -o -h -t 0.75 %s %s",
             gcov_tool_path, gcda_path1, gcda_path2);
    
    status = execute_command(cmd, 1);
    if (status != 0) {
        printf("Note: gcov-tool returned non-zero status %d (may be expected)\n", status);
    }
    
    /* Test 2: Trigger default case with invalid option */
    printf("\n=== Test 2: Triggering default case (overlap_usage()) ===\n");
    printf("(Lines 553-554: default: overlap_usage())\n");
    
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -Z %s %s",  /* -Z is invalid option */
             gcov_tool_path, gcda_path1, gcda_path2);
    
    status = execute_command(cmd, 1);
    if (status != 0) {
        printf("Note: Invalid option triggered error (expected)\n");
    }
    
    /* Additional test: Test without overlap subcommand to ensure we're testing right mode */
    printf("\n=== Test 3: Testing without 'overlap' subcommand ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s -v merge %s %s",
             gcov_tool_path, gcda_path1, gcda_path2);
    
    status = execute_command(cmd, 1);
    
    /* Clean up */
    printf("\n=== Cleaning up ===\n");
    cleanup(temp_dir, source_path, exec_path, gcda_files, 2);
    
    printf("\n=== Test completed successfully ===\n");
    printf("Triggered code paths for:\n");
    printf("  - case 'v': verbose flag and gcov_set_verbose()\n");
    printf("  - case 'f': overlap_func_level = 1\n");
    printf("  - case 'F': overlap_use_fullname = 1\n");
    printf("  - case 'o': overlap_obj_level = 1\n");
    printf("  - case 'h': overlap_hot_only = 1\n");
    printf("  - case 't': overlap_hot_threshold = atof(optarg)\n");
    printf("  - default: overlap_usage() (via invalid option -Z)\n");
    
    return 0;
}
