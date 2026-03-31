#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_PATH 1024
#define MAX_CMD 4096

/* Simple test program source code that will be compiled with coverage */
const char *test_program_source = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"\n"
"int process_value(int x) {\n"
"    int result = 0;\n"
"    for (int i = 0; i < 10; i++) {\n"
"        if (i < x) {\n"
"            result += i * 2;\n"
"        } else {\n"
"            result += i;\n"
"        }\n"
"    }\n"
"    return result;\n"
"}\n"
"\n"
"int main(int argc, char *argv[]) {\n"
"    int value = 5;  /* default */\n"
"    \n"
"    if (argc > 1) {\n"
"        value = atoi(argv[1]);\n"
"    }\n"
"    \n"
"    int result = process_value(value);\n"
"    printf(\"Result: %d\\n\", result);\n"
"    return 0;\n"
"}\n";

/* Function to create a temporary directory */
char *create_temp_dir() {
    char template[] = "/tmp/gcov_test_XXXXXX";
    char *dir = mkdtemp(template);
    if (!dir) {
        perror("Failed to create temporary directory");
        return NULL;
    }
    return strdup(dir);
}

/* Function to write the test program source file */
int write_test_program(const char *dir) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s/test_func.c", dir);
    
    FILE *fp = fopen(path, "w");
    if (!fp) {
        perror("Failed to create test program source");
        return 0;
    }
    
    fputs(test_program_source, fp);
    fclose(fp);
    return 1;
}

/* Function to compile the test program with coverage instrumentation */
int compile_with_coverage(const char *dir) {
    char cmd[MAX_CMD];
    char src_path[MAX_PATH];
    char exe_path[MAX_PATH];
    
    snprintf(src_path, sizeof(src_path), "%s/test_func.c", dir);
    snprintf(exe_path, sizeof(exe_path), "%s/test_prog", dir);
    
    /* Compile with coverage flags */
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s 2>&1",
             src_path, exe_path);
    
    printf("Compiling: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Compilation failed with status %d\n", status);
        return 0;
    }
    
    return 1;
}

/* Function to run the test program and generate .gcda files */
int run_test_program(const char *dir, int run_id, int value) {
    char cmd[MAX_CMD];
    char exe_path[MAX_PATH];
    char gcda_dir[MAX_PATH];
    
    snprintf(exe_path, sizeof(exe_path), "%s/test_prog", dir);
    
    /* Set environment to write .gcda files to our temp directory */
    snprintf(gcda_dir, sizeof(gcda_dir), "GCOV_PREFIX=%s", dir);
    putenv(gcda_dir);
    
    /* Also set GCOV_PREFIX_STRIP to strip path components */
    putenv("GCOV_PREFIX_STRIP=0");
    
    /* Run the program with different values to generate different coverage */
    snprintf(cmd, sizeof(cmd), "%s %d", exe_path, value);
    
    printf("Running test (run %d): %s\n", run_id, cmd);
    int status = system(cmd);
    
    if (status != 0) {
        fprintf(stderr, "Test run %d failed with status %d\n", run_id, status);
        return 0;
    }
    
    /* Rename the .gcda file to preserve it for multiple runs */
    char old_gcda[MAX_PATH];
    char new_gcda[MAX_PATH];
    
    snprintf(old_gcda, sizeof(old_gcda), "%s/test_func.gcda", dir);
    snprintf(new_gcda, sizeof(new_gcda), "%s/test_func_run%d.gcda", dir, run_id);
    
    if (rename(old_gcda, new_gcda) != 0 && errno != ENOENT) {
        perror("Failed to rename .gcda file");
        return 0;
    }
    
    return 1;
}

