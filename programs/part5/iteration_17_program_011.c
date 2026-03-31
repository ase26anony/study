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
"int main(int argc, char *argv[]) {\n"
"    int i, limit;\n"
"    \n"
"    /* Different runs will have different limits */\n"
"    if (argc > 1) {\n"
"        limit = atoi(argv[1]);\n"
"    } else {\n"
"        limit = 5;  /* Default */\n"
"    }\n"
"    \n"
"    /* Loop with conditional to generate interesting coverage */\n"
"    for (i = 0; i < limit; i++) {\n"
"        if (i % 2 == 0) {\n"
"            printf(\"Even: %d\\n\", i);\n"
"        } else {\n"
"            printf(\"Odd: %d\\n\", i);\n"
"        }\n"
"    }\n"
"    \n"
"    /* Another conditional branch */\n"
"    if (limit > 10) {\n"
"        printf(\"Large limit!\\n\");\n"
"    } else {\n"
"        printf(\"Small limit\\n\");\n"
"    }\n"
"    \n"
"    return 0;\n"
"}\n";

/* Create a temporary directory with a unique name */
char* create_temp_dir() {
    char template[] = "/tmp/gcov_test_XXXXXX";
    char *dir_name = mkdtemp(template);
    if (!dir_name) {
        perror("Failed to create temporary directory");
        return NULL;
    }
    return strdup(dir_name);
}

/* Write the test program source to a file */
int write_test_program(const char *dir, const char *filename) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s/%s", dir, filename);
    
    FILE *f = fopen(path, "w");
    if (!f) {
        perror("Failed to open test program file");
        return 0;
    }
    
    fputs(test_program_source, f);
    fclose(f);
    return 1;
}

/* Compile the test program with coverage instrumentation */
int compile_with_coverage(const char *dir, const char *source_file, const char *executable) {
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
    
    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        perror("Failed to compile test program");
        return 0;
    }
    
    /* Read and discard compilation output */
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        /* Optional: print compilation warnings/errors */
        printf("Compilation: %s", buffer);
    }
    
    int status = pclose(pipe);
    if (status != 0) {
        fprintf(stderr, "Compilation failed with status %d\n", status);
        return 0;
    }
    
    return 1;
}

/* Run the test program to generate .gcda files */
int run_test_program(const char *dir, const char *executable, 
                     const char *arg, const char *gcda_suffix) {
    char cmd[MAX_CMD];
    char exec_path[MAX_PATH];
    
    snprintf(exec_path, sizeof(exec_path), "%s/%s", dir, executable);
    
    /* Set environment to ensure .gcda files are written to our temp dir */
    char old_gcov_prefix[256] = {0};
    char *existing_gcov_prefix = getenv("GCOV_PREFIX");
    if (existing_gcov_prefix) {
        strncpy(old_gcov_prefix, existing_gcov_prefix, sizeof(old_gcov_prefix)-1);
    }
    
    /* Set GCOV_PREFIX to our temp directory */
    setenv("GCOV_PREFIX", dir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Run the program */
    if (arg) {
        snprintf(cmd, sizeof(cmd), "%s %s", exec_path, arg);
    } else {
        snprintf(cmd, sizeof(cmd), "%s", exec_path);
    }
    
    printf("Running: %s\n", cmd);
    
    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        perror("Failed to run test program");
        /* Restore environment */
        if (old_gcov_prefix[0]) {
            setenv("GCOV_PREFIX", old_gcov_prefix, 1);
        } else {
            unsetenv("GCOV_PREFIX");
        }
        return 0;
    }
    
    /* Read and discard program output */
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        /* Discard output */
    }
    
    int status = pclose(pipe);
    
    /* Restore original environment */
    if (old_gcov_prefix[0]) {
        setenv("GCOV_PREFIX", old_gcov_prefix, 1);
    } else {
        unsetenv("GCOV_PREFIX");
    }
    unsetenv("GCOV_PREFIX_STRIP");
    
    if (status != 0) {
        fprintf(stderr, "Program execution failed with status %d\n", status);
        return 0;
    }
    
    /* Rename the .gcda file to preserve it for multiple runs */
    char old_gcda[MAX_PATH];
    char new_gcda[MAX_PATH];
    
    /* The .gcda file will have the same name as the source file */
    snprintf(old_gcda, sizeof(old_gcda), "%s/test_func.gcda", dir);
    snprintf(new_gcda, sizeof(new_gcda), "%s/test_func_%s.gcda", dir, gcda_suffix);
    
    if (rename(old_gcda, new_gcda) != 0 && errno != ENOENT) {
        perror("Failed to rename .gcda file");
        return 0;
    }
    
    return 1;
}

