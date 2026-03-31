#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_gcc_cleanup.c"
#define TEMP_OBJECT_FILE "test_gcc_cleanup.o"

/* Create a minimal valid C source file */
static int create_test_source(void) {
    FILE *f = fopen(TEMP_SOURCE_FILE, "w");
    if (!f) {
        perror("Failed to create test source file");
        return -1;
    }
    
    fprintf(f, "/* Test file for GCC cleanup coverage */\n");
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main(void) {\n");
    fprintf(f, "    printf(\"Hello from test program\\n\");\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
    return 0;
}

/* Execute GCC with given arguments and wait for completion */
static int run_gcc(const char *args) {
    char command[1024];
    int status;
    
    /* Construct the full command */
    snprintf(command, sizeof(command), "gcc %s", args);
    
    /* Print for traceability */
    fprintf(stderr, "Executing: %s\n", command);
    
    /* Execute using system() - simpler and meets requirements */
    status = system(command);
    
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

/* Clean up temporary files */
static void cleanup_files(void) {
    remove(TEMP_SOURCE_FILE);
    remove(TEMP_OBJECT_FILE);
    /* Also clean up any dump files that might have been created */
    system("rm -f ./dump/* 2>/dev/null");
    system("rmdir ./dump 2>/dev/null 2>&1");
}

int main(void) {
    int ret = 0;
    
    /* Create the test source file */
    if (create_test_source() != 0) {
        return EXIT_FAILURE;
    }
    
    fprintf(stderr, "\n=== Starting GCC cleanup coverage test ===\n\n");
    
    /* Test 1: Basic invocation with dumpdir and save-temps */
    fprintf(stderr, "Test 1: -dumpdir with -save-temps\n");
    run_gcc("-dumpdir ./dump/ -save-temps -c " TEMP_SOURCE_FILE " -o " TEMP_OBJECT_FILE);
    
    /* Test 2: dumpbase and dumpbase-ext without dumpdir */
    fprintf(stderr, "\nTest 2: -dumpbase and -dumpbase-ext\n");
    run_gcc("-dumpbase myprog -dumpbase-ext .c -c " TEMP_SOURCE_FILE " -o " TEMP_OBJECT_FILE);
    
    /* Test 3: All options combined */
    fprintf(stderr, "\nTest 3: All dump options with -save-temps and custom -o\n");
    run_gcc("-dumpdir ./dump/ -dumpbase myprog -dumpbase-ext .c -save-temps=cwd "
            "-c " TEMP_SOURCE_FILE " -o custom_output.o");
    
    /* Test 4: Different save-temps variant */
    fprintf(stderr, "\nTest 4: -save-temps=obj with dump options\n");
    run_gcc("-dumpdir ./dump/ -dumpbase altprog -dumpbase-ext .c -save-temps=obj "
            "-c " TEMP_SOURCE_FILE " -o alt_output.o");
    
    /* Test 5: Trigger error exit to test cleanup after failure */
    fprintf(stderr, "\nTest 5: Invalid option to test cleanup after error\n");
    run_gcc("-dumpdir ./dump/ -dumpbase error -dumpbase-ext .c -invalid-opt "
            "-c " TEMP_SOURCE_FILE " 2>/dev/null");
    
    /* Test 6: Minimal compilation to ensure basic path is covered */
    fprintf(stderr, "\nTest 6: Minimal compilation (baseline)\n");
    run_gcc("-c " TEMP_SOURCE_FILE);
    
    /* Test 7: With outbase influence through -o option */
    fprintf(stderr, "\nTest 7: Explicit -o to influence outbase\n");
    run_gcc("-dumpdir ./dump/ -dumpbase explicit -dumpbase-ext .c "
            "-c " TEMP_SOURCE_FILE " -o explicit_output.o");
    
    /* Test 8: Combination that might trigger dumpdir_trailing_dash_added logic */
    fprintf(stderr, "\nTest 8: Dumpdir without trailing slash\n");
    run_gcc("-dumpdir ./dump -dumpbase notrail -dumpbase-ext .c "
            "-c " TEMP_SOURCE_FILE " -o notrail.o");
    
    fprintf(stderr, "\n=== GCC cleanup coverage test complete ===\n");
    
    /* Clean up temporary files */
    cleanup_files();
    
    /* Clean up additional output files from tests */
    remove("custom_output.o");
    remove("alt_output.o");
    remove("explicit_output.o");
    remove("notrail.o");
    remove("custom_output.i");
    remove("custom_output.s");
    remove("alt_output.i");
    remove("alt_output.s");
    remove("explicit_output.i");
    remove("explicit_output.s");
    remove("notrail.i");
    remove("notrail.s");
    
    return ret;
}
