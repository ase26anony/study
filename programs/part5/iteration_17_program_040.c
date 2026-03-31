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
"    int threshold = 30;\n"
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

/* Function to create a temporary directory */
char *create_temp_dir() {
    char *temp_dir = malloc(MAX_PATH);
    if (!temp_dir) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    
    strcpy(temp_dir, TEMP_DIR_TEMPLATE);
    if (mkdtemp(temp_dir) == NULL) {
        perror("mkdtemp failed");
        free(temp_dir);
        exit(EXIT_FAILURE);
    }
    
    return temp_dir;
}

/* Function to write the test program source file */
void write_test_program(const char *dir, const char *filename) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s/%s", dir, filename);
    
    FILE *fp = fopen(path, "w");
    if (!fp) {
        perror("fopen failed");
        exit(EXIT_FAILURE);
    }
    
    fputs(test_program_source, fp);
    fclose(fp);
}

/* Function to compile the test program with coverage instrumentation */
void compile_with_coverage(const char *dir, const char *source_file, const char *executable) {
    char cmd[MAX_CMD];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    
    snprintf(source_path, sizeof(source_path), "%s/%s", dir, source_file);
    snprintf(exec_path, sizeof(exec_path), "%s/%s", dir, executable);
    
    /* Compile with coverage flags */
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s 2>&1",
             source_path, exec_path);
    
    printf("Compiling: %s\n", cmd);
    
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        exit(EXIT_FAILURE);
    }
    
    /* Read and discard compilation output */
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        /* Optional: print compilation warnings/errors */
        printf("%s", buffer);
    }
    
    int status = pclose(fp);
    if (status != 0) {
        fprintf(stderr, "Compilation failed with status %d\n", status);
        exit(EXIT_FAILURE);
    }
}

/* Function to run the test program and generate .gcda files */
void run_test_program(const char *dir, const char *executable, 
                      const char *gcda_suffix, int arg1, int arg2) {
    char cmd[MAX_CMD];
    char exec_path[MAX_PATH];
    
    snprintf(exec_path, sizeof(exec_path), "%s/%s", dir, executable);
    
    /* Set environment variables to control .gcda file location */
    char old_gcov_prefix[256] = "";
    char *existing_gcov_prefix = getenv("GCOV_PREFIX");
    if (existing_gcov_prefix) {
        strncpy(old_gcov_prefix, existing_gcov_prefix, sizeof(old_gcov_prefix) - 1);
    }
    
    /* Force .gcda files to be written to our temp directory */
    setenv("GCOV_PREFIX", dir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Run the program with different arguments to generate different coverage data */
    snprintf(cmd, sizeof(cmd), "%s %d %d", exec_path, arg1, arg2);
    
    printf("Running: %s\n", cmd);
    
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        exit(EXIT_FAILURE);
    }
    
    /* Read and discard program output */
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        /* Optional: print program output */
        printf("Program output: %s", buffer);
    }
    
    int status = pclose(fp);
    if (status != 0) {
        fprintf(stderr, "Program execution failed with status %d\n", status);
    }
    
    /* Rename the generated .gcda file to preserve it for multiple runs */
    char old_gcda[MAX_PATH];
    char new_gcda[MAX_PATH];
    
    /* The .gcda file will have the same name as the source file */
    snprintf(old_gcda, sizeof(old_gcda), "%s/test_func.gcda", dir);
    snprintf(new_gcda, sizeof(new_gcda), "%s/test_func_%s.gcda", dir, gcda_suffix);
    
    if (rename(old_gcda, new_gcda) != 0 && errno != ENOENT) {
        perror("rename failed");
    }
    
    /* Restore original environment if it existed */
    if (strlen(old_gcov_prefix) > 0) {
        setenv("GCOV_PREFIX", old_gcov_prefix, 1);
    } else {
        unsetenv("GCOV_PREFIX");
    }
}

/* Function to execute gcov-tool with specified arguments */
int execute_gcov_tool(const char *dir, const char *gcda1, const char *gcda2, 
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
    
    /* Build the command */
    snprintf(cmd, sizeof(cmd), "%s %s %s %s 2>&1", 
             gcov_tool, extra_args, gcda_path1, gcda_path2);
    
    printf("\nExecuting gcov-tool: %s\n", cmd);
    
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        return -1;
    }
    
    /* Read and display output */
    char buffer[1024];
    printf("gcov-tool output:\n");
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
    }
    
    int status = pclose(fp);
    
    if (expect_failure) {
        printf("Expected failure - status: %d\n", status);
    } else {
        printf("gcov-tool completed with status: %d\n", status);
    }
    
    return status;
}

/* Function to clean up temporary directory */
void cleanup_temp_dir(const char *dir) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dir);
    
    printf("\nCleaning up: %s\n", dir);
    system(cmd);
}

int main() {
    char *temp_dir = NULL;
    int ret = EXIT_SUCCESS;
    
    printf("=== Starting gcov-tool coverage test ===\n");
    
    /* Step 1: Create temporary directory */
    temp_dir = create_temp_dir();
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Step 2: Write test program source */
    write_test_program(temp_dir, "test_func.c");
    printf("Wrote test program source\n");
    
    /* Step 3: Compile with coverage instrumentation */
    compile_with_coverage(temp_dir, "test_func.c", "test_prog");
    printf("Compiled test program with coverage\n");
    
    /* Step 4: Run program multiple times to generate different .gcda files */
    printf("\nGenerating first .gcda file...\n");
    run_test_program(temp_dir, "test_prog", "run1", 100, 30);
    
    printf("\nGenerating second .gcda file...\n");
    run_test_program(temp_dir, "test_prog", "run2", 200, 75);
    
    /* Step 5: Execute gcov-tool with all the flags to trigger the uncovered code */
    printf("\n=== Testing gcov-tool overlap analysis with all flags ===\n");
    
    /* This command triggers:
       - 'v': verbose = true; gcov_set_verbose();
       - 'f': overlap_func_level = 1;
       - 'F': overlap_use_fullname = 1;
       - 'o': overlap_obj_level = 1;
       - 'h': overlap_hot_only = 1;
       - 't': overlap_hot_threshold = atof(optarg);
    */
    const char *gcov_tool_args = "-v -f -F -o -h -t 0.75";
    int status = execute_gcov_tool(temp_dir, "test_func_run1.gcda", 
                                   "test_func_run2.gcda", gcov_tool_args, 0);
    
    if (status != 0) {
        printf("Warning: gcov-tool returned non-zero status: %d\n", status);
    }
    
    /* Step 6: Execute gcov-tool with invalid option to trigger default case */
    printf("\n=== Testing gcov-tool with invalid option (trigger default case) ===\n");
    
    /* This should trigger the default case and call overlap_usage() */
    const char *invalid_args = "-v -Z";
    status = execute_gcov_tool(temp_dir, "test_func_run1.gcda", 
                               "test_func_run2.gcda", invalid_args, 1);
    
    /* The invalid option should cause gcov-tool to exit with error */
    if (status == 0) {
        printf("Warning: gcov-tool with invalid option returned success (expected failure)\n");
    }
    
    /* Step 7: Clean up */
    cleanup_temp_dir(temp_dir);
    free(temp_dir);
    
    printf("\n=== Test completed ===\n");
    
    return ret;
}
