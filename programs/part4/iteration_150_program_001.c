/* main.c */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int run_command(const char *cmd) {
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Command failed: %s\n", cmd);
    }
    return status;
}

int main() {
    /* Check if source files exist */
    if (access("test.c", F_OK) != 0) {
        fprintf(stderr, "Error: test.c not found\n");
        return 1;
    }
    
    if (access("test2.c", F_OK) != 0) {
        fprintf(stderr, "Error: test2.c not found\n");
        return 1;
    }
    
    printf("=== GCC Help (first 5 lines) ===\n");
    run_command("gcc --help 2>&1 | head -5");
    
    printf("\n=== GCC Version ===\n");
    run_command("gcc --version");
    
    printf("\n=== Compiling test.c with verbose output ===\n");
    run_command("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10");
    
    printf("\n=== Compiling test2.c ===\n");
    run_command("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1");
    
    return 0;
}
