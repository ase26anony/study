#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_gcc_coverage.c"
#define TEMP_OUTPUT_FILE "test_gcc_coverage.o"

/* Create a simple valid C source file for compilation */
static int create_test_source(void)
{
    FILE *fp = fopen(TEMP_SOURCE_FILE, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return -1;
    }
    
    fprintf(fp, "/* Test file for GCC coverage */\n");
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    printf(\"Hello from GCC coverage test\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/* Execute GCC with specific options using fork/exec */
static int run_gcc(const char *description, const char *arg1, const char *arg2, 
                   const char *arg3, const char *arg4, const char *arg5,
                   const char *arg6, const char *arg7)
{
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process - execute GCC */
        const char *argv[20];
        int i = 0;
        
        argv[i++] = "gcc";
        
        /* Add all non-NULL arguments */
        if (arg1) argv[i++] = arg1;
        if (arg2) argv[i++] = arg2;
        if (arg3) argv[i++] = arg3;
        if (arg4) argv[i++] = arg4;
        if (arg5) argv[i++] = arg5;
        if (arg6) argv[i++] = arg6;
        if (arg7) argv[i++] = arg7;
        
        /* Always compile our test file */
        argv[i++] = "-c";
        argv[i++] = TEMP_SOURCE_FILE;
        argv[i++] = "-o";
        argv[i++] = TEMP_OUTPUT_FILE;
        
        argv[i] = NULL;
        
        /* Print the command for traceability */
        fprintf(stderr, "Running: ");
        for (int j = 0; j < i; j++) {
            fprintf(stderr, "%s ", argv[j]);
        }
        fprintf(stderr, "\n");
        
        execvp("gcc", (char *const *)argv);
        
        /* If we get here, exec failed */
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        /* Parent process - wait for child */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            fprintf(stderr, "%s: GCC exited with status %d\n", 
                    description, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "%s: GCC terminated by signal %d\n", 
                    description, WTERMSIG(status));
        }
        
        /* Clean up the output file if it was created */
        unlink(TEMP_OUTPUT_FILE);
        
        return 0;
    }
}

/* Alternative using system() for simpler option combinations */
static int run_gcc_system(const char *command)
{
    fprintf(stderr, "Running: %s\n", command);
    int result = system(command);
    fprintf(stderr, "Exit status: %d\n", result);
    
    /* Clean up the output file */
    unlink(TEMP_OUTPUT_FILE);
    
    return result;
}

int main(void)
{
    /* Create the test source file */
    if (create_test_source() != 0) {
        return EXIT_FAILURE;
    }
    
    fprintf(stderr, "=== Starting GCC coverage test ===\n");
    
    /* Test 1: Basic compilation to establish baseline */
    fprintf(stderr, "\n--- Test 1: Basic compilation ---\n");
    run_gcc("Basic compilation", NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    
    /* Test 2: With -dumpdir and -save-temps */
    fprintf(stderr, "\n--- Test 2: -dumpdir and -save-temps ---\n");
    run_gcc("-dumpdir and -save-temps", 
            "-dumpdir", "./dumpdir_test/",
            "-save-temps", 
            NULL, NULL, NULL, NULL);
    
    /* Test 3: With -dumpbase and -dumpbase-ext (no -dumpdir) */
    fprintf(stderr, "\n--- Test 3: -dumpbase and -dumpbase-ext ---\n");
    run_gcc("-dumpbase and -dumpbase-ext",
            "-dumpbase", "myprog",
            "-dumpbase-ext", ".c",
            NULL, NULL, NULL);
    
    /* Test 4: All dump options combined with -save-temps and custom -o */
    fprintf(stderr, "\n--- Test 4: All dump options with -save-temps ---\n");
    run_gcc("All dump options",
            "-dumpdir", "./full_dump/",
            "-dumpbase", "full_coverage",
            "-dumpbase-ext", ".test",
            "-save-temps=cwd",
            NULL, NULL);
    
    /* Test 5: Using system() for more complex command line */
    fprintf(stderr, "\n--- Test 5: Complex command with system() ---\n");
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -dumpbase complex -dumpbase-ext .c "
             "-save-temps -o custom_output.o -c %s",
             TEMP_SOURCE_FILE);
    run_gcc_system(cmd);
    unlink("custom_output.o");
    
    /* Test 6: Invalid option to test cleanup after error */
    fprintf(stderr, "\n--- Test 6: Invalid option (testing error cleanup) ---\n");
    run_gcc("Invalid option",
            "-dumpdir", "./error_test/",
            "-invalid-opt",  /* This should cause an error */
            NULL, NULL, NULL, NULL);
    
    /* Test 7: Multiple -dumpdir variations */
    fprintf(stderr, "\n--- Test 7: Multiple -dumpdir variations ---\n");
    
    /* With trailing slash */
    run_gcc("dumpdir with slash",
            "-dumpdir", "./with_slash/",
            "-save-temps",
            NULL, NULL, NULL, NULL);
    
    /* Without trailing slash */
    run_gcc("dumpdir without slash",
            "-dumpdir", "./without_slash",
            "-save-temps",
            NULL, NULL, NULL, NULL);
    
    /* Empty dumpdir (current directory) */
    run_gcc("empty dumpdir",
            "-dumpdir", "",
            "-save-temps",
            NULL, NULL, NULL, NULL);
    
    /* Test 8: Combination that should trigger outbase allocation */
    fprintf(stderr, "\n--- Test 8: Testing outbase with custom output ---\n");
    run_gcc("Custom output name",
            "-dumpbase", "outbase_test",
            "-o", "custom_name.o",
            NULL, NULL, NULL, NULL);
    unlink("custom_name.o");
    
    /* Clean up intermediate files created by -save-temps */
    fprintf(stderr, "\n--- Cleaning up temporary files ---\n");
    unlink("test_gcc_coverage.i");  /* Preprocessed output */
    unlink("test_gcc_coverage.s");  /* Assembly output */
    
    /* Remove the dump directory if created */
    system("rm -rf ./dumpdir_test ./dump ./full_dump ./with_slash ./without_slash ./error_test");
    
    /* Clean up the source file */
    unlink(TEMP_SOURCE_FILE);
    
    fprintf(stderr, "\n=== GCC coverage test completed ===\n");
    
    return EXIT_SUCCESS;
}
