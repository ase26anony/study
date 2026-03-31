/* main.c */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

int run_command(const char *cmd) {
    printf("Running: %s\n", cmd);
    int status = system(cmd);
    if (WIFEXITED(status)) {
        printf("Command exited with status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    } else {
        printf("Command failed to execute properly\n\n");
        return -1;
    }
}

int file_exists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

int main() {
    /* Check if source files exist */
    if (!file_exists("test.c")) {
        fprintf(stderr, "Error: test.c not found\n");
        return 1;
    }
    
    if (!file_exists("test2.c")) {
        fprintf(stderr, "Error: test2.c not found\n");
        return 1;
    }
    
    /* Create dump directories if they don't exist */
    system("mkdir -p ./testdump ./otherdump");
    
    /* Invoke gcc with help flag */
    run_command("gcc --help 2>&1 | head -5");
    
    /* Invoke gcc with version flag */
    run_command("gcc --version");
    
    /* Compile a simple file with save-temps and dumpdir flags */
    run_command("gcc -save-temps -dumpdir ./testdump/ -dumpbase mytest -c test.c -v 2>&1 | tail -10");
    
    /* Compile another file with different dumpdir to trigger cleanup */
    run_command("gcc -dumpdir ./otherdump/ -dumpbase other -c test2.c 2>&1");
    
    /* Optional: Clean up temporary files */
    printf("Cleaning up temporary files...\n");
    system("rm -f test.i test.s test.o test2.i test2.s test2.o");
    system("rm -f mytest* other*");
    system("rm -rf ./testdump ./otherdump");
    
    return 0;
}
