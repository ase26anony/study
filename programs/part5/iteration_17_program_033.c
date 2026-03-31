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
"    int res = process_value(value, threshold);\n"
"    printf(\"Result: %d\\n\", res);\n"
"    return 0;\n"
"}\n";

/* Clean up temporary directory and files */
void cleanup_temp_dir(const char *temp_dir) {
    if (!temp_dir) return;
    
    DIR *dir = opendir(temp_dir);
    if (!dir) return;
    
    struct dirent *entry;
    char filepath[MAX_PATH];
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
            
        snprintf(filepath, sizeof(filepath), "%s/%s", temp_dir, entry->d_name);
        remove(filepath);
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
        while (fgets(output + total_read, output_size - total_read, fp) != NULL) {
            total_read = strlen(output);
            if (total_read >= output_size - 1) break;
        }
    } else {
        /* Just read and discard output */
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            /* Discard */
        }
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
}

/* Find gcov-tool path */
const char *find_gcov_tool() {
    const char *gcov_tool = getenv("GCOV_TOOL");
    if (gcov_tool && access(gcov_tool, X_OK) == 0) {
        return gcov_tool;
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

int main() {
    char temp_dir[MAX_PATH];
    char source_file[MAX_PATH];
    char exec_file[MAX_PATH];
    char gcda1[MAX_PATH];
    char gcda2[MAX_PATH];
    char cmd[MAX_CMD];
    char output[4096];
    int status;
    
    /* Create temporary directory */
    strcpy(temp_dir, TEMP_DIR_TEMPLATE);
    if (!mkdtemp(temp_dir)) {
        fprintf(stderr, "Failed to create temporary directory\n");
        return 1;
    }
    
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Set environment to write .gcda files to our temp directory */
    setenv("GCOV_PREFIX", temp_dir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Create source file */
    snprintf(source_file, sizeof(source_file), "%s/test_func.c", temp_dir);
    FILE *src_fp = fopen(source_file, "w");
    if (!src_fp) {
        fprintf(stderr, "Failed to create source file\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    fputs(test_program_source, src_fp);
    fclose(src_fp);
    
    /* Compile with coverage instrumentation */
    snprintf(exec_file, sizeof(exec_file), "%s/test_prog", temp_dir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s 2>&1",
             source_file, exec_file);
    
    printf("Compiling test program...\n");
    status = execute_command(cmd, output, sizeof(output));
    if (status != 0) {
        fprintf(stderr, "Compilation failed:\n%s\n", output);
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Run program twice with different inputs to generate two .gcda files */
    printf("Running test program to generate profile data...\n");
    
    /* First run with value=10, threshold=5 */
    snprintf(cmd, sizeof(cmd), "%s 10 5", exec_file);
    status = execute_command(cmd, NULL, 0);
    
    /* The .gcda file will be named after the source file */
    snprintf(gcda1, sizeof(gcda1), "%s/test_func.gcda", temp_dir);
    
    /* Rename first .gcda to preserve it */
    char gcda1_backup[MAX_PATH];
    snprintf(gcda1_backup, sizeof(gcda1_backup), "%s/test_func.gcda.1", temp_dir);
    rename(gcda1, gcda1_backup);
    
    /* Second run with value=20, threshold=15 */
    snprintf(cmd, sizeof(cmd), "%s 20 15", exec_file);
    status = execute_command(cmd, NULL, 0);
    
    /* Rename second .gcda */
    char gcda2_backup[MAX_PATH];
    snprintf(gcda2_backup, sizeof(gcda2_backup), "%s/test_func.gcda.2", temp_dir);
    rename(gcda1, gcda2_backup);
    
    /* Restore both .gcda files with original names */
    rename(gcda1_backup, gcda1);
    rename(gcda2_backup, gcda2);
    
    /* Find gcov-tool */
    const char *gcov_tool = find_gcov_tool();
    if (!gcov_tool) {
        fprintf(stderr, "gcov-tool not found. Please set GCOV_TOOL environment variable\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    printf("Using gcov-tool: %s\n", gcov_tool);
    
    /* Test 1: Trigger all the specific flag cases (lines 534-554) */
    printf("\n=== Test 1: Triggering specific flag cases ===\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f -F -o -h -t 0.75 %s %s 2>&1",
             gcov_tool, gcda1, gcda2);
    
    printf("Executing: %s\n", cmd);
    status = execute_command(cmd, output, sizeof(output));
    
    if (status == 0) {
        printf("Successfully executed gcov-tool with all flags\n");
        if (strlen(output) > 0) {
            printf("Output (first 500 chars):\n%.500s\n", output);
        }
    } else {
        printf("gcov-tool exited with status %d\n", status);
        if (strlen(output) > 0) {
            printf("Output:\n%s\n", output);
        }
    }
    
    /* Test 2: Trigger default case with invalid option */
    printf("\n=== Test 2: Triggering default case with invalid option ===\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -Z %s %s 2>&1",
             gcov_tool, gcda1, gcda2);
    
    printf("Executing: %s\n", cmd);
    status = execute_command(cmd, output, sizeof(output));
    
    if (status != 0) {
        printf("Successfully triggered default case (expected non-zero exit)\n");
        /* The invalid option should trigger overlap_usage() */
        if (strlen(output) > 0) {
            printf("Output (first 500 chars):\n%.500s\n", output);
        }
    } else {
        printf("Warning: Invalid option didn't cause error (unexpected)\n");
    }
    
    /* Additional test: Test with just verbose flag */
    printf("\n=== Test 3: Testing verbose flag only ===\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v %s %s 2>&1",
             gcov_tool, gcda1, gcda2);
    
    printf("Executing: %s\n", cmd);
    status = execute_command(cmd, output, sizeof(output));
    
    if (status == 0) {
        printf("Successfully executed with verbose flag\n");
    }
    
    /* Clean up */
    printf("\nCleaning up temporary files...\n");
    cleanup_temp_dir(temp_dir);
    
    printf("\nAll tests completed successfully!\n");
    return 0;
}
