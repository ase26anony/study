#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_input.c"
#define TEMP_OBJECT_FILE "test_coverage_output.o"

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

/* Execute GCC command and wait for completion */
static int run_gcc_command(const char *command) {
    fprintf(stderr, "Executing: %s\n", command);
    
    int status = system(command);
    if (status == -1) {
        fprintf(stderr, "Failed to execute command: %s\n", strerror(errno));
        return -1;
    }
    
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        fprintf(stderr, "GCC exited with code: %d\n", exit_code);
        return exit_code;
    } else if (WIFSIGNALED(status)) {
        fprintf(stderr, "GCC terminated by signal: %d\n", WTERMSIG(status));
        return -1;
    }
    
    return 0;
}

int main(void) {
    int overall_status = 0;
    
    /* Step 1: Create a valid C source file */
    if (create_test_source(TEMP_SOURCE_FILE) != 0) {
        return 1;
    }
    
    /* Step 2: Execute multiple GCC invocations with different option combinations */
    
    /* Invocation 1: With dumpdir and save-temps */
    {
        char command[1024];
        snprintf(command, sizeof(command),
                 "gcc -dumpdir ./dump/ -save-temps=cwd -c %s -o %s",
                 TEMP_SOURCE_FILE, TEMP_OBJECT_FILE);
        
        if (run_gcc_command(command) != 0) {
            fprintf(stderr, "Warning: First invocation failed\n");
        }
    }
    
    /* Invocation 2: With dumpbase and dumpbase-ext but no dumpdir */
    {
        char command[1024];
        snprintf(command, sizeof(command),
                 "gcc -dumpbase myprog -dumpbase-ext .c -c %s -o %s_2",
                 TEMP_SOURCE_FILE, TEMP_OBJECT_FILE);
        
        if (run_gcc_command(command) != 0) {
            fprintf(stderr, "Warning: Second invocation failed\n");
        }
    }
    
    /* Invocation 3: With all options (dumpdir, dumpbase, dumpbase-ext, save-temps, -o) */
    {
        char command[1024];
        snprintf(command, sizeof(command),
                 "gcc -dumpdir ./dump_all/ -dumpbase full_coverage "
                 "-dumpbase-ext .c -save-temps -o %s_all %s",
                 TEMP_OBJECT_FILE, TEMP_SOURCE_FILE);
        
        if (run_gcc_command(command) != 0) {
            fprintf(stderr, "Warning: Third invocation failed\n");
        }
    }
    
    /* Invocation 4: With outbase influenced by -o option */
    {
        char command[1024];
        snprintf(command, sizeof(command),
                 "gcc -dumpbase custom_base -o custom_output.o -c %s",
                 TEMP_SOURCE_FILE);
        
        if (run_gcc_command(command) != 0) {
            fprintf(stderr, "Warning: Fourth invocation failed\n");
        }
    }
    
    /* Invocation 5: Trigger cleanup after error exit (invalid option) */
    {
        char command[1024];
        snprintf(command, sizeof(command),
                 "gcc -dumpdir ./error_dump/ -dumpbase error_test "
                 "-invalid-opt -c %s",
                 TEMP_SOURCE_FILE);
        
        /* We expect this to fail, but it should still trigger cleanup */
        run_gcc_command(command);
        fprintf(stderr, "Note: Expected error from invalid option\n");
    }
    
    /* Invocation 6: Another combination with verbose flag */
    {
        char command[1024];
        snprintf(command, sizeof(command),
                 "gcc -v -dumpdir ./verbose_dump/ -dumpbase verbose "
                 "-dumpbase-ext .c -save-temps=cwd -c %s",
                 TEMP_SOURCE_FILE);
        
        if (run_gcc_command(command) != 0) {
            fprintf(stderr, "Warning: Sixth invocation failed\n");
        }
    }
    
    /* Invocation 7: Minimal options to ensure basic compilation works */
    {
        char command[1024];
        snprintf(command, sizeof(command),
                 "gcc -c %s",
                 TEMP_SOURCE_FILE);
        
        if (run_gcc_command(command) != 0) {
            fprintf(stderr, "Warning: Minimal invocation failed\n");
            overall_status = 1;
        }
    }
    
    /* Step 3: Clean up temporary files */
    if (unlink(TEMP_SOURCE_FILE) != 0) {
        fprintf(stderr, "Warning: Failed to remove %s: %s\n",
                TEMP_SOURCE_FILE, strerror(errno));
    }
    
    /* Clean up generated object files */
    unlink(TEMP_OBJECT_FILE);
    unlink("custom_output.o");
    
    /* Clean up dump directories if they exist */
    system("rm -rf ./dump ./dump_all ./error_dump ./verbose_dump 2>/dev/null");
    
    /* Clean up save-temps files */
    unlink("test_coverage_input.i");
    unlink("test_coverage_input.s");
    unlink("test_coverage_input.o");
    
    fprintf(stderr, "\nAll GCC invocations completed. Check coverage data.\n");
    
    return overall_status;
}
