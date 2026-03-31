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
        fprintf(stderr, "Failed to create source file: %s\n", strerror(errno));
        return -1;
    }
    
    fprintf(fp, "int main(void) { return 0; }\n");
    fclose(fp);
    return 0;
}

/* Clean up temporary files */
static void cleanup_files(void) {
    unlink(TEMP_SOURCE_FILE);
    unlink(TEMP_OUTPUT_FILE);
    
    /* Also clean up any dump files that might have been created */
    unlink("myprog.c.*");
    unlink("./dump/myprog.c.*");
    unlink("test_coverage_source.c.*");
}

/* Execute GCC with given arguments and wait for completion */
static int run_gcc(const char *args) {
    printf("[EXECUTING] gcc %s\n", args);
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        execlp("gcc", "gcc", args, NULL);
        /* If execlp fails */
        fprintf(stderr, "Failed to execute gcc: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("[COMPLETED] Exit code: %d\n\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else {
            printf("[FAILED] Process terminated abnormally\n\n");
            return -1;
        }
    } else {
        fprintf(stderr, "Fork failed: %s\n", strerror(errno));
        return -1;
    }
}

/* Execute GCC using system() call (alternative implementation) */
static int run_gcc_system(const char *args) {
    char command[1024];
    snprintf(command, sizeof(command), "gcc %s", args);
    
    printf("[EXECUTING SYSTEM] %s\n", command);
    int status = system(command);
    
    if (status == -1) {
        printf("[FAILED] system() call failed\n\n");
        return -1;
    } else if (WIFEXITED(status)) {
        printf("[COMPLETED] Exit code: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    } else {
        printf("[FAILED] Process terminated abnormally\n\n");
        return -1;
    }
}

int main(void) {
    int overall_status = 0;
    
    /* Create the test source file */
    if (create_test_source() != 0) {
        return EXIT_FAILURE;
    }
    
    printf("=== Starting GCC driver coverage test ===\n\n");
    
    /* Test 1: Basic compilation with dumpdir and save-temps */
    printf("--- Test 1: -dumpdir and -save-temps ---\n");
    run_gcc("-dumpdir ./dump/ -save-temps -c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 2: dumpbase and dumpbase-ext without dumpdir */
    printf("--- Test 2: -dumpbase and -dumpbase-ext ---\n");
    run_gcc("-dumpbase myprog -dumpbase-ext .c -c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 3: All dump options combined with save-temps and custom output */
    printf("--- Test 3: All dump options with -save-temps and -o ---\n");
    run_gcc("-dumpdir ./dump/ -dumpbase myprog -dumpbase-ext .c -save-temps=cwd -c " TEMP_SOURCE_FILE " -o custom_output.o");
    
    /* Test 4: Different save-temps flag values */
    printf("--- Test 4: Different -save-temps variants ---\n");
    run_gcc("-dumpdir ./dump/ -save-temps=obj -c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 5: Using system() call instead of fork/exec */
    printf("--- Test 5: Using system() call ---\n");
    run_gcc_system("-dumpdir ./dump/ -dumpbase myprog -dumpbase-ext .c -save-temps -c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 6: Trigger cleanup after error exit (invalid option) */
    printf("--- Test 6: Cleanup after error (invalid option) ---\n");
    run_gcc("-dumpdir ./dump/ -dumpbase myprog -invalid-opt -c " TEMP_SOURCE_FILE);
    
    /* Test 7: Multiple invocations in sequence to stress cleanup */
    printf("--- Test 7: Multiple rapid invocations ---\n");
    for (int i = 0; i < 3; i++) {
        char dumpbase[64];
        snprintf(dumpbase, sizeof(dumpbase), "iter%d", i);
        char args[512];
        snprintf(args, sizeof(args), 
                 "-dumpdir ./dump/ -dumpbase %s -dumpbase-ext .c -save-temps -c %s -o iter%d.o",
                 dumpbase, TEMP_SOURCE_FILE, i);
        run_gcc(args);
    }
    
    /* Test 8: Test with empty dumpdir (current directory) */
    printf("--- Test 8: Empty dumpdir (current directory) ---\n");
    run_gcc("-dumpdir '' -dumpbase myprog -save-temps -c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 9: Test dumpdir with trailing slash */
    printf("--- Test 9: Dumpdir with trailing slash ---\n");
    run_gcc("-dumpdir 'dumpdir_with_slash/' -dumpbase myprog -save-temps -c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    /* Test 10: Test without dump options but with save-temps */
    printf("--- Test 10: Only save-temps (no dump options) ---\n");
    run_gcc("-save-temps -c " TEMP_SOURCE_FILE " -o " TEMP_OUTPUT_FILE);
    
    printf("=== GCC driver coverage test completed ===\n");
    
    /* Clean up temporary files */
    cleanup_files();
    
    return overall_status == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
