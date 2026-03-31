#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_gcc_cleanup.c"
#define TEMP_OUTPUT_FILE "test_gcc_cleanup.o"

/* Create a simple valid C source file */
static int create_test_source(void) {
    FILE *fp = fopen(TEMP_SOURCE_FILE, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return -1;
    }
    
    fprintf(fp, "/* Test file for GCC cleanup coverage */\n");
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    printf(\"Hello from test program\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/* Execute GCC with given arguments using fork/exec */
static int run_gcc(const char *args) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        char *argv[64];
        int argc = 0;
        char *token;
        char *args_copy = strdup(args);
        
        if (!args_copy) {
            perror("strdup failed");
            exit(EXIT_FAILURE);
        }
        
        /* Parse arguments */
        token = strtok(args_copy, " ");
        while (token != NULL && argc < 63) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
        argv[argc] = NULL;
        
        /* Execute GCC */
        execvp("gcc", argv);
        
        /* If we get here, exec failed */
        perror("execvp failed");
        free(args_copy);
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return -1;
        }
    }
}

/* Clean up temporary files */
static void cleanup_files(void) {
    remove(TEMP_SOURCE_FILE);
    remove(TEMP_OUTPUT_FILE);
    /* Clean up any dump files that might have been created */
    system("rm -f *.i *.s *.o dumpdir_* ./dump/* 2>/dev/null");
}

int main(void) {
    char cmd[1024];
    int ret;
    
    printf("=== GCC Cleanup Coverage Test Program ===\n");
    
    /* Step 1: Create test source file */
    if (create_test_source() < 0) {
        return EXIT_FAILURE;
    }
    
    printf("Created test source file: %s\n", TEMP_SOURCE_FILE);
    
    /* Step 2: Multiple GCC invocations with different option combinations */
    
    /* Invocation 1: With dumpdir and save-temps */
    printf("\n--- Invocation 1: -dumpdir and -save-temps ---\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -save-temps -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    printf("Command: %s\n", cmd);
    ret = system(cmd);
    printf("Exit status: %d\n", ret);
    
    /* Invocation 2: With dumpbase and dumpbase-ext (no dumpdir) */
    printf("\n--- Invocation 2: -dumpbase and -dumpbase-ext ---\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpbase myprog -dumpbase-ext .c -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    printf("Command: %s\n", cmd);
    ret = system(cmd);
    printf("Exit status: %d\n", ret);
    
    /* Invocation 3: All options combined */
    printf("\n--- Invocation 3: All dump options with -save-temps and -o ---\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dumpdir_ -dumpbase myprog_full "
             "-dumpbase-ext .c -save-temps=cwd -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    printf("Command: %s\n", cmd);
    ret = system(cmd);
    printf("Exit status: %d\n", ret);
    
    /* Invocation 4: Different output name to affect outbase */
    printf("\n--- Invocation 4: Different -o name ---\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -dumpbase altbase -dumpbase-ext .c "
             "-save-temps -c %s -o alternative_output.o",
             TEMP_SOURCE_FILE);
    printf("Command: %s\n", cmd);
    ret = system(cmd);
    printf("Exit status: %d\n", ret);
    
    /* Invocation 5: With fork/exec instead of system() */
    printf("\n--- Invocation 5: Using fork/exec ---\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump_fork/ -dumpbase fork_test "
             "-dumpbase-ext .c -save-temps -c %s -o fork_output.o",
             TEMP_SOURCE_FILE);
    printf("Command: %s\n", cmd);
    ret = run_gcc(cmd);
    printf("Exit status: %d\n", ret);
    
    /* Invocation 6: Trigger error exit to test cleanup after error */
    printf("\n--- Invocation 6: Error case with invalid option ---\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump_err/ -dumpbase error_test "
             "-dumpbase-ext .c -save-temps -invalid-opt -c %s -o error.o",
             TEMP_SOURCE_FILE);
    printf("Command: %s\n", cmd);
    ret = system(cmd);
    printf("Exit status: %d\n", ret);
    
    /* Invocation 7: Minimal case with just -save-temps */
    printf("\n--- Invocation 7: Just -save-temps ---\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -save-temps -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    printf("Command: %s\n", cmd);
    ret = system(cmd);
    printf("Exit status: %d\n", ret);
    
    /* Invocation 8: Empty dumpdir (edge case) */
    printf("\n--- Invocation 8: Empty dumpdir ---\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir '' -dumpbase empty_test -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    printf("Command: %s\n", cmd);
    ret = system(cmd);
    printf("Exit status: %d\n", ret);
    
    /* Step 3: Clean up temporary files */
    printf("\n=== Cleaning up temporary files ===\n");
    cleanup_files();
    
    printf("\n=== Test completed ===\n");
    return EXIT_SUCCESS;
}
