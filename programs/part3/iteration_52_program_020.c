#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_source.c"
#define TEMP_OBJECT_FILE "test_coverage_output.o"

/* Create a simple valid C source file */
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

/* Clean up temporary files */
static void cleanup_files(void) {
    remove(TEMP_SOURCE_FILE);
    remove(TEMP_OBJECT_FILE);
    /* Clean up any dump files that might have been created */
    system("rm -rf ./dump 2>/dev/null");
    system("rm -f *.i *.s *.o 2>/dev/null");
}

/* Execute GCC command and wait for completion */
static int execute_gcc(const char *cmd) {
    fprintf(stderr, "Executing: %s\n", cmd);
    
    int status = system(cmd);
    if (status == -1) {
        perror("system() failed");
        return -1;
    }
    
    if (WIFEXITED(status)) {
        fprintf(stderr, "GCC exited with status: %d\n\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        fprintf(stderr, "GCC terminated by signal: %d\n\n", WTERMSIG(status));
    }
    
    return 0;
}

int main(void) {
    char cmd[1024];
    
    /* Clean up any previous run */
    cleanup_files();
    
    /* Create test source file */
    if (create_test_source() != 0) {
        return 1;
    }
    
    fprintf(stderr, "=== Starting GCC coverage test ===\n\n");
    
    /* Test 1: Basic invocation with dumpdir and save-temps */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -save-temps=cwd -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OBJECT_FILE);
    execute_gcc(cmd);
    
    /* Test 2: With dumpbase and dumpbase-ext but no dumpdir */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpbase myprog -dumpbase-ext .c -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OBJECT_FILE);
    execute_gcc(cmd);
    
    /* Test 3: All options combined */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -dumpbase myprog -dumpbase-ext .c "
             "-save-temps -o custom_output.o -c %s",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    /* Test 4: Different save-temps option */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -dumpbase altprog -save-temps=obj "
             "-c %s -o alt_output.o",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    /* Test 5: Without -o option (uses default) */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -dumpbase defaultprog -dumpbase-ext .c "
             "-save-temps -c %s",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    /* Test 6: Minimal options to trigger basic cleanup */
    snprintf(cmd, sizeof(cmd),
             "gcc -c %s",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    /* Test 7: Invalid option to test cleanup after error */
    fprintf(stderr, "\n=== Testing cleanup after error ===\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -dumpbase errorprog -invalid-opt -c %s",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    /* Test 8: With verbose flag (verbose_only_flag in target block) */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -v -c %s -o verbose_output.o",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    /* Test 9: With version flag (print_version in target block) */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ --version -c %s 2>/dev/null",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    /* Test 10: Complex combination with multiple flags */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -dumpbase complex -dumpbase-ext .c "
             "-save-temps=cwd -o final_output.o -v -c %s 2>&1 | head -5",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    fprintf(stderr, "\n=== Cleaning up ===\n");
    cleanup_files();
    
    fprintf(stderr, "Coverage test completed.\n");
    return 0;
}
