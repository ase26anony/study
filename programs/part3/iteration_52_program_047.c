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

/* Execute GCC with given arguments */
static int run_gcc(const char *args) {
    printf("[EXECUTING] gcc %s\n", args);
    
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "fork failed: %s\n", strerror(errno));
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        char *argv[] = {
            "gcc",
            NULL, NULL, NULL, NULL, NULL, NULL, 
            NULL, NULL, NULL, NULL, NULL, NULL,
            NULL  /* Terminator */
        };
        
        /* Parse arguments into argv array */
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "gcc %s", args);
        
        /* Use system() as requested, but also show fork/exec alternative */
        execlp("sh", "sh", "-c", cmd, NULL);
        
        /* If execlp fails */
        fprintf(stderr, "execlp failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("[STATUS] Exit code: %d\n\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else {
            fprintf(stderr, "[ERROR] GCC terminated abnormally\n\n");
            return -1;
        }
    }
}

/* Alternative using system() as requested */
static int run_gcc_system(const char *args) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "gcc %s", args);
    
    printf("[EXECUTING SYSTEM] %s\n", cmd);
    
    int status = system(cmd);
    
    if (WIFEXITED(status)) {
        printf("[STATUS] Exit code: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    } else {
        fprintf(stderr, "[ERROR] System call failed\n\n");
        return -1;
    }
}

int main(void) {
    int overall_status = 0;
    
    /* Step 1: Create a valid C source file */
    if (create_test_source() != 0) {
        return EXIT_FAILURE;
    }
    
    printf("=== Starting GCC driver coverage tests ===\n\n");
    
    /* 
     * Test 1: Basic compilation with dumpdir and save-temps
     * This should allocate dumpdir and trigger save_temps_flag
     */
    printf("--- Test 1: -dumpdir with -save-temps ---\n");
    char cmd1[512];
    snprintf(cmd1, sizeof(cmd1),
             "-dumpdir ./coverage_dump/ "
             "-save-temps=cwd "
             "-c %s "
             "-o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    run_gcc_system(cmd1);
    
    /* 
     * Test 2: dumpbase and dumpbase-ext without dumpdir
     * This allocates dumpbase and dumpbase_ext
     */
    printf("--- Test 2: -dumpbase and -dumpbase-ext ---\n");
    char cmd2[512];
    snprintf(cmd2, sizeof(cmd2),
             "-dumpbase my_coverage_test "
             "-dumpbase-ext .c "
             "-c %s "
             "-o test2.o",
             TEMP_SOURCE_FILE);
    run_gcc_system(cmd2);
    
    /* 
     * Test 3: All options combined
     * This exercises the full initialization path
     */
    printf("--- Test 3: All dump options with -save-temps and custom -o ---\n");
    char cmd3[512];
    snprintf(cmd3, sizeof(cmd3),
             "-dumpdir ./full_dump/ "
             "-dumpbase full_test "
             "-dumpbase-ext .c "
             "-save-temps "
             "-c %s "
             "-o custom_output.o",
             TEMP_SOURCE_FILE);
    run_gcc_system(cmd3);
    
    /* 
     * Test 4: Different save-temps option
     * Tests save_temps_flag variations
     */
    printf("--- Test 4: Different save-temps mode ---\n");
    char cmd4[512];
    snprintf(cmd4, sizeof(cmd4),
             "-dumpdir ./dump2/ "
             "-save-temps=obj "
             "-c %s "
             "-o test4.o",
             TEMP_SOURCE_FILE);
    run_gcc_system(cmd4);
    
    /* 
     * Test 5: With verbose flag (touches verbose_only_flag)
     * Also tests cleanup after normal compilation
     */
    printf("--- Test 5: With verbose output ---\n");
    char cmd5[512];
    snprintf(cmd5, sizeof(cmd5),
             "-dumpbase verbose_test "
             "-v "
             "-c %s "
             "-o test5.o",
             TEMP_SOURCE_FILE);
    run_gcc_system(cmd5);
    
    /* 
     * Test 6: Invalid option to test cleanup after error
     * The cleanup block should still execute
     */
    printf("--- Test 6: Invalid option (testing error cleanup) ---\n");
    char cmd6[512];
    snprintf(cmd6, sizeof(cmd6),
             "-dumpdir ./error_dump/ "
             "-dumpbase error_test "
             "-invalid-opt "
             "-c %s",
             TEMP_SOURCE_FILE);
    run_gcc_system(cmd6);
    
    /* 
     * Test 7: Multiple dumpdir formats
     * Tests dumpdir_trailing_dash_added logic
     */
    printf("--- Test 7: Dumpdir with trailing slash variations ---\n");
    char cmd7[512];
    snprintf(cmd7, sizeof(cmd7),
             "-dumpdir ./trailing_test "
             "-dumpbase trail "
             "-save-temps "
             "-c %s "
             "-o test7.o",
             TEMP_SOURCE_FILE);
    run_gcc_system(cmd7);
    
    /* 
     * Test 8: Using fork/exec directly (alternative to system)
     * Demonstrates the requested fork/exec approach
     */
    printf("--- Test 8: Using fork/exec directly ---\n");
    char cmd8[512];
    snprintf(cmd8, sizeof(cmd8),
             "-dumpdir ./fork_dump/ "
             "-dumpbase fork_test "
             "-dumpbase-ext .c "
             "-save-temps=cwd "
             "-c %s "
             "-o test8.o",
             TEMP_SOURCE_FILE);
    run_gcc(cmd8);
    
    /* Cleanup temporary files */
    printf("=== Cleaning up temporary files ===\n");
    unlink(TEMP_SOURCE_FILE);
    unlink(TEMP_OUTPUT_FILE);
    unlink("test2.o");
    unlink("custom_output.o");
    unlink("test4.o");
    unlink("test5.o");
    unlink("test7.o");
    unlink("test8.o");
    
    /* Also clean up any save-temps files that might have been created */
    system("rm -f *.i *.s *.o coverage_dump/* full_dump/* dump2/* fork_dump/* 2>/dev/null");
    system("rm -rf coverage_dump full_dump dump2 fork_dump trailing_test 2>/dev/null");
    
    printf("\n=== GCC driver coverage tests completed ===\n");
    
    return overall_status;
}
