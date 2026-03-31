#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_gcc_coverage.c"
#define TEMP_OUTPUT_FILE "test_gcc_coverage.o"

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
        char args_copy[1024];
        
        /* Copy args string since strtok modifies it */
        strncpy(args_copy, args, sizeof(args_copy) - 1);
        args_copy[sizeof(args_copy) - 1] = '\0';
        
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

int main(void) {
    char command[2048];
    int ret;
    
    /* Create test source file */
    if (create_test_source() != 0) {
        return EXIT_FAILURE;
    }
    
    fprintf(stderr, "=== Starting GCC coverage test ===\n");
    
    /* Test 1: Basic compilation with dump options */
    fprintf(stderr, "\nTest 1: Basic compilation with dump options\n");
    snprintf(command, sizeof(command),
             "gcc -dumpdir ./dump/ -dumpbase myprog -dumpbase-ext .c -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    fprintf(stderr, "Command: %s\n", command);
    ret = system(command);
    fprintf(stderr, "Exit code: %d\n", ret);
    
    /* Test 2: With save-temps and different dumpbase */
    fprintf(stderr, "\nTest 2: With save-temps and different dumpbase\n");
    snprintf(command, sizeof(command),
             "gcc -save-temps=cwd -dumpbase different_prog -dumpbase-ext .c -c %s -o test2.o",
             TEMP_SOURCE_FILE);
    fprintf(stderr, "Command: %s\n", command);
    ret = system(command);
    fprintf(stderr, "Exit code: %d\n", ret);
    
    /* Test 3: All options combined */
    fprintf(stderr, "\nTest 3: All options combined\n");
    snprintf(command, sizeof(command),
             "gcc -dumpdir ./full_dump/ -dumpbase full_prog -dumpbase-ext .c "
             "-save-temps -o test3_output.o -c %s",
             TEMP_SOURCE_FILE);
    fprintf(stderr, "Command: %s\n", command);
    ret = system(command);
    fprintf(stderr, "Exit code: %d\n", ret);
    
    /* Test 4: Without dumpdir but with dumpbase and dumpbase-ext */
    fprintf(stderr, "\nTest 4: Without dumpdir but with dumpbase and dumpbase-ext\n");
    snprintf(command, sizeof(command),
             "gcc -dumpbase nodir_prog -dumpbase-ext .c -save-temps=cwd -c %s -o test4.o",
             TEMP_SOURCE_FILE);
    fprintf(stderr, "Command: %s\n", command);
    ret = system(command);
    fprintf(stderr, "Exit code: %d\n", ret);
    
    /* Test 5: Minimal options to trigger outbase allocation */
    fprintf(stderr, "\nTest 5: Minimal options with custom output name\n");
    snprintf(command, sizeof(command),
             "gcc -o custom_output.o -c %s",
             TEMP_SOURCE_FILE);
    fprintf(stderr, "Command: %s\n", command);
    ret = system(command);
    fprintf(stderr, "Exit code: %d\n", ret);
    
    /* Test 6: Invalid option to test cleanup after error */
    fprintf(stderr, "\nTest 6: Invalid option to test cleanup after error\n");
    snprintf(command, sizeof(command),
             "gcc -dumpdir ./error_dump/ -dumpbase error_prog -invalid-opt -c %s",
             TEMP_SOURCE_FILE);
    fprintf(stderr, "Command: %s\n", command);
    ret = system(command);
    fprintf(stderr, "Exit code: %d\n", ret);
    
    /* Test 7: Using fork/exec directly (alternative to system()) */
    fprintf(stderr, "\nTest 7: Using fork/exec directly\n");
    char *fork_exec_args = "gcc -dumpdir ./fork_dump/ -dumpbase fork_prog "
                           "-dumpbase-ext .c -save-temps -c " TEMP_SOURCE_FILE " -o fork_test.o";
    fprintf(stderr, "Command: %s\n", fork_exec_args);
    ret = run_gcc(fork_exec_args);
    fprintf(stderr, "Exit code: %d\n", ret);
    
    /* Clean up temporary files */
    fprintf(stderr, "\n=== Cleaning up ===\n");
    remove(TEMP_SOURCE_FILE);
    remove(TEMP_OUTPUT_FILE);
    remove("test2.o");
    remove("test3_output.o");
    remove("test4.o");
    remove("custom_output.o");
    remove("fork_test.o");
    
    /* Also clean up any .i, .s files created by -save-temps */
    remove("test_gcc_coverage.i");
    remove("test_gcc_coverage.s");
    remove("different_prog.i");
    remove("different_prog.s");
    remove("full_prog.i");
    remove("full_prog.s");
    remove("nodir_prog.i");
    remove("nodir_prog.s");
    remove("error_prog.i");
    remove("error_prog.s");
    remove("fork_prog.i");
    remove("fork_prog.s");
    
    fprintf(stderr, "Test completed\n");
    
    return EXIT_SUCCESS;
}
