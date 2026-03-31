#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_input.c"
#define TEMP_OUTPUT_FILE "test_coverage_output.o"

/* Create a simple valid C source file */
static int create_test_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create test source file");
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

/* Execute GCC with given arguments using fork/exec */
static int run_gcc(const char *args) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        fprintf(stderr, "Executing: gcc %s\n", args);
        
        /* Parse arguments */
        char *argv[64];
        int argc = 0;
        char *args_copy = strdup(args);
        char *token = strtok(args_copy, " ");
        
        argv[argc++] = "gcc";
        while (token && argc < 63) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
        argv[argc] = NULL;
        
        execvp("gcc", argv);
        
        /* If we get here, exec failed */
        perror("execvp failed");
        free(args_copy);
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            fprintf(stderr, "GCC exited with status: %d\n\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "GCC terminated by signal: %d\n\n", WTERMSIG(status));
            return -1;
        }
    }
    
    return 0;
}

/* Clean up temporary files */
static void cleanup_files(void) {
    remove(TEMP_SOURCE_FILE);
    remove(TEMP_OUTPUT_FILE);
    remove("test_coverage_input.i");
    remove("test_coverage_input.s");
    remove("test_coverage_input.ii");
    remove("dump/myprog.c.*");
    system("rm -rf ./dump 2>/dev/null");
}

int main(void) {
    int ret;
    
    /* Create test source file */
    if (create_test_source(TEMP_SOURCE_FILE) < 0) {
        return EXIT_FAILURE;
    }
    
    fprintf(stderr, "=== Starting GCC coverage test ===\n\n");
    
    /* Test 1: Basic compilation with dumpdir and save-temps */
    fprintf(stderr, "Test 1: Basic compilation with -dumpdir and -save-temps\n");
    ret = run_gcc("-dumpdir ./dump/ -dumpbase myprog -dumpbase-ext .c "
                  "-save-temps -c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 2: With dumpbase and dumpbase-ext but no dumpdir */
    fprintf(stderr, "Test 2: With -dumpbase and -dumpbase-ext but no -dumpdir\n");
    ret = run_gcc("-dumpbase testdump -dumpbase-ext .src "
                  "-c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 3: All options combined */
    fprintf(stderr, "Test 3: All dump options combined with -save-temps and custom -o\n");
    ret = run_gcc("-dumpdir ./dump/ -dumpbase fulltest -dumpbase-ext .c "
                  "-save-temps=cwd -c " TEMP_SOURCE_FILE " -o custom_output.o");
    
    /* Test 4: Different save-temps variant */
    fprintf(stderr, "Test 4: Different -save-temps variant with dump options\n");
    ret = run_gcc("-dumpdir ./dump/ -dumpbase variant -dumpbase-ext .c "
                  "-save-temps=obj -c " TEMP_SOURCE_FILE " -o variant.o");
    
    /* Test 5: Without -c flag (link step) */
    fprintf(stderr, "Test 5: Compile and link with dump options\n");
    ret = run_gcc("-dumpdir ./dump/ -dumpbase linktest -dumpbase-ext .c "
                  TEMP_SOURCE_FILE " -o linktest_output");
    
    /* Test 6: Trigger error exit to test cleanup path */
    fprintf(stderr, "Test 6: Trigger error with invalid option to test cleanup\n");
    ret = run_gcc("-dumpdir ./dump/ -dumpbase errortest -dumpbase-ext .c "
                  "-invalid-opt -c " TEMP_SOURCE_FILE);
    
    /* Test 7: Minimal options to ensure basic path is covered */
    fprintf(stderr, "Test 7: Minimal options for basic coverage\n");
    ret = run_gcc("-c " TEMP_SOURCE_FILE);
    
    /* Test 8: With just -o to affect outbase */
    fprintf(stderr, "Test 8: With just -o option\n");
    ret = run_gcc("-c " TEMP_SOURCE_FILE " -o just_output.o");
    
    /* Test 9: Combination that might trigger different internal paths */
    fprintf(stderr, "Test 9: Complex combination\n");
    ret = run_gcc("-dumpdir . -dumpbase complex -dumpbase-ext .xyz "
                  "-save-temps -ftime-report -c " TEMP_SOURCE_FILE " -o complex.o");
    
    /* Test 10: Empty dumpdir (edge case) */
    fprintf(stderr, "Test 10: Empty -dumpdir\n");
    ret = run_gcc("-dumpdir \"\" -dumpbase empty -dumpbase-ext .c "
                  "-c " TEMP_SOURCE_FILE " -o empty.o");
    
    fprintf(stderr, "=== GCC coverage test completed ===\n");
    
    /* Clean up temporary files */
    cleanup_files();
    
    return EXIT_SUCCESS;
}
