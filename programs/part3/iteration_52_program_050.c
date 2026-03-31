#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_gcc_coverage.c"
#define TEMP_OBJ_FILE "test_gcc_coverage.o"

/* Create a simple valid C source file */
static int create_test_source(void) {
    FILE *fp = fopen(TEMP_SOURCE_FILE, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return 0;
    }
    
    fprintf(fp, "/* Test file for GCC coverage */\n");
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    printf(\"Hello from test program\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 1;
}

/* Clean up temporary files */
static void cleanup_files(void) {
    remove(TEMP_SOURCE_FILE);
    remove(TEMP_OBJ_FILE);
    /* Clean up any dump files that might have been created */
    system("rm -f *.i *.s *.o dumpdir_* ./dump/* 2>/dev/null");
}

/* Execute GCC with given arguments and wait for completion */
static int run_gcc(const char *args) {
    char command[1024];
    int status;
    
    snprintf(command, sizeof(command), "gcc %s", args);
    fprintf(stderr, "Executing: %s\n", command);
    
    status = system(command);
    
    if (WIFEXITED(status)) {
        fprintf(stderr, "GCC exited with status: %d\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        fprintf(stderr, "GCC terminated by signal: %d\n", WTERMSIG(status));
        return -1;
    }
    
    return status;
}

int main(void) {
    char args[1024];
    int overall_status = 0;
    
    /* Create test source file */
    if (!create_test_source()) {
        return 1;
    }
    
    /* Ensure dump directory exists */
    system("mkdir -p ./dump 2>/dev/null");
    
    fprintf(stderr, "\n=== Starting GCC coverage test ===\n\n");
    
    /* Test 1: Basic compilation with dump options */
    fprintf(stderr, "Test 1: Basic dump options\n");
    snprintf(args, sizeof(args), 
             "-dumpdir ./dump/ -dumpbase myprog -dumpbase-ext .c "
             "-c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OBJ_FILE);
    run_gcc(args);
    
    /* Test 2: With save-temps flag */
    fprintf(stderr, "\nTest 2: With save-temps\n");
    snprintf(args, sizeof(args), 
             "-save-temps -dumpdir ./dump/ -dumpbase myprog2 "
             "-dumpbase-ext .c -c %s -o test2.o",
             TEMP_SOURCE_FILE);
    run_gcc(args);
    
    /* Test 3: Different dumpbase-ext and no dumpdir */
    fprintf(stderr, "\nTest 3: No dumpdir, different extensions\n");
    snprintf(args, sizeof(args), 
             "-dumpbase myprog3 -dumpbase-ext .ext "
             "-c %s -o test3.o",
             TEMP_SOURCE_FILE);
    run_gcc(args);
    
    /* Test 4: All options combined */
    fprintf(stderr, "\nTest 4: All options combined\n");
    snprintf(args, sizeof(args), 
             "-save-temps=cwd -dumpdir ./dump/ "
             "-dumpbase myprog4 -dumpbase-ext .cxx "
             "-c %s -o test4_output.o",
             TEMP_SOURCE_FILE);
    run_gcc(args);
    
    /* Test 5: With verbose flag (affects verbose_only_flag) */
    fprintf(stderr, "\nTest 5: With verbose flag\n");
    snprintf(args, sizeof(args), 
             "-v -dumpdir ./dump/verbose/ -dumpbase verbose_prog "
             "-c %s -o verbose.o",
             TEMP_SOURCE_FILE);
    run_gcc(args);
    
    /* Test 6: Error case - invalid option (should still trigger cleanup) */
    fprintf(stderr, "\nTest 6: Invalid option (testing error cleanup)\n");
    snprintf(args, sizeof(args), 
             "-invalid-opt -dumpdir ./dump/ -dumpbase error "
             "-c %s -o error.o",
             TEMP_SOURCE_FILE);
    run_gcc(args);
    
    /* Test 7: Multiple dumpbase-ext values */
    fprintf(stderr, "\nTest 7: Multiple dumpbase-ext variations\n");
    snprintf(args, sizeof(args), 
             "-dumpbase multi1 -dumpbase-ext .i -c %s -o multi1.o",
             TEMP_SOURCE_FILE);
    run_gcc(args);
    
    snprintf(args, sizeof(args), 
             "-dumpbase multi2 -dumpbase-ext .s -c %s -o multi2.o",
             TEMP_SOURCE_FILE);
    run_gcc(args);
    
    snprintf(args, sizeof(args), 
             "-dumpbase multi3 -dumpbase-ext .o -c %s -o multi3.o",
             TEMP_SOURCE_FILE);
    run_gcc(args);
    
    /* Test 8: Empty dumpdir (current directory) */
    fprintf(stderr, "\nTest 8: Empty dumpdir (current dir)\n");
    snprintf(args, sizeof(args), 
             "-dumpdir '' -dumpbase currdir -save-temps "
             "-c %s -o currdir.o",
             TEMP_SOURCE_FILE);
    run_gcc(args);
    
    /* Test 9: Different output base names */
    fprintf(stderr, "\nTest 9: Different output names\n");
    snprintf(args, sizeof(args), 
             "-dumpbase outbase_test -dumpbase-ext .test "
             "-c %s -o different_output_name.o",
             TEMP_SOURCE_FILE);
    run_gcc(args);
    
    /* Test 10: Compilation without -c flag (link step) */
    fprintf(stderr, "\nTest 10: Full compilation (with linking)\n");
    snprintf(args, sizeof(args), 
             "-dumpdir ./dump/full/ -dumpbase fullprog -save-temps "
             "%s -o full_program",
             TEMP_SOURCE_FILE);
    run_gcc(args);
    
    fprintf(stderr, "\n=== GCC coverage test complete ===\n");
    
    /* Clean up */
    cleanup_files();
    
    return overall_status;
}
