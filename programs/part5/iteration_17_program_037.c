#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define TEMPLATE "/tmp/gcov_test_XXXXXX"
#define MAX_PATH 1024
#define MAX_CMD 4096

/* Simple test program source code that will be compiled with coverage */
const char *test_source = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"\n"
"int process_value(int x, int threshold) {\n"
"    int result = 0;\n"
"    for (int i = 0; i < x; i++) {\n"
"        if (i % 2 == 0) {\n"
"            result += i;\n"
"        } else {\n"
"            result -= i;\n"
"        }\n"
"    }\n"
"    \n"
"    if (result > threshold) {\n"
"        return 1;\n"
"    } else {\n"
"        return 0;\n"
"    }\n"
"}\n"
"\n"
"int main(int argc, char *argv[]) {\n"
"    int value = 10;\n"
"    int threshold = 5;\n"
"    \n"
"    if (argc > 1) {\n"
"        value = atoi(argv[1]);\n"
"    }\n"
"    if (argc > 2) {\n"
"        threshold = atoi(argv[2]);\n"
"    }\n"
"    \n"
"    return process_value(value, threshold);\n"
"}\n";

/* Execute a shell command and return exit status */
int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Run a command and capture its output */
int execute_and_capture(const char *cmd, char *output, size_t output_size) {
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        return -1;
    }
    
    size_t total_read = 0;
    while (fgets(output + total_read, output_size - total_read, fp) != NULL) {
        total_read = strlen(output);
        if (total_read >= output_size - 1) {
            break;
        }
    }
    
    int status = pclose(fp);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
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
    char cmd[MAX_CMD];
    char output[4096];
    int ret = 0;
    
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
    
    /* Write test source file */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", temp_dir);
    FILE *src = fopen(source_path, "w");
    if (!src) {
        perror("Failed to create source file");
        ret = 1;
        goto cleanup;
    }
    fputs(test_source, src);
    fclose(src);
    
    /* Compile with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", temp_dir);
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s 2>&1",
             source_path, exec_path);
    
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Compilation failed\n");
        ret = 1;
        goto cleanup;
    }
    
    /* The .gcno file should be in the same directory as source */
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", temp_dir);
    
    /* Run the program twice with different inputs to generate distinct .gcda files */
    
    /* First run */
    printf("\n=== First run ===\n");
    snprintf(cmd, sizeof(cmd), "%s 10 5", exec_path);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "First run failed\n");
    }
    
    /* Wait a bit to ensure different timestamps */
    sleep(1);
    
    /* Second run with different parameters */
    printf("\n=== Second run ===\n");
    snprintf(cmd, sizeof(cmd), "%s 20 15", exec_path);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Second run failed\n");
    }
    
    /* Find the actual .gcda file paths */
    /* With GCOV_PREFIX set, .gcda files should be in temp_dir */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", temp_dir);
    
    /* For second run, we need to rename or handle multiple .gcda files */
    /* gcov-tool overlap needs multiple .gcda files, so we'll create a copy */
    char gcda2_copy[MAX_PATH];
    snprintf(gcda2_copy, sizeof(gcda2_copy), "%s/test_func_run2.gcda", temp_dir);
    
    /* Copy the .gcda file to create a second one */
    snprintf(cmd, sizeof(cmd), "cp %s %s", gcda1_path, gcda2_copy);
    execute_command(cmd);
    
    /* Actually, gcov-tool overlap needs .gcda files from different runs */
    /* Let's run the program again with different environment to get truly different data */
    printf("\n=== Third run (different branch) ===\n");
    snprintf(cmd, sizeof(cmd), "%s 5 10", exec_path);
    if (execute_command(cmd) != 0) {
        fprintf(stderr, "Third run failed\n");
    }
    
    /* Now we should have different .gcda data in the same file location */
    /* But gcov-tool overlap needs separate files. We'll copy before the third run */
    /* Actually, let's copy the .gcda after first run, then run again */
    
    /* Better approach: run, copy, run again */
    /* Remove previous .gcda */
    unlink(gcda1_path);
    
    /* Run first time */
    printf("\n=== Run 1/2 for overlap ===\n");
    snprintf(cmd, sizeof(cmd), "%s 100 50", exec_path);
    execute_command(cmd);
    
    /* Copy to gcda1 */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func_run1.gcda", temp_dir);
    snprintf(cmd, sizeof(cmd), "cp %s/test_func.gcda %s", temp_dir, gcda1_path);
    execute_command(cmd);
    
    /* Run second time with different input */
    printf("\n=== Run 2/2 for overlap ===\n");
    snprintf(cmd, sizeof(cmd), "%s 50 100", exec_path);
    execute_command(cmd);
    
    /* Copy to gcda2 */
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func_run2.gcda", temp_dir);
    snprintf(cmd, sizeof(cmd), "cp %s/test_func.gcda %s", temp_dir, gcda2_path);
    execute_command(cmd);
    
    /* Find gcov-tool path */
    const char *gcov_tool_path = getenv("GCOV_TOOL");
    if (!gcov_tool_path) {
        gcov_tool_path = "gcov-tool";  /* Rely on PATH */
    }
    
    /* Test 1: Trigger all the flag cases from uncovered lines */
    printf("\n=== Test 1: Triggering flag cases (lines 534-554) ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -v -f -F -o -h -t 0.75 %s %s 2>&1",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    output[0] = '\0';
    int exit_status = execute_and_capture(cmd, output, sizeof(output));
    printf("Exit status: %d\n", exit_status);
    printf("Output:\n%s\n", output);
    
    /* Test 2: Trigger default case with invalid option */
    printf("\n=== Test 2: Triggering default case (invalid option) ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -Z %s %s 2>&1",
             gcov_tool_path, gcda1_path, gcda2_path);
    
    output[0] = '\0';
    exit_status = execute_and_capture(cmd, output, sizeof(output));
    printf("Exit status: %d\n", exit_status);
    printf("Output:\n%s\n", output);
    
    /* Also test with just the invalid option */
    printf("\n=== Test 3: Just invalid option ===\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -Z 2>&1", gcov_tool_path);
    
    output[0] = '\0';
    exit_status = execute_and_capture(cmd, output, sizeof(output));
    printf("Exit status: %d\n", exit_status);
    printf("Output:\n%s\n", output);

cleanup:
    /* Cleanup */
    printf("\n=== Cleaning up ===\n");
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
    execute_command(cmd);
    
    if (ret == 0) {
        printf("\nSuccessfully executed all test cases!\n");
        printf("The following gcov-tool options were tested:\n");
        printf("  -v (verbose) to trigger: verbose = true; gcov_set_verbose();\n");
        printf("  -f to trigger: overlap_func_level = 1;\n");
        printf("  -F to trigger: overlap_use_fullname = 1;\n");
        printf("  -o to trigger: overlap_obj_level = 1;\n");
        printf("  -h to trigger: overlap_hot_only = 1;\n");
        printf("  -t 0.75 to trigger: overlap_hot_threshold = atof(optarg);\n");
        printf("  -Z (invalid) to trigger: default: overlap_usage();\n");
    }
    
    return ret;
}
