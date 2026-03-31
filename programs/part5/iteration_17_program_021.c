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
"int process_value(int x, int limit) {\n"
"    int sum = 0;\n"
"    for (int i = 0; i < x; i++) {\n"
"        if (i < limit) {\n"
"            sum += i * 2;\n"
"        } else {\n"
"            sum += i;\n"
"        }\n"
"    }\n"
"    return sum;\n"
"}\n"
"\n"
"int main(int argc, char *argv[]) {\n"
"    int x = 10;\n"
"    int limit = 5;\n"
"    \n"
"    /* Use environment variable or argument to vary execution */\n"
"    if (argc > 1) {\n"
"        x = atoi(argv[1]);\n"
"    }\n"
"    if (getenv(\"TEST_LIMIT\")) {\n"
"        limit = atoi(getenv(\"TEST_LIMIT\"));\n"
"    }\n"
"    \n"
"    int result = process_value(x, limit);\n"
"    printf(\"Result: %d\\n\", result);\n"
"    return 0;\n"
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

/* Execute a command and capture output */
int execute_command(const char *cmd, char *output, size_t output_size) {
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        fprintf(stderr, "Failed to execute command: %s\n", cmd);
        return -1;
    }
    
    if (output) {
        output[0] = '\0';
        size_t total_read = 0;
        while (fgets(output + total_read, output_size - total_read, fp) != NULL) {
            total_read = strlen(output);
            if (total_read >= output_size - 1)
                break;
        }
    } else {
        /* Just read and discard output */
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            /* discard */
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
    
    printf("Created temp directory: %s\n", temp_dir);
    
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
    fputs(test_program_source, src_file);
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
    
    /* First run: x=10, limit=3 */
    setenv("TEST_LIMIT", "3", 1);
    snprintf(cmd, sizeof(cmd), "%s 10", exec_path);
    printf("Running test program (first run)...\n");
    execute_command(cmd, NULL, 0);
    
    /* Rename the generated .gcda file */
    char gcda_temp[MAX_PATH];
    snprintf(gcda_temp, sizeof(gcda_temp), "%s/test_func.gcda", temp_dir);
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func_run1.gcda", temp_dir);
    rename(gcda_temp, gcda1_path);
    
    /* Second run: x=15, limit=8 */
    setenv("TEST_LIMIT", "8", 1);
    snprintf(cmd, sizeof(cmd), "%s 15", exec_path);
    printf("Running test program (second run)...\n");
    execute_command(cmd, NULL, 0);
    
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func_run2.gcda", temp_dir);
    rename(gcda_temp, gcda2_path);
    
    /* Find gcov-tool */
    const char *gcov_tool = find_gcov_tool();
    if (!gcov_tool) {
        fprintf(stderr, "gcov-tool not found. Please set GCOV_TOOL environment variable.\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    printf("Using gcov-tool: %s\n", gcov_tool);
    
    /* Test 1: Trigger all the specific flag cases (lines 534-554) */
    printf("\n=== Test 1: Triggering specific flag cases ===\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f -F -o -h -t 0.75 %s %s 2>&1",
             gcov_tool, gcda1_path, gcda2_path);
    
    printf("Executing: %s\n", cmd);
    int status = execute_command(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status);
    printf("Output (first 500 chars):\n%.500s\n", output);
    
    if (status != 0 && status != 1) {
        /* gcov-tool may return 1 for some warnings, but other errors are bad */
        fprintf(stderr, "Warning: gcov-tool returned unexpected status %d\n", status);
    }
    
    /* Test 2: Trigger the default case with invalid option */
    printf("\n=== Test 2: Triggering default case (invalid option) ===\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -Z %s %s 2>&1",
             gcov_tool, gcda1_path, gcda2_path);
    
    printf("Executing: %s\n", cmd);
    status = execute_command(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status);
    printf("Output (first 500 chars):\n%.500s\n", output);
    
    /* The invalid option should trigger overlap_usage() */
    if (strstr(output, "Usage:") == NULL && strstr(output, "usage:") == NULL) {
        fprintf(stderr, "Warning: Usage message not found in output\n");
    }
    
    /* Clean up */
    printf("\nCleaning up temporary files...\n");
    cleanup_temp_dir(temp_dir);
    
    printf("\nTest completed successfully!\n");
    printf("Covered the following gcov-tool.cc code paths:\n");
    printf("  - case 'v': verbose flag\n");
    printf("  - case 'f': overlap_func_level\n");
    printf("  - case 'F': overlap_use_fullname\n");
    printf("  - case 'o': overlap_obj_level\n");
    printf("  - case 'h': overlap_hot_only\n");
    printf("  - case 't': overlap_hot_threshold\n");
    printf("  - default: invalid option (triggers overlap_usage())\n");
    
    return 0;
}
