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
static void cleanup_temp_files(void) {
    unlink(TEMP_SOURCE_FILE);
    unlink(TEMP_OUTPUT_FILE);
    /* Clean up any dump files that might have been created */
    system("rm -f ./dump/*.i ./dump/*.s ./dump/*.o 2>/dev/null");
    system("rmdir ./dump 2>/dev/null 2>&1");
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
    
    /* Create test source file */
    if (create_test_source() != 0) {
        return 1;
    }
    
    /* Ensure we start clean */
    cleanup_temp_files();
    
    /* Create dump directory */
    system("mkdir -p ./dump 2>/dev/null");
    
    fprintf(stderr, "\n=== Starting GCC coverage test ===\n\n");
    
    /* Test 1: Basic compilation with dumpdir and save-temps */
    fprintf(stderr, "Test 1: Basic with -dumpdir and -save-temps\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -save-temps -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    execute_gcc(cmd);
    
    /* Test 2: With dumpbase and dumpbase-ext but no dumpdir */
    fprintf(stderr, "\nTest 2: With -dumpbase and -dumpbase-ext\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpbase myprog -dumpbase-ext .c -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    execute_gcc(cmd);
    
    /* Test 3: All options combined */
    fprintf(stderr, "\nTest 3: All options combined\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -dumpbase myprog2 -dumpbase-ext .c "
             "-save-temps=cwd -c %s -o custom_output.o",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    /* Test 4: Different save-temps option */
    fprintf(stderr, "\nTest 4: Different -save-temps option\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -dumpbase test3 -save-temps=obj "
             "-c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    execute_gcc(cmd);
    
    /* Test 5: With verbose flag (touches verbose_only_flag) */
    fprintf(stderr, "\nTest 5: With -v (verbose)\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -dumpbase verbose_test -v -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    execute_gcc(cmd);
    
    /* Test 6: With version flag (touches print_version) */
    fprintf(stderr, "\nTest 6: With --version\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ --version");
    execute_gcc(cmd);
    
    /* Test 7: Error case with invalid option */
    fprintf(stderr, "\nTest 7: Error case with invalid option\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -dumpbase error_test -invalid-opt -c %s",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    /* Test 8: Multiple output base variations */
    fprintf(stderr, "\nTest 8: Multiple output variations\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -dumpbase multi1 -o output1.o -c %s",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpbase multi2 -o output2.o -c %s",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    /* Test 9: Empty dumpdir (edge case) */
    fprintf(stderr, "\nTest 9: Empty dumpdir\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir '' -dumpbase emptydir -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    execute_gcc(cmd);
    
    /* Test 10: Complex dumpbase with path */
    fprintf(stderr, "\nTest 10: Complex dumpbase with path\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpbase ./dump/complex_base -dumpbase-ext .xyz "
             "-save-temps -c %s -o complex.o",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd);
    
    fprintf(stderr, "\n=== GCC coverage test completed ===\n");
    
    /* Clean up */
    cleanup_temp_files();
    
    return 0;
}
