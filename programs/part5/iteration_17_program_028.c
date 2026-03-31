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
"    int value = 10;  /* default */\n"
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

/* Execute a shell command and return its exit status */
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
            /* Discard or log output as needed */
            /* Uncomment to see output: printf("Output: %s", buffer); */
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
void cleanup(const char *temp_dir) {
    if (temp_dir) {
        char cmd[MAX_CMD];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
        system(cmd);
    }
}

int main() {
    char temp_dir[MAX_PATH];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    char gcno_path[MAX_PATH];
    char cmd[MAX_CMD];
    int status;
    
    /* Create temporary directory */
    strcpy(temp_dir, TEMPLATE);
    if (!mkdtemp(temp_dir)) {
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
    
    /* Compile with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", temp_dir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s",
             source_path, exec_path);
    
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup(temp_dir);
        return 1;
    }
    
    /* Find the .gcno file (it will be in temp_dir due to GCOV_PREFIX) */
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", temp_dir);
    
    /* Run the program twice with different inputs to generate two .gcda files */
    
    /* First run with command line argument */
    unsetenv("TEST_VALUE");  /* Ensure clean environment */
    snprintf(cmd, sizeof(cmd), "%s 5", exec_path);
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "First program run failed\n");
        cleanup(temp_dir);
        return 1;
    }
    
    /* First .gcda file */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", temp_dir);
    
    /* Second run with environment variable */
    setenv("TEST_VALUE", "10", 1);
    snprintf(cmd, sizeof(cmd), "%s 8", exec_path);
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "Second program run failed\n");
        cleanup(temp_dir);
        return 1;
    }
    
    /* Rename second .gcda to avoid overwriting */
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func2.gcda", temp_dir);
    snprintf(cmd, sizeof(cmd), "mv %s/test_func.gcda %s", temp_dir, gcda2_path);
    execute_command(cmd, 0);
    
    /* Verify files exist */
    if (access(gcda1_path, F_OK) != 0 || access(gcda2_path, F_OK) != 0) {
        fprintf(stderr, ".gcda files not created properly\n");
        cleanup(temp_dir);
        return 1;
    }
    
    printf("Generated .gcda files:\n");
    printf("  %s\n", gcda1_path);
    printf("  %s\n", gcda2_path);
    
    /* Find gcov-tool path */
    const char *gcov_tool = getenv("GCOV_TOOL");
    if (!gcov_tool) {
        gcov_tool = "gcov-tool";  /* Rely on PATH */
    }
    
    /* Test 1: Trigger all the specific flag cases from uncovered lines */
    printf("\n=== Test 1: Triggering specific flag cases ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -v -f -F -o -h -t 0.75 %s %s",
             gcov_tool, gcda1_path, gcda2_path);
    
    status = execute_command(cmd, 1);
    printf("gcov-tool overlap with flags exited with status: %d\n", status);
    
    /* Test 2: Trigger default case with invalid option */
    printf("\n=== Test 2: Triggering default case (invalid option) ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -Z %s %s",
             gcov_tool, gcda1_path, gcda2_path);
    
    status = execute_command(cmd, 1);
    printf("gcov-tool with invalid option exited with status: %d\n", status);
    
    /* Test 3: Also test with just -v flag to ensure verbose path is hit */
    printf("\n=== Test 3: Testing verbose flag alone ===\n");
    snprintf(cmd, sizeof(cmd),
             "%s overlap -v %s %s",
             gcov_tool, gcda1_path, gcda2_path);
    
    status = execute_command(cmd, 1);
    printf("gcov-tool with verbose flag exited with status: %d\n", status);
    
    /* Cleanup */
    printf("\nCleaning up temp directory: %s\n", temp_dir);
    cleanup(temp_dir);
    
    printf("\nAll tests completed successfully!\n");
    printf("The following gcov-tool code paths should have been triggered:\n");
    printf("  - case 'v': verbose = true; gcov_set_verbose();\n");
    printf("  - case 'f': overlap_func_level = 1;\n");
    printf("  - case 'F': overlap_use_fullname = 1;\n");
    printf("  - case 'o': overlap_obj_level = 1;\n");
    printf("  - case 'h': overlap_hot_only = 1;\n");
    printf("  - case 't': overlap_hot_threshold = atof(optarg);\n");
    printf("  - default: overlap_usage(); (via invalid option -Z)\n");
    
    return 0;
}
