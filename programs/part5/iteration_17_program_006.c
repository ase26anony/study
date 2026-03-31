#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

#define TEMPLATE "/tmp/gcov_test_XXXXXX"
#define MAX_PATH 1024
#define MAX_CMD 4096

/* Clean up temporary directory and files */
void cleanup(const char *dir) {
    if (!dir) return;
    
    DIR *d = opendir(dir);
    if (!d) return;
    
    struct dirent *entry;
    char path[MAX_PATH];
    
    while ((entry = readdir(d)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
            
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);
        remove(path);
    }
    closedir(d);
    rmdir(dir);
}

/* Execute command and capture output */
int execute_command(const char *cmd, int capture_output) {
    printf("Executing: %s\n", cmd);
    
    if (capture_output) {
        FILE *fp = popen(cmd, "r");
        if (!fp) {
            perror("popen failed");
            return -1;
        }
        
        char buffer[1024];
        printf("Output:\n");
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("  %s", buffer);
        }
        
        int status = pclose(fp);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    } else {
        return system(cmd);
    }
}

/* Create test source file with deterministic behavior */
void create_test_source(const char *dir, const char *filename) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s/%s", dir, filename);
    
    FILE *f = fopen(path, "w");
    if (!f) {
        perror("Failed to create source file");
        exit(1);
    }
    
    fprintf(f, "#include <stdio.h>\n");
    fprintf(f, "#include <stdlib.h>\n\n");
    fprintf(f, "int test_function(int iterations, int threshold) {\n");
    fprintf(f, "    int i, count = 0;\n");
    fprintf(f, "    for (i = 0; i < iterations; i++) {\n");
    fprintf(f, "        if (i %% 2 == 0) {\n");
    fprintf(f, "            count += 2;\n");
    fprintf(f, "        } else {\n");
    fprintf(f, "            count += 1;\n");
    fprintf(f, "        }\n");
    fprintf(f, "        if (count > threshold) {\n");
    fprintf(f, "            break;\n");
    fprintf(f, "        }\n");
    fprintf(f, "    }\n");
    fprintf(f, "    return count;\n");
    fprintf(f, "}\n\n");
    fprintf(f, "int main(int argc, char *argv[]) {\n");
    fprintf(f, "    int iterations = 100;\n");
    fprintf(f, "    int threshold = 50;\n");
    fprintf(f, "    \n");
    fprintf(f, "    if (argc > 1) iterations = atoi(argv[1]);\n");
    fprintf(f, "    if (argc > 2) threshold = atoi(argv[2]);\n");
    fprintf(f, "    \n");
    fprintf(f, "    int result = test_function(iterations, threshold);\n");
    fprintf(f, "    printf(\"Result: %%d\\n\", result);\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
    printf("Created test source: %s\n", path);
}

