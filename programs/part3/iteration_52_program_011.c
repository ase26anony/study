#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_source.c"
#define TEMP_OUTPUT_FILE "test_coverage_output.o"

/* Create a simple valid C source file */
static int create_test_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen");
        return -1;
    }
    
    fprintf(fp, "/* Test source for GCC coverage */\n");
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/* Execute GCC with given arguments and wait for completion */
static int run_gcc(const char *args) {
    printf("[EXEC] gcc %s\n", args);
    
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        char *argv[] = {
            "gcc",
            NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
            NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
        };
        
        /* Parse arguments into argv */
        char *token;
        char *args_copy = strdup(args);
        char *saveptr = NULL;
        int i = 1;
        
        token = strtok_r(args_copy, " ", &saveptr);
        while (token != NULL && i < 20) {
            argv[i++] = token;
            token = strtok_r(NULL, " ", &saveptr);
        }
        argv[i] = NULL;
        
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
    remove(TEMP_SOURCE_FILE);
    remove(TEMP_OUTPUT_FILE);
    /* Clean up any dump files that might have been created */
    remove("myprog.c.*");
    remove("./dump/myprog.c.*");
    remove("test_coverage_source.c.*");
}

int main(void) {
    int ret;
    
    /* Create test source file */
    if (create_test_source(TEMP_SOURCE_FILE) != 0) {
        fprintf(stderr, "Failed to create test source file\n");
        return EXIT_FAILURE;
    }
    
    printf("=== Testing GCC driver cleanup logic ===\n\n");
    
    /* Test 1: Basic compilation to trigger minimal cleanup */
    printf("--- Test 1: Basic compilation ---\n");
    ret = run_gcc("-c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 2: With dumpdir and save-temps */
    printf("--- Test 2: With -dumpdir and -save-temps ---\n");
    ret = run_gcc("-dumpdir ./dump/ -save-temps -c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 3: With dumpbase and dumpbase-ext (no dumpdir) */
    printf("--- Test 3: With -dumpbase and -dumpbase-ext ---\n");
    ret = run_gcc("-dumpbase myprog -dumpbase-ext .c -c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 4: With all dump options and custom output */
    printf("--- Test 4: All dump options with custom output ---\n");
    ret = run_gcc("-dumpdir ./dump/ -dumpbase myprog -dumpbase-ext .c "
                  "-save-temps=cwd -c " TEMP_SOURCE_FILE " -o custom_output.o");
    
    /* Test 5: With verbose flag and system root (affects other variables) */
    printf("--- Test 5: With verbose and system root ---\n");
    ret = run_gcc("-v -dumpdir ./dump/ -dumpbase verbose_test -dumpbase-ext .c "
                  "-save-temps -c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 6: Invalid option to test cleanup after error */
    printf("--- Test 6: Invalid option (cleanup after error) ---\n");
    ret = run_gcc("-invalid-opt -dumpdir ./dump/ -dumpbase error_test "
                  "-dumpbase-ext .c -c " TEMP_SOURCE_FILE);
    
    /* Test 7: Multiple dumpbase-ext variations */
    printf("--- Test 7: Multiple dumpbase-ext variations ---\n");
    ret = run_gcc("-dumpbase multi_ext -dumpbase-ext .c.x -dumpbase-ext .c.y "
                  "-c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 8: Empty dumpdir (edge case) */
    printf("--- Test 8: Empty dumpdir ---\n");
    ret = run_gcc("-dumpdir \"\" -dumpbase empty_dir -c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 9: Long dumpdir path */
    printf("--- Test 9: Long dumpdir path ---\n");
    ret = run_gcc("-dumpdir ./very/long/dump/path/that/might/trigger/edge/cases/ "
                  "-dumpbase longpath -save-temps -c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 10: Combined with optimization flags */
    printf("--- Test 10: With optimization flags ---\n");
    ret = run_gcc("-O2 -dumpdir ./dump/ -dumpbase optimized -dumpbase-ext .c "
                  "-save-temps -c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    printf("=== All tests completed ===\n");
    
    /* Clean up temporary files */
    cleanup_files();
    
    return EXIT_SUCCESS;
}
