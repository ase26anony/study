#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_gcc_coverage.c"
#define TEMP_OBJECT_FILE "test_gcc_coverage.o"
#define TEMP_DUMP_DIR "./dump_dir/"

/* Create a simple valid C source file */
static int create_test_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen");
        return -1;
    }
    
    fprintf(fp, "/* Test file for GCC coverage */\n");
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    printf(\"Hello from coverage test!\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/* Execute GCC with given arguments and wait for completion */
static int run_gcc(const char *args) {
    printf("[EXEC] gcc %s\n", args);
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        char *argv[] = {"gcc", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
        int argc = 1;
        
        /* Parse arguments string into argv array */
        char *args_copy = strdup(args);
        char *token = strtok(args_copy, " ");
        
        while (token != NULL && argc < 11) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
        argv[argc] = NULL;
        
        execvp("gcc", argv);
        perror("execvp");
        free(args_copy);
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("[STATUS] Exit code: %d\n\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else {
            printf("[STATUS] Process terminated abnormally\n\n");
            return -1;
        }
    }
}

/* Clean up temporary files */
static void cleanup_files(void) {
    unlink(TEMP_SOURCE_FILE);
    unlink(TEMP_OBJECT_FILE);
    
    /* Remove dump directory if it exists */
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", TEMP_DUMP_DIR);
    system(cmd);
}

int main(void) {
    char cmd[512];
    int ret;
    
    printf("=== GCC Driver Coverage Test ===\n\n");
    
    /* Create test source file */
    if (create_test_source(TEMP_SOURCE_FILE) < 0) {
        fprintf(stderr, "Failed to create test source file\n");
        return EXIT_FAILURE;
    }
    
    /* Create dump directory */
    char mkdir_cmd[256];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", TEMP_DUMP_DIR);
    system(mkdir_cmd);
    
    /* Test 1: Basic compilation with dumpdir and save-temps */
    printf("--- Test 1: -dumpdir with -save-temps ---\n");
    snprintf(cmd, sizeof(cmd),
             "-dumpdir %s -dumpbase myprog -dumpbase-ext .c "
             "-save-temps=cwd -c %s -o %s",
             TEMP_DUMP_DIR, TEMP_SOURCE_FILE, TEMP_OBJECT_FILE);
    run_gcc(cmd);
    
    /* Test 2: dumpbase and dumpbase-ext without dumpdir */
    printf("--- Test 2: -dumpbase and -dumpbase-ext without -dumpdir ---\n");
    snprintf(cmd, sizeof(cmd),
             "-dumpbase test2 -dumpbase-ext .c "
             "-c %s -o test2.o",
             TEMP_SOURCE_FILE);
    run_gcc(cmd);
    unlink("test2.o");
    
    /* Test 3: All options combined with custom output name */
    printf("--- Test 3: All options with custom -o ---\n");
    snprintf(cmd, sizeof(cmd),
             "-dumpdir %s -dumpbase fulltest -dumpbase-ext .c "
             "-save-temps -o custom_output.o -c %s",
             TEMP_DUMP_DIR, TEMP_SOURCE_FILE);
    run_gcc(cmd);
    unlink("custom_output.o");
    
    /* Test 4: Different save-temps mode */
    printf("--- Test 4: -save-temps=obj mode ---\n");
    snprintf(cmd, sizeof(cmd),
             "-dumpdir ./ -dumpbase objmode -dumpbase-ext .c "
             "-save-temps=obj -c %s -o objmode.o",
             TEMP_SOURCE_FILE);
    run_gcc(cmd);
    unlink("objmode.o");
    unlink("objmode.i");
    unlink("objmode.s");
    
    /* Test 5: Minimal options to trigger basic cleanup */
    printf("--- Test 5: Minimal options ---\n");
    snprintf(cmd, sizeof(cmd),
             "-dumpbase minimal -c %s",
             TEMP_SOURCE_FILE);
    run_gcc(cmd);
    
    /* Test 6: Error case with invalid option (should still trigger cleanup) */
    printf("--- Test 6: Error case with invalid option ---\n");
    snprintf(cmd, sizeof(cmd),
             "-dumpdir %s -dumpbase error -dumpbase-ext .c "
             "-invalid-opt -c %s",
             TEMP_DUMP_DIR, TEMP_SOURCE_FILE);
    run_gcc(cmd);
    
    /* Test 7: Empty dumpdir (edge case) */
    printf("--- Test 7: Empty dumpdir ---\n");
    snprintf(cmd, sizeof(cmd),
             "-dumpdir \"\" -dumpbase edgedir -dumpbase-ext .c "
             "-c %s -o edgedir.o",
             TEMP_SOURCE_FILE);
    run_gcc(cmd);
    unlink("edgedir.o");
    
    /* Test 8: Long dumpbase name */
    printf("--- Test 8: Long dumpbase name ---\n");
    snprintf(cmd, sizeof(cmd),
             "-dumpbase very_long_dumpbase_name_for_testing_coverage "
             "-dumpbase-ext .c -save-temps -c %s",
             TEMP_SOURCE_FILE);
    run_gcc(cmd);
    
    /* Clean up */
    printf("--- Cleaning up ---\n");
    cleanup_files();
    
    printf("=== Test completed ===\n");
    return EXIT_SUCCESS;
}
