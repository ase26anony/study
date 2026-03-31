#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_gcc_coverage.c"
#define TEMP_OBJ_FILE "test_gcc_coverage.o"

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
    fprintf(fp, "    printf(\"Hello from test program\\n\");\n");
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
            return WEXITSTATUS(status);
        } else {
            return -1;
        }
    }
}

int main(void) {
    int ret;
    
    /* Create test source file */
    if (create_test_source(TEMP_SOURCE_FILE) < 0) {
        return EXIT_FAILURE;
    }
    
    fprintf(stderr, "Created test source file: %s\n", TEMP_SOURCE_FILE);
    
    /* Test 1: Basic compilation with dump options */
    fprintf(stderr, "\n=== Test 1: Basic dump options ===\n");
    ret = run_gcc("-dumpdir ./dump/ -dumpbase myprog -dumpbase-ext .c "
                  "-c " TEMP_SOURCE_FILE " -o " TEMP_OBJ_FILE);
    fprintf(stderr, "Test 1 returned: %d\n", ret);
    
    /* Test 2: With save-temps flag */
    fprintf(stderr, "\n=== Test 2: With save-temps ===\n");
    ret = run_gcc("-save-temps -dumpdir ./temps/ -dumpbase test_save "
                  "-c " TEMP_SOURCE_FILE " -o " TEMP_OBJ_FILE);
    fprintf(stderr, "Test 2 returned: %d\n", ret);
    
    /* Test 3: All options combined */
    fprintf(stderr, "\n=== Test 3: All options combined ===\n");
    ret = run_gcc("-save-temps=cwd -dumpdir ./full_dump/ "
                  "-dumpbase full_test -dumpbase-ext .c "
                  "-c " TEMP_SOURCE_FILE " -o full_output.o");
    fprintf(stderr, "Test 3 returned: %d\n", ret);
    
    /* Test 4: Dumpbase and dumpbase-ext without dumpdir */
    fprintf(stderr, "\n=== Test 4: No dumpdir ===\n");
    ret = run_gcc("-dumpbase nodir -dumpbase-ext .c "
                  "-c " TEMP_SOURCE_FILE " -o nodir.o");
    fprintf(stderr, "Test 4 returned: %d\n", ret);
    
    /* Test 5: Different output base name */
    fprintf(stderr, "\n=== Test 5: Different outbase ===\n");
    ret = run_gcc("-dumpdir ./diff/ -dumpbase diffbase -dumpbase-ext .diff "
                  "-save-temps -c " TEMP_SOURCE_FILE " -o different_name.o");
    fprintf(stderr, "Test 5 returned: %d\n", ret);
    
    /* Test 6: Trigger error exit (invalid option) to test cleanup on error */
    fprintf(stderr, "\n=== Test 6: Error case (invalid option) ===\n");
    ret = run_gcc("-invalid-opt -dumpdir ./error/ -dumpbase error "
                  "-dumpbase-ext .c -c " TEMP_SOURCE_FILE);
    fprintf(stderr, "Test 6 returned: %d (expected non-zero)\n", ret);
    
    /* Test 7: Another variation with system() for comparison */
    fprintf(stderr, "\n=== Test 7: Using system() ===\n");
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./system_dump/ -dumpbase system_test "
             "-dumpbase-ext .sys -save-temps -c %s -o system_test.o",
             TEMP_SOURCE_FILE);
    fprintf(stderr, "Command: %s\n", cmd);
    ret = system(cmd);
    fprintf(stderr, "Test 7 returned: %d\n", ret);
    
    /* Clean up temporary files */
    fprintf(stderr, "\n=== Cleaning up ===\n");
    unlink(TEMP_SOURCE_FILE);
    unlink(TEMP_OBJ_FILE);
    unlink("full_output.o");
    unlink("nodir.o");
    unlink("different_name.o");
    unlink("system_test.o");
    
    /* Also clean up any .i, .s files created by -save-temps */
    unlink("test_gcc_coverage.i");
    unlink("test_gcc_coverage.s");
    unlink("full_test.i");
    unlink("full_test.s");
    unlink("system_test.i");
    unlink("system_test.s");
    
    fprintf(stderr, "Test program completed.\n");
    return EXIT_SUCCESS;
}
