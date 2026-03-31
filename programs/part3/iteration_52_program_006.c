#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_gcc_coverage.c"
#define TEMP_OUTPUT_FILE "test_gcc_coverage.o"

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
    fprintf(fp, "    printf(\"Hello from test program\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/* Clean up temporary files */
static void cleanup_files(void) {
    remove(TEMP_SOURCE_FILE);
    remove(TEMP_OUTPUT_FILE);
    /* Clean up any dump files that might have been created */
    system("rm -rf ./dump 2>/dev/null");
    system("rm -f *.i *.s *.o 2>/dev/null");
}

/* Execute GCC command and wait for completion */
static int run_gcc_command(const char *cmd) {
    fprintf(stderr, "Executing: %s\n", cmd);
    
    int status = system(cmd);
    if (status == -1) {
        perror("system() failed");
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
    
    /* Clean up any existing files first */
    cleanup_files();
    
    /* Create test source file */
    if (create_test_source() != 0) {
        return 1;
    }
    
    fprintf(stderr, "\n=== Starting GCC coverage test ===\n\n");
    
    /* Test 1: Basic compilation to ensure GCC works */
    fprintf(stderr, "Test 1: Basic compilation\n");
    snprintf(cmd, sizeof(cmd), "gcc -c %s -o %s", 
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    run_gcc_command(cmd);
    
    /* Test 2: With dumpdir and save-temps (triggers dumpdir allocation) */
    fprintf(stderr, "\nTest 2: With -dumpdir and -save-temps\n");
    snprintf(cmd, sizeof(cmd), 
             "gcc -dumpdir ./dump/ -save-temps=cwd -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    run_gcc_command(cmd);
    
    /* Test 3: With dumpbase and dumpbase-ext but no dumpdir */
    fprintf(stderr, "\nTest 3: With -dumpbase and -dumpbase-ext\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpbase myprog -dumpbase-ext .c -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    run_gcc_command(cmd);
    
    /* Test 4: With all dump options, save-temps, and custom -o */
    fprintf(stderr, "\nTest 4: All dump options with -save-temps and custom -o\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -dumpbase myprog -dumpbase-ext .c "
             "-save-temps -o custom_output.o -c %s",
             TEMP_SOURCE_FILE);
    run_gcc_command(cmd);
    
    /* Test 5: With dumpdir and verbose flag (triggers verbose_only_flag) */
    fprintf(stderr, "\nTest 5: With -dumpdir and -v (verbose)\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -v -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    run_gcc_command(cmd);
    
    /* Test 6: With dumpdir and version flag (triggers print_version) */
    fprintf(stderr, "\nTest 6: With -dumpdir and --version\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ --version -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    run_gcc_command(cmd);
    
    /* Test 7: With dumpdir and help flag (triggers print_help_list) */
    fprintf(stderr, "\nTest 7: With -dumpdir and --help\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ --help -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    run_gcc_command(cmd);
    
    /* Test 8: Invalid option to test cleanup after error */
    fprintf(stderr, "\nTest 8: Invalid option to test error cleanup\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -invalid-opt -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    run_gcc_command(cmd);
    
    /* Test 9: Multiple dumpdir variations */
    fprintf(stderr, "\nTest 9: Multiple dumpdir variations\n");
    
    /* With trailing slash */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump_slash/ -save-temps -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    run_gcc_command(cmd);
    
    /* Without trailing slash */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump_noslash -save-temps -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    run_gcc_command(cmd);
    
    /* Empty dumpdir */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir '' -save-temps -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    run_gcc_command(cmd);
    
    /* Test 10: Combination that should trigger all relevant allocations */
    fprintf(stderr, "\nTest 10: Comprehensive test\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./final_dump/ -dumpbase comprehensive_test "
             "-dumpbase-ext .c -save-temps=cwd -o final_output.o -c %s",
             TEMP_SOURCE_FILE);
    run_gcc_command(cmd);
    
    fprintf(stderr, "\n=== GCC coverage test completed ===\n");
    
    /* Clean up temporary files */
    cleanup_files();
    
    return 0;
}
