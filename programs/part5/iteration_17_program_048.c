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

/* Simple test program that will be compiled with coverage instrumentation */
const char *test_program_source = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"\n"
"int main(int argc, char *argv[]) {\n"
"    int i, limit = 10;\n"
"    int sum = 0;\n"
"    \n"
"    /* Use environment variable or argument to vary execution path */\n"
"    if (argc > 1) {\n"
"        limit = atoi(argv[1]);\n"
"    } else {\n"
"        char *env_limit = getenv(\"TEST_LIMIT\");\n"
"        if (env_limit) {\n"
"            limit = atoi(env_limit);\n"
"        }\n"
"    }\n"
"    \n"
"    /* Loop with conditional to generate interesting coverage */\n"
"    for (i = 0; i < limit; i++) {\n"
"        if (i % 2 == 0) {\n"
"            sum += i * 2;\n"
"        } else {\n"
"            sum += i;\n"
"        }\n"
"        \n"
"        /* Another condition for more coverage complexity */\n"
"        if (i > limit / 2) {\n"
"            sum -= 1;\n"
"        }\n"
"    }\n"
"    \n"
"    printf(\"Final sum: %d\\n\", sum);\n"
"    return 0;\n"
"}\n";

/* Create a temporary directory with a unique name */
char* create_temp_dir() {
    char *template = strdup("/tmp/gcov_test_XXXXXX");
    if (template == NULL) {
        perror("strdup failed");
        return NULL;
    }
    
    char *dir_name = mkdtemp(template);
    if (dir_name == NULL) {
        perror("mkdtemp failed");
        free(template);
        return NULL;
    }
    
    return dir_name;
}

/* Write the test program source to a file */
int write_test_program(const char *dir_path, const char *filename) {
    char filepath[MAX_PATH];
    snprintf(filepath, sizeof(filepath), "%s/%s", dir_path, filename);
    
    FILE *fp = fopen(filepath, "w");
    if (fp == NULL) {
        perror("Failed to open test program file");
        return -1;
    }
    
    fputs(test_program_source, fp);
    fclose(fp);
    
    return 0;
}

/* Compile the test program with coverage instrumentation */
int compile_with_coverage(const char *dir_path, const char *source_file, 
                          const char *executable) {
    char cmd[MAX_CMD];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    
    snprintf(source_path, sizeof(source_path), "%s/%s", dir_path, source_file);
    snprintf(exec_path, sizeof(exec_path), "%s/%s", dir_path, executable);
    
    /* Compile with coverage flags */
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s 2>&1",
             source_path, exec_path);
    
    printf("Compiling: %s\n", cmd);
    
    FILE *pipe = popen(cmd, "r");
    if (pipe == NULL) {
        perror("popen failed");
        return -1;
    }
    
    /* Read and discard compilation output (or log it) */
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        /* Optional: printf("Compile: %s", buffer); */
    }
    
    int status = pclose(pipe);
    if (status != 0) {
        fprintf(stderr, "Compilation failed with status %d\n", status);
        return -1;
    }
    
    return 0;
}

/* Run the test program to generate .gcda files */
int run_test_program(const char *dir_path, const char *executable, 
                     int run_id, int limit_value) {
    char cmd[MAX_CMD];
    char exec_path[MAX_PATH];
    
    snprintf(exec_path, sizeof(exec_path), "%s/%s", dir_path, executable);
    
    /* Set GCOV_PREFIX to ensure .gcda files go to our temp directory */
    char gcov_prefix[MAX_PATH];
    snprintf(gcov_prefix, sizeof(gcov_prefix), "GCOV_PREFIX=%s", dir_path);
    
    /* Also set GCOV_PREFIX_STRIP to strip path components */
    char gcov_strip[] = "GCOV_PREFIX_STRIP=100";
    
    /* Run with different limits to generate different coverage profiles */
    snprintf(cmd, sizeof(cmd), "%s %s %s %d",
             gcov_prefix, gcov_strip, exec_path, limit_value);
    
    printf("Running test (run %d): %s\n", run_id, exec_path);
    
    /* Use system() for simplicity - could use fork/exec for more control */
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Test run %d failed with status %d\n", run_id, status);
        return -1;
    }
    
    /* Rename the .gcda file to preserve it for multiple runs */
    char old_gcda[MAX_PATH];
    char new_gcda[MAX_PATH];
    
    /* The .gcda file will be in dir_path with mangled name */
    /* For simplicity, we'll just run with different limits and let them overwrite,
       then copy to unique names */
    
    return 0;
}

