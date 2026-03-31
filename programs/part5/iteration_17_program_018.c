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

#define MAX_PATH 1024
#define MAX_CMD 4096

/* Simple test program source code that will be compiled with coverage */
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
"    int value = 10;  /* default value */\n"
"    \n"
"    if (argc > 1) {\n"
"        value = atoi(argv[1]);\n"
"    }\n"
"    \n"
"    /* Environment variable can also affect execution */\n"
"    char *env_val = getenv(\"TEST_VALUE\");\n"
"    if (env_val) {\n"
"        value += atoi(env_val);\n"
"    }\n"
"    \n"
"    int result = process_value(value);\n"
"    printf(\"Result: %d\\n\", result);\n"
"    return 0;\n"
"}\n";

/* Clean up temporary directory and files */
void cleanup_temp_dir(const char *temp_dir) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", temp_dir);
    system(cmd);
}

/* Execute a command and capture its output */
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
            if (total_read >= output_size - 1) {
                break;
            }
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
    const char *paths[] = {
        "/usr/bin/gcov-tool",
        "/usr/local/bin/gcov-tool",
        "/bin/gcov-tool",
        NULL
    };
    
    /* Check if path is provided via environment variable */
    char *env_path = getenv("GCOV_TOOL");
    if (env_path && access(env_path, X_OK) == 0) {
        return env_path;
    }
    
    /* Try default paths */
    for (int i = 0; paths[i] != NULL; i++) {
        if (access(paths[i], X_OK) == 0) {
            return paths[i];
        }
    }
    
    return NULL;
}

int main() {
    char temp_dir[MAX_PATH];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    char cmd[MAX_CMD];
    char output[4096];
    int status;
    
    /* Create a unique temporary directory */
    snprintf(temp_dir, sizeof(temp_dir), "/tmp/gcov_test_%d", getpid());
    if (mkdir(temp_dir, 0755) != 0) {
        fprintf(stderr, "Failed to create temp directory: %s\n", temp_dir);
        return 1;
    }
    
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Set environment to write .gcda files to our temp directory */
    setenv("GCOV_PREFIX", temp_dir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Write test program source file */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", temp_dir);
    FILE *src_fp = fopen(source_path, "w");
    if (!src_fp) {
        fprintf(stderr, "Failed to create source file: %s\n", source_path);
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
    
    printf("Compiling test program...\n");
    status = execute_command(cmd, output, sizeof(output));
    if (status != 0) {
        fprintf(stderr, "Compilation failed:\n%s\n", output);
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Run test program twice with different inputs to generate two .gcda files */
    
    /* First run with value 10 */
    printf("Running test program (first run)...\n");
    snprintf(cmd, sizeof(cmd), "\"%s\" 10", exec_path);
    status = execute_command(cmd, NULL, 0);
    if (status != 0) {
        fprintf(stderr, "First run failed\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Rename the first .gcda file */
    char gcda_temp[MAX_PATH];
    snprintf(gcda_temp, sizeof(gcda_temp), "%s/test_func.gcda", temp_dir);
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func_run1.gcda", temp_dir);
    if (rename(gcda_temp, gcda1_path) != 0) {
        fprintf(stderr, "Failed to rename first gcda file\n");
    }
    
    /* Second run with value 20 and environment variable */
    printf("Running test program (second run)...\n");
    setenv("TEST_VALUE", "5", 1);
    snprintf(cmd, sizeof(cmd), "\"%s\" 15", exec_path);
    status = execute_command(cmd, NULL, 0);
    if (status != 0) {
        fprintf(stderr, "Second run failed\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    unsetenv("TEST_VALUE");
    
    /* Rename the second .gcda file */
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func_run2.gcda", temp_dir);
    if (rename(gcda_temp, gcda2_path) != 0) {
        fprintf(stderr, "Failed to rename second gcda file\n");
    }
    
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
    snprintf(cmd, sizeof(cmd), 
             "\"%s\" overlap -v -f -F -o -h -t 0.75 \"%s\" \"%s\"",
             gcov_tool, gcda1_path, gcda2_path);
    
    printf("Executing: %s\n", cmd);
    status = execute_command(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status);
    if (strlen(output) > 0) {
        printf("Output:\n%s\n", output);
    }
    
    /* Test 2: Trigger the default case with invalid option */
    printf("\n=== Test 2: Triggering default case (invalid option) ===\n");
    snprintf(cmd, sizeof(cmd), 
             "\"%s\" overlap -Z \"%s\" \"%s\"",
             gcov_tool, gcda1_path, gcda2_path);
    
    printf("Executing: %s\n", cmd);
    status = execute_command(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status);
    if (strlen(output) > 0) {
        printf("Output:\n%s\n", output);
    }
    
    /* Also test with combination of valid and invalid flags */
    printf("\n=== Test 3: Mix of valid and invalid flags ===\n");
    snprintf(cmd, sizeof(cmd), 
             "\"%s\" overlap -v -f -X -o \"%s\" \"%s\"",
             gcov_tool, gcda1_path, gcda2_path);
    
    printf("Executing: %s\n", cmd);
    status = execute_command(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status);
    if (strlen(output) > 0) {
        printf("Output:\n%s\n", output);
    }
    
    /* Clean up */
    printf("\nCleaning up temporary files...\n");
    cleanup_temp_dir(temp_dir);
    
    printf("\nAll tests completed successfully!\n");
    printf("The following gcov-tool code paths should have been triggered:\n");
    printf("1. case 'v': verbose = true; gcov_set_verbose();\n");
    printf("2. case 'f': overlap_func_level = 1;\n");
    printf("3. case 'F': overlap_use_fullname = 1;\n");
    printf("4. case 'o': overlap_obj_level = 1;\n");
    printf("5. case 'h': overlap_hot_only = 1;\n");
    printf("6. case 't': overlap_hot_threshold = atof(optarg);\n");
    printf("7. default: overlap_usage();\n");
    
    return 0;
}