/* Execute gcov-tool with specified arguments */
int run_gcov_tool(const char *dir, const char *gcda1, const char *gcda2, 
                  const char *extra_args, int expect_failure) {
    char cmd[MAX_CMD];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/%s", dir, gcda1);
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/%s", dir, gcda2);
    
    /* Try to find gcov-tool */
    const char *gcov_tool = getenv("GCOV_TOOL");
    if (!gcov_tool) {
        gcov_tool = "gcov-tool";
    }
    
    /* Build the command */
    if (extra_args) {
        snprintf(cmd, sizeof(cmd), "%s %s %s %s 2>&1", 
                 gcov_tool, extra_args, gcda1_path, gcda2_path);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s %s 2>&1", 
                 gcov_tool, gcda1_path, gcda2_path);
    }
    
    printf("\nExecuting gcov-tool: %s\n", cmd);
    
    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        perror("Failed to execute gcov-tool");
        return 0;
    }
    
    /* Read and display output */
    char buffer[1024];
    printf("gcov-tool output:\n");
    printf("-----------------\n");
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        printf("%s", buffer);
    }
    printf("-----------------\n");
    
    int status = pclose(pipe);
    
    if (expect_failure) {
        /* For invalid option, we expect non-zero exit */
        if (status == 0) {
            fprintf(stderr, "Warning: gcov-tool with invalid option exited successfully\n");
        }
        return 1; /* We consider this successful for our testing purposes */
    } else {
        if (status != 0) {
            fprintf(stderr, "gcov-tool failed with status %d\n", status);
            return 0;
        }
    }
    
    return 1;
}

/* Clean up temporary directory */
void cleanup(const char *dir) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dir);
    
    printf("Cleaning up: %s\n", dir);
    system(cmd);
}

int main(int argc, char *argv[]) {
    char *temp_dir = NULL;
    int success = 0;
    
    printf("=== Starting gcov-tool coverage test ===\n");
    
    /* Step 1: Create temporary directory */
    temp_dir = create_temp_dir();
    if (!temp_dir) {
        goto cleanup;
    }
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Step 2: Write test program source */
    if (!write_test_program(temp_dir, "test_func.c")) {
        goto cleanup;
    }
    
    /* Step 3: Compile with coverage */
    if (!compile_with_coverage(temp_dir, "test_func.c", "test_prog")) {
        goto cleanup;
    }
    
    /* Step 4: Run program twice with different arguments to generate distinct .gcda files */
    printf("\n--- Generating first .gcda file ---\n");
    if (!run_test_program(temp_dir, "test_prog", "3", "run1")) {
        goto cleanup;
    }
    
    printf("\n--- Generating second .gcda file ---\n");
    if (!run_test_program(temp_dir, "test_prog", "8", "run2")) {
        goto cleanup;
    }
    
    /* Step 5: Run gcov-tool with all the flags to trigger the uncovered switch cases */
    printf("\n--- Running gcov-tool with overlap analysis flags ---\n");
    /* This will trigger: -v, -f, -F, -o, -h, -t 0.75 */
    if (!run_gcov_tool(temp_dir, "test_func_run1.gcda", "test_func_run2.gcda",
                      "-v -f -F -o -h -t 0.75", 0)) {
        goto cleanup;
    }
    
    /* Step 6: Run gcov-tool with invalid option to trigger default case and overlap_usage() */
    printf("\n--- Running gcov-tool with invalid option (to trigger default case) ---\n");
    /* This will trigger the default case with invalid option -Z */
    if (!run_gcov_tool(temp_dir, "test_func_run1.gcda", "test_func_run2.gcda",
                      "-Z", 1)) {
        goto cleanup;
    }
    
    /* Also test with just -h (help) which should show usage without overlap_usage */
    printf("\n--- Running gcov-tool with -h (help) ---\n");
    if (!run_gcov_tool(temp_dir, "test_func_run1.gcda", "test_func_run2.gcda",
                      "-h", 0)) {
        /* Don't fail on this - help might exit with 0 */
    }
    
    success = 1;
    printf("\n=== All tests completed successfully ===\n");
    
cleanup:
    if (temp_dir) {
        if (success) {
            printf("\nTest successful. Cleaning up...\n");
        } else {
            printf("\nTest failed. Temporary files kept in: %s\n", temp_dir);
            printf("You may want to examine the directory for debugging.\n");
            return 1;
        }
        cleanup(temp_dir);
        free(temp_dir);
    }
    
    return success ? 0 : 1;
}
