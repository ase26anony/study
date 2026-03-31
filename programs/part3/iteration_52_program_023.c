#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_source.c"
#define TEMP_OBJECT_FILE "test_coverage_output.o"

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
    fprintf(fp, "    printf(\"Hello from coverage test!\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/* Execute GCC with specific options using fork/exec */
static int run_gcc_with_options(const char *options) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process - execute GCC */
        char *argv[64];
        int argc = 0;
        
        /* Split the options string into arguments */
        char *options_copy = strdup(options);
        char *token = strtok(options_copy, " ");
        
        argv[argc++] = "gcc";
        
        while (token && argc < 62) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
        argv[argc] = NULL;
        
        /* Execute GCC */
        execvp("gcc", argv);
        
        /* If we get here, exec failed */
        perror("execvp failed");
        free(options_copy);
        exit(EXIT_FAILURE);
    } else {
        /* Parent process - wait for child */
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

/* Clean up temporary files */
static void cleanup_files(void) {
    remove(TEMP_SOURCE_FILE);
    remove(TEMP_OBJECT_FILE);
    /* Clean up any dump files that might have been created */
    remove("myprog.c.*");  /* Pattern for dump files */
    remove("./dump/myprog.c.*");  /* Pattern for dump files in dump directory */
}

int main(void) {
    fprintf(stderr, "=== Starting GCC coverage test ===\n");
    
    /* Create test source file */
    if (create_test_source() < 0) {
        return EXIT_FAILURE;
    }
    
    /* Test 1: Basic compilation to trigger initialization */
    fprintf(stderr, "\nTest 1: Basic compilation\n");
    run_gcc_with_options("-c " TEMP_SOURCE_FILE " -o " TEMP_OBJECT_FILE);
    
    /* Test 2: With dumpdir and save-temps */
    fprintf(stderr, "\nTest 2: With -dumpdir and -save-temps\n");
    run_gcc_with_options("-dumpdir ./dump/ -save-temps=cwd -c " TEMP_SOURCE_FILE " -o " TEMP_OBJECT_FILE);
    
    /* Test 3: With dumpbase and dumpbase-ext */
    fprintf(stderr, "\nTest 3: With -dumpbase and -dumpbase-ext\n");
    run_gcc_with_options("-dumpbase myprog -dumpbase-ext .c -c " TEMP_SOURCE_FILE " -o " TEMP_OBJECT_FILE);
    
    /* Test 4: All options combined */
    fprintf(stderr, "\nTest 4: All dump options combined\n");
    run_gcc_with_options("-dumpdir ./dump/ -dumpbase myprog -dumpbase-ext .c -save-temps -c " TEMP_SOURCE_FILE " -o custom_output.o");
    
    /* Test 5: Different output base name */
    fprintf(stderr, "\nTest 5: Different output base\n");
    run_gcc_with_options("-dumpdir ./dump2/ -dumpbase different -dumpbase-ext .c -save-temps=cwd -c " TEMP_SOURCE_FILE " -o different_output.o");
    
    /* Test 6: Without dumpdir to test default behavior */
    fprintf(stderr, "\nTest 6: Without dumpdir (default behavior)\n");
    run_gcc_with_options("-dumpbase nodir -dumpbase-ext .c -save-temps -c " TEMP_SOURCE_FILE);
    
    /* Test 7: Error case - invalid option to test cleanup after error */
    fprintf(stderr, "\nTest 7: Error case with invalid option\n");
    run_gcc_with_options("-invalid-option -dumpdir ./dump/ -dumpbase error -c " TEMP_SOURCE_FILE);
    
    /* Test 8: Version and help flags (also referenced in uncovered lines) */
    fprintf(stderr, "\nTest 8: Version flag\n");
    run_gcc_with_options("--version");
    
    fprintf(stderr, "\nTest 9: Help flag\n");
    run_gcc_with_options("--help");
    
    /* Clean up */
    cleanup_files();
    
    fprintf(stderr, "\n=== GCC coverage test completed ===\n");
    return EXIT_SUCCESS;
}
