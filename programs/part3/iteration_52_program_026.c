#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_gcc_coverage.c"
#define TEMP_OUTPUT_FILE "test_gcc_coverage.o"

/* Create a simple valid C source file */
static int create_test_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Failed to create %s: %s\n", filename, strerror(errno));
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

/* Execute GCC command and wait for completion */
static int run_gcc_command(const char *cmd) {
    fprintf(stderr, "Executing: %s\n", cmd);
    
    int status = system(cmd);
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
    
    /* Step 2: Construct and execute multiple GCC invocations */
    
    /* Invocation 1: With dumpdir and save-temps */
    char cmd1[512];
    snprintf(cmd1, sizeof(cmd1),
             "gcc -dumpdir ./dump_output/ -save-temps=cwd -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    
    if (run_gcc_command(cmd1) != 0) {
        fprintf(stderr, "Warning: First GCC invocation failed\n");
    }
    
    /* Invocation 2: With dumpbase and dumpbase-ext but no dumpdir */
    char cmd2[512];
    snprintf(cmd2, sizeof(cmd2),
             "gcc -dumpbase mytestprog -dumpbase-ext .c -c %s -o %s_2",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    
    if (run_gcc_command(cmd2) != 0) {
        fprintf(stderr, "Warning: Second GCC invocation failed\n");
    }
    
    /* Invocation 3: With all options combined */
    char cmd3[512];
    snprintf(cmd3, sizeof(cmd3),
             "gcc -dumpdir ./full_dump/ -dumpbase fulltest -dumpbase-ext .c "
             "-save-temps -o custom_output.o -c %s",
             TEMP_SOURCE_FILE);
    
    if (run_gcc_command(cmd3) != 0) {
        fprintf(stderr, "Warning: Third GCC invocation failed\n");
    }
    
    /* Invocation 4: With different output name to influence outbase */
    char cmd4[512];
    snprintf(cmd4, sizeof(cmd4),
             "gcc -dumpdir ./alt/ -dumpbase altprog -dumpbase-ext .txt "
             "-save-temps=obj -o different_output.o -c %s",
             TEMP_SOURCE_FILE);
    
    if (run_gcc_command(cmd4) != 0) {
        fprintf(stderr, "Warning: Fourth GCC invocation failed\n");
    }
    
    /* Invocation 5: Trigger cleanup after error exit */
    char cmd5[512];
    snprintf(cmd5, sizeof(cmd5),
             "gcc -dumpdir ./error_dump/ -dumpbase error -dumpbase-ext .err "
             "-invalid-opt -c %s",
             TEMP_SOURCE_FILE);
    
    if (run_gcc_command(cmd5) != 0) {
        fprintf(stderr, "Expected error from invalid option\n");
    }
    
    /* Invocation 6: Minimal options to ensure basic path is covered */
    char cmd6[512];
    snprintf(cmd6, sizeof(cmd6),
             "gcc -dumpbase minimal -c %s",
             TEMP_SOURCE_FILE);
    
    if (run_gcc_command(cmd6) != 0) {
        fprintf(stderr, "Warning: Minimal GCC invocation failed\n");
    }
    
    /* Step 3: Clean up temporary files */
    if (unlink(TEMP_SOURCE_FILE) != 0) {
        fprintf(stderr, "Warning: Failed to remove %s: %s\n", 
                TEMP_SOURCE_FILE, strerror(errno));
    }
    
    /* Clean up generated files if they exist */
    unlink(TEMP_OUTPUT_FILE);
    unlink("custom_output.o");
    unlink("different_output.o");
    unlink("test_gcc_coverage.o_2");
    
    /* Clean up dump directories */
    system("rm -rf ./dump_output/ ./full_dump/ ./alt/ ./error_dump/ 2>/dev/null");
    
    fprintf(stderr, "Test program completed. Check GCC driver coverage.\n");
    return overall_status;
}
