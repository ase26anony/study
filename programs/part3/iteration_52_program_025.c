#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_gcc_coverage.c"
#define TEMP_OUTPUT_FILE "test_output.o"

/* Create a simple valid C source file */
static int create_test_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Failed to create source file %s: %s\n", 
                filename, strerror(errno));
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
        fprintf(stderr, "fork failed: %s\n", strerror(errno));
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        char *argv[64];
        int argc = 0;
        char *token;
        char *args_copy = strdup(args);
        
        /* Parse arguments */
        token = strtok(args_copy, " ");
        while (token != NULL && argc < 63) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
        argv[argc] = NULL;
        
        /* Execute GCC */
        execvp("gcc", argv);
        
        /* If we get here, exec failed */
        fprintf(stderr, "execvp failed: %s\n", strerror(errno));
        free(args_copy);
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            fprintf(stderr, "GCC exited with status %d\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "GCC terminated by signal %d\n", WTERMSIG(status));
            return -1;
        }
    }
    
    return 0;
}

/* Execute GCC using system() - simpler alternative */
static int run_gcc_system(const char *args) {
    fprintf(stderr, "Executing: gcc %s\n", args);
    
    char command[1024];
    snprintf(command, sizeof(command), "gcc %s", args);
    
    int status = system(command);
    
    if (status == -1) {
        fprintf(stderr, "system() failed\n");
        return -1;
    }
    
    if (WIFEXITED(status)) {
        fprintf(stderr, "GCC exited with status %d\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }
    
    return status;
}

int main(void) {
    int ret;
    
    /* Create test source file */
    if (create_test_source(TEMP_SOURCE_FILE) < 0) {
        return EXIT_FAILURE;
    }
    
    fprintf(stderr, "Created test source file: %s\n", TEMP_SOURCE_FILE);
    
    /* Test 1: -dumpdir and -save-temps */
    fprintf(stderr, "\n=== Test 1: -dumpdir and -save-temps ===\n");
    ret = run_gcc_system("-dumpdir ./dump/ -save-temps -c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 2: -dumpbase and -dumpbase-ext (no -dumpdir) */
    fprintf(stderr, "\n=== Test 2: -dumpbase and -dumpbase-ext ===\n");
    ret = run_gcc_system("-dumpbase myprog -dumpbase-ext .c -c " TEMP_SOURCE_FILE " -o test2.o");
    
    /* Test 3: All options combined */
    fprintf(stderr, "\n=== Test 3: All options combined ===\n");
    ret = run_gcc_system("-dumpdir ./dump_all/ -dumpbase fulltest -dumpbase-ext .c "
                         "-save-temps=cwd -c " TEMP_SOURCE_FILE " -o test3.o");
    
    /* Test 4: With -o option to influence outbase */
    fprintf(stderr, "\n=== Test 4: With custom -o option ===\n");
    ret = run_gcc_system("-dumpdir ./dump_o/ -dumpbase custom -dumpbase-ext .c "
                         "-save-temps -c " TEMP_SOURCE_FILE " -o custom_output.o");
    
    /* Test 5: Invalid option to test cleanup after error */
    fprintf(stderr, "\n=== Test 5: Invalid option (testing error cleanup) ===\n");
    ret = run_gcc_system("-dumpdir ./dump_err/ -invalid-opt -c " TEMP_SOURCE_FILE);
    
    /* Test 6: Multiple dumpdir variations */
    fprintf(stderr, "\n=== Test 6: Dumpdir with trailing slash ===\n");
    ret = run_gcc_system("-dumpdir ./dump_slash/ -save-temps -c " TEMP_SOURCE_FILE " -o test6.o");
    
    fprintf(stderr, "\n=== Test 7: Dumpdir without trailing slash ===\n");
    ret = run_gcc_system("-dumpdir ./dump_noslash -save-temps -c " TEMP_SOURCE_FILE " -o test7.o");
    
    /* Test 8: Empty dumpbase and dumpbase-ext */
    fprintf(stderr, "\n=== Test 8: Empty dump options ===\n");
    ret = run_gcc_system("-dumpdir ./dump_empty/ -dumpbase '' -dumpbase-ext '' "
                         "-save-temps -c " TEMP_SOURCE_FILE " -o test8.o");
    
    /* Test 9: Using fork/exec for more control */
    fprintf(stderr, "\n=== Test 9: Using fork/exec ===\n");
    {
        char cmd[512];
        snprintf(cmd, sizeof(cmd), 
                 "-dumpdir ./dump_fork/ -dumpbase fork_test -dumpbase-ext .c "
                 "-save-temps -c %s -o fork_test.o", TEMP_SOURCE_FILE);
        
        /* Using the fork/exec version */
        pid_t pid = fork();
        if (pid == 0) {
            /* Child */
            execlp("gcc", "gcc", 
                   "-dumpdir", "./dump_fork/",
                   "-dumpbase", "fork_test",
                   "-dumpbase-ext", ".c",
                   "-save-temps",
                   "-c", TEMP_SOURCE_FILE,
                   "-o", "fork_test.o",
                   NULL);
            perror("execlp failed");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            waitpid(pid, &ret, 0);
            fprintf(stderr, "Fork/exec test completed\n");
        }
    }
    
    /* Clean up temporary files */
    fprintf(stderr, "\nCleaning up temporary files...\n");
    unlink(TEMP_SOURCE_FILE);
    unlink(TEMP_OUTPUT_FILE);
    unlink("test2.o");
    unlink("test3.o");
    unlink("custom_output.o");
    unlink("test6.o");
    unlink("test7.o");
    unlink("test8.o");
    unlink("fork_test.o");
    
    /* Also clean up any .i, .s files created by -save-temps */
    system("rm -f *.i *.s 2>/dev/null");
    
    fprintf(stderr, "\nAll tests completed. Check GCC driver coverage.\n");
    return EXIT_SUCCESS;
}
