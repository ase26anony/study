#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_gcc_coverage.c"
#define TEMP_OUTPUT_FILE "test_gcc_coverage.o"

/* Create a simple valid C source file for compilation */
static int create_test_source(void) {
    FILE *fp = fopen(TEMP_SOURCE_FILE, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return -1;
    }
    
    fprintf(fp, "/* Test file for GCC coverage */\n");
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    printf(\"Hello from test program\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/* Execute GCC with given arguments using fork/exec */
static int execute_gcc(const char *args) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process - execute GCC */
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
        /* Parent process - wait for child */
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
    system("rm -f ./dump/*.i ./dump/*.s ./dump/*.o 2>/dev/null");
    system("rm -f *.i *.s *.o 2>/dev/null");
    system("rmdir ./dump 2>/dev/null");
}

int main(void) {
    int ret;
    char cmd[1024];
    
    printf("=== GCC Driver Coverage Test Program ===\n\n");
    
    /* Step 1: Create test source file */
    if (create_test_source() != 0) {
        return EXIT_FAILURE;
    }
    printf("Created test source file: %s\n", TEMP_SOURCE_FILE);
    
    /* Step 2: Execute multiple GCC invocations with different option combinations */
    
    /* Invocation 1: With dumpdir and save-temps */
    printf("\n--- Invocation 1: -dumpdir and -save-temps ---\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -save-temps=cwd -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    printf("Executing: %s\n", cmd);
    ret = execute_gcc(cmd);
    printf("Exit status: %d\n", ret);
    
    /* Invocation 2: With dumpbase and dumpbase-ext but no dumpdir */
    printf("\n--- Invocation 2: -dumpbase and -dumpbase-ext ---\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpbase myprog -dumpbase-ext .c -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    printf("Executing: %s\n", cmd);
    ret = execute_gcc(cmd);
    printf("Exit status: %d\n", ret);
    
    /* Invocation 3: All options combined */
    printf("\n--- Invocation 3: All options combined ---\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -dumpbase myprog -dumpbase-ext .c "
             "-save-temps -o custom_output.o -c %s",
             TEMP_SOURCE_FILE);
    printf("Executing: %s\n", cmd);
    ret = execute_gcc(cmd);
    printf("Exit status: %d\n", ret);
    
    /* Invocation 4: Different dumpdir format */
    printf("\n--- Invocation 4: Different dumpdir format ---\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir dumpdir_with_trailing_dash- -dumpbase test "
             "-save-temps=cwd -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    printf("Executing: %s\n", cmd);
    ret = execute_gcc(cmd);
    printf("Exit status: %d\n", ret);
    
    /* Invocation 5: Minimal options to ensure basic cleanup */
    printf("\n--- Invocation 5: Minimal options ---\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -c %s",
             TEMP_SOURCE_FILE);
    printf("Executing: %s\n", cmd);
    ret = execute_gcc(cmd);
    printf("Exit status: %d\n", ret);
    
    /* Invocation 6: With invalid option to test error cleanup */
    printf("\n--- Invocation 6: Invalid option (testing error cleanup) ---\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -dumpbase error_test -invalid-opt -c %s",
             TEMP_SOURCE_FILE);
    printf("Executing: %s\n", cmd);
    ret = execute_gcc(cmd);
    printf("Exit status: %d (expected non-zero)\n", ret);
    
    /* Invocation 7: With outbase influenced by -o option */
    printf("\n--- Invocation 7: Testing outbase with -o ---\n");
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpbase_ext .xyz -save-temps -o special_output.o -c %s",
             TEMP_SOURCE_FILE);
    printf("Executing: %s\n", cmd);
    ret = execute_gcc(cmd);
    printf("Exit status: %d\n", ret);
    
    /* Step 3: Clean up temporary files */
    printf("\n--- Cleaning up temporary files ---\n");
    cleanup_files();
    
    printf("\n=== Test completed ===\n");
    return EXIT_SUCCESS;
}
