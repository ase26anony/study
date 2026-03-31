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
    FILE *f = fopen(TEMP_SOURCE_FILE, "w");
    if (!f) {
        perror("Failed to create test source file");
        return 0;
    }
    
    fprintf(f, "/* Test file for GCC coverage */\n");
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main(void) {\n");
    fprintf(f, "    printf(\"Hello from coverage test!\\n\");\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
    return 1;
}

/* Execute GCC command and wait for completion */
static int run_gcc_command(const char *cmd) {
    fprintf(stderr, "Executing: %s\n", cmd);
    
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
        perror("execl failed");
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid failed");
            return -1;
        }
        
        if (WIFEXITED(status)) {
            fprintf(stderr, "GCC exited with status: %d\n\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "GCC terminated by signal: %d\n\n", WTERMSIG(status));
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
    system("rm -f *.i *.s *.o dumpdir_test.* ./dump/* 2>/dev/null");
    system("rm -rf ./dump 2>/dev/null");
}

int main(void) {
    char cmd[1024];
    int ret;
    
    /* Create test source file */
    if (!create_test_source()) {
        return EXIT_FAILURE;
    }
    
    /* Ensure dump directory exists */
    system("mkdir -p ./dump 2>/dev/null");
    
    fprintf(stderr, "=== Starting GCC coverage test ===\n\n");
    
    /* Test 1: Basic compilation with dumpdir and save-temps */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -save-temps=cwd -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    ret = run_gcc_command(cmd);
    
    /* Test 2: With dumpbase and dumpbase-ext but no dumpdir */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpbase myprog -dumpbase-ext .c -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    ret = run_gcc_command(cmd);
    
    /* Test 3: All options combined */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -dumpbase myprog -dumpbase-ext .c "
             "-save-temps -o custom_output.o -c %s",
             TEMP_SOURCE_FILE);
    ret = run_gcc_command(cmd);
    
    /* Test 4: Different save-temps option */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -save-temps=obj -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    ret = run_gcc_command(cmd);
    
    /* Test 5: Without -c flag (linking) to test different code path */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -dumpbase fullprog -dumpbase-ext .c "
             "-save-temps %s -o test_program",
             TEMP_SOURCE_FILE);
    ret = run_gcc_command(cmd);
    
    /* Test 6: Invalid option to test cleanup after error */
    snprintf(cmd, sizeof(cmd),
             "gcc -dumpdir ./dump/ -dumpbase error_test -invalid-opt -c %s",
             TEMP_SOURCE_FILE);
    ret = run_gcc_command(cmd);
    
    /* Test 7: Minimal options to ensure basic cleanup path */
    snprintf(cmd, sizeof(cmd),
             "gcc -c %s",
             TEMP_SOURCE_FILE);
    ret = run_gcc_command(cmd);
    
    /* Test 8: With verbose flag to test verbose_only_flag */
    snprintf(cmd, sizeof(cmd),
             "gcc -v -dumpdir ./dump/ -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OUTPUT_FILE);
    ret = run_gcc_command(cmd);
    
    /* Test 9: With version flag to test print_version */
    snprintf(cmd, sizeof(cmd),
             "gcc --version -dumpdir ./dump/");
    ret = run_gcc_command(cmd);
    
    /* Test 10: With help flag to test print_help_list */
    snprintf(cmd, sizeof(cmd),
             "gcc --help -dumpdir ./dump/");
    ret = run_gcc_command(cmd);
    
    fprintf(stderr, "=== GCC coverage test completed ===\n");
    
    /* Clean up */
    cleanup();
    
    return EXIT_SUCCESS;
}
