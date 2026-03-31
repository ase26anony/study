#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_gcc_cleanup.c"
#define TEMP_OUTPUT_FILE "test_gcc_cleanup.o"

/* Create a simple valid C source file */
static int create_test_source(void) {
    FILE *fp = fopen(TEMP_SOURCE_FILE, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return -1;
    }
    
    fprintf(fp, "/* Test file for GCC cleanup coverage */\n");
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    printf(\"Hello from test program\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/* Clean up temporary files */
static void cleanup_temp_files(void) {
    unlink(TEMP_SOURCE_FILE);
    unlink(TEMP_OUTPUT_FILE);
    
    /* Also clean up any dump files that might have been created */
    unlink("myprog.c.*");
    unlink("./dump/myprog.c.*");
    unlink("testprog.c.*");
    unlink("combined.c.*");
}

/* Execute GCC with given arguments using fork/exec */
static int execute_gcc(const char *args) {
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
        } else {
            fprintf(stderr, "GCC terminated abnormally\n\n");
            return -1;
        }
    }
}

int main(void) {
    int ret = 0;
    
    /* Create test source file */
    if (create_test_source() < 0) {
        return EXIT_FAILURE;
    }
    
    fprintf(stderr, "=== Testing GCC cleanup block coverage ===\n\n");
    
    /* Test 1: Basic compilation with dump options */
    fprintf(stderr, "Test 1: Basic dump options\n");
    ret |= execute_gcc("-dumpdir ./dump/ -dumpbase myprog -dumpbase-ext .c "
                      "-c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 2: Save temps with dumpbase (no dumpdir) */
    fprintf(stderr, "Test 2: Save temps with dumpbase\n");
    ret |= execute_gcc("-save-temps -dumpbase testprog -dumpbase-ext .c "
                      "-c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 3: All options combined */
    fprintf(stderr, "Test 3: All options combined\n");
    ret |= execute_gcc("-dumpdir ./dump/ -dumpbase combined -dumpbase-ext .c "
                      "-save-temps=cwd -c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 4: Different output base name */
    fprintf(stderr, "Test 4: Different output base\n");
    ret |= execute_gcc("-dumpdir ./ -dumpbase diffbase -dumpbase-ext .c "
                      "-o different_output.o -c " TEMP_SOURCE_FILE);
    
    /* Test 5: Minimal options to trigger outbase allocation */
    fprintf(stderr, "Test 5: Minimal options for outbase\n");
    ret |= execute_gcc("-o custom_output.o -c " TEMP_SOURCE_FILE);
    
    /* Test 6: Error case - invalid option (should still trigger cleanup) */
    fprintf(stderr, "Test 6: Error case with invalid option\n");
    ret |= execute_gcc("-dumpdir ./dump/ -dumpbase error -dumpbase-ext .c "
                      "-invalid-option -c " TEMP_SOURCE_FILE);
    
    /* Test 7: Version/help flags (should still go through cleanup) */
    fprintf(stderr, "Test 7: Version flag with dump options\n");
    ret |= execute_gcc("-dumpdir ./dump/ -dumpbase versiontest -dumpbase-ext .c --version");
    
    /* Test 8: Empty dumpdir (edge case) */
    fprintf(stderr, "Test 8: Empty dumpdir\n");
    ret |= execute_gcc("-dumpdir '' -dumpbase empty -dumpbase-ext .c "
                      "-c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 9: Long dumpbase (stress allocation) */
    fprintf(stderr, "Test 9: Long dumpbase name\n");
    ret |= execute_gcc("-dumpdir ./longdump/ -dumpbase very_long_dumpbase_name_for_testing "
                      "-dumpbase-ext .c -c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 10: Multiple dump options with different extensions */
    fprintf(stderr, "Test 10: Different dumpbase-ext\n");
    ret |= execute_gcc("-dumpdir ./ext/ -dumpbase multiext -dumpbase-ext .i "
                      "-c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    fprintf(stderr, "\n=== All tests completed ===\n");
    
    /* Clean up temporary files */
    cleanup_temp_files();
    
    return ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
