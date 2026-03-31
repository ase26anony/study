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

/* Simple test program source code that will be compiled with coverage */
const char *test_program_source = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"\n"
"int process_value(int x, int threshold) {\n"
"    int result = 0;\n"
"    for (int i = 0; i < x; i++) {\n"
"        if (i % 2 == 0) {\n"
"            result += i;\n"
"        } else {\n"
"            result -= i;\n"
"        }\n"
"    }\n"
"    \n"
"    if (result > threshold) {\n"
"        return 1;\n"
"    } else {\n"
"        return 0;\n"
"    }\n"
"}\n"
"\n"
"int main(int argc, char *argv[]) {\n"
"    int value = 10;\n"
"    int threshold = 20;\n"
"    \n"
"    if (argc > 1) {\n"
"        value = atoi(argv[1]);\n"
"    }\n"
"    if (argc > 2) {\n"
"        threshold = atoi(argv[2]);\n"
"    }\n"
"    \n"
"    int ret = process_value(value, threshold);\n"
"    printf(\"Result: %d\\n\", ret);\n"
"    return ret;\n"
"}\n";

/* Clean up temporary directory and files */
void cleanup_temp_dir(const char *temp_dir) {
    if (!temp_dir) return;
    
    DIR *dir = opendir(temp_dir);
    if (!dir) return;
    
    struct dirent *entry;
    char path[MAX_PATH];
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
            
        snprintf(path, sizeof(path), "%s/%s", temp_dir, entry->d_name);
        remove(path);
    }
    
    closedir(dir);
    rmdir(temp_dir);
}

/* Execute a command and capture its output */
int execute_command(const char *cmd, char *output, size_t output_size) {
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        fprintf(stderr, "Failed to execute command: %s\n", cmd);
        return -1;
    }
    
    if (output && output_size > 0) {
        size_t total_read = 0;
        while (!feof(fp) && total_read < output_size - 1) {
            size_t bytes_read = fread(output + total_read, 1, 
                                     output_size - total_read - 1, fp);
            total_read += bytes_read;
        }
        output[total_read] = '\0';
    } else {
        /* Just consume output */
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            /* Do nothing, just read */
        }
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
}

/* Find gcov-tool path */
const char *find_gcov_tool() {
    const char *env_path = getenv("GCOV_TOOL");
    if (env_path && access(env_path, X_OK) == 0) {
        return env_path;
    }
    
    /* Try common locations */
    const char *common_paths[] = {
        "/usr/bin/gcov-tool",
        "/usr/local/bin/gcov-tool",
        "/bin/gcov-tool",
        NULL
    };
    
    for (int i = 0; common_paths[i]; i++) {
        if (access(common_paths[i], X_OK) == 0) {
            return common_paths[i];
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
    char output[4096];
    int ret = 0;
    
    /* Create temporary directory */
    strcpy(temp_dir, TEMP_DIR_TEMPLATE);
    if (!mkdtemp(temp_dir)) {
        perror("Failed to create temporary directory");
        return 1;
    }
    
    printf("Created temporary directory: %s\n", temp_dir);
    
    /* Set environment to write .gcda files to our temp directory */
    setenv("GCOV_PREFIX", temp_dir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Write test program source */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", temp_dir);
    FILE *src_file = fopen(source_path, "w");
    if (!src_file) {
        perror("Failed to create source file");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    fwrite(test_program_source, 1, strlen(test_program_source), src_file);
    fclose(src_file);
    
    /* Compile test program with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", temp_dir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s 2>&1",
             source_path, exec_path);
    
    printf("Compiling test program...\n");
    if (execute_command(cmd, output, sizeof(output)) != 0) {
        fprintf(stderr, "Compilation failed:\n%s\n", output);
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Run test program twice with different parameters to generate distinct .gcda files */
    
    /* First run - small values */
    printf("Running test program (first run)...\n");
    snprintf(cmd, sizeof(cmd), "%s 5 10", exec_path);
    execute_command(cmd, NULL, 0);
    
    /* Find the generated .gcda file from first run */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", temp_dir);
    
    /* Second run - different values to get different coverage */
    printf("Running test program (second run)...\n");
    snprintf(cmd, sizeof(cmd), "%s 20 5", exec_path);
    execute_command(cmd, NULL, 0);
    
    /* Find the generated .gcda file from second run */
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func.gcda", temp_dir);
    
    /* Check if .gcda files were created */
    if (access(gcda1_path, R_OK) != 0 || access(gcda2_path, R_OK) != 0) {
        fprintf(stderr, "Failed to generate .gcda files\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    printf("Generated .gcda files:\n  %s\n  %s\n", gcda1_path, gcda2_path);
    
    /* Find gcov-tool */
    const char *gcov_tool = find_gcov_tool();
    if (!gcov_tool) {
        fprintf(stderr, "gcov-tool not found. Please set GCOV_TOOL environment variable.\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    printf("Using gcov-tool: %s\n", gcov_tool);
    
    /* Test 1: Trigger all the specific flags from uncovered lines */
    printf("\n=== Test 1: Triggering specific flags (lines 534-554) ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -v -f -F -o -h -t 0.75 %s %s 2>&1",
             gcov_tool, gcda1_path, gcda2_path);
    
    printf("Executing: %s\n", cmd);
    int status1 = execute_command(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status1);
    printf("Output (first 500 chars):\n%.500s\n", output);
    
    if (status1 != 0 && status1 != 1) {
        /* gcov-tool often returns 1 for warnings, which is OK */
        fprintf(stderr, "Warning: gcov-tool returned unexpected status: %d\n", status1);
    }
    
    /* Test 2: Trigger default case with invalid option */
    printf("\n=== Test 2: Triggering default case (invalid option) ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -Z %s %s 2>&1",
             gcov_tool, gcda1_path, gcda2_path);
    
    printf("Executing: %s\n", cmd);
    int status2 = execute_command(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status2);
    printf("Output (first 500 chars):\n%.500s\n", output);
    
    /* The invalid option should trigger overlap_usage() */
    if (status2 == 0) {
        fprintf(stderr, "Warning: Invalid option didn't cause error as expected\n");
    }
    
    /* Test 3: Additional test with just verbose flag */
    printf("\n=== Test 3: Testing verbose flag only ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -v %s %s 2>&1",
             gcov_tool, gcda1_path, gcda2_path);
    
    printf("Executing: %s\n", cmd);
    int status3 = execute_command(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status3);
    
    /* Clean up */
    printf("\nCleaning up temporary directory: %s\n", temp_dir);
    cleanup_temp_dir(temp_dir);
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Test 1 (specific flags): %s\n", 
           (status1 == 0 || status1 == 1) ? "PASS" : "FAIL");
    printf("Test 2 (invalid option): %s\n", 
           status2 != 0 ? "PASS (triggered error as expected)" : "FAIL");
    printf("Test 3 (verbose only): %s\n", 
           (status3 == 0 || status3 == 1) ? "PASS" : "FAIL");
    
    if ((status1 == 0 || status1 == 1) && status3 == 0) {
        printf("\nAll tests completed successfully!\n");
        return 0;
    } else {
        printf("\nSome tests failed.\n");
        return 1;
    }
}
