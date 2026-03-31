#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_source.c"
#define TEMP_OUTPUT_FILE "test_coverage_output.o"

/* Create a simple valid C source file for compilation */
static int create_test_source(void) {
    FILE *fp = fopen(TEMP_SOURCE_FILE, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return -1;
    }
    
    fprintf(fp, "/* Test source file for GCC coverage */\n");
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
    unlink(TEMP_SOURCE_FILE);
    unlink(TEMP_OUTPUT_FILE);
    
    /* Also clean up any dump files that might have been created */
    system("rm -f ./dump/*.i ./dump/*.s ./dump/*.o 2>/dev/null");
    system("rm -f *.i *.s *.o 2>/dev/null");
    system("rmdir ./dump 2>/dev/null");
}

/* Execute GCC with the given arguments and wait for completion */
static int run_gcc(const char *args) {
    printf("[EXECUTING] gcc %s\n", args);
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "gcc %s", args);
        
        /* Use system() to handle shell parsing */
        int ret = system(cmd);
        _exit(ret == -1 ? 127 : WEXITSTATUS(ret));
    } else {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("[EXIT STATUS] %d\n\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else {
            printf("[TERMINATED] by signal %d\n\n", WTERMSIG(status));
            return -1;
        }
    }
}

int main(void) {
    int overall_status = 0;
    
    /* Create the test source file */
    if (create_test_source() != 0) {
        return 1;
    }
    
    /* Ensure we clean up files on exit */
    atexit(cleanup_files);
    
    printf("=== Starting GCC coverage test ===\n\n");
    
    /* Test 1: Basic compilation with dumpdir and save-temps
       This should allocate dumpdir and trigger cleanup */
    printf("--- Test 1: Basic with dumpdir and save-temps ---\n");
    run_gcc("-dumpdir ./dump/ -dumpbase myprog -dumpbase-ext .c "
            "-save-temps=cwd -c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 2: With dumpbase and dumpbase-ext but no dumpdir
       Tests different allocation patterns */
    printf("--- Test 2: dumpbase and dumpbase-ext without dumpdir ---\n");
    run_gcc("-dumpbase testdump -dumpbase-ext .src "
            "-c " TEMP_SOURCE_FILE " -o test2.o");
    
    /* Test 3: Comprehensive test with all options
       Exercises the full initialization and cleanup path */
    printf("--- Test 3: All dump options with custom output ---\n");
    run_gcc("-dumpdir ./dump/ -dumpbase comprehensive "
            "-dumpbase-ext .test -save-temps "
            "-c " TEMP_SOURCE_FILE " -o custom_output.o");
    
    /* Test 4: Without dump options but with save-temps
       Tests save_temps_flag initialization */
    printf("--- Test 4: Only save-temps ---\n");
    run_gcc("-save-temps -c " TEMP_SOURCE_FILE " -o test4.o");
    
    /* Test 5: With -o option only (tests outbase allocation) */
    printf("--- Test 5: Custom output name only ---\n");
    run_gcc("-c " TEMP_SOURCE_FILE " -o special_output.o");
    
    /* Test 6: Invalid option to test cleanup after error exit
       The cleanup block should still execute */
    printf("--- Test 6: Invalid option (testing error cleanup) ---\n");
    run_gcc("-invalid-option -c " TEMP_SOURCE_FILE " 2>/dev/null");
    
    /* Test 7: Multiple dumpdir variations */
    printf("--- Test 7: Different dumpdir formats ---\n");
    run_gcc("-dumpdir dumpdir1 -dumpbase db1 -c " TEMP_SOURCE_FILE " -o t7.o");
    run_gcc("-dumpdir ./ -dumpbase db2 -c " TEMP_SOURCE_FILE " -o t7b.o");
    run_gcc("-dumpdir ../ -dumpbase db3 -c " TEMP_SOURCE_FILE " -o t7c.o");
    
    /* Test 8: Empty dumpbase and dumpbase-ext */
    printf("--- Test 8: Empty dump options ---\n");
    run_gcc("-dumpbase \"\" -dumpbase-ext \"\" -c " TEMP_SOURCE_FILE " -o t8.o");
    
    /* Test 9: Combination that might trigger edge cases */
    printf("--- Test 9: Edge case combination ---\n");
    run_gcc("-dumpdir very/long/path/that/might/need/allocation/ "
            "-dumpbase verylongdumpbasename123456789 "
            "-dumpbase-ext .extraextension "
            "-save-temps=cwd "
            "-c " TEMP_SOURCE_FILE " -o edge_case_output.o");
    
    printf("=== GCC coverage test completed ===\n");
    
    return overall_status;
}
