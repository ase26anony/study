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
        perror("Failed to create test source file");
        return -1;
    }
    
    fprintf(fp, "/* Test source for GCC coverage */\n");
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
    remove(TEMP_OUTPUT_FILE);
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
        fprintf(stderr, "GCC exited with status: %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        fprintf(stderr, "GCC terminated by signal: %d\n", WTERMSIG(status));
    }
    
    return 0;
}

int main(void) {
    char cmd[1024];
    int ret = 0;
    
    /* Clean up any previous test files */
    cleanup_files();
    
    /* Create test source file */
    if (create_test_source() != 0) {
        return EXIT_FAILURE;
    }
    
    fprintf(stderr, "\n=== Starting GCC coverage tests ===\n\n");
    
    /* Test 1: Basic invocation with dumpdir and save-temps */
    fprintf(stderr, "Test 1: -dumpdir with -save-temps\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -save-temps -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    execute_gcc(cmd);
    
    /* Test 2: dumpbase and dumpbase-ext without dumpdir */
    fprintf(stderr, "\nTest 2: -dumpbase and -dumpbase-ext\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpbase myprog -dumpbase-ext .c -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    execute_gcc(cmd);
    
    /* Test 3: All dump options with custom output */
    fprintf(stderr, "\nTest 3: All dump options with -save-temps and -o\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump_all/ -dumpbase alltest -dumpbase-ext .c "
             "-save-temps=cwd -c %s -o custom_output.o",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    /* Test 4: Different save-temps variant */
    fprintf(stderr, "\nTest 4: save-temps=obj with dump options\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump_obj/ -dumpbase objtest -dumpbase-ext .c "
             "-save-temps=obj -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    execute_gcc(cmd);
    
    /* Test 5: Minimal options to trigger basic cleanup */
    fprintf(stderr, "\nTest 5: Minimal compilation\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -c %s",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    /* Test 6: Error case with invalid option (should still trigger cleanup) */
    fprintf(stderr, "\nTest 6: Error case with invalid option\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -invalid-opt -c %s 2>/dev/null",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    /* Test 7: Combination with optimization flags */
    fprintf(stderr, "\nTest 7: With optimization and dump options\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -O2 -dumpdir ./dump_opt/ -dumpbase opttest -dumpbase-ext .c "
             "-save-temps -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    execute_gcc(cmd);
    
    /* Test 8: Empty dumpdir (edge case) */
    fprintf(stderr, "\nTest 8: Empty dumpdir\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir '' -dumpbase emptydir -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    execute_gcc(cmd);
    
    /* Test 9: Long dumpbase (stress test) */
    fprintf(stderr, "\nTest 9: Long dumpbase name\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpbase very_long_dumpbase_name_for_testing_coverage "
             "-dumpbase-ext .very_long_extension -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    execute_gcc(cmd);
    
    /* Test 10: Multiple dump options with different output base */
    fprintf(stderr, "\nTest 10: Complex combination\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./final_dump/ -dumpbase final -dumpbase-ext .c "
             "-save-temps=obj -O0 -g -c %s -o final_output.o",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    fprintf(stderr, "\n=== GCC coverage tests completed ===\n");
    
    /* Clean up temporary files */
    cleanup_files();
    
    return EXIT_SUCCESS;
}
