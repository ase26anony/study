#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_gcc_coverage.c"
#define TEMP_OBJECT_FILE "test_gcc_coverage.o"

/* Create a simple valid C source file */
static int create_test_source(void) {
    FILE *fp = fopen(TEMP_SOURCE_FILE, "w");
    if (!fp) {
        fprintf(stderr, "Failed to create source file: %s\n", strerror(errno));
        return -1;
    }
    
    fprintf(fp, "/* Test file for GCC coverage */\n");
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    printf(\"Hello from test program\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/* Execute GCC with given arguments */
static int run_gcc(const char *args) {
    printf("Executing: gcc %s\n", args);
    
    pid_t pid = fork();
    if (pid == -1) {
        fprintf(stderr, "fork failed: %s\n", strerror(errno));
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
        fprintf(stderr, "execvp failed: %s\n", strerror(errno));
        free(args_copy);
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        free(args_copy);
        
        if (WIFEXITED(status)) {
            printf("GCC exited with status: %d\n\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else {
            fprintf(stderr, "GCC terminated abnormally\n\n");
            return -1;
        }
    }
}

int main(void) {
    int ret;
    
    /* Create test source file */
    if (create_test_source() != 0) {
        return EXIT_FAILURE;
    }
    
    printf("=== Testing GCC driver cleanup logic ===\n\n");
    
    /* Test 1: Basic compilation to trigger initialization */
    printf("Test 1: Basic compilation\n");
    ret = run_gcc("-c " TEMP_SOURCE_FILE " -o " TEMP_OBJECT_FILE);
    
    /* Test 2: With dumpdir and save-temps */
    printf("Test 2: With -dumpdir and -save-temps\n");
    ret = run_gcc("-dumpdir ./dump/ -save-temps -c " TEMP_SOURCE_FILE " -o " TEMP_OBJECT_FILE);
    
    /* Test 3: With dumpbase and dumpbase-ext */
    printf("Test 3: With -dumpbase and -dumpbase-ext\n");
    ret = run_gcc("-dumpbase myprog -dumpbase-ext .c -c " TEMP_SOURCE_FILE " -o " TEMP_OBJECT_FILE);
    
    /* Test 4: With all dump options and custom output */
    printf("Test 4: All dump options with custom output\n");
    ret = run_gcc("-dumpdir ./dumpdir/ -dumpbase mydump -dumpbase-ext .ext "
                  "-save-temps=cwd -c " TEMP_SOURCE_FILE " -o custom_output.o");
    
    /* Test 5: With different dumpdir variations */
    printf("Test 5: Different dumpdir variations\n");
    ret = run_gcc("-dumpdir dumpdir_with_trailing_slash/ -dumpbase base1 "
                  "-save-temps -c " TEMP_SOURCE_FILE);
    
    /* Test 6: Without dumpdir but with other options */
    printf("Test 6: No dumpdir but other options\n");
    ret = run_gcc("-dumpbase standalone_base -dumpbase-ext .special "
                  "-save-temps -c " TEMP_SOURCE_FILE " -o standalone.o");
    
    /* Test 7: Multiple dump options with verbose flag */
    printf("Test 7: With verbose flag\n");
    ret = run_gcc("-dumpdir verbose_dump/ -dumpbase verbose -v "
                  "-save-temps -c " TEMP_SOURCE_FILE);
    
    /* Test 8: Trigger error exit to test cleanup on error */
    printf("Test 8: Invalid option to test cleanup on error\n");
    ret = run_gcc("-dumpdir error_dump/ -dumpbase error -invalid-opt "
                  "-c " TEMP_SOURCE_FILE);
    
    /* Test 9: Complex combination with multiple outputs */
    printf("Test 9: Complex combination\n");
    ret = run_gcc("-dumpdir complex/ -dumpbase complex -dumpbase-ext .multi "
                  "-save-temps=obj -c " TEMP_SOURCE_FILE " -o complex_output.o "
                  "-MF complex.d -MT complex.o");
    
    /* Test 10: Minimal dump options */
    printf("Test 10: Minimal dump options\n");
    ret = run_gcc("-dumpbase minimal -c " TEMP_SOURCE_FILE);
    
    /* Cleanup temporary files */
    printf("Cleaning up temporary files...\n");
    unlink(TEMP_SOURCE_FILE);
    unlink(TEMP_OBJECT_FILE);
    unlink("custom_output.o");
    unlink("standalone.o");
    unlink("complex_output.o");
    
    /* Also clean up any .i, .s files created by -save-temps */
    system("rm -f *.i *.s *.o dumpdir verbose_dump error_dump complex 2>/dev/null");
    system("rm -rf ./dump/ ./dumpdir/ 2>/dev/null");
    
    printf("\n=== All tests completed ===\n");
    
    return EXIT_SUCCESS;
}
