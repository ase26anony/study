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
"int main(int argc, char *argv[]) {\n"
"    int i, limit;\n"
"    \n"
"    /* Different runs will have different limits */\n"
"    if (argc > 1) {\n"
"        limit = atoi(argv[1]);\n"
"    } else {\n"
"        limit = 10;  /* Default */\n"
"    }\n"
"    \n"
"    /* Environment variable can also affect execution */\n"
"    char *env_limit = getenv(\"TEST_LIMIT\");\n"
"    if (env_limit) {\n"
"        limit += atoi(env_limit);\n"
"    }\n"
"    \n"
"    int sum = 0;\n"
"    for (i = 0; i < limit; i++) {\n"
"        if (i % 2 == 0) {\n"
"            sum += i * 2;  /* Even path */\n"
"        } else {\n"
"            sum += i;      /* Odd path */\n"
"        }\n"
"        \n"
"        /* Rarely executed branch */\n"
"        if (i == limit - 1 && limit > 100) {\n"
"            sum += 1000;\n"
"        }\n"
"    }\n"
"    \n"
"    printf(\"Sum: %d\\n\", sum);\n"
"    return 0;\n"
"}\n";

/* Execute a shell command and return its exit status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status == -1) {
        perror("system() failed");
        return -1;
    }
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Run gcov-tool with specified arguments */
int run_gcov_tool(const char *gcov_tool_path, const char *temp_dir, 
                  const char *gcda1, const char *gcda2, 
                  const char *extra_args, int expect_failure) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "%s %s %s %s 2>&1", 
             gcov_tool_path, extra_args, gcda1, gcda2);
    
    printf("\n=== Running gcov-tool ===\n");
    printf("Command: %s\n", cmd);
    
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        return -1;
    }
    
    char buffer[1024];
    printf("Output:\n");
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
    }
    
    int status = pclose(fp);
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        printf("Exit code: %d\n", exit_code);
        
        if (expect_failure) {
            /* For invalid option, we expect non-zero exit */
            return (exit_code != 0) ? 0 : -1;
        } else {
            return (exit_code == 0) ? 0 : -1;
        }
    }
    
    return -1;
}

int main(int argc, char *argv[]) {
    char temp_dir[MAX_PATH];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    char gcno_path[MAX_PATH];
    
    int ret = 0;
    
    /* Create temporary directory */
    strcpy(temp_dir, TEMPLATE);
    if (mkdtemp(temp_dir) == NULL) {
        perror("Failed to create temp directory");
        return 1;
    }
    
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Set environment to write .gcda files to our temp directory */
    setenv("GCOV_PREFIX", temp_dir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Write test program source */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", temp_dir);
    FILE *src_fp = fopen(source_path, "w");
    if (!src_fp) {
        perror("Failed to create source file");
        ret = 1;
        goto cleanup;
    }
    fputs(test_program_source, src_fp);
    fclose(src_fp);
    
    /* Compile with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", temp_dir);
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", temp_dir);
    
    char compile_cmd[MAX_CMD];
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s",
             source_path, exec_path);
    
    if (execute_command(compile_cmd) != 0) {
        fprintf(stderr, "Compilation failed\n");
        ret = 1;
        goto cleanup;
    }
    
    /* Run program twice with different parameters to generate different .gcda files */
    
    /* First run: small limit */
    unsetenv("TEST_LIMIT");
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", temp_dir);
    
    char run1_cmd[MAX_CMD];
    snprintf(run1_cmd, sizeof(run1_cmd), "%s 5", exec_path);
    printf("\n=== First run (limit=5) ===\n");
    if (execute_command(run1_cmd) != 0) {
        fprintf(stderr, "First run failed\n");
        ret = 1;
        goto cleanup;
    }
    
    /* Rename first .gcda file */
    char gcda1_orig[MAX_PATH];
    snprintf(gcda1_orig, sizeof(gcda1_orig), "%s/test_func.gcda.1", temp_dir);
    rename(gcda1_path, gcda1_orig);
    
    /* Second run: larger limit with environment variable */
    setenv("TEST_LIMIT", "20", 1);
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func.gcda", temp_dir);
    
    char run2_cmd[MAX_CMD];
    snprintf(run2_cmd, sizeof(run2_cmd), "%s 15", exec_path);
    printf("\n=== Second run (limit=15 + env 20 = 35) ===\n");
    if (execute_command(run2_cmd) != 0) {
        fprintf(stderr, "Second run failed\n");
        ret = 1;
        goto cleanup;
    }
    
    /* Rename second .gcda file */
    char gcda2_orig[MAX_PATH];
    snprintf(gcda2_orig, sizeof(gcda2_orig), "%s/test_func.gcda.2", temp_dir);
    rename(gcda2_path, gcda2_orig);
    
    /* Find gcov-tool path */
    const char *gcov_tool_path = getenv("GCOV_TOOL");
    if (!gcov_tool_path) {
        gcov_tool_path = "gcov-tool";  /* Rely on PATH */
    }
    
    /* Test 1: Run gcov-tool with all the flags to trigger the uncovered switch cases */
    printf("\n=== Test 1: Triggering all flag cases ===");
    const char *valid_flags = "-v -f -F -o -h -t 0.75";
    if (run_gcov_tool(gcov_tool_path, temp_dir, gcda1_orig, gcda2_orig, valid_flags, 0) != 0) {
        fprintf(stderr, "Valid flags test failed\n");
        ret = 1;
    }
    
    /* Test 2: Run with invalid option to trigger default case and overlap_usage() */
    printf("\n=== Test 2: Triggering default case with invalid option ===");
    const char *invalid_flags = "-v -Z";  /* -Z is invalid */
    if (run_gcov_tool(gcov_tool_path, temp_dir, gcda1_orig, gcda2_orig, invalid_flags, 1) != 0) {
        fprintf(stderr, "Invalid flags test didn't behave as expected\n");
        ret = 1;
    }
    
    /* Optional: Test with just -v flag */
    printf("\n=== Test 3: Just verbose flag ===");
    if (run_gcov_tool(gcov_tool_path, temp_dir, gcda1_orig, gcda2_orig, "-v", 0) != 0) {
        fprintf(stderr, "Verbose-only test failed\n");
        ret = 1;
    }
    
    /* Optional: Test with threshold only */
    printf("\n=== Test 4: Threshold flag only ===");
    if (run_gcov_tool(gcov_tool_path, temp_dir, gcda1_orig, gcda2_orig, "-t 0.5", 0) != 0) {
        fprintf(stderr, "Threshold test failed\n");
        ret = 1;
    }

cleanup:
    /* Clean up temporary directory */
    if (temp_dir[0]) {
        char cleanup_cmd[MAX_CMD];
        snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s", temp_dir);
        printf("\nCleaning up: %s\n", cleanup_cmd);
        system(cleanup_cmd);
    }
    
    if (ret == 0) {
        printf("\n=== All tests completed successfully ===\n");
    } else {
        printf("\n=== Some tests failed ===\n");
    }
    
    return ret;
}
