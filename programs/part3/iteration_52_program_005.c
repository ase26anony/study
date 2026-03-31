#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_gcc_coverage.c"
#define TEMP_OBJ_FILE "test_gcc_coverage.o"

/* Create a simple valid C source file for compilation */
static int create_test_source(void) {
    FILE *fp = fopen(TEMP_SOURCE_FILE, "w");
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

/* Clean up temporary files */
static void cleanup_files(void) {
    unlink(TEMP_SOURCE_FILE);
    unlink(TEMP_OBJ_FILE);
    /* Clean up any dump files that might have been created */
    system("rm -f ./dump/*.i ./dump/*.s ./dump/*.o 2>/dev/null");
    system("rmdir ./dump 2>/dev/null");
}

/* Execute GCC with the given arguments using fork/exec */
static int run_gcc(const char *args) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
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

int main(void) {
    int overall_status = 0;
    
    /* Create test source file */
    if (create_test_source() != 0) {
        return EXIT_FAILURE;
    }
    
    /* Create dump directory if it doesn't exist */
    system("mkdir -p ./dump 2>/dev/null");
    
    fprintf(stderr, "=== Starting GCC coverage test ===\n\n");
    
    /* Test 1: Basic compilation with dumpdir and save-temps */
    fprintf(stderr, "Test 1: Basic compilation with -dumpdir and -save-temps\n");
    overall_status |= run_gcc("-dumpdir ./dump/ -save-temps -c " TEMP_SOURCE_FILE " -o " TEMP_OBJ_FILE);
    
    /* Test 2: With dumpbase and dumpbase-ext but no dumpdir */
    fprintf(stderr, "Test 2: With -dumpbase and -dumpbase-ext but no -dumpdir\n");
    overall_status |= run_gcc("-dumpbase myprog -dumpbase-ext .c -save-temps=cwd -c " TEMP_SOURCE_FILE " -o " TEMP_OBJ_FILE);
    
    /* Test 3: All options combined */
    fprintf(stderr, "Test 3: All dump options combined with -save-temps and custom -o\n");
    overall_status |= run_gcc("-dumpdir ./dump/ -dumpbase myprog -dumpbase-ext .c -save-temps -c " TEMP_SOURCE_FILE " -o custom_output.o");
    
    /* Test 4: Different output base name */
    fprintf(stderr, "Test 4: Different output base with dumpdir\n");
    overall_status |= run_gcc("-dumpdir ./dump/ -dumpbase different -dumpbase-ext .xyz -save-temps -c " TEMP_SOURCE_FILE " -o different.o");
    
    /* Test 5: Without save-temps but with dump options */
    fprintf(stderr, "Test 5: Dump options without -save-temps\n");
    overall_status |= run_gcc("-dumpdir ./dump/ -dumpbase notemp -dumpbase-ext .c -c " TEMP_SOURCE_FILE " -o " TEMP_OBJ_FILE);
    
    /* Test 6: Error case - invalid option to test cleanup after error */
    fprintf(stderr, "Test 6: Invalid option to test cleanup after error\n");
    overall_status |= run_gcc("-dumpdir ./dump/ -dumpbase error -dumpbase-ext .c -invalid-opt -c " TEMP_SOURCE_FILE " -o " TEMP_OBJ_FILE);
    
    /* Test 7: Minimal compilation to ensure basic path is covered */
    fprintf(stderr, "Test 7: Minimal compilation (no dump options)\n");
    overall_status |= run_gcc("-c " TEMP_SOURCE_FILE " -o " TEMP_OBJ_FILE);
    
    /* Test 8: With verbose flag to trigger verbose_only_flag */
    fprintf(stderr, "Test 8: With -v flag\n");
    overall_status |= run_gcc("-v -dumpdir ./dump/ -dumpbase verbose -dumpbase-ext .c -save-temps -c " TEMP_SOURCE_FILE " -o " TEMP_OBJ_FILE);
    
    /* Test 9: With version flag to trigger print_version */
    fprintf(stderr, "Test 9: With --version flag\n");
    overall_status |= run_gcc("--version -dumpdir ./dump/ -dumpbase version -dumpbase-ext .c");
    
    /* Test 10: With help flag to trigger print_help_list */
    fprintf(stderr, "Test 10: With --help flag\n");
    overall_status |= run_gcc("--help -dumpdir ./dump/ -dumpbase help -dumpbase-ext .c");
    
    fprintf(stderr, "=== GCC coverage test completed ===\n");
    
    /* Clean up temporary files */
    cleanup_files();
    
    return overall_status == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
