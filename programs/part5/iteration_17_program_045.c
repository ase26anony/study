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

/* Simple test program source code that will be instrumented */
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
"    if (env_val != NULL) {\n"
"        value = atoi(env_val);\n"
"    }\n"
"    \n"
"    int result = process_value(value);\n"
"    printf(\"Result: %d\\n\", result);\n"
"    return 0;\n"
"}\n";

/* Create a temporary directory */
char *create_temp_dir() {
    char template[] = "/tmp/gcov_test_XXXXXX";
    char *temp_dir = mkdtemp(template);
    if (temp_dir == NULL) {
        perror("Failed to create temporary directory");
        exit(EXIT_FAILURE);
    }
    return strdup(temp_dir);
}

/* Write test program source to file */
void write_test_program(const char *dir, const char *filename) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s/%s", dir, filename);
    
    FILE *fp = fopen(path, "w");
    if (fp == NULL) {
        perror("Failed to create test program source");
        exit(EXIT_FAILURE);
    }
    
    fprintf(fp, "%s", test_program_source);
    fclose(fp);
}

/* Compile test program with coverage instrumentation */
void compile_with_coverage(const char *dir, const char *source, const char *output) {
    char cmd[MAX_CMD];
    char source_path[MAX_PATH];
    char output_path[MAX_PATH];
    
    snprintf(source_path, sizeof(source_path), "%s/%s", dir, source);
    snprintf(output_path, sizeof(output_path), "%s/%s", dir, output);
    
    /* Compile with coverage flags */
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s",
             source_path, output_path);
    
    printf("Compiling: %s\n", cmd);
    
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Compilation failed with code %d\n", ret);
        exit(EXIT_FAILURE);
    }
}

/* Run the test program to generate .gcda files */
void run_test_program(const char *dir, const char *program, 
                      const char *gcda_suffix, int arg_value) {
    char program_path[MAX_PATH];
    char cmd[MAX_CMD];
    
    snprintf(program_path, sizeof(program_path), "%s/%s", dir, program);
    
    /* Set environment to ensure .gcda files are written to our temp dir */
    setenv("GCOV_PREFIX", dir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Run with different arguments to generate different coverage data */
    if (arg_value > 0) {
        snprintf(cmd, sizeof(cmd), "%s %d", program_path, arg_value);
    } else {
        /* Use environment variable instead */
        char env_val[32];
        snprintf(env_val, sizeof(env_val), "TEST_VALUE=%d", -arg_value);
        putenv(env_val);
        snprintf(cmd, sizeof(cmd), "%s", program_path);
    }
    
    printf("Running: %s\n", cmd);
    
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Program execution failed with code %d\n", ret);
    }
    
    /* Rename the generated .gcda file to have a unique name */
    char old_gcda[MAX_PATH];
    char new_gcda[MAX_PATH];
    
    /* The .gcda file will be created in dir/test_func.gcda */
    snprintf(old_gcda, sizeof(old_gcda), "%s/test_func.gcda", dir);
    snprintf(new_gcda, sizeof(new_gcda), "%s/test_func_%s.gcda", dir, gcda_suffix);
    
    if (rename(old_gcda, new_gcda) != 0 && errno != ENOENT) {
        perror("Failed to rename .gcda file");
    }
}

/* Execute gcov-tool with specified arguments */
void run_gcov_tool(const char *dir, const char *gcda1, const char *gcda2, 
                   const char *extra_args, int expect_failure) {
    char cmd[MAX_CMD];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/%s", dir, gcda1);
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/%s", dir, gcda2);
    
    /* Try to find gcov-tool in common locations */
    const char *gcov_tool = getenv("GCOV_TOOL");
    if (gcov_tool == NULL) {
        gcov_tool = "gcov-tool";
    }
    
    /* Build the command */
    snprintf(cmd, sizeof(cmd), "%s overlap %s %s %s 2>&1", 
             gcov_tool, extra_args, gcda1_path, gcda2_path);
    
    printf("\nExecuting: %s\n", cmd);
    
    /* Use popen to capture output */
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("Failed to execute gcov-tool");
        return;
    }
    
    /* Read and display output */
    char buffer[1024];
    printf("Output:\n");
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("  %s", buffer);
    }
    
    int status = pclose(fp);
    if (expect_failure) {
        printf("Command completed with status %d (expected non-zero)\n", status);
    } else {
        printf("Command completed with status %d\n", status);
    }
}

/* Clean up temporary directory */
void cleanup(const char *dir) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dir);
    
    printf("\nCleaning up: %s\n", dir);
    system(cmd);
}

int main(int argc, char *argv[]) {
    printf("=== Starting gcov-tool coverage test ===\n");
    
    /* Step 1: Create temporary directory */
    char *temp_dir = create_temp_dir();
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Step 2: Write and compile test program */
    write_test_program(temp_dir, "test_func.c");
    compile_with_coverage(temp_dir, "test_func.c", "test_prog");
    
    /* Step 3: Generate two different .gcda files */
    printf("\n--- Generating first .gcda file ---\n");
    run_test_program(temp_dir, "test_prog", "run1", 5);
    
    printf("\n--- Generating second .gcda file ---\n");
    run_test_program(temp_dir, "test_prog", "run2", 15);
    
    /* Also generate a third one with environment variable */
    printf("\n--- Generating third .gcda file ---\n");
    run_test_program(temp_dir, "test_prog", "run3", -8);  /* Negative means use env var */
    
    /* Step 4: Run gcov-tool with all the flags to trigger uncovered lines */
    printf("\n=== Testing gcov-tool overlap analysis with all flags ===\n");
    
    /* First call: Use all the flags from the uncovered block */
    const char *all_flags = "-v -f -F -o -h -t 0.75";
    run_gcov_tool(temp_dir, "test_func_run1.gcda", "test_func_run2.gcda", 
                  all_flags, 0);
    
    /* Second call: Try with different threshold value */
    printf("\n=== Testing with different threshold ===");
    run_gcov_tool(temp_dir, "test_func_run1.gcda", "test_func_run3.gcda",
                  "-v -f -t 0.5", 0);
    
    /* Step 5: Trigger the default case with invalid option */
    printf("\n=== Testing invalid option to trigger default case ===");
    run_gcov_tool(temp_dir, "test_func_run1.gcda", "test_func_run2.gcda",
                  "-v -Z", 1);  /* -Z is invalid */
    
    /* Step 6: Try another invalid option combination */
    printf("\n=== Testing another invalid option ===");
    run_gcov_tool(temp_dir, "test_func_run2.gcda", "test_func_run3.gcda",
                  "-X", 1);  /* -X is invalid */
    
    /* Step 7: Clean up */
    printf("\n=== Test completed ===\n");
    cleanup(temp_dir);
    free(temp_dir);
    
    return 0;
}
