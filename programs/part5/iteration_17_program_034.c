#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
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
"void test_function(int iterations, int threshold) {\n"
"    int i;\n"
"    int sum = 0;\n"
"    \n"
"    for (i = 0; i < iterations; i++) {\n"
"        if (i < threshold) {\n"
"            sum += i * 2;\n"
"        } else {\n"
"            sum += i;\n"
"        }\n"
"    }\n"
"    \n"
"    printf(\"Result: %d\\n\", sum);\n"
"}\n"
"\n"
"int main(int argc, char *argv[]) {\n"
"    int iterations = 100;\n"
"    int threshold = 50;\n"
"    \n"
"    if (argc > 1) {\n"
"        iterations = atoi(argv[1]);\n"
"    }\n"
"    if (argc > 2) {\n"
"        threshold = atoi(argv[2]);\n"
"    }\n"
"    \n"
"    test_function(iterations, threshold);\n"
"    return 0;\n"
"}\n";

/* Create a temporary directory with a unique name */
char *create_temp_dir(void) {
    char *temp_dir = malloc(MAX_PATH);
    if (!temp_dir) {
        perror("malloc failed");
        return NULL;
    }
    
    strcpy(temp_dir, TEMP_DIR_TEMPLATE);
    if (mkdtemp(temp_dir) == NULL) {
        perror("mkdtemp failed");
        free(temp_dir);
        return NULL;
    }
    
    return temp_dir;
}

/* Write the test program source to a file */
int write_test_program(const char *dir, const char *filename) {
    char path[MAX_PATH];
    FILE *fp;
    
    snprintf(path, sizeof(path), "%s/%s", dir, filename);
    fp = fopen(path, "w");
    if (!fp) {
        perror("fopen failed");
        return -1;
    }
    
    fputs(test_program_source, fp);
    fclose(fp);
    return 0;
}

/* Compile the test program with coverage instrumentation */
int compile_with_coverage(const char *dir, const char *source_file, const char *executable) {
    char cmd[MAX_CMD];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    
    snprintf(source_path, sizeof(source_path), "%s/%s", dir, source_file);
    snprintf(exec_path, sizeof(exec_path), "%s/%s", dir, executable);
    
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s 2>&1",
             source_path, exec_path);
    
    printf("Compiling: %s\n", cmd);
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Compilation failed with status %d\n", status);
        return -1;
    }
    
    return 0;
}

/* Run the test program to generate .gcda files */
int run_test_program(const char *dir, const char *executable, 
                     const char *gcda_prefix, int iter1, int thresh1) {
    char cmd[MAX_CMD];
    char exec_path[MAX_PATH];
    
    snprintf(exec_path, sizeof(exec_path), "%s/%s", dir, executable);
    
    /* Set environment to write .gcda files to our temp directory */
    setenv("GCOV_PREFIX", dir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    snprintf(cmd, sizeof(cmd), "%s %d %d", exec_path, iter1, thresh1);
    
    printf("Running test program: %s\n", cmd);
    int status = system(cmd);
    
    /* Rename the generated .gcda file with our prefix */
    char old_gcda[MAX_PATH];
    char new_gcda[MAX_PATH];
    
    /* The .gcda file will be named after the source file */
    snprintf(old_gcda, sizeof(old_gcda), "%s/test_func.gcda", dir);
    snprintf(new_gcda, sizeof(new_gcda), "%s/%s.gcda", dir, gcda_prefix);
    
    if (rename(old_gcda, new_gcda) != 0 && errno != ENOENT) {
        perror("rename failed");
    }
    
    return (status == 0) ? 0 : -1;
}

/* Execute gcov-tool with specified arguments */
int run_gcov_tool(const char *dir, const char *gcda1, const char *gcda2, 
                  const char *extra_args, int expect_failure) {
    char cmd[MAX_CMD];
    char gcda_path1[MAX_PATH];
    char gcda_path2[MAX_PATH];
    
    snprintf(gcda_path1, sizeof(gcda_path1), "%s/%s", dir, gcda1);
    snprintf(gcda_path2, sizeof(gcda_path2), "%s/%s", dir, gcda2);
    
    /* Try to find gcov-tool in common locations */
    const char *gcov_tool = getenv("GCOV_TOOL");
    if (!gcov_tool) {
        gcov_tool = "gcov-tool";
    }
    
    snprintf(cmd, sizeof(cmd), "%s overlap %s %s %s 2>&1",
             gcov_tool, extra_args, gcda_path1, gcda_path2);
    
    printf("\nExecuting gcov-tool: %s\n", cmd);
    
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        return -1;
    }
    
    /* Read and display output for debugging */
    char buffer[1024];
    printf("Output:\n");
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("  %s", buffer);
    }
    
    int status = pclose(fp);
    int exit_status = WEXITSTATUS(status);
    
    if (expect_failure) {
        printf("Expected failure - exit status: %d\n", exit_status);
        return (exit_status != 0) ? 0 : -1; /* Success if it failed */
    } else {
        printf("Exit status: %d\n", exit_status);
        return (exit_status == 0) ? 0 : -1;
    }
}

