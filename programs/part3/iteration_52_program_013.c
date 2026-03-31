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
        fprintf(stderr, "Failed to create test source file: %s\n", strerror(errno));
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
        fprintf(stderr, "execvp failed: %s\n", strerror(errno));
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
    int result;
    
    /* Create test source file */
    if (create_test_source(TEMP_SOURCE_FILE) < 0) {
        return EXIT_FAILURE;
    }
    
    fprintf(stderr, "=== Starting GCC coverage test ===\n\n");
    
    /* Test 1: Basic compilation with dumpdir and save-temps */
    fprintf(stderr, "Test 1: Basic with -dumpdir and -save-temps\n");
    result = run_gcc("-dumpdir ./dump/ -dumpbase myprog -dumpbase-ext .c "
                     "-save-temps=cwd -c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 2: With dumpbase and dumpbase-ext but no dumpdir */
    fprintf(stderr, "Test 2: With -dumpbase and -dumpbase-ext (no -dumpdir)\n");
    result = run_gcc("-dumpbase test2 -dumpbase-ext .c -c " TEMP_SOURCE_FILE 
                     " -o test2_output.o");
    
    /* Test 3: All options combined */
    fprintf(stderr, "Test 3: All options combined\n");
    result = run_gcc("-dumpdir ./full_dump/ -dumpbase fulltest -dumpbase-ext .c "
                     "-save-temps -o full_output " TEMP_SOURCE_FILE);
    
    /* Test 4: Different save-temps option */
    fprintf(stderr, "Test 4: Different -save-temps option\n");
    result = run_gcc("-dumpdir ./tempdump/ -dumpbase temp -dumpbase-ext .c "
                     "-save-temps=obj -c " TEMP_SOURCE_FILE " -o temp_output.o");
    
    /* Test 5: Without dumpbase-ext */
    fprintf(stderr, "Test 5: Without -dumpbase-ext\n");
    result = run_gcc("-dumpdir ./simple/ -dumpbase simple -save-temps "
                     "-c " TEMP_SOURCE_FILE " -o simple_output.o");
    
    /* Test 6: With invalid option to test cleanup after error */
    fprintf(stderr, "Test 6: With invalid option (testing cleanup after error)\n");
    result = run_gcc("-dumpdir ./errordump/ -dumpbase error -dumpbase-ext .c "
                     "-invalid-opt -c " TEMP_SOURCE_FILE);
    
    /* Test 7: Minimal options to ensure basic cleanup path */
    fprintf(stderr, "Test 7: Minimal options\n");
    result = run_gcc("-dumpbase minimal -c " TEMP_SOURCE_FILE " -o minimal.o");
    
    /* Test 8: With outbase influenced by -o option */
    fprintf(stderr, "Test 8: Testing -o influence on outbase\n");
    result = run_gcc("-dumpdir ./outtest/ -dumpbase outtest -dumpbase-ext .c "
                     "-save-temps -o custom_output_name " TEMP_SOURCE_FILE);
    
    /* Clean up temporary files */
    fprintf(stderr, "=== Cleaning up temporary files ===\n");
    unlink(TEMP_SOURCE_FILE);
    unlink(TEMP_OUTPUT_FILE);
    unlink("test2_output.o");
    unlink("full_output");
    unlink("temp_output.o");
    unlink("simple_output.o");
    unlink("minimal.o");
    unlink("custom_output_name");
    
    /* Also clean up any .i, .s, .o files created by -save-temps */
    system("rm -f *.i *.s *.o ./dump/* ./full_dump/* ./tempdump/* ./simple/* ./errordump/* ./outtest/* 2>/dev/null");
    system("rmdir ./dump ./full_dump ./tempdump ./simple ./errordump ./outtest 2>/dev/null");
    
    fprintf(stderr, "\n=== GCC coverage test completed ===\n");
    
    return EXIT_SUCCESS;
}