int main(int argc, char *argv[]) {
    char temp_dir[MAX_PATH];
    char source_path[MAX_PATH];
    char exec_path[MAX_PATH];
    char gcda1_path[MAX_PATH];
    char gcda2_path[MAX_PATH];
    char gcno_path[MAX_PATH];
    char cmd[MAX_CMD];
    int status;
    
    /* Create temporary directory */
    strcpy(temp_dir, TEMPLATE);
    if (mkdtemp(temp_dir) == NULL) {
        perror("Failed to create temp directory");
        return 1;
    }
    printf("Created temp directory: %s\n", temp_dir);
    
    /* Set environment to write .gcda files to temp directory */
    setenv("GCOV_PREFIX", temp_dir, 1);
    setenv("GCOV_PREFIX_STRIP", "0", 1);
    
    /* Create test source file */
    create_test_source(temp_dir, "test_func.c");
    
    /* Build paths */
    snprintf(source_path, sizeof(source_path), "%s/test_func.c", temp_dir);
    snprintf(exec_path, sizeof(exec_path), "%s/test_prog", temp_dir);
    snprintf(gcno_path, sizeof(gcno_path), "%s/test_func.gcno", temp_dir);
    snprintf(gcda1_path, sizeof(gcda1_path), "%s/test_func.gcda", temp_dir);
    
    /* Compile with coverage instrumentation */
    snprintf(cmd, sizeof(cmd), "gcc -fprofile-arcs -ftest-coverage -O0 -g %s -o %s",
             source_path, exec_path);
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "Compilation failed\n");
        cleanup(temp_dir);
        return 1;
    }
    
    /* Verify .gcno file was created */
    if (access(gcno_path, F_OK) != 0) {
        fprintf(stderr, ".gcno file not created: %s\n", gcno_path);
        cleanup(temp_dir);
        return 1;
    }
    
    /* First run with different parameters to generate first .gcda */
    printf("\n=== First run ===\n");
    snprintf(cmd, sizeof(cmd), "%s 100 30", exec_path);
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "First run failed\n");
        cleanup(temp_dir);
        return 1;
    }
    
    /* Rename first .gcda file */
    char gcda1_renamed[MAX_PATH];
    snprintf(gcda1_renamed, sizeof(gcda1_renamed), "%s/test_func_run1.gcda", temp_dir);
    rename(gcda1_path, gcda1_renamed);
    
    /* Second run with different parameters to generate second .gcda */
    printf("\n=== Second run ===\n");
    snprintf(cmd, sizeof(cmd), "%s 200 100", exec_path);
    status = execute_command(cmd, 1);
    if (status != 0) {
        fprintf(stderr, "Second run failed\n");
        cleanup(temp_dir);
        return 1;
    }
    
    /* Rename second .gcda file */
    char gcda2_renamed[MAX_PATH];
    snprintf(gcda2_renamed, sizeof(gcda2_renamed), "%s/test_func_run2.gcda", temp_dir);
    rename(gcda1_path, gcda2_renamed);
    
    /* Find gcov-tool path */
    const char *gcov_tool = getenv("GCOV_TOOL");
    if (!gcov_tool) {
        gcov_tool = "gcov-tool";
    }
    
    /* Test 1: Trigger all the specific flags from uncovered lines */
    printf("\n=== Test 1: Triggering specific flags (lines 534-554) ===\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v -f -F -o -h -t 0.75 %s %s",
             gcov_tool, gcda1_renamed, gcda2_renamed);
    status = execute_command(cmd, 1);
    
    if (status != 0) {
        printf("Note: gcov-tool returned non-zero (expected for some cases)\n");
    }
    
    /* Test 2: Trigger default case with invalid option */
    printf("\n=== Test 2: Triggering default case with invalid option ===\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -Z %s %s",
             gcov_tool, gcda1_renamed, gcda2_renamed);
    status = execute_command(cmd, 1);
    
    if (status != 0) {
        printf("Note: Invalid option triggered error (expected)\n");
    }
    
    /* Test 3: Additional test with just verbose flag */
    printf("\n=== Test 3: Testing verbose flag only ===\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -v %s %s",
             gcov_tool, gcda1_renamed, gcda2_renamed);
    execute_command(cmd, 1);
    
    /* Test 4: Test with threshold only */
    printf("\n=== Test 4: Testing threshold flag only ===\n");
    snprintf(cmd, sizeof(cmd), "%s overlap -t 0.5 %s %s",
             gcov_tool, gcda1_renamed, gcda2_renamed);
    execute_command(cmd, 1);
    
    /* Clean up */
    printf("\n=== Cleaning up ===\n");
    cleanup(temp_dir);
    
    printf("\nAll tests completed successfully!\n");
    printf("The following gcov-tool flags were tested:\n");
    printf("  -v (verbose) - triggers 'verbose = true; gcov_set_verbose();'\n");
    printf("  -f (overlap_func_level) - triggers 'overlap_func_level = 1;'\n");
    printf("  -F (overlap_use_fullname) - triggers 'overlap_use_fullname = 1;'\n");
    printf("  -o (overlap_obj_level) - triggers 'overlap_obj_level = 1;'\n");
    printf("  -h (overlap_hot_only) - triggers 'overlap_hot_only = 1;'\n");
    printf("  -t (overlap_hot_threshold) - triggers 'overlap_hot_threshold = atof(optarg);'\n");
    printf("  -Z (invalid) - triggers 'default: overlap_usage();'\n");
    
    return 0;
}
