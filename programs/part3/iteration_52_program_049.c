#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_source.c"
#define TEMP_OBJECT_FILE "test_coverage_output.o"

/* Create a simple valid C source file for compilation */
static int create_test_source(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Error creating source file %s: %s\n", 
                filename, strerror(errno));
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

/* Clean up temporary files */
static void cleanup_files(void) {
    remove(TEMP_SOURCE_FILE);
    remove(TEMP_OBJECT_FILE);
    /* Clean up dump directory if created */
    system("rm -rf ./dump 2>/dev/null");
    system("rm -rf ./temps 2>/dev/null");
}

int main(void) {
    char command[1024];
    int ret;
    
    /* Create test source file */
    if (create_test_source(TEMP_SOURCE_FILE) != 0) {
        return 1;
    }
    
    /* Register cleanup handler */
    atexit(cleanup_files);
    
    fprintf(stderr, "\n=== Starting GCC coverage test ===\n\n");
    
    /* Test 1: Basic compilation with dumpdir and save-temps */
    fprintf(stderr, "Test 1: Basic with -dumpdir and -save-temps\n");
    snprintf(command, sizeof(command),
             "gcc -dumpdir ./dump/ -dumpbase myprog -dumpbase-ext .c "
             "-save-temps=cwd -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OBJECT_FILE);
    run_gcc_command(command);
    
    /* Test 2: With dumpbase and dumpbase-ext but no dumpdir */
    fprintf(stderr, "\nTest 2: With -dumpbase and -dumpbase-ext (no -dumpdir)\n");
    snprintf(command, sizeof(command),
             "gcc -dumpbase testdump -dumpbase-ext .src "
             "-c %s -o test2.o",
             TEMP_SOURCE_FILE);
    run_gcc_command(command);
    remove("test2.o");
    
    /* Test 3: All options combined with custom output */
    fprintf(stderr, "\nTest 3: All options with custom -o\n");
    snprintf(command, sizeof(command),
             "gcc -dumpdir ./temps/ -dumpbase fulltest -dumpbase-ext .cxx "
             "-save-temps -o custom_output.o -c %s",
             TEMP_SOURCE_FILE);
    run_gcc_command(command);
    remove("custom_output.o");
    
    /* Test 4: Different save-temps variant */
    fprintf(stderr, "\nTest 4: -save-temps=obj variant\n");
    snprintf(command, sizeof(command),
             "gcc -dumpdir ./dump2/ -dumpbase variant -dumpbase-ext .i "
             "-save-temps=obj -c %s -o variant.o",
             TEMP_SOURCE_FILE);
    run_gcc_command(command);
    remove("variant.o");
    system("rm -rf ./dump2 2>/dev/null");
    
    /* Test 5: Minimal options to trigger basic cleanup */
    fprintf(stderr, "\nTest 5: Minimal options\n");
    snprintf(command, sizeof(command),
             "gcc -dumpbase minimal -c %s",
             TEMP_SOURCE_FILE);
    run_gcc_command(command);
    remove("minimal.o");
    
    /* Test 6: Error case with invalid option (should still trigger cleanup) */
    fprintf(stderr, "\nTest 6: Error case with invalid option\n");
    snprintf(command, sizeof(command),
             "gcc -dumpdir ./error_dump/ -dumpbase error -dumpbase-ext .err "
             "-save-temps -invalid-opt -c %s 2>/dev/null",
             TEMP_SOURCE_FILE);
    run_gcc_command(command);
    
    /* Test 7: Empty dumpdir (edge case) */
    fprintf(stderr, "\nTest 7: Empty -dumpdir\n");
    snprintf(command, sizeof(command),
             "gcc -dumpdir '' -dumpbase empty -save-temps -c %s",
             TEMP_SOURCE_FILE);
    run_gcc_command(command);
    
    /* Test 8: Complex dumpbase with path */
    fprintf(stderr, "\nTest 8: Complex dumpbase with path\n");
    snprintf(command, sizeof(command),
             "gcc -dumpbase subdir/complex_name -dumpbase-ext .longext "
             "-save-temps=cwd -c %s",
             TEMP_SOURCE_FILE);
    run_gcc_command(command);
    
    fprintf(stderr, "\n=== GCC coverage test completed ===\n");
    
    /* Cleanup will happen via atexit */
    return 0;
}
