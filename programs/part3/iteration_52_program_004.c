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
    
    fprintf(fp, "/* Test source file for GCC coverage testing */\n");
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
    system("rm -f ./dump/*.i ./dump/*.s ./dump/*.o 2>/dev/null");
    system("rm -f *.i *.s *.o 2>/dev/null");
    system("rmdir ./dump 2>/dev/null");
}

/* Execute GCC with the given arguments and wait for completion */
static int execute_gcc(const char *args) {
    printf("[EXECUTING] gcc %s\n", args);
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "gcc %s", args);
        
        /* Use system() as requested, but could also use execvp() */
        int ret = system(cmd);
        _exit(ret == -1 ? 127 : WEXITSTATUS(ret));
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("[STATUS] Exit code: %d\n\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            printf("[STATUS] Terminated by signal: %d\n\n", WTERMSIG(status));
            return 128 + WTERMSIG(status);
        }
    } else {
        perror("fork failed");
        return -1;
    }
    
    return 0;
}

int main(void) {
    int overall_status = 0;
    
    /* Create the test source file */
    if (create_test_source() != 0) {
        return 1;
    }
    
    /* Ensure we start clean */
    cleanup_files();
    
    printf("=== Starting GCC coverage test ===\n\n");
    
    /* Test 1: Basic compilation with dumpdir and save-temps */
    printf("--- Test 1: -dumpdir with -save-temps ---\n");
    char cmd1[1024];
    snprintf(cmd1, sizeof(cmd1), 
             "-dumpdir ./dump/ -dumpbase myprog -dumpbase-ext .c "
             "-save-temps=cwd -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    execute_gcc(cmd1);
    
    /* Test 2: dumpbase and dumpbase-ext without dumpdir */
    printf("--- Test 2: -dumpbase and -dumpbase-ext (no -dumpdir) ---\n");
    char cmd2[1024];
    snprintf(cmd2, sizeof(cmd2),
             "-dumpbase testdump -dumpbase-ext .src "
             "-c %s -o test2.o",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd2);
    
    /* Test 3: All options combined with custom output name */
    printf("--- Test 3: All dump options with -save-temps and custom -o ---\n");
    char cmd3[1024];
    snprintf(cmd3, sizeof(cmd3),
             "-dumpdir ./dump/ -dumpbase fulltest -dumpbase-ext .c "
             "-save-temps -o custom_output.o -c %s",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd3);
    
    /* Test 4: Different save-temps option */
    printf("--- Test 4: Different -save-temps option ---\n");
    char cmd4[1024];
    snprintf(cmd4, sizeof(cmd4),
             "-dumpdir ./dump/ -dumpbase alt -dumpbase-ext .txt "
             "-save-temps=obj -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    execute_gcc(cmd4);
    
    /* Test 5: Minimal options to ensure basic cleanup path */
    printf("--- Test 5: Minimal options (just -c) ---\n");
    char cmd5[1024];
    snprintf(cmd5, sizeof(cmd5),
             "-c %s -o minimal.o",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd5);
    
    /* Test 6: Invalid option to test cleanup after error */
    printf("--- Test 6: Invalid option to trigger error cleanup ---\n");
    char cmd6[1024];
    snprintf(cmd6, sizeof(cmd6),
             "-dumpdir ./dump/ -dumpbase error -dumpbase-ext .c "
             "-invalid-opt -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    execute_gcc(cmd6);
    
    /* Test 7: Empty dumpdir (edge case) */
    printf("--- Test 7: Empty -dumpdir ---\n");
    char cmd7[1024];
    snprintf(cmd7, sizeof(cmd7),
             "-dumpdir \"\" -dumpbase empty -dumpbase-ext .c "
             "-save-temps -c %s -o empty.o",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd7);
    
    /* Test 8: Long dumpbase to test allocation */
    printf("--- Test 8: Long dumpbase name ---\n");
    char cmd8[1024];
    snprintf(cmd8, sizeof(cmd8),
             "-dumpdir ./dump/ -dumpbase very_long_dumpbase_name_for_testing_allocation "
             "-dumpbase-ext .very_long_extension -save-temps -c %s -o long.o",
             TEMP_SOURCE_FILE);
    execute_gcc(cmd8);
    
    printf("=== GCC coverage test completed ===\n");
    
    /* Clean up temporary files */
    cleanup_files();
    
    return overall_status;
}
