#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_gcc_coverage.c"
#define TEMP_OBJECT_FILE "test_gcc_coverage.o"

/* Create a simple valid C source file */
static int create_test_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen");
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
        perror("system");
        return -1;
    }
    
    if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        fprintf(stderr, "GCC exited with status: %d\n\n", exit_status);
        return exit_status;
    } else if (WIFSIGNALED(status)) {
        fprintf(stderr, "GCC terminated by signal: %d\n\n", WTERMSIG(status));
        return -1;
    }
    
    return 0;
}

int main(void) {
    char cmd[1024];
    int ret;
    
    /* Step 1: Create a valid C source file */
    fprintf(stderr, "Creating test source file: %s\n", TEMP_SOURCE_FILE);
    if (create_test_source(TEMP_SOURCE_FILE) != 0) {
        fprintf(stderr, "Failed to create test source file\n");
        return 1;
    }
    
    /* Step 2: Multiple distinct GCC invocations with different option combinations */
    
    /* Invocation 1: With -dumpdir and -save-temps */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -save-temps=cwd -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OBJECT_FILE);
    run_gcc_command(cmd);
    
    /* Invocation 2: With -dumpbase and -dumpbase-ext but no -dumpdir */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpbase myprog -dumpbase-ext .c -c %s -o %s_2.o",
             TEMP_SOURCE_FILE, TEMP_SOURCE_FILE);
    run_gcc_command(cmd);
    
    /* Invocation 3: With all options including -o with non-default name */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump_all/ -dumpbase fulltest -dumpbase-ext .c "
             "-save-temps -o custom_output.o -c %s",
             TEMP_SOURCE_FILE);
    run_gcc_command(cmd);
    
    /* Invocation 4: With -dumpdir and different suffix combinations */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump2 -dumpbase test2 -dumpbase-ext .x "
             "-save-temps=cwd -c %s -o test2.o",
             TEMP_SOURCE_FILE);
    run_gcc_command(cmd);
    
    /* Invocation 5: Minimal options to ensure basic compilation works */
    snprintf(cmd, sizeof(cmd),
             "gcc -c %s -o minimal.o",
             TEMP_SOURCE_FILE);
    run_gcc_command(cmd);
    
    /* Invocation 6: Trigger error exit with invalid option, then cleanup */
    snprintf(cmd, sizeof(cmd),
             "gcc -invalid-opt -c %s -o error.o 2>/dev/null",
             TEMP_SOURCE_FILE);
    run_gcc_command(cmd);
    
    /* Invocation 7: Test with empty dumpdir */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir '' -dumpbase empty -save-temps -c %s -o empty.o",
             TEMP_SOURCE_FILE);
    run_gcc_command(cmd);
    
    /* Invocation 8: Test with dumpdir containing special characters */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir './dump-dir' -dumpbase 'special-chars' "
             "-dumpbase-ext '.c' -save-temps -c %s -o special.o",
             TEMP_SOURCE_FILE);
    run_gcc_command(cmd);
    
    /* Invocation 9: Test with very long dumpbase (to test allocation) */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpbase 'very_long_dumpbase_name_that_might_trigger_edge_cases_in_allocation' "
             "-c %s -o long.o",
             TEMP_SOURCE_FILE);
    run_gcc_command(cmd);
    
    /* Invocation 10: Test combination that might trigger save_temps_flag variations */
    snprintf(cmd, sizeof(cmd),
             "gcc -save-temps=obj -dumpdir ./objdir/ -dumpbase objtest "
             "-dumpbase-ext .c -c %s",
             TEMP_SOURCE_FILE);
    run_gcc_command(cmd);
    
    /* Step 3: Clean up temporary files */
    fprintf(stderr, "Cleaning up temporary files...\n");
    
    /* Remove the source file */
    if (unlink(TEMP_SOURCE_FILE) != 0) {
        perror("unlink source file");
    }
    
    /* Remove generated object files */
    char *objects[] = {
        TEMP_OBJECT_FILE,
        "test_gcc_coverage.c_2.o",
        "custom_output.o",
        "test2.o",
        "minimal.o",
        "error.o",
        "empty.o",
        "special.o",
        "long.o",
        NULL
    };
    
    for (int i = 0; objects[i] != NULL; i++) {
        if (unlink(objects[i]) != 0 && errno != ENOENT) {
            fprintf(stderr, "Warning: Could not remove %s: %s\n", 
                    objects[i], strerror(errno));
        }
    }
    
    /* Clean up dump directories if they exist */
    system("rm -rf ./dump ./dump_all ./dump2 ./dump-dir ./objdir 2>/dev/null");
    
    /* Clean up save-temps files */
    system("rm -f *.i *.s *.o 2>/dev/null");
    
    fprintf(stderr, "Test completed successfully\n");
    return 0;
}
