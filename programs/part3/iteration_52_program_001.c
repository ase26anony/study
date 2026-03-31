#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_input.c"
#define TEMP_OBJECT_FILE "test_coverage_output.o"

/* Create a simple valid C source file for compilation */
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

/* Execute GCC with the given arguments using fork/exec */
static int run_gcc(const char *args) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        fprintf(stderr, "Executing: gcc %s\n", args);
        
        /* Parse arguments for execvp */
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
        perror("execvp");
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

/* Alternative using system() for simplicity */
static int run_gcc_system(const char *args) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "gcc %s", args);
    fprintf(stderr, "Executing: %s\n", cmd);
    return system(cmd);
}

int main(void) {
    int ret;
    
    /* Step 1: Create a valid C source file */
    if (create_test_source(TEMP_SOURCE_FILE) < 0) {
        fprintf(stderr, "Failed to create test source file\n");
        return EXIT_FAILURE;
    }
    
    fprintf(stderr, "Created test source file: %s\n", TEMP_SOURCE_FILE);
    
    /* Step 2: Multiple distinct GCC invocations with different combinations
       of options to trigger the cleanup logic */
    
    /* Invocation 1: With -dumpdir and -save-temps */
    ret = run_gcc_system("-dumpdir ./dump/ -save-temps -c " TEMP_SOURCE_FILE " -o " TEMP_OBJECT_FILE);
    fprintf(stderr, "Invocation 1 returned: %d\n\n", ret);
    
    /* Clean up temporary files from previous invocation */
    system("rm -f dumpdir.* dumpbase.* dumpbase_ext.* outbase.* 2>/dev/null");
    system("rm -f *.i *.s *.o 2>/dev/null");
    
    /* Invocation 2: With -dumpbase and -dumpbase-ext but no -dumpdir */
    ret = run_gcc_system("-dumpbase myprog -dumpbase-ext .c -c " TEMP_SOURCE_FILE " -o " TEMP_OBJECT_FILE);
    fprintf(stderr, "Invocation 2 returned: %d\n\n", ret);
    
    /* Clean up temporary files */
    system("rm -f dumpdir.* dumpbase.* dumpbase_ext.* outbase.* 2>/dev/null");
    system("rm -f *.i *.s *.o 2>/dev/null");
    
    /* Invocation 3: With all options combined */
    ret = run_gcc_system("-dumpdir ./dump_all/ -dumpbase fulltest -dumpbase-ext .c "
                         "-save-temps=cwd -c " TEMP_SOURCE_FILE " -o " TEMP_OBJECT_FILE);
    fprintf(stderr, "Invocation 3 returned: %d\n\n", ret);
    
    /* Clean up temporary files */
    system("rm -f dumpdir.* dumpbase.* dumpbase_ext.* outbase.* 2>/dev/null");
    system("rm -f *.i *.s *.o 2>/dev/null");
    
    /* Invocation 4: With -o option to influence outbase */
    ret = run_gcc_system("-dumpdir ./dump_out/ -dumpbase outtest -dumpbase-ext .c "
                         "-save-temps -c " TEMP_SOURCE_FILE " -o custom_output.o");
    fprintf(stderr, "Invocation 4 returned: %d\n\n", ret);
    
    /* Clean up temporary files */
    system("rm -f dumpdir.* dumpbase.* dumpbase_ext.* outbase.* 2>/dev/null");
    system("rm -f *.i *.s *.o 2>/dev/null");
    
    /* Invocation 5: Trigger cleanup after error exit with invalid option */
    ret = run_gcc_system("-dumpdir ./dump_err/ -dumpbase errortest -dumpbase-ext .c "
                         "-invalid-opt -c " TEMP_SOURCE_FILE);
    fprintf(stderr, "Invocation 5 (with error) returned: %d\n\n", ret);
    
    /* Invocation 6: Using fork/exec directly for more control */
    {
        char args[512];
        snprintf(args, sizeof(args), 
                 "-dumpdir ./dump_fork/ -dumpbase forktest -dumpbase-ext .c "
                 "-save-temps -c %s -o fork_output.o", TEMP_SOURCE_FILE);
        ret = run_gcc(args);
        fprintf(stderr, "Invocation 6 (fork/exec) returned: %d\n\n", ret);
    }
    
    /* Invocation 7: Minimal options to ensure basic cleanup path */
    ret = run_gcc_system("-dumpbase minimal -c " TEMP_SOURCE_FILE);
    fprintf(stderr, "Invocation 7 returned: %d\n\n", ret);
    
    /* Invocation 8: With verbose flag to trigger verbose_only_flag */
    ret = run_gcc_system("-dumpdir ./dump_verbose/ -v -c " TEMP_SOURCE_FILE);
    fprintf(stderr, "Invocation 8 returned: %d\n\n", ret);
    
    /* Step 3: Clean up the temporary source file */
    if (unlink(TEMP_SOURCE_FILE) < 0) {
        perror("unlink");
    } else {
        fprintf(stderr, "Cleaned up test source file\n");
    }
    
    /* Clean up any remaining temporary files */
    system("rm -rf ./dump/ ./dump_all/ ./dump_out/ ./dump_err/ ./dump_fork/ ./dump_verbose/ 2>/dev/null");
    system("rm -f " TEMP_OBJECT_FILE " custom_output.o fork_output.o 2>/dev/null");
    system("rm -f *.i *.s 2>/dev/null");
    
    fprintf(stderr, "All GCC invocations completed\n");
    return EXIT_SUCCESS;
}