/* Execute gcov-tool with specific arguments */
int run_gcov_tool(const char *dir_path, const char *gcda1, const char *gcda2, 
                  const char *args, int expect_failure) {
    char cmd[MAX_CMD];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/%s", dir_path, gcda1);
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/%s", dir_path, gcda2);
    
    /* Find gcov-tool - try environment variable first, then default path */
    const char *gcov_tool = getenv("GCOV_TOOL");
    if (gcov_tool == NULL) {
        gcov_tool = "gcov-tool";
    }
    
    /* Construct the command */
    snprintf(cmd, sizeof(cmd), "%s overlap %s %s %s 2>&1",
             gcov_tool, args, gcda1_path, gcda2_path);
    
    printf("Executing: %s\n", cmd);
    
    FILE *pipe = popen(cmd, "r");
    if (pipe == NULL) {
        perror("popen failed");
        return -1;
    }
    
    /* Read and display output */
    char buffer[1024];
    printf("Output from gcov-tool:\n");
    printf("----------------------------------------\n");
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        printf("%s", buffer);
    }
    printf("----------------------------------------\n");
    
    int status = pclose(pipe);
    
    if (expect_failure) {
        /* For invalid option, we expect non-zero exit */
        if (status == 0) {
            fprintf(stderr, "Warning: Invalid option didn't cause failure\n");
        }
        return 0; /* We don't care about exit status for this test */
    }
    
    if (status != 0) {
        fprintf(stderr, "gcov-tool failed with status %d\n", status);
        return -1;
    }
    
    return 0;
}

/* Clean up temporary directory */
void cleanup(const char *dir_path) {
    if (dir_path == NULL) return;
    
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dir_path);
    
    printf("Cleaning up: %s\n", dir_path);
    system(cmd);
}

int main(int argc, char *argv[]) {
    char *temp_dir = NULL;
    int ret = 0;
    
    printf("=== Starting gcov-tool coverage test ===\n");
    
    /* Step 1: Create temporary directory */
    temp_dir = create_temp_dir();
    if (temp_dir == NULL) {
        fprintf(stderr, "Failed to create temp directory\n");
        return 1;
    }
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Step 2: Write and compile test program */
    if (write_test_program(temp_dir, "test_func.c") != 0) {
        ret = 1;
        goto cleanup;
    }
    
    if (compile_with_coverage(temp_dir, "test_func.c", "test_prog") != 0) {
        ret = 1;
        goto cleanup;
    }
    
    /* Step 3: Run test program multiple times to generate different .gcda files */
    
    /* First run with limit 5 */
    char env_cmd1[MAX_CMD];
    snprintf(env_cmd1, sizeof(env_cmd1), 
             "GCOV_PREFIX=%s GCOV_PREFIX_STRIP=100 %s/test_prog 5",
             temp_dir, temp_dir);
    printf("Run 1: %s\n", env_cmd1);
    system(env_cmd1);
    
    /* Copy first .gcda file */
    char cp_cmd1[MAX_CMD];
    snprintf(cp_cmd1, sizeof(cp_cmd1), 
             "cp %s/test_func.gcda %s/test_func_run1.gcda 2>/dev/null", 
             temp_dir, temp_dir);
    system(cp_cmd1);
    
    /* Second run with limit 20 */
    char env_cmd2[MAX_CMD];
    snprintf(env_cmd2, sizeof(env_cmd2), 
             "GCOV_PREFIX=%s GCOV_PREFIX_STRIP=100 %s/test_prog 20",
             temp_dir, temp_dir);
    printf("Run 2: %s\n", env_cmd2);
    system(env_cmd2);
    
    /* Copy second .gcda file */
    char cp_cmd2[MAX_CMD];
    snprintf(cp_cmd2, sizeof(cp_cmd2), 
             "cp %s/test_func.gcda %s/test_func_run2.gcda 2>/dev/null", 
             temp_dir, temp_dir);
    system(cp_cmd2);
    
    /* Step 4: Run gcov-tool with all the target flags to trigger uncovered lines */
    printf("\n=== Testing gcov-tool with valid options ===\n");
    if (run_gcov_tool(temp_dir, "test_func_run1.gcda", "test_func_run2.gcda",
                     "-v -f -F -o -h -t 0.75", 0) != 0) {
        fprintf(stderr, "Valid options test failed\n");
        ret = 1;
    }
    
    /* Step 5: Run gcov-tool with invalid option to trigger default case */
    printf("\n=== Testing gcov-tool with invalid option ===\n");
    if (run_gcov_tool(temp_dir, "test_func_run1.gcda", "test_func_run2.gcda",
                     "-Z", 1) != 0) {
        fprintf(stderr, "Invalid option test failed\n");
        ret = 1;
    }
    
    /* Also test with just invalid option and no files to ensure overlap_usage is called */
    printf("\n=== Testing gcov-tool with invalid option (no files) ===\n");
    const char *gcov_tool = getenv("GCOV_TOOL");
    if (gcov_tool == NULL) {
        gcov_tool = "gcov-tool";
    }
    
    char invalid_cmd[MAX_CMD];
    snprintf(invalid_cmd, sizeof(invalid_cmd), "%s overlap -Z 2>&1", gcov_tool);
    printf("Executing: %s\n", invalid_cmd);
    
    FILE *pipe = popen(invalid_cmd, "r");
    if (pipe) {
        char buffer[1024];
        printf("Output:\n");
        printf("----------------------------------------\n");
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            printf("%s", buffer);
        }
        printf("----------------------------------------\n");
        pclose(pipe);
    }
    
    if (ret == 0) {
        printf("\n=== All tests completed successfully ===\n");
    } else {
        printf("\n=== Some tests failed ===\n");
    }
    
cleanup:
    /* Step 6: Clean up */
    if (temp_dir != NULL) {
        cleanup(temp_dir);
        free(temp_dir);
    }
    
    return ret;
}