/* Function to execute gcov-tool with specified arguments */
int execute_gcov_tool(const char *dir, const char *args, const char *gcda_files[]) {
    char cmd[MAX_CMD];
    char output[MAX_PATH * 2];
    FILE *fp;
    
    /* Find gcov-tool path */
    const char *gcov_tool = getenv("GCOV_TOOL");
    if (!gcov_tool) {
        gcov_tool = "gcov-tool";
    }
    
    /* Build command */
    snprintf(cmd, sizeof(cmd), "%s overlap %s", gcov_tool, args);
    
    /* Add .gcda files */
    strcat(cmd, " ");
    for (int i = 0; gcda_files[i] != NULL; i++) {
        strcat(cmd, gcda_files[i]);
        strcat(cmd, " ");
    }
    
    printf("Executing: %s\n", cmd);
    
    /* Execute and capture output */
    fp = popen(cmd, "r");
    if (!fp) {
        perror("Failed to execute gcov-tool");
        return 0;
    }
    
    /* Read and discard output (or log it) */
    printf("Output from gcov-tool:\n");
    while (fgets(output, sizeof(output), fp) != NULL) {
        printf("  %s", output);
    }
    
    int status = pclose(fp);
    if (status != 0) {
        fprintf(stderr, "gcov-tool exited with status %d\n", WEXITSTATUS(status));
        return 0;
    }
    
    return 1;
}

/* Function to clean up temporary files */
void cleanup(const char *dir) {
    char cmd[MAX_CMD];
    
    if (!dir) return;
    
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dir);
    system(cmd);
    
    printf("Cleaned up temporary directory: %s\n", dir);
    free((void *)dir);
}

int main() {
    char *temp_dir = NULL;
    const char *gcda_files[3];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    
    printf("=== Starting gcov-tool coverage test ===\n");
    
    /* Step 1: Create temporary directory */
    temp_dir = create_temp_dir();
    if (!temp_dir) {
        return 1;
    }
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Step 2: Write test program source */
    if (!write_test_program(temp_dir)) {
        cleanup(temp_dir);
        return 1;
    }
    
    /* Step 3: Compile with coverage */
    if (!compile_with_coverage(temp_dir)) {
        cleanup(temp_dir);
        return 1;
    }
    
    /* Step 4: Run test program twice with different inputs */
    if (!run_test_program(temp_dir, 1, 3)) {
        cleanup(temp_dir);
        return 1;
    }
    
    if (!run_test_program(temp_dir, 2, 7)) {
        cleanup(temp_dir);
        return 1;
    }
    
    /* Build paths to .gcda files */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func_run1.gcda", temp_dir);
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func_run2.gcda", temp_dir);
    
    gcda_files[0] = gcda1_path;
    gcda_files[1] = gcda2_path;
    gcda_files[2] = NULL;
    
    /* Step 5: Execute gcov-tool with all the flags to trigger uncovered lines */
    printf("\n=== Testing valid gcov-tool options ===\n");
    if (!execute_gcov_tool(temp_dir, "-v -f -F -o -h -t 0.75", gcda_files)) {
        cleanup(temp_dir);
        return 1;
    }
    
    /* Step 6: Execute gcov-tool with invalid option to trigger default case */
    printf("\n=== Testing invalid gcov-tool option (to trigger default case) ===\n");
    if (!execute_gcov_tool(temp_dir, "-Z", gcda_files)) {
        /* Note: We expect this to fail since -Z is invalid */
        printf("Expected failure due to invalid option -Z\n");
    }
    
    /* Step 7: Success message and cleanup */
    printf("\n=== Test completed successfully ===\n");
    printf("Triggered the following uncovered lines in gcov-tool.cc:\n");
    printf("  - case 'v': verbose = true; gcov_set_verbose();\n");
    printf("  - case 'f': overlap_func_level = 1;\n");
    printf("  - case 'F': overlap_use_fullname = 1;\n");
    printf("  - case 'o': overlap_obj_level = 1;\n");
    printf("  - case 'h': overlap_hot_only = 1;\n");
    printf("  - case 't': overlap_hot_threshold = atof(optarg);\n");
    printf("  - default: overlap_usage();\n");
    
    cleanup(temp_dir);
    return 0;
}
