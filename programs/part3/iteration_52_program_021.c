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
        perror("Failed to create test source file");
        return -1;
    }
    
    fprintf(fp, "/* Test file for GCC coverage */\n");
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    printf(\"Hello from coverage test!\\n\");\n");
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
            fprintf(stderr, "GCC exited with status %d\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "GCC terminated by signal %d\n", WTERMSIG(status));
            return -1;
        }
    }
    
    return 0;
}

/* Clean up temporary files */
static void cleanup(void) {
    unlink(TEMP_SOURCE_FILE);
    unlink(TEMP_OUTPUT_FILE);
    
    /* Also clean up any dump files that might have been created */
    system("rm -f *.i *.s *.o dumpdir_* ./dump/* 2>/dev/null");
}

int main(void) {
    char command[2048];
    int ret;
    
    /* Create test source file */
    if (create_test_source() < 0) {
        return EXIT_FAILURE;
    }
    
    /* Register cleanup handler */
    atexit(cleanup);
    
    fprintf(stderr, "=== Starting GCC coverage test ===\n");
    
    /* Test 1: Basic compilation with dump options */
    fprintf(stderr, "\nTest 1: Basic compilation with dumpdir and dumpbase\n");
    snprintf(command, sizeof(command),
             "gcc -dumpdir ./dump/ -dumpbase myprog -dumpbase-ext .c -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    fprintf(stderr, "Command: %s\n", command);
    run_gcc(command);
    
    /* Test 2: With save-temps flag */
    fprintf(stderr, "\nTest 2: With save-temps flag\n");
    snprintf(command, sizeof(command),
             "gcc -save-temps=cwd -dumpdir dumpdir_test -dumpbase testdump "
             "-dumpbase-ext .ext -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    fprintf(stderr, "Command: %s\n", command);
    run_gcc(command);
    
    /* Test 3: All options combined */
    fprintf(stderr, "\nTest 3: All options combined\n");
    snprintf(command, sizeof(command),
             "gcc -save-temps -dumpdir ./dumps/ -dumpbase fulltest "
             "-dumpbase-ext .c -o test_output %s",
             TEMP_SOURCE_FILE);
    fprintf(stderr, "Command: %s\n", command);
    run_gcc(command);
    
    /* Test 4: Different dumpbase-ext variations */
    fprintf(stderr, "\nTest 4: Different dumpbase-ext variations\n");
    snprintf(command, sizeof(command),
             "gcc -dumpbase base1 -dumpbase-ext .i -c %s",
             TEMP_SOURCE_FILE);
    fprintf(stderr, "Command: %s\n", command);
    run_gcc(command);
    
    snprintf(command, sizeof(command),
             "gcc -dumpbase base2 -dumpbase-ext .s -c %s",
             TEMP_SOURCE_FILE);
    fprintf(stderr, "Command: %s\n", command);
    run_gcc(command);
    
    /* Test 5: Without dumpdir (to test default behavior) */
    fprintf(stderr, "\nTest 5: Without dumpdir\n");
    snprintf(command, sizeof(command),
             "gcc -dumpbase nodir -dumpbase-ext .c -save-temps=cwd -c %s",
             TEMP_SOURCE_FILE);
    fprintf(stderr, "Command: %s\n", command);
    run_gcc(command);
    
    /* Test 6: With verbose flag (triggers verbose_only_flag) */
    fprintf(stderr, "\nTest 6: With verbose flag\n");
    snprintf(command, sizeof(command),
             "gcc -v -dumpdir verbose_dump -dumpbase verbose -c %s",
             TEMP_SOURCE_FILE);
    fprintf(stderr, "Command: %s\n", command);
    run_gcc(command);
    
    /* Test 7: Error case with invalid option (should still trigger cleanup) */
    fprintf(stderr, "\nTest 7: Error case with invalid option\n");
    snprintf(command, sizeof(command),
             "gcc -invalid-opt -dumpdir error_dump -dumpbase error -c %s",
             TEMP_SOURCE_FILE);
    fprintf(stderr, "Command: %s\n", command);
    run_gcc(command);
    
    /* Test 8: Multiple output specifications */
    fprintf(stderr, "\nTest 8: Multiple output specifications\n");
    snprintf(command, sizeof(command),
             "gcc -dumpdir multi -dumpbase multi -o multi_output1 -o multi_output2 -c %s",
             TEMP_SOURCE_FILE);
    fprintf(stderr, "Command: %s\n", command);
    run_gcc(command);
    
    /* Test 9: With version flag (triggers print_version) */
    fprintf(stderr, "\nTest 9: Version flag with dump options\n");
    snprintf(command, sizeof(command),
             "gcc --version -dumpdir version_dump -dumpbase version");
    fprintf(stderr, "Command: %s\n", command);
    run_gcc(command);
    
    /* Test 10: Help flag (triggers print_help_list) */
    fprintf(stderr, "\nTest 10: Help flag with dump options\n");
    snprintf(command, sizeof(command),
             "gcc --help -dumpdir help_dump -dumpbase help");
    fprintf(stderr, "Command: %s\n", command);
    run_gcc(command);
    
    fprintf(stderr, "\n=== GCC coverage test completed ===\n");
    
    return EXIT_SUCCESS;
}
