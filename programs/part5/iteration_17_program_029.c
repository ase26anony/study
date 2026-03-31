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

/* Simple test program source code that will be compiled with coverage */
const char *test_program_source = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"\n"
"int process_value(int x, int limit) {\n"
"    int result = 0;\n"
"    for (int i = 0; i < x; i++) {\n"
"        if (i % 2 == 0) {\n"
"            result += i;\n"
"        } else {\n"
"            result -= i;\n"
"        }\n"
"        if (result > limit) {\n"
"            result = limit;\n"
"        }\n"
"    }\n"
"    return result;\n"
"}\n"
"\n"
"int main(int argc, char *argv[]) {\n"
"    int value = 10;\n"
"    int limit = 50;\n"
"    \n"
"    /* Use environment variable to vary execution path */\n"
"    char *env_value = getenv(\"TEST_VALUE\");\n"
"    if (env_value) {\n"
"        value = atoi(env_value);\n"
"    }\n"
"    \n"
"    /* Or use command line argument */\n"
"    if (argc > 1) {\n"
"        limit = atoi(argv[1]);\n"
"    }\n"
"    \n"
"    int result = process_value(value, limit);\n"
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
        printf("Output:\n");
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("%s", buffer);
        }
        
        int status = pclose(fp);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    } else {
        int status = system(cmd);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    }
}

/* Clean up temporary directory */
void cleanup_temp_dir(const char *temp_dir) {
    if (temp_dir) {
        char cmd[MAX_CMD];
        snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", temp_dir);
        system(cmd);
    }
}

int main(int argc, char *argv[]) {
    char temp_dir[MAX_PATH];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    char gcno_path[MAX_PATH];
    char cmd[MAX_CMD];
    int ret = 0;
    
    /* Create temporary directory */
    strcpy(temp_dir, TEMP_DIR_TEMPLATE);
    if (mkdtemp(temp_dir) == NULL) {
        perror("Failed to create temporary directory");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", temp_dir);
    
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
             "gcc -fprofile-arcs -ftest-coverage -O0 -g \"%s\" -o \"%s\"",
             source_path, exec_path);
    
    if (execute_command(cmd, 1) != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Get path to .gcno file (created during compilation) */
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", temp_dir);
    
    /* First run with different parameters to generate first .gcda */
    printf("\n=== First run ===\n");
    setenv("TEST_VALUE", "20", 1);
    snprintf(cmd, sizeof(cmd), "\"%s\" 30", exec_path);
    if (execute_command(cmd, 1) != 0) {
        fprintf(stderr, "First run failed\n");
    }
    
    /* Rename the generated .gcda file */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", temp_dir);
    char gcda1_final[MAX_PATH];
    snprintf(gcda1_final, sizeof(gcda1_final), "%s/test_func_run1.gcda", temp_dir);
    rename(gcda1_path, gcda1_final);
    
    /* Second run with different parameters to generate second .gcda */
    printf("\n=== Second run ===\n");
    setenv("TEST_VALUE", "15", 1);
    snprintf(cmd, sizeof(cmd), "\"%s\" 100", exec_path);
    if (execute_command(cmd, 1) != 0) {
        fprintf(stderr, "Second run failed\n");
    }
    
    /* Rename the second .gcda file */
    char gcda2_final[MAX_PATH];
    snprintf(gcda2_final, sizeof(gcda2_final), "%s/test_func_run2.gcda", temp_dir);
    rename(gcda1_path, gcda2_final);
    
    /* Find gcov-tool path */
    char gcov_tool_path[MAX_PATH] = "/usr/bin/gcov-tool";
    char *env_gcov_tool = getenv("GCOV_TOOL");
    if (env_gcov_tool && strlen(env_gcov_tool) > 0) {
        strcpy(gcov_tool_path, env_gcov_tool);
    }
    
    printf("\n=== Testing gcov-tool overlap analysis with all flags ===\n");
    
    /* Test 1: Trigger all the case statements in the uncovered block */
    snprintf(cmd, sizeof(cmd),
             "\"%s\" overlap -v -f -F -o -h -t 0.75 \"%s\" \"%s\"",
             gcov_tool_path, gcda1_final, gcda2_final);
    
    printf("Running gcov-tool with: -v -f -F -o -h -t 0.75\n");
    int status1 = execute_command(cmd, 1);
    
    if (status1 != 0) {
        printf("Note: gcov-tool returned non-zero status %d (may be expected)\n", status1);
    }
    
    /* Test 2: Trigger the default case with invalid option */
    printf("\n=== Testing gcov-tool with invalid option (to trigger default case) ===\n");
    snprintf(cmd, sizeof(cmd),
             "\"%s\" overlap -Z \"%s\" \"%s\"",
             gcov_tool_path, gcda1_final, gcda2_final);
    
    printf("Running gcov-tool with invalid option -Z\n");
    int status2 = execute_command(cmd, 1);
    
    if (status2 != 0) {
        printf("Note: gcov-tool returned non-zero status %d (expected for invalid option)\n", status2);
    }
    
    /* Verify that the .gcno file exists (required for overlap analysis) */
    struct stat st;
    if (stat(gcno_path, &st) != 0) {
        printf("Warning: .gcno file not found at %s\n", gcno_path);
        printf("Creating a dummy .gcno file for testing...\n");
        
        /* Create a minimal .gcno file for testing */
        FILE *gcno_fp = fopen(gcno_path, "wb");
        if (gcno_fp) {
            /* Write minimal gcno header */
            unsigned int magic = 0x67636e6f; /* 'gcno' */
            unsigned int version = 0x3430392a; /* GCC 9* format */
            unsigned int stamp = 0x12345678;
            
            fwrite(&magic, sizeof(magic), 1, gcno_fp);
            fwrite(&version, sizeof(version), 1, gcno_fp);
            fwrite(&stamp, sizeof(stamp), 1, gcno_fp);
            fclose(gcno_fp);
            
            /* Run gcov-tool again with the dummy .gcno file */
            printf("\n=== Re-running gcov-tool with dummy .gcno file ===\n");
            snprintf(cmd, sizeof(cmd),
                     "\"%s\" overlap -v -f -F -o -h -t 0.5 \"%s\" \"%s\"",
                     gcov_tool_path, gcda1_final, gcda2_final);
            execute_command(cmd, 1);
        }
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Temporary directory: %s\n", temp_dir);
    printf("Generated files:\n");
    printf("  Source: %s\n", source_path);
    printf("  Executable: %s\n", exec_path);
    printf("  .gcno: %s\n", gcno_path);
    printf("  .gcda files: %s, %s\n", gcda1_final, gcda2_final);
    
    /* Ask user if they want to keep the files */
    printf("\nClean up temporary files? (y/n): ");
    char response[10];
    if (fgets(response, sizeof(response), stdin)) {
        if (response[0] == 'y' || response[0] == 'Y') {
            cleanup_temp_dir(temp_dir);
            printf("Cleaned up temporary directory.\n");
        } else {
            printf("Files kept in: %s\n", temp_dir);
            printf("You can manually inspect them or run:\n");
            printf("  %s overlap -v -f -F -o -h -t 0.5 %s %s\n",
                   gcov_tool_path, gcda1_final, gcda2_final);
        }
    }
    
    return ret;
}
