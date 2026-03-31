#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_PATH 1024
#define MAX_CMD 4096

/* Simple test program that will be compiled with coverage instrumentation */
const char *test_program_source = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"\n"
"int process_value(int x) {\n"
"    int result = 0;\n"
"    for (int i = 0; i < 10; i++) {\n"
"        if (x > 5) {\n"
"            result += i * 2;\n"
"        } else {\n"
"            result += i;\n"
"        }\n"
"    }\n"
"    return result;\n"
"}\n"
"\n"
"int main(int argc, char *argv[]) {\n"
"    int value = 0;\n"
"    \n"
"    /* Use environment variable or argument to vary execution */\n"
"    if (argc > 1) {\n"
"        value = atoi(argv[1]);\n"
"    } else {\n"
"        char *env_val = getenv(\"TEST_VALUE\");\n"
"        if (env_val) {\n"
"            value = atoi(env_val);\n"
"        }\n"
"    }\n"
"    \n"
"    int result = process_value(value);\n"
"    printf(\"Result: %d\\n\", result);\n"
"    return 0;\n"
"}\n";

/* Create a temporary directory with a unique name */
char* create_temp_dir() {
    char *template = "/tmp/gcov_test_XXXXXX";
    char *dir_name = strdup(template);
    if (!dir_name) {
        perror("strdup failed");
        return NULL;
    }
    
    if (mkdtemp(dir_name) == NULL) {
        perror("mkdtemp failed");
        free(dir_name);
        return NULL;
    }
    
    return dir_name;
}

/* Write the test program source to a file */
int write_test_program(const char *dir_path, const char *filename) {
    char filepath[MAX_PATH];
    snprintf(filepath, sizeof(filepath), "%s/%s", dir_path, filename);
    
    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        perror("fopen failed");
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
    if (!pipe) {
        perror("popen failed");
        return -1;
    }
    
    /* Read and discard compilation output */
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        /* Optional: print compilation warnings/errors */
        printf("%s", buffer);
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
                    int run_id, int test_value) {
    char cmd[MAX_CMD];
    char exec_path[MAX_PATH];
    
    snprintf(exec_path, sizeof(exec_path), "%s/%s", dir_path, executable);
    
    /* Set GCOV_PREFIX to ensure .gcda files go to our temp directory */
    char gcov_prefix[MAX_PATH];
    snprintf(gcov_prefix, sizeof(gcov_prefix), "GCOV_PREFIX=%s", dir_path);
    
    /* Run with different test values to generate different coverage profiles */
    snprintf(cmd, sizeof(cmd), "%s %s=%s %s %d",
             gcov_prefix,
             "GCOV_PREFIX_STRIP", "0",
             exec_path, test_value);
    
    printf("Running test (run %d): %s\n", run_id, cmd);
    
    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        perror("popen failed");
        return -1;
    }
    
    /* Read and discard program output */
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        /* Optional: print program output */
        printf("Run %d output: %s", run_id, buffer);
    }
    
    int status = pclose(pipe);
    if (status != 0) {
        fprintf(stderr, "Program run %d failed with status %d\n", run_id, status);
        return -1;
    }
    
    return 0;
}

/* Execute gcov-tool with specified arguments */
int run_gcov_tool(const char *dir_path, const char *gcda1, const char *gcda2, 
                 const char *extra_args, int expect_failure) {
    char cmd[MAX_CMD];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/%s", dir_path, gcda1);
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/%s", dir_path, gcda2);
    
    /* Try to find gcov-tool in common locations */
    const char *gcov_tool = getenv("GCOV_TOOL");
    if (!gcov_tool) {
        gcov_tool = "gcov-tool";  /* Rely on PATH */
    }
    
    /* Construct the command */
    snprintf(cmd, sizeof(cmd), "%s %s %s %s 2>&1",
             gcov_tool, extra_args, gcda1_path, gcda2_path);
    
    printf("\nExecuting gcov-tool: %s\n", cmd);
    
    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        perror("popen failed");
        return -1;
    }
    
    /* Read and display output */
    char buffer[1024];
    printf("Output:\n");
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        printf("%s", buffer);
    }
    
    int status = pclose(pipe);
    
    if (expect_failure) {
        /* For invalid option, we expect non-zero exit */
        if (status == 0) {
            fprintf(stderr, "Warning: Expected failure but got success\n");
        } else {
            printf("Got expected failure status: %d\n", status);
        }
    } else if (status != 0) {
        fprintf(stderr, "gcov-tool failed with status %d\n", status);
        return -1;
    }
    
    return 0;
}

/* Clean up temporary directory */
void cleanup_temp_dir(const char *dir_path) {
    if (!dir_path) return;
    
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dir_path);
    
    printf("\nCleaning up: %s\n", cmd);
    system(cmd);
    
    free((void*)dir_path);
}

int main() {
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
    
    /* Step 2: Write test program */
    if (write_test_program(temp_dir, "test_func.c") != 0) {
        fprintf(stderr, "Failed to write test program\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Step 3: Compile with coverage */
    if (compile_with_coverage(temp_dir, "test_func.c", "test_prog") != 0) {
        fprintf(stderr, "Failed to compile test program\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* Step 4: Run test program twice with different values */
    if (run_test_program(temp_dir, "test_prog", 1, 3) != 0) {
        fprintf(stderr, "Failed first test run\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    if (run_test_program(temp_dir, "test_prog", 2, 8) != 0) {
        fprintf(stderr, "Failed second test run\n");
        cleanup_temp_dir(temp_dir);
        return 1;
    }
    
    /* The .gcda files will be named after the source file */
    const char *gcda_file = "test_func.gcda";
    
    /* Step 5: Run gcov-tool with all the flags to trigger the uncovered lines */
    /* This triggers: case 'v', case 'f', case 'F', case 'o', case 'h', case 't' */
    if (run_gcov_tool(temp_dir, gcda_file, gcda_file, 
                     "-v -f -F -o -h -t 0.75 overlap", 0) != 0) {
        fprintf(stderr, "Failed to run gcov-tool with valid options\n");
        ret = 1;
    }
    
    /* Step 6: Run gcov-tool with invalid option to trigger default case */
    /* This triggers: default: overlap_usage() */
    if (run_gcov_tool(temp_dir, gcda_file, gcda_file, 
                     "-Z invalid_option", 1) != 0) {
        /* We don't fail here because we expect this to show usage */
        printf("Invalid option test completed (showed usage as expected)\n");
    }
    
    /* Optional: Test with just verbose flag */
    printf("\n=== Testing verbose flag only ===\n");
    run_gcov_tool(temp_dir, gcda_file, gcda_file, "-v overlap", 0);
    
    /* Step 7: Cleanup */
    cleanup_temp_dir(temp_dir);
    
    if (ret == 0) {
        printf("\n=== All tests completed successfully ===\n");
    } else {
        printf("\n=== Some tests failed ===\n");
    }
    
    return ret;
}