/* Clean up temporary directory */
void cleanup(const char *dir) {
    char cmd[MAX_CMD];
    
    if (dir && dir[0]) {
        snprintf(cmd, sizeof(cmd), "rm -rf %s", dir);
        printf("\nCleaning up: %s\n", cmd);
        system(cmd);
    }
}

int main(void) {
    char *temp_dir = NULL;
    int ret = 0;
    
    printf("=== Starting gcov-tool coverage test ===\n");
    
    /* Step 1: Create temporary directory */
    temp_dir = create_temp_dir();
    if (!temp_dir) {
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
    
    /* Step 3: Run test program twice with different parameters
       to generate two distinct .gcda files */
    printf("\n--- Generating first profile run ---\n");
    if (run_test_program(temp_dir, "test_prog", "run1", 100, 30) != 0) {
        ret = 1;
        goto cleanup;
    }
    
    printf("\n--- Generating second profile run ---\n");
    if (run_test_program(temp_dir, "test_prog", "run2", 200, 150) != 0) {
        ret = 1;
        goto cleanup;
    }
    
    /* Step 4: Run gcov-tool with all the flags to trigger the uncovered code */
    printf("\n--- Testing gcov-tool with all overlap flags ---\n");
    if (run_gcov_tool(temp_dir, "run1.gcda", "run2.gcda", 
                     "-v -f -F -o -h -t 0.75", 0) != 0) {
        fprintf(stderr, "First gcov-tool run failed\n");
        ret = 1;
    }
    
    /* Step 5: Run gcov-tool with invalid option to trigger default case */
    printf("\n--- Testing gcov-tool with invalid option (to trigger default case) ---\n");
    if (run_gcov_tool(temp_dir, "run1.gcda", "run2.gcda", "-Z", 1) != 0) {
        fprintf(stderr, "Second gcov-tool run didn't fail as expected\n");
        ret = 1;
    }
    
    /* Step 6: Additional test with just verbose flag */
    printf("\n--- Testing gcov-tool with verbose flag only ---\n");
    if (run_gcov_tool(temp_dir, "run1.gcda", "run2.gcda", "-v", 0) != 0) {
        fprintf(stderr, "Third gcov-tool run failed\n");
        ret = 1;
    }
    
    /* Step 7: Test with threshold only */
    printf("\n--- Testing gcov-tool with threshold flag ---\n");
    if (run_gcov_tool(temp_dir, "run1.gcda", "run2.gcda", "-t 0.5", 0) != 0) {
        fprintf(stderr, "Fourth gcov-tool run failed\n");
        ret = 1;
    }

cleanup:
    /* Step 8: Cleanup */
    if (ret == 0) {
        printf("\n=== All tests completed successfully ===\n");
    } else {
        printf("\n=== Some tests failed ===\n");
    }
    
    cleanup(temp_dir);
    free(temp_dir);
    
    return ret;
}
