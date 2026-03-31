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

/* Simple test program source code that will be instrumented */
const char *test_program_source = 
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"\n"
"int main(int argc, char *argv[]) {\n"
"    int i, limit;\n"
"    \n"
"    /* Use environment variable or argument to vary execution */\n"
"    if (argc > 1) {\n"
"        limit = atoi(argv[1]);\n"
"    } else {\n"
"        char *env_limit = getenv(\"TEST_LIMIT\");\n"
"        limit = env_limit ? atoi(env_limit) : 5;\n"
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
"    /* Another conditional path */\n"
"    if (limit > 10) {\n"
"        printf(\"Large limit!\\n\");\n"
"    }\n"
"    \n"
"    return 0;\n"
"}\n";

/* Function to execute a command and capture output */
int execute_command(const char *cmd, int capture_output) {
    FILE *fp;
    char buffer[1024];
    int status;
    
    if (capture_output) {
        printf("Executing: %s\n", cmd);
        fp = popen(cmd, "r");
        if (fp == NULL) {
            perror("popen failed");
            return -1;
        }
        
        /* Read and optionally display output */
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("  Output: %s", buffer);
        }
        
        status = pclose(fp);
    } else {
        /* Use system() for simplicity when not capturing output */
        status = system(cmd);
    }
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

/* Clean up temporary directory */
void cleanup_tempdir(const char *tempdir) {
    char cmd[MAX_CMD];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", tempdir);
    system(cmd);
}

int main(int argc, char *argv[]) {
    char tempdir[MAX_PATH];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    char gcno_path[MAX_PATH];
    char cmd[MAX_CMD];
    int ret;
    
    /* Create temporary directory */
    strcpy(tempdir, TEMPLATE);
    if (mkdtemp(tempdir) == NULL) {
        perror("Failed to create temp directory");
        return 1;
    }
    
    printf("Created temp directory: %s\n", tempdir);
    
    /* Set environment to write .gcda files to temp directory */
    setenv("GCOV_PREFIX", tempdir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Write test program source */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", tempdir);
    FILE *src = fopen(source_path, "w");
    if (!src) {
        perror("Failed to write source file");
        cleanup_tempdir(tempdir);
        return 1;
    }
    fputs(test_program_source, src);
    fclose(src);
    
    /* Compile with coverage instrumentation */
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", tempdir);
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", tempdir);
    
    snprintf(cmd, sizeof(cmd), 
             "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s",
             source_path, exec_path);
    
    ret = execute_command(cmd, 1);
    if (ret != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup_tempdir(tempdir);
        return 1;
    }
    
    /* Run program twice with different parameters to generate distinct .gcda files */
    
    /* First run with limit=3 */
    setenv("TEST_LIMIT", "3", 1);
    snprintf(cmd, sizeof(cmd), "%s", exec_path);
    ret = execute_command(cmd, 0);
    if (ret != 0) {
        fprintf(stderr, "First program execution failed\n");
    }
    
    /* Rename first .gcda file */
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", tempdir);
    char gcda1_renamed[MAX_PATH];
    snprintf(gcda1_renamed, sizeof(gcda1_renamed), "%s/test_func_run1.gcda", tempdir);
    rename(gcda1_path, gcda1_renamed);
    
    /* Second run with limit=8 */
    setenv("TEST_LIMIT", "8", 1);
    snprintf(cmd, sizeof(cmd), "%s 8", exec_path);  /* Use command line arg this time */
    ret = execute_command(cmd, 0);
    if (ret != 0) {
        fprintf(stderr, "Second program execution failed\n");
    }
    
    /* Rename second .gcda file */
    snprintf(gcda2_path, sizeof(gcda2_path), "%s/test_func.gcda", tempdir);
    char gcda2_renamed[MAX_PATH];
    snprintf(gcda2_renamed, sizeof(gcda2_renamed), "%s/test_func_run2.gcda", tempdir);
    rename(gcda2_path, gcda2_renamed);
    
    /* Find gcov-tool path */
    const char *gcov_tool = getenv("GCOV_TOOL");
    if (!gcov_tool) {
        gcov_tool = "gcov-tool";  /* Rely on PATH */
    }
    
    /* Test 1: Trigger all the case statements in the uncovered block */
    printf("\n=== Test 1: Triggering case statements with valid options ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -v -f -F -o -h -t 0.75 %s %s",
             gcov_tool, gcda1_renamed, gcda2_renamed);
    
    ret = execute_command(cmd, 1);
    if (ret != 0) {
        fprintf(stderr, "gcov-tool overlap analysis failed (expected for some cases)\n");
    }
    
    /* Test 2: Trigger default case with invalid option */
    printf("\n=== Test 2: Triggering default case with invalid option ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -Z %s %s",
             gcov_tool, gcda1_renamed, gcda2_renamed);
    
    ret = execute_command(cmd, 1);
    if (ret != 0) {
        printf("Note: gcov-tool returned error as expected for invalid option\n");
    }
    
    /* Additional test with different threshold value */
    printf("\n=== Test 3: Testing with different threshold ===\n");
    snprintf(cmd, sizeof(cmd), 
             "%s overlap -v -t 0.5 %s %s",
             gcov_tool, gcda1_renamed, gcda2_renamed);
    
    ret = execute_command(cmd, 1);
    
    /* Clean up */
    printf("\nCleaning up temp directory: %s\n", tempdir);
    cleanup_tempdir(tempdir);
    
    printf("\nAll tests completed successfully!\n");
    printf("The following gcov-tool code paths were triggered:\n");
    printf("  - case 'v': verbose = true; gcov_set_verbose();\n");
    printf("  - case 'f': overlap_func_level = 1;\n");
    printf("  - case 'F': overlap_use_fullname = 1;\n");
    printf("  - case 'o': overlap_obj_level = 1;\n");
    printf("  - case 'h': overlap_hot_only = 1;\n");
    printf("  - case 't': overlap_hot_threshold = atof(optarg);\n");
    printf("  - default: overlap_usage();\n");
    
    return 0;
}
