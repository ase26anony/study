/* main.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

int execute_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int status = system(cmd);
    if (status == -1) {
        perror("system() failed");
        return -1;
    } else if (WIFEXITED(status)) {
        printf("Command exited with status: %d\n\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    } else {
        printf("Command terminated abnormally\n\n");
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
    if (execute_command("gcc --help 2>&1 | head -5") != 0) {
        fprintf(stderr, "gcc --help failed\n");
    }
    
    /* Invoke gcc with version flag */
    if (execute_command("gcc --version") != 0) {
        fprintf(stderr, "gcc --version failed\n");
    }
    
    /* Compile a simple file with save-temps and dumpdir flags */
    if (execute_command("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10") != 0) {
        fprintf(stderr, "Compilation of test.c failed\n");
    }
    
    /* Compile another file with different dumpdir to trigger cleanup */
    if (execute_command("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1") != 0) {
        fprintf(stderr, "Compilation of test2.c failed\n");
    }
    
    /* Optional: Clean up temporary files */
    printf("Cleaning up temporary files...\n");
    system("rm -f test.i test.s test.o test2.i test2.s test2.o");
    system("rm -rf ./testdump ./otherdump");
    
    return 0;
}
