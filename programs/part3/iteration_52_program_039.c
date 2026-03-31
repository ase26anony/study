#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_source.c"
#define TEMP_OUTPUT_FILE "test_coverage_output.o"

/* Create a simple valid C source file */
static int create_test_source(void) {
    FILE *fp = fopen(TEMP_SOURCE_FILE, "w");
    if (!fp) {
        fprintf(stderr, "Failed to create source file: %s\n", strerror(errno));
        return -1;
    }
    
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/* Clean up temporary files */
static void cleanup_files(void) {
    unlink(TEMP_SOURCE_FILE);
    unlink(TEMP_OUTPUT_FILE);
    
    /* Also clean up any dump files that might have been created */
    unlink("myprog.c.*");
    unlink("./dump/myprog.c.*");
    unlink("test_coverage_source.c.*");
    system("rm -rf ./dump 2>/dev/null");
}

/* Execute GCC command and wait for completion */
static int execute_gcc(const char *cmd) {
    fprintf(stderr, "Executing: %s\n", cmd);
    
    int status = system(cmd);
    if (status == -1) {
        fprintf(stderr, "Failed to execute command\n");
        return -1;
    }
    
    if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        fprintf(stderr, "GCC exited with status: %d\n", exit_status);
        return exit_status;
    } else if (WIFSIGNALED(status)) {
        fprintf(stderr, "GCC terminated by signal: %d\n", WTERMSIG(status));
        return -1;
    }
    
    return 0;
}

int main(void) {
    char cmd[1024];
    int ret;
    
    /* Create test source file */
    if (create_test_source() != 0) {
        return 1;
    }
    
    /* Ensure we clean up on exit */
    atexit(cleanup_files);
    
    /* 1. Invocation with -dumpdir and -save-temps */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -dumpbase myprog -dumpbase-ext .c "
             "-save-temps=cwd -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    execute_gcc(cmd);
    
    /* 2. Invocation with -dumpbase and -dumpbase-ext but no -dumpdir */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpbase myprog2 -dumpbase-ext .c "
             "-c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    execute_gcc(cmd);
    
    /* 3. Invocation with all options */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -dumpbase myprog3 -dumpbase-ext .c "
             "-save-temps -o %s %s",
             TEMP_OUTPUT_FILE, TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    /* 4. Another variation with different dumpdir */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dumps/ -dumpbase test -dumpbase-ext .foo "
             "-save-temps=obj -c %s",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    /* 5. Invocation with -o option to influence outbase */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpbase myprog5 -o custom_output.o -c %s",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    /* 6. Test cleanup after error exit (invalid option) */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -dumpbase error_test -invalid-opt %s",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    /* 7. Test with SAVE_TEMPS_NONE (no -save-temps) but with dump options */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./final/ -dumpbase final -dumpbase-ext .c "
             "-c %s",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    /* 8. Test with empty dumpdir (edge case) */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir '' -dumpbase empty -c %s",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    /* 9. Test with very long dumpbase to ensure proper allocation */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpbase very_long_dumpbase_name_that_might_trigger_edge_cases "
             "-c %s",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    /* 10. Final comprehensive test with all variables */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./comprehensive/ -dumpbase comprehensive "
             "-dumpbase-ext .ext -save-temps=obj -o final_output.o -c %s",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    fprintf(stderr, "All GCC invocations completed\n");
    
    /* Cleanup will happen via atexit */
    return 0;
}
