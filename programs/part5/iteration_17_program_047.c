#define _GNU_SOURCE
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

#define TEMPLATE "/tmp/gcov_test_XXXXXX"
#define MAX_PATH 4096
#define MAX_CMD 8192

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
"    int value = 10;  /* default value */\n"
"    \n"
"    if (argc > 1) {\n"
"        value = atoi(argv[1]);\n"
"    }\n"
"    \n"
"    /* Environment variable can also affect behavior */\n"
"    char *env_val = getenv(\"TEST_VALUE\");\n"
"    if (env_val) {\n"
"        value += atoi(env_val);\n"
"    }\n"
"    \n"
"    int result = process_value(value);\n"
"    printf(\"Result: %d\\n\", result);\n"
"    return 0;\n"
"}\n";

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
            if (total_read >= output_size - 1) break;
        }
    } else {
        /* Just read and discard output */
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            /* discard */
        }
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
}

/* Clean up temporary directory */
void cleanup_temp_dir(const char *temp_dir) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", temp_dir);
    system(cmd);
}

/* Find gcov-tool path */
const char *find_gcov_tool() {
    const char *paths[] = {
        "/usr/bin/gcov-tool",
        "/usr/local/bin/gcov-tool",
        "/bin/gcov-tool",
        NULL
    };
    
    /* Check if path is provided via environment */
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
    strcpy(temp_dir, TEMPLATE);
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
    
    printf("Compiling test program...\n");
    if (execute_command(cmd, output, sizeof(output)) != 0) {
        fprintf(stderr, "Compilation failed:\n%s\n", output);
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Run test program twice with different inputs to generate distinct .gcda files */
    
    /* First run with value 10 */
    printf("Running test program (first run)...\n");
    unsetenv("TEST_VALUE");  /* Ensure clean environment */
    snprintf(cmd, sizeof(cmd), "\"%s\" 10", exec_path);
    execute_command(cmd, NULL, 0);
    
    /* Find the generated .gcda file */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", temp_dir);
    
    /* Second run with different value via environment variable */
    printf("Running test program (second run)...\n");
    setenv("TEST_VALUE", "5", 1);
    snprintf(cmd, sizeof(cmd), "\"%s\" 5", exec_path);
    execute_command(cmd, NULL, 0);
    
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func.gcda", temp_dir);
    
    /* Verify .gcda files were created */
    if (access(gcda1_path, F_OK) != 0 || access(gcda2_path, F_OK) != 0) {
        fprintf(stderr, "Failed to generate .gcda files\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    printf("Generated .gcda files:\n  %s\n  %s\n", gcda1_path, gcda2_path);
    
    /* Find gcov-tool */
    const char *gcov_tool = find_gcov_tool();
    if (!gcov_tool) {
        fprintf(stderr, "gcov-tool not found. Set GCOV_TOOL environment variable.\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    printf("Using gcov-tool: %s\n", gcov_tool);
    
    /* Test 1: Trigger all the specific flag cases from uncovered lines */
    printf("\n=== Test 1: Triggering specific flag cases ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -v -f -F -o -h -t 0.75 \"%s\" \"%s\"",
             gcov_tool, gcda1_path, gcda2_path);
    
    printf("Executing: %s\n", cmd);
    int status1 = execute_command(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status1);
    if (output[0] != '\0') {
        printf("Output:\n%s\n", output);
    }
    
    /* Test 2: Trigger default case with invalid option */
    printf("\n=== Test 2: Triggering default case (invalid option) ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -Z \"%s\" \"%s\"",
             gcov_tool, gcda1_path, gcda2_path);
    
    printf("Executing: %s\n", cmd);
    int status2 = execute_command(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status2);
    if (output[0] != '\0') {
        printf("Output:\n%s\n", output);
    }
    
    /* Test 3: Additional test with just verbose flag */
    printf("\n=== Test 3: Testing verbose flag alone ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -v \"%s\" \"%s\"",
             gcov_tool, gcda1_path, gcda2_path);
    
    printf("Executing: %s\n", cmd);
    int status3 = execute_command(cmd, output, sizeof(output));
    printf("Exit status: %d\n", status3);
    
    /* Summary */
    printf("\n=== Summary ===\n");
    printf("Test 1 (all flags): %s\n", status1 == 0 ? "PASS" : "FAIL");
    printf("Test 2 (invalid option): %s (should call overlap_usage)\n", 
           status2 != 0 ? "PASS" : "FAIL");
    printf("Test 3 (verbose only): %s\n", status3 == 0 ? "PASS" : "FAIL");
    
    /* Clean up */
    printf("\nCleaning up temporary directory: %s\n", temp_dir);
    cleanup_temp_dir(temp_dir);
    
    return (status1 == 0 && status2 != 0 && status3 == 0) ? 0 : 1;
}
